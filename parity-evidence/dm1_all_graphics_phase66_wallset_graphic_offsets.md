# DM1 all-graphics phase 66 — live wall-set graphic offsets

## Change

Source-bound wall blits now offset wall graphics by the current map's `wallSet` instead of always using wall-set 0 graphics.

DM1 wall sets are 15 graphics each starting at graphic 93. The renderer now resolves wall graphics as:

```c
93 + wallSet * 15 + (wallSet0GraphicIndex - 93)
```

This affects the source-bound front/side wall panel blits that use D0/D1/D2/D3 wall-zone graphics.

## Gate

Added invariant:

- `INV_GV_110B` — source wall blits offset wall graphics by current map wallSet

Pinned examples:

- wallSet 0, graphic 97 -> 97
- wallSet 1, graphic 97 -> 112
- wallSet 2, graphic 93 -> 123
- wallSet 3, graphic 107 -> 152

Also removed an unused helper function that caused build-warning noise.

## Visual capture

Fresh in-game captures:

- `verification-m11/wallset-live-offset-20260425-150009/01_ingame_start_latest.png`
- `verification-m11/wallset-live-offset-20260425-150009/05_ingame_after_cast_latest.png`

Visual review:

- Live corridor structure improved and is more recognizable.
- UI remains stable/readable.
- Colored wall/ceiling corruption remains: rainbow/static-like pixels, vertical multicolor streaks, incorrect wall texture colors/patterns.
- Therefore wall-set offset was a real correction but not the whole viewport issue.

## Verification

```text
PASS INV_GV_110B source wall blits offset wall graphics by current map wallSet
# summary: 400/400 invariants passed
ctest: 4/4 PASS
build log: no compiler warnings
```

## Remaining

Next likely causes to inspect:

- wall graphic decode/bitplane interpretation
- source blit alignment/stride for wall slices
- palette/index remapping for wall graphics
- remaining wall-zone offset/order mismatches
