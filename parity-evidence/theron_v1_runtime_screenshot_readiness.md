# Theron V1 runtime screenshot readiness

Status: `SKIP`

This gate runs real Firestaff Theron launches when hash-verified Track 02
data is present. It records runtime probe fields plus BMP geometry and
hash receipts only; it does not add screenshots to public docs.

## Case Results

| Case | Status | Boot marker | Runtime source | Fallback assets | Source BMP | Presented BMP |
|---|---:|---:|---|---:|---:|---:|
| Theron canonical PC Engine root | SKIP | no | `` | no | `0` | `0` |
| Theron JP extras Track 02 | SKIP | no | `` | no | `0` | `0` |
| Theron US extras Track 02 | SKIP | no | `` | no | `0` | `0` |

## Public Screenshot Boundary

- These receipts prove Firestaff can emit Theron runtime screenshot artifacts when the Track 02 launch reaches M11.
- Any deterministic fallback asset rejects this gate; only source-backed startup graphics may pass.
- No generated, mock, or synthetic image is promoted by this gate.
- Without an explicit authenticated VDC/VCE pair, this gate is SKIP; a black fail-closed viewport is never treated as a passing screenshot.
- Supply an externally retained, hash-authenticated VDC/VCE pair with --vram-snapshot and --vce-snapshot; this admits only the captured source screen and does not open square, object, HUD or gameplay semantics.
- README-eligible Theron screenshots still need reviewed real runtime frames and stronger semantic Track 02 loader parity.

Manifest: `parity-evidence/verification/theron_v1_runtime_screenshot_readiness/manifest.json`
