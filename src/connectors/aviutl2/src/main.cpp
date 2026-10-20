#include <winsock2.h>
#include <windows.h>
#include <wx/wx.h>
#include "aviutl2_sdk/plugin2.h"
#include "ui/mainframe.hpp"

namespace {
HINSTANCE g_hModule = nullptr;
MainFrame* g_frame = nullptr;
bool g_wx_started = false;

COMMON_PLUGIN_TABLE g_plugin_table = {
    L"CometConnector for AviUtl2",
    L"CometConnector for AviUtl2",
};
} // namespace

// 本体のメインウィンドウ取得 (get_host_app_window) 用。mainframe.cpp から参照する。
EDIT_HANDLE* g_edit_handle = nullptr;

EXTERN_C __declspec(dllexport) COMMON_PLUGIN_TABLE* GetCommonPluginTable(void)
{
    return &g_plugin_table;
}

EXTERN_C __declspec(dllexport) void RegisterPlugin(HOST_APP_TABLE* host)
{
    // DLL 内では wxIMPLEMENT_APP が使えないので手動で起動する
    wxApp::SetInstance(new wxApp());
    int argc = 0;
    if (!wxEntryStart(argc, static_cast<wchar_t**>(nullptr)))
        return;
    g_wx_started = true;
    wxTheApp->CallOnInit();

    g_edit_handle = host->create_edit_handle();

    g_frame = new MainFrame();
    host->register_window_client(L"CometSuite", static_cast<HWND>(g_frame->GetHandle()));
}

EXTERN_C __declspec(dllexport) void UninitializePlugin()
{
    if (g_frame) {
        g_frame->Destroy();
        g_frame = nullptr;
    }
    if (g_wx_started) {
        wxEntryCleanup();
        g_wx_started = false;
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
        g_hModule = hinstDLL;
    return TRUE;
}
