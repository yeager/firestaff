# Pass242 — DM1 V1 runtime debugger probe attempt

Status: `BLOCKED_COMMAND_MOVEMENT_VIEWPORT_CHAIN_NOT_PROVEN`

This pass tested the pass241 numeric CS:IP candidates with N2-local `dosbox-debug`/Xvfb. It deliberately does **not** promote any `verified_runtime_hit`.

## What was observed

- `DEBUG FIRES.EXE` with temporary unpacked `FIRES.EXENEW` reproduced the pass235 entry image (`01ED:0000`, SHA256 `fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94`).
- Breakpoint `25B9:110E` was debugger-observed (`25B9:110E 55 push bp ...`). This is only the pass241 viewport-adjacent candidate and happened without a verified command/movement route.
- Breakpoints `1D69:06E9`, `195E:010D`, `195E:01AA`, and `1313:0516` were not observed in the bounded probe.
- `DEBUG DM.EXE` with the staged PC34 tree accepted all pass241 breakpoints, but no pass241 candidate hit occurred during the bounded xdotool route.

## Blocker

The current tooling still lacks a verified map from the real `DM.EXE` loaded FIRES runtime image to the pass241 static candidates. Pass241 candidates are based on the direct `FIRES.EXENEW` debug load segment. The real loader route either uses a different segment/state path or needs a more precise runtime load-segment capture. Until that exists, the chain `command_accepted -> movement_applied -> viewport_present` is not proven.

Evidence manifest: `parity-evidence/verification/pass242_dm1_v1_runtime_debugger_probe_attempt/manifest.json`.
