# Pass566 - DM1 V1 turn/cooldown gate boundary

Status: PASS566_DM1_V1_TURN_COOLDOWN_GATE_BOUNDARY_LOCKED

## ReDMCSB-first audit
- GAMELOOP.C:cooldown_decrement_before_F0380 - [150, 215]
- COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC - [2045, 2829]
- COMMAND.C:F0380_step_only_gate - [2095, 2096, 2151, 2155]
- CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty - [142, 174]

## Firestaff audit
- DM1_V1_InputCommandQueue_ProcessOnePc34Compat gates only move commands, never C001/C002 turns
- DM1_V1_MovementCommandCore_ProcessOnePc34Compat returns from the turn branch before step stamina/collision/cooldown

## Closed gap
turn commands C001/C002 bypass G0310/G0311 movement-disabled gating and do not run step stamina/collision/cooldown, while C003..C006 step commands remain queued until the cooldown gate clears

## Not claimed
- new original DOS runtime breakpoint transcript
- new pixel capture parity
- full creature AI reaction timing beyond command-gate boundary
