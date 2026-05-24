# DM1 V1 Test Coverage Audit

## Source-Lock: ReDMCSB WIP20210206 / Toolchains/Common/Source/

Total CTest suite: 387 tests
DM1 V1 tests (dm1_v1 prefix): ~240 tests
Source files: 68 dm1_v1_*_pc34_compat.c files
Test files: 39 test_dm1_v1*.c

## Probe Tests (NOT BUILT)

dm1_v1_viewport_draw_order_probe - exists in probes/dm1/ but not built
dm1_v1_game_loop_redraw_cadence_probe - exists in probes/dm1/ but not built  
dm1_v1_viewport_palette_as_before_probe - exists in probes/dm1/ but not built

CTest entries for these fail with Could not find executable.

## PC34 Compat Tests (39 files, grouped by system)

Movement/Collision (7): movement_core, movement_command_core, movement_timing,
  command_movement_sensor_timing, movement_pipeline, group_move_removal, turn_step_timing_gate

Viewport (5): viewport_3d, viewport_click, viewport_hand_overlay,
  dungeon_view_touch_route, viewport_palette_as_before_probe (not built)

Inventory (3): inventory_equip_slots, inventory_backpack_chest, inventory_consumables

Champion (4): champion_stats, champion_panel_hud, champion_needs, resurrection

Combat/Creatures (5): combat, creature_ai_behavior, creature_render, creature_sound,
  projectile_explosion_render

Sensors/Objects (5): sensor_trigger, object_interaction, door_button_click,
  fountain_interaction, group_management

Spells (2): spell_casting, light_darkness_spell_bridge

UI/Dialog (3): text_message, dialog_scroll, input_command_queue

Save/Load (2): save_load, save_load_slots

Other (3): room_transition, event_timer, endgame_system

## Coverage Gaps

- Dungeon decompression LZW edge cases: not tested
- GRAPHICS.DAT with real canonical file: no integration test
- TITLE.DAT asset loading: not tested (zoom animation tested only)
- Ambient dungeon sound: FAILING test dm1_v1_ambient_dungeon_sound_source_lock
- Projectile collision detection: not independently tested
- Pit/fakewall edge cases: partial only
- BUG0_86 champion portrait memory limitation: not tested
- BUG0_83 Thieves Eye hole animation: not independently tested
- BUG0_02 timeline 24-bit overflow: no test for ~850+ hour play session

## CTest Run Results (387 tests)

89% passed (346/387), 41 FAILED

FAILURES:
- M12 SEGFAULT (8 tests): menu_hit, csb_v1_launch_blocker, m12_quick_resume,
  dm1_v2_launch_smoke, m12_startup_menu, m11_game_view, m11_capture_route_state,
  m11_turn_viewport_orientation, dm1_v1_hall_walkaround_runtime
- DM1 V1 viewport wall occlusion (16 tests): pass508/511/515/516/519/560/561/562/563/
  565/576/577 wall occlusion source-lock gates
- DM1 V1 front cell collision (1 test): pass562_dm1_v1_front_cell_collision_source_lock
- Source-lock gates (4 tests): pass373, pass623, pass625, pass626
- Misc DM1 V1 (4 tests): alcove item, pit floor ornament BUG0_64, status refresh order,
  hall walkaround
- Ambient sound (1 test): dm1_v1_ambient_dungeon_sound_source_lock
- CSB V1 experimental launch intent (1 test)
