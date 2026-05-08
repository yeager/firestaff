# Pass293 — DM1 V1 direct F0380 hook address/window probe

Status: `BLOCKED_SOURCE_MAP_DEBUGGER_WINDOW_FOR_DIRECT_F0380`

## Verdict

- Direct F0380 entry BP `22F4:0699`: `not seen` — no transcript stop line at `22F4:0699`.
- Source/map chain still resolves public symbol `1BC1:0699` + load segment `0733` => `22F4:0699`; this pass does **not** disprove the map, it blocks direct-hook promotion.
- Pass293 candidate-window runtime captured queue/index memory activity (`G0432/G0433`), but the broad watch set was too noisy and did not improve pass278.
- Prior pass278 remains the stronger runtime chain: controlled keys, `G0432` write, party tuple mutation, and `F0128` draw hit — still no direct F0380 BP.

## Source audit anchors

- `GAMELOOP.C:160-216`: active loop reads keyboard through `F0361_COMMAND_ProcessKeyPress` then calls `F0380_COMMAND_ProcessQueue_CPSC()`.
- `COMMAND.C:2045-2156`: F0380 queue empty/gate check, `G0432[G0433]` dequeue, index advance/unlock, then turn/move dispatch.
- `MOVESENS.C:316-556`: accepted movement reaches `F0267` and writes `G0306/G0307`.
- `DUNVIEW.C:8318-8611`: `F0128` consumes direction/mapX/mapY and reaches viewport draw.

## Exact blocker

No direct F0380 runtime proof. The next smallest run should avoid the broad pass293 G0433-noise window: set only `BP 22F4:0699` plus tuple/queue watches; if it remains silent, capture live bytes/disassembly for `22F4:0680..06F0` and move breakpoints to post-prologue/dequeue instruction offsets inside `COMMAND.C:F0380`.

## Artifacts

- Manifest: `parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/manifest.json`
- Candidate transcript: `parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/dosbox_debug_candidate_window.clean.txt`
- Live BP/code-window probe transcript: `parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/live_code_window_dump.clean.txt`
- Route log: `parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/route_candidate_window_keylog.json`
