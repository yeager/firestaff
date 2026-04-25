# DM1 all-graphics phase 15 — source-bound side doors

Date: 2026-04-25 12:15 Europe/Stockholm
Scope: Firestaff V1 / DM1 viewport side door zones.

## Change

Added source-bound side-door rendering for closed/visible side doors:

- D3L2 / D3R2 clipped door panels
- D3L / D3R side frames + clipped door panels
- D2L / D2R top frames + clipped door panels
- D1L / D1R top frames + clipped door panels

Door panels use the same map `DoorSet0/1` + door thing type selection introduced in phase 14. Placements come from layout-696 via the `COORD.C F0635_` resolver.

## Resolved placements used

### Door panels

- D3L2: graphic D3, zone `C3700`, `srcX=35 y=0 -> x=0 y=28 w=9 h=38`
- D3R2: graphic D3, zone `C3710`, `srcX=0 y=0 -> x=210 y=28 w=14 h=38`
- D3L: graphic D3, zone `M624`, `srcX=1 y=0 -> x=30 y=29 w=43 h=38`
- D3R: graphic D3, zone `M626`, `srcX=0 y=0 -> x=151 y=29 w=43 h=38`
- D2L: graphic D2, zone `M627`, `srcX=4 y=0 -> x=0 y=24 w=60 h=59`
- D2R: graphic D2, zone `M629`, `srcX=0 y=0 -> x=164 y=23 w=60 h=59`
- D1L: graphic D1, zone `M630`, `srcX=64 y=0 -> x=0 y=18 w=32 h=86`
- D1R: graphic D1, zone `M632`, `srcX=0 y=0 -> x=192 y=18 w=32 h=86`

### Frames

- D3L frames: graphic `90`, zones `C718/C719`
- D3R frames: graphic `90`, zones `C720/C721`
- D2L top: graphic `92`, zone `C729`
- D2R top: graphic `92`, zone `C731`
- D1L top: graphic `91`, zone `C732`
- D1R top: graphic `91`, zone `C734`

## Artifacts

Generated screenshot set:

- `verification-m11/dm1-all-graphics/phase15-source-side-doors-20260425-1215/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase15-source-side-doors-20260425-1215/normal/*.png`

Quick crop:

- `verification-m11/dm1-all-graphics/phase15-source-side-doors-20260425-1215/normal/party_hud_top_190_crop.png`

Visual inspection: no obvious major regression versus phase 14. The viewport is more DM1-like structurally because side-door/source zones participate in the perspective composition. Remaining visible issues: high-frequency speckle/palette verification, bright debug-like seams, bottom floor/threshold messiness, exact DUNVIEW draw order, opening-state shifts, ornaments/buttons.

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

Port `F0111_DUNGEONVIEW_DrawDoor` opening-state semantics and start replacing the remaining prototype object/floor/pit/stairs paths with zone-bound equivalents.
