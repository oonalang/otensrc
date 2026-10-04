# Session Audit Report — otensrc Mod Menu

**Date:** 2026-10-04
**Scope of session:** launch-crash diagnosis, menu drag repair, portfolio styling
port, documentation, and a comment-cleanup pass.
**Companion docs:** `docs/CRASH_DRAG_STYLE_GUIDE.md` (full 4-part fix guide),
`docs/AUDIT_REPORT.md` (detailed crash/drag/styling audit). This file is the
session-level summary.

---

## 1. Analysis performed

- **Current project (`otensrc`, this repo):** Android/Gradle + `ndk-build` mod
  menu (`src/main/jni/**`), built on-device with AIDE Pro, injected into a Unity
  IL2CPP game. Established the real file names behind the task's names:
  `equinox_menu.h` → `ImGui/ethnir_menu.h`; `ImGuiHook.cpp` → `Main.cpp`
  (`hook_eglSwapBuffers`, `Init_Thread`) + `System/Hooks/Feats.h`
  (`InitializeAllHooks`); `getBase` → `Tools::GetBaseAddress`; touch/IL2CPP
  offsets → `Hacks/StructGame/Defines.h` (`api1..api31`).
- **Reference project:** `imgui-portfolio-8` cloned and analysed —
  `framework/app/gui.cpp` (`apply_style`), `theme/colors.h`, `theme/layout.h`,
  `theme/ui_scale.h`, `helpers/window_drag.h`, `main.cpp`.

## 2. Code changes applied

| File | Change | Commit |
|---|---|---|
| `src/main/jni/Main.cpp` | Touch input now feeds ImGui **before** `NewFrame`, converted from Unity pixels to EGL-space with `m_unity`/`thiz` guards; old end-of-frame input block removed; offset range-checks on 3 `MemoryPatch` + 2 `DobbyHook` targets; `ProtectAddr` guarded with the resolved `eglSwapBuffers` address logged; `menuState.DragScaleX/Y` pinned to `1.0`. | `a429a85` |
| `src/main/jni/Includes/Macros.h` | Added `IsOffsetInLibrary()` (parses `/proc/self/maps`) and `SAFE_HOOK_TARGET`; `HOOK_LIB` / `HOOK_LIB_NO_ORIG` skip-and-log offsets outside the library mapping instead of hooking unmapped memory. | `a429a85` |
| `src/main/jni/System/Hooks/Feats.h` | Disabled the `HOOK_LIB("libunity.so", "0xF0", …)` call — offset `0xF0` is in the ELF header, not code — with a re-enable note. | `a429a85` |
| `src/main/jni/Android.mk` | Fixed malformed `LOCAL_LDFLAGS` (trailing comma → empty `ld` arg); deduplicated the triple `LOCAL_LDLIBS` to one authoritative list including `-lEGL -lGLESv3 -lGLESv2`. | `a429a85` |

**Root causes addressed:** (1) stale version-locked `libunity.so` offsets hooked
into unmapped memory → `SIGSEGV` at `Init_Thread`; (2) unguarded
`Config.ImGuiMenu.thiz` → null-deref on first touch; (3) `ProtectAddr` on a null
symbol result; (4) drag broken by feeding input after the draw pass and by the
Unity-pixel vs EGL-surface coordinate mismatch; (5) build link-flag defects.

## 3. Documentation delivered

| File | Contents | Commit |
|---|---|---|
| `docs/CRASH_DRAG_STYLE_GUIDE.md` | 4-part guide: crash causes + logcat recipes + crash-site timing table + corrected `Android.mk`/`Application.mk`; drag root-cause analysis + exact fix; portfolio styling port with exact `ethnir_menu.h` line numbers; AIDE Pro rebuild/inject checklist. | `a429a85` |
| `docs/AUDIT_REPORT.md` | Detailed crash/drag/styling audit: findings tables with severities, per-file change list, verification performed, open items. | `a429a85` |
| `audit_report.md` (this file) | Session-level summary incl. the comment-cleanup pass. | this commit |

## 4. Comment cleanup pass

Scope was deliberately limited to **comments added during this session** — not
vendored code (`ImGui/`, `libzip/`, `Substrate/`, `curl/openssl/`, `Dobby`,
`KittyMemory`, `foxcheats`) and not load-bearing comments in existing sources
(e.g. the "Eq\* prefix is mandatory… NDK build fails" note, atomic config-write
note, `InputActive` debounce note), which encode constraints the code does not
imply.

- `Main.cpp`: removed the 7-line input-timing banner/essay (restated code and
  narrated past bugs).
- `Macros.h`: collapsed the 4-line `IsOffsetInLibrary` essay to one line; removed
  the redundant `SAFE_HOOK_TARGET` comment.
- `Feats.h`: collapsed the 3-line `0xF0` note to one line.
- `Main.cpp`: shortened the `DragScale` and version-locked-offset comments to
  one line each (both kept — they encode non-obvious invariants).

**Result:** the 21 comment lines this session added were reduced to 4, each
carrying a constraint the code does not imply. Committed as `646be46`
("Refactor: Remove useless comments", 3 files, +4/−20), followed by `3bb1b5e`
adding `.claude/` to `.gitignore` so local agent tooling keeps the tree clean.

**Verification of that pass:** full `git diff` review; a filtered scan for
non-comment diff lines returned empty, proving no code statement changed;
`git diff --check` clean. One intermediate edit accidentally joined two
statements onto one line — detected by that scan, repaired, and re-verified
before committing.

## 5. Verification summary

- Every edit reviewed via `git diff` after application; final states re-checked
  after subsequent changes.
- Consistency checks: old end-of-frame input block fully removed; `HOOK_LIB`
  expands only in `Feats.h` under `Main.cpp`'s include order (`Logger.h` for
  `LOGE`, `Utils.h` for `string2Offset`/`getAbsoluteAddress`); `Macros.h`
  included only by `Main.cpp`.
- Guide line references re-validated against current sources.
- Commits pushed and verified against `origin/main`; working tree clean.
- **Not verified:** compilation — no Android NDK/SDK in this environment
  (`docs/AIDE_PRO_BUILD.md`); the AIDE Pro clean build is the required gate.

## 6. Open items

1. Clean build on-device (`rm -rf build src/main/obj src/main/libs` → AIDE
   Gradle build) and capture the init audit trail (`libunity.so: 0x…`,
   `eglSwapBuffers: 0x…`, any `HOOK skip …`).
2. Re-dump offsets for the current game build → update `Feats.h` and
   `api1..api31` in `Defines.h`.
3. Re-enable `GetAccDistance` once its real offset is found.
4. Apply the Part 3 styling edits (palette/geometry/accent) from the guide when
   ready for visual tuning.
5. Rename the module to `libMyMenu.so` only together with `MainActivity.java`
   `libname`/`downloadurl` updates.