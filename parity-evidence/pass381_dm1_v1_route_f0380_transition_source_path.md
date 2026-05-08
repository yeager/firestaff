# Pass381 — DM1 V1 route input to F0380/F0365/F0366 transition

Status: `BLOCKED_PASS381_STATIC_F0380_STOPWAIT_TRANSITION_PROVEN_RUNTIME_BREAKPOINTS_NEXT`

## Decision

Static ReDMCSB source path proves the intended post-route transition: GAMELOOP resets G0321_B_StopWaitingForPlayerInput, calls F0380_COMMAND_ProcessQueue_CPSC inside the wait loop, F0380 dispatches turn/move commands to F0365/F0366, and both handlers set G0321_B_StopWaitingForPlayerInput true before GAMELOOP can exit toward the next outer-loop F0128 draw. pass379/pass380 already prove route delivery and F0128/F0097 map binding, so the remaining runtime blocker is to arm the newly derived F0380/F0365/F0366 runtime breakpoints rather than retargeting F0128/F0097.

## Evidence

- Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- GAMELOOP order verified: F0128 draw precedes wait-loop reset; wait loop resets `G0321`, calls `F0380`, then loops until stop-wait/game-tick predicates allow exit.
- COMMAND dispatch verified: queued turn commands call `F0365`; queued movement commands call `F0366`.
- CLIKMENU stop-wait writes verified: both `F0365` and `F0366` set `G0321_B_StopWaitingForPlayerInput = C1_TRUE`.
- Newly derived runtime breakpoint candidates: F0380_COMMAND_ProcessQueue_CPSC `22F4:0699`, F0365_COMMAND_ProcessTypes1To2_TurnParty `1EA4:010D`, F0366_COMMAND_ProcessTypes3To6_MoveParty `1EA4:01AA`.

Manifest: `parity-evidence/verification/pass381_dm1_v1_route_f0380_transition_source_path/manifest.json`
