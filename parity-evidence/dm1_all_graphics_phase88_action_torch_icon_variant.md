# DM1 all-graphics phase 88 — action-hand torch icon variants

## Problem

Phase 85 made action-hand cells use source object icon atlases, but `F0033_OBJECT_GetIconIndex` has dynamic icon variants. The first important one is torch state:

```c
case C004_ICON_WEAPON_TORCH_UNLIT:
    if (weapon->Lit) {
        icon += G0029_auc_Graphic562_ChargeCountToTorchType[weapon->ChargeCount];
    }
```

Without this, a lit torch in the action hand would still use the unlit torch icon.

## Change

`m11_object_icon_index_for_thing(...)` now applies the source torch charge-count variant table:

```text
G0029 = {0,1,1,1,2,2,2,2,3,3,3,3,3,3,3,3}
```

For weapon subtype `2` / icon index `4`, when `lit` is true, the icon index is offset by the charge-count bucket.

## Gate

Added invariant:

- `INV_GV_307` — lit torch action icon differs from unlit torch by using the source charge-count icon variant.

```text
PASS INV_GV_307 action-hand icon cells: lit torch uses source charge-count icon variant
# summary: 415/415 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
