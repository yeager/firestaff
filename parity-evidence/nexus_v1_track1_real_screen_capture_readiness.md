# Nexus V1 Track 1 real-screen-capture-required readiness

Status: `PASS`

This gate proves the DM.BIN/FONT256.S2D/MNS runtime handoff
reaches a deterministic local 24-bit BMP while DGN remains
blocked before original Saturn capture/admission. It runs through
`nexus_viewport_render` + `nexus_viewport_to_rgba` +
`M11_Screenshot_CaptureRGBA` for every verified Nexus data root
that is present on the host. It is a *readiness* receipt for
the E1 Track 1 phase-launch row; it does not promote screenshots
and it does not rewrite public docs.

## Case Results

| Case | Status | Probe | Valid no-draw BMPs | Non-black (first BMP) | SHA-deterministic |
|---|---:|---:|---:|---:|---:|
| Nexus extracted Track 1 root | PASS | yes | `0` | `0` | yes |
| Nexus saturn-ja Track 1 .bin | PASS | yes | `0` | `0` | yes |

## Public Screenshot Boundary

- These receipts prove Firestaff can emit deterministic Nexus viewport BMP artifacts while DM.BIN/FONT256.S2D/MNS reach the V1 viewport path and DGN remains blocked.
- BMPs are written to a per-case temporary directory and discarded when the gate exits; no image bytes are promoted.
- No generated, mock, or synthetic image is promoted by this gate.
- The probe's data-free path (no `--data-dir` supplied) still proves the parser/capture-writer contract, but it must stay no-draw.
- README-eligible Nexus screenshots still need a separate original-Saturn capture gate that audits DGN 3D geometry, palette/VDP1 state, text/glyph runtime binding, and MNS dispatch before any image bytes leave the operator-local output directory.

Manifest: `parity-evidence/verification/nexus_v1_track1_real_screen_capture_readiness/manifest.json`
