# Audit Report — Crash / Drag / Styling Work (otensrc)

**Date:** 2026-10-04
**Scope:** Native mod-menu library (`src/main/jni/**`) — launch crash, broken menu
dragging, and UI styling alignment with `imgui-portfolio-8`.
**Deliverable:** `docs/CRASH_DRAG_STYLE_GUIDE.md` (full fix guide, on-device readable)
plus four applied source/build fixes.

---

## 1. Context

- **Current project:** otensrc (this repo) — AIDE Pro on-device NDK build,
  output `libv+++.so` (`LOCAL_MODULE`), injected into a Unity IL2CPP game via
  `System.loadLibrary` in the launcher/main-activity smali.
- **Reference project:** `imgui-portfolio-8` (cloned and analysed:
  `framework/app/gui.cpp`, `theme/colors.h`, `theme/layout.h`, `theme/ui_scale.h`,
  `helpers/window_drag.h`, `main.cpp`).
- **Name mapping established** (task names vs. actual files):
  `equinox_menu.h` → `src/main/jni/ImGui/ethnir_menu.h` · `ImGuiHook.cpp` →
  `src/main/jni/Main.cpp` (`hook_eglSwapBuffers`, `Init_Thread`) and
  `System/Hooks/Feats.h` (`InitializeAllHooks`) · `getBase` →
  `Tools::GetBaseAddress` · touch offsets → `Hacks/StructGame/Defines.h`
  (`api1..api31`).

## 2. Findings

### 2.1 Crash on launch

| # | Finding | Severity | Status |
|---|---|---|---|
| 1 | Stale hard-coded `libunity.so` offsets: `MemoryPatch` ×3 (`0x5755800`, `0x9FEC8AC`, `0x8D781DC`), `DobbyHook` ×2 (`0xC9B6F90`, `0xC9C33A4`), ~30 `HOOK_LIB` entries in `Feats.h`. `getAbsoluteAddress` = `base + offset` with no validation → trampoline writes into unmapped memory → `Fatal signal 11` after any game update. | Critical | **Fixed** (guarded) |
| 2 | `HOOK_LIB("libunity.so", "0xF0", GetAccDistance, …)` — offset `0xF0` is inside the ELF header, not executable code; hooking it corrupts the ELF header even on a matching build. | Critical | **Fixed** (commented out with re-enable note) |
| 3 | Touch input block dereferenced `Input_GetTouch(Config.ImGuiMenu.thiz, 0)` / `Input_get_mousePosition(thiz)` with no `thiz != 0` check → first-touch null deref when IL2CPP Input didn't resolve. | High | **Fixed** |
| 4 | `KittyMemory::ProtectAddr` called on a possibly-null `DobbySymbolResolver("libunity.so","eglSwapBuffers")` result. | Medium | **Fixed** (guarded + logged) |
| 5 | `Android.mk`: malformed `LOCAL_LDFLAGS += -Wl,--gc-sections,--strip-all, -llog` (trailing comma → empty `ld` arg) and `LOCAL_LDLIBS` assigned three times (last-wins). | Medium | **Fixed** |
| 6 | `build.gradle` lists `armeabi-v7a` but `Application.mk` and all prebuilt static libs are arm64-only — a PC/gradle build would fail; AIDE with `APP_ABI=arm64-v8a` is correct. | Low | **Documented** |

### 2.2 Menu not draggable

1. **Coordinate-space mismatch (root cause):** `io->MousePos` was fed in Unity
   Screen pixels while `io->DisplaySize` is the raw EGL surface. Where they
   differ, every hit-test on the header strip (`IsMouseHoveringRect`) failed →
   the drag never grabbed. The old `DragScaleX/Y` workaround corrected only
   deltas, never hover tests.
2. **Input timing:** touch state was written at the end of the draw pass (just
   before `EndFrame`), so `NewFrame` computed hover/clicks from a frame-old
   position; the drag threshold swallowed fast swipes.
3. **Unguarded `thiz`** (finding 2.1-3) also destabilised input.
4. The shell's `ImGuiWindowFlags_NoMove` + manual `SetWindowPos` drag design
   (`ethnir_menu.h` ~line 1225) is correct for a borderless touch overlay and
   was retained.

### 2.3 Styling gap vs `imgui-portfolio-8`

- Portfolio look: black-glass Night palette (`rgba(0,0,0,0.5)` panels), white
  text (muted @60 %), accent `#615DCE`, 1160×669 window, 243 px sidebar, 6 px
  boxes / 14 px popups, zero window rounding/borders/padding.
- otensrc shell uses an iOS-style palette (`#0A84FF` accent, white edges) and
  smaller geometry (880×520, 200 px sidebar, 22 px radius).
- Port delivered as exact line-referenced changes (`ethnir_menu.h` lines
  80–88, 91, 94, 114–131, ~333) plus a one-time global `ImGuiStyle` block for
  `Main.cpp` (~line 248/257). Applied as paste-in guide (visual tuning is
  iterative); not auto-applied.

## 3. Changes applied (this tree)

| File | Change |
|---|---|
| `src/main/jni/Main.cpp` | Touch input now feeds ImGui **before** `NewFrame` with Unity→GL coordinate conversion and `m_unity`/`thiz` guards; old end-of-frame input block removed; offset range-checks on 3 MemoryPatches + 2 DobbyHooks; `ProtectAddr` guarded with `eglSwapBuffers` address logged; `menuState.DragScaleX/Y` fixed at `1.0` (input pre-scaled). |
| `src/main/jni/Includes/Macros.h` | Added `IsOffsetInLibrary()` (parses `/proc/self/maps`) and `SAFE_HOOK_TARGET`; `HOOK_LIB` / `HOOK_LIB_NO_ORIG` now skip-and-log offsets outside the library mapping instead of hooking them. |
| `src/main/jni/System/Hooks/Feats.h` | Disabled the `0xF0` (ELF header) GetAccDistance hook with a re-enable note. |
| `src/main/jni/Android.mk` | Fixed malformed `LOCAL_LDFLAGS`; deduplicated `LOCAL_LDLIBS` to a single authoritative list (`-llog -landroid -lEGL -lGLESv3 -lGLESv2 -lGLESv1_CM -lz`). |
| `docs/CRASH_DRAG_STYLE_GUIDE.md` | Full 4-part fix guide (crash debugging incl. logcat recipes and crash-site timing table; drag explanation + exact fix; portfolio styling port with exact line numbers; AIDE Pro rebuild/inject checklist). |

Net diff at time of audit: 4 source/build files, +108/−57, plus two new docs.

## 4. Verification performed

- Every edit reviewed via `git diff` after application; final state re-checked
  after each subsequent change.
- Consistency checks: old end-of-frame input block fully removed (remaining
  `Input_get_touchCount` references are the new block and the pre-existing
  collapsed-pill lambda); `HOOK_LIB` expands only in `Feats.h`, whose sole
  include chain is `Main.cpp` where `Logger.h` (`LOGE`) and `Utils.h`
  (`string2Offset`, `getAbsoluteAddress`) are included first; `Macros.h` is
  included only by `Main.cpp` (no multi-TU issues).
- Guide line references re-validated against the current sources
  (`ethnir_menu.h` constants at 80–88, palette at 114–131, drag block at ~1225;
  `Main.cpp` init block at ~248–257).
- **Not verified:** compilation. No Android NDK/SDK exists in this environment
  (`docs/AIDE_PRO_BUILD.md`); the AIDE Pro clean build
  (`rm -rf build src/main/obj src/main/libs` → Gradle build) is the required
  gate. Runtime behaviour (drag, guards, offset validity) must be confirmed
  on-device via the logcat audit trail described in the guide.

## 5. Open items / recommendations

1. Build on-device and capture the init audit trail (`libunity.so: 0x…`,
   `eglSwapBuffers: 0x…`, any `HOOK skip …` lines).
2. Re-dump offsets for the current game build and update `Feats.h` +
   `api1..api31` in `Defines.h` — stale offsets now degrade to disabled
   features instead of crashing.
3. Re-enable `GetAccDistance` once its real offset is found.
4. Apply the Part 3 styling edits (or the deeper sidebar rewrite: Lucide-style
   icon column, gradient tab fill, avatar block) when ready for visual tuning.
5. Optional: rename module to `libMyMenu.so` only together with
   `MainActivity.java` `libname`/`downloadurl` updates.

## 6. Sign-off

Work completed as requested: root causes for the launch crash and the
non-draggable menu identified and fixed in-tree; styling port documented with
exact integration points; complete guide delivered at
`docs/CRASH_DRAG_STYLE_GUIDE.md`. Remaining risk is limited to on-device
compile/runtime verification, which this environment cannot perform.
