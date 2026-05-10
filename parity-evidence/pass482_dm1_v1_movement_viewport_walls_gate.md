# Pass482 — DM1 V1 movement → viewport walls gate

Status: `PASS482_DM1_V1_MOVEMENT_VIEWPORT_WALLS_SOURCE_LOCKED`

## Audited ReDMCSB slices
- `IO2.C:arrow_normalization` — `[47, 61]`
- `COMMAND.C:key_table_queue_dispatch` — `[677, 2156]`
- `CLIKMENU.C:F0365_F0366_movement` — `[142, 346]`
- `DUNGEON.C:F0150_relative_projection` — `[1371, 1391]`
- `GAMELOOP.C:redraw_and_command_cadence` — `[55, 185]`
- `DUNVIEW.C:F0128_wall_replay` — `[8318, 8618]`
- `DUNVIEW.C:wall_door_handoff` — `[6400, 6816]`
- `DRAWVIEW.C:F0097_present` — `[709, 858]`

## Source slice SHA-256
- `COMMAND.C:2045-2156` — `99233462e2aed38076d7a33937af86db31a47ea28196de127028b88446fe0550`
- `CLIKMENU.C:142-346` — `c80104584b4481d9b5875cf5a5003d43718b92e5c76fadb0624f7c6ff0ed0938`
- `DUNVIEW.C:8318-8618` — `80f0c3db43784d8d933f122c95b7b7febce5ac28ad8a3677bc26aa5174345461`
- `DRAWVIEW.C:709-858` — `cab0eceeb9ea0074fcfe4dfa200f3893fe49ce979ee135615f2265f87c3d3abf`

## Firestaff seams
{
  "core": 6,
  "m11": 3,
  "pipeline": 5,
  "queue": 4,
  "viewport": 5
}

No original-runtime/pixel parity claim is made by this gate.
