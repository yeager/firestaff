# Pass 127 — viewport/world F0115 C2500/C2900 row mapping

## Source lock

ReDMCSB `DUNVIEW.C` drives object/projectile placement in `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` by mapping the current view square through `G2028_ac_ViewSquareIndexTo`, then indexing layout-696 zones as:

- objects/items: `C2500_ZONE_ + (G2028[viewSquare] * 4) + viewCell`
- projectiles: `C2900_ZONE_ + (G2028[viewSquare] * 4) + viewCell`

The relevant MEDIA720 view-square order from `DEFS.H` is:

- `D1C/L/R = 3/4/5`
- `D2C/L/R = 6/7/8`
- `D3C/L/R = 11/12/13`

So Firestaff's relative viewport cells map to C2500/C2900 rows:

| Firestaff relative cell | ReDMCSB square | `G2028` row |
| --- | --- | --- |
| `relForward=1, relSide=0/-1/+1` | `D1C/D1L/D1R` | `8/9/10` |
| `relForward=2, relSide=0/-1/+1` | `D2C/D2L/D2R` | `5/6/7` |
| `relForward=3, relSide=0/-1/+1` | `D3C/D3L/D3R` | `0/1/2` |

## Firestaff change

Normal V1 side-lane contents now carry the source row into the item and projectile sprite placement helpers.  Side-lane objects use the full `kC2500Raw` layout-696 rows, projectiles use the full `kC2900Raw` rows, and the old five-row helper remains available for existing center/debug callers.

This keeps side contents source-anchored without reintroducing the old invented side-pane fill/border path.

## Gates

- `cmake --build build -j2`
- `./build/firestaff_m11_game_view_probe` → `591/591 invariants passed`
- `python3 tools/verify_v1_viewport_source_zone_tables.py`
- CTest #1..#9 passed individually; #8 was run via `/tmp/fs_test8.log` because it takes ~45s to rebuild/run the launcher smoke path.
