# Pass298 — DM1 V2 entry bitmap-materialization blocker

Pixel parity is **not** claimed. This pass locks the exact missing bitmap seam for the current `16405/30464` entry viewport mismatch.

## Required original GRAPHICS.DAT assets

- `floor.base`: `GRAPHICS.DAT` index `78` / `M650_GRAPHIC_FLOOR_SET_0_FLOOR` -> `G2108_Floor`; zone `C701_ZONE_VIEWPORT_FLOOR_AREA`; decode `F0490_MEMORY_LoadDecompressAndExpandGraphic(78, F0631_GetBitmapPointer(G2108_Floor))`; blit `F0792_DUNGEONVIEW_DrawBitmapYYY(G2108_Floor, C701_ZONE_VIEWPORT_FLOOR_AREA, flip)`.
- `ceiling.base`: `GRAPHICS.DAT` index `79` / `M651_GRAPHIC_FLOOR_SET_0_CEILING` -> `G2109_Ceiling`; zone `C700_ZONE_VIEWPORT_CEILING_AREA`; decode `F0490_MEMORY_LoadDecompressAndExpandGraphic(79, F0631_GetBitmapPointer(G2109_Ceiling))`; blit `F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport) or F0792_DUNGEONVIEW_DrawBitmapYYY(G2109_Ceiling, C700_ZONE_VIEWPORT_CEILING_AREA, flip)`.
- `wall.front.blocking.d3c`: `GRAPHICS.DAT` index `107` / `C107_GRAPHIC_WALLSET_0_D3C` -> `G2107_WallSet[C14_WALL_D3C]`; zone `C704_ZONE_WALL_D3C`; decode `F0490_MEMORY_LoadDecompressAndExpandGraphic(107, F0631_GetBitmapPointer(G2107_WallSet[C14_WALL_D3C]))`; blit `F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C, flip)`.

## Source-locked route

- `DUNVIEW.C:8337-8368` starts viewport rebuild and materializes ceiling/floor bitmaps into `G0296_puc_Bitmap_Viewport` / floor area.
- `DUNVIEW.C:8490-8542` walks D3/D2/D1/D0 visible squares for route state `map=0 x=1 y=3 dir=2`.
- `DUNVIEW.C:6708-6720` draws the entry front blocking wall through `F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C, ...)` and returns.
- `DUNVIEW.C:3288-3301` is the I34E bitmap-zone clipping/blit function; it must replace the current two-color symbolic rectangle path.

## Repro

```sh
cmake -S . -B build-pass298
cmake --build build-pass298 --target dm1_v2_export_entry_viewport_png
ctest --test-dir build-pass298 --output-on-failure -R "dm1_v2_(entry_viewport_png_export_gate|entry_viewport_png_comparator_gate|completion_matrix)"
python3 tools/verify_dm1_v2_entry_bitmap_materialization_blocker.py
```

Evidence JSON: `parity-evidence/verification/pass298_dm1_v2_entry_bitmap_materialization_blocker.json`.
