# Pass162 C080 movement_pid_control live break-start probe

Status: `BLOCKED_PASS162_LIVE_MOVEMENT_PID_CONTROL_NO_C080_STOPS`

Debugger accepted the pass162 BP/BPM packet, but the bounded movement pid control route produced no C080-chain stops.

## Runtime

- observed symbols: `[]`
- ordered prefix: `[]`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- mouse post mode: `pid`
- mouse warp: `true`
- route window found: `True`
- route control ok: `True`
- engine ready seen: `True`
- runtime ready seen: `True`
- memory stop count: `34`

## Artifacts

- Manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_pid_control_break_start_probe/manifest.json`
- Transcript: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_pid_control_break_start_probe/live_movement_pid_control_break_start.clean.txt`
- Route log: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_pid_control_break_start_probe/route_log.json`
- Command log: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_pid_control_break_start_probe/command_log.json`
