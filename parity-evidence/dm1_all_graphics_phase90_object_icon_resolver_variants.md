# DM1 all-graphics phase 90 — source object icon resolver variants

## Problem

Phases 85, 88, and 89 made action cells use source object icons and covered weapon dynamic variants. `F0033_OBJECT_GetIconIndex` also varies non-weapon object icons:

- scroll closed/open: icon `30 -> 31`
- compass direction: icon `0 + party direction`
- water / Jewel Symal / Illumulet charge: icon `8/10/12 -> +1`

Some of these objects do not render in action cells because `ActionSetIndex == 0`, but the resolver should still match the source so inventory/hand/pointer icon work can reuse it safely.

## Change

Added public probe helper:

```c
int M11_GameView_GetObjectIconIndexForThing(const M11_GameViewState* state,
                                            unsigned short thingId);
```

Extended `m11_object_icon_index_for_thing(...)` to cover source dynamic variants for:

- closed scroll
- compass direction
- charged water/Jewel Symal/Illumulet

## Gate

Added invariant:

- `INV_GV_309` — object icon resolver returns source values for open/closed scroll, east-facing compass, and charged water.

```text
PASS INV_GV_309 object icon resolver follows source scroll, compass, and charged-junk variants
# summary: 417/417 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
