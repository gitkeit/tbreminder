// ==WindhawkMod==
// @id              reminder
// @name            Reminder
// @description     JSONスケジュールファイルに基づきカスタムToast通知とタスクバー次回予定表示を行う
// @version         1.7
// @include         explorer.exe
// @include         windhawk.exe
// @architecture    x86-64
// @architecture    x86
// @compilerOptions -lgdi32 -luser32 -lshell32 -lshcore -lwinmm -lcomctl32 -luxtheme
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Reminder

指定JSONファイルのスケジュールに従い、Toast通知とタスクバー表示を行います。

## スケジュールファイル (reminder.json)

`#` / `//` 行はコメントとして無視されます。先頭にサンプルを置けます。
残りはJSON配列として解釈されます（末尾カンマ許容、BOM可）。

```
# {"time": "09:00", "message": "スタンドアップです"}  ← 毎日
# {"time": "2026-09-07 15:00", "message": "レポート提出"}  ← 一回きり
# {"time": "*:30", "message": "目を休めましょう"}  ← 毎時
[
  {"time": "09:00", "message": "今日の予定を確認しましょう"},
]
```

- time: "HH:MM"(毎日) / "YYYY-MM-DD HH:MM"(一回きり) / "*:MM"(毎時)
- message: 必須。通知・表示の本文
- 記述順は不問（読み込み時に次回発火時刻でソート）
- 不正要素はスキップ＋ログ、全体破損時は前回スケジュール保持

## 動作

- スケジュール監視＋Toast通知ワーカー: 読み込まれたプロセス内で動作（explorer.exe優先、単一化）
- 通知方式は toastBackend で切替可能（custom=独自小窓 / system=Windows標準 / both=両方）
- 小窓はタスクバー内に表示し、アイコン混雑時は自動縮小・一時非表示（通知発火時は一瞬だけ表示）
- explorer.exe: タスクバー上に小窓オーバーレイで直近N件表示（既定トレイ左）
- 小窓の行クリックで該当スケジュールファイル、余白クリックで先頭ファイルを開く
- Toastクリックで先頭スケジュールファイルを開く
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- scheduleFiles:
  - - path: 'C:\Users\kiev\_app\fujikeit\windhawk\reminder\reminder.json'
      $name: パス
  $name: スケジュールファイル一覧
  $description: パスの追加・削除が可能。行クリックで該当ファイルを開けます
- pollIntervalSec: 30
  $name: ポーリング間隔（秒）
  $description: ファイル再読込・発火チェックの最低間隔。分境界でも起床します
- toastDurationMs: 5000
  $name: Toast表示時間（ms）
- toastBackend: custom
  $name: 通知方式
  $description: custom=独自小窓 / system=Windows標準の通知 / both=両方表示
  $options:
  - custom: 独自小窓のみ
  - system: Windows標準のみ
  - both: 両方表示
- toastAnchor: above-taskbar
  $name: Toast表示位置
  $options:
  - above-taskbar: タスクバー小窓の上方
  - top-right: 右上
  - top-left: 左上
  - bottom-right: 右下
  - bottom-left: 左下
  - bottom-center: 中央下
  - center: 中央
- toastMargin: 8
  $name: Toast表示マージン（px）
  $description: 画面端またはタスクバー小窓からの距離
- preNotifyMin: 0
  $name: N分前にも通知（分）
  $description: 0で無効（1〜120）。同一内容で発火します
- catchupWindowMin: 30
  $name: 後追い発火の上限（分）
  $description: スリープ等で過ぎた通知の後追い範囲。0で無効
- taskbarShowNext: true
  $name: タスクバーに表示
- taskbarCount: 2
  $name: 表示件数（段数）
  $description: 1〜5
- taskbarAnchor: tray-left
  $name: タスクバー表示位置
  $options:
  - tray-left: トレイ左
  - taskbar-center: 中央右端
  - taskbar-left: Start右隣
- taskbarAutoFit: true
  $name: 空きに合わせて自動縮小
  $description: タスクバーの空き幅に収まるよう小窓幅を縮める
- taskbarCrowdedHide: true
  $name: 混雑時は一時非表示
  $description: 置ける空きがない間は小窓を隠し、空きが戻ったら再表示
- taskbarMinWidth: 120
  $name: 最小幅（px）
  $description: 空きがこれ未満なら非表示。80〜230
- taskbarFlashSec: 8
  $name: 通知時の表示時間（秒）
  $description: 非表示中に通知が来たら一瞬だけ出す秒数。3〜60
- taskbarOffsetX: 0
  $name: 小窓位置の微調整X（px）
  $description: +で右/−で左（−500〜500）
- taskbarOffsetY: 0
  $name: 小窓位置の微調整Y（px）
  $description: +で下/−で上（−500〜500）
- taskbarTooltipCount: 5
  $name: ホバー表示件数
  $description: 小窓ホバーで表示する件数。0で無効（0〜50）
- fontName: 'Segoe UI Variable'
  $name: 表示フォント（共通）
  $description: 小窓・Toast・ホバー表示で共用。空欄でシステム既定
- fontSize: 12
  $name: 表示フォントサイズ（共通）
- fontColor: '#FFFFFF'
  $name: 表示フォントカラー（共通）
  $description: "'#RRGGBB' / '#AARRGGBB' / 色名(White, Black, Red...)"
- taskbarOpacity: 90
  $name: タスクバー小窓の不透明度（%）
- checkOnStart: true
  $name: 起動時の猶予発火
  $description: 起動直前に期限が来た一回きり予定を発火するか
- debugBeacon: false
  $name: デバッグ用ビーコン表示
  $description: 小窓とToastの背景を赤くして可視性を確認する
*/
// ==/WindhawkModSettings==

#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <tlhelp32.h>
#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdio>
#include <cwchar>
#include <cctype>
#include <cstdarg>
#include <climits>
#include <cstring>
#include <process.h>

#ifndef WH_MOD_ID
#define WH_MOD_ID L"reminder"
#endif

// ---------- Settings ----------
struct Settings {
    std::vector<std::wstring> scheduleFiles;
    int pollIntervalSec = 30;
    int toastDurationMs = 5000;
    std::wstring toastBackend = L"custom";
    std::wstring toastAnchor = L"above-taskbar";
    int toastMargin = 8;
    int preNotifyMin = 0;
    int catchupWindowMin = 30;
    bool taskbarShowNext = true;
    int taskbarCount = 2;
    std::wstring taskbarAnchor = L"tray-left";
    bool taskbarAutoFit = true;
    bool taskbarCrowdedHide = true;
    int taskbarMinWidth = 120;
    int taskbarFlashSec = 8;
    int taskbarOffsetX = 0;
    int taskbarOffsetY = 0;
    int taskbarTooltipCount = 5;
    std::wstring fontName = L"Segoe UI Variable";
    int fontSize = 12;
    std::wstring fontColor = L"#FFFFFF";
    int taskbarOpacity = 90;
    bool checkOnStart = true;
    bool debugBeacon = false;
};
static Settings g_settings;
static CRITICAL_SECTION g_settingsCs;
static CRITICAL_SECTION g_reloadCs;
static bool g_settingsCsInit = false;

static std::wstring GetStrSetting(LPCWSTR key, LPCWSTR def) {
    const WCHAR* v = Wh_GetStringSetting(key);
    std::wstring r = v ? v : (def ? def : L"");
    if (v) Wh_FreeStringSetting(v);
    return r;
}
static int GetIntSetting(LPCWSTR key, int def) {
    int v = Wh_GetIntSetting(key);
    // Absent/invalid settings read as 0: fall back to the default so keys with
    // non-zero defaults behave correctly even before YAML defaults are available.
    if (v == 0) v = def;
    return v;
}

static std::wstring ExpandEnv(const std::wstring& s) {
    if (s.empty()) return s;
    DWORD n = ExpandEnvironmentStringsW(s.c_str(), nullptr, 0);
    if (!n) return s;
    std::wstring out(n, L'\0');
    DWORD m = ExpandEnvironmentStringsW(s.c_str(), out.data(), n);
    if (!m) return s;
    out.resize(wcslen(out.c_str()));
    return out;
}
static std::wstring Trim(const std::wstring& s) {
    size_t a = 0, b = s.size();
    while (a < b && iswspace(s[a])) a++;
    while (b > a && iswspace(s[b - 1])) b--;
    return s.substr(a, b - a);
}

// Safe substring that does not split surrogate pairs (for log truncation etc.)
static std::wstring SafeSubstr(const std::wstring& s, size_t maxChars) {
    if (s.size() <= maxChars) return s;
    size_t end = maxChars;
    // Don't split a surrogate pair: if the cut lands right after a high
    // surrogate, its low half lives at index `end` (valid: end < s.size()).
    if (end > 0 && s[end - 1] >= 0xD800 && s[end - 1] <= 0xDBFF)
        end++;
    return s.substr(0, end);
}

static std::wstring CleanPath(const std::wstring& s) {
    std::wstring p = ExpandEnv(Trim(s));
    if (p.size() >= 2 && p.front() == L'"' && p.back() == L'"')
        p = p.substr(1, p.size() - 2);
    return Trim(p);
}

static void LoadSettings() {
    Settings s;
    // scheduleFiles[i].path (list setting, up to 32 entries)
    for (int i = 0; i < 32; i++) {
        wchar_t key[64];
        swprintf_s(key, L"scheduleFiles[%d].path", i);
        std::wstring p = CleanPath(GetStrSetting(key, L""));
        if (p.empty()) break;
        s.scheduleFiles.push_back(std::move(p));
    }
    if (s.scheduleFiles.empty()) {
        s.scheduleFiles.push_back(
            L"C:\\Users\\kiev\\_app\\fujikeit\\windhawk\\reminder\\reminder.json");
    }
    s.pollIntervalSec = GetIntSetting(L"pollIntervalSec", 30);
    s.toastDurationMs = GetIntSetting(L"toastDurationMs", 5000);
    s.toastBackend = Trim(GetStrSetting(L"toastBackend", L"custom"));
    for (auto& c : s.toastBackend) c = towlower(c);
    if (s.toastBackend != L"custom" && s.toastBackend != L"system" && s.toastBackend != L"both")
        s.toastBackend = L"custom";
    s.toastAnchor = Trim(GetStrSetting(L"toastAnchor", L"above-taskbar"));
    s.toastMargin = GetIntSetting(L"toastMargin", 8);
    s.preNotifyMin = GetIntSetting(L"preNotifyMin", 0);
    s.catchupWindowMin = GetIntSetting(L"catchupWindowMin", 30);
    s.taskbarShowNext = GetIntSetting(L"taskbarShowNext", 1) != 0;
    s.taskbarCount = GetIntSetting(L"taskbarCount", 2);
    s.taskbarAnchor = Trim(GetStrSetting(L"taskbarAnchor", L"tray-left"));
    s.taskbarAutoFit = GetIntSetting(L"taskbarAutoFit", 1) != 0;
    s.taskbarCrowdedHide = GetIntSetting(L"taskbarCrowdedHide", 1) != 0;
    s.taskbarMinWidth = GetIntSetting(L"taskbarMinWidth", 120);
    s.taskbarFlashSec = GetIntSetting(L"taskbarFlashSec", 8);
    s.taskbarOffsetX = GetIntSetting(L"taskbarOffsetX", 0);
    s.taskbarOffsetY = GetIntSetting(L"taskbarOffsetY", 0);
    s.taskbarTooltipCount = GetIntSetting(L"taskbarTooltipCount", 5);
    s.fontName = Trim(GetStrSetting(L"fontName", L"Segoe UI Variable"));
    s.fontSize = GetIntSetting(L"fontSize", 12);
    s.fontColor = Trim(GetStrSetting(L"fontColor", L"#FFFFFF"));
    s.taskbarOpacity = GetIntSetting(L"taskbarOpacity", 90);
    s.checkOnStart = GetIntSetting(L"checkOnStart", 1) != 0;
    s.debugBeacon = GetIntSetting(L"debugBeacon", 0) != 0;
    if (s.toastMargin < 0) s.toastMargin = 0;
    if (s.toastMargin > 200) s.toastMargin = 200;
    if (s.preNotifyMin < 0) s.preNotifyMin = 0;
    if (s.preNotifyMin > 120) s.preNotifyMin = 120;
    if (s.catchupWindowMin < 0) s.catchupWindowMin = 0;
    if (s.catchupWindowMin > 720) s.catchupWindowMin = 720;
    if (s.taskbarOffsetX < -500) s.taskbarOffsetX = -500;
    if (s.taskbarOffsetX > 500) s.taskbarOffsetX = 500;
    if (s.taskbarOffsetY < -500) s.taskbarOffsetY = -500;
    if (s.taskbarOffsetY > 500) s.taskbarOffsetY = 500;
    if (s.taskbarTooltipCount < 0) s.taskbarTooltipCount = 0;
    if (s.taskbarTooltipCount > 50) s.taskbarTooltipCount = 50;
    if (s.pollIntervalSec < 10) s.pollIntervalSec = 10;
    if (s.pollIntervalSec > 3600) s.pollIntervalSec = 3600;
    if (s.taskbarCount < 1) s.taskbarCount = 1;
    if (s.taskbarCount > 5) s.taskbarCount = 5;
    if (s.taskbarMinWidth < 80) s.taskbarMinWidth = 80;
    if (s.taskbarMinWidth > 230) s.taskbarMinWidth = 230;
    if (s.taskbarFlashSec < 3) s.taskbarFlashSec = 3;
    if (s.taskbarFlashSec > 60) s.taskbarFlashSec = 60;
    if (s.fontSize < 8) s.fontSize = 8;
    if (s.fontSize > 28) s.fontSize = 28;
    if (s.taskbarOpacity < 20) s.taskbarOpacity = 20;
    if (s.taskbarOpacity > 100) s.taskbarOpacity = 100;
    if (s.toastDurationMs < 1000) s.toastDurationMs = 1000;
    if (s.toastDurationMs > 60000) s.toastDurationMs = 60000;
    EnterCriticalSection(&g_settingsCs);
    g_settings = std::move(s);
    LeaveCriticalSection(&g_settingsCs);
}

// First schedule file (for click-to-open actions)
static std::wstring FirstFile() {
    EnterCriticalSection(&g_settingsCs);
    std::wstring f = g_settings.scheduleFiles.empty() ? std::wstring() : g_settings.scheduleFiles[0];
    LeaveCriticalSection(&g_settingsCs);
    return f;
}

// ---------- Color ----------
static bool ParseColor(const std::wstring& in, COLORREF& out) {
    std::wstring s = Trim(in);
    std::wstring low = s;
    for (auto& c : low) c = towlower(c);
    if (low == L"white") { out = RGB(255,255,255); return true; }
    if (low == L"black") { out = RGB(0,0,0); return true; }
    if (low == L"red") { out = RGB(255,0,0); return true; }
    if (low == L"green") { out = RGB(0,255,0); return true; }
    if (low == L"blue") { out = RGB(0,120,215); return true; }
    if (low == L"yellow") { out = RGB(255,255,0); return true; }
    if (low == L"gray" || low == L"grey") { out = RGB(128,128,128); return true; }
    if (!s.empty() && s[0] == L'#') {
        std::wstring h = s.substr(1);
        unsigned long v = 0;
        if (h.size() == 6 || h.size() == 8) {
            for (wchar_t c : h) {
                v <<= 4;
                if (c >= L'0' && c <= L'9') v |= (c - L'0');
                else if (c >= L'a' && c <= L'f') v |= (c - L'a' + 10);
                else if (c >= L'A' && c <= L'F') v |= (c - L'A' + 10);
                else return false;
            }
            unsigned r, g, b;
            if (h.size() == 6) { r = (v >> 16) & 0xFF; g = (v >> 8) & 0xFF; b = v & 0xFF; }
            else { r = (v >> 16) & 0xFF; g = (v >> 8) & 0xFF; b = v & 0xFF; } // ignore AA
            out = RGB(r, g, b);
            return true;
        }
    }
    return false;
}

// ---------- Logging via Windhawk ----------
void LogMsg(const wchar_t* fmt, ...);

// ---------- Schedule ----------
enum class TimeKind { Daily, Once, Hourly, Invalid };
struct Entry {
    TimeKind kind = TimeKind::Invalid;
    int hour = 0, minute = 0;           // Daily/Hourly
    int year = 0, month = 0, day = 0;   // Once
    std::wstring message;
    std::wstring rawTime;
    std::wstring sourceFile;
};
// Entries are kept behind a shared_ptr so readers can snapshot the immutable
// vector cheaply (a refcount bump) instead of copying every Entry on each 2s
// taskbar paint. Writes replace the whole pointer under g_entriesCs.
static std::shared_ptr<std::vector<Entry>> g_entries;
static CRITICAL_SECTION g_entriesCs;
static std::map<std::wstring, FILETIME> g_fileWrites;            // per-file mtime
static std::map<std::wstring, std::vector<Entry>> g_perFile;     // per-file entries
static std::map<std::wstring, bool> g_missingState;              // per-file presence (log only on change)

static bool AllDigits(const std::wstring& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](wchar_t c){ return c >= L'0' && c <= L'9'; });
}

static bool ParseTimeStr(const std::wstring& t, Entry& e, std::wstring* reason) {
    e.rawTime = t;
    auto fail = [&](const wchar_t* r){ if (reason) *reason = r; return false; };
    // "*:MM"
    if (t.size() >= 3 && t[0] == L'*' && t[1] == L':') {
        std::wstring mm = t.substr(2);
        if ((mm.size() != 1 && mm.size() != 2) || !AllDigits(mm))
            return fail(L"hourly minutes must be 1-2 digits");
        int m = _wtoi(mm.c_str());
        if (m < 0 || m > 59) return fail(L"hourly minutes out of range");
        e.kind = TimeKind::Hourly; e.minute = m;
        return true;
    }
    // "YYYY-MM-DD HH:MM" (strict: full consumption)
    {
        int Y=0,M=0,D=0,H=0,Mi=0,n=0;
        if (swscanf_s(t.c_str(), L"%d-%d-%d %d:%d%n", &Y,&M,&D,&H,&Mi,&n) == 5 && n == (int)t.size()) {
            if (M<1||M>12||D<1||D>31||H<0||H>23||Mi<0||Mi>59) return fail(L"date/time out of range");
            e.kind = TimeKind::Once; e.year=Y; e.month=M; e.day=D; e.hour=H; e.minute=Mi;
            return true;
        }
    }
    // "HH:MM" (strict: H:MM digits only)
    {
        size_t c = t.find(L':');
        if (c != std::wstring::npos && t.find(L':', c + 1) == std::wstring::npos &&
            t.find(L'-') == std::wstring::npos && t.find(L' ') == std::wstring::npos &&
            t.find(L'*') == std::wstring::npos) {
            std::wstring hs = t.substr(0, c), ms = t.substr(c + 1);
            if ((hs.size() == 1 || hs.size() == 2) && ms.size() == 2 &&
                AllDigits(hs) && AllDigits(ms)) {
                int H = _wtoi(hs.c_str()), Mi = _wtoi(ms.c_str());
                if (H<0||H>23||Mi<0||Mi>59) return fail(L"time out of range");
                e.kind = TimeKind::Daily; e.hour=H; e.minute=Mi;
                return true;
            }
        }
    }
    return fail(L"unknown time format (use HH:MM, YYYY-MM-DD HH:MM, or *:MM)");
}

// Minimal JSON: strip # and // comment lines, then parse array of {"time","message"}
// ("title" is obsolete and ignored for compatibility)
static std::wstring ReadFileUtf8OrAnsi(const std::wstring& path) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return L"";
    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(h, &sz) || sz.QuadPart <= 0 || sz.QuadPart > 10 * 1024 * 1024) {
        CloseHandle(h);
        return L"";
    }
    std::string buf;
    buf.resize((size_t)sz.QuadPart);
    DWORD read = 0;
    if (!ReadFile(h, buf.data(), (DWORD)sz.QuadPart, &read, nullptr)) { CloseHandle(h); return L""; }
    CloseHandle(h);
    buf.resize(read);
    // UTF-8 BOM skip
    size_t off = 0;
    if (buf.size() >= 3 && (unsigned char)buf[0]==0xEF && (unsigned char)buf[1]==0xBB && (unsigned char)buf[2]==0xBF) off = 3;
    // Try UTF-8 -> UTF-16; fallback ANSI
    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, buf.data()+off, (int)(buf.size()-off), nullptr, 0);
    if (wlen > 0) {
        std::wstring w(wlen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, buf.data()+off, (int)(buf.size()-off), w.data(), wlen);
        return w;
    }
    wlen = MultiByteToWideChar(CP_ACP, 0, buf.data()+off, (int)(buf.size()-off), nullptr, 0);
    if (wlen <= 0) return L"";
    std::wstring w(wlen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, buf.data()+off, (int)(buf.size()-off), w.data(), wlen);
    return w;
}

static std::wstring UnescapeJson(const std::wstring& s) {
    std::wstring o;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == L'\\' && i + 1 < s.size()) {
            wchar_t n = s[i+1];
            if (n == L'n') { o += L'\n'; i++; }
            else if (n == L't') { o += L'\t'; i++; }
            else if (n == L'r') { o += L'\r'; i++; }
            else if (n == L'"' || n == L'\\' || n == L'/') { o += n; i++; }
            else if (n == L'u') {
                bool ok = (i + 5 < s.size());
                unsigned v = 0;
                if (ok) {
                    for (int k = 0; k < 4 && ok; k++) {
                        wchar_t c = s[i + 2 + k];
                        v <<= 4;
                        if (c >= L'0' && c <= L'9') v |= (c - L'0');
                        else if (c >= L'a' && c <= L'f') v |= (c - L'a' + 10);
                        else if (c >= L'A' && c <= L'F') v |= (c - L'A' + 10);
                        else ok = false;
                    }
                }
                if (ok) { o += (wchar_t)v; i += 5; }
                else { o += s[i]; } // invalid \u: keep the backslash literally
            } else { o += s[i]; }
        } else o += s[i];
    }
    return o;
}

// Parse one JSON string value starting at pos (expects '"'), returns value and end pos (after closing quote)
static bool ParseJsonString(const std::wstring& s, size_t& pos, std::wstring& out) {
    if (pos >= s.size() || s[pos] != L'"') return false;
    pos++;
    std::wstring raw;
    while (pos < s.size()) {
        wchar_t c = s[pos];
        if (c == L'\\') { raw += c; if (pos+1 < s.size()) { raw += s[pos+1]; pos += 2; } else pos++; }
        else if (c == L'"') { pos++; out = UnescapeJson(raw); return true; }
        else { raw += c; pos++; }
    }
    return false;
}
static void SkipWs(const std::wstring& s, size_t& pos) {
    while (pos < s.size() && iswspace(s[pos])) pos++;
}

static std::vector<Entry> ParseScheduleText(const std::wstring& raw, std::vector<std::wstring>& warnings) {
    // 1. strip comment lines
    std::wstring filtered;
    size_t p = 0;
    while (p <= raw.size()) {
        size_t e = raw.find(L'\n', p);
        std::wstring line = (e == std::wstring::npos) ? raw.substr(p) : raw.substr(p, e - p);
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        std::wstring t = Trim(line);
        if (!(t.empty() || t[0] == L'#' || (t.size() >= 2 && t[0] == L'/' && t[1] == L'/')))
            filtered += line + L'\n';
        if (e == std::wstring::npos) break;
        p = e + 1;
    }
    std::vector<Entry> entries;
    size_t pos = filtered.find(L'[');
    if (pos == std::wstring::npos) { warnings.push_back(L"JSON array '[' not found"); return entries; }
    pos++;
    int idx = 0;
    while (true) {
        SkipWs(filtered, pos);
        // allow trailing comma
        if (pos < filtered.size() && filtered[pos] == L',') { pos++; continue; }
        if (pos >= filtered.size()) { warnings.push_back(L"unexpected end"); break; }
        if (filtered[pos] == L']') break;
        if (filtered[pos] != L'{') { warnings.push_back(L"expected '{' for element"); break; }
        pos++;
        std::wstring time, msg;
        while (true) {
            SkipWs(filtered, pos);
            if (pos < filtered.size() && filtered[pos] == L'}') { pos++; break; }
            std::wstring key;
            if (!ParseJsonString(filtered, pos, key)) { warnings.push_back(L"bad key"); break; }
            SkipWs(filtered, pos);
            if (pos < filtered.size() && filtered[pos] == L':') pos++;
            SkipWs(filtered, pos);
            std::wstring val;
            if (pos < filtered.size() && filtered[pos] == L'"') {
                if (!ParseJsonString(filtered, pos, val)) { warnings.push_back(L"bad value"); break; }
            } else {
                // non-string: read until , or }
                size_t st = pos;
                while (pos < filtered.size() && filtered[pos] != L',' && filtered[pos] != L'}') pos++;
                val = Trim(filtered.substr(st, pos - st));
            }
            std::wstring kl = key;
            for (auto& c : kl) c = towlower(c);
            if (kl == L"time") time = val;
            else if (kl == L"message") msg = val;
            else if (kl == L"title") { /* obsolete: ignored for compatibility */ }
            SkipWs(filtered, pos);
            if (pos < filtered.size() && filtered[pos] == L',') { pos++; continue; }
            if (pos < filtered.size() && filtered[pos] == L'}') continue; // loop will break
        }
        idx++;
        Entry e;
        time = Trim(time); msg = Trim(msg);
        if (time.empty()) { warnings.push_back(L"element " + std::to_wstring(idx) + L": missing time"); continue; }
        if (msg.empty()) { warnings.push_back(L"element " + std::to_wstring(idx) + L": missing message"); continue; }
        std::wstring reason;
        if (!ParseTimeStr(time, e, &reason)) { warnings.push_back(L"element " + std::to_wstring(idx) + L": bad time '" + time + L"' (" + reason + L")"); continue; }
        e.message = msg;
        entries.push_back(std::move(e));
    }
    return entries;
}

static __int64 ToMinKey(int y,int mo,int d,int h,int mi) {
    return ((((__int64)y*12+mo)*31+d)*24+h)*60+mi;
}
static __int64 NextFireKey(const Entry& e, const SYSTEMTIME& now) {
    if (e.kind == TimeKind::Daily) {
        __int64 today = ToMinKey(now.wYear,now.wMonth,now.wDay,e.hour,e.minute);
        __int64 cur = ToMinKey(now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute);
        if (today < cur) {
            // tomorrow (approx: +1 day, ignore month overflow for sort purposes by adding 24*60)
            return today + 24*60;
        }
        return today;
    } else if (e.kind == TimeKind::Hourly) {
        __int64 thisH = ToMinKey(now.wYear,now.wMonth,now.wDay,now.wHour,e.minute);
        __int64 cur = ToMinKey(now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute);
        if (thisH < cur) return thisH + 60;
        return thisH;
    } else if (e.kind == TimeKind::Once) {
        return ToMinKey(e.year,e.month,e.day,e.hour,e.minute);
    }
    return 0;
}
static std::wstring FiredKey(const Entry& e) {
    // Normalized from the parsed components so formatting variants ("*:5" vs
    // "*:05", etc.) map to the same key for dedup and fired tracking.
    wchar_t b[64];
    if (e.kind == TimeKind::Daily) swprintf_s(b, L"%02d:%02d|%s", e.hour, e.minute, e.message.c_str());
    else if (e.kind == TimeKind::Hourly) swprintf_s(b, L"*:%02d|%s", e.minute, e.message.c_str());
    else swprintf_s(b, L"%04d-%02d-%02d %02d:%02d|%s", e.year, e.month, e.day, e.hour, e.minute, e.message.c_str());
    return b;
}
static std::wstring WhenLabel(const Entry& e) {
    wchar_t b[64];
    if (e.kind == TimeKind::Daily) swprintf_s(b, L"%02d:%02d", e.hour, e.minute);
    else if (e.kind == TimeKind::Hourly) swprintf_s(b, L"*:%02d", e.minute);
    else swprintf_s(b, L"%04d-%02d-%02d %02d:%02d", e.year, e.month, e.day, e.hour, e.minute);
    return b;
}

// Tooltip label: hourly "*:MM" padded to 5 chars so rows roughly align
static std::wstring TipWhenLabel(const Entry& e) {
    if (e.kind == TimeKind::Hourly) return L" " + WhenLabel(e);
    return WhenLabel(e);
}

// Reload files whose mtime changed; merges all files. Returns true if reloaded.
// A file that fails to parse keeps its previous entries (loud log).
// If every file is missing/broken and we already have entries, old ones are kept.
static bool MaybeReload(bool force, bool& fileMissing) {
    EnterCriticalSection(&g_reloadCs);
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    fileMissing = false;
    bool keptPrevious = false; // a BROKEN file kept its old entries
    bool anyChange = false;
    // Drop state for files removed from the list
    for (auto it = g_perFile.begin(); it != g_perFile.end(); ) {
        if (std::find(s.scheduleFiles.begin(), s.scheduleFiles.end(), it->first) == s.scheduleFiles.end()) {
            g_fileWrites.erase(it->first);
            g_missingState.erase(it->first);
            it = g_perFile.erase(it);
            anyChange = true;
        } else ++it;
    }
    for (auto& path : s.scheduleFiles) {
        WIN32_FILE_ATTRIBUTE_DATA fad{};
        if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fad)) {
            fileMissing = true;
            if (!g_missingState[path]) {
                g_missingState[path] = true;
                LogMsg(L"Reminder: schedule file missing: %s", path.c_str());
            }
            continue;
        }
        if (g_missingState[path]) {
            g_missingState[path] = false;
            LogMsg(L"Reminder: schedule file recovered: %s", path.c_str());
        }
        auto wit = g_fileWrites.find(path);
        if (!force && wit != g_fileWrites.end() &&
            wit->second.dwLowDateTime == fad.ftLastWriteTime.dwLowDateTime &&
            wit->second.dwHighDateTime == fad.ftLastWriteTime.dwHighDateTime)
            continue;
        std::wstring text = ReadFileUtf8OrAnsi(path);
        if (text.empty()) {
            g_fileWrites[path] = fad.ftLastWriteTime;
            g_perFile[path].clear();
            anyChange = true;
            continue;
        }
        std::vector<std::wstring> warns;
        auto entries = ParseScheduleText(text, warns);
        for (auto& w : warns)
            LogMsg(L"Reminder parse warning [%s]: %s", path.c_str(), w.c_str());
        for (auto& e : entries) e.sourceFile = path;
        if (entries.empty() && !warns.empty()) {
            // Broken (had content but nothing valid): keep previous entries for this file
            LogMsg(L"Reminder: BROKEN schedule file, keeping previous entries: %s", path.c_str());
            g_fileWrites[path] = fad.ftLastWriteTime;
            keptPrevious = true;
            continue;
        }
        g_fileWrites[path] = fad.ftLastWriteTime;
        g_perFile[path] = std::move(entries);
        anyChange = true;
    }
    if (!anyChange && !force) { LeaveCriticalSection(&g_reloadCs); return false; }
    std::vector<Entry> merged;
    // Merge in settings order so duplicate elimination prefers earlier files.
    for (auto& path : s.scheduleFiles) {
        auto it = g_perFile.find(path);
        if (it != g_perFile.end())
            merged.insert(merged.end(), it->second.begin(), it->second.end());
    }
    // Deduplicate identical (time+message) entries, keeping the first
    {
        std::map<std::wstring, bool> seen;
        std::vector<Entry> uniq;
        for (auto& e : merged) {
            std::wstring k = FiredKey(e);
            if (seen[k]) continue;
            seen[k] = true;
            uniq.push_back(std::move(e));
        }
        if (uniq.size() != merged.size())
            LogMsg(L"Reminder: removed %zu duplicate entries", merged.size() - uniq.size());
        merged = std::move(uniq);
    }
    EnterCriticalSection(&g_entriesCs);
    bool hadOld = g_entries && !g_entries->empty();
    if (merged.empty() && hadOld && (fileMissing || keptPrevious)) {
        LeaveCriticalSection(&g_entriesCs);
        LogMsg(L"Reminder: all schedule files empty/missing/broken, keeping previous entries");
        LeaveCriticalSection(&g_reloadCs);
        return true;
    }
    SYSTEMTIME now{}; GetLocalTime(&now);
    std::sort(merged.begin(), merged.end(), [&](const Entry& a, const Entry& b){
        return NextFireKey(a, now) < NextFireKey(b, now);
    });
    size_t n = merged.size();
    g_entries = std::make_shared<std::vector<Entry>>(std::move(merged));
    LeaveCriticalSection(&g_entriesCs);
    LogMsg(L"Reminder: reloaded %zu entries from %zu files", n, s.scheduleFiles.size());
    LeaveCriticalSection(&g_reloadCs);
    return true;
}

// ---------- Shared: compute upcoming N ----------
static std::vector<Entry> Upcoming(int n) {
    SYSTEMTIME now{}; GetLocalTime(&now);
    __int64 cur = ToMinKey(now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute);
    std::vector<std::pair<__int64,Entry>> v;
    std::shared_ptr<std::vector<Entry>> sp;
    EnterCriticalSection(&g_entriesCs);
    sp = g_entries;
    LeaveCriticalSection(&g_entriesCs);
    if (!sp) return {};
    for (const Entry& e : *sp) {
        __int64 k = NextFireKey(e, now);
        if (e.kind == TimeKind::Once && k < cur) continue; // past one-shot
        v.emplace_back(k, e);
    }
    std::sort(v.begin(), v.end(), [](auto& a, auto& b){ return a.first < b.first; });
    std::vector<Entry> o;
    for (size_t i = 0; i < v.size() && (int)o.size() < n; i++) o.push_back(v[i].second);
    return o;
}

// ---------- Logging via Windhawk (impl) ----------
void LogMsg(const wchar_t* fmt, ...) {
    wchar_t buf[1024];
    va_list ap; va_start(ap, fmt);
    _vsnwprintf_s(buf, 1024, _TRUNCATE, fmt, ap);
    va_end(ap);
    Wh_Log(L"%s", buf);
}

// ---------- Async file opener ----------
// ShellExecute* must never run on a window thread: it pumps messages internally,
// so rapid clicks nest the WndProc (timer/queue state churn) and crash the host.
// Click handlers only enqueue; this dedicated thread performs the open.
static CRITICAL_SECTION g_openCs;
static HANDLE g_openEvent = nullptr; // auto-reset
static HANDLE g_openThread = nullptr;
static std::deque<std::wstring> g_openQueue;
static bool g_openStop = false;
static bool g_openShutdown = false; // set at unload; no lazy restart afterwards
static volatile LONG g_openGen = 0; // generation: bumped on stop/reload so an orphan
                                    // worker self-terminates instead of spinning
static unsigned __stdcall OpenThreadProc(LPVOID p) {
    long gen = (long)(LONG_PTR)p;
    for (;;) {
        HANDLE ev = g_openEvent;
        if (!ev) return 0; // event torn down (unload race): exit cleanly
        WaitForSingleObject(ev, INFINITE);
        for (;;) {
            std::wstring target;
            EnterCriticalSection(&g_openCs);
            bool stale = (g_openGen != gen);
            if (!stale && !g_openQueue.empty()) { target = std::move(g_openQueue.front()); g_openQueue.pop_front(); }
            LeaveCriticalSection(&g_openCs);
            if (stale) return 0;
            if (target.empty()) break;
            SHELLEXECUTEINFOW sei{ sizeof(sei) };
            sei.fMask = SEE_MASK_ASYNCOK | SEE_MASK_FLAG_NO_UI;
            sei.lpVerb = L"open";
            sei.lpFile = target.c_str();
            sei.nShow = SW_SHOWNORMAL;
            LogMsg(L"Reminder open: %s", target.c_str());
            BOOL ok = ShellExecuteExW(&sei);
            LogMsg(L"Reminder open result=%d err=%lu", (int)ok, ok ? 0UL : GetLastError());
        }
        EnterCriticalSection(&g_openCs);
        bool stop = g_openStop || (g_openGen != gen);
        LeaveCriticalSection(&g_openCs);
        if (stop) break;
    }
    return 0;
}
static void RequestOpen(const std::wstring& target) {
    if (target.empty()) return;
    EnterCriticalSection(&g_openCs);
    if (g_openShutdown) { LeaveCriticalSection(&g_openCs); return; }
    if (!g_openThread) {
        g_openEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (g_openEvent) {
            HANDLE th = (HANDLE)_beginthreadex(nullptr, 0, OpenThreadProc,
                (void*)(LONG_PTR)(long)g_openGen, 0, nullptr);
            if (th) g_openThread = th;
            else { CloseHandle(g_openEvent); g_openEvent = nullptr; }
        }
        if (!g_openThread)
            LogMsg(L"Reminder open: thread start failed err=%lu", GetLastError());
    }
    if (g_openQueue.size() < 8) {
        g_openQueue.push_back(target);
        LogMsg(L"Reminder open queued: %s", target.c_str());
    } else {
        LogMsg(L"Reminder open: queue full, dropping");
    }
    HANDLE ev = g_openEvent;
    LeaveCriticalSection(&g_openCs);
    if (ev) SetEvent(ev);
}
static void StopOpener() {
    EnterCriticalSection(&g_openCs);
    g_openShutdown = true;
    g_openStop = true;
    InterlockedIncrement(&g_openGen); // orphan worker self-terminates
    LeaveCriticalSection(&g_openCs);
    if (g_openEvent) SetEvent(g_openEvent);
    if (g_openThread) { WaitForSingleObject(g_openThread, 2000); CloseHandle(g_openThread); g_openThread = nullptr; }
    if (g_openEvent) { CloseHandle(g_openEvent); g_openEvent = nullptr; }
    EnterCriticalSection(&g_openCs);
    g_openQueue.clear();
    LeaveCriticalSection(&g_openCs);
}

// ---------- Custom Toast window (used in windhawk tool process) ----------
static HWND g_toastHwnd = nullptr;
static std::wstring g_toastTitle, g_toastBody, g_toastSourceFile;
// Pending notifications: (title, body, sourceFile)
static std::deque<std::tuple<std::wstring, std::wstring, std::wstring>> g_toastQueue;
static const size_t kToastQueueMax = 10;
static UINT_PTR g_toastTimer = 0;
static bool g_toastBusy = false;
static int g_toastPaints = 0;
static int g_barPaints = 0;
#define WM_APP_TOAST (WM_APP + 101)
#define WM_APP_REPOS (WM_APP + 102)
#define WM_APP_TRAYICON (WM_APP + 103)
#define WM_APP_BAR_FLASH (WM_APP + 203) // bar flash-show (defined early for NotifyBarFlash)
// System-notification backend (standard Windows balloon via tray icon):
// queued like the custom toast, shown one at a time, click opens the file.
static std::deque<std::tuple<std::wstring, std::wstring, std::wstring>> g_sysQueue;
static bool g_sysBusy = false;
static UINT_PTR g_sysTimer = 0; // timer id 2 on the toast window
static std::wstring g_sysSourceFile;
static bool g_trayAdded = false;
static const UINT kTrayId = 0x524D; // 'RM'
static const int kToastW = 360;
static int g_toastH = 100;
static RECT g_trTitle{ 16, 10, 344, 36 };
static RECT g_trBody{ 16, 43, 344, 86 };
static int g_sepY = 36;
static RECT g_lastToastPos{ -1, -1, -1, -1 };

// Positions the toast window per toastAnchor setting (per-monitor work area)
static void PositionToastWindow(HWND h) {
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    // above-taskbar follows the bar's monitor; otherwise follow the toast's own monitor.
    HWND barForAnchor = nullptr;
    if (s.toastAnchor == L"above-taskbar") {
        HWND bar = FindWindowW(L"ReminderBarCls", L"ReminderBar");
        RECT br{};
        if (bar && GetWindowRect(bar, &br) && br.right > br.left && br.bottom > br.top)
            barForAnchor = bar;
    }
    HMONITOR mon = MonitorFromWindow(barForAnchor ? barForAnchor : h, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{ sizeof(mi) };
    RECT wa{};
    if (mon && GetMonitorInfoW(mon, &mi)) {
        wa = mi.rcWork;
    } else {
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0);
    }
    if (wa.right <= wa.left || wa.bottom <= wa.top) {
        wa.left = 0; wa.top = 0;
        wa.right = GetSystemMetrics(SM_CXSCREEN);
        wa.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    int m = s.toastMargin;
    int x = wa.right - kToastW - m, y = wa.top + m; // top-right
    const std::wstring& a = s.toastAnchor;
    if (a == L"top-left") { x = wa.left + m; y = wa.top + m; }
    else if (a == L"bottom-right") { x = wa.right - kToastW - m; y = wa.bottom - g_toastH - m; }
    else if (a == L"bottom-left") { x = wa.left + m; y = wa.bottom - g_toastH - m; }
    else if (a == L"bottom-center") { x = (wa.left + wa.right - kToastW) / 2; y = wa.bottom - g_toastH - m; }
    else if (a == L"center") { x = (wa.left + wa.right - kToastW) / 2; y = (wa.top + wa.bottom - g_toastH) / 2; }
    else if (a == L"above-taskbar") {
        RECT br{};
        if (barForAnchor && GetWindowRect(barForAnchor, &br) && br.right > br.left && br.bottom > br.top) {
            x = (br.left + br.right - kToastW) / 2;
            y = br.top - g_toastH - m;
        } else {
            // Fallback: bottom-center
            x = (wa.left + wa.right - kToastW) / 2; y = wa.bottom - g_toastH - m;
            LogMsg(L"Reminder toast: bar window not found, bottom-center fallback");
        }
    }
    if (x < wa.left) x = wa.left;
    if (y < wa.top) y = wa.top;
    if (x + kToastW > wa.right) x = wa.right - kToastW;
    if (y + g_toastH > wa.bottom) y = wa.bottom - g_toastH;
    SetWindowPos(h, HWND_TOPMOST, x, y, kToastW, g_toastH, SWP_NOACTIVATE);
    RECT placed{ x, y, x + kToastW, y + g_toastH };
    bool moved = (placed.left != g_lastToastPos.left || placed.top != g_lastToastPos.top ||
                  placed.right != g_lastToastPos.right || placed.bottom != g_lastToastPos.bottom);
    if (moved) {
        g_lastToastPos = placed;
        EnterCriticalSection(&g_settingsCs);
        bool beacon = g_settings.debugBeacon;
        LeaveCriticalSection(&g_settingsCs);
        if (beacon) LogMsg(L"Reminder toast: anchor=%s pos=(%d,%d) size=(%dx%d)", s.toastAnchor.c_str(), x, y, kToastW, g_toastH);
    }
}
// Cached toast title (bold) and body (normal) fonts. They are recreated on the
// first use after any font/size setting change (spec mismatch); callers must
// NEVER delete these handles — teardown on the toast worker thread does that.
static HFONT g_toastFontBold = nullptr;
static HFONT g_toastFontNormal = nullptr;
static std::wstring g_toastFontSpec;
static void EnsureToastFonts(HDC dc) {
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    int px = -MulDiv(s.fontSize, GetDeviceCaps(dc, LOGPIXELSY), 72);
    wchar_t spec[160];
    swprintf_s(spec, L"%s|%d|%d", s.fontName.c_str(), s.fontSize, px);
    if (g_toastFontBold && g_toastFontSpec == spec) return;
    if (g_toastFontBold) { DeleteObject(g_toastFontBold); g_toastFontBold = nullptr; }
    if (g_toastFontNormal) { DeleteObject(g_toastFontNormal); g_toastFontNormal = nullptr; }
    LPCWSTR face = s.fontName.empty() ? L"Segoe UI Variable" : s.fontName.c_str();
    g_toastFontBold   = CreateFontW(px,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,0,0,face);
    g_toastFontNormal = CreateFontW(px,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,face);
    g_toastFontSpec = spec;
}
// Measures the body text and resizes the toast window (all lines shown).
// Called from WM_APP_TOAST before showing; never from WM_PAINT.
static void LayoutToast(HWND h) {
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    int lineH = (std::max)(20, s.fontSize + 10);
    int lines = 1;
    HDC dc = GetDC(h);
    if (dc) {
        EnsureToastFonts(dc);
        HFONT of = (HFONT)SelectObject(dc, g_toastFontNormal);
        RECT mr{ 0, 0, 328, 0 };
        DrawTextW(dc, g_toastBody.c_str(), -1, &mr, DT_CALCRECT|DT_WORDBREAK|DT_LEFT);
        if (mr.bottom > 0 && lineH > 0)
            lines = (std::max)(1, (int)((mr.bottom + lineH - 1) / lineH));
        SelectObject(dc, of);
        ReleaseDC(h, dc);
    }
    g_trTitle = { 16, 10, 344, 10 + lineH };
    g_sepY = 10 + lineH + 5;
    g_trBody = { 16, g_sepY + 7, 344, g_sepY + 7 + lines * lineH };
    g_toastH = g_sepY + 7 + lines * lineH + 10;
    SetWindowPos(h, nullptr, 0, 0, kToastW, g_toastH,
                 SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
}
// Shows one system balloon for (title, body). Creates the tray icon on demand;
// the icon is removed again when the queue drains (no permanent tray clutter).
// NOTE: the icon is shared (LoadIconW(nullptr, ...)) and must never be destroyed.
static HICON g_trayIcon = nullptr;
static int g_sysAddFails = 0; // consecutive NIM_ADD failures (retry backoff)
static bool SysBalloonShow(HWND h, const std::wstring& title, const std::wstring& body) {
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = h;
    nid.uID = kTrayId;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO;
    nid.uCallbackMessage = WM_APP_TRAYICON;
    if (!g_trayIcon) g_trayIcon = LoadIconW(nullptr, IDI_APPLICATION);
    nid.hIcon = g_trayIcon;
    wcsncpy_s(nid.szTip, ARRAYSIZE(nid.szTip), L"Reminder", _TRUNCATE);
    std::wstring t = SafeSubstr(title, 63), b = SafeSubstr(body, 255);
    wcsncpy_s(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), t.c_str(), _TRUNCATE);
    wcsncpy_s(nid.szInfo, ARRAYSIZE(nid.szInfo), b.c_str(), _TRUNCATE);
    nid.dwInfoFlags = NIIF_INFO;
    if (!g_trayAdded) {
        if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
            if (g_sysAddFails < 4) LogMsg(L"Reminder system toast: NIM_ADD failed err=%lu", GetLastError());
            return false;
        }
        g_trayAdded = true;
    } else {
        if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) {
            // Icon lost meanwhile (e.g. explorer restarted around us): re-add.
            g_trayAdded = false;
            if (g_sysAddFails < 4) LogMsg(L"Reminder system toast: NIM_MODIFY failed err=%lu, re-adding", GetLastError());
            if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
                if (g_sysAddFails < 4) LogMsg(L"Reminder system toast: NIM_ADD failed err=%lu", GetLastError());
                return false;
            }
            g_trayAdded = true;
        }
    }
    g_sysAddFails = 0;
    return true;
}
// Pops the next queued system notification and shows it.
static void SysBalloonNext(HWND h) {
    if (g_sysQueue.empty()) { g_sysBusy = false; return; }
    auto [t, b, sf] = g_sysQueue.front();
    g_sysQueue.pop_front();
    g_sysSourceFile = sf;
    if (!SysBalloonShow(h, t, b)) {
        // Transient (tray not ready yet at startup): requeue at head and retry
        // with backoff. Timer id 2 normally hides+advances, which re-attempts
        // the requeued head, so no special timer handling is needed.
        if (g_sysQueue.size() < kToastQueueMax)
            g_sysQueue.emplace_front(std::move(t), std::move(b), std::move(sf));
        else
            LogMsg(L"Reminder system toast: queue full, dropping failed item");
        g_sysBusy = false;
        if (g_sysTimer) KillTimer(h, g_sysTimer);
        int shift = g_sysAddFails < 4 ? g_sysAddFails : 4;
        DWORD delayMs = (DWORD)15000 << shift; // 15s,30s,60s,120s,240s...
        g_sysAddFails++;
        g_sysTimer = SetTimer(h, 2, delayMs, nullptr);
        // Log the first few backoff steps, then only every 16th to stay quiet
        // during a long-lived tray outage.
        if (g_sysAddFails <= 4 || (g_sysAddFails % 16) == 0)
            LogMsg(L"Reminder system toast: retry in %lus (fail#%d)", delayMs / 1000, g_sysAddFails);
        return;
    }
    g_sysBusy = true;
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    if (g_sysTimer) KillTimer(h, g_sysTimer);
    g_sysTimer = SetTimer(h, 2, (UINT)s.toastDurationMs, nullptr);
    LogMsg(L"Reminder system toast: show title=%s", SafeSubstr(t, 40).c_str());
}
// Hides the balloon (and tray icon); advances to the next queued item if asked.
static void SysBalloonHide(HWND h, bool advance) {
    if (g_sysTimer) { KillTimer(h, g_sysTimer); g_sysTimer = 0; }
    if (g_trayAdded) {
        NOTIFYICONDATAW nid{};
        nid.cbSize = sizeof(nid);
        nid.hWnd = h;
        nid.uID = kTrayId;
        Shell_NotifyIconW(NIM_DELETE, &nid);
        g_trayAdded = false;
    }
    g_sysBusy = false;
    if (advance && !g_sysQueue.empty())
        SysBalloonNext(h);
}
static LRESULT CALLBACK ToastWndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    // Tray-icon callbacks (NOTIFYICON_VERSION default: lParam carries the event).
    if (m == WM_APP_TRAYICON) {
        UINT evt = (UINT)l;
        if (evt == NIN_BALLOONUSERCLICK) {
            std::wstring f = g_sysSourceFile.empty() ? FirstFile() : g_sysSourceFile;
            SysBalloonHide(h, true);
            if (!f.empty())
                RequestOpen(f); // never ShellExecute on the window thread
        } else if (evt == NIN_BALLOONTIMEOUT || evt == NIN_BALLOONHIDE) {
            SysBalloonHide(h, true);
        }
        return 0;
    }
    switch (m) {
    case WM_APP_REPOS:
        PositionToastWindow(h);
        return 0;
    case WM_APP_TOAST: {
        if (g_toastQueue.empty()) return 0;
        auto [t, b, sf] = g_toastQueue.front();
        g_toastTitle = std::move(t);
        g_toastBody = std::move(b);
        g_toastSourceFile = std::move(sf);
        g_toastQueue.pop_front();
        LayoutToast(h);
        PositionToastWindow(h);
        InvalidateRect(h, nullptr, TRUE);
        ShowWindow(h, SW_SHOWNOACTIVATE);
        SetWindowPos(h, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
        g_toastBusy = true;
        if (g_toastTimer) KillTimer(h, g_toastTimer);
        Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
        g_toastTimer = SetTimer(h, 1, (UINT)s.toastDurationMs, nullptr);
        return 0;
    }
    case WM_TIMER:
        if (w == 2) {
            // system balloon slot elapsed -> hide icon and show next queued item
            SysBalloonHide(h, true);
            return 0;
        }
        if (w != 1) return 0;
        KillTimer(h, 1); g_toastTimer = 0;
        ShowWindow(h, SW_HIDE);
        g_toastBusy = false;
        if (!g_toastQueue.empty())
            PostMessageW(h, WM_APP_TOAST, 0, 0); // show next queued notification
        return 0;
    case WM_LBUTTONUP: {
        // Single-shot: dismiss at once and debounce rapid clicks. ShellExecute
        // must not run here (it re-enters this thread's loop); the opener
        // thread performs the open asynchronously.
        DWORD nowTick = GetTickCount();
        static DWORD lastClickTick = 0; // window-thread only
        if (nowTick - lastClickTick < 500) return 0;
        lastClickTick = nowTick;
        std::wstring f = g_toastSourceFile.empty() ? FirstFile() : g_toastSourceFile;
        if (g_toastTimer) { KillTimer(h, g_toastTimer); g_toastTimer = 0; }
        ShowWindow(h, SW_HIDE);
        g_toastBusy = false;
        if (!f.empty()) {
            LogMsg(L"Reminder toast click: dismiss + open queued");
            RequestOpen(f);
        }
        if (!g_toastQueue.empty())
            PostMessageW(h, WM_APP_TOAST, 0, 0); // show next queued notification
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        Settings ts; EnterCriticalSection(&g_settingsCs); ts = g_settings; LeaveCriticalSection(&g_settingsCs);
        if (ts.debugBeacon) {
            g_toastPaints++;
            LogMsg(L"Reminder toast: paint #%d", g_toastPaints);
        }
        RECT rc; GetClientRect(h, &rc);
        HBRUSH bg = CreateSolidBrush(ts.debugBeacon ? RGB(255,0,0) : RGB(24,24,24));
        FillRect(dc, &rc, bg); DeleteObject(bg);
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(120,120,120));
        HPEN oldPen = (HPEN)SelectObject(dc, pen);
        HBRUSH oldBr = (HBRUSH)SelectObject(dc, GetStockObject(NULL_BRUSH));
        Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
        SelectObject(dc, oldBr); SelectObject(dc, oldPen); DeleteObject(pen);
        SetBkMode(dc, TRANSPARENT);
        COLORREF tcol; if (!ParseColor(ts.fontColor, tcol)) tcol = RGB(255,255,255);
        SetTextColor(dc, tcol);
        EnsureToastFonts(dc);
        RECT r1 = g_trTitle, r2 = g_trBody;
        HFONT of = (HFONT)SelectObject(dc, g_toastFontBold);
        DrawTextW(dc, g_toastTitle.c_str(), -1, &r1, DT_LEFT|DT_SINGLELINE|DT_END_ELLIPSIS);
        SelectObject(dc, g_toastFontNormal);
        // Separator line between title and body
        {
            HPEN sep = CreatePen(PS_SOLID, 1, RGB(90,90,90));
            HPEN oldSep = (HPEN)SelectObject(dc, sep);
            MoveToEx(dc, 16, g_sepY, nullptr);
            LineTo(dc, 344, g_sepY);
            SelectObject(dc, oldSep); DeleteObject(sep);
        }
        SetTextColor(dc, tcol);
        DrawTextW(dc, g_toastBody.c_str(), -1, &r2, DT_LEFT|DT_WORDBREAK|DT_END_ELLIPSIS);
        SelectObject(dc, of);
        EndPaint(h, &ps);
        return 0;
    }
    }
    return DefWindowProcW(h, m, w, l);
}
// Nudges the taskbar overlay to flash-show briefly when a notification fires
// while it is hidden due to crowding. Posted only; safe cross-thread/process.
static void NotifyBarFlash() {
    HWND bar = FindWindowW(L"ReminderBarCls", L"ReminderBar");
    if (bar && IsWindow(bar))
        PostMessageW(bar, WM_APP_BAR_FLASH, 0, 0);
}
static void ShowToast(const std::wstring& title, const std::wstring& body, const std::wstring& sourceFile = L"") {
    Settings s0; EnterCriticalSection(&g_settingsCs); s0 = g_settings; LeaveCriticalSection(&g_settingsCs);
    bool wantCustom = (s0.toastBackend != L"system");
    bool wantSys = (s0.toastBackend == L"system" || s0.toastBackend == L"both");
    if (wantCustom) {
        if (g_toastQueue.size() >= kToastQueueMax) {
            LogMsg(L"Reminder toast: queue full (%zu), dropping newest", g_toastQueue.size());
        } else {
            g_toastQueue.emplace_back(title, body, sourceFile);
        }
        if (g_toastHwnd) {
            RECT wr{}; GetWindowRect(g_toastHwnd, &wr);
            static RECT lastLogRect{ 0, 0, 0, 0 };
            static bool logInit = false;
            if (s0.debugBeacon || !logInit ||
                wr.left != lastLogRect.left || wr.top != lastLogRect.top ||
                wr.right != lastLogRect.right || wr.bottom != lastLogRect.bottom) {
                HMONITOR mon = MonitorFromWindow(g_toastHwnd, MONITOR_DEFAULTTOPRIMARY);
                MONITORINFO mi{ sizeof(mi) };
                GetMonitorInfoW(mon, &mi);
                LogMsg(L"Reminder toast: show rect=(%d,%d,%d,%d) visible=%d busy=%d mon=(%d,%d,%d,%d)",
                    wr.left, wr.top, wr.right, wr.bottom, (int)(IsWindowVisible(g_toastHwnd) != FALSE),
                    (int)g_toastBusy,
                    mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
                lastLogRect = wr; logInit = true;
            }
            if (!g_toastBusy)
                PostMessageW(g_toastHwnd, WM_APP_TOAST, 0, 0);
        } else {
            LogMsg(L"Reminder toast: no window, cannot show");
        }
        MessageBeep(MB_ICONASTERISK);
    }
    if (wantSys) {
        // System balloon has its own sound; no extra MessageBeep here.
        if (g_sysQueue.size() >= kToastQueueMax) {
            LogMsg(L"Reminder system toast: queue full (%zu), dropping newest", g_sysQueue.size());
        } else {
            g_sysQueue.emplace_back(title, body, sourceFile);
        }
        if (g_toastHwnd) {
            if (!g_sysBusy)
                SysBalloonNext(g_toastHwnd);
        } else {
            LogMsg(L"Reminder system toast: no window, cannot show");
        }
    }
    // Crowded-hidden overlay flashes briefly on a fresh notification.
    NotifyBarFlash();
}

// Due-check with pre-notify and catch-up; called on poll tick in worker process.
// Fired state is tracked per (entry, minute) so ticks never double-fire.
static std::map<std::wstring, long long> g_fired;
static long long g_lastCheckMin = LLONG_MIN;

static void FireEntry(const Entry& e, long long bucket, bool pre) {
    std::wstring key = FiredKey(e) + (pre ? L"@pre@" : L"@") + std::to_wstring(bucket);
    if (g_fired.find(key) != g_fired.end()) return;
    g_fired[key] = bucket;
    ShowToast(WhenLabel(e), e.message, e.sourceFile);
    LogMsg(L"Reminder fired%s: %s | %s", pre ? L" (pre)" : L"", WhenLabel(e).c_str(), SafeSubstr(e.message, 40).c_str());
}

static void CheckDue(bool startup) {
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    SYSTEMTIME now{}; GetLocalTime(&now);
    long long curMin = ToMinKey(now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute);
    long long fromMin;
    if (g_lastCheckMin == LLONG_MIN) {
        fromMin = curMin; // first run: current minute only (+startup grace below)
    } else if (curMin <= g_lastCheckMin) {
        return; // this minute already handled
    } else if (s.catchupWindowMin <= 0) {
        fromMin = curMin; // catch-up disabled
    } else {
        fromMin = g_lastCheckMin + 1;
        if (curMin - fromMin > s.catchupWindowMin) {
            LogMsg(L"Reminder: skipped %lld catch-up minutes (window %d)",
                (curMin - fromMin) - s.catchupWindowMin, s.catchupWindowMin);
            fromMin = curMin - s.catchupWindowMin;
        } else if (fromMin < curMin) {
            LogMsg(L"Reminder: catching up minutes %lld..%lld", fromMin, curMin);
        }
    }
    std::shared_ptr<std::vector<Entry>> sp;
    EnterCriticalSection(&g_entriesCs);
    sp = g_entries;
    LeaveCriticalSection(&g_entriesCs);
    if (!sp) return;
    for (const Entry& e : *sp) {
        // Most recent scheduled fire minute <= curMin
        long long fire = LLONG_MIN;
        bool haveFire = false;
        long long onceK = LLONG_MIN;
        if (e.kind == TimeKind::Daily) {
            long long cand = ToMinKey(now.wYear,now.wMonth,now.wDay,e.hour,e.minute);
            if (cand > curMin) cand -= 1440;
            fire = cand; haveFire = true;
        } else if (e.kind == TimeKind::Hourly) {
            long long cand = ToMinKey(now.wYear,now.wMonth,now.wDay,now.wHour,e.minute);
            if (cand > curMin) cand -= 60;
            fire = cand; haveFire = true;
        } else if (e.kind == TimeKind::Once) {
            long long k = ToMinKey(e.year,e.month,e.day,e.hour,e.minute);
            onceK = k;
            if (k <= curMin) { fire = k; haveFire = true; }
            if (startup && s.checkOnStart && k < curMin &&
                (curMin - k) * 60 <= (s.pollIntervalSec + 120)) {
                FireEntry(e, k, false); // startup grace for a just-missed one-shot
            }
        }
        if (haveFire && fire >= fromMin && fire <= curMin)
            FireEntry(e, fire, false);
        // Pre-notify skipped on the startup pass: a previous owner of the
        // singleton already fired this minute's pre in this session.
        if (s.preNotifyMin > 0 && !startup) {
            long long upcoming = LLONG_MIN;
            bool haveUpcoming = false;
            if (e.kind == TimeKind::Once) {
                upcoming = onceK; haveUpcoming = true;
            } else if (haveFire) {
                upcoming = fire;
                if (e.kind == TimeKind::Daily && fire < curMin) upcoming += 1440;
                else if (e.kind == TimeKind::Hourly && fire < curMin) upcoming += 60;
                haveUpcoming = true;
            }
            if (haveUpcoming) {
                long long pre = upcoming - s.preNotifyMin;
                if (pre >= fromMin && pre <= curMin)
                    FireEntry(e, pre, true);
            }
        }
    }
    // Prune fired state older than 2 days to bound memory
    for (auto it = g_fired.begin(); it != g_fired.end(); ) {
        if (it->second < curMin - 2880) it = g_fired.erase(it);
        else ++it;
    }
    g_lastCheckMin = curMin;
}

// Tool worker thread
static HANDLE g_stopEvent = nullptr;
static HANDLE g_workerThread = nullptr;
static DWORD g_workerThreadId = 0;
static HANDLE g_workerMtx = nullptr;
static bool g_workerRunning = false;
static bool g_lastMissing = false; // worker-side "schedules unavailable" log-once
// Returns true if the current process image is explorer.exe (forward decl helper).
static bool IsSelfExplorer() {
    WCHAR p[MAX_PATH]{}; GetModuleFileNameW(nullptr, p, ARRAYSIZE(p));
    const wchar_t* n = wcsrchr(p, L'\\'); n = n ? n + 1 : p;
    return _wcsicmp(n, L"explorer.exe") == 0;
}
// True if any explorer.exe process exists (used to prefer explorer as worker owner).
static bool AnyExplorerRunning() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, L"explorer.exe") == 0) { found = true; break; }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}
static DWORD WINAPI ToolWorker(LPVOID) {
    g_workerThreadId = GetCurrentThreadId();
    // Drop a stale class registration from a previous load (see BarThreadProc).
    UnregisterClassW(L"ReminderToastCls", GetModuleHandle(nullptr));
    WNDCLASSW wc{}; wc.lpfnWndProc = ToastWndProc; wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"ReminderToastCls"; wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClassW(&wc)) {
        LogMsg(L"Reminder tool: RegisterClassW failed err=%lu", GetLastError());
        return 1;
    }
    g_toastHwnd = CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE, L"ReminderToastCls", L"Reminder",
        WS_POPUP, 0, 0, kToastW, g_toastH, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    if (!g_toastHwnd) {
        LogMsg(L"Reminder tool: toast window creation failed err=%lu", GetLastError());
        UnregisterClassW(L"ReminderToastCls", GetModuleHandle(nullptr));
        return 1;
    }
    PositionToastWindow(g_toastHwnd);
    bool missing = false;
    MaybeReload(true, missing);
    {
        EnterCriticalSection(&g_entriesCs);
        size_t n = g_entries ? g_entries->size() : 0;
        LeaveCriticalSection(&g_entriesCs);
        Settings s0; EnterCriticalSection(&g_settingsCs); s0 = g_settings; LeaveCriticalSection(&g_settingsCs);
        LogMsg(L"Reminder tool started: files=%zu entries=%zu missing=%d", s0.scheduleFiles.size(), n, (int)missing);
    }
    g_lastMissing = missing;
    // startup grace fire
    CheckDue(true);
    MSG msg;
    bool done = false;
    bool yielded = false;
    while (!done) {
        // pump messages with timeout
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { done = true; break; }
            TranslateMessage(&msg); DispatchMessageW(&msg);
        }
        if (done) break;
        Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
        // Wake at the next minute boundary (+2s) or after pollIntervalSec, whichever first,
        // so notifications fire just after :00 instead of drifting with the poll phase.
        SYSTEMTIME st{}; GetLocalTime(&st);
        DWORD msToBoundary = (DWORD)((60 - st.wSecond) * 1000) - st.wMilliseconds + 2000;
        DWORD waitMs = (DWORD)s.pollIntervalSec * 1000;
        if (msToBoundary < waitMs) waitMs = msToBoundary;
        if (WaitForSingleObject(g_stopEvent, waitMs) == WAIT_OBJECT_0) break;
        bool miss = false;
        MaybeReload(false, miss);
        if (miss != g_lastMissing) {
            g_lastMissing = miss;
            if (miss) LogMsg(L"schedule file missing, keeping previous");
        }
        CheckDue(false);
        // Explorer-preferred ownership: a windhawk-hosted worker yields once the
        // explorer overlay exists, so the explorer process can take over via retry.
        // Toasts keep working during handoff (cross-process FindWindow still finds the bar).
        if (!IsSelfExplorer()) {
            HWND bar = FindWindowW(L"ReminderBarCls", L"ReminderBar");
            if (bar && IsWindow(bar)) {
                LogMsg(L"Reminder worker (windhawk): explorer bar detected, yielding to explorer");
                yielded = true;
                break;
            }
        }
    }
    // Owner-thread cleanup: always destroy the window and drop the class here
    // (both the stop-event and the WM_QUIT paths converge here).
    // System-balloon state belongs to this thread too: drop the tray icon first.
    if (g_toastHwnd) {
        if (g_sysTimer) { KillTimer(g_toastHwnd, g_sysTimer); g_sysTimer = 0; }
        if (g_trayAdded) {
            NOTIFYICONDATAW nid{};
            nid.cbSize = sizeof(nid);
            nid.hWnd = g_toastHwnd;
            nid.uID = kTrayId;
            Shell_NotifyIconW(NIM_DELETE, &nid);
            g_trayAdded = false;
        }
        g_sysQueue.clear();
        g_sysBusy = false;
        g_sysSourceFile.clear();
    } else {
        g_sysTimer = 0;
    }
    if (g_toastHwnd) {
        if (g_toastTimer) { KillTimer(g_toastHwnd, g_toastTimer); g_toastTimer = 0; }
        DestroyWindow(g_toastHwnd); g_toastHwnd = nullptr;
    } else {
        g_toastTimer = 0;
    }
    UnregisterClassW(L"ReminderToastCls", GetModuleHandle(nullptr));
    if (g_toastFontBold) { DeleteObject(g_toastFontBold); g_toastFontBold = nullptr; }
    if (g_toastFontNormal) { DeleteObject(g_toastFontNormal); g_toastFontNormal = nullptr; }
    g_toastFontSpec.clear();
    if (yielded) {
        // Release the singleton so the explorer retry can acquire it.
        // g_workerRunning stays true until ModUninit; the thread simply exits.
        // Atomic take: ModUninit may concurrently close from another thread.
        HANDLE h = (HANDLE)InterlockedExchangePointer((PVOID volatile*)&g_workerMtx, nullptr);
        if (h) CloseHandle(h);
        LogMsg(L"Reminder worker (windhawk): yielded, mutex released");
    }
    return 0;
}
static bool WhTool_ModInit() {
    if (!g_settingsCsInit) { InitializeCriticalSection(&g_settingsCs); InitializeCriticalSection(&g_entriesCs); InitializeCriticalSection(&g_reloadCs); InitializeCriticalSection(&g_openCs); g_settingsCsInit = true; }
    LoadSettings();
    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_workerThread = CreateThread(nullptr, 0, ToolWorker, nullptr, 0, nullptr);
    g_workerRunning = (g_stopEvent != nullptr && g_workerThread != nullptr);
    {
        // NOTE: no MaybeReload here; the worker thread does the initial load.
        // Doing it on the init thread too only delayed enable reflection.
        Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
        LogMsg(L"Reminder tool starting: files=%zu", s.scheduleFiles.size());
    }
    return TRUE;
}
static void WhTool_ModSettingsChanged() {
    LoadSettings();
    bool miss = false; MaybeReload(true, miss);
    if (g_toastHwnd && IsWindow(g_toastHwnd)) PostMessageW(g_toastHwnd, WM_APP_REPOS, 0, 0);
}
static void WhTool_ModUninit() {
    if (!g_workerRunning) return;
    g_workerRunning = false;
    if (g_stopEvent) SetEvent(g_stopEvent);
    if (g_workerThreadId) PostThreadMessageW(g_workerThreadId, WM_QUIT, 0, 0);
    if (g_workerThread) { WaitForSingleObject(g_workerThread, 2000); CloseHandle(g_workerThread); g_workerThread = nullptr; }
    if (g_stopEvent) { CloseHandle(g_stopEvent); g_stopEvent = nullptr; }
    // NOTE: g_toastHwnd is owned by the worker thread; only it clears it.
    // Nulling it here would race with a still-exiting thread.
    g_workerThreadId = 0;
    HANDLE h = (HANDLE)InterlockedExchangePointer((PVOID volatile*)&g_workerMtx, nullptr);
    if (h) CloseHandle(h);
    LogMsg(L"Reminder tool stopped");
}

// Starts the schedule-watcher/toast worker exactly once per desktop session.
// Safe to call from any interactive host process (explorer.exe, windhawk.exe).
// Ownership converges to explorer: windhawk yields when the bar appears (see ToolWorker),
// and explorer retries acquisition on its slow poll timer.
static bool StartWorkerOnce(const wchar_t* owner) {
    HANDLE mtx = CreateMutexW(nullptr, FALSE, L"Local\\reminder-worker-singleton");
    if (!mtx) {
        LogMsg(L"Reminder worker (%s pid=%lu): mutex failed err=%lu", owner, GetCurrentProcessId(), GetLastError());
        return false;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        LogMsg(L"Reminder worker (%s pid=%lu): already running elsewhere", owner, GetCurrentProcessId());
        CloseHandle(mtx);
        return false;
    }
    g_workerMtx = mtx; // closed in WhTool_ModUninit (or on windhawk yield)
    if (!WhTool_ModInit() || !g_workerRunning) {
        if (g_workerMtx) { CloseHandle(g_workerMtx); g_workerMtx = nullptr; }
        return false;
    }
    LogMsg(L"Reminder worker (%s pid=%lu): started", owner, GetCurrentProcessId());
    return true;
}

// ---------- Explorer overlay (taskbar small window) ----------
// NOTE: the bar window and its tooltip are owned by a dedicated UI thread
// (BarThreadProc). Creation, message pump, and destruction all happen on that
// thread: the thread that calls Wh_ModInit never pumps, so owning windows
// there hangs input (hourglass on hover) and crashes on click/unload.
static HWND g_barHwnd = nullptr;
static UINT_PTR g_barTimer = 0;      // UI refresh timer id 7 (fast, 2s)
static UINT_PTR g_barPollTimer = 0;  // schedule reload timer id 8 (slow)
static UINT_PTR g_barFastTimer = 0;  // taskbar retry timer id 9 (500ms, fallback only)
static UINT g_taskbarCreatedMsg = 0; // RegisterWindowMessageW(L"TaskbarCreated")
static bool g_barInFallback = false; // last BarPositionWindow used workarea/none
#define WM_APP_BAR_UPDATE (WM_APP + 201)
#define WM_APP_BAR_OPEN (WM_APP + 202) // obsolete (opener thread); kept for in-flight messages
static HANDLE g_barStopEvent = nullptr;
static HANDLE g_barThread = nullptr;
static DWORD g_barThreadId = 0;
static bool IsExplorerProcess() {
    WCHAR p[MAX_PATH]{}; GetModuleFileNameW(nullptr, p, ARRAYSIZE(p));
    const wchar_t* n = wcsrchr(p, L'\\'); n = n ? n + 1 : p;
    return _wcsicmp(n, L"explorer.exe") == 0;
}
static RECT g_lastBarPos{ -1, -1, -1, -1 };
static bool g_lastBarShown = false;
static bool g_barCrowded = false;     // true while hidden/shrunk due to icon crowding
static UINT_PTR g_barFlashTimer = 0;  // one-shot timer id 10 (flash show on fire)
#ifndef WM_APP_BAR_FLASH
#define WM_APP_BAR_FLASH (WM_APP + 203) // briefly show the bar when a notification fires
#endif
// Cached bar text font (recreated on font/size/DPI change; owned by the bar UI
// thread, deleted in BarThreadProc teardown).
static HFONT g_barFont = nullptr;
static std::wstring g_barFontSpec;
static void EnsureBarFont(HDC dc, const Settings& s) {
    int px = -MulDiv(s.fontSize, GetDeviceCaps(dc, LOGPIXELSY), 72);
    wchar_t spec[160];
    swprintf_s(spec, L"%s|%d|%d", s.fontName.c_str(), s.fontSize, px);
    if (g_barFont && g_barFontSpec == spec) return;
    if (g_barFont) { DeleteObject(g_barFont); g_barFont = nullptr; }
    LPCWSTR face = s.fontName.empty() ? nullptr : s.fontName.c_str();
    g_barFont = CreateFontW(px,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,face);
    g_barFontSpec = spec;
}
// Free horizontal pixels between the running-apps button area and the notify
// area on the primary taskbar. Returns false when the layout can't be read
// (caller falls back to the legacy fixed placement).
static bool GetTrayGap(int& freePx) {
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!tray) return false;
    HWND taskSw = FindWindowExW(tray, nullptr, L"MSTaskSwWClass", nullptr);
    HWND taskList = taskSw ? FindWindowExW(taskSw, nullptr, L"MSTaskListWClass", nullptr) : nullptr;
    if (!taskList) taskList = FindWindowExW(tray, nullptr, L"MSTaskListWClass", nullptr);
    HWND nt = FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr);
    RECT tr{}, nr{};
    if (!taskList || !GetWindowRect(taskList, &tr) || tr.right <= tr.left) return false;
    if (!nt || !GetWindowRect(nt, &nr) || nr.right <= nr.left) return false;
    freePx = nr.left - tr.right;
    return true;
}
// Width that fits the current rows with the taskbar font (<= fullW, >= minW).
static int MeasureBarWidth(HWND h, const Settings& s, int fullW, int minW) {
    HDC dc = GetDC(h);
    if (!dc) return fullW;
    EnsureBarFont(dc, s);
    HFONT of = (HFONT)SelectObject(dc, g_barFont);
    SIZE tsz{}; GetTextExtentPoint32W(dc, L"00:00", 5, &tsz);
    int timeW = tsz.cx + 10;
    auto items = Upcoming(s.taskbarCount);
    int need = 0;
    if (items.empty()) {
        SIZE sz{}; GetTextExtentPoint32W(dc, L"予定なし", 4, &sz);
        need = timeW + sz.cx + 28;
    } else {
        for (auto& e : items) {
            std::wstring first = e.message;
            size_t nl = first.find_first_of(L"\r\n");
            if (nl != std::wstring::npos) first.resize(nl);
            size_t nch = (std::min)(first.size(), (size_t)60);
            SIZE sz{};
            if (nch > 0) GetTextExtentPoint32W(dc, first.c_str(), (int)nch, &sz);
            int row = timeW + sz.cx + 28;
            if (row > need) need = row;
        }
    }
    SelectObject(dc, of); ReleaseDC(h, dc);
    if (need < minW) need = minW;
    if (need > fullW) need = fullW;
    return need;
}
static void BarPositionWindow(HWND h, bool logPos) {
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    // 1. Real taskbar rect via Shell_TrayWnd (robust across multi-monitor setups)
    RECT task{}; const wchar_t* src = L"none";
    UINT taskEdge = ABE_BOTTOM;
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (tray && GetWindowRect(tray, &task) && task.right > task.left && task.bottom > task.top) {
        src = L"TrayWnd";
    } else {
        APPBARDATA abd{}; abd.cbSize = sizeof(abd);
        if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd) && abd.rc.right > abd.rc.left && abd.rc.bottom > abd.rc.top) {
            task = abd.rc; taskEdge = abd.uEdge; src = L"AppBar";
        }
    }
    // Tray (notify area) rect for tray-left anchoring
    RECT notifyR{}; bool haveNotify = false;
    if (tray) {
        HWND nt = FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr);
        if (nt && GetWindowRect(nt, &notifyR) && notifyR.right > notifyR.left && notifyR.bottom > notifyR.top)
            haveNotify = true;
    }
    if (wcscmp(src, L"none") == 0) {
        // Fallback: primary monitor work area bottom-right
        RECT wa{}; SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0);
        if ((wa.right > wa.left) && (wa.bottom > wa.top)) {
            task = wa;
            task.top = wa.bottom - 48;
            taskEdge = ABE_BOTTOM;
            src = L"workarea";
            g_barInFallback = true;
            LogMsg(L"Reminder: no taskbar rect (tray=%d appbar failed), using work-area fallback",
                (int)(tray != nullptr));
        } else {
            g_barInFallback = true;
            LogMsg(L"Reminder: no taskbar rect and no work area, skip positioning");
            return;
        }
    } else {
        if (g_barInFallback)
            LogMsg(L"Reminder bar: taskbar recovered via %s", src);
        g_barInFallback = false;
    }
    if (wcscmp(src, L"TrayWnd") == 0) {
        // Derive edge from monitor geometry
        HMONITOR mon = MonitorFromRect(&task, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi{ sizeof(mi) };
        if (GetMonitorInfoW(mon, &mi)) {
            const RECT& mr = mi.rcMonitor;
            int tol = 4;
            if (abs(task.bottom - mr.bottom) <= tol && (task.bottom - task.top) < (mr.bottom - mr.top) / 2) taskEdge = ABE_BOTTOM;
            else if (abs(task.top - mr.top) <= tol && (task.bottom - task.top) < (mr.bottom - mr.top) / 2) taskEdge = ABE_TOP;
            else if (abs(task.right - mr.right) <= tol) taskEdge = ABE_RIGHT;
            else if (abs(task.left - mr.left) <= tol) taskEdge = ABE_LEFT;
        }
    }
    // window size
    int dpi = 96;
    HMODULE shcore = GetModuleHandleW(L"shcore.dll");
    if (shcore) {
        // fallback: GetDpiForWindow may not exist on old builds
        HMODULE u32 = GetModuleHandleW(L"user32.dll");
        FARPROC fp = u32 ? GetProcAddress(u32, "GetDpiForWindow") : nullptr;
        if (fp) dpi = ((UINT(WINAPI*)(HWND))fp)(h);
    }
    int lineH = (std::max)(20, s.fontSize + 10);
    int fullW = MulDiv(230, dpi, 96);
    int w = fullW, hh = MulDiv(s.taskbarCount * lineH + 14, dpi, 96);
    int minW = MulDiv(s.taskbarMinWidth, dpi, 96);
    int x = 0, y = 0;
    int tw = task.right - task.left, th = task.bottom - task.top;
    bool crowded = false; // true when icons leave no room for minW
    if (tw >= th) { // horizontal taskbar (bottom/top)
        // Limit window height to taskbar height with margin
        if (hh > th - 8) hh = (std::max)(th - 8, 20);
        // Auto-fit: shrink to the text width and to the free gap so running
        // app buttons are not covered. Applies to tray-left (gap measurable);
        // other anchors only shrink to text width.
        bool isTrayLeft = (s.taskbarAnchor != L"taskbar-center" && s.taskbarAnchor != L"taskbar-left");
        if (s.taskbarAutoFit)
            w = MeasureBarWidth(h, s, fullW, minW);
        if (isTrayLeft && (s.taskbarAutoFit || s.taskbarCrowdedHide) && wcscmp(src, L"TrayWnd") == 0) {
            int freePx = 0;
            if (GetTrayGap(freePx)) {
                freePx -= 8; // breathing room next to the notify area (x is -8)
                if (freePx < minW) {
                    crowded = true;
                } else if (w > freePx) {
                    w = (std::max)(freePx, minW);
                }
            }
        }
        // Embed inside the taskbar: bottom-aligned (4px margin) for bottom bars,
        // top-aligned for top bars, clamped to stay within the bar.
        y = (taskEdge == ABE_TOP) ? task.top + 4 : task.bottom - hh - 4;
        if (y < task.top) y = task.top;
        if (y + hh > task.bottom) y = task.bottom - hh;
        if (s.taskbarAnchor == L"taskbar-center") x = task.left + (tw - w) / 2;
        else if (s.taskbarAnchor == L"taskbar-left") x = task.left + 120;
        else if (haveNotify) x = notifyR.left - w - 8; // exact tray-left
        else x = task.right - w - 8;
        if (x < task.left + 4) x = task.left + 4;
        if (x + w > task.right - 4) x = task.right - w - 4;
    } else { // vertical taskbar
        x = (taskEdge == ABE_LEFT) ? task.right + 4 : task.left - w - 4;
        y = task.bottom - hh - 40;
        // Clamp into the work area so the bar cannot go off-screen.
        RECT wa{}; SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0);
        if (y < wa.top) y = wa.top;
        if (y + hh > wa.bottom) y = (hh <= wa.bottom - wa.top) ? wa.bottom - hh : wa.top;
        if (x < wa.left) x = wa.left;
        if (x + w > wa.right) x = wa.right - w;
    }
    if (crowded != g_barCrowded) {
        g_barCrowded = crowded;
        if (crowded)
            LogMsg(L"Reminder bar: crowded, hiding (icons leave no room)");
        else
            LogMsg(L"Reminder bar: crowding cleared, showing again");
    }
    // Fine adjustment (no re-clamp by design)
    x += s.taskbarOffsetX;
    y += s.taskbarOffsetY;
    RECT placed{ x, y, x + w, y + hh };
    bool moved = (placed.left != g_lastBarPos.left || placed.top != g_lastBarPos.top ||
                  placed.right != g_lastBarPos.right || placed.bottom != g_lastBarPos.bottom);
    // Hidden while crowded (unless flashing on a fresh notification); flash keeps
    // it visible for taskbarFlashSec even with no room.
    bool flashActive = (g_barFlashTimer != 0);
    bool showNow = s.taskbarShowNext && !(crowded && s.taskbarCrowdedHide && !flashActive);
    // Avoid re-asserting TOPMOST every tick (fights fullscreen apps):
    // only take TOPMOST on init/settings/TaskbarCreated/move/show-change.
    bool zChange = logPos || moved || (showNow != g_lastBarShown);
    UINT showFlag = showNow ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
    if (zChange)
        SetWindowPos(h, HWND_TOPMOST, x, y, w, hh, SWP_NOACTIVATE | showFlag);
    else
        SetWindowPos(h, nullptr, x, y, w, hh, SWP_NOZORDER | SWP_NOACTIVATE | showFlag);
    g_lastBarShown = showNow;
    if (logPos || moved) {
        g_lastBarPos = placed;
        EnterCriticalSection(&g_entriesCs);
        size_t n = g_entries ? g_entries->size() : 0;
        LeaveCriticalSection(&g_entriesCs);
        LogMsg(L"Reminder bar: src=%s pos=(%d,%d) size=(%dx%d) task=(%d,%d,%d,%d) edge=%u notify=%d show=%d entries=%zu",
            src, x, y, w, hh, task.left, task.top, task.right, task.bottom,
            taskEdge, (int)haveNotify, (int)s.taskbarShowNext, n);
    }
    // opacity
    LONG ex = GetWindowLongW(h, GWL_EXSTYLE);
    SetWindowLongW(h, GWL_EXSTYLE, ex | WS_EX_LAYERED);
    BYTE alpha = (BYTE)(s.taskbarOpacity * 255 / 100);
    SetLayeredWindowAttributes(h, 0, alpha, LWA_ALPHA);
}
static LRESULT CALLBACK BarWndProc(HWND h, UINT m, WPARAM w, LPARAM l);
static HWND g_tipHwnd = nullptr;
static std::wstring g_tipText;
static HFONT g_tipFont = nullptr;
static std::wstring g_tipFontSpec;
static std::vector<RECT> g_rowRects;
static std::vector<Entry> g_rowEntries;

// Applies taskbar font family/size and text color to the hover tooltip
static void ApplyTipFont() {
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    if (s.taskbarTooltipCount <= 0 || !g_tipHwnd) return;
    wchar_t spec[160];
    swprintf_s(spec, L"%s|%d", s.fontName.c_str(), s.fontSize);
    if (!g_tipFont || g_tipFontSpec != spec) {
        if (g_tipFont) { DeleteObject(g_tipFont); g_tipFont = nullptr; }
        HDC dc = GetDC(g_tipHwnd);
        int px = -MulDiv(s.fontSize, GetDeviceCaps(dc, LOGPIXELSY), 72);
        ReleaseDC(g_tipHwnd, dc);
        LPCWSTR face = s.fontName.empty() ? nullptr : s.fontName.c_str();
        g_tipFont = CreateFontW(px,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,face);
        g_tipFontSpec = spec;
        if (g_tipFont)
            SendMessageW(g_tipHwnd, WM_SETFONT, (WPARAM)g_tipFont, TRUE);
    }
    COLORREF col;
    COLORREF bkcol = s.debugBeacon ? RGB(255,0,0) : RGB(24,24,24);
    if (ParseColor(s.fontColor, col))
        SendMessageW(g_tipHwnd, TTM_SETTIPTEXTCOLOR, (WPARAM)col, 0);
    SendMessageW(g_tipHwnd, TTM_SETTIPBKCOLOR, (WPARAM)bkcol, 0);
}

// Creates/destroys the hover tooltip per taskbarTooltipCount setting
static void EnsureTip(HWND h) {
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    if (s.taskbarTooltipCount <= 0) {
        if (g_tipHwnd) { DestroyWindow(g_tipHwnd); g_tipHwnd = nullptr; }
        if (g_tipFont) { DeleteObject(g_tipFont); g_tipFont = nullptr; }
        g_tipFontSpec.clear();
        return;
    }
    if (g_tipHwnd) return;
    g_tipHwnd = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        h, nullptr, GetModuleHandle(nullptr), nullptr);
    if (!g_tipHwnd) {
        LogMsg(L"Reminder: tooltip creation failed err=%lu", GetLastError());
        return;
    }
    TOOLINFOW ti{}; ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = h; ti.uId = (UINT_PTR)h;
    SendMessageW(g_tipHwnd, TTM_ADDTOOLW, 0, (LPARAM)&ti);
    SendMessageW(g_tipHwnd, TTM_SETMAXTIPWIDTH, 0, 480);
    // Disable visual styles so custom text/background colors take effect
    SetWindowTheme(g_tipHwnd, L"", L"");
    ApplyTipFont();
}

// Refreshes hover tooltip text: "HH:MM message" per row
static void UpdateTip(HWND h) {
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    if (s.taskbarTooltipCount <= 0 || !g_tipHwnd) return;
    auto items = Upcoming(s.taskbarTooltipCount);
    std::wstring tip;
    if (items.empty()) {
        tip = L"予定なし";
    } else {
        for (auto& e : items)
            tip += TipWhenLabel(e) + L" " + e.message + L"\r\n";
        while (!tip.empty() && (tip.back() == L'\n' || tip.back() == L'\r')) tip.pop_back();
    }
    g_tipText = tip;
    TOOLINFOW ti{}; ti.cbSize = sizeof(ti);
    ti.hwnd = h; ti.uId = (UINT_PTR)h;
    ti.lpszText = g_tipText.data(); // never empty (falls back to L"予定なし")
    SendMessageW(g_tipHwnd, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
    ApplyTipFont();
}

static void DestroyTip() {
    if (g_tipHwnd) { DestroyWindow(g_tipHwnd); g_tipHwnd = nullptr; }
    if (g_tipFont) { DeleteObject(g_tipFont); g_tipFont = nullptr; }
    g_tipFontSpec.clear();
    g_tipText.clear();
}
static LRESULT CALLBACK BarWndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    // TaskbarCreated is a registered message (dynamic id): check before switch.
    if (g_taskbarCreatedMsg && m == g_taskbarCreatedMsg) {
        LogMsg(L"Reminder bar: TaskbarCreated, repositioning");
        BarPositionWindow(h, true);
        if (!g_barInFallback && g_barFastTimer) { KillTimer(h, g_barFastTimer); g_barFastTimer = 0; }
        EnsureTip(h);
        UpdateTip(h);
        InvalidateRect(h, nullptr, TRUE);
        return 0;
    }
    switch (m) {
    case WM_APP_BAR_UPDATE: {
        // Applied on the owner (UI) thread. Posted by Explorer_ModSettingsChanged.
        Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
        if (g_barTimer) KillTimer(h, g_barTimer);
        if (g_barPollTimer) KillTimer(h, g_barPollTimer);
        if (g_barFastTimer) KillTimer(h, g_barFastTimer);
        if (g_barFlashTimer) KillTimer(h, g_barFlashTimer);
        g_barTimer = SetTimer(h, 7, 2000, nullptr);
        g_barPollTimer = SetTimer(h, 8, (UINT)s.pollIntervalSec * 1000, nullptr);
        g_barFastTimer = 0;
        g_barFlashTimer = 0;
        BarPositionWindow(h, true);
        if (g_barInFallback)
            g_barFastTimer = SetTimer(h, 9, 500, nullptr);
        EnsureTip(h);
        UpdateTip(h);
        InvalidateRect(h, nullptr, TRUE);
        return 0;
    }
    case WM_APP_BAR_FLASH: {
        // A notification fired while the bar is hidden due to crowding:
        // show briefly at the last computed position, then hide again on timer 10.
        Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
        if (!s.taskbarShowNext || !g_barCrowded) return 0;
        ShowWindow(h, SW_SHOWNOACTIVATE);
        SetWindowPos(h, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        if (g_barFlashTimer) KillTimer(h, g_barFlashTimer);
        g_barFlashTimer = SetTimer(h, 10, (UINT)s.taskbarFlashSec * 1000, nullptr);
        LogMsg(L"Reminder bar: flash show for %ds (crowded)", s.taskbarFlashSec);
        InvalidateRect(h, nullptr, TRUE);
        return 0;
    }
    case WM_APP_BAR_OPEN: {
        // Obsolete: opens now go through the opener thread (RequestOpen).
        // Kept so any in-flight posted message is harmlessly ignored.
        return 0;
    }
    case WM_TIMER:
        if (w == 7) {
            if (!IsWindow(h)) return 0;
            RECT before = g_lastBarPos;
            BarPositionWindow(h, false);
            // Bar moved (e.g. fallback -> TrayWnd): nudge any toast to re-anchor above it.
            if (memcmp(&before, &g_lastBarPos, sizeof(RECT)) != 0) {
                HWND toast = FindWindowW(L"ReminderToastCls", L"Reminder");
                if (toast && IsWindow(toast))
                    PostMessageW(toast, (WM_APP + 102), 0, 0); // WM_APP_REPOS
            }
            EnsureTip(h);
            UpdateTip(h);
            InvalidateRect(h, nullptr, TRUE);
        } else if (w == 9) {
            // Fast retry while in work-area fallback (explorer restart race).
            if (!IsWindow(h)) return 0;
            BarPositionWindow(h, false);
            if (!g_barInFallback) {
                KillTimer(h, g_barFastTimer); g_barFastTimer = 0;
                LogMsg(L"Reminder bar: fast retry recovered, back to 2s timer");
            }
            EnsureTip(h);
            UpdateTip(h);
            InvalidateRect(h, nullptr, TRUE);
        } else if (w == 8) {
            bool miss = false;
            MaybeReload(false, miss);
            // Explorer-preferred worker: retry acquisition if windhawk yielded/never started.
            if (!g_workerRunning && !g_workerMtx) {
                if (StartWorkerOnce(L"explorer-retry"))
                    LogMsg(L"Reminder worker (explorer-retry): acquired after yield");
            }
            if (!IsWindow(h)) return 0;
            UpdateTip(h);
            InvalidateRect(h, nullptr, TRUE);
        } else if (w == 10) {
            // Flash time elapsed: hide again if still crowded.
            if (g_barFlashTimer) { KillTimer(h, g_barFlashTimer); g_barFlashTimer = 0; }
            LogMsg(L"Reminder bar: flash end");
            BarPositionWindow(h, false);
            InvalidateRect(h, nullptr, TRUE);
        }
        return 0;
    case WM_LBUTTONUP: {
        DWORD nowTick = GetTickCount();
        static DWORD lastBarClickTick = 0; // UI-thread only
        if (nowTick - lastBarClickTick < 500) return 0;
        lastBarClickTick = nowTick;
        // Decide the target here; the opener thread performs the open so this
        // UI thread never runs ShellExecute (re-entrancy crash, see toast).
        POINT pt{ (int)(short)LOWORD(l), (int)(short)HIWORD(l) };
        std::wstring target;
        for (size_t i = 0; i < g_rowRects.size(); i++) {
            if (PtInRect(&g_rowRects[i], pt)) {
                if (i < g_rowEntries.size() && !g_rowEntries[i].sourceFile.empty())
                    target = g_rowEntries[i].sourceFile;
                break;
            }
        }
        if (target.empty()) target = FirstFile();
        if (!target.empty())
            RequestOpen(target);
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
        if (s.debugBeacon) {
            g_barPaints++;
            LogMsg(L"Reminder bar: paint #%d", g_barPaints);
        }
        RECT rc; GetClientRect(h, &rc);
        HBRUSH bg = CreateSolidBrush(s.debugBeacon ? RGB(255,0,0) : RGB(24,24,24));
        FillRect(dc, &rc, bg); DeleteObject(bg);
        COLORREF col; if (!ParseColor(s.fontColor, col)) col = RGB(255,255,255);
        SetBkMode(dc, TRANSPARENT); SetTextColor(dc, col);
        EnsureBarFont(dc, s);
        HFONT of = (HFONT)SelectObject(dc, g_barFont);
        auto items = Upcoming(s.taskbarCount);
        g_rowRects.clear(); g_rowEntries.clear();
        // Fixed-width time column so row starts always align
        SIZE tsz{}; GetTextExtentPoint32W(dc, L"00:00", 5, &tsz);
        int timeW = tsz.cx + 10;
        HPEN framePen = CreatePen(PS_SOLID, 1, s.debugBeacon ? RGB(255,255,255) : RGB(90,90,90));
        HBRUSH rowBg = CreateSolidBrush(s.debugBeacon ? RGB(255,0,0) : RGB(36,36,36));
        int y = 7; int lineH = (std::max)(20, s.fontSize + 10);
        auto drawRow = [&](const RECT& fr, const std::wstring& timeStr,
                           const std::wstring& textStr, const Entry* ent) {
            HBRUSH oldB = (HBRUSH)SelectObject(dc, rowBg);
            HPEN oldP = (HPEN)SelectObject(dc, framePen);
            RoundRect(dc, fr.left, fr.top, fr.right, fr.bottom, 6, 6);
            SelectObject(dc, oldB); SelectObject(dc, oldP);
            RECT rt{ fr.left + 8, fr.top, fr.left + 8 + timeW, fr.bottom };
            DrawTextW(dc, timeStr.c_str(), -1, &rt, DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
            RECT ri{ fr.left + 8 + timeW + 6, fr.top, fr.right - 6, fr.bottom };
            DrawTextW(dc, textStr.c_str(), -1, &ri, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
            g_rowRects.push_back(fr);
            if (ent) g_rowEntries.push_back(*ent);
        };
        if (items.empty()) {
            RECT fr{ 4, y, rc.right - 4, y + lineH };
            drawRow(fr, L"--:--", L"予定なし", nullptr);
        } else {
            for (auto& e : items) {
                RECT fr{ 4, y, rc.right - 4, y + lineH };
                std::wstring firstLine = e.message;
                size_t nl = firstLine.find_first_of(L"\r\n");
                if (nl != std::wstring::npos) firstLine.resize(nl);
                else firstLine = SafeSubstr(firstLine, 60);
                drawRow(fr, WhenLabel(e), firstLine, &e);
                y += lineH;
            }
        }
        DeleteObject(framePen); DeleteObject(rowBg);
        SelectObject(dc, of);
        EndPaint(h, &ps);
        return 0;
    }
    case WM_DISPLAYCHANGE:
    case WM_SETTINGCHANGE:
        BarPositionWindow(h, false);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}
static DWORD WINAPI BarThreadProc(LPVOID) {
    g_barThreadId = GetCurrentThreadId();
    g_taskbarCreatedMsg = RegisterWindowMessageW(L"TaskbarCreated");
    // Drop a stale class registration from a previous load: its WndProc would
    // point into unloaded code and crash on first message after re-enable.
    UnregisterClassW(L"ReminderBarCls", GetModuleHandle(nullptr));
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icc);
    WNDCLASSW wc{}; wc.lpfnWndProc = BarWndProc; wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"ReminderBarCls"; wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClassW(&wc)) {
        LogMsg(L"Reminder explorer: RegisterClassW failed err=%lu", GetLastError());
        return 1;
    }
    g_barHwnd = CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE|WS_EX_LAYERED,
        L"ReminderBarCls", L"ReminderBar", WS_POPUP,
        0,0,230,60, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    if (!g_barHwnd) {
        LogMsg(L"Reminder explorer: CreateWindowExW failed err=%lu", GetLastError());
        UnregisterClassW(L"ReminderBarCls", GetModuleHandle(nullptr));
        return 1;
    }
    bool miss = false; MaybeReload(true, miss);
    EnterCriticalSection(&g_entriesCs);
    size_t n = g_entries ? g_entries->size() : 0;
    LeaveCriticalSection(&g_entriesCs);
    LogMsg(L"Reminder explorer: loaded entries=%zu missing=%d", n, (int)miss);
    BarPositionWindow(g_barHwnd, true);
    EnsureTip(g_barHwnd);
    UpdateTip(g_barHwnd);
    {
        RECT wr{}; GetWindowRect(g_barHwnd, &wr);
        HMONITOR mon = MonitorFromWindow(g_barHwnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi{ sizeof(mi) };
        GetMonitorInfoW(mon, &mi);
        LogMsg(L"Reminder explorer: isWindow=%d visible=%d rect=(%d,%d,%d,%d) mon=(%d,%d,%d,%d) paints=%d",
            (int)(IsWindow(g_barHwnd) != FALSE), (int)(IsWindowVisible(g_barHwnd) != FALSE),
            wr.left, wr.top, wr.right, wr.bottom,
            mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom,
            g_barPaints);
    }
    Settings s; EnterCriticalSection(&g_settingsCs); s = g_settings; LeaveCriticalSection(&g_settingsCs);
    g_barTimer = SetTimer(g_barHwnd, 7, 2000, nullptr);
    g_barPollTimer = SetTimer(g_barHwnd, 8, (UINT)s.pollIntervalSec * 1000, nullptr);
    g_barFastTimer = 0;
    g_barFlashTimer = 0;
    if (g_barInFallback) {
        g_barFastTimer = SetTimer(g_barHwnd, 9, 500, nullptr);
        LogMsg(L"Reminder bar: fallback at startup, fast retry (500ms) armed");
    }
    // Prompt stop handling: wake on stop event or on any queued message.
    while (true) {
        DWORD r = MsgWaitForMultipleObjects(1, &g_barStopEvent, FALSE, INFINITE, QS_ALLINPUT);
        if (r == WAIT_OBJECT_0) break;
        MSG msg;
        bool quit = false;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { quit = true; break; }
            TranslateMessage(&msg); DispatchMessageW(&msg);
        }
        if (quit) break;
        if (r == WAIT_FAILED) break;
    }
    // Teardown on the owner thread only. Other threads must never touch these.
    if (g_barHwnd) {
        if (g_barTimer) { KillTimer(g_barHwnd, g_barTimer); g_barTimer = 0; }
        if (g_barPollTimer) { KillTimer(g_barHwnd, g_barPollTimer); g_barPollTimer = 0; }
        if (g_barFastTimer) { KillTimer(g_barHwnd, g_barFastTimer); g_barFastTimer = 0; }
        if (g_barFlashTimer) { KillTimer(g_barHwnd, g_barFlashTimer); g_barFlashTimer = 0; }
    } else {
        g_barTimer = 0; g_barPollTimer = 0; g_barFastTimer = 0; g_barFlashTimer = 0;
    }
    DestroyTip();
    if (g_barHwnd && IsWindow(g_barHwnd)) DestroyWindow(g_barHwnd);
    g_barHwnd = nullptr;
    if (g_barFont) { DeleteObject(g_barFont); g_barFont = nullptr; }
    g_barFontSpec.clear();
    UnregisterClassW(L"ReminderBarCls", GetModuleHandle(nullptr));
    return 0;
}
static bool Explorer_ModInit() {
    // Fast return: the UI thread creates and shows the window asynchronously.
    LoadSettings();
    Settings s0; EnterCriticalSection(&g_settingsCs); s0 = g_settings; LeaveCriticalSection(&g_settingsCs);
    LogMsg(L"Reminder explorer init: files=%zu show=%d count=%d anchor=%s tip=%d",
        s0.scheduleFiles.size(), (int)s0.taskbarShowNext, s0.taskbarCount,
        s0.taskbarAnchor.c_str(), s0.taskbarTooltipCount);
    g_barStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_barStopEvent) {
        LogMsg(L"Reminder explorer: CreateEventW failed err=%lu", GetLastError());
        return FALSE;
    }
    g_barThread = CreateThread(nullptr, 0, BarThreadProc, nullptr, 0, nullptr);
    if (!g_barThread) {
        LogMsg(L"Reminder explorer: CreateThread failed err=%lu", GetLastError());
        CloseHandle(g_barStopEvent); g_barStopEvent = nullptr;
        return FALSE;
    }
    LogMsg(L"Reminder explorer: ui thread started");
    return TRUE;
}
static void Explorer_ModSettingsChanged() {
    LoadSettings();
    bool miss = false; MaybeReload(true, miss);
    (void)miss;
    // Applied on the owner (UI) thread; just queue it and return fast.
    if (g_barHwnd && IsWindow(g_barHwnd))
        PostMessageW(g_barHwnd, WM_APP_BAR_UPDATE, 0, 0);
    // Toast lives on the worker thread in the same process when explorer owns it:
    // reposition/resize with the new font/margin/anchor immediately.
    if (g_toastHwnd && IsWindow(g_toastHwnd))
        PostMessageW(g_toastHwnd, WM_APP_REPOS, 0, 0);
}
static void Explorer_ModUninit() {
    // Signal only; destruction runs on the owner thread. Never DestroyWindow here.
    // Globals (g_barHwnd/timers) are cleared by BarThreadProc on exit; this thread
    // only resets them if the worker already exited, to avoid racing a live thread.
    if (g_barStopEvent) SetEvent(g_barStopEvent);
    if (g_barThreadId) PostThreadMessageW(g_barThreadId, WM_QUIT, 0, 0);
    bool exited = false;
    if (g_barThread) {
        exited = (WaitForSingleObject(g_barThread, 3000) == WAIT_OBJECT_0); // owner thread cleans up
        CloseHandle(g_barThread); g_barThread = nullptr;
    }
    if (g_barStopEvent) { CloseHandle(g_barStopEvent); g_barStopEvent = nullptr; }
    g_barThreadId = 0;
    if (exited) {
        g_barHwnd = nullptr;
        g_barTimer = 0; g_barPollTimer = 0; g_barFastTimer = 0; g_barFlashTimer = 0;
        g_barInFallback = false;
        g_barCrowded = false;
    }
    LogMsg(L"Reminder explorer stopped");
}

////////////////////////////////////////////////////////////////////////////////
// Process entry points.
// explorer.exe -> taskbar overlay window.
// Any other host (e.g. 32-bit windhawk.exe) -> schedule watcher + toast
// worker thread running in-process (no dedicated child process).

BOOL Wh_ModInit() {
    if (!g_settingsCsInit) {
        InitializeCriticalSection(&g_settingsCs);
        InitializeCriticalSection(&g_entriesCs);
        InitializeCriticalSection(&g_reloadCs);
        InitializeCriticalSection(&g_openCs);
        g_settingsCsInit = true;
    }
    // Opener may be left stopped by a previous Wh_ModUninit in this loaded
    // instance; re-arm it with a fresh generation so clicks work again.
    EnterCriticalSection(&g_openCs);
    g_openStop = false;
    g_openShutdown = false;
    InterlockedIncrement(&g_openGen);
    LeaveCriticalSection(&g_openCs);
    {
        WCHAR modPath[MAX_PATH]{};
        GetModuleFileNameW(nullptr, modPath, ARRAYSIZE(modPath));
        LogMsg(L"Reminder Wh_ModInit in: %s", modPath);
    }
    if (IsExplorerProcess()) {
        BOOL ok = Explorer_ModInit() ? TRUE : FALSE;
        LogMsg(L"Reminder Wh_ModInit explorer result=%d", (int)ok);
        if (ok) StartWorkerOnce(L"explorer"); // toast worker lives here too
        return ok;
    }
    // windhawk.exe side: only the interactive UI process runs the worker.
    // Service/session-0 instances are skipped (their windows would be invisible).
    // Explorer is the preferred owner: if explorer is already running, defer here
    // so the explorer process takes the singleton directly (no yield handoff needed).
    {
        DWORD sessionId = 0;
        if (ProcessIdToSessionId(GetCurrentProcessId(), &sessionId) && sessionId == 0) {
            LogMsg(L"Reminder Wh_ModInit: skip session-0 process");
            return TRUE;
        }
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        bool svc = false;
        if (argv) {
            for (int i = 1; i < argc; i++) {
                if (wcscmp(argv[i], L"-service") == 0 ||
                    wcscmp(argv[i], L"-service-start") == 0 ||
                    wcscmp(argv[i], L"-service-stop") == 0) { svc = true; break; }
            }
            LocalFree(argv);
        }
        if (svc) {
            LogMsg(L"Reminder Wh_ModInit: skip service process");
            return TRUE;
        }
        if (AnyExplorerRunning()) {
            LogMsg(L"Reminder Wh_ModInit: explorer running, defer worker to explorer");
            return TRUE;
        }
    }
    BOOL ok = StartWorkerOnce(L"windhawk") ? TRUE : FALSE;
    LogMsg(L"Reminder Wh_ModInit worker result=%d", (int)ok);
    return ok;
}

void Wh_ModAfterInit() {
}

void Wh_ModSettingsChanged() {
    if (IsExplorerProcess()) { Explorer_ModSettingsChanged(); return; }
    WhTool_ModSettingsChanged();
}

void Wh_ModUninit() {
    if (IsExplorerProcess()) { Explorer_ModUninit(); WhTool_ModUninit(); StopOpener(); return; }
    WhTool_ModUninit();
    StopOpener();
}
