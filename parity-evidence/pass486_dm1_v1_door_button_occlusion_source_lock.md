# Pass486 — DM1 V1 door-button occlusion source lock

Status: `PASS486_DM1_V1_DOOR_BUTTON_OCCLUSION_SOURCE_LOCKED`

## Audited ReDMCSB anchors
- `DUNVIEW.C:door_button_coordinate_table` — `[485, 485]`
- `DUNVIEW.C:F0110_door_button_blit` — `[4119, 4217]`
- `DUNVIEW.C:D3R_D3C_D2C_D1C_button_branches` — `[6500, 1968]`
- `DEFS.H:door_button_constants` — `[2794, 4224]`

## Source slice SHA-256
- `DUNVIEW.C:1210-1216` — `501a1a93cc8d0142c573974ff7a874ecf74eb6f24afeae76c4c3200eefb6f417`
- `DUNVIEW.C:4119-4216` — `07ba10d0b6ab169c547e2b0b3926ba2a27f5fc1f544ad332b625e13c3fc5231a`
- `DUNVIEW.C:6579-7908` — `b70d357ac3b2121ffad41d8a596648a6931a2007f3557ec00ab6de851433e937`

## Firestaff seams
{
  "centerButtonChecks": 8,
  "centerCoordinateChecks": 3,
  "d3rButtonChecks": 10,
  "viewportOrderChecks": 6
}

No original-runtime/pixel parity claim is made by this gate.
