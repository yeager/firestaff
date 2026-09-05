# Pass590 - DM1 V1 blocked wall/door self-damage source lock

Status: PASS590_DM1_V1_BLOCKED_WALL_DOOR_SELF_DAMAGE_SOURCE_LOCKED
Manifest: parity-evidence/verification/pass590_dm1_v1_blocked_wall_door_self_damage_source_lock/manifest.json

## ReDMCSB Source Audit
- CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty: lines 180-349, focused 278-323 - wall/door/closed real fakewall blocks request attack=1 self-damage to first and next target cells with torso|legs wound mask, then discard input/vblank/return before F0267/cooldown
- CHAMPION.C:F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage: lines 1803-1924, focused 1842-1911 - non-normal self attacks compute defense from the allowed wound mask and materialize pending wounds through that same mask

## Firestaff Gate
- Command core records attack=1, attackType=C2_ATTACK_SELF, allowedWounds=0x0018, first target cell, and next target cell for wall/door/closed-real-fakewall blocks.
- Focused CTest: dm1_v1_movement_command_core_pc34_compat.

## Not Claimed
- new original DOS runtime trace
- actual random wound materialization parity beyond the recorded request
- viewport or wall occlusion behavior
- CSB or DM2 movement behavior
