# Pass267 DM1 V2 completion matrix gate

Date: 2026-05-06 19:36+02:00
Worktree: `<firestaff-worktree>/firestaff-oauth-n2-dm1v2-completion-matrix-gate-pass267-20260506-1936`
Audited head: `b272cad9751eb6b11998d5ed89a9c9e00757381f` (`b272cad`)
Scope: DM1 V2 readiness/completion gates only. This pass does **not** claim DM1 V2 is complete.

## Conclusion

DM1 V2 verified completion status: **not verified complete / percentage unknown**.

What is verified on this head:

- 32 top-level `dm1_v2_*_pc34.c` modules exist and have matching headers via `tools/verify_dm1_v2_completion_matrix.py`.
- 14 existing DM1 V2 / V2 CTest gates pass on N2.
- The V2 shared logical catalog validates with 8 categories, 45 logical IDs, 4 manifest bindings, and 3 source-evidence bindings.

What blocks completion:

- The gate set is mostly scaffold/source-lock coverage, not a definition-of-done model.
- Many V2 modules have no module-specific CTest or ReDMCSB/source-lock evidence.
- Runtime input parity, full dungeon visual composition, assets/material binding, package smoke, and V1 regression proof for V2 are still unproven.
- Existing `parity-evidence/verification/dm1_v2_completion_status.json` is stale for this pass: it records audited head `978463b`, while this worktree is `b272cad`.

## ReDMCSB/source/reference audit before implementation/probe

Primary source root audited: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

Existing V2 source-lock gates cite these relevant ReDMCSB anchors before exercising Firestaff V2 code:

- Movement command IDs and queue: `DEFS.H:238` `C001_COMMAND_TURN_LEFT`, `DEFS.H:243` `C006_COMMAND_MOVE_LEFT`, `COMMAND.C:2045` `F0380_COMMAND_ProcessQueue_CPSC`, `COMMAND.C:2151` `F0365_COMMAND_ProcessTypes1To2_TurnParty`.
- Runtime loop/input wait: `GAMELOOP.C:164` `G0321_B_StopWaitingForPlayerInput`, `GAMELOOP.C:215` `F0380_COMMAND_ProcessQueue_CPSC`, `GAMELOOP.C:219` `G0301_B_GameTimeTicking`.
- Coordinate updates: `DUNGEON.C:35` east-step table, `DUNGEON.C:40` north-step table, `DUNGEON.C:1371` `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`, `DUNGEON.C:1389` table application.
- Viewport/walls: `DUNVIEW.C:8318` `F0128_DUNGEONVIEW_Draw_CPSF`, `DUNVIEW.C:8357` flipped wall bitmap parity, `DUNVIEW.C:8498-8499` D3C draw route, `DUNVIEW.C:6697-6720` wall occlusion termination.
- HUD/champion status: `TIMELINE.C:1817` `F0260_TIMELINE_RefreshAllChampionStatusBoxes`, `PANEL.C:2195` `F0354_INVENTORY_DrawStatusBoxPortrait`.
- Lighting: `PANEL.C:336` `F0337_INVENTORY_SetDungeonViewPalette`, `PANEL.C:370` torch-light audit comment, `PANEL.C:412` light-power table use, `PANEL.C:417` magical light amount.

No new implementation was added beyond this report; the obvious missing hook was status clarity, so the useful change is this concrete current-head matrix.

## Gates run on N2

- `cmake -S . -B /tmp/firestaff-pass267-dm1v2-gate -DCMAKE_BUILD_TYPE=Release`: pass
- `cmake --build /tmp/firestaff-pass267-dm1v2-gate --target test_dm1_v2_movement_viewport_pc34 test_dm1_v2_runtime_shell_pc34 test_dm1_v2_camera_controller_pc34 test_dm1_v2_movement_command_adapter_pc34 test_dm1_v2_hud_overlay_pc34 test_dm1_v2_lighting_dynamic_pc34 -j2`: pass
- `ctest --test-dir /tmp/firestaff-pass267-dm1v2-gate -R "dm1_v2|v2_" --output-on-failure`: pass, 14/14
- `python3 scripts/validate_v2_logical_catalog.py --require-existing-manifest-binding fs.v2.shared.dungeon-view.viewport.frame --require-bound-manifest-source-evidence fs.v2.shared.dungeon-view.viewport.frame`: pass

Passed CTest names:

1. `dm1_v2_upscale_dry_run_validator`
2. `dm1_v2_movement_viewport_pc34`
3. `dm1_v2_completion_matrix_gate`
4. `dm1_v2_runtime_shell_pc34`
5. `dm1_v2_runtime_shell_source_lock`
6. `dm1_v2_camera_controller_pc34`
7. `dm1_v2_camera_controller_source_lock`
8. `dm1_v2_movement_command_adapter_pc34`
9. `dm1_v2_movement_command_adapter_source_lock`
10. `dm1_v2_hud_overlay_pc34`
11. `dm1_v2_hud_overlay_source_lock`
12. `dm1_v2_lighting_dynamic_pc34`
13. `dm1_v2_lighting_dynamic_source_lock`
14. `dm1_v2_viewport_wall_occlusion_source_lock`

## Completion matrix

Status meanings:

- **verified/pass**: existing test/gate passed in this pass.
- **unknown**: code/module exists, but current evidence does not prove completion.
- **fail**: an existing gate failed. No DM1 V2 gates failed in this pass.

| Module/lane | Evidence artifact/test | Status | Exact blocker | Next pass if not done |
|---|---|---:|---|---|
| runtime/shell: `dm1_v2_runtime_pc34.c` | `dm1_v2_runtime_shell_pc34`, `dm1_v2_runtime_shell_source_lock` | verified/pass | Shell scaffold is source-locked, but full original input/runtime transcript parity is not proven. | Add original-runtime command transcript adapter gate for V2 shell commands. |
| runtime/movement: `dm1_v2_movement_engine_pc34.c` | `dm1_v2_movement_viewport_pc34` | verified/pass | Unit movement/collision checks pass; full DM1 movement semantics and door interactions are not proven. | Source-lock and test turn/move/blocked/door interaction sequences against ReDMCSB/runtime traces. |
| runtime/movement-command: `dm1_v2_movement_command_adapter_pc34.c` | `dm1_v2_movement_command_adapter_pc34`, `dm1_v2_movement_command_adapter_source_lock` | verified/pass | Command IDs and route are source-locked; end-to-end input delivery/touch/key parity is not proven. | Add key/mouse/touch command-route parity transcript gate. |
| visual/camera: `dm1_v2_camera_controller_pc34.c` | `dm1_v2_camera_controller_pc34`, `dm1_v2_camera_controller_source_lock` | verified/pass | Presentation interpolation passes; original frame timing and capture parity are not proven. | Add frame-timed movement/turn camera capture comparator. |
| runtime/viewport: `dm1_v2_viewport_renderer_pc34.c` | `dm1_v2_movement_viewport_pc34`, `dm1_v2_viewport_wall_occlusion_source_lock` | verified/pass | D3C wall occlusion and flip parity are source-locked; full distance-row wall/door/stairs/object composition is not proven. | Add D0-D3 viewport composition matrix with original runtime/reference captures. |
| asset/upscale: `dm1_v2_texture_upscale_pc34.c` | `dm1_v2_upscale_dry_run_validator` | verified/pass | Upscale dry-run passes; production asset binding and visual acceptance are not complete. | Bind viewport/wall/door logical IDs to source-evidenced manifests and screenshot gates. |
| ui/hud: `dm1_v2_hud_overlay_pc34.c` | `dm1_v2_hud_overlay_pc34`, `dm1_v2_hud_overlay_source_lock` | verified/pass | HUD overlay basics pass; full champion panels/status/touch-zone equivalence is absent. | Add champion status panel/touch route source-lock tests. |
| visual/lighting: `dm1_v2_lighting_dynamic_pc34.c` | `dm1_v2_lighting_dynamic_pc34`, `dm1_v2_lighting_dynamic_source_lock` | verified/pass | Lighting math is source-referenced; integrated viewport palette/fog/capture parity is not proven. | Add integrated viewport lighting/fog capture gate using torch/magical light states. |
| assets/catalog shared V2 | `scripts/validate_v2_logical_catalog.py` required viewport binding/source-evidence | verified/pass | Catalog validates; many logical IDs remain planned/blocked/in-progress. | Convert dungeon wall/door/floor/stairs IDs from planned to source-evidenced manifest bindings. |
| support/achievements: `dm1_v2_achievements_pc34.c` | module/header inventory only | unknown | No module-specific CTest/source-lock found. | Add deterministic achievement event/state tests or defer out of V2 completion definition. |
| support/audio: `dm1_v2_audio_mixer_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add mixer behavior test or mark non-blocking for DM1 V2 gameplay completion. |
| support/persistence: `dm1_v2_auto_save_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add autosave slot/interval persistence test and package smoke. |
| visual/camera-fx: `dm1_v2_camera_shake_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add deterministic shake offset/decay test or exclude from core completion. |
| ui/champion: `dm1_v2_champion_select_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add champion select/status source-lock and route tests. |
| visual/creature: `dm1_v2_creature_animation_pc34.c` | module/header inventory only | unknown | No creature render/capture gate found. | Add creature aspect/distance source-lock render gate. |
| visual/effects: `dm1_v2_damage_numbers_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add deterministic render/lifetime test or classify as enhancement. |
| support/audio: `dm1_v2_footstep_audio_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add movement-triggered footstep event test or classify as enhancement. |
| ui/input: `dm1_v2_input_remap_pc34.c` | module/header inventory only | unknown | No input-remap/touch/key delivery gate found. | Add key/mouse/touch remap route tests. |
| ui/inventory: `dm1_v2_inventory_sort_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add inventory list/sort/filter behavior test; decide if V2 core or enhancement. |
| ui/journal: `dm1_v2_journal_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add persistence/search tests or classify as enhancement. |
| runtime/transition: `dm1_v2_level_transition_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add stairs/level transition state tests. |
| ui/message-log: `dm1_v2_message_log_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add add/scroll/filter/render bounds tests. |
| ui/minimap: `dm1_v2_minimap_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add map visibility/render route tests or mark optional. |
| visual/effects: `dm1_v2_particle_emitter_presets_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add preset count/parameter tests or classify as enhancement. |
| visual/effects: `dm1_v2_particle_system_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add deterministic spawn/update/render bounds tests. |
| runtime/ai: `dm1_v2_pathfinding_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add grid path/collision tests or classify as non-core. |
| visual/transition: `dm1_v2_screen_transition_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add deterministic fade/wipe lifecycle tests. |
| support/tooling: `dm1_v2_screenshot_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add screenshot writer smoke with temp output and cleanup. |
| runtime/movement: `dm1_v2_smooth_movement_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add interpolation lifecycle tests or fold into camera-controller gate. |
| visual/effects: `dm1_v2_spell_effect_overlay_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add spell overlay lifecycle/render bounds tests. |
| support/stats: `dm1_v2_stat_tracker_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add increment/save/load/reset tests. |
| ui/input: `dm1_v2_tooltip_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add hover/show/hide/render bounds tests or classify optional. |
| visual/effects: `dm1_v2_weather_fx_pc34.c` | module/header inventory only | unknown | No CTest/source-lock found. | Add deterministic weather lifecycle/render bounds tests or classify optional. |

## Failing/unknown lanes

Failing lanes: none from existing DM1 V2 gates run in this pass.

Unknown/unproven completion lanes:

1. Runtime input parity: no original-runtime transcript gate for key/mouse/touch delivery into V2.
2. Full viewport/walls/dungeon visuals: current wall occlusion gate is narrow; full distance-row composition, materials, doors, stairs, creatures/items/effects are unproven.
3. Assets: catalog validates, but most dungeon-view logical IDs are planned/blocked rather than source-evidenced manifest bindings.
4. UI/champion/inventory/message/minimap: mostly module inventory only.
5. Support/enhancement modules: audio, save, stats, achievements, screenshots, journal, weather/particles have no module-specific gate in current CTest.
6. Release readiness: no V2 launcher/package smoke plus V1 regression gate bundle tied to V2 completion.

## Next 3 concrete passes

1. **V2 input/runtime transcript pass:** connect V2 command adapter/runtime shell to original-runtime command transcript evidence for turn/move/blocked/invalid commands and key/mouse/touch routes.
2. **V2 viewport composition pass:** extend viewport source-lock from D3C wall occlusion to D0-D3 wall/door/stairs/floor/ceiling/object composition with source-cited ReDMCSB anchors and screenshot/capture comparator.
3. **V2 asset binding pass:** promote the dungeon-view logical IDs for wall/front, wall/side, floor, ceiling, doors, and stairs from planned/blocked to source-evidenced manifest bindings, then gate with `validate_v2_logical_catalog.py` requirements.
