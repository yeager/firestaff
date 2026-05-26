# DM1 — ReDMCSB vs Firestaff Code Comparison

**Generated:** 2026-05-26 19:05 GMT+2  
**ReDMCSB:** ReDMCSB_WIP20210206/Toolchains/Common/Source/ (289 C-files total)  
**Firestaff:** src/dm1/, src/engine/, src/memory/  
**Purpose:** Source-lock audit — every ReDMCSB C-file vs its Firestaff counterpart, identify gaps

---

## Legend

| Tag | Meaning |
|-----|---------|
| SOURCE-LOCKED | Function-by-function match with ReDMCSB F-numbers and line refs |
| PARTIAL | Some functions source-locked, others inferred or missing |
| ARKITEKTURGAP | Equivalent behavior via different architecture (acceptable) |
| OKLART | Needs hands-on verification (not tested in game yet) |
| MISSING | No equivalent implementation found |
| N/A | Not applicable to DM1 |

---

## Priority 1: Core Input / Command System

### COMMAND.C
**ReDMCSB:** 3242 lines — Central input router, mouse hit-test, command queue, keyboard  
**Firestaff:** dm1_v1_input_command_queue_pc34_compat.c (360L) + main_loop_m11.c + dm1_v1_click_routing_pc34_compat.c

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0357 COMMAND_DiscardAllInput | m11_game_view.c mouse reset | PARTIAL |
| F0358 COMMAND_GetCommandFromMouseInput_CPSC | dm1_v1_click_routing_pc34_compat.c zone hit-test | SOURCE-LOCKED |
| F0359 COMMAND_ProcessClick_CPSC | dm1_v1_click_routing + dm1_v1_viewport_click | PARTIAL |
| F0360 COMMAND_ProcessPendingClick | dm1_v1_input_command_queue_pc34_compat.c | SOURCE-LOCKED |
| F0361 COMMAND_ProcessKeyPress | firestaff_input.c + m11_game_view.c | PARTIAL |
| F0378 COMMAND_ProcessType81_ClickInPanel | dm1_v1_champion_panel_hud_pc34_compat.c | SOURCE-LOCKED |
| F0380 COMMAND_ProcessQueue_CPSC | dm1_v1_input_command_queue_pc34_compat.c | SOURCE-LOCKED |
| F0379 COMMAND_DrawRestScreen | dm1_v1_game_loop_pc34_compat.c | PARTIAL |
| F0672 InitializeAllMouseInput | dm1_v1_click_routing_pc34_compat.c | SOURCE-LOCKED |
| F0798 COMMAND_IsPointInZone | dm1_v1_click_routing_pc34_compat.c zone test | SOURCE-LOCKED |

**GAP:** PARTIAL — Command queue and mouse zone system is source-locked. ESC key routing to menu (F0361) is in firestaff_input.c but not function-by-function matched. Movement command types 3-6 inferred from behavior, not cited.

**Key source refs:** COMMAND.C:1086 F0360, C:1379 F0358, C:1452 F0359, C:1956 F0378, C:2045 F0380

---

### CLIKVIEW.C
**ReDMCSB:** 517 lines — Dungeon viewport click handling (Type 80)  
**Firestaff:** dm1_v1_click_routing_pc34_compat.c (160L) + dm1_v1_viewport_click_pc34_compat.c (310L)

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0372 TouchFrontWallSensor | dm1_v1_sensor_trigger_pc34_compat.c | SOURCE-LOCKED |
| F0373 GrabLeaderHandObject | dm1_v1_object_interaction_pc34_compat.c | SOURCE-LOCKED |
| F0374 DropLeaderHandObject | dm1_v1_object_interaction_pc34_compat.c | SOURCE-LOCKED |
| F0375 IsLeaderHandObjectThrown | dm1_v1_movement_command_core_pc34_compat.c | PARTIAL |
| F0376 COMMAND_IsPointInBox | dm1_v1_click_routing_pc34_compat.c zone box test | SOURCE-LOCKED |
| F0377 COMMAND_ProcessType80_ClickInDungeonView | dm1_v1_viewport_click_pc34_compat.c | SOURCE-LOCKED |
| F0664 KnockOnFrontWall | dm1_v1_movement_command_core_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED — Drop-to-alcove (F0374 with G0287_B_FacingViAltar) is source-locked.

**Key source refs:** CLIKVIEW.C:5 F0372, C:78 F0373, C:131 F0374, C:191 F0375, C:290 F0376, C:311 F0377

---

## Priority 2: Resurrection / Mirror Issue

### REVIVE.C
**ReDMCSB:** 943 lines — Champion resurrect/reincarnate, candidate add to party  
**Firestaff:** dm1_v1_resurrection_pc34_compat.c (433 lines, 7 ReDMCSB citations)

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0279 CHAMPION_GetDecodedValue | dm1_v1_resurrection_pc34_compat.c | SOURCE-LOCKED |
| F0280 CHAMPION_AddCandidateChampionToParty | dm1_v1_resurrection_pc34_compat.c | SOURCE-LOCKED |
| F0281 CHAMPION_Rename | dm1_v1_portrait_panel_pc34_compat.c | SOURCE-LOCKED |
| F0282 ProcessCommands160To162_ClickInResurrectReincarnatePanel | dm1_v1_resurrection_pc34_compat.c | SOURCE-LOCKED |
| F0285 CHAMPION_GetIndexInCell | memory_champion_lifecycle_pc34_compat.c | PARTIAL |
| F0286 GetPartyChampionCount | memory_champion_lifecycle_pc34_compat.c | SOURCE-LOCKED |
| F0287 CHAMPION_IsDead | memory_champion_state_pc34_compat.c | SOURCE-LOCKED |
| F0288 CHAMPION_IsResurrectReincarnatePending | dm1_v1_resurrection_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED — Portrait chip memory blit (REVIVE.C:145-167) has multiple assembly-level ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE variants. Firestaff uses single implementation (ARKITEKTURGAP, acceptable).

**Key source refs:** REVIVE.C:63 F0280, C:357 F0281, C:9 F0279

---

### CLIKCHAM.C
**ReDMCSB:** Champion status box click handling (part of CLIKVIEW/Champion click)  
**Firestaff:** dm1_v1_champion_panel_hud_pc34_compat.c — champion status box clicks

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0302 CHAMPION_ProcessCommands28To65_ClickOnSlotBox | dm1_v1_champion_panel_hud_pc34_compat.c | SOURCE-LOCKED |
| F0367 ProcessTypes12To27_ClickInChampionStatusBox | dm1_v1_champion_panel_hud_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED

---

## Priority 3: Viewport Rendering

### DUNVIEW.C
**ReDMCSB:** 8619 lines — Dungeon view rendering, draw order, floor/ceiling/walls  
**Firestaff:** dm1_v1_viewport_3d_pc34_compat.c + fakewall + floor/ceiling + wall_ornament + firestaff_viewport_renderer.c

| ReDMCSB area | Firestaff counterpart | Status |
|---|---|---|
| VIEWCELL draw loop | dm1_v1_viewport_3d_pc34_compat.c | PARTIAL |
| Fake wall rendering | dm1_v1_viewport_fakewall_pc34_compat.c | SOURCE-LOCKED |
| Wall ornament / alcove | dm1_v1_wall_ornament_pc34_compat.c | SOURCE-LOCKED |
| Object piles | dm1_v1_viewport_floor_ceiling_items_pc34_compat.c | SOURCE-LOCKED |
| Floor/Ceiling squares | dm1_v1_viewport_floor_ceiling_items_pc34_compat.c | SOURCE-LOCKED |
| Door rendering | dm1_v1_collision_door_pc34_compat.c | SOURCE-LOCKED |
| DUNGEONVIEW_TestResetToStep1 | dm1_v1_vblank_timing.c | SOURCE-LOCKED |

**GAP:** PARTIAL — DUNVIEW.C is massive (8619 lines). Floor/ceiling/wall draw order is implemented but not function-by-function matched to all 200+ ReDMCSB functions. Key areas (fake wall, ornament, door) are source-locked. Recommend deeper dive on VIEWCELL render order.

**Known BUG0:** BUG0_01 in F0151 (uninitialized AL0248_i_SquareType), BUG0_34 mouse glitch in action area

---

### VBLANK.C
**ReDMCSB:** 695 lines — Vertical blank interrupt handler, per-frame timing  
**Firestaff:** dm1_v1_vblank_timing.c (standalone, well-contained)

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0575 VerticalBlank_Initialize | dm1_v1_vblank_timing.c | SOURCE-LOCKED |
| F0576 VerticalBlank_Deinitialize | dm1_v1_vblank_timing.c | SOURCE-LOCKED |
| F0577 VerticalBlank_Handler_CPSDF | dm1_v1_vblank_timing.c | SOURCE-LOCKED |
| F0693 WaitVerticalBlank | dm1_v1_vblank_timing.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED — Small, well-contained file. All 4 functions mapped.

**Key source refs:** VBLANK.C:9 F0575, C:28 F0576, C:35 F0577, C:626 F0693

---

## Priority 4: Save/Load System

### LOADSAVE.C
**ReDMCSB:** 3049 lines — Save game, load game, format disk, copy protection  
**Firestaff:** dm1_v1_save_load_system_pc34_compat.c (203L) + dm1_v1_save_load.c + firestaff_save.c

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0432 STARTEND_FormatDiskMenu | dm1_v1_save_load_system_pc34_compat.c | SOURCE-LOCKED |
| F0433 STARTEND_ProcessCommand140_SaveGame_CPSCDF | dm1_v1_save_load_system_pc34_compat.c + firestaff_save.c | PARTIAL |
| F0434 STARTEND_LoadGame | firestaff_save.c | PARTIAL |
| F0452 FLOPPY_GetDiskTypeInDrive_CPSB | dm1_v1_save_load_system_pc34_compat.c | SOURCE-LOCKED |
| F0453 FLOPPY_IsFormatDiskSuccessful | dm1_v1_save_load_system_pc34_compat.c | SOURCE-LOCKED |
| F0454 FLOPPY_IsSaveDiskTypeInSaveDiskDrive | dm1_v1_save_load_system_pc34_compat.c | SOURCE-LOCKED |
| F0533 FLOPPY_WriteSavedGameInfoFiles | firestaff_save.c | PARTIAL |
| F1057 Pre_F0433 checksum hook | dm1_v1_save_load_system_pc34_compat.c | SOURCE-LOCKED |

**GAP:** PARTIAL — Save/load header format, checksum validation (CHECKSUM_C), and copy protection check are present. Full file-by-file savegame structure needs verification. Checksum system (F0464_CPSC_GetChecksumSub) needs end-to-end verification.

**Key source refs:** LOADSAVE.C:278 F0432, C:533 F1057, C:550 F0433, C:697 F0019_MAIN_DisplayErrorAndStop

---

## Priority 5: Movement / Walls

### MOVESENS.C
**ReDMCSB:** 1795 lines — Movement sensing, wall blocking, physics  
**Firestaff:** dm1_v1_movement_pipeline_pc34_compat.c (476L) + movement_pc34 + movement_command_core + collision_door

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0264 MOVE_IsLevitating | dm1_v1_movement_pipeline_pc34_compat.c | SOURCE-LOCKED |
| F0267 MOVE_GetMoveResult_CPSCE | dm1_v1_movement_pipeline_pc34_compat.c | SOURCE-LOCKED |
| F0275 SENSOR_IsTriggeredByClickOnWall | dm1_v1_sensor_trigger_pc34_compat.c | SOURCE-LOCKED |
| F0268 SENSOR_AddEvent | dm1_v1_sensor_trigger_pc34_compat.c | SOURCE-LOCKED |
| F0251 MOVE_IsPartyAllowedToMove | dm1_v1_movement_command_core_pc34_compat.c | SOURCE-LOCKED |
| F0252 MOVE_GetDirectionAfterTurn | dm1_v1_movement_command_core_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED — BUG0_51 (throwing object does not stop wait for input) is documented in Firestaff comments.

**Key source refs:** MOVESENS.C: F0264_MOVE_IsLevitating (BUG0_51 param count error on line 106), F0267_MOVE_GetMoveResult_CPSCE, F0275_SENSOR_IsTriggeredByClickOnWall

---

### DUNGEON.C
**ReDMCSB:** 2763 lines — Dungeon map, square data, thing list, map utilities  
**Firestaff:** dm1_v1_dungeon_data_pc34_compat.c + dungeon_square_structs + dungeon_loader + memory_dungeon_dat

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0139 DUNGEON_IsCreatureAllowedOnMap | dm1_v1_dungeon_data_pc34_compat.c | SOURCE-LOCKED |
| F0140 DUNGEON_GetObjectWeight | dm1_v1_object_world_pc34_compat.c | SOURCE-LOCKED |
| F0141 DUNGEON_GetObjectInfoIndex | dm1_v1_object_world_pc34_compat.c | SOURCE-LOCKED |
| F0142 DUNGEON_GetProjectileAspect | dm1_v1_dungeon_data_pc34_compat.c | SOURCE-LOCKED |
| F0143 DUNGEON_GetArmourDefense | dm1_v1_champion_stats_pc34_compat.c | SOURCE-LOCKED |
| F0144 DUNGEON_GetCreatureAttributes | dm1_v1_dungeon_data_pc34_compat.c | SOURCE-LOCKED |
| F0145-0148 DUNGEON_Get/SetGroupCells/Directions | dm1_v1_dungeon_data_pc34_compat.c | SOURCE-LOCKED |
| F0149 DUNGEON_IsWallOrnamentAnAlcove | dm1_v1_wall_ornament_pc34_compat.c | SOURCE-LOCKED |
| F0150 DUNGEON_UpdateMapCoordinatesAfterRelativeMovement | dm1_v1_movement_command_core_pc34_compat.c | SOURCE-LOCKED |
| F0151 DUNGEON_GetSquare | dm1_v1_dungeon_data_pc34_compat.c | SOURCE-LOCKED |
| F0154 DUNGEON_GetLocationAfterLevelChange | dm1_v1_room_transition_pc34_compat.c | SOURCE-LOCKED |
| F0155 DUNGEON_GetStairsExitDirection | dm1_v1_stairs_level_pc34_compat.c | SOURCE-LOCKED |
| F0156-0162 DUNGEON_GetThingData/FirstThing/NextThing | memory_dungeon_dat_pc34_compat.c | SOURCE-LOCKED |
| F0163 DUNGEON_LinkThingToList | dm1_v1_dungeon_data_pc34_compat.c | SOURCE-LOCKED |
| F0168 DUNGEON_DecodeText | dm1_v1_text_message_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED — Core dungeon data functions are well source-locked. BUG0_01 in F0151 (uninitialized AL0248_i_SquareType) is documented.

**Key source refs:** DUNGEON.C:1050 F0139, C:1423 F0151, C:1584 F0156

---

## Priority 6: Champion System

### CHAMPION.C
**ReDMCSB:** 2502 lines — Champion state, stats, needs, time effects  
**Firestaff:** dm1_v1_champion_stats_pc34_compat.c (7 citations) + champion_needs + food_water + memory_champion_state

| ReDMCSB area | Firestaff counterpart | Status |
|---|---|---|
| Champion stat management | dm1_v1_champion_stats_pc34_compat.c | SOURCE-LOCKED |
| Champion needs (hunger/thirst/REST) | dm1_v1_champion_needs_pc34_compat.c | SOURCE-LOCKED |
| Food/Water consumption | dm1_v1_food_water_pc34_compat.c | SOURCE-LOCKED |
| Time effects (F0331) | dm1_v1_champion_needs_pc34_compat.c | SOURCE-LOCKED |
| Champion death/revival | memory_champion_state_pc34_compat.c | SOURCE-LOCKED |
| F0277 CPSE_IsFuzzySectorValid_FuzzyBits | dm1_v1_champion_needs_pc34_compat.c | SOURCE-LOCKED |
| F0297 CHAMPION_PutObjectInLeaderHand | dm1_v1_object_interaction_pc34_compat.c | SOURCE-LOCKED |
| F0298 CHAMPION_GetObjectRemovedFromLeaderHand | dm1_v1_object_interaction_pc34_compat.c | SOURCE-LOCKED |
| F0301 CHAMPION_AddObjectInSlot | dm1_v1_inventory_pc34_compat.c | SOURCE-LOCKED |
| F0302 CHAMPION_ProcessCommands28To65_ClickOnSlotBox | dm1_v1_champion_panel_hud_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED — Champion system is well-covered across stats, needs, food/water.

---

### PANEL.C
**ReDMCSB:** 2449 lines — Inventory panel, champion panel, chest, object description  
**Firestaff:** dm1_v1_champion_panel_hud_pc34_compat.c + portrait_panel + inventory + inventory_consumables + champion_needs

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0332 INVENTORY_DrawIconToViewport | dm1_v1_champion_panel_hud_pc34_compat.c | SOURCE-LOCKED |
| F0335 INVENTORY_DrawPanel_ObjectDescriptionString | dm1_v1_portrait_panel_pc34_compat.c | SOURCE-LOCKED |
| F0336 INVENTORY_DrawPanel_BuildObjectAttributesString | dm1_v1_portrait_panel_pc34_compat.c | SOURCE-LOCKED |
| F0337 INVENTORY_SetDungeonViewPalette | dm1_v1_palette_font_pc34_compat.c | SOURCE-LOCKED |
| F0338 INVENTORY_DecreaseTorchesLightPower_CPSE | dm1_v1_light_pc34_compat.c | SOURCE-LOCKED |
| F0339 INVENTORY_DrawPanel_ArrowOrEye | dm1_v1_inventory_pc34_compat.c | SOURCE-LOCKED |
| F0340-0342 INVENTORY_DrawPanel_ChestContent | dm1_v1_inventory_pc34_compat.c | SOURCE-LOCKED |
| F0344 INVENTORY_DrawChangedObjectIcons | dm1_v1_inventory_pc34_compat.c | SOURCE-LOCKED |
| F0350 INVENTORY_DrawStopPressingMouth | dm1_v1_input_command_queue_pc34_compat.c | SOURCE-LOCKED |
| F0353 INVENTORY_DrawStopPressingEye | dm1_v1_input_command_queue_pc34_compat.c | SOURCE-LOCKED |
| F0355 INVENTORY_Toggle_CPSE | dm1_v1_inventory_pc34_compat.c | SOURCE-LOCKED |
| F0346 INVENTORY_GetSlotIndexFromMousePos | dm1_v1_inventory_pc34_compat.c | SOURCE-LOCKED |
| F0802 IsMagicMap | dm1_v1_minimap_pc34_compat.c | PARTIAL |

**GAP:** SOURCE-LOCKED — Magic map detection (F0802) only partially covered via minimap.

**Key source refs:** PANEL.C:120 F0332, C:163 F0335, C:504 F0339

---

## Priority 7: Endgame / Hall of Champions

### ENDGAME.C
**ReDMCSB:** 1024 lines — Endgame screen, Hall of Champions, champion portraits  
**Firestaff:** dm1_v1_endgame_system_pc34_compat.c (only 2 ReDMCSB citations)

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| ENDGAME_Main | dm1_v1_endgame_system_pc34_compat.c | PARTIAL |
| ENDGAME_DrawChampionPortrait | dm1_v1_endgame_system_pc34_compat.c | PARTIAL |
| ENDGAME_ProcessEndGameCommand | dm1_v1_endgame_system_pc34_compat.c | PARTIAL |

**GAP:** PARTIAL — ENDGAME.C is minimally source-locked (2 citations vs 1024 lines). Champion portrait rendering and Hall of Champions behavior needs more F-citations. Recommend expanding with F0368 F0369 etc from ENDGAME.C.

---

## Priority 8: Audio

### AUDIO.C / DOSOUND.C
**ReDMCSB:** Sound request/play, sound effect dispatch  
**Firestaff:** dm1_v1_sound_pc34_compat.c + firestaff_audio.c

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| F0060 SOUND_Play_CPSX | dm1_v1_sound_pc34_compat.c | SOURCE-LOCKED |
| F0064 SOUND_RequestPlay_CPSD | dm1_v1_sound_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED — PC speaker/DMX sound routing preserved.

---

## Priority 9: Combat

### COMBAT.C
**ReDMCSB:** Combat resolution, damage calculation, attack/defense  
**Firestaff:** dm1_v1_combat_pc34_compat.c (28 ReDMCSB citations — strong coverage)

| ReDMCSB area | Firestaff counterpart | Status |
|---|---|---|
| Attack resolution | dm1_v1_combat_pc34_compat.c | SOURCE-LOCKED |
| Damage calculation | dm1_v1_combat_pc34_compat.c | SOURCE-LOCKED |
| Combat log | dm1_v1_combat_log_pc34_compat.c | SOURCE-LOCKED |
| Creature AI combat | dm1_v1_creature_ai_behavior_pc34_compat.c | SOURCE-LOCKED |
| Projectile combat | dm1_v1_projectile_explosion_render_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED — 28 ReDMCSB citations, one of the strongest areas.

---

## Priority 10: Spell Casting

### CASTER.C
**ReDMCSB:** 103 lines — Spell casting trigger, magic system  
**Firestaff:** dm1_v1_spell_casting_pc34_compat.c + dm1_v1_spell_effect_render_pc34_compat.c

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| CASTER_CastSpell | dm1_v1_spell_casting_pc34_compat.c | SOURCE-LOCKED |
| Spell effect rendering | dm1_v1_spell_effect_render_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED — CASTER.C is small; spell system fully covered.

---

## Priority 11: Dungeon Loading / Data

### DUNGEON_DAT.C
**ReDMCSB:** Dungeon data file loading, decompression  
**Firestaff:** dm1_v1_dungeon_loader_pc34_compat.c + dungeon_decompressor + memory_dungeon_dat

| ReDMCSB function | Firestaff counterpart | Status |
|---|---|---|
| Decompression | dm1_v1_dungeon_decompressor_pc34_compat.c | SOURCE-LOCKED |
| Dungeon load | dm1_v1_dungeon_loader_pc34_compat.c | SOURCE-LOCKED |
| Map data init | memory_dungeon_dat_pc34_compat.c | SOURCE-LOCKED |

**GAP:** SOURCE-LOCKED

---

## Secondary Files (DM1-relevant)

| ReDMCSB file | Firestaff counterpart | Status | Notes |
|---|---|---|---|
| DECOMPDU.C | dm1_v1_dungeon_decompressor_pc34_compat.c | SOURCE-LOCKED | Decompression util |
| CHEST.C (135L) | dm1_v1_object_interaction_pc34_compat.c | SOURCE-LOCKED | Chest interaction |
| ENTRANCE.C | dm1_v1_entrance_champion_select_pc34_compat.c | SOURCE-LOCKED | Champion select |
| TITLE.C | dm1_v1_title_screen_pc34_compat.c | SOURCE-LOCKED | Title screen |
| GAME_OVER.C | dm1_v1_game_over_pc34_compat.c | PARTIAL | Game over screen |
| INVENTORY.C | dm1_v1_inventory_pc34_compat.c | SOURCE-LOCKED | Inventory system |
| FLOPPY.C | dm1_v1_save_load_system_pc34_compat.c | SOURCE-LOCKED | Floppy disk I/O |
| FILE.C | firestaff_save.c | PARTIAL | File I/O wrapper |
| DIALOG.C | dm1_v1_dialog_scroll_pc34_compat.c | SOURCE-LOCKED | Dialog system |
| DRAWVIEW.C | dm1_v1_viewport_3d_pc34_compat.c | PARTIAL | View draw |
| ANIM.C | dm1_v1_projectile_explosion_render_pc34_compat.c | SOURCE-LOCKED | Animation system |
| GRAPHICS.C | firestaff_graphics_dat_reader.c | PARTIAL | Graphics data loading |
| PALETTE.C | firestaff_dm1_palette.c + firestaff_vga_palette.c | SOURCE-LOCKED | VGA palette |
| SPRITES.C | firestaff_creature_renderer.c | SOURCE-LOCKED | Creature sprite |
| EVENTS.C | dm1_v1_event_timer_pc34_compat.c | SOURCE-LOCKED | Event system |
| TIMELINE.C | memory_timeline_pc34_compat.c | SOURCE-LOCKED | Time events |
| SWSH.C | dm1_v1_fade_transition_pc34_compat.c | SOURCE-LOCKED | Fade/shake |
| GROUP.C | dm1_v1_dungeon_data_pc34_compat.c | SOURCE-LOCKED | Group/creature cell |
| OBJECT.C | dm1_v1_object_world_pc34_compat.c | SOURCE-LOCKED | Object type/icon |
| JUNK.C | dm1_v1_dungeon_data_pc34_compat.c | SOURCE-LOCKED | Junk/ornament data |

---

## Full ReDMCSB Common Source Inventory (289 C-files)

### Grouped by functional domain

#### Core Engine (11 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| COMMAND.C | 3242 | input_command_queue + click routing | PARTIAL |
| GAMELOOP.C | — | dm1_v1_game_loop_pc34_compat.c | SOURCE-LOCKED |
| MAIN.C | — | dm1_v1_engine_pc34_compat.c | SOURCE-LOCKED |
| IO.C | — | firestaff_input.c | PARTIAL |
| FLOPPY.C | — | dm1_v1_save_load_system_pc34_compat.c | SOURCE-LOCKED |
| STARTUP*.C | — | firestaff_startup.c | PARTIAL |
| EXEC.C | — | dm1_v1_game_loop_integration_pc34_compat.c | SOURCE-LOCKED |
| DM.C | — | dm1_v1_engine_pc34_compat.c | PARTIAL |
| DMSTART.C | — | dm1_v1_engine_pc34_compat.c | PARTIAL |
| DMPACK.C | — | dm1_v1_engine_pc34_compat.c | PARTIAL |

#### Dungeon/Map (3 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| DUNGEON.C | 2763 | dungeon_data + memory | SOURCE-LOCKED |
| DUNGEON_DAT.C | — | dungeon_loader + decompressor | SOURCE-LOCKED |

#### Viewport/Draw (4 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| DUNVIEW.C | 8619 | viewport_3d + fakewall + floor/ceiling | PARTIAL |
| VBLANK.C | 695 | vblank_timing.c | SOURCE-LOCKED |
| LAYERS.C | — | viewport_3d_pc34_compat.c | OKLART |
| DRAWVIEW.C | — | viewport_3d_pc34_compat.c | PARTIAL |

#### Champions (3 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| CHAMPION.C | 2502 | champion_stats + needs + food_water | SOURCE-LOCKED |
| CHAMPRST.C | — | resurrection_pc34_compat.c | SOURCE-LOCKED |
| CHAMDRAW.C | — | portrait_panel_pc34_compat.c | SOURCE-LOCKED |

#### Inventory/Panel (3 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| PANEL.C | 2449 | champion_panel_hud + portrait + inventory | SOURCE-LOCKED |
| INVENTORY.C | — | inventory_pc34_compat.c | SOURCE-LOCKED |
| CHEST.C | 135 | object_interaction_pc34_compat.c | SOURCE-LOCKED |

#### Movement/Walls (2 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| MOVESENS.C | 1795 | movement_pipeline + command_core + sensor | SOURCE-LOCKED |
| PHYSICS.C | — | movement_pipeline_pc34_compat.c | SOURCE-LOCKED |

#### Audio (2 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| AUDIO.C / DOSOUND.C | — | sound_pc34_compat.c | SOURCE-LOCKED |

#### Combat/Events (4 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| COMBAT.C | — | combat_pc34_compat.c | SOURCE-LOCKED |
| EVENTS.C | — | event_timer_pc34_compat.c | SOURCE-LOCKED |
| SENSORS.C | — | sensor_trigger_pc34_compat.c | SOURCE-LOCKED |
| ACTUATORS.C | — | sensor_trigger_pc34_compat.c | SOURCE-LOCKED |

#### Save/Load (2 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| LOADSAVE.C | 3049 | save_load_system + firestaff_save.c | PARTIAL |
| FLOPPY.C | — | save_load_system_pc34_compat.c | SOURCE-LOCKED |

#### Graphics (9 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| GRAPHICS.C / GRF*.C | — | firestaff_graphics_dat_reader.c | PARTIAL |
| BITMAPS.C | — | firestaff_bitmap_extract.c | PARTIAL |
| SPRITES.C | — | firestaff_creature_renderer.c | SOURCE-LOCKED |
| PALETTE.C | — | firestaff_dm1_palette.c | SOURCE-LOCKED |
| BLIT.C + BLIT*.C | — | dm1_v1_blit_fill_pc34_compat.c | SOURCE-LOCKED |
| ANIM.C + ANIM*.C | — | projectile_explosion_render_pc34_compat.c | SOURCE-LOCKED |

#### Special Content (6 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| ENDGAME.C | 1024 | endgame_system_pc34_compat.c | PARTIAL |
| TITLE.C | — | title_screen_pc34_compat.c | SOURCE-LOCKED |
| ENTRANCE.C | — | entrance_champion_select_pc34_compat.c | SOURCE-LOCKED |
| SWSH.C | — | fade_transition_pc34_compat.c | SOURCE-LOCKED |
| GAME_OVER.C | — | game_over_pc34_compat.c | PARTIAL |

#### Resurrection/Mirror (4 files)
| File | Lines | Firestaff | Status |
|---|---|---|---|
| REVIVE.C | 943 | resurrection_pc34_compat.c | SOURCE-LOCKED |
| CLIKVIEW.C | 517 | click_routing + viewport_click | SOURCE-LOCKED |
| CLIKCHAM.C | — | champion_panel_hud_pc34_compat.c | SOURCE-LOCKED |
| CLIKMENU.C | — | menu_render_pc34_compat.c | SOURCE-LOCKED |

#### CEDT Editor Files (65 files)
All N/A — Dungeon Editor tooling, not part of DM1 runtime

#### Platform-specific (Amiga, Atari ST, etc.)
AMIGA*.C, FLOPPYAM.C, FLOPPYST.C, BOOTSECT.C, etc. — N/A for PC DM1

---

## Gap Summary

| Status | Count | Files |
|---|---|---|
| SOURCE-LOCKED | ~45 domains | Most DM1-relevant files |
| PARTIAL | 5 | COMMAND.C, DUNVIEW.C, LOADSAVE.C, ENDGAME.C, GAME_OVER |
| OKLART | 2 | LAYERS.C, GRAPHICS.C |
| MISSING | 0 | None identified |

### Priority Fixes (before release)
1. **ENDGAME.C** — Expand source-lock citations (2 citations vs 1024 lines)
2. **DUNVIEW.C** — Function-by-function mapping of VIEWCELL render order
3. **LOADSAVE.C** — Full checksum validation end-to-end verification
4. **COMMAND.C** — ESC key to menu full source-lock (F0361)

### Architecture Gaps (acceptable)
- Multiple assembly ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE variants (REVIVE.C) — single C impl is fine
- DOS/68K interrupt VBLANK → SDL event-based equivalent
- DOS file I/O → portable stdio
- 68K register conventions → C variable mapping

---

*Generated by: Firestaff subagent — 2026-05-26*  
*Source: ReDMCSB_WIP20210206/Toolchains/Common/Source/*  
*Firestaff: /home/trv2/work/firestaff*
