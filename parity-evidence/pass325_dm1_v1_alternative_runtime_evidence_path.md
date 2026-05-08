# Pass325 — DM1 V1 alternative runtime-evidence path

Status: `PASS_ALTERNATIVE_RUNTIME_EVIDENCE_PATH_FOUND`

## Decision

A non-tmux command/control path exists on N2: direct Python PTY (`pty.openpty`) to `/usr/bin/dosbox-debug` under Xvfb with `TERM=vt100`. The bounded control test wrote `HELP`, `BP 1234:5678`, and `BPLIST` to the PTY master and observed `DEBUG: Set breakpoint` plus `00. BP 1234:5678` in stdout.

This removes the tmux pane-capture dependency for the debugger command channel. It does **not** by itself promote a fresh stock DM1 runtime hit.

## ReDMCSB audit

- `DUNVIEW.C`: `F0128_DUNGEONVIEW_Draw_CPSF`, `F0674_F0128_sub`, and the F0128 -> F0097 viewport draw call are present.
- `DRAWVIEW.C`: `F0097_DUNGEONVIEW_DrawViewport`, viewport zone handling, and `VIDRV_09_BlitViewPort` are present.
- `COMMAND.C`: `F0380_COMMAND_ProcessQueue_CPSC` and turn/move dispatch to `F0365`/`F0366` are present.

## Prior evidence gap

- pass315 verified a narrow F0128 hit but left F0380/F0097 unpromoted.
- pass318 identified BPLIST/setup echo confusion and left F0097 runtime blocked.
- pass320 strict filtering avoided echo confusion but did not recapture F0128.
- pass321 names the missing primitive: reliable debugger code-stop/control sequencing.
- pass322/pass323 source/data-lock movement and wall offsets but preserve the runtime blocker.

## Bounded tests

1. `direct_pty_dosbox_debug_stdin_stdout` — PASS, <=60s. Direct PTY accepted debugger commands without tmux.
2. `strace_read_write_capture` — BLOCKED as a promotion path, <=60s. It can archive byte I/O, but has no emulator CPU/code-stop semantics by itself.

## Next required primitive

Integrate the direct-PTY command channel with the controlled FIRES window route, then promote only after strict unprompted code-stop evidence at `23AD:40FE` followed by `2809:1E31`/`2809:1EFF`, or a controlled `22F4:0699` dequeue stop. Keep rejecting BPLIST/setup echoes.

Manifest: `parity-evidence/verification/pass325_dm1_v1_alternative_runtime_evidence_path/manifest.json`
