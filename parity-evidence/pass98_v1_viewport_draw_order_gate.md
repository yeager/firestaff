# Pass 98 — V1 viewport world draw-order gate

Date: 2026-04-28

## Goal

Add a narrow source-shape gate for the DM1 V1 viewport world stack so future viewport/world visual work cannot silently reorder layers.

This is evidence hardening only. It does not claim original-runtime pixel parity and it does not compare against DOSBox captures.

## Contract checked

Tool: `tools/verify_v1_viewport_draw_order_gate.py`

The verifier checks `m11_game_view.c` for two draw-order contracts:

1. Wall/door/stair face rendering happens before wall and door ornaments, and ornaments happen before open-cell contents.
2. Open-cell contents remain ordered as:
   - floor ornaments
   - floor items
   - creatures
   - projectiles/explosions

It also checks that open-cell contents are guarded by `m11_viewport_cell_is_open(...)` before layer 0, so non-open center cells do not inherit the item/creature/projectile stack by accident.

## Result

Command:

```sh
python3 tools/verify_v1_viewport_draw_order_gate.py
```

Result:

```text
V1 viewport draw-order source-shape verification passed
```

## Interpretation

- This protects draw-order/evidence acceptance while the lane continues toward original overlay comparison.
- It is intentionally source-shape based: renderer refactors may need to update the verifier, but accidental layer inversions should fail loudly.
- No gameplay behavior, capture route, or public parity claim changed in this pass.
