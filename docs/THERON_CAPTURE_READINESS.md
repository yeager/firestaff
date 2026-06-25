# Theron Capture Readiness

Theron's Quest is not yet represented by README screenshots. The current public
status should stay clear: Firestaff can hash-verify JP/US Track 02 data, lock
raw bank-anchor offsets, validate the 9-word descriptor-table shape, and reach
the M11 Theron runtime, but per-entry semantic Track 02 dungeon-table binding,
broader loader parity, runtime playability proof, and reviewed screenshot
promotion are still active work.

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
- `theron_v1_track02_bank` and `theron_v1_track02_descriptor_table`: when
  real Track 02 data is present, both probes hash-gate the bank-anchor
  offsets and the 9-word little-endian stride table shape on the US ISO
  (`0x1584`) plus all three US raw BIN anchors and all three JP raw BIN
  anchors. The descriptor-table decoder is shape-driven only: it validates
  the 9-word stride sequence (entries 0x0020..0x2020, stride 0x0400) without
  claiming per-entry semantic type, dungeon-level binding, or loader
  handoff.
- `theron_24h_readiness`: the daily readiness rollup ties the Track 02 bank,
  save/load, cross-route mechanics, runtime screenshot, dungeon progression,
  cross-slot, and M11 launch gates into one PASS/FAIL manifest so later
  screenshot promotion can start from a known-good baseline.

These are readiness receipts, not public screenshot assets. Do not add Theron
images to the README until a reviewed real Firestaff runtime frame is promoted
from tracked evidence. Do not use generated, illustrated, or mock Theron images
as screenshots. A green readiness row proves the launch/capture path is alive;
it does not prove per-entry semantic Track 02 dungeon-table binding, real
`.srm` interchange, or broader playability.

Run the focused readiness check with:

```bash
ctest --test-dir build -R '^theron_v1_runtime_screenshot_readiness$' --output-on-failure
ctest --test-dir build -R '^theron_v1_track02_' --output-on-failure
```

The generated report lives at
`parity-evidence/theron_v1_runtime_screenshot_readiness.md`; it records
geometry and hashes only, leaving screenshot bytes operator-local until a
separate promotion decision.
