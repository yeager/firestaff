# Pass385 — DM1 V1 corrected-loader-delta semantic route probe

Status: `BLOCKED_PASS385_F0365_F0366_COMMAND_DISPATCH_NOT_PROVEN`

## Decision

Blocked: corrected loader delta proves F0380/G0321/F0128 runtime path, but semantic command dispatch still missing f0365OrF0366Hit

## Runtime predicates

- `sawRunning`: `True`
- `routeInputAfterArming`: `True`
- `breakpointsRetainedAtArm`: `True`
- `f0380Hit`: `True`
- `f0365OrF0366Hit`: `False`
- `g0321StopWaitWriteObserved`: `True`
- `nextF0128AfterStopWaitObserved`: `True`

## Corrected addresses armed

- `F0380_COMMAND_ProcessQueue_CPSC`: `22F7:0699`
- `F0365_COMMAND_ProcessTypes1To2_TurnParty`: `1EA7:010D`
- `F0366_COMMAND_ProcessTypes3To6_MoveParty`: `1EA7:01AA`
- `F0128_DUNGEONVIEW_Draw_CPSF`: `23B0:40FE`
- `F0097_VIDRV_09_BlitViewPort_indirect_call`: `280C:1EFF`
- `G0321_B_StopWaitingForPlayerInput`: `2C23:1A7C`

## Evidence

- Manifest: `parity-evidence/verification/pass385_dm1_v1_corrected_loader_delta_semantic_route/manifest.json`
- Transcript: `parity-evidence/verification/pass385_dm1_v1_corrected_loader_delta_semantic_route/pass385_runtime.clean.txt`
- Route keylog: `parity-evidence/verification/pass385_dm1_v1_corrected_loader_delta_semantic_route/pass385_route_keylog.json`
- Command log: `parity-evidence/verification/pass385_dm1_v1_corrected_loader_delta_semantic_route/pass385_command_log.json`
