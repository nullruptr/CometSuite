#pragma once
#include <winsock2.h>
#include <windows.h>
#include "aviutl2_sdk/plugin2.h"

extern EDIT_HANDLE* g_edit_handle;

// wxMessageBox / wxDialog::ShowModal を AviUtl2 上でモーダルにするためのガード。
// モーダルを出す前に必ずスコープ内に置くこと。
//
// wx は自分のトップレベルウィンドウしか無効化しないため、そのままでは AviUtl2 本体が
// ロックされない。登録した HWND (wx のフレーム) は WS_CHILD にされ、AviUtl2 のコンテナ
// (クラス aviutl2Manager) の子になる。
//   ドッキング時   : root = 本体のメインウィンドウ
//   非ドッキング時 : root = 本体がオーナーの別の最上位ウィンドウ
// そのため、GA_ROOT と本体の両方を無効化する。スコープを抜けると元に戻す。
// HWND は状態により変わるので、毎回取得し直す (保持しない)。
class ModalGuard {
public:
    explicit ModalGuard(HWND self)
        : m_root(GetAncestor(self, GA_ROOT))
        , m_host(g_edit_handle->get_host_app_window())
    {
        if (m_host && m_host != m_root)
            EnableWindow(m_host, FALSE);
        EnableWindow(m_root, FALSE);
    }

    ~ModalGuard()
    {
        EnableWindow(m_root, TRUE);
        if (m_host && m_host != m_root)
            EnableWindow(m_host, TRUE);
    }

    ModalGuard(const ModalGuard&) = delete;
    ModalGuard& operator=(const ModalGuard&) = delete;

private:
    HWND m_root;
    HWND m_host;
};
