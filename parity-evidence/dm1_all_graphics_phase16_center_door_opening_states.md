# DM1 all-graphics phase 16 — center door opening-state clipping

Date: 2026-04-25 12:24 Europe/Stockholm
Scope: Firestaff V1 / DM1 center door opening states.

## Change

Added first source-bound opening-state support for center doors (`D1C`, `D2C`, `D3C`). Door states `1..3` now use the corresponding `zone + state` clipping resolved from layout 696 / `COORD.C F0635_`, instead of always drawing the full closed door panel.

Conservative behavior:

- state `0`: treated as open, no door panel drawn (existing behavior)
- states `1..3`: use source-resolved partial panel clipping
- states `4/5`: keep base closed/destroyed panel behavior for now until destroyed masks/ornaments are ported

## Resolved center-door partial zones

### D1C (`M631_ZONE_DOOR_D1C = 3790`, panel graphic D1)

- state 1 -> zone `3791`: `srcY=65 x=64 y=15 w=96 h=23`
- state 2 -> zone `3792`: `srcY=43 x=64 y=15 w=96 h=45`
- state 3 -> zone `3793`: `srcY=21 x=64 y=15 w=96 h=67`

### D2C (`M628_ZONE_DOOR_D2C = 3760`, panel graphic D2)

- state 1 -> zone `3761`: `srcY=44 x=80 y=24 w=64 h=17`
- state 2 -> zone `3762`: `srcY=29 x=80 y=24 w=64 h=32`
- state 3 -> zone `3763`: `srcY=14 x=80 y=24 w=64 h=47`

### D3C (`M625_ZONE_DOOR_D3C = 3730`, panel graphic D3)

- state 1 -> zone `3731`: `srcY=27 x=90 y=29 w=44 h=11`
- state 2 -> zone `3732`: `srcY=17 x=90 y=29 w=44 h=21`
- state 3 -> zone `3733`: `srcY=7 x=90 y=29 w=44 h=31`

## Source anchors

- `DUNVIEW.C F0111_DUNGEONVIEW_DrawDoor`
- `P2084_i_ZoneIndex += P0125_ui_DoorState` for non-open/non-closed states
- `M631_ZONE_DOOR_D1C`, `M628_ZONE_DOOR_D2C`, `M625_ZONE_DOOR_D3C`
- `COORD.C F0635_`

## Artifacts

Generated screenshot set:

- `verification-m11/dm1-all-graphics/phase16-center-door-opening-states-20260425-1224/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase16-center-door-opening-states-20260425-1224/normal/*.png`

Quick crop:

- `verification-m11/dm1-all-graphics/phase16-center-door-opening-states-20260425-1224/normal/party_hud_top_190_crop.png`

Visual inspection: no obvious static regression in the sampled frame. Motion/opening sequence still needs explicit capture to verify flicker and clipping across states.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Next step

Extend opening-state clipping to side doors and then add destroyed-mask/ornament/button handling from `F0111_DUNGEONVIEW_DrawDoor` / `F0109` / `F0110`.
