#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace cm {

namespace net {
class TcpSocket;
}

// CometSuite.exe (host) へ接続するクライアント。専用スレッドで動く。
//
// 流れ:
//   1. セッションファイルを読んで接続を試す (host が既に起動していればここで成功する)
//   2. つながらなければ、CometSuite.exe を 1 回だけ起動し、接続できるまで待つ
//   3. Hello / Welcome を交換する。続けて Ping を送り、Pong が返れば往復の確認ができる
//   4. 以後は host からのメッセージを受ける。切断されたら終了する (自動再接続は未実装)
//
// 注意: StatusCallback は、このクラスのスレッドから呼ばれる。GUI は直接触らず、
// CallAfter などでメインスレッドに渡すこと。AviUtl2 のメインスレッドを待たせないため、
// ネットワーク処理はすべてこのスレッドで行う。
class HostClient {
public:
    using StatusCallback = std::function<void(const std::wstring&)>;

    HostClient();
    ~HostClient();

    HostClient(const HostClient&) = delete;
    HostClient& operator=(const HostClient&) = delete;

    // 接続を始める。すでに動作中なら何もしない。
    void Start(StatusCallback callback);

    // 切断してスレッドを止める。
    void Stop();

private:
    void Run();
    void Report(const std::wstring& text);

    StatusCallback m_callback;
    std::thread m_thread;
    std::atomic<bool> m_stop{false};
    std::atomic<bool> m_running{false};

    std::mutex m_mutex; // m_active を守る
    net::TcpSocket* m_active = nullptr;
};

} // namespace cm
