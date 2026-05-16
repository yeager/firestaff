# Pass544 - DM1 V1 blocked collision queue lifecycle

Status: PASS544_DM1_V1_BLOCKED_COLLISION_QUEUE_LIFECYCLE_LOCKED

## Why this is not pass542 again
- Pass542 already locks the pre-dequeue movement-disabled gate: cooldown/projectile timing leaves the front movement command queued and replays pending click.
- This pass locks the next boundary: cooldown is clear, the movement command is dequeued and dispatched to F0366, then wall/door collision blocks the step and F0357 discards ordinary queued/pending input while preserving reserved release/stop commands.

## ReDMCSB-first source audit
- COMMAND.C:2045-2829 / F0380 dequeues the front command, unlocks/replays pending click, then dispatches movement to F0366.
- CLIKMENU.C:180-351 / F0366 detects blocked wall/door/fake-wall/group collision, calls F0357, waits one PC-34 VBlank, keeps input wait armed, and returns before successful-step timing/cooldown.
- COMMAND.C:1305-1378 / F0357 flushes input, preserves C129/C254 reserved release/stop commands, unlocks, and replays one pending click.

## Firestaff guards
- dm1_v1_input_command_queue_pc34_compat.c:286-328 pops and replays pending input before move dispatch.
- dm1_v1_input_command_queue_pc34_compat.c:202-229 preserves only reserved release/stop commands during discard and replays pending click after unlock.
- dm1_v1_movement_command_core_pc34_compat.c:182-376 returns from blocked collision before successful movement timing/cooldown.
- test_dm1_v1_movement_command_core_pc34_compat covers: front move dequeued after cooldown clears, wall collision blocks, trailing nonreserved turn is dropped, pre-existing reserved release and replayed pending stop survive, one blocked VBlank is requested, input wait remains armed, and no successful-step cooldown is assigned.

## Scope guard
- DM1 V1 movement/förflyttning only. No original runtime capture, viewport/walls, damage RNG, or pixel parity claim.

Manifest: parity-evidence/verification/pass544_dm1_v1_blocked_collision_queue_lifecycle/manifest.json
