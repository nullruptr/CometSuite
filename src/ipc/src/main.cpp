#include "main.hpp"
#include "ui/mainframe.hpp"

wxIMPLEMENT_APP(cm::ipc::App);

namespace cm::ipc {

bool App::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    auto* frame = new MainFrame();
    frame->Show();
    return true;
}

} // namespace cm::ipc
