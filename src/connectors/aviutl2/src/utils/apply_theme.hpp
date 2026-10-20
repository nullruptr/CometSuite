#pragma once
#include <wx/wx.h>
#include "utils/get_colour.hpp"
#include "wxButtonCustom.hpp"

// AviUtl2 のテーマ色 (get_colour.hpp) を wxCustomControls の部品へ適用する。
namespace cm::theme {

// ボタンの色 (通常 / ホバー / 押下 / 文字) を style.conf の ButtonBody* と Text にする。
inline void ApplyButton(wxButtonCustom& button)
{
    button.SetColors(GetColour("ButtonBody"), GetColour("Text"));
    button.SetHoverColor(GetColour("ButtonBodyHover"));
    button.SetPressedColor(GetColour("ButtonBodyPress"));
}

// root の背景色を Background、以下の子孫のうち
//   wxStaticText は文字色を Text、 wxButtonCustom は ApplyButton の色
// にする。ライブラリ側 (wxMessageBoxCustom など) が固定色で作った部品を、構築後に上書きする用途。
inline void ApplyTree(wxWindow& root)
{
    root.SetBackgroundColour(GetColour("Background"));
    for (wxWindow* child : root.GetChildren()) {
        if (auto* button = dynamic_cast<wxButtonCustom*>(child)) {
            ApplyButton(*button);
        } else if (dynamic_cast<wxStaticText*>(child)) {
            child->SetForegroundColour(GetColour("Text"));
        } else {
            ApplyTree(*child);
        }
    }
    root.Refresh();
}

} // namespace cm::theme
