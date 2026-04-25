# DM1 all-graphics phase 91 — inventory slot source object icons

## Problem

Inventory slots were still drawing scaled viewport object sprites via `m11_inventory_thing_sprite_index(...)`. DM1 does not do that for slot boxes. It uses `F0038_OBJECT_DrawIconInSlotBox`, which blits the 16×16 source object icon from graphics `42..48` directly into the slot box.

There is also an important palette distinction:

- action-area object icons use `G0498` (`12 -> C04 cyan`)
- inventory slot icons do **not** use that action-area palette rewrite

## Change

`m11_draw_inv_slot(...)` now first resolves the source object icon with `m11_object_icon_index_for_thing(...)` and draws that 16×16 icon at the slot-box inset.

The old scaled-sprite path remains only as a compatibility fallback if icon assets are unavailable.

`m11_draw_dm_object_icon_index(...)` now takes an `applyActionPalette` flag so action cells and inventory slots share the source atlas extractor without accidentally sharing palette behavior.

## Gate

Added invariant:

- `INV_GV_309B` — inventory slot dagger icon preserves source color 12/dark gray, proving it uses source object icons without the action-area cyan palette remap.

```text
PASS INV_GV_309B inventory slot icons use source object icons without action palette remap
# summary: 418/418 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
