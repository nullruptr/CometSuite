#pragma once
#include <winsock2.h>
#include <windows.h>
#include <wx/wx.h>

namespace cm {

// AviUtl2 に登録した wx のフレームで、High DPI を正しく扱うためのパッチ。
// (背景と原因は CLAUDE.md の「wxWidgets の High DPI (DLL)」を参照)
//
// 使い方: フレームのメンバに置き、MSWWindowProc から呼ぶ。
//
//   MainFrame::MainFrame() : wxFrame(...), m_dpi_patch(*this) { ... }
//
//   WXLRESULT MainFrame::MSWWindowProc(WXUINT msg, WXWPARAM w, WXLPARAM l)
//   {
//       if (m_dpi_patch.BeforeWndProc(msg))
//           return 0;
//       const WXLRESULT r = wxFrame::MSWWindowProc(msg, w, l);
//       m_dpi_patch.AfterWndProc(msg);
//       return r;
//   }
class AviUtl2DpiPatch {
public:
    // window は構築済みであること (コンストラクタで GetDPI を呼ぶ)。
    explicit AviUtl2DpiPatch(wxWindow& window);

    // wx に渡す前に呼ぶ。true なら wx に渡さず、戻り値 0 で返すこと。
    // 対象は WM_DPICHANGED。ドッキング切替時に DPI 96 の仮の親から届くものを wx に渡すと、
    // コントロールが縮んだまま戻らなくなる。DPI は Sync() で管理するので常に捨てる。
    bool BeforeWndProc(WXUINT msg);

    // wx の処理が終わった後に呼ぶ。DPI を調べ直すタイミングのメッセージを見て Sync() する。
    void AfterWndProc(WXUINT msg);

    // 実際の DPI が最後に適用した DPI と違えば、wx に DPI 変更を適用して再配置する。
    void Sync();

private:
    wxWindow& m_window;
    wxSize m_dpi;           // 最後に wx へ適用した DPI
    bool m_syncing = false; // Sync の再入防止
};

// 親が AviUtl2 のコンテナ (本体、または非ドッキング用の最上位ウィンドウ) かを調べる。
// ドッキング切替の途中の、DPI 96 の仮の親にいる間は false。
// 注意: クラス名 (aviutl2Manager) は AviUtl2 の内部実装に依存する。
bool IsInAviUtl2Container(HWND hwnd);

// w 以下の全ウィンドウの sizer を再配置する。
// wxWindow::Layout() は自分の sizer しか配置しないため、子パネルは自分にサイズ変更が来るまで
// 再配置されない。DPI 変更でサイズが変わらない場合に古い配置が残るのを防ぐ。
void RelayoutAll(wxWindow* w);

} // namespace cm
