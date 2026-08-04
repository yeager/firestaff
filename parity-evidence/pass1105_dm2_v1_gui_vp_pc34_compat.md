# Pass 1105 — DM2 V1 Viewport Rendering (c_gui_vp.cpp)

## What

Port all 49 viewport rendering functions from skproject c_gui_vp.cpp into
Firestaff using the callback-based architecture pattern. This is the largest
single module in the DM2 port (~7500 lines in skproject).

## Implemented functions

| Function | skproject source | Description |
|----------|-----------------|-------------|
| TRIM_BLIT_RECT | c_gui_vp.cpp:570 | Set global blit rectangle |
| guivp_098d_0cd7 | c_gui_vp.cpp:150 | GDAT index from position |
| SET_GRAPHICS_FLIP_FROM_POSITION | c_gui_vp.cpp:79 | Compute flip state |
| DRAW_MIRRORED_PIC | c_gui_vp.cpp:47 | Mirror-blit bitmap |
| guivp_32cb_0649 | c_gui_vp.cpp:793 | Query/transform palette |
| guivp_32cb_00f1 | c_gui_vp.cpp:723 | Point-in-rect hit test |
| guivp_32cb_0a4c | c_gui_vp.cpp:966 | Store clickable zone |
| guivp_32cb_0c7d | c_gui_vp.cpp:993 | Fill/rain background |
| DRAW_PIT_ROOF | c_gui_vp.cpp:158 | Ceiling above pit |
| DRAW_PIT_TILE | c_gui_vp.cpp:234 | Pit opening |
| DRAW_STAIRS_FRONT | c_gui_vp.cpp:468 | Stairs facing player |
| DRAW_STAIRS_SIDE | c_gui_vp.cpp:532 | Stairs from side |
| DRAW_WALL | c_gui_vp.cpp:575 | Wall face |
| DRAW_WALL_TILE | c_gui_vp.cpp:6703 | Wall + ornaments dispatch |
| DRAW_DOOR_TILE | c_gui_vp.cpp:5125 | Door dispatch |
| DRAW_DOOR | c_gui_vp.cpp:4806 | Door panel |
| DRAW_DOOR_FRAMES | c_gui_vp.cpp:2333 | Door frame ornaments |
| DRAW_DEFAULT_DOOR_BUTTON | c_gui_vp.cpp:1904 | Button on door |
| DRAW_TELEPORTER_TILE | c_gui_vp.cpp:824 | Teleporter effect |
| DRAW_EXTERNAL_TILE | c_gui_vp.cpp:4427 | External elements |
| DRAW_PLAYER_TILE | c_gui_vp.cpp:5288 | Player's own tile |
| DRAW_RAIN | c_gui_vp.cpp:1994 | Rain overlay |
| DRAW_ITEM | c_gui_vp.cpp:2078 | Item on tile |
| DRAW_FLYING_ITEM | c_gui_vp.cpp:3458 | Projectile in flight |
| DRAW_PUT_DOWN_ITEM | c_gui_vp.cpp:3893 | Item on ground |
| MAKE_PUT_DOWN_ITEM_CLICKABLE_ZONE | c_gui_vp.cpp:3816 | Item clickzone |
| DRAW_STATIC_OBJECT | c_gui_vp.cpp:4239 | Static objects on tile |
| SUMMARY_DRAW_CREATURE | c_gui_vp.cpp:4144 | Creatures on tile |
| guivp_32cb_3e08 | c_gui_vp.cpp:4070 | Put-down items around creature |
| guivp_32cb_3edd | c_gui_vp.cpp:5106 | Finalize deferred items |
| guivp_32cb_2cf3 | c_gui_vp.cpp:1057 | Creature summary image |
| guivp_32cb_2d8c | c_gui_vp.cpp:1091 | Creature items on tile |
| guivp_32cb_35c1 | c_gui_vp.cpp:1408 | Item position perspective |
| guivp_32cb_4069 | c_gui_vp.cpp:1495 | Creature movement interpolation |
| guivp_32cb_48d5 | c_gui_vp.cpp:1599 | Stretched door dimension |
| ENVIRONMENT_DRAW_DISTANT_ELEMENT | c_gui_vp.cpp:296 | Draw distant element |
| ENVIRONMENT_SET_DISTANT_ELEMENT | c_gui_vp.cpp:6746 | Set distant element |
| ENVIRONMENT_DISPLAY_ELEMENTS | c_gui_vp.cpp:6815 | Display environment |
| guivp_32cb_54ce | c_gui_vp.cpp:1621 | Distance/direction compute |
| guivp_32cb_15b8 | c_gui_vp.cpp:6043 | Wall ornament drawing |
| guivp_32cb_3f0d | c_gui_vp.cpp:5430 | Ornament actuator items |
| guivp_32cb_0f82 | c_gui_vp.cpp:5537 | Creature item masks |
| SUMMARIZE_STONE_ROOM | c_gui_vp.cpp:2497 | Tile summary (590 lines) |
| guivp_32cb_4185 | c_gui_vp.cpp:3089 | Tile info table entry |
| guivp_32cb_5a8f | c_gui_vp.cpp:1705 | Wall visibility mask |
| guivp_32cb_5c67 | c_gui_vp.cpp:1850 | Teleporter visibility clear |
| DRAW_DUNGEON_TILES | c_gui_vp.cpp:6932 | Main viewport draw loop |
| CHANCE_TABLE_OPERATION | c_gui_vp.cpp:7177 | Mouse-based arrows |
| DISPLAY_VIEWPORT | c_gui_vp.cpp:7325 | Top-level entry point |

## Design

Uses the callback-based architecture with `DM2_V1_GuiVpCallbacks` containing
~50 function pointers organized by role: backbuffer, blit rect, graphics flip,
palette, hit test, tile drawing, item/creature drawing, environment, wall
ornament, stone room summary, tile table, and main pipeline.

Receipt structs for the three public functions with observable output:
`DM2_V1_DisplayViewportReceipt`, `DM2_V1_GuiVp00f1Receipt`,
`DM2_V1_GuiVp15b8Receipt`.

Data tables: `dm2_guivp_table1d27a0[16]` (GDAT lookup) and
`dm2_guivp_table1d7029[20]` (tile iteration order) defined in source.

## Test

`test_dm2_v1_gui_vp_pc34_compat` — 28 tests covering null safety for all
public entry points and many internal functions, GDAT index computation,
direction math, data table values, and callback dispatch verification.
