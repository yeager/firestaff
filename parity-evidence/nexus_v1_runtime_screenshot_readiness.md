# Nexus V1 runtime screenshot readiness

Status: `PASS`

This gate runs real Firestaff Nexus launches when Track 1 data is
present. It records runtime probe fields plus BMP geometry and hash
receipts only; it does not add screenshots to public docs.

## Case Results

| Case | Status | Boot marker | Runtime source | Data source | Source BMP | Presented BMP |
|---|---:|---:|---|---|---:|---:|
| Nexus canonical extracted Track 1 root | PASS | yes | `nexus` | `extracted` | `1` | `1` |
| Nexus Saturn JA Track 1 ISO/CUE root | PASS | yes | `nexus` | `iso` | `1` | `1` |

## Public Screenshot Boundary

- These receipts prove Firestaff can emit Nexus runtime screenshot artifacts after a real Track 1 launch reaches M11.
- The stored evidence is metadata only: command status, runtime probe fields, BMP dimensions, non-black pixel counts, and SHA256 values.
- No generated, mock, synthetic, or operator-supplied image bytes are promoted by this gate.
- README-eligible Nexus screenshots still need reviewed real runtime frames and stronger semantic DGN/DMDF/BPK/text rendering parity.

Manifest: `parity-evidence/verification/nexus_v1_runtime_screenshot_readiness/manifest.json`
