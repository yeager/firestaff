# Pass506 - DM1 V1 stairs movement side-effect source lock

Status: PASS506_DM1_V1_STAIRS_MOVEMENT_SIDE_EFFECT_SOURCE_LOCK_PROVEN

## ReDMCSB-first source audit
- CLIKMENU.C:124-140 / F0364 takes stairs by calling F0267 with a non-square destination, resolving level/offset, setting the target map for exit-direction lookup, then restoring the current map.
- CLIKMENU.C:142-174 / F0365 turns on a stairs square by taking stairs and returning before same-square turn leave/enter sensors.
- CLIKMENU.C:180-349 / F0366 handles backward-on-stairs before relative stepping, target-stairs before normal blockers/cooldown, and source-stairs normal steps with a non-square source before normal cooldown.
- DUNGEON.C:1508-1559 and DUNGEON.C:1560-1582 bind stairs target map/coordinates and exit direction.

## Firestaff guards
- dm1_v1_movement_command_core_pc34_compat.c now records source-stairs walk-off suppression for normal steps while preserving destination walk-on and cooldown.
- test_dm1_v1_movement_pipeline_pc34_compat.c covers target-stairs, backward-on-stairs, and source-stairs-to-corridor step semantics.

## Scope guard
- Movement-only evidence. No viewport, capture, or artwork lane changes.

Manifest: parity-evidence/verification/pass506_dm1_v1_stairs_movement_side_effect_source_lock/manifest.json
