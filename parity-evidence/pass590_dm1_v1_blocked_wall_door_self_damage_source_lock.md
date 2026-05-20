# Pass590 - DM1 V1 blocked wall/door self-damage source lock

Status: PASS590_DM1_V1_BLOCKED_WALL_DOOR_SELF_DAMAGE_SOURCE_LOCKED
Manifest: parity-evidence/verification/pass590_dm1_v1_blocked_wall_door_self_damage_source_lock/manifest.json

## ReDMCSB Source Audit
- CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty: lines 180-349, focused 278-323 - wall/door/closed real fakewall blocks request attack=1 self-damage to first and next target cells with torso|legs wound mask, then discard input/vblank/return before F0267/cooldown
- CHAMPION.C:F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage: lines 1803-1924, focused 1842-1911 - non-normal self attacks compute defense from the allowed wound mask and materialize pending wounds through that same mask

## Firestaff Gate
- Command core records attack=1, attackType=C2_ATTACK_SELF, allowedWounds=0x0018, first target cell, and next target cell for wall/door/closed-real-fakewall blocks.
- Focused CTest: dm1_v1_movement_command_core_pc34_compat.

## Reference Anchors
- DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- Greatstone overview sha256 7fdd2c8daef24250d58bc35632e245def338c0e63cf3832ce9af6534da54896c

## Not Claimed
- new original DOS runtime trace
- actual random wound materialization parity beyond the recorded request
- viewport or wall occlusion behavior
- CSB or DM2 movement behavior
