# Pass620 DM1 V1 side-wall inscription audit

Scope: DM1 V1 side-wall inscription rendering for D3L/R and D2L/R only. This is an evidence-only source-lock pass; no renderer behavior changed.

## ReDMCSB audit

- `DUNGEON.C:2568-2594` scans wall-square things in `F0172_DUNGEON_SetSquareAspect`. Visible `C02_THING_TYPE_TEXTSTRING` entries write `G0265_i_CurrentMapInscriptionWallOrnamentIndex + 1` into the side-specific square-aspect ornament slot and store the visible text thing in `G0290_T_DungeonView_InscriptionThing`.
- `DUNVIEW.C:3589-3593` enters `F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF`, checks the current-map inscription ornament index, and decodes `G0290_T_DungeonView_InscriptionThing` with `C0_TEXT_TYPE_INSCRIPTION`.
- `DUNVIEW.C:3608-3717` draws readable inscription glyphs only when `P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT`, then jumps to the done path. D3/D2 side projections do not take this readable path.
- `DUNVIEW.C:3864-3902` handles decoded inscription line counts for non-D1C projections. When the inscription has fewer than four lines, PC34 sets `MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR`, `G2154_i_ZoneShiftX`, and `G2155_i_ZoneShiftY` before `F0791_DUNGEONVIEW_DrawBitmapXX` clips the unreadable plaque.
- `DUNVIEW.C:1327-1332` defines `G0204_auc_Graphic558_UnreadableInscriptionBoxY2`: D3 side `5/8/13`, D3 front `7/13/20`, D2 side `5/12/19`, D2 front `10/17/27`, D1 side `11/22/33`.
- `DUNVIEW.C:6432`, `6568`, `6968`, and `7119` call `F0107` for D3L right, D3R left, D2L right, and D2R left side-wall ornament projections.

## Firestaff binding checked

- `src/engine/m11_game_view.c:11535-11542` decodes visible wall text and counts inscription lines.
- `src/engine/m11_game_view.c:11544-11577` mirrors the PC34 unreadable plaque height table and row selection for D3 side/front, D2 side/front, and D1 side projections.
- `src/engine/m11_game_view.c:11717-11732` enumerates D3 and D2 side ornament specs, including the D3L/R and D2L/R side-wall view indices.
- `src/engine/m11_game_view.c:11815-11825` applies the unreadable inscription height only for inscription ornament `0` on non-D1C projections before loading and drawing the ornament bitmap.
- `src/engine/m11_game_view.c:21190-21194` draws side walls and then wall ornaments in the main viewport pass; `src/engine/m11_game_view.c:21223-21226` replays near side walls and ornaments after a blocking center wall/door.
- `tools/verify_pass582_dm1_v1_side_inscription_source_lock.py` gates the ReDMCSB anchors, Firestaff height table, non-D1C clipping path, and CMake registration.
- `CMakeLists.txt:2821-2829` registers `pass582_dm1_v1_side_inscription_source_lock` as a CTest.

## Conclusion

No new D3L/R or D2L/R implementation gap was found. Main already has equivalent behavior for the slice: visible wall text becomes the current-map inscription ornament in square aspect data, side-wall projections use unreadable plaque rendering rather than readable front-wall glyph rendering, and Firestaff clips the unreadable plaque height from the same PC34 source table.

## Verification

Focused gates for this audit:

```sh
python3 tools/verify_pass582_dm1_v1_side_inscription_source_lock.py
cmake -S . -B build -DBUILD_TESTING=ON
ctest --test-dir build -R pass582_dm1_v1_side_inscription_source_lock --output-on-failure
git diff --check
```
