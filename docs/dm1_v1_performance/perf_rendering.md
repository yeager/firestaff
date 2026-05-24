# DM1 V1 — Rendering Pipeline

## ReDMCSB source

**Viewport draw entry**
- `GAMELOOP.C:80-90` / `F0002_MAIN_GameLoop_CPSDF`:  
  Calls `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)` when party is not resting and inventory is closed.

**Dungeon view construction**
- `DUNVIEW.C:8318-8618` / `F0128_DUNGEONVIEW_Draw_CPSF`:  
  Builds the full viewport buffer (far-to-near cell draw order).  
  Calls `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)` at line 8608–8611 to request presentation.

**Presentation and VBLANK sync**
- `DRAWVIEW.C:709-723` / `F0097_DUNGEONVIEW_DrawViewport`:  
  1. Sets `G0324_B_DrawViewportRequested = C1_TRUE` — triggers blit in VBLANK handler
  2. Calls `M526_WaitVerticalBlank()` — blocks until next VBLANK so the bitmap is on screen when function returns

**VBLANK handler blit**
- `BASE.C:830-1075` / `E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF`:  
  When `G0324_B_DrawViewportRequested` is true, blits `G0296_puc_Bitmap_Viewport` to the screen.  
  Also runs palette switching via Timer B at a programmed scan line (line ~31 below champion status area).

**Per-frame budget**
There is no fixed frame budget enforcement in the source. The architecture uses interrupt-driven VBLANK:
- Composition (`F0128`) runs in main loop time (non-realtime)
- Presentation (`F0097`) blocks on next VBLANK
- Actual screen blit happens in the VBLANK exception handler (IRQ4, level 4)

**BUG0_03** (BASE.C:837 comment): if VBLANK fires while a previous VBLANK is still executing, the second is masked out by the CPU's automatic interrupt priority, causing momentary palette glitches. Fixed in CSB v2.1 for S20E/S21E via `G0351_i_ConcurrentVerticalBlankExceptionCount` counter and mask-drop to level 3.

**BUG0_03 impact**: DM1 V1 (pre-fix) can briefly display the wrong palette on heavily-loaded frames. Firestaff does not model this glitch.

## Firestaff coverage
- `dm1_v1_viewport_3d_pc34_compat.c` — full DUNVIEW draw chain
- `m11_game_view.c:6175-6291` / `m11_apply_dm1_v1_pipeline_tick` — mirrors F0002 game loop tick in Firestaff
- `dm1_v1_movement_pipeline_pc34_compat.c:244-443` — turn/step pipeline with viewport dirty provenance

## Status
✅ SOURCE-LOCKED — rendering pipeline documented with file:line citations.
