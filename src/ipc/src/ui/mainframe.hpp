#pragma once
#include <wx/wx.h>

namespace cm::ipc {

class MainFrame : public wxFrame {
public:
    MainFrame();
private:
    wxStaticText* m_connections_num;
    wxStaticText* m_host_num;
    wxStaticText* m_status_text;
    wxStaticText* m_kinds_text;
    wxStaticText* m_pid_num;
    wxStaticText* m_state_text;
};

} // namespace cm::ipc
