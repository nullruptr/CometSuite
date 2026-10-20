#pragma once
#include <wx/wx.h>
#include "utils/apply_theme.hpp"
#include "utils/modal_guard.hpp"
#include "wxMessageBoxCustom.hpp"

namespace cm {

namespace detail {
// ダイアログを閉じる (EndModal) 直前に ModalGuard を解除する wxMessageBoxCustomDialog。
// 解除が閉じた後だと、全ウィンドウが無効なままダイアログが消え、AviUtl2 の外のウィンドウが
// 前面に出てしまう。ボタン・Esc・× のどれで閉じても EndModal を通る。
class GuardedMessageBoxDialog : public wxMessageBoxCustomDialog {
public:
    GuardedMessageBoxDialog(wxWindow* parent, const wxString& message, const wxString& caption,
                            long style, ModalGuard& guard)
        : wxMessageBoxCustomDialog(parent, message, caption, style)
        , m_guard(guard)
    {
    }

    void EndModal(int retCode) override
    {
        m_guard.Release();
        wxMessageBoxCustomDialog::EndModal(retCode);
    }

private:
    ModalGuard& m_guard;
};
} // namespace detail

// AviUtl2 上でモーダルになり、AviUtl2 のテーマ色で表示される wxMessageBox。
// 引数・戻り値は wxMessageBoxCustom (= wxMessageBox) と同じ。
//
// - ModalGuard で、AviUtl2 のメインスレッドの最上位ウィンドウ (本体、非ドッキングの各ウィンドウ)
//   を無効化する (そのままではモーダルにならないため)。ダイアログを作る前に置くこと。
// - wxMessageBoxCustom は色を固定で作るため、構築後に ApplyTree で AviUtl2 のテーマ色へ上書きする。
//   (ボタンは全て ButtonBody 系の色になり、既定ボタンの強調色の区別はなくなる)
//
// parent には、呼び出し元のウィンドウ (通常は this) を渡すこと。ModalGuard はその HWND から
// 無効化する対象を求める。
inline int ShowMessageBox(wxWindow* parent, const wxString& message,
                          const wxString& caption = "Message", long style = wxOK | wxCENTRE)
{
    ModalGuard guard(static_cast<HWND>(parent->GetHWND()));

    detail::GuardedMessageBoxDialog dlg(parent, message, caption, style, guard);
    theme::ApplyTree(dlg);

    switch (dlg.ShowModal()) {
    case wxYES:
        return wxYES;
    case wxNO:
        return wxNO;
    case wxOK:
        return wxOK;
    default:
        return wxCANCEL; // × ボタンなどで閉じた場合
    }
}

} // namespace cm
