#pragma once
#include <wx/wx.h>
#include "utils/dpi_patch.hpp"

// AviUtl2 のウィンドウクライアントとして登録する wx のフレーム。
class MainFrame : public wxFrame {
public:
    MainFrame();

protected:
    WXLRESULT MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam) override;

private:
    cm::AviUtl2DpiPatch m_dpi_patch;
};
