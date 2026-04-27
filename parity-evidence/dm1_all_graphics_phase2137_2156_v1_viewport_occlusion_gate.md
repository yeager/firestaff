# DM1 all-graphics phase 2137-2156 — V1 viewport source occlusion gate

Date: 2026-04-27 07:01 Europe/Stockholm
Scope: Firestaff V1 dungeon viewport walls, floor ornaments/pits, stairs, teleporter fields, side doors, side door ornaments/masks.

## Change

Added a source-order visibility gate for normal V1 viewport passes.  The renderer now computes the nearest non-open center-lane square from the sampled 3×3 viewport cells and prevents farther source-backed side/floor/field/door overlays from being drawn beyond that blocker.

This keeps layout-696 wall/object work bounded by the same center-line occlusion rule already used by center contents, avoiding the bad class of viewport regressions where distant D2/D3 side features leak around a nearer closed center wall or door.  The source-backed content anchor seams remain intact, including C2500 object placement and C3200 creature `/side/back` and `/mid/large` bitmap-selection paths.

## Source basis

- ReDMCSB `DUNVIEW.C` draw order: visible squares are drawn from distant to near, but center-line blockers terminate visibility behind them.
- Existing Firestaff V1 layout-696 zone tables and GRAPHICS.DAT blits are unchanged; this pass only gates which already-source-backed specs may draw.
- No HUD/champion top-row, original overlay/capture, DM1 V1 original-faithful, or V2 scaffold/assets paths were touched.

## Verification

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
ctest --test-dir build -R 'm11_phase_a|m11_game_view|m11_launcher_smoke' --output-on-failure
./build/firestaff_m11_phase_a_probe
./build/firestaff_m11_game_view_probe
git diff --check
targeted grep over committed diff for sensitive-data patterns
```

Results:

- Build: passed.
- M11 phase A: `19/19 invariants passed`.
- M11 game-view probe: `577/577 invariants passed`.
- Launcher smoke: passed via CTest.
- Relevant M10 verify: N/A; no M10 files or gameplay/memory compatibility code touched.
- `git diff --check`: passed.
- Targeted credential scan: no matches.

## Evidence path

- This evidence note: `parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md`
- Existing generated V1 screenshot crops remain under `verification-screens/*_latest_viewport_224x136.png` for visual inspection of the normal DM1 viewport aperture.

## 2026-04-27 addendum — narrow source-shape gate

Added `tools/verify_v1_viewport_occlusion_gate.py` as a lightweight source-shape gate for the same occlusion rule.  It verifies that:

- `m11_dm1_max_visible_forward_from_center` stops at the nearest non-open center-lane cell (`m11_game_view.c:8771`).
- The pit, floor-ornament, stair, teleporter-field, side-wall, side-door, side-door-ornament, and side destroyed-door-mask source-backed passes all test `relForward > maxVisibleForward` before calling `m11_sample_viewport_cell`.
- `m11_draw_viewport` derives `maxVisibleForward` once from the sampled 3×3 cells and passes it into those families.

This is deliberately narrower than a pixel-parity proof: it catches the regression class where pits, floor ornaments, stairs, fields, side walls, or side-door overlays behind a blocking front wall/door become eligible for sampling/drawing again.  It does not validate HUD/original-capture/V2 paths and does not replace the asset-backed `firestaff_m11_game_view_probe` visual matrix.

Gate run:

```sh
python3 tools/verify_v1_viewport_occlusion_gate.py
```

Result: passed.
