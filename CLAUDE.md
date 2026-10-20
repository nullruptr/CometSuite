# CLAUDE.md

## ディレクトリ構成(予定)

```
CometSuite/
├─ src/
│  ├─ common/                    # exe 側の共通部分（CPP）
│  │  └─ protocol/              # IPC 仕様（メッセージ定義、共有メモリのレイアウト）
│  ├─ core/                      # 処理コア
│  │  ├─ tracking/
│  ├─ host/                      # CometSuite.exe
│  │  ├─ app/                    # wxApp、フレーム、wxAUI のペイン構成
│  │  ├─ doc/                    # 画面仕様
│  │  └─ ipc/                    # 全コネクタを受けるサーバ
│  └─ connectors/
│     ├─ aviutl2/                # CometConnector_AviUtl2.dll（CPP）
│     └─ ymm4/                   # YMM4 プラグイン（C# / .NET）※当面は空
├─ docs/
│  ├─ ipc/                       # 通信仕様
│  └─ connectors/                # 仕様
├─ CMakeLists.txt                # ホストとコネクタのCMake
```

## wxWidgets (DLL)

- コネクタ(DLL)側において、イベントループの関係で、モーダルが機能しない。
従って、wxMessageBox などは使えないので、WinAPI 側の MessageBoxを使うこと。
どうしても、wxDialog したいなら、描画前に、EnableWindow(hwnd, FALSE)後に、描画すること。

## wxWidgets (共通)

- 必ず、FromDIPすること。なお、コンストラクタで、FromDIP したらクラッシュするので、後から明示的にSetSize すること。
例えば、SetSize(FromDIP(wxSize(800, 600))); 等で対応する。
- GUI は英語で記述する。日本語対応も視野に入れているので、テキストは_("foo") と書くこと。

## 利用技術

- wxWidgets
  - メインの GUI 描画を担う。wxAUI のwxWindow 内にBGFX を描画させる。
- BGFX
  - 主に映像表示を担う。
- ImGUI
  - 高度な編集 UI を提供する。BGFX の上に描く。
- Eigen
  - 行列計算
- nlohmann/json
  - 設定内容、ファイル保存は json で行う。あくまで予定。
- https://github.com/nullruptr/wxCustomControls
  - カスタムウィジェット描画
- OpenCV
  - モーショントラッキング関連
