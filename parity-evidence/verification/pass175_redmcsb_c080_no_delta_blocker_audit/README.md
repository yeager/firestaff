# Pass175 — ReDMCSB C080 no-delta blocker audit

## Purpose

Pass174 proved that twelve source-derived click delivery variants at/near the portrait center produced **zero visible delta** after the entrance gate. This pass records the source-first blocker analysis before any more coordinate guessing.

## Pass174 runtime fact

- `helper_scaled_window_relative_source_screen_portrait_center`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `helper_scaled_window_relative_source_center_if_viewport_y31`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `helper_scaled_window_relative_source_center_plus_one`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `absolute_screen_click_source_screen_portrait_center`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `absolute_screen_click_source_center_if_viewport_y31`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `absolute_screen_click_source_center_plus_one`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_down_up_source_screen_portrait_center`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_down_up_source_center_if_viewport_y31`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_down_up_source_center_plus_one`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_click_repeat3_source_screen_portrait_center`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_click_repeat3_source_center_if_viewport_y31`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_click_repeat3_source_center_plus_one`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}

## Source anchors checked

- `COMMAND.C` `C007_ZONE_VIEWPORT`: 2 hit(s)
- `COMMAND.C` `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`: 3 hit(s)
- `COMMAND.C` `BUG0_73`: 3 hit(s)
- `CLIKVIEW.C` `F0377_COMMAND_ProcessType80_ClickInDungeonView`: 1 hit(s)
- `CLIKVIEW.C` `C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT`: 4 hit(s)
- `CLIKVIEW.C` `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor`: 3 hit(s)
- `MOVESENS.C` `C127_SENSOR_WALL_CHAMPION_PORTRAIT`: 2 hit(s)
- `MOVESENS.C` `F0280_CHAMPION_AddCandidateChampionToParty`: 1 hit(s)
- `DUNVIEW.C` `G0109_auc_Graphic558_Box_ChampionPortraitOnWall`: 4 hit(s)
- `DUNVIEW.C` `M635_ZONE_PORTRAIT_ON_WALL`: 1 hit(s)
- `COORD.C` `G2067_i_ViewportScreenX`: 1 hit(s)
- `COORD.C` `G2068_i_ViewportScreenY`: 2 hit(s)

## Interpretation

- The click coordinates remain source-backed: viewport portrait box `{96,127,35,63}` and PC viewport origin `(0,33)` imply screen center `(111,82)`.
- Because pass174 saw no pixel delta for helper, absolute, down/up, and repeat-click variants, the next blocker is not another blind x/y permutation.
- The source path to verify next is command delivery/state precondition: whether the runtime has actually left entrance/menu command mode before C007/C080 is eligible, and whether BUG0_73-style queue ordering can drop the mouse command.
- Next runtime pass should instrument/state-gate around the exact transition into dungeon command processing, then issue one C007 click and immediately test for a candidate-state/panel transition before C160/C161. If no instrumentation exists, add a Firestaff-side gate that rejects unchanged pass174 frames and records the active frame class before clicking.

## Anchor excerpts

### COMMAND.C — C007_ZONE_VIEWPORT
```c
401:         { C005_COMMAND_MOVE_BACKWARD,           CM1_SCREEN_RELATIVE, C072_ZONE_MOVE_BACKWARD, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
402:         { C004_COMMAND_MOVE_RIGHT,              CM1_SCREEN_RELATIVE, C071_ZONE_MOVE_RIGHT,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
403:         { C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   CM1_SCREEN_RELATIVE, C007_ZONE_VIEWPORT,      0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
404:         { C083_COMMAND_TOGGLE_INVENTORY_LEADER, CM1_SCREEN_RELATIVE, C002_ZONE_SCREEN,        0, 0, MASK0x0001_MOUSE_RIGHT_BUTTON },
405:         { 0, 0, 0, 0, 0, 0 } };
```
```c
452:         { 0, 0, 0, 0, 0, 0 } };
453: MOUSE_INPUT G0450_as_Graphic561_PrimaryMouseInput_PartyResting[3] = {
454:         { C146_COMMAND_WAKE_UP, CM1_SCREEN_RELATIVE, C007_ZONE_VIEWPORT, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
455:         { C146_COMMAND_WAKE_UP, CM1_SCREEN_RELATIVE, C002_ZONE_SCREEN,   0, 0, MASK0x0001_MOUSE_RIGHT_BUTTON },
456:         { 0, 0, 0, 0, 0, 0 } };
```

### COMMAND.C — C080_COMMAND_CLICK_IN_DUNGEON_VIEW
```c
112:         { C005_COMMAND_MOVE_BACKWARD,         263, 289, 147, 167, MASK0x0002_MOUSE_LEFT_BUTTON },
113:         { C004_COMMAND_MOVE_RIGHT,            291, 318, 147, 167, MASK0x0002_MOUSE_LEFT_BUTTON },
114:         { C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   0, 223,  33, 168, MASK0x0002_MOUSE_LEFT_BUTTON },
115: #ifdef MEDIA351_G14ED_G20E_G21E
116:         { C083_COMMAND_TOGGLE_INVENTORY_LEADER, 0, 319,   0, 199, MASK0x0001_MOUSE_RIGHT_BUTTON },
```
```c
401:         { C005_COMMAND_MOVE_BACKWARD,           CM1_SCREEN_RELATIVE, C072_ZONE_MOVE_BACKWARD, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
402:         { C004_COMMAND_MOVE_RIGHT,              CM1_SCREEN_RELATIVE, C071_ZONE_MOVE_RIGHT,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
403:         { C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   CM1_SCREEN_RELATIVE, C007_ZONE_VIEWPORT,      0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
404:         { C083_COMMAND_TOGGLE_INVENTORY_LEADER, CM1_SCREEN_RELATIVE, C002_ZONE_SCREEN,        0, 0, MASK0x0001_MOUSE_RIGHT_BUTTON },
405:         { 0, 0, 0, 0, 0, 0 } };
```
```c
2320:                 goto T0380042;
2321:         }
2322:         if (L1160_i_Command == C080_COMMAND_CLICK_IN_DUNGEON_VIEW) {
2323:                 F0377_COMMAND_ProcessType80_ClickInDungeonView(L1161_i_CommandX, L1162_i_CommandY);
2324:                 goto T0380042;
```

### COMMAND.C — BUG0_73
```c
1476:         asm {
1477:                         tas     G0435_B_CommandQueueLocked(A4)  /* Test and set bit 7 in G0435_B_CommandQueueLocked
1478:                                                                 BUG0_73 A mouse click may be ignored. The tas instruction works on the most significant byte of G0435_B_CommandQueueLocked but when G0435_B_CommandQueueLocked is set to C1_TRUE elsewhere in the code, this affects the least significant byte only. Consequently, the code below may run while another function is modifying the command queue. The only possible consequence is when a key press is being processed in F0361_COMMAND_ProcessKeyPress */
1479:                         beq.s   T0359002                        /* Branch if the command queue is not already being accessed (if G0435_B_CommandQueueLocked = C0_FALSE, before bit 7 is set above) */
1480:         }
```
```c
1483:         asm {
1484:                         tas     G0435_B_CommandQueueLocked(A5)  /* Test and set bit 7 in G0435_B_CommandQueueLocked
1485:                                                                 BUG0_73 A mouse click may be ignored. The tas instruction works on the most significant byte of G0435_B_CommandQueueLocked but when G0435_B_CommandQueueLocked is set to C1_TRUE elsewhere in the code, this affects the least significant byte only. Consequently, the code below may run while another function is modifying the command queue. The only possible consequence is when a key press is being processed in F0361_COMMAND_ProcessKeyPress */
1486:                         beq.s   @T0359002                        /* Branch if the command queue is not already being accessed (if G0435_B_CommandQueueLocked = C0_FALSE, before bit 7 is set above) */
1487:         }
```
```c
1751:                         goto T0361014;
1752: #endif
1753:                 /* BUG0_73 A mouse click may be ignored. At this point, the command index where the keyboard command will be stored in the command queue is determined by L1110_i_CommandQueueIndex. If a mouse click interrupt occurs before the command is actually written into the command queue below then the mouse command will be stored at the same location in the queue and then be overwritten by the keyboard command */
1754:                 while (L1111_i_Command = L1112_ps_KeyboardInput->Command) {
1755: #ifdef MEDIA008_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_G21E
```

### CLIKVIEW.C — F0377_COMMAND_ProcessType80_ClickInDungeonView
```c
309: #endif
310: 
311: void F0377_COMMAND_ProcessType80_ClickInDungeonView(
312: #ifdef MEDIA009_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_G21E_A20ED_A20E_A20F_A20G_A21E_A22E_A22G
313: P0752_i_X, P0753_i_Y)
```

### CLIKVIEW.C — C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT
```c
365:                 if (G0415_ui_LeaderEmptyHanded) {
366: #ifdef MEDIA001_S10EA_S10EB /* CHANGE2_01_OPTIMIZATION Inline code replaced by function calls */
367:                         if ((((DOOR*)G0284_apuc_ThingData[C00_THING_TYPE_DOOR])[M013_INDEX(F0161_DUNGEON_GetSquareFirstThing(L1155_i_MapX, L1156_i_MapY))].Button) && (F0376_COMMAND_IsPointInBox(G0291_aauc_DungeonViewClickableBoxes[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT], P0752_i_X, P0753_i_Y - 33))) {
368: #endif
369: #ifdef MEDIA227_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_G21E_A20ED_A20E_A20F_A20G_A21E_A22E_A22G_F20E_F20J_X30J_P20JA_P20JB_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J /* CHANGE2_01_OPTIMIZATION Inline code replaced by function calls */
```
```c
371: #endif
372: #ifdef MEDIA224_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_G21E_A20ED_A20E_A20F_A20G_A21E_A22E_A22G /* CHANGE2_01_OPTIMIZATION Inline code replaced by function calls */
373:                         if ((((DOOR*)L1151_ps_Junk)->Button) && (F0376_COMMAND_IsPointInBox(G0291_aauc_DungeonViewClickableBoxes[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT], P0752_i_X, P0753_i_Y - 33))) {
374: #endif
375: #ifdef MEDIA508_F20E_F20J_X30J_P20JA_P20JB /* CHANGE2_01_OPTIMIZATION Inline code replaced by function calls */
```
```c
374: #endif
375: #ifdef MEDIA508_F20E_F20J_X30J_P20JA_P20JB /* CHANGE2_01_OPTIMIZATION Inline code replaced by function calls */
376:                         if ((((DOOR*)L1151_ps_Junk)->Button) && (F0376_COMMAND_IsPointInBox(G2210_aai_XYZ_DungeonViewClickable[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT], P0752_i_X, P0753_i_Y))) {
377: #endif
378: #ifdef MEDIA720_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J
```
```c
377: #endif
378: #ifdef MEDIA720_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J
379:                         if ((((DOOR*)L1151_ps_Junk)->Button) && (F0798_COMMAND_IsPointInZone(G2210_aai_XYZ_DungeonViewClickable[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT], P0752_i_X, P0753_i_Y))) {
380: #endif
381: #ifdef MEDIA014_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_G21E_A20ED_A20E_A20F_A20G_A21E_A22E_A22G_F20E_F20J
```

### CLIKVIEW.C — F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor
```c
3: #endif
4: #ifdef MEDIA042_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_G21E_A20ED_A20E_A20F_A20G_A21E_A22E_A22G_F20E_F20J_X30J_P20JA_P20JB_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J
5: STATICFUNCTION void F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor(
6: #ifdef MEDIA529_F20E_F20J_X30J_P20JA_P20JB_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J
7: void
```
```c
429:         L1150_ui_Multiple++;
430: #endif
431:                                                 F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor();
432:                                         }
433:                                 } else {
```
```c
494:                                         }
495:                                         T0377019:
496:                                         F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor();
497:                                 }
498:                         }
```

### MOVESENS.C — C127_SENSOR_WALL_CHAMPION_PORTRAIT
```c
1390:                         if ((L0757_ui_SensorType = M039_TYPE(L0755_ps_Sensor)) == C000_SENSOR_DISABLED)
1391:                                 goto T0275058_ProceedToNextThing;
1392:                         if ((G0411_i_LeaderIndex == CM1_CHAMPION_NONE) && (L0757_ui_SensorType != C127_SENSOR_WALL_CHAMPION_PORTRAIT))
1393:                                 goto T0275058_ProceedToNextThing;
1394:                         if (L0752_ui_Cell != P0587_ui_Cell)
```
```c
1499:                                         L0753_B_DoNotTriggerSensor = C0_FALSE;
1500:                                         break;
1501:                                 case C127_SENSOR_WALL_CHAMPION_PORTRAIT:
1502:                                         F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData);
1503:                                         goto T0275058_ProceedToNextThing;
```

### MOVESENS.C — F0280_CHAMPION_AddCandidateChampionToParty
```c
1500:                                         break;
1501:                                 case C127_SENSOR_WALL_CHAMPION_PORTRAIT:
1502:                                         F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData);
1503:                                         goto T0275058_ProceedToNextThing;
1504: #ifdef MEDIA006_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB /* CHANGE7_14_FIX Useless code removed */
```

### DUNVIEW.C — G0109_auc_Graphic558_Box_ChampionPortraitOnWall
```c
382: unsigned char G0107_auc_Graphic558_Box_ThievesEye_VisibleArea[4];
383: unsigned char G0108_auc_Graphic558_Box_ThievesEye_HoleInDoorFrame[4];
384: unsigned char G0109_auc_Graphic558_Box_ChampionPortraitOnWall[4];
385: unsigned char G0110_auc_Graphic558_Frame_StairsUpFront_D3L[8];
386: unsigned char G0111_auc_Graphic558_Frame_StairsUpFront_D3C[8];
```
```c
523: unsigned char G0107_auc_Graphic558_Box_ThievesEye_VisibleArea[4] = { 0, 95, 0, 94 };
524: unsigned char G0108_auc_Graphic558_Box_ThievesEye_HoleInDoorFrame[4] = { 0, 31, 19, 113 };
525: unsigned char G0109_auc_Graphic558_Box_ChampionPortraitOnWall[4] = { 96, 127, 35, 63 };
526: unsigned char G0110_auc_Graphic558_Frame_StairsUpFront_D3L[8] = { 0, 79, 25, 70, 40, 46, 0, 0 };
527: unsigned char G0111_auc_Graphic558_Frame_StairsUpFront_D3C[8] = { 64, 159, 25, 70, 48, 46, 0, 0 };
```
```c
3914: #endif
3915: #ifdef MEDIA008_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_G21E
3916:                         F0132_VIDEO_Blit(F0489_MEMORY_GetNativeBitmapOrGraphic(C026_GRAPHIC_CHAMPION_PORTRAITS), G0296_puc_Bitmap_Viewport, G0109_auc_Graphic558_Box_ChampionPortraitOnWall, (G0289_i_DungeonView_ChampionPortraitOrdinal & 0x0007) << 5, (G0289_i_DungeonView_ChampionPortraitOrdinal >> 3) * 29, C128_BYTE_WIDTH, C112_BYTE_WIDTH_VIEWPORT, C01_COLOR_DARK_GRAY); /* A portrait is 32x29 pixels */
3917: #endif
3918: #ifdef MEDIA413_A20ED_A20E_A20F_A20G_A21E_A22E_A22G
```
```c
3917: #endif
3918: #ifdef MEDIA413_A20ED_A20E_A20F_A20G_A21E_A22E_A22G
3919:                         F0132_VIDEO_Blit(F0489_MEMORY_GetNativeBitmapOrGraphic(C026_GRAPHIC_CHAMPION_PORTRAITS), G0296_puc_Bitmap_Viewport, G0109_auc_Graphic558_Box_ChampionPortraitOnWall, (G0289_i_DungeonView_ChampionPortraitOrdinal & 0x0007) << 5, (G0289_i_DungeonView_ChampionPortraitOrdinal >> 3) * 29, C128_BYTE_WIDTH, C112_BYTE_WIDTH_VIEWPORT, C01_COLOR_DARK_GRAY, 87, C136_HEIGHT_VIEWPORT); /* A portrait is 32x29 pixels */
3920: #endif
3921: #ifdef MEDIA529_F20E_F20J_X30J_P20JA_P20JB_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J
```

### DUNVIEW.C — M635_ZONE_PORTRAIT_ON_WALL
```c
3926:                                 L2452_i_Width = G2078_C32_PortraitWidth;
3927:                                 L2453_i_Height = G2079_C29_PortraitHeight;
3928:                                 F0654_Call_F0132_VIDEO_Blit(M772_CAST_PC(F0489_MEMORY_GetNativeBitmapOrGraphic(C026_GRAPHIC_CHAMPION_PORTRAITS)), M772_CAST_PC(G0296_puc_Bitmap_Viewport), F0635_(NULL, L2454_ai_XYZ, M635_ZONE_PORTRAIT_ON_WALL, &L2452_i_Width, &L2453_i_Height), (G0289_i_DungeonView_ChampionPortraitOrdinal & 0x0007) * G2078_C32_PortraitWidth, (G0289_i_DungeonView_ChampionPortraitOrdinal >> 3) * G2079_C29_PortraitHeight, C01_COLOR_DARK_GRAY);
3929:                         }
3930: #endif
```

### COORD.C — G2067_i_ViewportScreenX
```c
1691: #endif
1692: #endif
1693: int16_t G2067_i_ViewportScreenX = 0;
1694: #ifdef MEDIA571_F20J_X30J_P20JA_P20JB_F31J_X31J_P31J
1695: int16_t G2068_i_ViewportScreenY = 31;
```

### COORD.C — G2068_i_ViewportScreenY
```c
1693: int16_t G2067_i_ViewportScreenX = 0;
1694: #ifdef MEDIA571_F20J_X30J_P20JA_P20JB_F31J_X31J_P31J
1695: int16_t G2068_i_ViewportScreenY = 31;
1696: #endif
1697: #ifdef MEDIA562_F20E_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E
```
```c
1696: #endif
1697: #ifdef MEDIA562_F20E_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E
1698: int16_t G2068_i_ViewportScreenY = 33;
1699: #endif
1700: #ifdef MEDIA508_F20E_F20J_X30J_P20JA_P20JB
```
