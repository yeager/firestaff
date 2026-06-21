# RESOLVED: Inventory / Items Route Parity

## Status

**RESOLVED for DM1 finish scope.**

pass1061 closed the stale `m11_obj_use()` consumable wiring gap described
below. pass1070 audits the broader inventory route row and verifies the
existing route/runtime/pixel coverage together:

- `inventory_panel_slot_routes_pc34_compat`
- `dm1_v1_inventory_equip_slots_pc34_compat`
- `dm1_v1_inventory_backpack_chest_pc34_compat`
- `dm1_v1_inventory_consumables_pc34_compat`
- `inventory_mouth_eye_routes_pc34_compat`
- `dm1_v1_object_interaction_source_lock`
- `m11_inventory_scroll_panel_render_source_lock`
- `m11_inventory_full_panel_runtime_source_lock`
- `m11_inventory_mouth_visual_blit_source_lock`

Those gates cover equipment slots, backpack/chest route semantics, source-style
consumables, mouth/eye dispatch, panel-slot routing, compact object interaction,
and M11 rendered/runtime inventory behavior. Further visual polish can still
land normally, but it is no longer tracked as an open inventory route parity gap.

## Source Location

`src/dm1/dm1_v1_object_interaction_pc34_compat.c`, function `m11_obj_use()`.

Consumables implementation: `src/dm1/dm1_v1_inventory_consumables_pc34_compat.c`.

Runtime mouth-click path: `src/engine/m11_game_view.c`, function `m11_process_v1_mouth_click()`.

Reference: ReDMCSB `PANEL.C:1743-1950`, function `F0349_INVENTORY_ProcessCommand70_ClickOnMouth`.

## Finding

The previous version of this document described `m11_obj_use()` as a two-argument stub that only checked an object's `usable` flag. Current main no longer matches that description.

`m11_obj_use()` now has champion data and result outputs:

```c
int m11_obj_use(M11_ObjectState* s, int champIdx, int objIdx,
                DM1ConsumableChampionPc34* champData,
                DM1ConsumableResultPc34* result);
```

The function delegates consumable object classes to the F0349-compatible consumables module:

- `DM1_OBJTYPE_POTION` -> `dm1_inventory_consume_potion_pc34()`
- `DM1_OBJTYPE_FOOD` -> `dm1_inventory_consume_food_junk_pc34()`
- `DM1_OBJTYPE_WATER` -> `dm1_inventory_consume_water_junk_pc34()`

The compact `M11_ObjectState` abstraction does not carry full THING subtype data, so the object-use wrapper uses testable proxies:

- potion type from `stackCount` when it is in the source potion type range `6..15`;
- potion power from `weight`;
- food icon from `weight` when it is in the source food icon range `168..175`, with apple as fallback;
- water/waterskin icon from `weight` when it is `8` or `9`, with waterskin as fallback;
- waterskin charges from `stackCount`.

Equipment remains intentionally outside this mouth-use path. Weapons and armor return `0` from `m11_obj_use()` because equipping is owned by the slot system, not by F0349 mouth consumption.

## Runtime Path

The live M11 inventory mouth click does not rely on the compact object wrapper. `m11_process_v1_mouth_click()` reads the actual leader-hand THING, resolves junk/potion subtype data from `state->world.things`, calls the same consumables helpers, and commits the changed champion fields back into the active champion.

That runtime route covers:

- water and waterskin charge use;
- food consumption and mouth animation;
- potion effects, empty-flask conversion, VI wound masks, shield defense and health/stamina/mana/stat changes.

## Source-Lock

- `PANEL.C:1743-1785` declares the F0349 mouth-consumption locals.
- `PANEL.C:1824-1844` gates mouth-allowed objects, water/waterskin charge use, and leader-hand removal.
- `PANEL.C:1850-1917` applies potion effects and converts potions to C20 empty flask.
- `PANEL.C:1918-1919` applies `G0242` food amounts.
- `PANEL.C:1922-1945` clamps health/stamina, drives the mouth animation, and routes the swallow sound.
- `DUNGEON.C:428-436` defines the food amounts table.

## Verification

- `dm1_v1_inventory_consumables_pc34_compat` verifies the F0349-compatible consumables module.
- `dm1_v1_object_interaction_source_lock` now verifies that `m11_obj_use()` delegates ROS potion, food icon, and waterskin objects to that module and leaves equipment to the slot system.

## Remaining Inventory Work

None for the DM1 finish-scope route parity row. Remaining chest-specific detail
coverage is tracked separately in `docs/FIRESTAFF_GAP_LIST.md` as
`Chest runtime detail coverage`.
