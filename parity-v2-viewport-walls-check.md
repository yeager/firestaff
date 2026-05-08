# Parity V2 viewport/walls check

Worktree: `<firestaff-worktree>/n2-dm1v1-viewport-walls-20260505-1227`
Branch: `worker/n2-dm1v1-viewport-walls-20260505-1227`
Checked: 2026-05-05

## Git state

- Initial `git status --short --branch`: clean at `9aedbb8 Source-lock DM1 viewport wall frame blits`.
- Recent relevant commits:
  - `9aedbb8 Source-lock DM1 viewport wall frame blits`
  - `4809fbb dm1: source-lock viewport wall panel plan`
  - `fe3842b test: lock dm1 v1 viewport draw plan`
  - `6ac6980 test: lock viewport wall rectangle clipping`
  - `77e41cd Lock DM1 V1 viewport wall golden comparison`

## ReDMCSB source audit

Reference root used: `<redmcsb-source>/Toolchains/Common/Source/`.

Files requested/found:

- `VIEWPORT.C`: present. Confirms `F0564_VIEWPORT_InitializeBitPlanes` uses a 224x136 viewport source and places destination at screen line 33; `F0566_VIEWPORT_BlitToScreen` copies 224x136 to the screen.
- `DRAWVIEW.C`: present. Platform viewport blit/palette path exists; current Firestaff comments cite it for `F0097_DUNGEONVIEW_DrawViewport`/present.
- `DUNVIEW.C`: present and is the actual wall/viewport composition source:
  - `F0100` lines 3048-3059: transparent wall-set bitmap blit into `G0296_puc_Bitmap_Viewport`, using frame `C6_X/C7_Y` as source offsets and viewport width/height bounds.
  - `F0101` lines 3065-3075: optimized opaque center-wall blit with `CM1_COLOR_NO_TRANSPARENCY`.
  - `F0128` lines 8318-8580: viewport draw order, far-to-near square traversal, edge wall panel calls (`D3L2/D3R2`) before matching depth squares, and native wall-set restore.
- `BLIT.C`: present and relevant even though not in the original request list. `F0132_VIDEO_Blit` lines 30, 142-147, and 188-194 separate source x/y from destination box x/y; this matches the new Firestaff wall frame source-box tests.
- `WALLS.C`: not present under the ReDMCSB source root.
- `VISUAL.C`: not present under the ReDMCSB source root.
- `DRAWINGS` / viewport-related drawing directory: no `DRAWINGS` tree found. The viewport-related drawing files present are `DRAWVIEW.C`, `DUNVIEW.C`, `BLIT.C`, and `VIEWPORT.C`.

Firestaff implementation touched by `9aedbb8`:

- `dm1_v1_viewport_3d_pc34_compat.c`
- `dm1_v1_viewport_3d_pc34_compat.h`
- `test_dm1_v1_viewport_draw_plan_pc34_compat.c`

Audit result: source citations and tests line up with the available ReDMCSB files. The only naming mismatch is that `WALLS.C`, `VISUAL.C`, and `DRAWINGS` do not exist in this source checkout; wall behavior lives in `DUNVIEW.C` plus `BLIT.C`.

## Build/test results

Commands run from the worktree:

```sh
cmake --build build -j2
cd build && ctest -R "viewport|walls" --output-on-failure
```

Results:

- Build: PASS (`100% Built target firestaff_m11_game_view_probe`).
- Targeted ctest: PASS, `31/31` tests passed in `3.43 sec`.

Targeted tests included:

- `dm1_v1_viewport_f0128_draw_plan_source_lock`
- `dm1_v1_viewport_walls_golden_comparison`
- `v1_viewport_wall_rect_clip_gate`
- `v1_viewport_wall_blit_transparency_gate`
- `v1_viewport_wall_parity_flip_gate`
- `dm1_v1_viewport_world_redmcsb_source_lock`
- `dm1_v1_original_movement_viewport_blocker_gate`
- plus the remaining viewport/walls filtered tests (`Total Tests: 31`).

## Blockers

No build or targeted-test blockers found.

Source-audit caveat: requested `WALLS.C`, `VISUAL.C`, and `DRAWINGS` artifacts are absent from the local ReDMCSB source tree, so they could not be audited directly. The equivalent authoritative wall/viewport paths were audited in `DUNVIEW.C`, `BLIT.C`, `DRAWVIEW.C`, and `VIEWPORT.C`.
