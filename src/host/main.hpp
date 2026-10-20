#pragma once
#include <wx/wx.h>

namespace cm {

class App : public wxApp {
public:
    bool OnInit() override;
};

} // namespace cm
