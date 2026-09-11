# tbreminder -almost discontinued

JSONスケジュールに基づき、Toast通知とタスクバー次回予定表示を行うWindhawkモジュール。
スリープ復帰時の後追い発火やアイコン混雑時の自動調整など、日常使いに合わせた実装。

## 機能一覧

- スケジュールファイルの読み込み（UTF-8/ANSI、コメント対応、最大32ファイル）
- 3種類の時刻形式（毎日 `HH:MM`、一回きり `YYYY-MM-DD HH:MM`、毎時 `*:MM`）
- 3種類の通知方式（独自小窓 / Windows標準 / 両方）
- タスクバーオーバーレイ（直近N件表示、自動縮小、混雑時は一時非表示）
- ホバー表示（ツールチップで詳細表示）
- クリック操作（行クリックで該当ファイル、余白クリックで先頭ファイル）
- N分前通知、後追い発火（スリープ復帰時）
- デバッグモード（背景色変更で可視化）

## セットアップ

1. [Windhawk](https://windhawk.net/) をインストール
2. Windhawkのモジュール一覧から reminder モジュールを検索・インストール
3. 設定の「スケジュールファイル一覧」に `reminder.json` のパスを指定
4. 指定したパスにスケジュールファイルを作成

## スケジュールファイル

### 書式

- `#` または `//` で始まる行はコメント（無視される）
- 末尾カンマは許容、UTF-8 BOMあり/なしの両方に対応
- 読み込み後、コメント除去 → JSON解析 → 不正要素スキップ → 次回発火時刻でソート
- 全要素が不正な場合、前回のスケジュールを保持

### 時刻形式

```
# 毎日 HH:MM
{"time": "09:00", "message": "今日の予定を確認しましょう"}

# 一回きり YYYY-MM-DD HH:MM
{"time": "2026-12-24 18:00", "message": "申込締切のリマインド"}

# 毎時 *:MM
{"time": "*:30", "message": "目を休めましょう"}
```

### サンプル

```json
[
  {"time": "09:00", "message": "おはようございます"},
  {"time": "12:30", "message": "お昼休みです"},
  {"time": "18:00", "message": "退勤前チェック"},
  {"time": "*:30", "message": "姿勢と目を休めましょう"},
  {"time": "2026-12-24 18:00", "message": "締切リマインド"},
]
```

## 設定項目一覧

| 設定名 | 型 | デフォルト | 説明 |
|---|---|---|---|
| scheduleFiles | 文字列リスト | `reminder.json` | スケジュールファイルのパス一覧（最大32件） |
| pollIntervalSec | 整数 | 30 | ポーリング間隔（秒）。10〜3600 |
| toastDurationMs | 整数 | 5000 | Toast表示時間（ms）。1000〜60000 |
| toastBackend | 列挙 | custom | 通知方式（custom / system / both） |
| toastAnchor | 列挙 | above-taskbar | Toast表示位置（7種） |
| toastMargin | 整数 | 8 | Toast表示マージン（px）。0〜200 |
| preNotifyMin | 整数 | 0 | N分前にも通知（分）。0で無効。0〜120 |
| catchupWindowMin | 整数 | 30 | 後追い発火の上限（分）。0で無効。0〜720 |
| taskbarShowNext | 真偽値 | true | タスクバーに次回予定を表示するか |
| taskbarCount | 整数 | 2 | 表示件数（段数）。1〜5 |
| taskbarAnchor | 列挙 | tray-left | タスクバー表示位置（tray-left / taskbar-center / taskbar-left） |
| taskbarAutoFit | 真偽値 | true | タスクバーの空き幅に合わせて自動縮小 |
| taskbarCrowdedHide | 真偽値 | true | 置ける空きがない間は小窓を非表示 |
| taskbarMinWidth | 整数 | 120 | 最小幅（px）。空きがこれ未満なら非表示。80〜230 |
| taskbarFlashSec | 整数 | 8 | 非表示中に通知が来たら一瞬だけ出す秒数。3〜60 |
| taskbarOffsetX | 整数 | 0 | 小窓位置の微調整X（px）。-500〜500 |
| taskbarOffsetY | 整数 | 0 | 小窓位置の微調整Y（px）。-500〜500 |
| taskbarTooltipCount | 整数 | 5 | ホバー表示件数。0で無効。0〜50 |
| fontName | 文字列 | Segoe UI Variable | 表示フォント名（小窓・Toast・ホバー共用） |
| fontSize | 整数 | 12 | 表示フォントサイズ。8〜28 |
| fontColor | 文字列 | #FFFFFF | フォントカラー（#RRGGBB / #AARRGGBB / 色名） |
| taskbarOpacity | 整数 | 90 | タスクバー小窓の不透明度（%）。20〜100 |
| checkOnStart | 真偽値 | true | 起動直前に期限が来た一回きり予定を発火するか |
| debugBeacon | 真偽値 | false | 小窓とToastの背景を赤くして可視性を確認する |

## 動作環境

- Windows 10 / 11
- [Windhawk](https://windhawk.net/)

## FAQ

**Q: スケジュールファイルはどこに作る？**

任意の場所に作成できます。Windhawk設定の「スケジュールファイル一覧」にフルパスを指定してください。環境変数（`%USERPROFILE%` 等）も展開されます。

**Q: 通知が飛ばされるのはなぜ？**

スリープ等で_PCを長時間_offにした場合、`catchupWindowMin`（デフォルト30分）を超えた時刻の通知は後追い発火されません。この範囲を広げると、より多くの過去通知が後追いされます。

**Q: タスクバー小窓が消えるのは？**

`taskbarCrowdedHide` が有効な場合、タスクバーのアイコンが混雑して小窓を置く空きがない間は一時非表示になります。空きが戻ると再表示され、通知発火時は `taskbarFlashSec` 秒だけフラッシュ表示されます。
