# Theron Capture Readiness

Theron's Quest is not yet represented by README screenshots. The current public
status should stay clear: Firestaff can hash-verify JP/US Track 02 data, lock
raw bank-anchor offsets, and reach the M11 Theron runtime, but semantic
Track 02 dungeon-table decoding, broader loader parity, runtime playability
proof, and reviewed screenshot promotion are still active work.

The strongest current proof is:

- `tier1_strict_boot_probe`: JP canonical, JP extras, and US extras reach the
  `TQR level load` boot milestone when those data paths are present.
- `theron_v1_m11_direct_launch`: M11 consumes the hash-verified Track 02 path
  without re-walking the data root and builds the Theron world and viewport.
- `theron_v1_viewport_renderer` and `theron_v1_rendering`: the data-free
  Theron viewport, palette, UI chrome, and M11 blit contracts are covered.
- `theron_v1_cross_route_mechanics`: synthetic runtime mechanics now cover a
  bounded movement/interact route across doors, pools, alarms, triggers,
  teleporters, pits, post-move drain, and click-route TAKE.
- `theron_v1_runtime_screenshot_readiness`: when real Track 02 data is
  present, the verifier launches Firestaff under dummy video and records M11
  runtime-probe JSON plus source/presented BMP hash receipts. Rows that report
  deterministic fallback assets are capture-path proof, not final art or
  source-bank proof. Missing data is a successful SKIP, not a failure.

These are readiness receipts, not public screenshot assets. Do not add Theron
images to the README until a reviewed real Firestaff runtime frame is promoted
from tracked evidence. Do not use generated, illustrated, or mock Theron images
as screenshots.

Run the focused readiness check with:

```bash
ctest --test-dir build -R '^theron_v1_runtime_screenshot_readiness$' --output-on-failure
```

The generated report lives at
`parity-evidence/theron_v1_runtime_screenshot_readiness.md`; it records
geometry and hashes only, leaving screenshot bytes operator-local until a
separate promotion decision.
