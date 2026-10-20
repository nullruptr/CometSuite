#pragma once
#include <wx/wx.h>
#include "ipc/ipc_server.hpp"

class MainFrame : public wxFrame {
public:
    MainFrame();
    ~MainFrame() override;

    void ApplySizes();

private:
    void StartIpc();

    wxButton* m_mta_button = nullptr;
    wxButton* m_exit_button = nullptr;
    cm::IpcServer m_server;
};
