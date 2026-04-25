# DM1 all-graphics phase 11 — source-bound side wall blits

Date: 2026-04-25 11:35 Europe/Stockholm
Scope: Firestaff V1 / DM1 viewport side wall-zone pass.

## Change

Extended the source-bound wall path from front blockers to side wall zones. Normal V1 now samples side squares and, when they are blocked, blits the matching DM1 wall-set 0 graphic into the original layout-696 viewport zone.

This keeps the old Firestaff procedural corridor/trapezoids debug-only and replaces more of the normal viewport with source-backed wall panels.

## Added wall zones

Resolved via `tools/resolve_dm1_zone.py` / ReDMCSB `COORD.C F0635_` semantics:

| View square | Graphic | Zone | Placement |
|---|---:|---:|---|
| D3L2 | 104 | C702 | `x=0 y=25 w=44 h=49` |
| D3R2 | 103 | C703 | `x=180 y=25 w=44 h=49` |
| D3L | 106 | C705 | `x=7 y=25 w=83 h=49` |
| D3R | 105 | C706 | `x=134 y=25 w=83 h=49` |
| D2L2 | 99 | C707 | `x=0 y=24 w=8 h=52` |
| D2R2 | 98 | C708 | `x=216 y=24 w=8 h=52` |
| D2L | 101 | C710 | `x=0 y=19 w=78 h=74` |
| D2R | 100 | C711 | `x=146 y=19 w=78 h=74` |
| D1L | 96 | C713 | `x=0 y=9 w=60 h=111` |
| D1R | 95 | C714 | `x=164 y=9 w=60 h=111` |
| D0L | 94 | C716 | `x=0 y=0 w=33 h=136` |
| D0R | 93 | C717 | `x=191 y=0 w=33 h=136` |

Sampling model used for this pass:

- `D0L/R`: `relForward=0`, `relSide=-1/+1`
- `D1L/R`: `relForward=1`, `relSide=-1/+1`
- `D2L/R`: `relForward=2`, `relSide=-1/+1`
- `D2L2/R2`: `relForward=2`, `relSide=-2/+2`
- `D3L/R`: `relForward=3`, `relSide=-1/+1`
- `D3L2/R2`: `relForward=3`, `relSide=-2/+2`

## Artifacts

Generated screenshot set:

- `verification-m11/dm1-all-graphics/phase11-source-side-walls-20260425-1135/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase11-source-side-walls-20260425-1135/normal/*.png`

Quick crop:

- `verification-m11/dm1-all-graphics/phase11-source-side-walls-20260425-1135/normal/party_hud_top_190_crop.png`

Visual inspection: the viewport is more DM1-like than phase 10 because side wall depth slices now frame the scene. It is still not 1:1: exact `DUNVIEW.C` ordering/masking, door/pit/stair/teleporter/field handling, wall ornaments, object/creature/projectile order, palette verification, and UI clipping remain unfinished.

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

Move from simple "blocked square -> wall blit" to the actual `DUNVIEW.C` square draw order:

1. Port the call ordering around `F0120..F0127_DUNGEONVIEW_DrawSquare*`.
2. Add per-square element handling for wall/door/corridor/pit/stairs/teleporter rather than treating all non-open as wall.
3. Then bind door frames/doors and pits/stairs to the same zone resolver.
