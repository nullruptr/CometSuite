#include "ui/mainframe.hpp"
#include "utils/apply_theme.hpp"
#include "utils/message_box.hpp"
#include "wxButtonCustom.hpp"

MainFrame::MainFrame()
    // wxBORDER_NONE にしないと AviUtl2 側で MDI のような見た目になる
    : wxFrame(nullptr, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
    , m_dpi_patch(*this)
{
    auto* panel = new wxPanel(this);

    auto* button = new wxButtonCustom(panel, wxID_ANY, _("wxMessageBox"));

    button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        cm::ShowMessageBox(this, _("wxMessageBoxCustom + ModalGuard"), _("Comet"), wxOK);
    });

    // CometSuite.exe の起動と接続 (通信の検証用)
    auto* connect_button = new wxButtonCustom(panel, wxID_ANY, _("Connect to CometSuite"));
    m_status = new wxStaticText(panel, wxID_ANY, _("Not connected"));
    connect_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        // 通信は専用スレッドで行う。状態の表示は、CallAfter でメインスレッドに戻す。
        m_host_client.Start([this](const std::wstring& text) {
            CallAfter([this, text] { m_status->SetLabel(wxString(text)); });
        });
    });

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    // コンストラクタ内で FromDIP するとクラッシュするため、wxSizerFlags の既定の余白を使う
    sizer->Add(button, wxSizerFlags().Border(wxALL));
    sizer->Add(connect_button, wxSizerFlags().Border(wxALL));
    sizer->Add(m_status, wxSizerFlags().Border(wxALL));
    panel->SetSizer(sizer);

    // AviUtl2 のテーマ色を適用 (背景・ボタン・文字)
    cm::theme::ApplyTree(*panel);
}

MainFrame::~MainFrame()
{
    // 通信スレッドを止めてから破棄する (コールバックが破棄済みの this を触らないように)
    m_host_client.Stop();
}

WXLRESULT MainFrame::MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
{
    if (m_dpi_patch.BeforeWndProc(msg))
        return 0;
    const WXLRESULT r = wxFrame::MSWWindowProc(msg, wParam, lParam);
    m_dpi_patch.AfterWndProc(msg);
    return r;
}
