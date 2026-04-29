# Pass 159 — V1 viewport source-zone table lock

Static-no-party original-route blocker remains (`48ed3743ab6a`), so this pass advances viewport/world visuals without claiming pixel parity: it locks Firestaff world-content anchor tables to source GRAPHICS.DAT layout-696 reconstruction.

- evidence root: `parity-evidence/verification/pass159_viewport_source_zone_tables`
- source zone reconstruction: `zones_h_reconstruction.json` (`GRAPHICS.DAT` sha256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`)
- gate: `python3 tools/verify_v1_viewport_source_zone_tables.py`

## Locked visual families

- `C2500` object/item floor anchors: full helper `kC2500Raw` matches layout-696 `C2500..C2567`; renderer subset `kC2500` matches the first source rows.
- `C2900` projectile/effect anchors: full helper `kC2900Raw` matches layout-696 `C2900..C2947`; renderer subset `kC2900` matches the first source rows.
- `C3200` creature anchors: center and side helper points are constrained to the source `C3200` family records.

## Result

Source/evidence gate only. No original runtime pixel parity is claimed until a non-static party/control route is available.
