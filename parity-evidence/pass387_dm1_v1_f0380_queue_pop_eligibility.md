# Pass387 — DM1 V1 F0380 queue-pop eligibility

Status: `BLOCKED_PASS387_F0380_QUEUE_POP_ELIGIBILITY_PREDICATE_PROVEN`

## Decision

Blocked/narrowed: ReDMCSB proves the exact branch that can enter `F0380_COMMAND_ProcessQueue_CPSC` but bypass semantic movement dispatch is the PC34 queue-pop eligibility predicate in `COMMAND.C`:

- `if (G2153_i_QueuedCommandsCount == 0)` runs before the command is loaded or popped.
- That branch reaches `T0380xxx`, unlocks the command queue, calls `F0360_COMMAND_ProcessPendingClick()`, then jumps to `T0380042`.
- Therefore it bypasses the later `G2153_i_QueuedCommandsCount--`, `F0365_COMMAND_ProcessTypes1To2_TurnParty(...)`, and `F0366_COMMAND_ProcessTypes3To6_MoveParty(...)` calls.

## Mandatory ReDMCSB source audit

Source root audited: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

Exact files/functions:

- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC` — locks the queue, computes the next queue index, checks `G2153_i_QueuedCommandsCount == 0`, then only after eligibility loads `G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command`, decrements `G2153_i_QueuedCommandsCount`, and dispatches turn/move commands to `F0365/F0366`.
- `CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty` — turn semantic handler; writes `G0321_B_StopWaitingForPlayerInput = C1_TRUE` at entry.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty` — move semantic handler; writes `G0321_B_StopWaitingForPlayerInput = C1_TRUE` at entry.
- `GAMELOOP.C:F0002_MAIN_GameLoop_CPSDF` — drains keyboard through `F0361_COMMAND_ProcessKeyPress(...)`, calls `F0380_COMMAND_ProcessQueue_CPSC()`, disables highlight if `G0321` remains false, and loops while `!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking`.
- `PC.H` — defines the PC alias for `G2153_i_QueuedCommandsCount`.

Secondary eligibility branch: movement commands `C003..C006` also bypass `F0366` if `G0310_i_DisabledMovementTicks` is set, or if projectile-disabled movement matches the attempted direction.

## Runtime tie-back to pass386

Pass386 evidence remains the runtime anchor:

- Keyboard reaches `F0361`, but no `G2153_i_QueuedCommandsCount` write was observed.
- Panel click reaches `F0380/G0321/F0128`, but not `F0365/F0366`.

This combination matches the source predicate above: execution can reach `F0380` and still not dispatch if the queue count is zero/not pop-eligible before the command load/decrement.

## Evidence

- Verifier: `scripts/verify_dm1_v1_f0380_queue_pop_eligibility.py`
- Manifest: `parity-evidence/verification/pass387_dm1_v1_f0380_queue_pop_eligibility/manifest.json`
- Upstream runtime artifact: `parity-evidence/verification/pass386_dm1_v1_keyboard_vs_click_command_dispatch/manifest.json`
