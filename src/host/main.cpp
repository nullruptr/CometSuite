#include "main.hpp"
#include "ui/mainframe.hpp"

wxIMPLEMENT_APP(cm::App);

namespace cm {

bool App::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    auto* frame = new MainFrame();
    // コンストラクタ内で FromDIP するとクラッシュするため、サイズは生成後に設定する
    frame->SetSize(frame->FromDIP(wxSize(1280, 720)));
    frame->Show();
    return true;
}

} // namespace cm
