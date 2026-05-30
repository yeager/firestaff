# Pass515 DM1 V1 D0 side wall occlusion source lock

Status: passed

## Claim

ReDMCSB draws D0L and D0R before D0C. If either side lane is a wall, it draws its side wall zone and returns before the open-lane ceiling/object/teleporter-field path. This is narrower than the D0C foreground work.

## Primary ReDMCSB Evidence

- PASS redmcsb_f0128_d0_side_lanes_precede_d0c (DUNVIEW.C:8534-8542)
  - F0128 renders near side lanes D0L then D0R before D0C.
  - line 8536: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 0, -1, &L0224_i_MapX, &L0225_i_MapY);
  - line 8537: F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8540: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 0, 1, &L0224_i_MapX, &L0225_i_MapY);
  - line 8541: F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8542: F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);

- PASS redmcsb_f0125_d0l_wall_return_blocks_open_lane_path (DUNVIEW.C:7999-8059)
  - D0L wall draws C716 and returns before shared ceiling/field tail.
  - line 8000: case C16_ELEMENT_DOOR_SIDE:
  - line 8001: case C05_ELEMENT_TELEPORTER:
  - line 8005: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0220_ai_SquareAspect[M550_FIRST_THING], P0174_i_Direction, P0175_i_MapX, P0176_i_MapY, M610_VIEW_SQUARE_D0L, C0x0002_CELL_ORDER_BACKRIGHT);
  - line 8007: case C00_ELEMENT_WALL:
  - line 8017: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C00_WALL_D0R], C716_ZONE_WALL_D0L);
  - line 8033: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);
  - line 8038: return;
  - line 8041: F0112_DUNGEONVIEW_DrawCeilingPit(C067_GRAPHIC_CEILING_PIT_D0L
  - line 8059: F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M610_VIEW_SQUARE_D0L]], C716_ZONE_WALL_D0L);

- PASS redmcsb_f0126_d0r_wall_return_blocks_open_lane_path (DUNVIEW.C:8103-8159)
  - D0R wall draws C717 and returns before open object/field path.
  - line 8104: case C16_ELEMENT_DOOR_SIDE:
  - line 8105: case C05_ELEMENT_TELEPORTER:
  - line 8115: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0221_ai_SquareAspect[M550_FIRST_THING], P0177_i_Direction, P0178_i_MapX, P0179_i_MapY, M611_VIEW_SQUARE_D0R, C0x0001_CELL_ORDER_BACKLEFT);
  - line 8117: case C00_ELEMENT_WALL:
  - line 8127: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C01_WALL_D0L], C717_ZONE_WALL_D0R);
  - line 8139: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R);
  - line 8144: return;
  - line 8159: F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M611_VIEW_SQUARE_D0R]], C717_ZONE_WALL_D0R);

- PASS redmcsb_d0_wall_zone_ids_are_side_specific (DEFS.H:4050-4060)
  - PC34/I34E D0 side-wall zones are distinct from D0C.
  - line 4056: #define C716_ZONE_WALL_D0L                                      716
  - line 4057: #define C717_ZONE_WALL_D0R                                      717

## Firestaff Evidence

- PASS firestaff_existing_d0_side_wall_specs_cite_return_lines (dm1_v1_viewport_3d_pc34_compat.c:424-430)
  - Existing runtime metadata records direct D0 side-wall return evidence.

- PASS firestaff_existing_d0_side_occlusion_orders_are_single_back_cells (dm1_v1_viewport_3d_pc34_compat.c:251-256)
  - Open D0 side lanes use one back cell each; wall is a separate return path.

- PASS firestaff_viewport_test_covers_d0_side_wall_specs (test_dm1_v1_viewport_3d_pc34_compat.c:294-305)
  - Focused runtime test asserts D0 side return lines and zone/wall pairing.

## Verification

- command: /Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_viewport_3d_pc34_compat
  - returncode: 0
  - output tail:
~~~
PASS source_evidence.wall_empty_blit_gate == 1
PASS source_evidence.occlusion == 1
PASS source_evidence.command_dispatch == 1
PASS source_evidence.next_redraw == 1
PASS source_evidence.present_wait == 1
PASS source_evidence.same_viewport_mouse == 1
PASS source_evidence.same_viewport_turn == 1
PASS source_evidence.same_viewport_move == 1
PASS source_evidence.same_viewport_draw == 1
PASS source_evidence.same_viewport_present == 1
PASS source_evidence.same_viewport_assets == 1
PASS dm1_v1_viewport_3d_source_lock
~~~

- command: /opt/homebrew/opt/python@3.14/bin/python3.14 /Users/bosse/.openclaw/workspace-main/tools/verify_pass515_dm1_v1_d0_side_wall_occlusion_source_lock.py --check-only
  - returncode: 0
  - output tail:
~~~
PASS check-only
~~~

## Local References
- dm1_graphics_dat: exists=True path=/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT, sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- dm1_dungeon_dat: exists=True path=/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT, sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- greatstone_root: exists=True path=/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-greatstone-atlas

## Non-Claims

- No runtime metadata was changed.
- No D0C foreground behavior is promoted by this pass.
- DANNESBURK was not used.
