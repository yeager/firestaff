# Pass 83 — viewport content anchor coverage

Evidence-only expansion of the layout-696 viewport content overlay. It uses the full source ranges for floor object, projectile, and creature placement anchors and does not claim pixel parity.

- Source: `zones_h_reconstruction.json`
- Viewport rect: `[0, 33, 224, 136]`
- TSV: `parity-evidence/overlays/pass83/pass83_firestaff_viewport_content_points.tsv`

| Family | Anchors | Inside viewport |
| --- | ---: | ---: |
| `C2500_object` | 56 | 48 |
| `C2900_projectile` | 36 | 28 |
| `C3200_creature` | 179 | 110 |

Notes:
- `(0,0)` sentinel records are excluded from visible anchor counts.
- Negative/off-edge source coordinates are retained in JSON/TSV so clipping and side-cell placement remain auditable.
- Overlay SVGs are current Firestaff V1 evidence only, not original-runtime parity claims.
