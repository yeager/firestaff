# Pass384 — DM1 V1 F0380/F0365/F0366 runtime breakpoint chain

Status: `BLOCKED_PASS384_RUNTIME_CHAIN_NOT_PROVEN`

## Decision

Blocked: strict post-Running runtime chain incomplete; missing f0380Hit, f0365OrF0366Hit, g0321StopWaitWriteObserved, nextF0128AfterStopWaitObserved

## Runtime predicates

- `sawRunning`: `True`
- `routeInputAfterArming`: `True`
- `breakpointsRetainedAtArm`: `True`
- `f0380Hit`: `False`
- `f0365OrF0366Hit`: `False`
- `g0321StopWaitWriteObserved`: `False`
- `nextF0128AfterStopWaitObserved`: `False`

## Addresses armed

- `F0380_COMMAND_ProcessQueue_CPSC`: `22F4:0699`
- `F0365_COMMAND_ProcessTypes1To2_TurnParty`: `1EA4:010D`
- `F0366_COMMAND_ProcessTypes3To6_MoveParty`: `1EA4:01AA`
- `F0128_DUNGEONVIEW_Draw_CPSF`: `23AD:40FE`
- `F0097_VIDRV_09_BlitViewPort_indirect_call`: `2809:1EFF`
- `G0321_B_StopWaitingForPlayerInput`: `2C20:1A7C`

## Evidence

- Manifest: `parity-evidence/verification/pass384_dm1_v1_f0380_runtime_breakpoint_chain/manifest.json`
- Transcript: `parity-evidence/verification/pass384_dm1_v1_f0380_runtime_breakpoint_chain/pass384_runtime.clean.txt`
- Route keylog: `parity-evidence/verification/pass384_dm1_v1_f0380_runtime_breakpoint_chain/pass384_route_keylog.json`
- Command log: `parity-evidence/verification/pass384_dm1_v1_f0380_runtime_breakpoint_chain/pass384_command_log.json`
