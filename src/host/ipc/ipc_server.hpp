#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace cm {

namespace net {
class TcpSocket;
}

// 接続してきたコネクタの情報 (protocol::ConnectorKind / プロセス ID)
struct ConnectorInfo {
    uint32_t kind = 0;
    uint32_t processId = 0;
};

// 全コネクタを受けるサーバー。
//
// 127.0.0.1 の空きポートで待ち受け、接続情報 (ポート・トークン) をセッションファイルに書く。
// コネクタはそれを読んで接続する。接続ごとにスレッドを 1 本使う。
//
// 注意: コールバックは、サーバーのスレッドから呼ばれる。GUI は直接触らず、
// wxEvtHandler::CallAfter などでメインスレッドに渡すこと。
class IpcServer {
public:
    struct Callbacks {
        std::function<void(const ConnectorInfo&)> onConnected;
        std::function<void(const ConnectorInfo&)> onDisconnected;
    };

    IpcServer();
    ~IpcServer();

    IpcServer(const IpcServer&) = delete;
    IpcServer& operator=(const IpcServer&) = delete;

    // 待ち受けを始め、セッションファイルを書く。失敗したら false。
    bool Start(Callbacks callbacks);

    // 全接続を閉じ、スレッドを止めて、セッションファイルを消す。
    void Stop();

    uint16_t Port() const { return m_port; }

private:
    struct Client;

    void AcceptLoop();
    void ClientLoop(Client* client);

    Callbacks m_callbacks;
    std::unique_ptr<net::TcpSocket> m_listen;
    std::thread m_accept_thread;
    std::atomic<bool> m_running{false};
    uint16_t m_port = 0;
    std::string m_token;

    std::mutex m_clients_mutex;
    std::vector<std::unique_ptr<Client>> m_clients;
};

} // namespace cm
