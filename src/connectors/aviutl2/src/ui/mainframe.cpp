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

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    // コンストラクタ内で FromDIP するとクラッシュするため、wxSizerFlags の既定の余白を使う
    sizer->Add(button, wxSizerFlags().Border(wxALL));
    panel->SetSizer(sizer);

    // AviUtl2 のテーマ色を適用 (背景・ボタン・文字)
    cm::theme::ApplyTree(*panel);
}

WXLRESULT MainFrame::MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
{
    if (m_dpi_patch.BeforeWndProc(msg))
        return 0;
    const WXLRESULT r = wxFrame::MSWWindowProc(msg, wParam, lParam);
    m_dpi_patch.AfterWndProc(msg);
    return r;
}
