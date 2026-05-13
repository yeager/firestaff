# Pass505 - DM1 V1 blocked movement collision timing gap

Status: PASS505_DM1_V1_BLOCKED_MOVEMENT_COLLISION_TIMING_SOURCE_LOCKED

## ReDMCSB-first audit
- COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC - [2045, 2829]
- CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty - [180, 351]
- MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE - [316, 910]
- VBLANK.C:F0693_WaitVerticalBlank - [626, 646]

## Original data / Greatstone anchors
- DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- Greatstone overview sha256 7fdd2c8daef24250d58bc35632e245def338c0e63cf3832ce9af6534da54896c
- Greatstone PC34 diff manifest sha256 506c65d3a1aad453c3040c9c0031fb7419d6ec62d5b97f621d6494906afd9494

## Closed gap
blocked step/collision timing: a dequeued step can spend stamina and then be blocked, but blocked collision discards queued follow-up input, requests one vblank wait, keeps input wait armed, and returns before F0267 sensor/timestamp/cooldown/viewport side effects

## Not claimed
- new original DOS runtime breakpoint hit
- pixel parity
- creature reaction timeline materialization beyond source-locked request flag
