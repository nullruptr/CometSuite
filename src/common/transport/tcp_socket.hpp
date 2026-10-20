#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#error "TcpSocket is implemented for Windows only so far (POSIX sockets are planned for Linux / macOS)."
#endif

// 127.0.0.1 限定の TCP ソケットの薄いラッパー。
// 制御メッセージ用 (映像は共有メモリで送る予定)。Wine の内側と外側のプロセスの間でも使える。
namespace cm::net {

namespace detail {
inline void EnsureWinsock()
{
    // 一度だけ初期化する。DLL のアンロード時の WSACleanup は、ローダーロック中の呼び出しを避けるため行わない。
    static const bool ok = [] {
        WSADATA data;
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    (void)ok;
}
} // namespace detail

class TcpSocket {
public:
    TcpSocket() = default;
    explicit TcpSocket(SOCKET s) : m_sock(s) {}
    ~TcpSocket() { Close(); }

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;
    TcpSocket(TcpSocket&& other) noexcept : m_sock(other.m_sock.exchange(INVALID_SOCKET)) {}
    TcpSocket& operator=(TcpSocket&& other) noexcept
    {
        if (this != &other) {
            Close();
            m_sock = other.m_sock.exchange(INVALID_SOCKET);
        }
        return *this;
    }

    bool IsValid() const { return m_sock.load() != INVALID_SOCKET; }

    // 127.0.0.1:port で待ち受ける。port=0 なら空いているポートを OS が選ぶ。
    // 戻り値は実際のポート (失敗は 0)。
    uint16_t Listen(uint16_t port = 0)
    {
        detail::EnsureWinsock();
        Close();
        SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET)
            return 0;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
        int len = sizeof(addr);
        if (::bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 || ::listen(s, SOMAXCONN) != 0 ||
            ::getsockname(s, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
            ::closesocket(s);
            return 0;
        }
        m_sock = s;
        return ntohs(addr.sin_port);
    }

    // 接続を 1 つ受ける (ブロックする)。別スレッドが Close() すると、無効なソケットを返して戻る。
    TcpSocket Accept()
    {
        const SOCKET listener = m_sock.load();
        if (listener == INVALID_SOCKET)
            return TcpSocket();
        SOCKET client = ::accept(listener, nullptr, nullptr);
        if (client == INVALID_SOCKET)
            return TcpSocket();
        TcpSocket result(client);
        result.SetNoDelay();
        return result;
    }

    // 127.0.0.1:port へ接続する。
    bool Connect(uint16_t port)
    {
        detail::EnsureWinsock();
        Close();
        SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET)
            return false;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
        if (::connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
            ::closesocket(s);
            return false;
        }
        m_sock = s;
        SetNoDelay();
        return true;
    }

    // 全部送るまで繰り返す。
    bool SendAll(const void* data, size_t size)
    {
        const SOCKET s = m_sock.load();
        const char* p = static_cast<const char*>(data);
        while (size > 0) {
            const int chunk = size > 0x10000 ? 0x10000 : static_cast<int>(size);
            const int n = ::send(s, p, chunk, 0);
            if (n <= 0)
                return false;
            p += n;
            size -= static_cast<size_t>(n);
        }
        return true;
    }

    // ちょうど size バイト受け取るまで繰り返す。切断されたら false。
    bool RecvExact(void* data, size_t size)
    {
        const SOCKET s = m_sock.load();
        char* p = static_cast<char*>(data);
        while (size > 0) {
            const int chunk = size > 0x10000 ? 0x10000 : static_cast<int>(size);
            const int n = ::recv(s, p, chunk, 0);
            if (n <= 0)
                return false;
            p += n;
            size -= static_cast<size_t>(n);
        }
        return true;
    }

    // 別スレッドで待機中の Accept / RecvExact も、これで解除される。
    void Close()
    {
        const SOCKET s = m_sock.exchange(INVALID_SOCKET);
        if (s != INVALID_SOCKET) {
            ::shutdown(s, SD_BOTH);
            ::closesocket(s);
        }
    }

private:
    void SetNoDelay()
    {
        // 小さな制御メッセージが、まとめ待ちで遅れないようにする
        const BOOL on = TRUE;
        ::setsockopt(m_sock.load(), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&on), sizeof(on));
    }

    std::atomic<SOCKET> m_sock{INVALID_SOCKET};
};

} // namespace cm::net
