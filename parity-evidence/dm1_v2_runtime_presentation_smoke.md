# DM1 V2 runtime presentation smoke

Status: `PASS`

This gate runs the real Firestaff CLI with a temporary launcher config
for launchable DM1 enhanced presentation modes. It records M11 runtime
probe JSON as script evidence, without promoting any original-DOS
pixel-parity or finished V2.2 art claim.

## Mode results

| Mode | Status | Runtime source | Presentation mode | Source BMP | Presented BMP |
|---|---:|---|---:|---:|---:|
| V2.0 filtered | PASS | `dm1` | `1` | `1` | `1` |
| V2.1 enhanced 2D | PASS | `dm1` | `2` | `1` | `1` |

Presented-frame hashes differ across configured modes, so this gate
covers the post-palette/post-filter buffer and not only the source
indexed framebuffer.

## Non-claims

- This is not an original PC 3.4 screenshot pairing.
- This is not finished V2.2 real-art material/pixel verification.
- This does not unblock the DOSBox debugger/capture rows in DM1 B1.

Manifest: `parity-evidence/verification/dm1_v2_runtime_presentation_smoke/manifest.json`
