# DM1 V1 ReDMCSB viewport pixel-capture / occlusion probe — 2026-05-08

Scope: source-only audit on N2 against `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

## Conclusion

ReDMCSB builds the dungeon view in the off-screen viewport bitmap (`G0296_puc_Bitmap_Viewport`) and then copies that bitmap to screen. Occlusion is not represented by a retained z-buffer or explicit clipping mask. It is encoded by draw order plus transparent blits: farther cells are drawn first, nearer cells later overwrite them, and wall-square cases return early after drawing the blocking wall (except alcove ornament cases that deliberately continue into object/ornament draw paths).

For Firestaff viewport capture, the trustworthy capture boundary is the completed `G0296_puc_Bitmap_Viewport` immediately before/at `F0097_DUNGEONVIEW_DrawViewport`, not the final display surface after UI/copper/message interrupts. Entrance/title paths are separate compositing paths and must not be treated as ordinary dungeon viewport occlusion.

## Evidence

### Viewport buffer and screen blit boundary

- `DRAWVIEW.C:709-723` (`F0097_DUNGEONVIEW_DrawViewport`) on ST-like media sets `G0324_B_DrawViewportRequested = C1_TRUE` and waits VBlank so the already-built `G0296_puc_Bitmap_Viewport` is displayed.
- `DRAWVIEW.C:821-863` on PC/Towns-style media switches palette then blits `G0296_puc_Bitmap_Viewport` to `C007_ZONE_VIEWPORT` via `F0021_MAIN_BlitToScreen` or `VIDRV_09_BlitViewPort`.
- `DRAWVIEW.C:1033-1070` on Amiga-style media wraps the screen blit in `OwnBlitter(); Forbid(); WaitBlit(); ... Permit();` and sets `G3126_B_ViewportIsBeingBlittedToScreen` while `F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT, CM1_COLOR_NO_TRANSPARENCY)` runs.
- `DRAWVIEW.C:1077-1100` has the IIgs direct draw routine and comments: height `136`, width `224`, source is `G0296_puc_Bitmap_Viewport`, start line `33` on screen.

### Interrupt exclusion during viewport blit

- `DRAWVIEW.C:18` defines `BOOLEAN G3126_B_ViewportIsBeingBlittedToScreen`.
- `VBLANK.C:210-276` updates/scolls message area only when `G3222_B_MessageAreaDataInitialized && !G3126_B_ViewportIsBeingBlittedToScreen`.
- `VBLANK.C:283-295` also defers copy-protection/floppy timing work while viewport blit is active.
- `COPERINT.C:23-33` updates the mouse pointer only when `!G3126_B_ViewportIsBeingBlittedToScreen`.

This means original Amiga logic treats viewport-to-screen copy as a critical section against unrelated screen mutations. That reinforces using `G0296_puc_Bitmap_Viewport` as the pixel-capture artifact.

### Floor/ceiling base and draw order

- `DUNVIEW.C:2962-3003` (`F0098_DUNGEONVIEW_DrawFloorAndCeiling`) initializes the viewport buffer: clears the black area, copies ceiling/floor into `G0296_puc_Bitmap_Viewport`, then clears `G0297_B_DrawFloorAndCeilingRequested`.
- `DUNVIEW.C:8318-8444` (`F0128_DUNGEONVIEW_Draw_CPSF`) optionally refreshes floor/ceiling, allocates temporary bitmap storage, sets flipped wall variants, and establishes the base image.
- `DUNVIEW.C:8466-8542` then draws cells in far-to-near order:
  - D4 objects: `M598/M599/M597_VIEW_SQUARE_D4*` at `DUNVIEW.C:8468-8477`
  - D3 side/center squares: `F0116`, `F0117`, `F0118` at `DUNVIEW.C:8488-8499`
  - D2 side/center squares: `F0119`, `F0120`, `F0121` at `DUNVIEW.C:8510-8521`
  - D1 side/center squares: `F0122`, `F0123`, `F0124` at `DUNVIEW.C:8522-8533`
  - D0 side/center squares: `F0125`, `F0126`, `F0127` at `DUNVIEW.C:8534-8542`
- `DUNVIEW.C:8604-8616` calls `F0097_DUNGEONVIEW_DrawViewport(...)` after drawing and then anticipates the next draw by repainting floor/ceiling if not in entrance.

### Wall/door/object occlusion mechanics

- `DUNVIEW.C:3048-3061` (`F0100_DUNGEONVIEW_DrawWallSetBitmap`) transparent-blits wall-set bitmaps into `G0296_puc_Bitmap_Viewport` using transparent color `C10_COLOR_FLESH`.
- `DUNVIEW.C:3064-3078` (`F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency`) uses `CM1_COLOR_NO_TRANSPARENCY` only for center wall performance cases.
- `DUNVIEW.C:3082-3094` (`F0102_DUNGEONVIEW_DrawDoorBitmap`) draws door bitmap from `G0074_puc_Bitmap_Temporary` into the viewport with transparency.
- `DUNVIEW.C:3113-3156` (`F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap`) draws floor/pit/stairs native bitmaps into the same viewport target, again with transparency on the old media paths.
- D3 wall examples show the occlusion gate:
  - `DUNVIEW.C:6406-6437` (`F0116_DUNGEONVIEW_DrawSquareD3L`) draws a D3-left wall, draws relevant wall ornaments, continues only if front wall ornament is an alcove, otherwise `return`.
  - `DUNVIEW.C:6545-6573` (`F0117_DUNGEONVIEW_DrawSquareD3R`) mirrors that behavior for D3-right.
- Door drawing deliberately composes masks/ornaments into temporary door bitmap before drawing the door:
  - `DUNVIEW.C:4255-4263` copies native door bitmap into `G0074_puc_Bitmap_Temporary` and draws door ornament into that temp image.
  - `DUNVIEW.C:4289-4295` applies the Thieves Eye mask before door draw; source comment notes this known original behavior/bug.
  - `DUNVIEW.C:4297-4314` draws closed/destroyed/opening door frames into viewport via `F0102_DUNGEONVIEW_DrawDoorBitmap`.
  - `DUNVIEW.C:4333-4335` performs the newer media equivalent via `F0791_DUNGEONVIEW_DrawBitmapXX(..., G0296_puc_Bitmap_Viewport, ...)`.

### Entrance/title are not normal dungeon viewport captures

- `ENTRANCE.C:178-187` copies the viewport into the entrance-door animation buffer using `G0006_ai_Graphic562_Box_Entrance_DungeonView`.
- `DATA.C:125-127` defines that box as `{ 0, 223, 3, 138 }`, i.e. 224 px wide with vertical placement inside the 161-line entrance-door buffer; not the same as screen viewport origin.
- `ENTRANCE.C:191-231` then draws left/right doors over that captured dungeon view; `ENTRANCE.C:296-299` blits the composite entrance step to screen.
- `ENTRANCE.C:317-350` newer media path does the same order: resolve left-door/right-door/viewport zones (`F0788_`), copy interface entrance screen, draw visible dungeon view, then draw left and right door overlays.
- `ENTRANCE.C:367` calls `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)` after entrance animation on some media.
- `TITLE.C` has no `G0296_puc_Bitmap_Viewport` references. It blits title art to `G0348_Bitmap_Screen` or temporary title buffers directly (`TITLE.C:111-129`, `TITLE.C:188-236`, `TITLE.C:321-457`). Title rendering should be excluded from dungeon viewport occlusion validation.

## Firestaff implications

1. Capture/compare dungeon viewport pixels at logical viewport size `224x136`, after the complete far-to-near `F0128_DUNGEONVIEW_Draw_CPSF` equivalent and before any UI/message/mouse/screen compositing.
2. Preserve original draw-order semantics: far cells first, near cells later, and wall-square early returns block behind-cell content unless the original alcove path explicitly continues.
3. Do not infer occlusion from final screen pixels during entrance/title sequences. Entrance composes `G0296_puc_Bitmap_Viewport` under doors in a separate buffer; title does not use the viewport at all.
4. Treat `G3126_B_ViewportIsBeingBlittedToScreen` as original evidence that viewport blit is an atomic presentation boundary, not as dungeon occlusion state.
