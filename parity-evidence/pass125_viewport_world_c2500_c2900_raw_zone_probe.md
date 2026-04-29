# Pass125 viewport/world — C2500/C2900 raw source-zone blocker probe

Date: 2026-04-28
Worktree: `<N2_WORKTREES>/pass125-viewport-world-auto-1777391888`
Branch: `worker/pass125-viewport-world-auto-1777391888`
Base: `sync/n2-dm1-v1-20260428`

## Scope

Pass125 inspected the recent pass122/viewport evidence and focused on DM1 V1 world-content placement for items/projectiles.  Previous passes bound the normal object/projectile renderer to the first five source-backed C2500/C2900 layout rows, but `zones_h_reconstruction.json` exposes additional layout-696 records:

- `C2500..C2567` (17 rows × 4 cells) for object/creature placement
- `C2900..C2947` (12 rows × 4 cells) for projectile placement

The concrete blocker is the exact F0115 `viewSquareTo`/side-deep row mapping.  Rather than inventing side-pane coordinates, this pass adds raw source-zone helpers and probe invariants so every relevant row remains executable and regression-gated while the renderer keeps its existing five-row path.

## Change

Added source-backed raw-zone accessors:

- `M11_GameView_GetC2500ObjectRawZonePoint(rowIndex, relativeCell, ...)`
- `M11_GameView_GetC2900ProjectileRawZonePoint(rowIndex, relativeCell, ...)`

Added probe gates:

- `INV_GV_114C4` — C2500 rows 5..16 expose full layout-696 side/deep source family
- `INV_GV_245G` — C2900 rows 5..11 expose full layout-696 side/deep source family

Pinned samples include:

- C2500 row 5 cell 1 → `(131,78)`
- C2500 row 11 cell 1 → `(158,133)`
- C2500 row 16 cell 3 → `(218,74)`
- C2900 row 5 cell 1 → `(132,46)`
- C2900 row 8 cell 3 → `(76,47)`
- C2900 row 10 cell 2 → `(301,47)`
- C2900 row 11 cell 1 → `(158,47)`

## Verification

```sh
cmake -S . -B build
cmake --build build -j4
./run_firestaff_m11_game_view_probe.sh
ctest --test-dir build --output-on-failure
```

Results:

```text
M11 game-view probe: # summary: 582/582 invariants passed
100% tests passed, 0 tests failed out of 7
```

## Blocker status

No tool/library blocker.  Implementation blocker remains conceptual/source-mapping only: exact normal-renderer use of C2500 rows 5..16 and C2900 rows 5..11 still requires reconciling DUNVIEW.C F0115 `viewSquareTo`/side/deep row selection.  The raw helpers and probes make that next pass safe and source-backed instead of relying on approximate pane math.
