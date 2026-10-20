#include "ui/mainframe.hpp"
#include "utils/modal_guard.hpp"
#include "wxButtonCustom.hpp"

#include "wxMessageBoxCustom.hpp"
#include <wx/msgdlg.h>

MainFrame::MainFrame()
    // wxBORDER_NONE にしないと AviUtl2 側で MDI のような見た目になる
    : wxFrame(nullptr, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
    , m_dpi_patch(*this)
{
    auto* panel = new wxPanel(this);

    // テーマ未対応。色は暫定でシステム色を使う。
    auto* button = new wxButtonCustom(panel, wxID_ANY, _("wxMessageBox"));
    button->SetColors(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE),
                      wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));

    button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        ModalGuard guard(static_cast<HWND>(GetHWND()));
        wxMessageBoxCustom(_("wxMessageBox + ModalGuard"), _("Comet"), wxOK, this);
    });

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    // コンストラクタ内で FromDIP するとクラッシュするため、wxSizerFlags の既定の余白を使う
    sizer->Add(button, wxSizerFlags().Border(wxALL));
    panel->SetSizer(sizer);
}

WXLRESULT MainFrame::MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
{
    if (m_dpi_patch.BeforeWndProc(msg))
        return 0;
    const WXLRESULT r = wxFrame::MSWWindowProc(msg, wParam, lParam);
    m_dpi_patch.AfterWndProc(msg);
    return r;
}
