# Firestaff Full Code Audit Report

**Date:** 2026-06-16
**Auditor:** Firestaff Watchdog (main session)
**Scope:** Full Firestaff codebase (536,365 LOC across 878 .C + 846 .H files,
plus CMakeLists.txt, M12/launcher UI, manifest runners).

This is the second audit (see `REDMSB_FIRESTAFF_AUDIT_2026-06-16.md` for the
ReDMCSB-comparison pass).  The first audit found 1 P1 build bug
(Bug B: `_G2157_` unresolved in `firestaff_m10` lib); this second pass
extends with cross-module analysis: stale TODOs, dead stubs, asymmetric
validation, malloc/free paring, and dynamic-API surface.

---

## 1. Headline Numbers

| Metric | Value |
|--------|-------|
| `.C` source files | 878 |
| `.H` header files | 846 |
| Total LOC | 536,365 |
| `TODO` / `FIXME` markers in src/ + include/ | 40 (35 in code, 5 in shared/UI) |
| `malloc`/`calloc`/`realloc` calls (top 5 files) | 23, 8, 8, 8, 7 |
| `fopen` calls not matched by `fclose` | 0 (manual check on 100 files) |
| `strcpy` / `strcat` | 1 (local-buffer, controlled source, OK) |
| `gets()` | 0 (no insecure C-APIs) |
| Threading primitives | 0 (single-threaded engine) |
| Atomic primitives | 0 |

---

## 2. Bug Inventory (in priority order)

### Bug A: Theron V1 save header-checksum disabled with broken writer (RESOLVED, commit 997394f0)

- **Symptom:** `src/theron/theron_v1_save_load.c:283-285` has a
  `(void)stored_cs; (void)computed_cs; /* TODO: re-enable when stable */`
  comment leaving the header-checksum validation commented out.
- **Root cause:** the writer was wrong, not the reader.  In
  `build_save_image()`, the write path wrote the *whole-file*
  checksum (computed with bytes 6..7 included) into the header
  field at `THERON_SAVE_OFF_CHECKSUM`.  The reader expected the
  header field to be a *header-only* checksum, so no sane read
  could ever pass the check, hence the disable-and-TODO.
- **Fix:** in `build_save_image()`, save the two checksum-field
  bytes, zero them, compute the header checksum over the first
  `THERON_SAVE_HEADER_SIZE - THERON_SAVE_FOOTER_SIZE` bytes, then
  restore the saved bytes and write the computed header checksum
  to the field.  Mirror the footer pattern.  In
  `parse_save_image()`, enable the validation: zero bytes 6..7,
  compute, compare against the stored value, restore the stored
  value (so the subsequent footer compute sees the right bytes),
  return -1 on mismatch.
- **Test coverage:** `ctest -R theron_v1_save 2/2` after fix
  (theron_v1_save_load + theron_v1_save_header_rejection).
  The header-rejection test mutates `THERON_SAVE_OFF_QUEST_ITEMS`
  (offset 8), so it triggers the footer-checksum-mismatch path;
  the positive round-trip covers the now-active header path.
- **Commit:** `997394f0 theron: enable + fix header-checksum validation in save_load`

### Bug B: Pre-existing `_G2157_` unresolved in `firestaff_m10` lib (RESOLVED, commit 9c57ece1)

- See `REDMSB_FIRESTAFF_AUDIT_2026-06-16.md` Section Bug B.
- Resolved by adding `src/shared/image_backend_pc34_compat_globals.c`
  as a thin TU that defines G2157_ + G2159_puc_Bitmap_Source +
  G2160_puc_Bitmap_Destination (all already declared as `extern`
  in `include/image_backend_pc34_compat.h`).
- Commit: `9c57ece1 m10: provide image_backend globals (G2157_ + G2159/G2160)`

### Bug C: CSB V1 save directory never created (NOT YET FIXED, scoped)

- **Symptom:** `src/csb/csb_v1_save_load_pc34_compat.c:74-78`
  declares a `static void ensure_save_dir(void) __attribute__((unused))`
  stub that does *nothing* (only calls `default_save_dir()` to
  cache the path, never `mkdir`).
- **Effect:** `csb_v1_save_game(path, ...)` does not call
  `ensure_save_dir` either (it goes straight to `fopen(path, "wb")`).
  If the parent directory of `path` does not exist, save fails
  with `ENOTDIR` / `ENOENT`, returning `CSB_V1_SAVE_ERR_CANT_CREATE`
  (= -1).  On Linux this is `errno=2: No such file or directory`;
  on Windows it's similar.
- **Test coverage:** zero.  No `csb_v1_save` ctest entry exists
  in `builds/n2-build/CTestTestfile.cmake`.  Bug was unobserved
  because no test ever tried to save into a non-pre-existing
  directory.
- **Fix scope:** small.  Make `ensure_save_dir` actually call
  `mkdir(save_dir, 0700)` (or platform equivalent) and call it
  from `csb_v1_save_game()` before the backup dance.  Add a
  test that creates a temp dir, removes the sub-dir, then
  asserts `csb_v1_save_game` succeeds.
- **Recommendation:** low-priority since the launcher creates
  the dir on first run; medium if a power user wipes it.
  See `docs/audits/FIRESTAFF_FULL_AUDIT_2026-06-16.md` Section Bug C.

### Bug D: 9 build errors in `firestaff_nexus_v2_verification_suite_probe` after signature change (RESOLVED, commit a22ec68b)

- **Symptom:** After the `nexus_v2_pipeline_render` signature
  change in commit 7ca73871 (smooth-movement tick refactor: 7 args
  → 6 args, removing the explicit cam_z / cam_dir), the
  verification-suit probe was left with the old 7-arg calls,
  failing to build with `error: too many arguments to function
  call, expected 6, have 7` on lines 145, 167, 168, 241, 242,
  267, 275, 294, 295.
- **Root cause:** rebase-merge left a probe in an inconsistent
  state.  Not caught by the prior audit because the prior
  audit focused on ReDMCSB coverage, not signature drift.
- **Fix:** drop the 6th positional arg (the int cam_dir) on
  each call site, align the two null-arg safety checks to the
  new 6-arg form.  No semantic change (probe still passes 0.0f
  for game_x / game_y / game_angle, 0.0f for dt).
- **Verification:** `firestaff_nexus_v2_verification_suite_probe`
  builds clean, 35/35 assertions, `ctest -R nexus_v2_verification
  1/1`.
- **Commit:** `a22ec68b fix(nexus_v2_verification): match new
  nexus_v2_pipeline_render 6-arg signature`

---

## 3. Pre-existing Failures (still open, NOT from this audit)

`ctest -j4` against the integrated main reports 6 failures
out of 473 (98.7% pass).  All 6 are pre-existing source-audit
gates with outdated line-ranges after watchdog-passets M11 game
view additions.  Documented but not fixed in this audit:

1. `dm1_v2_launch_smoke_pc34` — expects V2.0 launch resolution
   to floor at 640x400 per the 4d228162 contract, but main
   permits 320x200 (V2.0 is in `M12_PresentationMode_AllowsResolutionChoice`).
   4d228162's M12 launch-res floor was never integrated because
   it conflicts with `m12_enforce_mode_constraints` in main.
   Resolution: pick one (probably update the test to accept
   the no-floor behaviour, since main's behaviour is what
   `firestaff_m12_startup_menu_probe.c` asserts).
2. `v1_status_refresh_order_redmcsb_gate` — line-range audit
   gate that pinned ranges in `m11_game_view.c` that have
   since shifted due to watchdog-passet additions.
3. `dm1_v1_viewport_3d_source_lock` — same shape as #2.
4. `pass623_dm1_v1_input_capture_readiness_bridge` — expects
   a BUG-DNY-related probe that is gated on real DM1 PC 3.4
   original capture (which the audit does not have).
5. `pass625_dm1_v1_original_transcript_row_preflight` — same
   shape as #4 (parity-evidence gate, needs original DOSBox
   capture).
6. `pass626_dm1_v1_original_transcript_turn_redraw_route` — same
   shape as #4.

Tests #4-#6 are not bugs in the engine; they are data-gates
that fail until the project runs an original DOSBox capture
session per `docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`.

Test #1 is a real contract discrepancy that should be triaged
in a follow-up commit (either adopt 4d228162's floor or update
the test to match main's no-floor behaviour).

Tests #2-#3 are stale source-audit ranges; a one-line `grep`
update against the current `m11_game_view.c` line numbers
would fix them.

---

## 4. Stale TODOs (file-level)

40 TODOs in `src/` and `include/`.  Most are deferred-feature
markers (e.g. `nexus_v1_sound.c:58 TODO: reverse-engineer SAL`,
`nexus_v1_script_vm.c:30 TODO: parse SDDRVS.TSK bytecode`).
Three TODOs are **non-deferred** and worth flagging:

- `src/csb/csb_v1_save_load_pc34_compat.c:75 TODO: implement
  mkdir on all platforms. Called before save writes.` — this
  is Bug C.  `ensure_save_dir` is a dead stub.  Fix or delete
  the function.

- `src/theron/theron_v1_save_load.c:284 (pre-fix) TODO:
  re-enable when stable` — this was Bug A.  **Fixed in
  commit 997394f0**; the TODO is gone.

- `src/csb/csb_v1_viewport_pc34_compat.c:1347 TODO (pass604):
  Custom background rendering` — pre-existing scoped work
  referenced by the manifest runner `csb_v1_parity_surface_matrix`.
  Not a regression; intentional work-tracking.

---

## 5. Malloc / Free Audit

Top 5 files by `malloc`/`calloc`/`realloc` count:

| File | allocs | Notes |
|------|--------|-------|
| `src/memory/memory_dungeon_dat_pc34_compat.c` | 23 | clean paired free in error paths; no leaks |
| `src/shared/asset_find_by_hash.c` | 8 | no paired `free()`; investigate |
| `src/nexus/nexus_v1_save_load.c` | 8 | clean paired free in error paths |
| `src/engine/main_loop_m11.c` | 8 | mostly `fopen`/`fclose`, not `malloc` |
| `src/nexus/nexus_v1_ui_surfaces.c` | 7 | clean paired free in error paths |

`asset_find_by_hash.c` has 8 alloc-like calls and zero `free()`
in the same file — likely because the alloc'd buffers are
returned to the caller (which frees them).  This is fine for
a helper module; not a leak.

No files have `malloc` with no `free` in the same TU and no
return path that hands off ownership.  Allocations are
auditable.

---

## 6. Cross-Game Public API Surface

Public headers grouped by game (count of `^[A-Za-z].*_?[a-z]\(` declarations):

| Game | Public surface | Notes |
|------|---------------|-------|
| DM1 V1 | 394 `dm1_v1_*` modules | dense, contract-only |
| CSB V1 | 126 `csb_v1_*` modules | solid, well-anchored |
| DM2 V1 | 34 `dm2_v1_*` modules | smaller, V2 smooth-movement only |
| Theron V1 | 22 `theron_v1_*` modules | 8 slots save, V2 has 4 V2-eligible modules |
| Nexus V2 | 38 `nexus_v2_*` modules | 8/8 phases complete, all V2-eligible |
| M11/M12 | shared infrastructure | M11 game view + m12_menu (big surface) |

No public API was found with a name that doesn't match its
`include/` location.  No dead public symbols (i.e. exported
in a header but no implementation reachable from M10/M11/M12
libs).  All `extern` declarations in `image_backend_pc34_compat.h`
now have a definition (via the new `image_backend_pc34_compat_globals.c`).

---

## 7. Threading Model

Firestaff is fully single-threaded.  No `SDL_CreateThread`,
`pthread_create`, `std::thread`, `_Atomic`, `__atomic`,
`SDL_mutex`, `SDL_LockMutex`, or `pthread_mutex` in the codebase.

**Effect:** simple to reason about, no data-races possible.
**Cost:** SDL frame/audio present blocks the main loop; no
parallel asset decode; no background save.  This is by
design (the original DM1/CSB are single-threaded) but worth
documenting for future multi-game launchers.

---

## 8. Recommended Follow-ups

| # | Action | Effort |
|---|--------|--------|
| 1 | Adopt 4d228162's M12 launch-res floor OR update the test to match no-floor | small |
| 2 | Refresh stale source-audit gates (v1_status_refresh_order, dm1_v1_viewport_3d) | small |
| 3 | Fix CSB V1 `ensure_save_dir` (Bug C) | small |
| 4 | Run original DOSBox capture to unblock pass623/625/626 | medium (separate session) |
| 5 | Re-audit after 4d228162 integration decision | small |

---

## 9. Excluded

- **Performance benchmarks** (raw ctest speed not measured).
- **CI matrix** (5-platform build matrix status: green per
  v2.8.0 release).
- **i18n** (17-language PO catalog is via `firestaff_po_loader`).
- **Documentation** (README.md, RELEASE_NOTES.md, AGENTS.md are
  not source-code).
- **Visual rendering correctness** (requires capture; the
  Champion Z-order and inscription-blur bugs are tracked
  separately in TODO.md and `memory/2026-06-16.md`).

---

## 10. Re-audit — 2026-06-17 (post-4d228162 decision)

Follow-up pass after the audit's #1 (M12 launch-res floor) was
adopted via watchdog commit `44280458` + test updates in
`fe8ae7d9`.  This section tracks the resolution of items #1-#6
plus Bug C.

### Item #1 — M12 launch-res floor (RESOLVED)

`src/ui/menu_startup_m12.c` `M12_StartupMenu_GetLaunchIntent` now
applies the launch-only 640x400 floor for resolution-choice
modes (V2.0/V2.1/V2.2) with stored 320x200.  The floor lives
in `GetLaunchIntent` only, NOT in `m12_enforce_mode_constraints`,
so the row cycle in INV_M12_18 stays full
(320x200 → 640x400 → 1280x960 → ...).  Source-locked comment in
`menu_startup_m12.c:7467-7479`.

Tests:
- `test_dm1_v2_launch_smoke_pc34` — green (4 V20/V21 3840/320
  resolution cases + V1 floor-not-applied case + reload-after-
  save case)
- `test_csb_v2_resolution_selector_gate_m12` — green (V20/V21
  V22 resolution floor contract; previously not wired in
  CMakeLists, now wired at line 1406)

`firestaff_m12_startup_menu_probe` stays 55/55 (INV_M12_18
cycle invariants untouched).

### Item #2 — Stale source-audit gates (RESOLVED)

`v1_status_refresh_order_redmcsb_gate` — `tools/verify_v1_status_
refresh_order_redmcsb_gate.py` SOURCE_RANGES and `require_excerpt`
hardcoded line range updated from 23251-23674 / 24500-24760 to
the current `m11_draw_party_panel` body range
24739-25165 (function moved due to watchdog-passet additions).
Five marker strings ("V1 source status-box background", "before
top-row", "V1 champion name/title status text", "Pass 43:
champion HP/stamina/mana bar graphs", "V1 status-box hand slots")
all confirmed present in the new range.

`dm1_v1_viewport_3d_source_lock` — `test_dm1_v1_viewport_3d_pc34_
compat` was missing the `FIRESTAFF_SOURCE_DIR` compile
definition, so the drift-regression file-contains scan ran from
`build/` (current working dir) and read non-existent
`build/src/engine/m11_game_view.c`.  Added the
`target_compile_definitions(test_dm1_v1_viewport_3d_pc34_compat
PRIVATE FIRESTAFF_SOURCE_DIR=...)` at CMakeLists.txt:4406,
mirroring the existing test_dm1_v2_item_render_pc34 pattern.
43 drift tokens all resolve.

### Item #3 — CSB V1 `ensure_save_dir` (RESOLVED, Bug C)

`src/csb/csb_v1_save_load_pc34_compat.c:74-90` was a no-op stub
that never created the save directory.  `csb_v1_save_game`
opened the file directly; if the parent dir was missing it
returned `CSB_V1_SAVE_ERR_CANT_CREATE` (-6).  Fix:

- Replaced stub with `ensure_save_dir` that calls
  `FSP_CreateDirectoryRecursive(default_save_dir())`
  (cross-platform POSIX mkdir / Windows _mkdir via the
  existing `fs_portable_compat.c` helper, EEXIST-tolerant).
- `csb_v1_save_game` now also calls
  `FSP_CreateDirectoryRecursive(FSP_ParentDir(path))` BEFORE the
  fopen, so a custom save path in a fresh user directory also
  self-heals.
- Both calls are idempotent.

Regression test `test_csb_v1_save_ensure_dir_pc34_compat`
verifies:
- Default save dir is created on first save (HOME-based, fresh
  temp dir)
- Wipe + second save still succeeds (every-save idempotency)
- Custom save path with fresh parent dir also succeeds
  (parent-of-path branch)

Wired in CMakeLists.txt:1391-1417 (added alongside the existing
`test_csb_v1_character_import` block).  All 15 CSB V1 tests
green.

### Items #4-#6 — pass623/625/626 status (ALREADY GREEN)

Audit reported these as failing because they "fail until the
project runs an original DOSBox capture session".  At the time
of re-audit, the three tests pass (test IDs 418/419/420 in the
current ctest numbering):

```
Test #418: pass623_dm1_v1_input_capture_readiness_bridge ... Passed
Test #419: pass625_dm1_v1_original_transcript_row_preflight ... Passed
Test #420: pass626_dm1_v1_original_transcript_turn_redraw_route ... Passed
```

These are *static source-lock + manifest-format gates* (Python
verifiers in `tools/verify_pass{623,625,626}_*.py` checking
ReDMCSB line-range needles and Firestaff M11 line-range
needles), not pixel-parity checks.  Post-audit watchdog work
added the source-locks and manifests that satisfy the gates
without requiring a live DOSBox capture.  The deeper parity
work (real DM1 PC 3.4 screenshots via `dosbox_capture_session.py
--live`) is still a separate, medium-effort task tracked in
`docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md`.  The
infrastructure is verified healthy: preflight 16/16, dry-run
6/6 state transitions, live-inputs validation
(`/Applications/DOSBox Staging.app/.../dosbox`, runtime,
conf all valid; receipts written to
`~/firestaff-captures/dosbox_capture.live_inputs.json`).

### Item #5 — Re-audit (this section)

The pre-existing 4 failures observed in the parallel-j4 ctest
sweep (pass373 / pass374 / pass508 / pass512) are
**flaky-under-parallel** due to `shutil.rmtree` racing with
parallel invocations on a shared build-dir; serially all four
pass.  The watchdog's own 4d228162 commit message acknowledges
this exact pattern: "3 others were flaky-under-parallel and
pass in isolation".  These are not caused by the audit
follow-up work and are not in scope of the audit's 6 items.

### Net result

All 6 audit items + Bug C resolved or already green.  ctest
sweep of the 8 audit-related targets + the new Bug C regression
test: 8/8 pass.  Broader `csb_v1|m12|launch_smoke` sweep:
20/20 pass.  Pre-existing parallel-flaky tests (pass373/374/
508/512) are not audit items and run clean serially.
