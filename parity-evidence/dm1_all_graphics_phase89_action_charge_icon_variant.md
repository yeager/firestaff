# DM1 all-graphics phase 89 — charged weapon action icon variants

## Problem

Phase 88 handled the special torch charge-count icon table. `F0033_OBJECT_GetIconIndex` also has a second dynamic branch for chargeable weapons:

```c
case C023_ICON_WEAPON_BOLT_BLADE_STORM_EMPTY:
case C014_ICON_WEAPON_FLAMITT_EMPTY:
case C018_ICON_WEAPON_STORMRING_EMPTY:
case C025_ICON_WEAPON_FURY_RA_BLADE_EMPTY:
case C016_ICON_WEAPON_EYE_OF_TIME_EMPTY:
case C020_ICON_WEAPON_STAFF_OF_CLAWS_EMPTY:
    if (weapon->ChargeCount) icon++;
```

Without this, charged Flamitt/Stormring/etc. action-hand cells would keep showing the empty icon.

## Change

`m11_object_icon_index_for_thing(...)` now applies the source `+1` icon variant for charged weapons with base icon indices:

```text
14, 16, 18, 20, 23, 25
```

## Gate

Added invariant:

- `INV_GV_308` — charged Flamitt action icon differs from empty Flamitt by using the source `+1` icon variant.

```text
PASS INV_GV_308 action-hand icon cells: charged weapon uses source +1 icon variant
# summary: 416/416 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
