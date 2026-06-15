# CSB V1 CustomBackgrounds Room-Slot Backdrop1 Source Lock

## Source Anchors

- ReDMCSB `DUNVIEW.C` `F0128_DUNGEONVIEW_Draw_CPSF`, lines 8318-8542: viewport dispatch order after the floor/ceiling baseline.
- ReDMCSB `DUNVIEW.C` `F0098_DUNGEONVIEW_DrawFloorAndCeiling`, lines 2962-3002: base floor/ceiling pixels and `G0297_B_DrawFloorAndCeilingRequested` reset.
- ReDMCSB `DUNVIEW.C` `F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF`, lines 3502-3938: wall-ornament/alcove keepout before the second masked backdrop absorb step.
- ReDMCSB `DEFS.H`, lines 2596-2614: I34E/P31J view-square ordinals.
- ReDMCSB `DUNGEON.C` `F0163`, lines 1769-1838; `F0164`, lines 1840-1905; `F0172`, lines 2466-2523: dungeon thing-list/aspect ordinals used by the room-square walk.
- CSB-lineage `Viewport.cpp`, lines 6451-6505: `ApplyBackground` masked composite.
- CSB-lineage `Viewport.cpp`, lines 6599-6619: roomNum application of `pSkinDef[0]/[4]`, `pSkinDef[2]/[6]`, then room-gated `pSkinDef[1]/[5]`.

## Disjointness

This gate is intentionally separate from the existing first-backdrop, first-backdrop source-lock, second-backdrop, both-backdrops, D0C-first-backdrop, room-slot, mask-after-floor/ceiling, and room-pass-order tests. It reuses the existing CSB room-slot selector, then pins the previously separate backdrop1 path: `pSkinDef[1]` bitmap lookup, `pSkinDef[5]` mask lookup by `roomNum`, draw-after-`pSkinDef[0]` ordering, and absorb-after-F0107 keepout.
