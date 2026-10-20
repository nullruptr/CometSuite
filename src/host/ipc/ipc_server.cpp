#include "ipc/ipc_server.hpp"
#include "protocol/protocol.hpp"
#include "protocol/session_file.hpp"
#include "transport/message_io.hpp"
#include "transport/tcp_socket.hpp"
#include <chrono>
#include <cstring>
#include <random>

namespace cm {

struct IpcServer::Client {
    net::TcpSocket sock;
    std::thread thread;
};

namespace {
// 16 バイトの乱数を 16 進にした 32 文字。他のプロセスが勝手に接続してこないようにする合言葉。
std::string MakeToken()
{
    static const char kHex[] = "0123456789abcdef";
    std::random_device rd;
    std::string token;
    token.reserve(protocol::kTokenSize);
    for (int i = 0; i < 4; ++i) {
        const uint32_t v = rd();
        for (int b = 0; b < 8; ++b)
            token.push_back(kHex[(v >> (b * 4)) & 0xF]);
    }
    return token;
}
} // namespace

IpcServer::IpcServer() = default;

IpcServer::~IpcServer()
{
    Stop();
}

bool IpcServer::Start(Callbacks callbacks)
{
    if (m_running)
        return false;

    m_callbacks = std::move(callbacks);
    m_listen = std::make_unique<net::TcpSocket>();
    m_port = m_listen->Listen(0);
    if (m_port == 0)
        return false;

    m_token = MakeToken();
    protocol::SessionInfo info;
    info.port = m_port;
    info.token = m_token;
    info.pid = GetCurrentProcessId();
    if (!protocol::WriteSession(info)) {
        m_listen->Close();
        m_port = 0;
        return false;
    }

    m_running = true;
    m_accept_thread = std::thread(&IpcServer::AcceptLoop, this);
    return true;
}

void IpcServer::Stop()
{
    if (!m_running.exchange(false))
        return;

    // 待機中の accept を解除して、スレッドを止める
    m_listen->Close();
    if (m_accept_thread.joinable())
        m_accept_thread.join();

    // 受信待ちの接続も閉じて解除し、スレッドを止める
    {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        for (auto& client : m_clients)
            client->sock.Close();
        for (auto& client : m_clients) {
            if (client->thread.joinable())
                client->thread.join();
        }
        m_clients.clear();
    }

    protocol::RemoveSession(GetCurrentProcessId());
    m_port = 0;
}

void IpcServer::AcceptLoop()
{
    while (m_running) {
        net::TcpSocket accepted = m_listen->Accept();
        if (!accepted.IsValid()) {
            if (!m_running)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 失敗が続くときの空回り防止
            continue;
        }

        auto client = std::make_unique<Client>();
        client->sock = std::move(accepted);
        Client* raw = client.get();

        std::lock_guard<std::mutex> lock(m_clients_mutex);
        m_clients.push_back(std::move(client));
        raw->thread = std::thread(&IpcServer::ClientLoop, this, raw);
    }
}

void IpcServer::ClientLoop(Client* client)
{
    net::TcpSocket& sock = client->sock;
    protocol::Header header{};
    std::vector<uint8_t> body;

    // 最初のメッセージは Hello でなければならない
    if (!net::ReadMessage(sock, header, body) || header.type != static_cast<uint32_t>(protocol::MessageType::Hello) ||
        body.size() != sizeof(protocol::Hello))
        return;

    protocol::Hello hello{};
    std::memcpy(&hello, body.data(), sizeof(hello));

    protocol::Welcome welcome{};
    welcome.protocolVersion = protocol::kVersion;
    welcome.hostProcessId = GetCurrentProcessId();
    welcome.result = static_cast<uint32_t>(protocol::WelcomeResult::Ok);
    if (hello.protocolVersion != protocol::kVersion)
        welcome.result = static_cast<uint32_t>(protocol::WelcomeResult::VersionMismatch);
    else if (std::memcmp(hello.token, m_token.data(), protocol::kTokenSize) != 0)
        welcome.result = static_cast<uint32_t>(protocol::WelcomeResult::BadToken);

    net::WriteMessage(sock, protocol::MessageType::Welcome, &welcome, sizeof(welcome), header.requestId);
    if (welcome.result != static_cast<uint32_t>(protocol::WelcomeResult::Ok))
        return;

    const ConnectorInfo info{hello.connectorKind, hello.processId};
    if (m_callbacks.onConnected)
        m_callbacks.onConnected(info);

    bool bye = false;
    while (!bye && net::ReadMessage(sock, header, body)) {
        switch (static_cast<protocol::MessageType>(header.type)) {
        case protocol::MessageType::Ping:
            net::WriteMessage(sock, protocol::MessageType::Pong, nullptr, 0, header.requestId);
            break;
        case protocol::MessageType::Bye:
            bye = true;
            break;
        default:
            break; // 未知のメッセージは無視する
        }
    }

    if (m_callbacks.onDisconnected)
        m_callbacks.onDisconnected(info);
}

} // namespace cm
