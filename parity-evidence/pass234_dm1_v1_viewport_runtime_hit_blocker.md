# Pass234 — DM1 PC34 viewport runtime-hit blocker

Status: `BLOCKED_RUNTIME_HITS_REQUIRED`.

Scope: source-lock viewport draw/present seams, inventory debugger availability, and keep runtime CS:IP promotion blocked until verified runtime hits exist.

## Source seam audit

- PASS `draw_uses_mutated_tuple` — `GAMELOOP.C:55-95` / `F0002_MAIN_GameLoop_CPSDF`; observable: main loop consumes G0308_i_PartyDirection/G0306_i_PartyMapX/G0307_i_PartyMapY in the F0128 draw call
  - line 55: `for (;;) { /*_Infinite loop_*/`
  - line 69: `F0261_TIMELINE_Process_CPSEF();`
  - line 84: `if (!G0300_B_PartyIsResting) {`
  - line 90: `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);`
- PASS `viewport_buffer_composed` — `DUNVIEW.C:8336-8611` / `F0128_DUNGEONVIEW_Draw_CPSF`; observable: I34E-family F0128 composes G0296_puc_Bitmap_Viewport from direction/X/Y before calling F0097
  - line 8337: `if (G0297_B_DrawFloorAndCeilingRequested) {`
  - line 8355: `G0074_puc_Bitmap_Temporary = F0606_AllocateMemForGraphic`
  - line 8357: `G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001`
  - line 8367: `F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);`
  - line 8542: `F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);`
  - line 8602: `F0607_FreeMemForGraphic(G0074_puc_Bitmap_Temporary);`
  - line 8610: `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);`
- PASS `viewport_present` — `DRAWVIEW.C:709-858` / `F0097_DUNGEONVIEW_DrawViewport`; observable: PC I34E-family F0097 resolves C007_ZONE_VIEWPORT and blits G0296_puc_Bitmap_Viewport via VIDRV_09_BlitViewPort
  - line 709: `void F0097_DUNGEONVIEW_DrawViewport(`
  - line 821: `switch (P0100_i_PaletteSwitchingRequestedState) {`
  - line 850: `F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);`
  - line 857: `(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);`

## Runtime/debugger inventory

- dosbox-debug: `/usr/bin/dosbox-debug`
- dosbox-x: `/usr/bin/dosbox-x`
- symbol map exists: `True`
- verified runtime hits in symbol map: `True`

## Blocker

Exact blocker: `blocked/runtime-base-and-symbol-map-unavailable` — Need FIRES.MAP/public symbols or another reproducible debugger binding from ReDMCSB source seams/globals to loaded original FIRES CS:IP/data addresses.

Required formula:
- `runtime_cs = (PSP + 0x10) + map_segment`
- `runtime_ip = map_offset`

Guardrail: No static source line, MZ entrypoint, or EXENEW offset may be promoted without verified_runtime_hit evidence.
