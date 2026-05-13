# Pass506 - DM1 V1 alcove wall item occlusion evidence

Scope: DM1 V1 viewport/world visuals only. This pass consolidates the wall-cell item exception after the viewport/wall draw-order passes: a normal wall square occludes contents, but a center/front wall ornament that is an alcove draws items from the alcove sub-cell after the wall ornament.

## ReDMCSB source anchors

- DUNVIEW.C:834-837 defines the three alcove-capable ornament identities: square alcove, Vi altar, and arched alcove.
- DUNGEON.C:1330-1346 implements F0149_DUNGEON_IsWallOrnamentAnAlcove by comparing the current map wall ornament index against G0267_ai_CurrentMapAlcoveOrnamentIndices.
- DUNVIEW.C:3502-3589 starts F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF, resolves the wall ornament, and computes L0096_B_IsAlcove.
- DUNVIEW.C:4555-4582 documents F0115 cell-order semantics: a first nibble of zero means objects are drawn in an alcove on a wall square.
- DUNVIEW.C:4800-4823 converts the zero first nibble to L0135_B_DrawAlcoveObjects, C04_VIEW_CELL_ALCOVE, and M018_OPPOSITE(P0142_i_Direction).
- DUNVIEW.C:7119-7123 proves D2R front-wall alcoves switch to C0x0000_CELL_ORDER_ALCOVE before the object/creature/projectile stack.
- DUNVIEW.C:7840-7844 proves D1C center wall draws the wall, checks the alcove ornament, then calls F0115 with C0x0000_CELL_ORDER_ALCOVE.

## Firestaff source lock

- m11_game_view.c:m11_sample_viewport_cell must continue collecting item things even when the sampled cell is a wall. Open-floor render paths do their own element checks; alcove rendering needs wall-square items available.
- m11_game_view.c:m11_draw_dm1_alcove_wall_items must filter collected wall-square items to the ReDMCSB alcove sub-cell relative to the party, then draw them using the same item-sprite path as viewport item piles.
- m11_game_view.c:m11_draw_dm1_wall_ornaments must draw the wall ornament first and invoke the alcove item pass only for source-backed alcove global ornament indices.

## Result

This is an evidence/test pass. It does not claim pixel parity or new runtime capture. It prevents regressions where wall-square items are discarded before alcove rendering, where all wall-square items leak through instead of only the alcove sub-cell, or where alcove items draw before the wall ornament.
