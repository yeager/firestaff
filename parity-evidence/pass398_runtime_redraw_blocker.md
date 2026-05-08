# Pass398 — runtime redraw blocker

Status: `PASS398_RUNTIME_REDRAW_CHAIN_PROVEN`

## ReDMCSB-first source anchors
- `COMMAND.C:F0361_COMMAND_ProcessKeyPress` — `[1709, 1956]`
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC` — `[2045, 2829]`
- `CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty` — `[142, 174]`
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty` — `[180, 352]`
- `GAMELOOP.C:F0002_MAIN_GameLoop_CPSDF` — `F0128 before input wait; F0380 inside wait loop; waits on G0321/G0301`
- `DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF` — `[8318, 8620]`

## Runtime predicate
- Script: `enter,down,down,down,down,down,down,enter,right,up`
- Probe JSON: `parity-evidence/verification/pass398_runtime_redraw_blocker/runtime_redraw_probe.json`
- Pipeline: `{"anyMovementOccurred": 0, "anyTurnOccurred": 1, "command": 2, "dequeued": 1, "movementBlocked": 0, "stepApplied": 0, "turnApplied": 1, "viewportDirty": 1}`
- Redraw: `{"inputRedrawAfterViewportDirtyCount": 1, "inputRedrawDrawCount": 1, "lastInputRedrawAfterViewportDirty": 1}`

## Verdict
- Proven chain: queued command dispatch/movement state reaches `viewportDirty`, returns `M11_GAME_INPUT_REDRAW`, and the same loop immediately calls `M11_GameView_Draw` after the dirty viewport result.
- Scope guard: no pixel-parity or DOSBox debugger-hit claim.
