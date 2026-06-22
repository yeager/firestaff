# Theron V1 runtime screenshot readiness

Status: `PASS`

This gate runs real Firestaff Theron launches when hash-verified Track 02
data is present. It records runtime probe fields plus BMP geometry and
hash receipts only; it does not add screenshots to public docs.

## Case Results

| Case | Status | Boot marker | Runtime source | Fallback assets | Source BMP | Presented BMP |
|---|---:|---:|---|---:|---:|---:|
| Theron canonical PC Engine root | PASS | yes | `theron` | yes | `1` | `1` |
| Theron JP extras Track 02 | PASS | yes | `theron` | yes | `1` | `1` |
| Theron US extras Track 02 | PASS | yes | `theron` | yes | `1` | `1` |

## Public Screenshot Boundary

- These receipts prove Firestaff can emit Theron runtime screenshot artifacts when the Track 02 launch reaches M11.
- Rows that use deterministic fallback assets are runtime/capture-path proof, not final art or source-bank proof.
- No generated, mock, or synthetic image is promoted by this gate.
- README-eligible Theron screenshots still need reviewed real runtime frames and stronger semantic Track 02 loader parity.

Manifest: `parity-evidence/verification/theron_v1_runtime_screenshot_readiness/manifest.json`
