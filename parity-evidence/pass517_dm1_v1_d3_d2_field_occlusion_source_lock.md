# Pass517 DM1 V1 D3/D2 field occlusion source lock

Status: passed

## Primary ReDMCSB Evidence

- PASS D3R DUNVIEW.C:6514-6638
  - line 6514: case C19_ELEMENT_STAIRS_FRONT:
  - line 6544: goto T0117016;
  - line 6603: case C02_ELEMENT_PIT:
  - line 6615: case C05_ELEMENT_TELEPORTER:
  - line 6616: case C01_ELEMENT_CORRIDOR:
  - line 6618: L0202_i_Order = C0x4312_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT;
  - line 6620: F0108_DUNGEONVIEW_DrawFloorOrnament
  - line 6622: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
  - line 6628: if (L0203_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER)
  - line 6631: F0113_DUNGEONVIEW_DrawField
  - line 6634: C706_ZONE_WALL_D3R

- PASS D2L DUNVIEW.C:6914-7048
  - line 6914: case C19_ELEMENT_STAIRS_FRONT:
  - line 6944: goto T0119018;
  - line 7005: case C02_ELEMENT_PIT:
  - line 7015: case C05_ELEMENT_TELEPORTER:
  - line 7016: case C01_ELEMENT_CORRIDOR:
  - line 7018: L0207_i_Order = C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT;
  - line 7020: F0108_DUNGEONVIEW_DrawFloorOrnament
  - line 7023: F0112_DUNGEONVIEW_DrawCeilingPit
  - line 7031: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
  - line 7037: if (L0208_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER)
  - line 7040: F0113_DUNGEONVIEW_DrawField
  - line 7046: C710_ZONE_WALL_D2L

- PASS D2R DUNVIEW.C:7065-7240
  - line 7065: case C19_ELEMENT_STAIRS_FRONT:
  - line 7095: goto T0120027;
  - line 7198: case C02_ELEMENT_PIT:
  - line 7208: case C05_ELEMENT_TELEPORTER:
  - line 7209: case C01_ELEMENT_CORRIDOR:
  - line 7211: L0209_i_Order = C0x4312_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT;
  - line 7213: F0108_DUNGEONVIEW_DrawFloorOrnament
  - line 7215: F0112_DUNGEONVIEW_DrawCeilingPit
  - line 7224: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
  - line 7230: if (L0210_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER)
  - line 7233: F0113_DUNGEONVIEW_DrawField
  - line 7239: C711_ZONE_WALL_D2R

## Verification

- /home/trv2/work/firestaff-worktrees/viewport-occlusion-consolidation-20260521-0024/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS source_evidence.d0c_thieves_eye_frame_occlusion == 1
PASS source_evidence.side_occlusion == 1
PASS source_evidence.defs_zones == 1
PASS source_evidence.wall_source_clip_gate == 1
PASS source_evidence.wall_empty_blit_gate == 1
PASS source_evidence.occlusion == 1
PASS source_evidence.command_dispatch == 1
PASS source_evidence.next_redraw == 1
PASS source_evidence.present_wait == 1
PASS dm1_v1_viewport_3d_source_lock
~~~

## Non-Claims

- No input or movement queue edits.
- No original DOS pixel parity claim.
- DANNESBURK was not used.
