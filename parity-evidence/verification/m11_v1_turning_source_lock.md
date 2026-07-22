# M11 DM1 V1 turning source lock

Status: **pass**

## ReDMCSB citations

- `COMMAND.C:396-405` — movement arrows map click zones to turn-left/turn-right and four movement commands
- `COMMAND.C:2045-2156` — queue gate blocks only movement commands C003..C006; turns dispatch to F0365
- `CLIKMENU.C:142-174` — turn command sets stop-waiting, highlights arrow, stairs take-stairs, otherwise sensors wrap one normalized 90-degree F0284 direction mutation
- `CHAMPION.C:117-130` — F0284 computes delta, rotates champion Cell/Direction by delta, stores G0308_i_PartyDirection
- `GAMELOOP.C:90-92` — main loop redraw uses current G0308_i_PartyDirection after command processing
- `GAMELOOP.C:215-219` — command processing loop stops after G0321_B_StopWaitingForPlayerInput is set by turn
- `DRAWVIEW.C:709-724` — viewport presentation is a requested draw plus vertical blank wait, not a multi-frame yaw animation loop
- `DUNGEON.C:35-44` — direction constants use north/east/south/west vector tables

## Firestaff evidence

- `src/engine/m11_v1_turning_presentation_pc34_compat.c` — guarded V1 presentation turning seam stores one-step 90-degree endpoint render semantics with no wall block check
- `src/dm1/dm1_v1_movement_command_core_pc34_compat.c` — turn dispatch uses guarded M11 V1 seam and handles stairs-before-turn source sequence
- `tests/test_m11_v1_turning_presentation_pc34_compat.c` — ctest asserts direction delta, one endpoint frame, no intermediate yaw frames, no wall block, V1 guard, and champion pose rotation

## Answers

- Steps per 90° turn: 1
- Facing storage: PartyState_Compat.direction mirrors G0308_i_PartyDirection; champion directions rotate by delta.
- Frames/timing: one endpoint viewport presentation, zero intermediate yaw frames, vertical-blank wait on viewport draw.
- Movement interaction: movement cooldown gate blocks only move commands C003..C006; turns still dispatch.
- Wall blocking: none for turning; current-square stairs are the only turn-command special case.
