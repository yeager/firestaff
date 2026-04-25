# DM1 all-graphics phase 102 — map overlay classification

## Problem

`PARITY_MATRIX_DM1_V1.md` listed the map overlay as `BLOCKED_ON_REFERENCE` with "Unknown — may not exist in DM1 V1 scope". That was no longer the clearest status.

Local ReDMCSB PC source has `NEWMAP.C`, but it is not an automap UI. It is map-transition plumbing:

```c
F0003_MAIN_ProcessNewPartyMap_CPSE(mapIndex)
    F0742_MUSIC_SetTrack(mapIndex)
    F0194_GROUP_RemoveAllActiveGroups()
    F0174_DUNGEON_SetCurrentMapAndPartyMap(mapIndex)
    F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF()
    F0195_GROUP_AddAllActiveGroups()
    F0337_INVENTORY_SetDungeonViewPalette()
```

No source-backed full-screen player map/automap renderer has been identified in the DM1 V1 runtime.

## Change

Updated the matrix row:

- Firestaff's `mapOverlayActive` / `m11_draw_fullscreen_map(...)` / `M` key overlay is now explicitly classified as a convenience/debug surface.
- Status moved from `BLOCKED_ON_REFERENCE` to `KNOWN_DIFF`.
- Next action: hide/disable from default V1 parity mode or keep strictly debug-only; do not count it as original UI unless contrary source/runtime evidence appears.

Updated summary counts:

- `KNOWN_DIFF`: `12 -> 13`
- `BLOCKED_ON_REFERENCE`: `~12 -> ~11`

## Gate

Documentation/source-classification pass; no runtime code changed.
