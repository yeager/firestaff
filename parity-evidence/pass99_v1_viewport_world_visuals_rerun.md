# Pass 99 — V1 viewport/world visuals gate rerun

Date: 2026-04-28
Lane: Viewport/world visuals (DM1 V1 walls/items/ornaments/creatures/draw-order/evidence)

## Scope

Reran the narrow viewport/world gates on N2 after the draw-order source-shape verifier was present in the worktree. This pass is evidence hardening only; it does not claim pixel-perfect original runtime parity.

## Commands

```sh
python3 tools/verify_v1_viewport_draw_order_gate.py
cmake --build build --target firestaff_m11_game_view_probe -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
```

Full local run log is stored at:

- `parity-evidence/runs/pass99_viewport_world_visuals_20260428T065041Z/output.log` (ignored `*.log`, local evidence run-dir)

## Results

- `tools/verify_v1_viewport_draw_order_gate.py`: PASS
  - wall/door ornaments remain before open-cell contents at `m11_game_view.c:7555`
  - open-cell contents remain ordered floor ornaments → floor items → creatures → projectiles/effects at `m11_game_view.c:7678`
  - non-open cells are guarded before layer 0
- `firestaff_m11_game_view_probe`: PASS, `578/578 invariants passed`
  - includes the DM1 V1 viewport/world seams for viewport rect/base, wall/floor ornaments, source item/projectile/creature placement, side-cell creatures, viewport clipping, parity floor/ceiling flip, source draw-order seam, and relevant world-content visibility gates.

## Notes

Build warnings observed are pre-existing/out-of-scope M12 menu indentation warnings plus probe header/truncation warnings; no viewport/world failures were observed.

Remaining blocker is unchanged from phase 3037: semantically matched original DM1 gameplay capture/route is still needed before treating pixel deltas as final parity evidence.
