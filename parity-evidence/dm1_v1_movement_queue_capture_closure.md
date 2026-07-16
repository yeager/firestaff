# DM1 V1 movement queue capture closure

Status: `FAIL_DM1_V1_MOVEMENT_QUEUE_CAPTURE_CLOSURE`

Scope: command input -> queue -> movement dispatch capture/evidence closure. This gate does not claim original pixel parity.

## ReDMCSB source audit

- PASS `COMMAND.C:106-121` - PC34 movement mouse zones feed the command queue labels used by capture routes.
- PASS `COMMAND.C:1452-1661` - F0359 stores accepted mouse commands or pending clicks into G0432.
- PASS `COMMAND.C:1709-1813` - F0361 scans keyboard tables and writes matched commands to G0432/G2153.
- PASS `COMMAND.C:2045-2156` - F0380 gates movement, replays pending clicks, pops a command, and dispatches F0365/F0366.
- PASS `CLIKMENU.C:142-347` - F0365/F0366 own the turn and step state changes before redraw promotion is legal.
- PASS `DUNVIEW.C:8318-8611` - F0128 composes the viewport from the post-command party tuple.
- PASS `DRAWVIEW.C:709-858` - F0097 is the PC34 viewport-present boundary required for promoted captures.

## Consumed evidence gates

- FAIL `parity-evidence/verification/pass564_dm1_v1_original_movement_viewport_transcript_gate/manifest.json` status `None`
- FAIL `parity-evidence/verification/pass608_dm1_v1_same_viewport_capture_blocker/manifest.json` status `None`
- FAIL `parity-evidence/pass609_dm1_v1_same_viewport_capture_contract.md` status `None`
- FAIL `parity-evidence/verification/pass545_dm1_v1_movement_queue_sensor_consequences/manifest.json` status `None`
- FAIL `parity-evidence/verification/pass559_dm1_v1_gated_movement_pending_click_queue_replay/manifest.json` status `None`

## Closure decision

The Firestaff source route and queue-dispatch behavior are already covered. The current open TODO is capture-only: a promotable original PC/I34E transcript must bind command input, G0432/G2153 queue mutation, F0380 pop, F0365/F0366 dispatch, party tuple state, F0128 redraw, and F0097 present to paired original/Firestaff viewport crops.

No new implementation behavior is changed by this pass.
