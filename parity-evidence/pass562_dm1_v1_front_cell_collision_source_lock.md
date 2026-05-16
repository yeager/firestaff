# Pass562 - DM1 V1 front-cell collision source lock

Status: PASS562_DM1_V1_FRONT_CELL_COLLISION_SOURCE_LOCKED

## ReDMCSB source audit
- CLIKMENU.C:256-328 proves relative coordinate, stairs consequence, wall, door states 0/1/5, fake-wall OPEN/IMAGINARY, group blocker, blocked discard/VBlank/re-arm, then accepted F0267 move.
- DUNGEON.C:1371-1421 proves forward/right relative coordinate math.
- MOVESENS.C:738-818 proves accepted movement commits move-result globals and leave/enter sensors.

## Firestaff guard
- dm1_v1_movement_command_core_pc34_compat.c:266-355 keeps blockers before party tuple commit.
- memory_movement_pc34_compat.c:405-490 owns wall/door/fake-wall legality.
- memory_movement_pc34_compat.c:820-890 owns the champion-count gated group blocker.
- test_dm1_v1_movement_command_core_pc34_compat.c asserts closed door, closed fake-wall, and group blocked-command behavior.

Manifest: parity-evidence/verification/pass562_dm1_v1_front_cell_collision_source_lock/manifest.json
