#pragma once
#include <wx/wx.h>

namespace cm::ipc {

class App : public wxApp {
public:
    bool OnInit() override;
};

} // namespace cm::ipc
