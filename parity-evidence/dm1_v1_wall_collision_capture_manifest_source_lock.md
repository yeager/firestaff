# DM1 V1 wall/collision capture manifest source lock

Status: DM1_V1_WALL_COLLISION_CAPTURE_MANIFEST_SOURCE_LOCKED

This closes a narrow movement/collision runtime-evidence gap: wall/collision reports now have a tested manifest path that records exact map/x/y/direction, movement-pipeline state, and PPM screenshots for a deterministic blocked and accepted movement sequence. This is Firestaff runtime evidence, not an original DOS pixel-parity claim.

## ReDMCSB anchors

- `DUNGEON.C:1371-1391` - `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` applies forward and right-step deltas through the direction tables.
- `CLIKMENU.C:180-347` - `F0366_COMMAND_ProcessTypes3To6_MoveParty` maps movement arrows to forward/right deltas, checks wall/door/fakewall blockers, discards input and waits vblank for blocked movement, and only reaches `F0267` plus `G0310` cooldown after accepted movement.
- `MOVESENS.C:316-565` - `F0267_MOVE_GetMoveResult_CPSCE` owns accepted party/object movement consequences; the focused party coordinate mutation is at `MOVESENS.C:438-444`.
- `DUNVIEW.C:8318-8618` - `F0128_DUNGEONVIEW_Draw_CPSF` redraws the viewport from the party direction/map coordinate tuple and derives visible square coordinates through `F0150`.

## Provenance lock

- DM1 PC canonical `DUNGEON.DAT` SHA-256: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`.
- DM1 PC canonical `GRAPHICS.DAT` SHA-256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`.
- The verifier checks both canonical anchors under `~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/` and the runtime `FIRESTAFF_DATA` copies before accepting generated captures.

## Runtime evidence

The verifier runs `firestaff_m11_wall_collision_capture_probe` and validates the generated manifest rows:

| label | action | expected party state | expected movement state |
| --- | --- | --- | --- |
| `01_start_south_1_3` | start | map 0, x 1, y 3, direction south | no turn, no step, not blocked |
| `02_turn_right_west_1_3` | turn right | map 0, x 1, y 3, direction west | turn applied, no step, viewport dirty |
| `03_blocked_west_wall_1_3` | forward into west wall | map 0, x 1, y 3, direction west | blocked, no step, viewport not dirtied |
| `04_forward_south_1_4` | turn left then forward south | map 0, x 1, y 4, direction south | step applied, not blocked, viewport dirty |

Verification command:

```sh
ctest --test-dir build-dm1v1-movement-collision-source-lock -R 'dm1_v1_wall_collision_(runtime_capture|capture_manifest_source_lock)$' --output-on-failure
```
