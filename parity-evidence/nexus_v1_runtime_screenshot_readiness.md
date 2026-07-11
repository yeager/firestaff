# Nexus V1 runtime screenshot readiness

Status: `PASS`

This gate runs a real Firestaff Nexus boot probe when Track 1 data is
present, then consumes the Nexus-owned Track 1 BMP readiness probe for
image geometry and hash receipts. It does not add screenshots to public docs.

## Case Results

| Case | Status | Boot marker | Runtime source | Data source | App BMP | Probe BMP |
|---|---:|---:|---|---|---:|---:|
| Nexus canonical extracted Track 1 root | PASS | yes | `nexus` | `` | `1` | `4` |
| Nexus Saturn JA Track 1 ISO/CUE root | PASS | yes | `nexus` | `` | `1` | `4` |

## Public Screenshot Boundary

- These receipts prove Firestaff can boot Nexus Track 1 data and that the Nexus-owned viewport/font handoff can emit BMP artifacts from the same data root.
- The stored evidence is metadata only: command status, runtime probe fields, BMP dimensions, non-black pixel counts, and SHA256 values.
- No generated, mock, synthetic, or operator-supplied image bytes are promoted by this gate.
- README-eligible Nexus screenshots still need reviewed real runtime frames and stronger semantic DGN/DMDF/BPK/text rendering parity.

Manifest: `parity-evidence/verification/nexus_v1_runtime_screenshot_readiness/manifest.json`
