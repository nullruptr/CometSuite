#include "ipc/host_client.hpp"
#include "protocol/protocol.hpp"
#include "protocol/session_file.hpp"
#include "transport/message_io.hpp"
#include "transport/tcp_socket.hpp"
#include <chrono>
#include <cstring>
#include <vector>

namespace cm {

namespace {

bool FileExists(const std::wstring& path)
{
    const DWORD attr = GetFileAttributesW(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

// host の実行ファイルの場所を返す。
//   このプラグイン (.aux2) と同じフォルダの CometSuite\CometSuite.exe のみ。
// 見つからなければ空文字列。
std::wstring ResolveHostExe()
{
    HMODULE module = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&ResolveHostExe), &module))
        return L"";

    wchar_t path[MAX_PATH] = {};
    if (GetModuleFileNameW(module, path, MAX_PATH) == 0)
        return L"";
    std::wstring dir(path);
    dir = dir.substr(0, dir.find_last_of(L"\\/") + 1);

    const std::wstring exe = dir + L"CometSuite\\CometSuite.exe";
    return FileExists(exe) ? exe : L"";
}

// CometSuite.exe を起動する。AviUtl2 の終了とは独立して動く別プロセス。
bool LaunchHost()
{
    const std::wstring exe = ResolveHostExe();
    if (exe.empty())
        return false;

    std::wstring command = L"\"" + exe + L"\"";
    const std::wstring dir = exe.substr(0, exe.find_last_of(L"\\/"));

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(exe.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr, dir.c_str(), &si, &pi))
        return false;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

} // namespace

HostClient::HostClient() = default;

HostClient::~HostClient()
{
    Stop();
}

void HostClient::Start(StatusCallback callback)
{
    if (m_running.exchange(true))
        return;
    // 前回の接続が終わっていれば、スレッドを回収してから作り直す (joinable のまま代入すると terminate する)
    if (m_thread.joinable())
        m_thread.join();
    m_callback = std::move(callback);
    m_stop = false;
    m_thread = std::thread(&HostClient::Run, this);
}

void HostClient::Stop()
{
    m_stop = true;
    {
        // 受信待ちを解除する。可能なら Bye を送ってから閉じる。
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_active) {
            net::WriteMessage(*m_active, protocol::MessageType::Bye, nullptr, 0);
            m_active->Close();
        }
    }
    if (m_thread.joinable())
        m_thread.join();
    m_running = false;
}

void HostClient::Report(const std::wstring& text)
{
    if (m_callback)
        m_callback(text);
}

void HostClient::Run()
{
    using namespace std::chrono;

    Report(L"Connecting to CometSuite...");

    // 1. 接続する。つながらなければ host を 1 回だけ起動して、待つ。
    net::TcpSocket sock;
    protocol::SessionInfo session;
    bool launched = false;
    const auto deadline = steady_clock::now() + seconds(20);

    while (!m_stop) {
        if (protocol::ReadSession(session) && sock.Connect(session.port))
            break;

        if (!launched) {
            launched = true;
            if (!LaunchHost()) {
                Report(L"Cannot find CometSuite\\CometSuite.exe next to the plugin");
                m_running = false;
                return;
            }
            Report(L"Launching CometSuite...");
        }
        if (steady_clock::now() > deadline) {
            Report(L"Timed out waiting for CometSuite");
            m_running = false;
            return;
        }
        std::this_thread::sleep_for(milliseconds(200));
    }
    if (m_stop || !sock.IsValid()) {
        m_running = false;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_active = &sock;
    }

    // 2. Hello / Welcome
    bool ok = false;
    protocol::Header header{};
    std::vector<uint8_t> body;
    do {
        protocol::Hello hello{};
        hello.protocolVersion = protocol::kVersion;
        hello.connectorKind = static_cast<uint32_t>(protocol::ConnectorKind::AviUtl2);
        hello.processId = GetCurrentProcessId();
        std::memcpy(hello.token, session.token.data(), protocol::kTokenSize);
        if (!net::WriteMessage(sock, protocol::MessageType::Hello, &hello, sizeof(hello), 1))
            break;

        if (!net::ReadMessage(sock, header, body) ||
            header.type != static_cast<uint32_t>(protocol::MessageType::Welcome) ||
            body.size() != sizeof(protocol::Welcome))
            break;
        protocol::Welcome welcome{};
        std::memcpy(&welcome, body.data(), sizeof(welcome));
        if (welcome.result != static_cast<uint32_t>(protocol::WelcomeResult::Ok)) {
            Report(L"CometSuite rejected the connection (token or protocol version mismatch)");
            break;
        }

        Report(L"Connected to CometSuite (host pid " + std::to_wstring(welcome.hostProcessId) + L")");
        ok = true;

        // 3. Ping を送って、往復を確認する (応答は下のループで受ける)
        net::WriteMessage(sock, protocol::MessageType::Ping, nullptr, 0, 2);

        // 4. host からのメッセージを受ける
        bool bye = false;
        while (!bye && net::ReadMessage(sock, header, body)) {
            switch (static_cast<protocol::MessageType>(header.type)) {
            case protocol::MessageType::Ping:
                net::WriteMessage(sock, protocol::MessageType::Pong, nullptr, 0, header.requestId);
                break;
            case protocol::MessageType::Pong:
                Report(L"Connected to CometSuite (round trip OK)");
                break;
            case protocol::MessageType::Bye:
                bye = true;
                break;
            default:
                break;
            }
        }
    } while (false);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_active = nullptr;
    }
    sock.Close();

    if (!m_stop)
        Report(ok ? L"Disconnected from CometSuite" : L"Handshake with CometSuite failed");
    m_running = false;
}

} // namespace cm
