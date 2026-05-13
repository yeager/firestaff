# Pass505 - DM1 V1 blocked movement side-effect source lock

Status: PASS505_DM1_V1_BLOCKED_MOVEMENT_SIDE_EFFECT_SOURCE_LOCK_PROVEN

## ReDMCSB-first source audit
- CLIKMENU.C:180-349 / F0366_COMMAND_ProcessTypes3To6_MoveParty is the primary source.
- CLIKMENU.C:237-322 proves the blocked-step order: input wait is armed, living champions spend stamina, legality runs, blocked wall/door/fake-wall damage is requested, input is discarded, PC-34 waits one VBlank, input wait is re-armed by setting G0321_B_StopWaitingForPlayerInput = C0_FALSE, and the function returns before accepted-move/cooldown code.

## Firestaff executable guards
- dm1_v1_movement_command_core_pc34_compat.c:191-266 keeps the same command-core blocked branch order.
- dm1_v1_movement_command_core_pc34_compat.c:25-52 records the source-locked self-damage request seam without claiming combat RNG/wound materialization.
- build/test_dm1_v1_movement_command_core_pc34_compat asserts blocked movement spends stamina, flushes queued input, does not release wait/redraw, and records the wall/door self-damage request fields.

## Scope guard
- This gate advances movement legality/timing/source-lock completion only for blocked-step side effects. It does not touch viewport/walls or original-capture lanes.

Manifest: parity-evidence/verification/pass505_dm1_v1_blocked_movement_side_effect_source_lock/manifest.json
