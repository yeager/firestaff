# Firestaff DM1 V2 completion plan

Goal: finish DM1 V2 with the same coding method used for the successful DM1 V1 work: source first, narrow modules, hard gates, small commits, no unverified drops.

## Non-negotiables

1. **Source audit first**
   - Every V2 lane starts by reading the relevant V1/ReDMCSB source and existing Firestaff V1 module.
   - V2 may modernize presentation, but not invent gameplay semantics.
   - Every implementation comment must cite source anchors where behavior comes from.

2. **One lane, one commit series**
   - One functional lane owns one coherent worktree/branch.
   - No parallel workers on the same feature.
   - No “dump files and hope” merges.

3. **Gate before commit**
   - C integration test for runtime behavior.
   - Python source-lock gate for source anchors and manifests.
   - `cmake --build` and targeted `ctest` green before commit.

4. **Main verifies workers**
   - Subagent reports are evidence, not truth.
   - Main checks `git status`, `git diff`, build/test output, and source anchors before accepting anything.

5. **Model discipline**
   - Sonnet/Opus for complex C/source-lock work.
   - q3.6 only for small, isolated probes.
   - At most one q3.6 job active at a time.

## Definition of done for V2

V2 is done when Firestaff can run a coherent DM1 V2 preview from the launcher with:

- modern scalable viewport and HUD,
- smooth movement/turning/camera transitions,
- source-faithful command semantics,
- V2 asset manifests for viewport, UI, creatures, items, spells and effects,
- deterministic tests and source-lock gates,
- clean macOS package build,
- no regressions in V1 gates.

## Work phases

### Phase 0 — Inventory and gate scaffold

Purpose: turn current V2 pieces into a controlled pipeline.

Existing pieces:

- `dm1_v2_movement_engine_pc34.*`
- `dm1_v2_viewport_renderer_pc34.*`
- `dm1_v2_texture_upscale_pc34.*`
- `dm1_v2_*` UI/effects/support modules
- `assets-v2/manifests/*`
- `tools/test_v2_upscale_dry_run.py`
- `tools/verify_v2_viewport_asset_source_lock.py`
- `test_dm1_v2_movement_viewport_pc34.c`

Tasks:

- Add `tools/verify_dm1_v2_completion_matrix.py`.
- Add `parity-evidence/verification/dm1_v2_completion_matrix.json`.
- Classify every V2 module as one of: runtime, visual, asset, UI, support, orphan.
- Gate that every non-orphan V2 module has at least one test/gate owner.

Done when:

- `ctest -R dm1_v2` covers all current V2 runtime modules.
- Completion matrix is checked into repo.

### Phase 1 — V2 runtime shell

Purpose: make V2 a real launchable mode, not loose modules.

Source anchors:

- V1 launcher handoff: `main_loop_m11.c`, `menu_startup_m12.*`
- ReDMCSB command loop: `GAMELOOP.C`, `COMMAND.C`
- Existing V1 runtime shell: `m11_game_view.*`

Tasks:

- Add a V2 runtime state wrapper, e.g. `dm1_v2_runtime_pc34.*`.
- Wire V2 presentation mode in the launcher without touching V1 behavior.
- Add a deterministic smoke path: launch V2, render first frame, process one input tick, exit.

Gates:

- `test_dm1_v2_runtime_shell_pc34`
- `tools/verify_dm1_v2_runtime_shell_source_lock.py`
- Headless launch-smoke for V2.

### Phase 2 — Movement, turning and camera

Purpose: finish the modern movement feel while preserving DM1 semantics.

Source anchors:

- ReDMCSB coordinate updates: `DUNGEON.C:F150_wzzz_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`
- Command queue: `COMMAND.C:F380_xzzz_COMMAND_ProcessQueue_COPYPROTECTIONC`
- Movement gates/timing: `GAMELOOP.C`, champion movement ticks in `CHAMPION.C`
- Existing Firestaff V1 movement modules.

Tasks:

- Lock movement command mapping: forward/back/strafe/turn.
- Add camera interpolation layer over source-faithful tile movement.
- Separate input command, logical movement, and visual camera offset.
- Ensure collision and door interaction use existing V1 semantics.

Gates:

- `test_dm1_v2_movement_camera_pc34`
- `test_dm1_v2_turning_pc34`
- Source-lock gate for ReDMCSB movement anchors.

### Phase 3 — Viewport, walls and lighting

Purpose: make V2 viewport visibly modern while keeping DM1 draw order.

Source anchors:

- ReDMCSB viewport constants: `DEFS.H`, `DUNVIEW.C`
- Draw order and field aspects: `DUNVIEW.C`, `GRAPHICS.C`
- Existing V1 viewport modules and asset gates.

Tasks:

- Finish V2 wall/floor/ceiling material pipeline.
- Implement distance-row composition and clipping.
- Add dynamic lighting/fog as a V2 overlay, not a gameplay change.
- Ensure wall occlusion/order is source-locked.

Gates:

- `test_dm1_v2_viewport_materials_pc34`
- `test_dm1_v2_lighting_pc34`
- `tools/verify_v2_viewport_asset_source_lock.py`
- New draw-order source gate.

### Phase 4 — HUD, champion panels and interaction surfaces

Purpose: make the V2 UI playable.

Source anchors:

- Existing V1 HUD/champion panel modules.
- ReDMCSB menu/action area and champion panel source.

Tasks:

- Scale HUD panels to modern resolution.
- Wire champion select, hand slots, action icons and inspect text.
- Keep V1 click zones as semantic base; V2 can redraw, not remap behavior silently.
- Add touch-friendly zones as a layer over source routes.

Gates:

- `test_dm1_v2_hud_interaction_pc34`
- `test_dm1_v2_touch_zones_pc34`
- Source gate for V1/V2 route equivalence.

### Phase 5 — Creatures, items, spells and effects

Purpose: finish the visible gameplay layer.

Source anchors:

- Existing V1 creature/render/spell/item modules.
- ReDMCSB group, projectile, explosion and spell source.
- `assets-v2/manifests/firestaff-v2-wave1-*.manifest.json`

Tasks:

- Connect creature family manifests to viewport renderer.
- Add item floor/hand rendering through V2 asset store.
- Add spell/effect overlays using V2 particles where source allows.
- Keep gameplay outcomes in V1-compatible core.

Gates:

- `test_dm1_v2_creature_render_pc34`
- `test_dm1_v2_item_render_pc34`
- `test_dm1_v2_spell_effect_pc34`
- Manifest validation for every used asset family.

### Phase 6 — Persistence, settings and packaging

Purpose: make V2 safe to ship.

Tasks:

- Add V2 settings: scale, smoothing, lighting, accessibility/touch toggle.
- Save V2 settings without breaking existing data-dir behavior.
- Include V2 smoke tests in release checklist.
- Package macOS preview from clean worktree only.

Gates:

- `test_dm1_v2_settings_pc34`
- package smoke: launcher → V2 → one movement → exit
- V1 regression gates still green.

## Execution order

1. Phase 0 completion matrix.
2. Phase 1 runtime shell.
3. Phase 2 movement/camera.
4. Phase 3 viewport/walls/lighting.
5. Phase 4 HUD/touch interaction.
6. Phase 5 creatures/items/spells/effects.
7. Phase 6 release hardening.

## First three commits to land

1. `plan: add DM1 V2 completion plan`
2. `test: add DM1 V2 completion matrix gate`
3. `feat: add DM1 V2 runtime shell smoke gate`

## Worker prompts rule

Every V2 worker prompt must include:

- work in N2 repo: `/home/trv2/work/firestaff`, not Mac worktrees,
- start with source audit and list files/lines,
- do not push,
- commit only after build/test,
- report exact commit hash and gate output,
- if no commit/test exists, report “evidence only, no landing”.
