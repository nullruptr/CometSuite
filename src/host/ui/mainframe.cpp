#include "ui/mainframe.hpp"
#include "protocol/protocol.hpp"
#include <Eigen/Core>
#include <opencv2/core/version.hpp>

namespace {
// ステータスバーのフィールド
enum StatusField { kStatusInfo = 0, kStatusConnection = 1 };

wxString ConnectorName(uint32_t kind)
{
    switch (static_cast<cm::protocol::ConnectorKind>(kind)) {
    case cm::protocol::ConnectorKind::AviUtl2:
        return "AviUtl2";
    case cm::protocol::ConnectorKind::YMM4:
        return "YMM4";
    default:
        return "Unknown";
    }
}
} // namespace

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "CometSuite")
{
    auto* panel = new wxPanel(this);
    wxFont font = GetFont();
    font.SetPointSize(16);
    font.MakeBold();
    panel->SetFont(font);

    auto* title = new wxStaticText(panel, wxID_ANY, _("MAIN MENU"));
    m_mta_button = new wxButton(panel, wxID_ANY, _("Motion Tracking Analyzer"));
    m_exit_button = new wxButton(panel, wxID_ANY, _("EXIT"));
    m_mta_button->Disable();
    m_exit_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Close(); });

    auto* panel_sizer = new wxBoxSizer(wxVERTICAL);
    const auto flags = wxSizerFlags().Align(wxALIGN_CENTER_HORIZONTAL).Border(wxALL);
    panel_sizer->Add(title, flags);
    panel_sizer->Add(m_mta_button, flags);
    panel_sizer->AddStretchSpacer();
    panel_sizer->Add(m_exit_button, flags);
    panel->SetSizer(panel_sizer);

    auto* frame_sizer = new wxBoxSizer(wxVERTICAL);
    frame_sizer->Add(panel, 1, wxEXPAND);
    SetSizer(frame_sizer);

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

void MainFrame::ApplySizes()
{
    const wxSize button_size = FromDIP(wxSize(350, 50));
    m_mta_button->SetMinSize(button_size);
    m_exit_button->SetMinSize(button_size);
}

MainFrame::~MainFrame()
{
    // スレッドを止めてから、メンバとウィンドウを破棄する (コールバックが破棄済みの this を触らないように)
    m_server.Stop();
}

void MainFrame::StartIpc()
{
    // コールバックは IPC のスレッドから呼ばれる。GUI は CallAfter でメインスレッドに渡す。
    cm::IpcServer::Callbacks callbacks;
    callbacks.onConnected = [this](const cm::ConnectorInfo& info) {
        CallAfter([this, info] {
            SetStatusText(wxString::Format("Connected: %s (pid %u)", ConnectorName(info.kind), info.processId),
                          kStatusConnection);
        });
    };
    callbacks.onDisconnected = [this](const cm::ConnectorInfo& info) {
        CallAfter([this, info] {
            SetStatusText(wxString::Format("Disconnected: %s", ConnectorName(info.kind)), kStatusConnection);
        });
    };

    if (m_server.Start(std::move(callbacks)))
        SetStatusText(wxString::Format("Waiting for connector (port %u)", m_server.Port()), kStatusConnection);
    else
        SetStatusText("IPC: failed to start", kStatusConnection);
}
