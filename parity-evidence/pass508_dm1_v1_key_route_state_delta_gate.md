# Pass508 - DM1 V1 key-route state-delta gate

PASS508_DM1_V1_KEY_ROUTE_STATE_DELTA_GATE_LOCKED

Movement/key-route processing is source/runtime proven through F0380 and F0365/F0366; original capture is still blocked at post-command state-delta-to-redraw evidence because the current post-entry frames repeat sha256 48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397.

## ReDMCSB source audit
- COMMAND.C:636-685 - PC34/I34E keyboard rows map route keys to C001/C002 turns and C003..C006 movement commands.
- COMMAND.C:1709-1813 - F0361 resolves keyboard input and writes the command queue before F0380 consumes it.
- COMMAND.C:2045-2156 - F0380 rejects gated movement before dequeue, otherwise pop-loads exactly one command and dispatches turns/steps.
- CLIKMENU.C:142-174 - F0365 accepted turns set stop-wait and mutate party direction.
- CLIKMENU.C:180-347 - F0366 computes movement, blocks before commit, or commits via F0267 and applies cooldown after success.
- MOVESENS.C:738-780 - F0267 records destination map tuple and movement timing/scent state after accepted party movement.
- GAMELOOP.C:90-219 - The next promotable capture must be after stop/tick lets the loop redraw from the updated party tuple.

## Promotion rule
- F0361 queue write and G2153 increment
- F0380 pop/load/decrement for same command
- F0365 turn or F0366 movement handling after dequeue
- later redraw/present over mutated direction/X/Y tuple, or explicit source-proven blocked/no-op
- reject repeated 48ed static gameplay frames and route-label-only filenames

## Secondary references
- canonical DM1 DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- canonical DM1 GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- canonical DM1 TITLE sha256 adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745
- Greatstone atlas: /Users/bosse/.openclaw/data/firestaff-greatstone-atlas/index/SUMMARY.md
- CSBWin movement cross-check: /Users/bosse/.openclaw/data/firestaff-csbwin-source/CSBWin/Code11f52.cpp

## Gate
- python3 tools/verify_pass508_dm1_v1_key_route_state_delta_gate.py
