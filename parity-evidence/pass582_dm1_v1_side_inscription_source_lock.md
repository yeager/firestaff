# Pass582 DM1 V1 side-wall inscription source lock

Scope: DM1 V1 viewport wall inscription rendering for side projections, specifically D3L/R and D2L/R side walls.

## ReDMCSB source anchors

- `DUNGEON.C:2568-2594` (`F0172_DUNGEON_SetSquareAspect`) scans wall square TextString things; when a visible TextString is found, PC34 writes `G0265_i_CurrentMapInscriptionWallOrnamentIndex + 1` into the side-specific square-aspect ornament slot and captures the TextString in `G0290_T_DungeonView_InscriptionThing`.
- `DUNVIEW.C:3589-3593` (`F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF`) identifies the current-map inscription ornament and decodes `G0290_T_DungeonView_InscriptionThing` as `C0_TEXT_TYPE_INSCRIPTION`.
- `DUNVIEW.C:3608-3717` draws readable glyphs only for `M587_VIEW_WALL_D1C_FRONT`; other projections continue to the ordinary unreadable ornament path.
- `DUNVIEW.C:3864-3902` counts decoded inscription lines for non-D1C plaques. For fewer than four lines, it sets `MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR`, `G2154_i_ZoneShiftX`, and `G2155_i_ZoneShiftY` before `F0791_DUNGEONVIEW_DrawBitmapXX`.
- `DUNVIEW.C:1327-1332` is the PC34/I34E `G0204_auc_Graphic558_UnreadableInscriptionBoxY2` height table: D3 side `5/8/13`, D3 front `7/13/20`, D2 side `5/12/19`, D2 front `10/17/27`, D1 side `11/22/33`.
- `DUNVIEW.C:6432`, `6568`, `6968`, and `7119` call `F0107` for D3L right, D3R left, D2L right, and D2R left side-wall ornaments respectively.

## Firestaff binding

- `src/engine/m11_game_view.c` now counts decoded visible wall inscription lines and applies the source PC34 unreadable-plaque height table before drawing non-D1C inscription ornaments.
- The side/front row selection is based on the existing wall-ornament render spec: D3 side projections use the D3 side row, D2 side projections use the D2 side row, and D1C remains on the existing readable front-wall text route.
- `tools/verify_pass582_dm1_v1_side_inscription_source_lock.py` gates the source anchors, the height rows, and the Firestaff render-path ordering.

## Verification

Source-lock gate:

```sh
python3 tools/verify_pass582_dm1_v1_side_inscription_source_lock.py
```
