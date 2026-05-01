# Pass 181 — DM1 V1 front-wall perspective/source-row gate

Scope: source-shape gate only; no renderer implementation change.

## ReDMCSB source audit

- `DUNVIEW.C:582-590` defines `G0163_aauc_Graphic558_Frame_Walls`; D1C is `{32,191,9,119,128,111,48,0}`, i.e. viewport destination x/y extent plus source row/height fields.
- `DUNVIEW.C:3048-3058` (`F0100_DUNGEONVIEW_DrawWallSetBitmap`) blits wall-set bitmaps using the frame row fields `C6_X`, `C7_Y`, `C4_BYTE_WIDTH`, and `C5_HEIGHT`.
- `DUNVIEW.C:3065-3075` (`F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency`) is the center-wall no-transparency variant and uses the same frame row fields.
- `DUNVIEW.C:7727-7872` (`F0124_DUNGEONVIEW_DrawSquareD1C`) handles D1C; its wall case draws `G0163_aauc_Graphic558_Frame_Walls[M606_VIEW_SQUARE_D1C]`, then gates alcove wall contents through `F0107...`/`F0115...`, and returns so deeper center content is occluded.

## Firestaff gate

Strengthened `tools/verify_v1_viewport_draw_order_gate.py` to lock:

- ReDMCSB D1C frame row/source-row evidence above.
- Firestaff D1C/D2C/D3C front-wall perspective rectangles in `m11_draw_dm1_front_walls`.
- Center-front-wall occlusion via the `occluded = 1` path.
- The clipped extracted-asset blit seam in `m11_draw_dm1_wall_blit_with_transparency` (`slot` dimensions must match the source rectangle before `M11_AssetLoader_BlitRegion`).

## Verification

```sh
python3 tools/verify_v1_viewport_draw_order_gate.py
```

Result: PASS.
