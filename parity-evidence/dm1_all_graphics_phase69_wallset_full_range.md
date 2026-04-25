# DM1 all-graphics phase 69 — wallset offset applies to full 40-entry graphic range

## Problem

The previous wallset offset fix applied the `M647_WALL_SET_GRAPHIC_COUNT = 40` stride to front/side wall-panel blits, but the source wallset graphic range is the whole 40-entry block:

```text
M646_GRAPHIC_FIRST_WALL_SET = 86
M647_WALL_SET_GRAPHIC_COUNT = 40
range for wall set 0: 86..125
```

That range includes door-side/top wallset graphics and stair graphics (`108..125`), not only the visible wall panels around `93..107`.

DM1 currently appears to load maps with wallSet 0, so this does not change the default live screenshot. It prevents the next nonzero-wallset map from drawing mixed wall panels from one set and stairs/door-side strips from wallset 0.

## Change

- Added one central wallset graphic resolver for source range `86..125`.
- Applied it to:
  - wall-panel blits
  - generic zone blits
  - flipped zone blits
- Updated `M11_GameView_GetWallSetGraphicIndex(...)` to expose the same full-range behavior.

## Gate

Updated invariant:

- `INV_GV_110B` — source wall/stairs blits offset full wallset graphic range by current map wallSet

Pinned examples:

```text
wallSet 0, graphic 86  -> 86
wallSet 1, graphic 86  -> 126
wallSet 1, graphic 97  -> 137
wallSet 2, graphic 93  -> 173
wallSet 3, graphic 107 -> 227
wallSet 1, graphic 125 -> 165
```

## Verification

```text
PASS INV_GV_110B source wall/stairs blits offset full wallset graphic range by current map wallSet
# summary: 400/400 invariants passed
ctest --test-dir build --output-on-failure
100% tests passed, 0 tests failed out of 4
```
