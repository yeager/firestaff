# Pass578 - DM1 V1 stairs backstep cooldown gate

- Status: PASS578_DM1_V1_STAIRS_BACKSTEP_COOLDOWN_GATE_SOURCE_LOCKED
- Manifest: parity-evidence/verification/pass578_dm1_v1_stairs_backstep_cooldown_gate/manifest.json

## ReDMCSB Source Audit

- COMMAND.C:2045-2829, focused 2095-2155: movement commands are gated by G0310/G0311 before dequeue and F0366 dispatch.
- CLIKMENU.C:180-349, focused 256-345: backward-on-stairs takes F0364 only after dispatch reaches F0366 and returns before normal relative-step/cooldown flow.
- DUNGEON.C:1423-1479, focused 1440-1475: stairs classification comes from current-map square data; out-of-bounds reads are wall fallback.

## Firestaff Gate

- dm1_v1_input_command_queue_pc34_compat.c:300-342, focused 317-332: queue returns before popping a gated move.
- dm1_v1_movement_command_core_pc34_compat.c:182-376, focused 206-364: command core returns before stamina/stairs/timing when queue did not dequeue.
- test_dm1_v1_movement_pipeline_pc34_compat asserts a cooldown-gated backward command on stairs remains queued, leaves party/stamina unchanged, and does not apply a stairs transition.

## Not Claimed

- does not replace pass571 turn no-step timing
- does not cover pass571 front-command queue/pending replay
- no original DOS runtime capture
