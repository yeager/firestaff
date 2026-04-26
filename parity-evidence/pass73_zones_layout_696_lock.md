# Pass 73 — ZONES/PANEL layout-696 lock

Date: 2026-04-26

## Goal

Retire the vague “ZONES.H missing” blocker by committing a reproducible extraction of the original DM1 PC 3.4 layout table from `GRAPHICS.DAT` entry `696` (`C696_GRAPHIC_LAYOUT`).

This is a layout-data lock, not a full UI overlay claim.

## Tool

- `tools/extract_zones_layout_696.py`
- Output: `zones_h_reconstruction.json`

The extractor reads the unique `0xFC0D` layout blob at absolute `GRAPHICS.DAT` offset `0x47a6f`, validates the original PC 3.4 `GRAPHICS.DAT` SHA-256, parses 23 layout ranges, and writes all 1133 records.

## Gate command

```sh
python3 tools/extract_zones_layout_696.py \
  verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DATA/GRAPHICS.DAT \
  zones_h_reconstruction.json
```

Result:

```text
OK -- 1133 records across 23 ranges (9160 bytes) -> zones_h_reconstruction.json
Sanity zones verified: 1, 3, 7, 100, 101, 150, 151, 152, 153, 154, 187
```

## Locked records

| zone | meaning | record |
| ---: | --- | --- |
| 1 | screen dimensions | `type=9 parent=0 d1=320 d2=200` |
| 3 | viewport bitmap dimensions | `type=9 parent=0 d1=224 d2=136` |
| 7 | viewport placement | `type=1 parent=3 d1=0 d2=33` |
| 100 | panel bitmap dimensions | `type=9 parent=4 d1=144 d2=73` |
| 101 | centered panel anchor | `type=0 parent=100 d1=152 d2=89` |
| 150 | champion status-box bitmap | `type=9 parent=0 d1=67 d2=29` |
| 151..154 | champion status-box slot offsets | `x=0/69/138/207 y=0` |
| 187 | champion 0 bar-graph zone | `type=1 parent=183 d1=43 d2=0` |
| 507..519 | inventory equipment/quiver slot boxes | source slot-box zone family present |

## Impact

The next UI overlay passes should cite `zones_h_reconstruction.json` and the existing `M11_GameView_GetV1*Zone()` helpers rather than treating side-panel, inventory, spell, and status-box placement as unknown.
