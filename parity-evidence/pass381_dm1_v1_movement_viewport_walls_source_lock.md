# Pass381 — DM1 V1 movement/viewport/walls source-lock

Date: 2026-05-08
Branch: worker/dm1v1-pass381-viewport-walls-20260508
Scope: ReDMCSB source audit + verifier only. No renderer/runtime behavior changes.

## Verdict

`PASS381_DM1_V1_MOVEMENT_VIEWPORT_WALLS_SOURCE_LOCKED`

This pass source-locks the path that matters for movement/viewport/wall parity: queued turn/move commands mutate the party state, the viewport redraw samples from that state, and wall/door/content drawing is ordered source replay into the viewport bitmap rather than a flat primitive depth sort.

## Primary ReDMCSB audit anchors

Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `COMMAND.C:2045-2156` — `F0380_COMMAND_ProcessQueue_CPSC` dequeues commands and dispatches `C001/C002` to `F0365_COMMAND_ProcessTypes1To2_TurnParty`, and `C003..C006` to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKMENU.C:142-174` — `F0365_COMMAND_ProcessTypes1To2_TurnParty` processes sensors around the party square and calls `F0284_CHAMPION_SetPartyDirection(...)` for left/right turns.
- `CLIKMENU.C:180-347` — `F0366_COMMAND_ProcessTypes3To6_MoveParty` computes relative target coordinates via `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`, blocks walls/closed doors/fake walls/groups, then applies movement through `F0267_MOVE_GetMoveResult_CPSCE`.
- `CHAMPION.C:93-131` — `F0284_CHAMPION_SetPartyDirection` rotates champion cells/directions and assigns `G0308_i_PartyDirection`, binding viewport orientation to party/champion state.
- `MOVESENS.C:315-560` — `F0267_MOVE_GetMoveResult_CPSCE` updates `G0306_i_PartyMapX/G0307_i_PartyMapY`; teleporter rotation reuses `F0284`; visible pit-fall traversal calls `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, ...)`.
- `DUNVIEW.C:8318-8542` — `F0128_DUNGEONVIEW_Draw_CPSF` starts from floor/ceiling, samples relative map coordinates from the current direction, then draws D3/D2/D1/D0 visible squares before presentation.
- `DUNVIEW.C:6361-6835` — `F0116/F0117/F0118` D3 wall/door branches draw wall zones and normally return for walls; door-front branches split rear contents, door/frame, then front contents with explicit cell-order constants.
- `DUNVIEW.C:4547-4910` — `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` consumes encoded cell-order nibbles and rescans square things by cell, proving ordered content replay.
- `DRAWVIEW.C:709-722` — `F0097_DUNGEONVIEW_DrawViewport` sets `G0324_B_DrawViewportRequested` and waits for VBlank.
- `BASE.C:961-987` — the VBlank handler clears `G0324_B_DrawViewportRequested` and copies `G0296_puc_Bitmap_Viewport` to the screen viewport region.

## Landed verifier

- `tools/verify_pass381_dm1_v1_movement_viewport_walls_source_lock.py`

The verifier checks all anchors above and chains the existing `scripts/verify_dm1_v1_viewport_wall_draw_order_source_lock.py` gate so the movement/turn path and existing wall draw-order source lock fail together. It is registered as CTest `pass381_dm1_v1_movement_viewport_walls_source_lock`, keeping this source-lock visible in the standard DM1 V1 viewport gate set instead of relying on manual script runs.

## Scope guard

- This is not a pixel-parity claim.
- This is not an original DOSBox debugger-hit capture.
- This is a source-lock/metadata gate for the movement → viewport redraw → wall/occlusion ordering path.
