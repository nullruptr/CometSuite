#pragma once
#include <winsock2.h>
#include <windows.h>
#include <wx/colour.h>
#include <wx/wx.h>
#include "aviutl2_sdk/config2.h"

// AviUtl2 のテーマ色 (style.conf の [Color]) を wxColour で取得する。
//
// 使い方:
//   1. main.cpp の InitializeConfig で cm::theme::SetConfig(handle) を呼ぶ。
//      (InitializeConfig は InitializePlugin / RegisterPlugin より先に呼ばれる)
//   2. ウィンドウ生成時などに cm::theme::GetColour("Text") のように呼ぶ。
//
// キー名は AviUtl2 の style.conf を参照 (Background, Text, TextDisable, ButtonBody,
// ButtonBodyHover, ButtonBodyPress, ...)。ユーザーが ProgramData\aviutl2\style.conf で上書きしている
// 場合は、その値が返る。
//
// 注意:
//   - 実行中のテーマ変更への追従は確認していない。呼んだ時点の値を返す。
//   - get_color_code は、キーが無いときも 0 (黒) を返すため、黒との区別はできない。
namespace cm::theme {

namespace detail {
inline CONFIG_HANDLE*& Config()
{
    static CONFIG_HANDLE* config = nullptr;
    return config;
}

// AviUtl2 の色コード (0x00RRGGBB) → wxColour
inline wxColour FromColorCode(int code)
{
    return wxColour(static_cast<unsigned char>((code >> 16) & 0xFF),
                    static_cast<unsigned char>((code >> 8) & 0xFF),
                    static_cast<unsigned char>(code & 0xFF));
}
} // namespace detail

// InitializeConfig で受け取ったハンドルを保存する。
inline void SetConfig(CONFIG_HANDLE* handle) { detail::Config() = handle; }

// style.conf の [Color] のキーの色を返す。
// ハンドルが未設定 (InitializeConfig 前など) のときは fallback を返す。
inline wxColour GetColour(const char* key, const wxColour& fallback = *wxBLACK)
{
    CONFIG_HANDLE* config = detail::Config();
    if (!config)
        return fallback;
    return detail::FromColorCode(config->get_color_code(config, key));
}

// 複数色を持つキー (例: FooterProgress=903838,b84848) の色の数。取れなければ 0。
inline int GetColourCount(const char* key)
{
    CONFIG_HANDLE* config = detail::Config();
    if (!config)
        return 0;
    return config->get_color_code_index(config, key, -1);
}

// 複数色を持つキーの index 番目の色を返す。
inline wxColour GetColourAt(const char* key, int index, const wxColour& fallback = *wxBLACK)
{
    CONFIG_HANDLE* config = detail::Config();
    if (!config)
        return fallback;
    return detail::FromColorCode(config->get_color_code_index(config, key, index));
}

} // namespace cm::theme
