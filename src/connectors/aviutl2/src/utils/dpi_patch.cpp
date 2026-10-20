#include "utils/dpi_patch.hpp"

namespace cm {

namespace {
constexpr WXUINT kWmDpiChanged = 0x02E0;
constexpr WXUINT kWmDpiChangedAfterParent = 0x02E3;
} // namespace

bool IsInAviUtl2Container(HWND hwnd)
{
    HWND root = GetAncestor(hwnd, GA_ROOT);
    wchar_t cls[64] = {};
    if (!root || !GetClassNameW(root, cls, 64))
        return false;
    return wcscmp(cls, L"aviutl2Manager") == 0;
}

void RelayoutAll(wxWindow* w)
{
    w->Layout();
    for (wxWindow* child : w->GetChildren()) {
        if (!child->IsTopLevel())
            RelayoutAll(child);
    }
}

AviUtl2DpiPatch::AviUtl2DpiPatch(wxWindow& window)
    : m_window(window)
    , m_dpi(window.GetDPI())
{
}

bool AviUtl2DpiPatch::BeforeWndProc(WXUINT msg)
{
    if (msg != kWmDpiChanged)
        return false;
    // WM_DPICHANGED 自体は届かない/信用できないことがあるので、通知を契機に調べ直すだけにする。
    Sync();
    return true;
}

void AviUtl2DpiPatch::AfterWndProc(WXUINT msg)
{
    // フレームは WS_CHILD のため、非ドッキングで DPI の異なるモニターへ動かしても
    // WM_DPICHANGED は届かない (最上位のコンテナにしか届かない)。
    // コンテナがリサイズされれば WM_SIZE などが届くので、そのタイミングで調べ直す。
    // WM_DPICHANGED_AFTERPARENT は DPI の値を運ばない。親が確定した合図として使う。
    switch (msg) {
    case kWmDpiChangedAfterParent:
    case WM_SIZE:
    case WM_WINDOWPOSCHANGED:
        Sync();
        break;
    }
}

void AviUtl2DpiPatch::Sync()
{
    if (m_syncing)
        return;
    HWND hwnd = static_cast<HWND>(m_window.GetHWND());
    // 付け替えの途中 (DPI 96 の仮の親) の値は使わない
    if (!hwnd || !IsInAviUtl2Container(hwnd))
        return;
    const UINT dpi = GetDpiForWindow(hwnd);
    if (dpi == 0 || static_cast<int>(dpi) == m_dpi.x)
        return;

    // wx 内部の m_activeDPI は private で更新できないため、DPI はここで自前管理する。
    // (wx に WM_DPICHANGED を渡すと二重にスケールされる)
    m_syncing = true;
    const wxSize new_dpi(static_cast<int>(dpi), static_cast<int>(dpi));
    // サイズ・フォント・sizer を再スケールし、wxEVT_DPI_CHANGED を送る (wx 自身の処理と同じ)
    m_window.MSWUpdateOnDPIChange(m_dpi, new_dpi);
    m_dpi = new_dpi;
    RelayoutAll(&m_window);
    m_window.Refresh();
    m_syncing = false;
}

} // namespace cm
