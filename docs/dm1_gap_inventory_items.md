# GAP: Inventory / Items — m11_obj_use() Champion-Slot Wiring Gap

## Status
**GAP — Item pickup/drop/use stubs need champion-slot wiring; F0349 complete but m11_obj_use() not wired**

## Source Location
`src/dm1/dm1_v1_object_interaction_pc34_compat.c`, function `m11_obj_use()` (line 104).

Consumables implementation: `src/dm1/dm1_v1_inventory_consumables_pc34_compat.c`.
Reference: ReDMCSB PANEL.C:1743-1950, function F0349_INVENTORY_ProcessCommand70_ClickOnMouth.

## Gap Description

`m11_obj_use()` is a stub that checks the `usable` flag on a world object and returns 1 if set:
```c
int m11_obj_use(M11_ObjectState* s, int objIdx) {
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;
    M11_WorldObject* obj = &s->objects[objIdx];
    if (obj->usable) {
        return 1;   // item is usable — but no stat effect applied
    }
    return 0;
}
```

The TODO comment in the source explicitly acknowledges the gap:
> "TODO: delegate to consumables module for actual stat effects."

The actual consumption logic (potions, food, water, scrolls, wands) is implemented in `dm1_v1_inventory_consumables_pc34_compat.c` and labeled with F0349 source references (lines 56, 272). F0349 is the ReDMCSB reference function that handles mouth-consumption (eating, drinking, reading).

**The gap is the wiring:** `m11_obj_use()` does not call into the F0349 consumables module. The `usable` flag check is necessary but not sufficient — the function must also identify *which champion* is consuming the item, apply the stat effect to that champion, and handle champion-specific state (e.g., anti-magic field, food/water tracking).

## Item Pickup / Drop Stubs

Three related stub functions exist in the same file:
- `m11_obj_throw()` — marks object as in-flight (`x = -2`), returns 1 on success; no champion-slot association
- `m11_obj_activate()` — checks `activatable` flag, returns 1; no champion association
- `m11_obj_examine()` — returns object type name and weight; no champion association

None of these functions are wired to the champion that is interacting with the object (no `championIndex` parameter).

## Champion-Slot Wiring Required

To properly implement item use, the following information must be threaded through:
1. **Which champion** is performing the action (leader or selected party member)
2. **Which hand/slot** the item is in (left hand, right hand, backpack slot)
3. **What type of item** it is (consumable: food/water/potion/scroll; equippable: weapon/armour)
4. **Champion state** at time of use (anti-magic, anti-fire, current HP/MP/stamina)

## F0349 Completeness

`dm1_v1_inventory_consumables_pc34_compat.c` implements F0349 consumption effects:
- Mouth consumption counter loop (lines 272: `for (counter=5; --counter;)`)
- Stat effect application (HP, stamina, mana restoration)
- Food/water tracking

However, F0349 is never called from `m11_obj_use()` or any parent function in the DM1 V1 compat layer. The wiring is the gap.

## Impact
- Normal play: items with `usable=1` are recognized but their effects are not applied
- Food/water depletion and restoration is tracked separately via `dm1_v1_food_water_pc34_compat.c`
- Consumable items (potions, scrolls) appear usable but do nothing when consumed

## Required Fix (Non-Blocking)
1. Extend `m11_obj_use()` signature to include `int championIndex` and `int handSlot`
2. Route to F0349 consumables handler based on `objectType`
3. Wire champion state (stats, conditions) into the F0349 call
4. Update call sites (viewport click routing, panel click routing) to pass the correct champion
5. Add integration tests: use potion → check champion HP increases; eat food → check food decreases
