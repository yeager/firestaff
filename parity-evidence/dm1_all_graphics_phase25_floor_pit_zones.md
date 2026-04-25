# DM1 all-graphics phase 25 — source-bound floor pit zones

Date: 2026-04-25 13:45 Europe/Stockholm
Scope: Firestaff V1 / DM1 floor pit rendering.

## Change

Added first source-bound floor pit rendering pass for normal V1.

When sampled viewport cells are `DUNGEON_ELEMENT_PIT`, the renderer now blits original DM1 floor-pit graphics into layout-696 resolved zones instead of relying only on the old procedural/cue pit rendering.

Covered pit positions:

| View | Graphic | Zone | Placement |
|---|---:|---:|---|
| D3L2 | 49 | C850 | `x=0 y=66 w=22 h=10` |
| D3R2 | 49 | C851 | `x=202 y=66 w=22 h=10` |
| D3L | 50 | C852 | `x=4 y=65 w=78 h=8` |
| D3C | 51 | C853 | `x=79 y=65 w=64 h=8` |
| D3R | 50 | C854 | `x=142 y=65 w=78 h=8` |
| D2L | 52 | C855 | `srcX=1 x=0 y=76 w=71 h=13` |
| D2C | 53 | C856 | `x=66 y=77 w=92 h=12` |
| D2R | 52 | C857 | `x=153 y=76 w=71 h=13` |
| D1L | 54 | C858 | `srcX=3 x=0 y=94 w=54 h=24` |
| D1C | 55 | C859 | `x=43 y=94 w=139 h=24` |
| D1R | 54 | C860 | `x=169 y=94 w=55 h=24` |
| D0L | 56 | C861 | `srcX=4 x=0 y=126 w=20 h=10` |
| D0C | 57 | C862 | `x=27 y=127 w=170 h=9` |
| D0R | 56 | C863 | `x=200 y=126 w=24 h=10` |

These placements come from `tools/resolve_dm1_zone.py`, matching ReDMCSB `COORD.C F0635_` behavior for layout 696.

## Source anchors

- `DUNVIEW.C F0120..F0127_DUNGEONVIEW_DrawSquare*`
- `C049_GRAPHIC_FLOOR_PIT_D3L2`
- `M754..M761_GRAPHIC_FLOOR_PIT_*`
- `C850..C863_ZONE_FLOORPIT_*`
- `COORD.C F0635_`

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Artifacts

Corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase25-floor-pit-zones-20260425-1345/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase25-floor-pit-zones-20260425-1345/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase25-floor-pit-zones-20260425-1345/normal/party_hud_top_190_crop_vga.png`

Visual note: the large black central rectangle visible in this frame was already present in phase24 before pit blits, so it is not evidence of a new pit regression. A deterministic pit-focused scene is still needed to prove the pit placements visually.

## Remaining work

- Invisible pit graphics (`58..63`) require proper pit/teleporter visibility detection from square aspect/sensors.
- Add deterministic pit-specific capture scene.
- Next viewport domains: stairs and teleporter/field zones.
