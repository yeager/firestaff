# Pass230 — DM1 V1 FIRES.EXENEW symbol-map bootstrap

Classification: `blocked/source-symbol-map-and-debugger-hits-required`
Exact remaining blocker: FIRES.EXENEW now supplies the decompressed runtime image layout, but no N2-local FIRES.MAP/public-symbol table or debugger-observed source seam CS:IP/global addresses bind ReDMCSB functions to the loaded image.

## What is now unblocked
- Decompressed FIRES image evidence: sha256 `fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94`, size `178224`, header bytes `10240`, relocations `2518`.
- Loaded image bytes excluding MZ header: `167984`.
- Entry relative CS:IP from FIRES.EXENEW: `0000:0000`.
- Runtime-base formula is now explicit: `program_load_segment = PSP + 0x10`; map segment:offsets bind as `runtime_cs = program_load_segment + map_segment`, `runtime_ip = map_offset`.

## ReDMCSB source seams re-audited
- PASS `command_accepted` — `COMMAND.C:2045-2156` `F0380_COMMAND_ProcessQueue_CPSC`; observable: queue dequeue plus turn/step/click dispatch after accepted command id is read; breakpoint anchor: after COMMAND.C:2126 queue unlock or before COMMAND.C:2150 turn/move dispatch
  - COMMAND.C:2075-2085 locks/emptiness-checks the queue
  - COMMAND.C:2095 reads accepted command id from G0432_as_CommandQueue
  - COMMAND.C:2118-2126 reads X/Y, decrements queued count, advances first index, unlocks queue
  - COMMAND.C:2150-2156 dispatches turn or movement handlers
- PASS `turn_or_step_state_applied` — `CLIKMENU.C:142-328` `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty`; observable: party direction or destination state after turn/step handler; breakpoint anchor: after CLIKMENU.C:173 for turns; after CLIKMENU.C:328 for steps
  - CLIKMENU.C:171-173 removes/adds party sensor state around F0284_CHAMPION_SetPartyDirection
  - CLIKMENU.C:264-269 derives move destination from G0308_i_PartyDirection and movement-arrow deltas
  - CLIKMENU.C:325-328 commits legal movement through F0267_MOVE_GetMoveResult_CPSCE
- PASS `party_coordinates_committed` — `MOVESENS.C:316-556` `F0267_MOVE_GetMoveResult_CPSCE`; observable: party X/Y global writes for successful move result; breakpoint anchor: memory write watchpoint on G0306_i_PartyMapX/G0307_i_PartyMapY, source-anchored at MOVESENS.C:442-443
  - MOVESENS.C:438-443 enters destination path and writes G0306_i_PartyMapX/G0307_i_PartyMapY
  - MOVESENS.C:493-506 rewrites party X/Y/direction again for teleporter destinations
  - MOVESENS.C:556 redraws during fall using G0308_i_PartyDirection and destination X/Y
- PASS `draw_uses_mutated_tuple` — `GAMELOOP.C:35-91` `F0002_MAIN_GameLoop_CPSDF`; observable: post-command draw call consumes direction/X/Y tuple; breakpoint anchor: GAMELOOP.C:90 call to F0128_DUNGEONVIEW_Draw_CPSF with the mutated direction/X/Y tuple
  - GAMELOOP.C:55-63 begins each main-loop iteration and handles new map movement result
  - GAMELOOP.C:80-90 gates drawing while not resting/in inventory and passes G0308/G0306/G0307 to F0128
- PASS `viewport_buffer_composed` — `DUNVIEW.C:8318-8610` `F0128_DUNGEONVIEW_Draw_CPSF`; observable: dungeon view path composes viewport buffer from current party tuple and requests present; breakpoint anchor: before DUNVIEW.C:8608-8611 after buffer composition and before F0097_DUNGEONVIEW_DrawViewport
  - DUNVIEW.C:8336-8356 prepares floor/ceiling/clickable/temporary viewport buffers
  - DUNVIEW.C:8357-8368 derives flip state from map X/Y plus direction
  - DUNVIEW.C:8466-8542 walks relative squares and draws D4..D0 into G0296_puc_Bitmap_Viewport
  - DUNVIEW.C:8608-8611 requests viewport presentation for I34E/I34M/PC-family builds
- PASS `viewport_present` — `DRAWVIEW.C:709-858` `F0097_DUNGEONVIEW_DrawViewport`; observable: C007 viewport-zone blit presents the composed viewport bitmap; breakpoint anchor: DRAWVIEW.C:857 PC-family VIDRV_09_BlitViewPort call with G0296_puc_Bitmap_Viewport
  - DRAWVIEW.C:709-713 enters F0097_DUNGEONVIEW_DrawViewport
  - DRAWVIEW.C:821-839 resolves requested dungeon/inventory palette state
  - DRAWVIEW.C:849-857 resolves C007_ZONE_VIEWPORT and calls VIDRV_09_BlitViewPort for I34E/I34M/P31J

## Still blocked
- No FIRES.MAP/TLINK public-symbol table or debugger-observed CS:IP/global addresses exist for the seams above.
- The packed loader entry and static compressed offsets remain non-promotable.

## Artifacts
- `/home/trv2/work/firestaff/parity-evidence/verification/pass230_dm1_v1_fires_exenew_symbol_map_bootstrap/manifest.json`
- `/home/trv2/work/firestaff/parity-evidence/verification/pass230_dm1_v1_fires_exenew_symbol_map_bootstrap/runtime_address_formula.json`
- `/home/trv2/work/firestaff/parity-evidence/verification/pass230_dm1_v1_fires_exenew_symbol_map_bootstrap/symbol_map_gap.json`
- `/home/trv2/work/firestaff/parity-evidence/verification/pass230_dm1_v1_fires_exenew_symbol_map_bootstrap/breakpoint_promotion_contract.md`
