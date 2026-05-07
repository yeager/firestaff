# DM1 V1 runtime provenance trace source lock — 2026-05-06

Scope: worker-only source audit and compat trace for the chain
`command_accepted -> movement_applied -> viewport_present`.  This is source-locked
Firestaff/compat provenance, **not** DOSBox/original screenshot evidence and not a
pixel-parity claim.

Reference root:
`<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

## ReDMCSB anchors audited before implementation

- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2075-2099` locks the queue, handles
  empty queue, and leaves movement commands queued while movement/projectile
  cooldown gates are active.
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2118-2127` reads command X/Y,
  advances the circular queue, unlocks, and replays pending clicks.
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2150-2156` dispatches C001/C002
  turns to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and C003..C006 movement
  commands to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:325-328` calls
  `F0267_MOVE_GetMoveResult_CPSCE` for accepted stairs/non-stairs movement.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:330-346` computes and
  stores `G0310_i_DisabledMovementTicks` and clears projectile movement cooldown
  after an accepted step.
- `GAMELOOP.C:90` redraws the dungeon viewport with
  `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX,
  G0307_i_PartyMapY)` from the mutated party state.
- `DRAWVIEW.C:F0097_DUNGEONVIEW_DrawViewport:721-722` sets
  `G0324_B_DrawViewportRequested` and waits one vblank so the viewport is on
  screen when the function returns.
- `DRAWVIEW.C:F0097_DUNGEONVIEW_DrawViewport:1056-1068` blits
  `G0296_puc_Bitmap_Viewport` to `C007_ZONE_VIEWPORT` in both palette-change and
  no-palette-change branches.

## Firestaff change

Added `Dm1V1MovementPipelineProvenancePc34Compat` to
`dm1_v1_movement_pipeline_pc34_compat.h` and populate it in
`DM1_V1_MovementPipeline_ProcessOneTickPc34Compat`:

- `commandAccepted`: dequeued, handled, not blocked, and produced a turn/step/stair
  effect.
- `movementApplied`: step/stair/post-move transition was applied.
- `viewportPresent`: compat viewport dirty/redraw flag was raised for a movement or
  turn.
- `originalRuntimeObserved=0` and `noPixelParityClaim=1` are explicit guardrails so
  this trace cannot be promoted as original runtime screenshot evidence.

Verification is locked by
`test_dm1_v1_movement_pipeline_pc34_compat.c:test_command_movement_viewport_provenance`.
