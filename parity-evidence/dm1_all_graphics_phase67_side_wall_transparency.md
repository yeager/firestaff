# DM1 all-graphics phase 67 — side wall transparency and wallset stride correction

## Change

Corrected two source-binding details in the DM1 wall blit path:

1. Side wall panels now use the same transparent key as ReDMCSB `F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap`:

```c
C10_COLOR_FLESH
```

Front/center wall panels remain no-transparency, matching `F0792_DUNGEONVIEW_DrawBitmapYYY`.

2. Wall-set graphic offset helper now uses the source wall-set stride:

```c
M647_WALL_SET_GRAPHIC_COUNT = 40
```

rather than the 15 visible wall-panel subset count. This is source-correct even though the current DM1 data appears to use wall set 0 for all loaded maps.

## Gate

Updated invariant:

- `INV_GV_110B` — source wall blits offset wall graphics by current map wallSet

Pinned examples now use stride 40:

- wallSet 0, graphic 97 -> 97
- wallSet 1, graphic 97 -> 137
- wallSet 2, graphic 93 -> 173
- wallSet 3, graphic 107 -> 227

## Visual capture

Fresh in-game captures:

- `verification-m11/side-wall-transparency-20260425-150730/01_ingame_start_latest.png`
- `verification-m11/side-wall-transparency-20260425-150730/05_ingame_after_cast_latest.png`

Visual review:

- Side wall transparency/structure improved somewhat.
- The live dungeon viewport is still not presentable: colored/noisy wall/ceiling artifacts remain.
- UI/spell overlay remains the most presentable capture.

## Verification

```text
PASS INV_GV_110B source wall blits offset wall graphics by current map wallSet
# summary: 400/400 invariants passed
ctest: 4/4 PASS
```

## Remaining

The raw wall assets rendered with the current DM palette look mostly gray/stone, so the remaining rainbow/static issue is likely in live viewport composition rather than the source art itself. Continue inspecting wall-zone blit order, transparency keys per wall family, clipping, and depth/light post-processing.
