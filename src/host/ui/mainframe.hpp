#pragma once
#include <wx/wx.h>
#include "ipc/ipc_server.hpp"

namespace cm {

class MainFrame : public wxFrame {
public:
    MainFrame();
    ~MainFrame() override;

private:
    void StartIpc();

    IpcServer m_server;
};

} // namespace cm
