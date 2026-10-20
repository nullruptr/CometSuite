#include "ui/mainframe.hpp"
#include "protocol/protocol.hpp"
#include <Eigen/Core>
#include <opencv2/core/version.hpp>

namespace cm {

namespace {
// ステータスバーのフィールド
enum StatusField { kStatusInfo = 0, kStatusConnection = 1 };

wxString ConnectorName(uint32_t kind)
{
    switch (static_cast<protocol::ConnectorKind>(kind)) {
    case protocol::ConnectorKind::AviUtl2:
        return "AviUtl2";
    case protocol::ConnectorKind::YMM4:
        return "YMM4";
    default:
        return "Unknown";
    }
}
} // namespace

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "CometSuite")
{
    CreateStatusBar(2);
    const int widths[] = {-1, 320};
    SetStatusWidths(2, widths);

    // ビルド確認用: 依存ライブラリのバージョンを表示する
    SetStatusText(wxString::Format("wxWidgets %s / OpenCV %s / Eigen %d.%d.%d", wxVERSION_NUM_DOT_STRING,
                                   CV_VERSION, EIGEN_WORLD_VERSION, EIGEN_MAJOR_VERSION,
                                   EIGEN_MINOR_VERSION),
                  kStatusInfo);

    StartIpc();
}

MainFrame::~MainFrame()
{
    // スレッドを止めてから、メンバとウィンドウを破棄する (コールバックが破棄済みの this を触らないように)
    m_server.Stop();
}

void MainFrame::StartIpc()
{
    // コールバックは IPC のスレッドから呼ばれる。GUI は CallAfter でメインスレッドに渡す。
    IpcServer::Callbacks callbacks;
    callbacks.onConnected = [this](const ConnectorInfo& info) {
        CallAfter([this, info] {
            SetStatusText(wxString::Format("Connected: %s (pid %u)", ConnectorName(info.kind), info.processId),
                          kStatusConnection);
        });
    };
    callbacks.onDisconnected = [this](const ConnectorInfo& info) {
        CallAfter([this, info] {
            SetStatusText(wxString::Format("Disconnected: %s", ConnectorName(info.kind)), kStatusConnection);
        });
    };

    if (m_server.Start(std::move(callbacks)))
        SetStatusText(wxString::Format("Waiting for connector (port %u)", m_server.Port()), kStatusConnection);
    else
        SetStatusText("IPC: failed to start", kStatusConnection);
}

} // namespace cm
