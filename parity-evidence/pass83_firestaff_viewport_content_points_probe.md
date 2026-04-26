# Pass 83 — Firestaff V1 viewport content source points

Date: 2026-04-26

## Goal

Add source-backed visual evidence for the dynamic viewport content anchor points used by original-faithful V1 mode, without claiming fake pixel parity.

This pass overlays the layout-696 `C2500`, `C2900`, and `C3200` point families on the current deterministic Firestaff 320×200 screenshot series:

- `C2500` — floor object anchor points used with `MASK0x8000_SHIFT_OBJECTS_AND_CREATURES`.
- `C2900` — projectile anchor points, parallel to `C2500` but higher in the viewport.
- `C3200` — creature center/side slot anchor points.

## Source evidence

- `zones_h_reconstruction.json` is reconstructed from original DM1 PC `GRAPHICS.DAT` entry 696 / ReDMCSB layout data.
- The probe reads the zone records directly from that reconstruction; it does not scrape renderer comments or hard-code the tables.
- Viewport-local coordinates are projected into the DM1 V1 screen viewport at `(0,33,224,136)`.

## Tool and outputs

- Tool: `tools/pass83_firestaff_viewport_content_points_probe.py`
- Stats: `parity-evidence/overlays/pass83/pass83_firestaff_viewport_content_points_stats.json`
- SVG overlays: `parity-evidence/overlays/pass83/*_viewport_content_points.svg`

Command:

```sh
python3 tools/pass83_firestaff_viewport_content_points_probe.py
```

Result:

```text
points=65, scenes=6, problems=[]
```

## Point counts

| family | total source points | inside viewport |
| --- | ---: | ---: |
| C2500 object | 10 | 8 |
| C2900 projectile | 10 | 8 |
| C3200 creature | 45 | 34 |

## Interpretation

- The SVGs make source content anchors inspectable on every deterministic V1 screenshot without storing local absolute paths.
- Off-viewport points are preserved and edge-marked instead of discarded; those are expected for extreme side-cell source coordinates.
- This is geometry/evidence only. It does not assert that the current sprites, clipping, draw order, or palette are pixel-identical to original DM1 runtime captures.
- No gameplay/M10 behavior was changed.
