# Nexus V1 Track 1 real-screen-capture readiness

Status: `PASS`

This gate proves the DM.BIN/FONT256.S2D/MNS runtime handoff
reaches a real 24-bit BMP on disk through
`nexus_viewport_render` + `nexus_viewport_to_rgba` +
the probe-owned 24-bit BMP receipt writer for every verified Nexus data root
that is present on the host. It is a *readiness* receipt for
the E1 Track 1 phase-launch row; it does not promote screenshots
and it does not rewrite public docs.

## Case Results

| Case | Status | Probe | Valid BMPs | Non-black (first BMP) | SHA-deterministic |
|---|---:|---:|---:|---:|---:|
| Nexus extracted Track 1 root | PASS | yes | `4` | `256` | yes |
| Nexus saturn-ja Track 1 .bin | PASS | yes | `4` | `256` | yes |

## Public Screenshot Boundary

- These receipts prove Firestaff can emit Nexus viewport BMP artifacts when DM.BIN/FONT256.S2D/MNS reach the V1 viewport render path.
- BMPs are written to a per-case temporary directory and discarded when the gate exits; no image bytes are promoted.
- No generated, mock, or synthetic image is promoted by this gate.
- The probe's data-free path (no `--data-dir` supplied) still proves the synthetic viewport → RGBA → BMP contract, but is *not* a Track 1 capture.
- README-eligible Nexus screenshots still need a separate eligibility gate that audits real Track 1 loader parity (DGN 3D geometry decode, text/glyph runtime binding, MNS model dispatch) before any image bytes leave the operator-local output directory.

Manifest: `parity-evidence/verification/nexus_v1_track1_real_screen_capture_readiness/manifest.json`
