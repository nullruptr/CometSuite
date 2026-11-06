#include "ui/mainframe.hpp"
#include <wx/filefn.h>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/msw/colour.h>
#include <wx/msw/stattext.h>
#include <wx/toplevel.h>
#include <wx/wx.h>
#include "wxButtonCustom.hpp"

namespace cm::ipc {

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, _("IPC"), wxDefaultPosition, wxDefaultSize)
{
    auto* panel = new wxPanel(this);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    auto* root = new wxBoxSizer(wxVERTICAL);

    // 一番上
    auto* badge_sizer = new wxGridBagSizer(FromDIP(1), FromDIP(1));

    auto* connections_panel = new wxPanel(panel);
    connections_panel->SetBackgroundColour(*wxBLUE);
    auto* connections_sizer = new wxBoxSizer(wxVERTICAL);
    connections_panel->SetSizer(connections_sizer);

    // 接続数
    wxStaticText* connections = new wxStaticText(connections_panel, wxID_ANY, wxT("接続数:"));
    connections->SetForegroundColour(*wxWHITE);

    wxFont font = connections->GetFont();
    font.SetFaceName(wxT("ＭＳ 明朝"));
    font.SetPointSize(12);
    connections->SetFont(font);
    // ---

    // 接続数の数字
    auto* connections_num_panel = new wxPanel(panel);
    connections_num_panel->SetBackgroundColour(wxColour(64, 255, 255));
    auto* connections_num_sizer = new wxBoxSizer(wxVERTICAL);
    connections_num_panel->SetSizer(connections_num_sizer);

    m_connections_num = new wxStaticText(connections_num_panel, wxID_ANY, "0");

    font = m_connections_num->GetFont();
    font.SetFaceName(wxT("ＭＳ 明朝"));
    font.SetPointSize(12);
    m_connections_num->SetFont(font);
    // ---

    // ホスト
    auto* host_panel = new wxPanel(panel);
    host_panel->SetBackgroundColour(*wxBLUE);
    auto* host_sizer = new wxBoxSizer(wxVERTICAL);
    host_panel->SetSizer(host_sizer);

    wxStaticText* host = new wxStaticText(host_panel, wxID_ANY, wxT("ホスト:"));
    host->SetForegroundColour(*wxWHITE);
    host->SetFont(connections->GetFont());
    // --- 

    // ホスト数字
    auto* host_num_panel = new wxPanel(panel);
    host_num_panel->SetBackgroundColour(wxColour(64, 255, 255));
    auto* host_num_sizer = new wxBoxSizer(wxVERTICAL);
    host_num_panel->SetSizer(host_num_sizer);

    m_host_num = new wxStaticText(host_num_panel, wxID_ANY, "0");
    m_host_num->SetFont(connections->GetFont());
    // ---

    // 状態
    auto* kinds_panel = new wxPanel(panel);
    kinds_panel->SetBackgroundColour(*wxBLUE);
    auto* kinds_sizer = new wxBoxSizer(wxVERTICAL);
    kinds_panel->SetSizer(kinds_sizer);

    wxStaticText* kinds = new wxStaticText(kinds_panel, wxID_ANY, wxT("種類:"));
    kinds->SetForegroundColour(*wxWHITE);
    kinds->SetFont(connections->GetFont());
    // ---

    // 状態内容
    auto* kinds_text_panel = new wxPanel(panel);
    kinds_text_panel->SetBackgroundColour(wxColour(64, 255, 255));
    auto* kinds_text_sizer = new wxBoxSizer(wxVERTICAL);
    kinds_text_panel->SetSizer(kinds_text_sizer);

    m_kinds_text = new wxStaticText(kinds_text_panel, wxID_ANY, "AviUtl2 / YMM4");
    m_kinds_text->SetFont(connections->GetFont());
    // ---

    // PID
    auto* pid_panel = new wxPanel(panel);
    pid_panel->SetBackgroundColour(*wxBLUE);
    auto* pid_sizer = new wxBoxSizer(wxVERTICAL);
    pid_panel->SetSizer(pid_sizer);

    wxStaticText* pid_title = new wxStaticText(pid_panel, wxID_ANY, wxT("ＰＩＤ:"));
    pid_title->SetForegroundColour(*wxWHITE);
    pid_title->SetFont(connections->GetFont());
    // ---

    // PID num
    auto* pid_num_panel = new wxPanel(panel);
    pid_num_panel->SetBackgroundColour(wxColour(64, 255, 255));
    auto* pid_num_sizer = new wxBoxSizer(wxVERTICAL);
    pid_num_panel->SetSizer(pid_num_sizer);

    m_pid_num = new wxStaticText(pid_num_panel, wxID_ANY, "1234");
    m_pid_num->SetFont(connections->GetFont());
    // ---

    // 状態
    auto* state_panel = new wxPanel(panel);
    state_panel->SetBackgroundColour(*wxBLUE);
    auto* state_sizer = new wxBoxSizer(wxVERTICAL);
    state_panel->SetSizer(state_sizer);

    wxStaticText* state = new wxStaticText(state_panel, wxID_ANY, wxT("状態:"));
    state->SetForegroundColour(*wxWHITE);
    state->SetFont(connections->GetFont());
    // ---

    // 状態内容
    auto* state_text_panel = new wxPanel(panel);
    state_text_panel->SetBackgroundColour(wxColour(64, 255, 255));
    auto* state_text_sizer = new wxBoxSizer(wxVERTICAL);
    state_text_panel->SetSizer(state_text_sizer);

    m_state_text = new wxStaticText(state_text_panel, wxID_ANY, wxT("接続中"));
    m_state_text->SetFont(connections->GetFont());
    // ---

    // ボタン
    wxFont button_font = connections->GetFont();
    button_font.MakeBold();

    auto* button = new wxButtonCustom(panel, wxID_ANY, wxT("ボタン"));
    button->SetColors(*wxWHITE, *wxBLUE);
    button->SetButtonFont(button_font);
    // ---

    // ステータス赤文字
    m_status_text = new wxStaticText(panel, wxID_ANY, wxT("ここに状態メッセージを表示。"));
    m_status_text->SetForegroundColour(*wxRED);

    font = m_status_text->GetFont();
    font.SetFaceName(wxT("ＭＳ 明朝"));
    font.SetPointSize(12);
    m_status_text->SetFont(font);
    // ---

    connections_sizer->Add(connections, 0, wxALL, FromDIP(10));
    connections_num_sizer->Add(m_connections_num, 0, wxALL, FromDIP(10));
    host_sizer->Add(host, 0, wxALL, FromDIP(10));
    host_num_sizer->Add(m_host_num, 0, wxALL, FromDIP(10));
    kinds_sizer->Add(kinds, 0, wxALL, FromDIP(10));
    kinds_text_sizer->Add(m_kinds_text, 0, wxALL, FromDIP(10));
    pid_sizer->Add(pid_title, 0, wxALL, FromDIP(10));
    pid_num_sizer->Add(m_pid_num, 0, wxALL, FromDIP(10));
    state_sizer->Add(state, 0, wxALL, FromDIP(10));
    state_text_sizer->Add(m_state_text, 0, wxALL, FromDIP(10));

    badge_sizer->Add(connections_panel, wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
    badge_sizer->Add(connections_num_panel, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);
    badge_sizer->Add(host_panel, wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND);
    badge_sizer->Add(host_num_panel, wxGBPosition(0, 3), wxDefaultSpan, wxEXPAND);
    badge_sizer->Add(kinds_panel, wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);
    badge_sizer->Add(kinds_text_panel, wxGBPosition(1, 1), wxGBSpan(1, 3), wxEXPAND);
    badge_sizer->Add(pid_panel, wxGBPosition(1, 4), wxDefaultSpan, wxEXPAND);
    badge_sizer->Add(pid_num_panel, wxGBPosition(1, 5), wxDefaultSpan, wxEXPAND);
    badge_sizer->Add(state_panel, wxGBPosition(1, 6), wxDefaultSpan, wxEXPAND);
    badge_sizer->Add(state_text_panel, wxGBPosition(1, 7), wxDefaultSpan, wxEXPAND);
    badge_sizer->Add(button, wxGBPosition(2, 6), wxGBSpan(1, 2), wxEXPAND);
    sizer->Add(badge_sizer, 0, wxALL, FromDIP(0));
    sizer->AddStretchSpacer(1);
    sizer->Add(m_status_text, 0, wxALL, FromDIP(8));

    root->Add(panel, 1, wxEXPAND);
    SetSizer(root);

    SetSize(FromDIP(wxSize(500, 500)));
    SetBackgroundColour(wxColour(224, 224, 224));
}

} // namespace cm::ipc
