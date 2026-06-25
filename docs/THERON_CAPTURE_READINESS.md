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
- `theron_v1_runtime_screenshot_promotion_gate`: a bounded provenance gate
  that audits every row of the readiness manifest against an explicit
  README-eligibility contract (real Firestaff runtime capture, no
  deterministic fallback assets, `TQR level load` boot marker, semantic
  Track 02 loader evidence in the probe, unique source BMP sha256 per row,
  valid 320x200 presented BMP) and locks the non-promotion state until at
  least one row is `README_ELIGIBLE` AND a human reviewer promotes it from
  tracked evidence.
- `theron_24h_readiness`: the daily readiness rollup ties the Track 02 bank,
  save/load, cross-route mechanics, runtime screenshot, dungeon progression,
  cross-slot, and M11 launch gates into one PASS/FAIL manifest so later
  screenshot promotion can start from a known-good baseline.
- `theron_v1_srm_classifier` (probe + unit test, 2026-06-25): bounded
  Save Disk manifest that detects the gzipped-deflate body dmweb's
  `community/documentation/miscellaneous.html` and greatstone page
  reference for Theron's Quest savegames, reports a clean
  `present_count=0, recognized_count=0` ABSENT manifest on the current
  host (no real `.srm` file is staged), and accepts a real `.srm`
  when one is placed under
  `$HOME/.firestaff/data/theron/save/slot0.srm` ... `slot4.srm` or
  the `FIRESTAFF_THERON_SRM_DIR` override. Default root uses 5 disk
  slots matching the original Save Disk cartridge model. The
  classifier is the bounded real-artifact counterpart to the
  synthetic `theron_v1_save_load.c` `slotN.tqsv` in-game save
  format; the two are kept separate because the underlying save
  models are different (Save Disk cartridge vs. in-game save
  format). See `include/theron_v1_srm_classifier.h` for the
  classifier contract and `docs/DMWEB_REFERENCE.md` §6 for the
  format anchor. Real `.srm` payload decoding (gzipped Theron save
  body) and cross-slot import to `Theron_DungeonProgression`/
  champion blocks remain out of scope and are tracked under
  `docs/FIRESTAFF_GAP_LIST.md` A3 `Savegame format (Theron .SRM)`.

These are readiness receipts, not public screenshot assets. Do not add Theron
images to the README until the `theron_v1_runtime_screenshot_promotion_gate`
classifies at least one readiness row as `README_ELIGIBLE` AND a human
reviewer promotes that specific row from tracked evidence into
`verification-screens/`. Do not use generated, illustrated, or mock Theron
images as screenshots. A green readiness row proves the launch/capture path
is alive; it does not prove semantic Track 02 dungeon-table parity, real
`.srm` interchange, broader playability, or that the captured frame is
README-eligible.

The promotion gate is the single source of truth for whether a Theron
screenshot is eligible to be promoted into public docs. Today it reports
`NO_README_PROMOTION_PERMITTED`: every readiness row still uses
deterministic fallback assets and shows no semantic Track 02 loader
evidence, so no Theron capture may yet be promoted into the README even
if the runtime-screenshot gate is green. This is the honest current state.

Run the focused readiness check with:

```bash
ctest --test-dir build -R '^theron_v1_runtime_screenshot_readiness$' --output-on-failure
ctest --test-dir build -R '^theron_v1_track02_' --output-on-failure
ctest --test-dir build -R '^theron_v1_srm_classifier' --output-on-failure
```

Run the promotion-provenance check with:

```bash
ctest --test-dir build -R '^theron_v1_runtime_screenshot_promotion_gate$' --output-on-failure
```

The generated reports live at:

- `parity-evidence/theron_v1_runtime_screenshot_readiness.md` — geometry
  and hashes only, screenshot bytes operator-local until promotion.
- `parity-evidence/theron_v1_runtime_screenshot_promotion_gate.md` — row
  classification against the eligibility contract and the current
  `NO_README_PROMOTION_PERMITTED` lock.
