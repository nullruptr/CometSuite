# CLAUDE.md

## 共通

冗長なコメントは避けること。
シンプルかつ簡潔に。

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

- コネクタ(DLL)側において、wx 系の wxMessageBox や、 wxDialog::ShowModal などのモーダル表示を伴う物は、正常にモーダル表示されない。
  wxWidgetsは自分のトップレベルウィンドウしか無効化しないため、AviUtl2 本体がロックされないのが原因。
- 登録した wxWidgets の HWNDは WS_CHILD にされ、AviUtl2 のaviutl2Managerコンテナの子になる。
  - ドッキング時　：get_host_app_window()と同じHWNDになる。
  - 非ドッキング時：本体がオーナーの、違うHWNDになる。

#### 対処

- モーダルを出す前に、AviUtl2 のメインスレッドの最上位ウィンドウをすべて EnableWindow(FALSE) で無効にし、閉じたら TRUE に戻す。
  本体と自分の root (GetAncestor(self, GA_ROOT)) だけでは足りない。別のプラグインが非ドッキングで表示しているウィンドウが
  有効なまま残り、クリックするとダイアログより前面に出てしまうため。
  これで、wxMessageBox, wxDialog 系は使えるようになる。
- ダイアログを閉じる (非表示にする) 前に、無効にしたウィンドウを有効に戻すこと。
  全て無効なままダイアログが消えると、Windows は次に前面にするウィンドウを AviUtl2 の外から選んでしまい
  (直前に使っていた別アプリが前面に出る)、その後の SetForegroundWindow も拒否される。
  先に有効に戻せば、ダイアログが消えたときにオーナーが通常どおりアクティブになる。
- 実装は `src/utils/modal_guard.hpp` の `ModalGuard` (ダイアログを作る前にスコープ内に置く)。
  `Release()` で有効に戻せる。`src/utils/message_box.hpp` の `cm::ShowMessageBox` は、
  `EndModal` をオーバーライドして閉じる前に `Release()` している。

### wxWidgets の High DPI (DLL)

- 登録した子ウィンドウは、`WS_CHILD`である。従って、`WM_DPICHANGED` が正常に届かない。
  wxWidgets はWinAPIの`WM_DPICHANGED`のイベントに頼ってコントロールのサイズを再計算する仕様のため、以下の不具合が生じる。

1. ドッキング切替でコントロールのみ縮む
   - 文字は正常のサイズで描画される。
   - 切替時の処理
     - AviUtl2 が、いったん DPI 96 の仮のウィンドウを経由し、ドッキング。
       このとき Windows から、`WM_DPICHANGED` (96) のmsgを受け取り、wxWidgets が 144 -> 96 の DPI 変更として扱ってコントロールを縮める。
       `WM_DPICHANGED_AFTERPARENT` は、正常なDPI値を、`GetDpiForWindow(hwnd)`経由でとれるが、AFTERPARENT では、wxWidgets が再スケールしないため、縮んだまま残る。
2. DPI の異なるモニターへ動かしても、サイズが追従しない
   - 原因として、フレームは `WS_CHILD` のため、`WM_DPICHANGED` は最上位のコンテナにしか届かない。

#### 対処

- `MainFrame::SyncDpi` で、通知に頼らず自分で DPI を調べて適用すること。
  - `WM_SIZE` / `WM_WINDOWPOSCHANGED` / `WM_DPICHANGED_AFTERPARENT` のたびに、
    `GetDpiForWindow(self)` と最後に適用した DPI (`m_dpi`) を比べ、違えば
    `MSWUpdateOnDPIChange(旧DPI, 新DPI)` を呼ぶ。
  - 仮の親にいる(`GA_ROOT` のクラスが `aviutl2Manager` でない)間は何もしない。
  - wx 内部の `m_activeDPI` は private で更新できない。従って、DPI は `m_dpi` で自前管理。
    `WM_DPICHANGED` は 二重にスケーリングされるのを防ぐため、wx 側に渡さずに捨てる。
  - `wxWindow::Layout()` は自分の sizer しか配置せず、子パネルはサイズ変更が来るまで再配置されない。
    DPI 適用後は `RelayoutAll` で子孫の `Layout()` を再帰的に呼ぶ。

  - これらの処理は、AviUtl2 の更新でクラス名が変わると、症状 1 が再発する

## wxWidgets (共通)

- 必ず、FromDIPすること。なお、コンストラクタで、FromDIP したらクラッシュするので、後から明示的にSetSize すること。
  例えば、SetSize(FromDIP(wxSize(800, 600))); 等で対応する。
- GUI は英語で記述する。日本語対応も視野に入れているので、テキストは\_("foo") と書くこと。

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
