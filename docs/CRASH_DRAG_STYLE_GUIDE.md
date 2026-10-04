# otensrc — Crash / Drag / Styling Fix Guide (imgui-portfolio-8 port)

> Repo mapping (the names in the task don't exist under those names):
>
> | Task name | Actual file |
> |---|---|
> | `equinox_menu.h` | `src/main/jni/ImGui/ethnir_menu.h` (`Eq*` = equinox lineage) |
> | `ImGuiHook.cpp` | `src/main/jni/Main.cpp` (`hook_eglSwapBuffers`, `Init_Thread`, xhook registration) |
> | hook list | `src/main/jni/System/Hooks/Feats.h` (`InitializeAllHooks`) |
> | `getBase` | `Tools::GetBaseAddress` (`Init_Thread` / `Init_Thread2`) |
> | build config | `src/main/jni/Android.mk`, `src/main/jni/Application.mk` |
>
> Status: the Part 1 + Part 2 fixes are **already applied** in this tree
> (`Main.cpp`, `Includes/Macros.h`, `System/Hooks/Feats.h`, `Android.mk`).
> The Part 3 styling is a paste-in guide. No NDK exists in the sandbox, so
> everything was diff-reviewed, not compiled — the AIDE Pro build is the test.

---

## PART 1 — CRASH DEBUGGING

### 1.1 Ranked causes found in the source

**#1 — Stale hard-coded offsets → `SIGSEGV` in `Init_Thread` (most likely).**
`Init_Thread` (Main.cpp) runs the instant the lib loads and immediately calls:

- `MemoryPatch::createWithHex("libunity.so", 0x5755800, ...)` / `0x9FEC8AC` / `0x8D781DC`
- `DobbyHook(getAbsoluteAddress("libunity.so", 0xC9B6F90), ...)` and `0xC9C33A4`
- `InitializeAllHooks()` → ~30 more `HOOK_LIB` offsets in `Feats.h`

`getAbsoluteAddress` (Utils.h) returns `base + offset` with **zero validation**.
After one game update those addresses point into unmapped memory and Dobby
writes a trampoline there → `Fatal signal 11 (SIGSEGV)`. Worse, `Feats.h` had
`HOOK_LIB("libunity.so", "0xF0", GetAccDistance, ...)` — `0xF0` is inside the
**ELF header**, not executable code; hooking it corrupts the ELF header even on
the "matching" build.

**Fix applied:**
- `Includes/Macros.h` gained `IsOffsetInLibrary(lib, offset)` (parses
  `/proc/self/maps` for the lib's mapping range) and `HOOK_LIB` /
  `HOOK_LIB_NO_ORIG` now resolve the target only when the offset is inside the
  mapping, otherwise log `HOOK skip <lib>+<offset>` and skip.
- `Main.cpp::Init_Thread` range-checks all three MemoryPatch offsets and both
  DobbyHook offsets before using them.
- `Feats.h`: the `0xF0` GetAccDistance hook is commented out with a note.

A game update now degrades to "features off + skip logs" instead of a crash.

**#2 — Touch crash.** The old end-of-frame input block called
`Input_GetTouch(Config.ImGuiMenu.thiz, 0)` and
`Input_get_mousePosition(Config.ImGuiMenu.thiz)` **without checking
`thiz != 0`**. If the IL2CPP Input methods didn't resolve (`thiz == 0`), the
first touch dereferenced a null `this` → SIGSEGV. The collapsed-pill lambda
checked; the main block didn't. Fixed by the Part 2 rewrite (guards included).

**#3 — `KittyMemory::ProtectAddr` on a null symbol.** `DobbySymbolResolver`
often returns 0 for `eglSwapBuffers` on modern Unity (not exported); the old
code passed that straight to `mprotect`. Now guarded + logged
(`eglSwapBuffers: 0x...`).

**#4 — Build configuration** (see 1.4 for the corrected snippets):

- `LOCAL_LDFLAGS += -Wl,--gc-sections,--strip-all, -llog` — trailing comma
  passed an empty argument to `ld`.
- `LOCAL_LDLIBS` was assigned **three times**; only the last one survived
  (silently). Cleaned to one authoritative list.
- `Application.mk` is `arm64-v8a`-only but `build.gradle` has
  `abiFilters 'arm64-v8a','armeabi-v7a'`; every prebuilt static lib
  (curl/openssl/dobby/foxcheats/ctorHook) is arm64-only. Always build with
  `APP_ABI=arm64-v8a` (AIDE Gradle build honours the mk).
- `MainActivity.java` loads `libname = "libv+++.so"` from `downloadurl`. If you
  rename the module to `libMyMenu.so`, update **both** `libname` and the URL.
  `System.load` is already in a try/catch (toast + `LoadLibrary` log instead of
  a hard kill).

### 1.2 logcat filtering (MT Manager terminal)

```sh
# wipe, reproduce, capture everything relevant
logcat -c
logcat | grep -E "Fatal signal|SIGSEGV|SIGABRT|SIGBUS|backtrace|libc|AndroidRuntime|ETHNIR|LoadLibrary|HOOK skip|eglSwapBuffers"

# after the crash, the crash buffer alone is usually enough:
logcat -b crash -d

# scope to the game process only:
logcat --pid=$(pidof com.tencent.tmgp.cod) -v threadtime

# root devices: full tombstone with backtrace
ls /data/tombstones/
```

MT Manager also has a built-in logcat viewer — filter it on `Fatal signal`
and `DEBUG` (the debuggerd backtrace tag).

### 1.3 Locating the crash: timing + backtrace

| Symptom | Crash site |
|---|---|
| Dies instantly at `System.loadLibrary`, `UnsatisfiedLinkError` under `AndroidRuntime`/`LoadLibrary` | wrong-ABI or corrupt .so — never reaches native init |
| Dies instantly, no Java error | `__attribute__((constructor))` / `JNI_OnLoad` (yours are safe — threads only; `native_Init` is an empty body) |
| Game loads, dies ~1–3 s later, faulting frame in `libv+++.so` or Dobby | **`Init_Thread`** — stale-offset DobbyHook / MemoryPatch / `HOOK_LIB`. New build prints the audit trail first: `libunity.so: 0x...`, `eglSwapBuffers: 0x...`, `HOOK skip ...` |
| Menu starts drawing, then dies before any touch | **`hook_eglSwapBuffers`** — ImGui init / font build / `DrawESP` |
| Dies only when you touch the screen | old input block (`Input_GetTouch(0, 0)`) — now guarded |
| Never crashes, just hangs at launch | `Init_Thread`/`Init_Thread2` spin-forever loops waiting for `libunity.so` / `libanogs.so` (`getBase` — these can't crash, they only sleep) |

In the tombstone/backtrace, one thing matters: **which library owns the
faulting `pc`**:

- `libv+++.so` → your code (symbolize the offset later with ndk `addr2line`)
- `libdobby` → a hook target was garbage
- `libunity.so` / `libil2cpp.so` → a trampoline landed on a wrong offset and
  the game called it
- `libEGL.so` / GPU driver → rendering-hook problem

If you need finer granularity, add breadcrumbs around `Init_Thread` steps:

```cpp
LOGI("init: offsets checked");
InitializeAllHooks();
LOGI("init: all hooks done");
xhook_register(...);
LOGI("init: xhook registered");
```

The last breadcrumb printed before `Fatal signal` marks the failing step.

### 1.4 Corrected Android.mk / Application.mk

The repo file is already fixed; the corrected section of the shared-library
module in `Android.mk` now reads:

```make
LOCAL_LDFLAGS          += -Wl,--gc-sections,--strip-all
LOCAL_ARM_MODE         := arm
# single authoritative list; -lEGL/-lGLES* are what the swap-buffer hook renders with
LOCAL_LDLIBS           := -llog -landroid -lEGL -lGLESv3 -lGLESv2 -lGLESv1_CM -lz
# ... (wildcard FILE_LIST for ImGui/IL2Cpp/KittyMemory/System/Texture/root .cpp) ...
LOCAL_STATIC_LIBRARIES := libcurl libssl libcrypto libdobby libfoxcheats libxhook libctorHook
include $(BUILD_SHARED_LIBRARY)
```

`Application.mk` needs no change:

```make
APP_ABI             := arm64-v8a
APP_PLATFORM        := android-21
APP_STL             := c++_static
APP_OPTIM           := release
APP_THIN_ARCHIVE    := true
APP_PIE             := true
```

Never delete `-lEGL -lGLESv3 -lGLESv2` — `hook_eglSwapBuffers` renders through
`ImGui_ImplOpenGL3` (`#version 300 es`) and needs them at link time.

---

## PART 2 — FIXING THE DRAG ISSUE

### 2.1 Why an ImGui window stops being draggable in a game overlay

1. **`ImGuiWindowFlags_NoMove` on the window.** The ethnir shell *intentionally*
   uses `NoMove` (ethnir_menu.h ~line 1105, the `Begin("##ethnir_shell", ...)`
   call) because it implements its own drag: press on the header strip →
   `st.DragRef = io.MousePos` → `SetWindowPos` by the delta (ethnir_menu.h
   line ~1225, `// ---- window dragging`). That is the correct pattern for a
   borderless touch overlay — ImGui's native move wants a real title bar. So
   `NoMove` itself is not the bug; losing the *manual* drag path is.
2. **Input fed at the wrong time.** The touch block ran at the END of the draw
   pass (after every widget, right before `EndFrame`). `NewFrame` on the next
   frame then computed hover/clicks from a frame-old position and
   `io.MouseDragThreshold` ate fast swipes — the window wouldn't grab.
3. **Coordinate-space mismatch (the big one).** `io->MousePos` was written in
   **Unity Screen pixels** (`get_width()/get_height()` space, from
   `Class_Screen_get_width/height` = `m_unity + api4/api5`) while
   `io->DisplaySize` is the **raw EGL surface** (`g_GlWidth/g_GlHeight`). On any
   device where the two differ (render scaling, cutouts), `IsMouseHoveringRect`
   on the header strip fails → drag never grabs and taps land in the wrong
   place. The old `DragScaleX/Y` hack corrected only the *delta*, never the
   hover tests.
4. **Input events not forwarded at all.** When `Config.ImGuiMenu.thiz == 0`
   (IL2CPP Input not resolved) the old code either crashed (cause #2 above) or
   fed garbage; either way `io.MouseDown[0]` never became usable.
5. **Touch consumption.** ImGui has no way to "consume" touch back from the
   game; the game keeps receiving every event. Buttons in the menu that also
   fire game actions are normal for this injection style — not a drag factor.

### 2.2 How input actually reaches ImGui in otensrc (no AInputEvent)

There is **no `AInputEvent` handler** in this project — no
`ImGui_ImplAndroid_NewFrame`, no input-hook. Touch is read by calling the
game's own Unity Input class through IL2CPP entry points computed as
`m_unity + apiN` (`Hacks/StructGame/Defines.h`):

```cpp
#define Class_Input_get_touchCount    (m_unity + api1)
#define Class_Input_GetTouch          (m_unity + api2)
#define Class_Input_get_mousePosition (m_unity + api3)
#define Class_Screen_get_width        (m_unity + api4)
#define Class_Screen_get_height       (m_unity + api5)
```

`hook_eglSwapBuffers` polls these once per frame and writes them into
`ImGui::GetIO()`. That is the equivalent of the "ImGuiHook input" you were
looking for.

### 2.3 The exact fix (applied in Main.cpp, inside `hook_eglSwapBuffers`)

The old end-of-frame block was deleted and replaced by this block placed
**before** `ImGui_ImplOpenGL3_NewFrame(); ImGui::NewFrame();`:

```cpp
// ---- touch -> ImGui, BEFORE NewFrame ----
io->MouseWheel = 0.0f;
io->MouseWheelH = 0.0f;
io->MouseDown[0] = false;
if (m_unity != 0 && Config.ImGuiMenu.thiz != 0)                 // crash guards
{
    const int uniW = get_width(), uniH = get_height();
    const float sx = (uniW > 0) ? (float)g_GlWidth  / (float)uniW : 1.0f;
    const float sy = (uniH > 0) ? (float)g_GlHeight / (float)uniH : 1.0f;
    auto Input_get_touchCount    = (int (*)())(Class_Input_get_touchCount);
    auto Input_get_mousePosition = (Vector3(*)(uintptr_t))(Class_Input_get_mousePosition);
    if (Input_get_touchCount() > 0)
    {
        Vector3 touchPos = Input_get_mousePosition(Config.ImGuiMenu.thiz);
        io->MousePos = ImVec2(touchPos.x * sx, g_GlHeight - touchPos.y * sy);  // Unity px -> GL px
        auto Input_GetTouch = (Touch(*)(uintptr_t, int))(Class_Input_GetTouch);
        switch (Input_GetTouch(Config.ImGuiMenu.thiz, 0).m_Phase)
        {
            case TouchPhase::Began:
            case TouchPhase::Stationary:
            case TouchPhase::Moved:
                io->MouseDown[0] = true;
                break;
            case TouchPhase::Ended:
            case TouchPhase::Canceled:
            default:
                io->MouseDown[0] = false;
                break;
        }
    }
}
ImGui_ImplOpenGL3_NewFrame();
ImGui::NewFrame();
```

And the drag rescale is neutralised in the menu wiring (input is pre-scaled, so
a second scale would double-apply):

```cpp
// was: menuState.DragScaleX = (float)get_width() / (float)g_GlWidth; ...
menuState.DragScaleX = 1.0f;
menuState.DragScaleY = 1.0f;
```

Window flags in the draw call need **no change**: `MouseDown[0]` is now set
correctly, `NoMove` stays (manual drag), and `io.MouseDragThreshold = 2.f` is
already tuned for touch. After this, every hit-test (`IsMouseHoveringRect`,
`EqPress`, sidebar taps, sliders) lives in one coordinate space.

If it is still un-grabbable: the header strip marks real controls via
`st.HeaderControl` (save pill, search, gear, minimise). Drag from the wordmark
area ("ETHNIR", top-left) to confirm the drag path itself works.

---

## PART 3 — STYLING PORT FROM imgui-portfolio-8

The portfolio's look lives in three files of the reference repo:

- `framework/app/gui.cpp → apply_style()` — global style: zero window
  rounding/borders/padding, transparent window/child backgrounds, anti-aliasing on.
- `framework/theme/colors.h` — "Night" palette: pure black glass
  `rgba(0,0,0,0.5)` panels, white text, muted = white @60 %, accent
  `#615DCE` (periwinkle), borders are **black** `rgba(0,0,0,0.25)`.
- `framework/theme/layout.h` — 1160×669 window, 243 px sidebar, 51 px tabs,
  6 px box rounding, 14 px popup rounding, 42×22 toggles, 24 px controls.

Fonts there are MuseoSansEx + Lucide. Your `inter_semibold` + merged
FontAwesome is the closest already-in-tree pair — keep them; the look comes
from palette + geometry, not the font.

### 3.1 Global ImGuiStyle — add once, in Main.cpp

Insert inside the `if (!g_App)` init block (Main.cpp line ~248), right after
`ImGui_ImplOpenGL3_Init("#version 300 es")` (line ~257):

```cpp
{
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding   = 0.f;   s.WindowBorderSize = 0.f;
    s.WindowPadding    = ImVec2(0, 0);
    s.ChildBorderSize  = 0.f;   s.FrameBorderSize  = 0.f;
    s.AntiAliasedLines = true;  s.AntiAliasedFill   = true;
    s.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_ChildBg]  = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_Text]     = ImVec4(1, 1, 1, 1);
    s.Colors[ImGuiCol_PopupBg]  = ImVec4(0, 0, 0, 0.70f);
}
```

(The reference also zeroes `ScrollbarSize`; your shell uses a custom 4 px
scrollbar in the content child — keep it.)

### 3.2 Exact lines to change in `ethnir_menu.h`

**Geometry constants — lines 80–88:**

| Line | Was | Portfolio value |
|---|---|---|
| 80 `kWinW` | `880.0f` | `1160.0f` (auto-clamps to screen in EqRender) |
| 81 `kWinH` | `520.0f` | `669.0f` |
| 84 `kSideW` | `200.0f` | `243.0f` |
| 86 `kRadius` | `22.0f` | `14.0f` |
| 87 `kRowH` | `30.0f` | `37.0f` |
| 88 `kSliderH` | `32.0f` | `32.0f` (keep; matches control_h + padding) |

**Switch green — line 91:**

```cpp
constexpr ImVec4 kSwitchOn(0.35f, 0.78f, 0.48f, 1.0f);   // reference success #5AC77A-ish
```

**Accent — line 94, first entry of `kAccents[]`:** change the Blue hue from
`0.5833f` (iOS `#0A84FF`) to `0.6726f` (= `#615DCE`, the portfolio's periwinkle
accent; `ApplyAccentFromHue()` derives saturation itself).

**Night palette — `EqPalDark()`, lines 114–131.** Replace the body:

```cpp
inline Palette EqPalDark()
{
    Palette p;
    p.base      = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);   // was (0.035,0.040,0.055,0.88) — pure black glass
    p.scrim     = ImVec4(0.0f, 0.0f, 0.0f, 0.25f);
    p.text      = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    p.textDim   = ImVec4(1.0f, 1.0f, 1.0f, 0.60f);   // muted = white @60%, not grey
    p.textFaint = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);   // header_text is 26%; 45% reads better at row size
    p.cardBg    = ImVec4(0.0f, 0.0f, 0.0f, 0.40f);   // content boxes: black @40%
    p.cardEdge  = ImVec4(0.0f, 0.0f, 0.0f, 0.25f);   // reference borders are black, not white
    p.glassRim  = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    p.hover     = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
    p.side      = ImVec4(0.0f, 0.0f, 0.0f, 0.40f);
    p.sideEdge  = ImVec4(1.0f, 1.0f, 1.0f, 0.17f);   // sidebar_sep
    p.switchOff = ImVec4(0.10f, 0.10f, 0.10f, 1.0f); // toggle_off #1A1A1A
    p.track     = ImVec4(0.17f, 0.17f, 0.17f, 1.0f); // slider_bg  #2B2B2B
    p.popupBg   = ImVec4(0.0f, 0.0f, 0.0f, 0.70f);   // dropdown bg
    p.sep       = ImVec4(0.0f, 0.0f, 0.0f, 0.25f);
    return p;
}
```

**Card padding** — in `BeginGroupCard` (~line 333): `WindowPadding`
`ImVec2(10, 9)` → `ImVec2(14, 8)` and `ChildRounding` `14.0f` → `6.0f`
(portfolio `box_round` is 6).

### 3.3 What this gets you — and what it doesn't

The palette, window geometry, rounding, and widget feel land at ~90 % of the
reference with ~30 changed lines. What stays different without a sidebar
rewrite: the reference's Lucide icon column, gradient-fill selected tab,
user-avatar block and config cards. The shell's structure (child panes +
`EqPress` release-commit rows) already matches, so those are additive
tweaks if you want to go further.

Also note: `c::ApplyMainWindowStyle(...)` still runs in Main.cpp before
`EqRender`. The shell pushes its own style per window so it mostly overrides
it, but if stray old-style widgets appear, neutralise that call second.

---

## PART 4 — FINAL BUILD & INJECT CHECKLIST

1. **Clean the stale build outputs** (they ship in the repo and will silently
   relink old objects):
   ```sh
   rm -rf build src/main/obj src/main/libs
   ```
2. **Build in AIDE Pro** (Gradle build, arm64) → verify a fresh
   `src/main/libs/arm64-v8a/libv+++.so` (or `libMyMenu.so`) exists — check the
   timestamp.
3. **Inject** via MT Manager: drop the .so into the game's `lib/arm64-v8a/`,
   add `System.loadLibrary("v+++")` (or `"MyMenu"`) to the game's main activity
   smali. (The standalone launcher in this repo downloads + loads via
   `MainActivity`; for injecting into the *game* APK the smali path is what
   matters.)
4. **Re-sign** the APK (MT Manager zipalign + sign), install.
5. **Start a logcat session before launching** (Part 1.2 command).
6. **Launch and read the init audit trail:** expect
   `libunity.so: 0x...`, `eglSwapBuffers: 0x...`, and (only for bad offsets)
   `HOOK skip ...` lines.
7. **Test drag first** — grab the empty header strip next to the wordmark —
   then tabs, toggles, sliders.
8. **If stable but features dead** → offsets are stale for your game build;
   the skip-logs name exactly which ones. Re-dump and update
   `Feats.h` + `api1..api31` in `Defines.h`.
9. **If it still crashes** → `logcat -b crash -d`, match the faulting library
   against the Part 1.3 timing table.

---

## Applied-fix summary (this tree)

| File | Change |
|---|---|
| `src/main/jni/Main.cpp` | input block moved before `NewFrame` with Unity→GL conversion + `m_unity`/`thiz` guards; old end-of-frame block removed; offset range-checks on 3 MemoryPatches + 2 DobbyHooks; `ProtectAddr` guarded + `eglSwapBuffers` logged; `DragScaleX/Y = 1.0` |
| `src/main/jni/Includes/Macros.h` | `IsOffsetInLibrary()` + `SAFE_HOOK_TARGET`; `HOOK_LIB`/`HOOK_LIB_NO_ORIG` skip-and-log invalid offsets |
| `src/main/jni/System/Hooks/Feats.h` | `0xF0` (ELF header) GetAccDistance hook disabled with re-enable note |
| `src/main/jni/Android.mk` | malformed `-Wl,..., -llog` fixed; triple `LOCAL_LDLIBS` deduplicated |
