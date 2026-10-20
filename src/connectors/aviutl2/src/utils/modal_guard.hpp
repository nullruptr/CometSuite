#pragma once
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include "aviutl2_sdk/plugin2.h"

extern EDIT_HANDLE* g_edit_handle;

// wxMessageBox / wxDialog::ShowModal を AviUtl2 上でモーダルにするためのガード。
// モーダルを出す前 (ダイアログを作る前) に、必ずスコープ内に置くこと。
//
// wx は自分のトップレベルウィンドウしか無効化しないため、そのままでは AviUtl2 本体や
// 他のウィンドウが操作できてしまう。登録した HWND (wx のフレーム) は WS_CHILD にされ、
// AviUtl2 のコンテナ (クラス aviutl2Manager) の子になる。
//   ドッキング時   : root = 本体のメインウィンドウ
//   非ドッキング時 : root = 本体がオーナーの別の最上位ウィンドウ
// 自分の root と本体だけでは足りない。別のプラグインが非ドッキングで表示しているウィンドウも
// 最上位ウィンドウで、有効なまま残るとダイアログより前面に出てしまう。
// そのため、AviUtl2 のメインスレッドの最上位ウィンドウのうち、表示中で有効なものを全て無効化し、
// スコープを抜けるときに、無効化したものだけを元に戻す。
// HWND は状態により変わるので、毎回取得し直す (保持しない)。
class ModalGuard {
public:
    explicit ModalGuard(HWND self)
        : m_return_to(GetAncestor(self, GA_ROOT))
    {
        // 本体と自分の root は、スレッドが違っても確実に無効化する
        Disable(g_edit_handle->get_host_app_window());
        Disable(m_return_to);

        EnumThreadWindows(GetCurrentThreadId(), &ModalGuard::EnumProc, reinterpret_cast<LPARAM>(this));
    }

    ~ModalGuard()
    {
        Release();
        // 念のため、呼び出し元の root を前面に戻す
        if (m_return_to && IsWindow(m_return_to))
            SetForegroundWindow(m_return_to);
    }

    // 無効化したウィンドウを元に戻す (2 回目以降は何もしない)。
    //
    // ダイアログを閉じる (非表示にする) より前に呼ぶこと。全ウィンドウが無効なままダイアログが
    // 消えると、Windows は次にアクティブにするウィンドウを、AviUtl2 の外から選んでしまい
    // (直前に使っていた別アプリが前面に出る)、その後は SetForegroundWindow も拒否される。
    // 先に有効に戻しておけば、ダイアログが消えたとき、オーナーが通常どおりアクティブになる。
    void Release()
    {
        // 無効化した逆順で戻す
        for (auto it = m_disabled.rbegin(); it != m_disabled.rend(); ++it) {
            if (IsWindow(*it))
                EnableWindow(*it, TRUE);
        }
        m_disabled.clear();
    }

    ModalGuard(const ModalGuard&) = delete;
    ModalGuard& operator=(const ModalGuard&) = delete;

private:
    // 表示中で有効なら無効化して覚える (元から無効なものは触らない)
    void Disable(HWND hwnd)
    {
        if (!hwnd || !IsWindowVisible(hwnd) || !IsWindowEnabled(hwnd))
            return;
        EnableWindow(hwnd, FALSE);
        m_disabled.push_back(hwnd);
    }

    static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lparam)
    {
        reinterpret_cast<ModalGuard*>(lparam)->Disable(hwnd);
        return TRUE;
    }

    std::vector<HWND> m_disabled;
    HWND m_return_to; // 閉じた後に前面へ戻すウィンドウ (呼び出し元の root)
};
