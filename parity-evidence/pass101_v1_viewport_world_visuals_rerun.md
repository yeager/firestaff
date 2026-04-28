# Pass 101 — V1 viewport/world visuals gate rerun

Date: 2026-04-28
Lane: Viewport/world visuals (DM1 V1 walls/items/ornaments/creatures/draw-order/evidence)

## Scope

Reran the current narrow viewport/world source-shape and GRAPHICS.DAT-backed invariant gates on N2. This was evidence hardening only; no renderer/code changes were required in this pass.

## Commands

```sh
python3 tools/verify_v1_viewport_draw_order_gate.py
cmake --build build --target firestaff_m11_game_view_probe -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
```

Full local run log is stored at:

- `parity-evidence/runs/pass101_viewport_world_visuals_20260428T065423Z/output.log` (ignored `*.log`, local evidence run-dir)

## Data anchors observed

- `$HOME/.firestaff/data/GRAPHICS.DAT` exists on N2 (`363417` bytes).
- Original reference extracted set exists on N2 at `$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/` with `dm-pc34/` and `dm2-dos-asm/`.

## Results

- `tools/verify_v1_viewport_draw_order_gate.py`: PASS
  - wall/door ornaments before open-cell contents at `m11_game_view.c:7555`
  - open-cell content order remains floor ornaments → floor items → creatures → projectiles/effects at `m11_game_view.c:7678`
  - non-open cells remain guarded before layer 0
- `firestaff_m11_game_view_probe`: PASS, `578/578 invariants passed`
  - includes viewport rect/base seams, parity floor/ceiling flip, wall/floor ornaments, floor items, projectiles/effects, creature placement/aspect families, source draw-order seam, viewport clipping, and relevant world-content visibility gates.

## Notes

The working tree already had out-of-scope dirty files before this pass (`firestaff_m11_pass42_chrome_reduction_probe.c`, `firestaff_m11_pass43_bar_graph_probe.c`, `verification-m11/verification_summary.md`, plus a stale untracked worktree directory). This pass only adds this evidence note.

Remaining blocker is unchanged: the lane has strong source/data/probe coverage, but semantically matched original DM1 gameplay capture/route is still needed before claiming pixel-perfect original runtime parity.
