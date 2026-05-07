# DM1 V1 movement + viewport merge plan — N2 2026-05-06

Scope: N2 only (`trv2@<n2-private-ip>`, `<firestaff-repo>`). This plan starts from worker head `752a3fa` on `worker/n2-dm1v1-evidence-capture-20260505` and uses ReDMCSB as the source oracle under `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

## Source locks re-audited

Movement command chain:

- `COMMAND.C:2095-2127` reads the queued command/x/y, applies disabled-movement rejection, unlocks the queue, and processes pending clicks.
- `COMMAND.C:2150-2156` dispatches turns to `F0365_COMMAND_ProcessTypes1To2_TurnParty()` and step/strafe commands to `F0366_COMMAND_ProcessTypes3To6_MoveParty()`.
- `CLIKMENU.C:156-173` is the turn path: stop waiting, highlight turn zone, stairs special case, sensor remove/add, and `F0284_CHAMPION_SetPartyDirection()`.
- `CLIKMENU.C:224-233` maps movement arrows to forward/right deltas: forward `(1,0)`, right `(0,1)`, backward `(-1,0)`, left `(0,-1)`.
- `CLIKMENU.C:264-329` resolves current/target square, stairs, wall/door/fakewall/group blocking, blocked-input discard, and successful `F0267_MOVE_GetMoveResult_CPSCE()` calls.
- `CLIKMENU.C:330-346` derives movement cooldown from living champions and stores `G0310_i_DisabledMovementTicks`, clearing `G0311_i_ProjectileDisabledMovementTicks`.
- `DUNGEON.C:35-44` defines direction-to-axis deltas; `DUNGEON.C:1371-1391` applies forward then simulated-right movement to map coordinates.
- `MOVESENS.C:438-444`, `493-518`, and `573-578` commit party coordinates for normal moves, teleporter chains/rotation, and pit level changes; `MOVESENS.C:556-558` redraws during eligible pit-fall traversal.
- `GAMELOOP.C:150-155` ticks down movement/projectile movement locks; `GAMELOOP.C:164-168` reads keyboard input; `GAMELOOP.C:215-219` processes command queue until input/time state is settled.

Viewport draw/present chain:

- `GAMELOOP.C:83-91` redraws `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)` when not resting/in inventory.
- `DUNVIEW.C:8318-8350` enters `F0128_DUNGEONVIEW_Draw_CPSF`, clears clickable state, and uses supplied direction/map coordinate.
- `DUNVIEW.C:8357-8439` selects flipped/native floor, ceiling, and wall-set assets from `(mapX + mapY + direction) & 1`.
- `DUNVIEW.C:8468-8542` draws far-to-near viewport squares from D4 through D0 using `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement()` for each relative square.
- `DUNVIEW.C:8543-8579` restores native wall/floor/ceiling asset routing after flipped draws.
- `DUNVIEW.C:8604-8616` presents via `F0097_DUNGEONVIEW_DrawViewport()` and pre-draws floor/ceiling for the next call.
- `DRAWVIEW.C:849-858` resolves `C007_ZONE_VIEWPORT` and blits `G0296_puc_Bitmap_Viewport` through the video driver.
- `DUNVIEW.C:7391-7460` (`F0122_DUNGEONVIEW_DrawSquareD1L`) and `DUNVIEW.C:7559-7628` (`F0123_DUNGEONVIEW_DrawSquareD1R`) draw an opaque near side wall, process side ornament, and `return` before the open-cell content branch; this is the side-lane occlusion rule.
- `DUNVIEW.C:7493-7536` and `7661-7704` cover D1 side door/front/open branches where floor ornaments, door pass ordering, ceiling pits, objects/creatures/projectiles/explosions, and teleporter fields may continue.

## Current Firestaff owner files

Movement core:

- `dm1_v1_movement_command_core_pc34_compat.c/.h` — queue-level accepted/rejected command semantics, turn/step command result, movement-arrow deltas, collision result, timing payload.
- `dm1_v1_movement_pipeline_pc34_compat.c/.h` — command dispatch plus post-step resolver/timing state bridge.
- `dm1_v1_movement_timing_pc34_compat.c/.h` — `G0310`/`G0311` style movement lock and per-tick decrement behavior.
- `dm1_v1_movement_pc34_compat.c/.h` — lower-level movement compatibility helpers still used by older tests.
- Tests: `test_dm1_v1_movement_core_pc34_compat.c`, `test_dm1_v1_movement_command_core_pc34_compat.c`, `test_dm1_v1_movement_pipeline_pc34_compat.c`, `test_dm1_v1_movement_timing_pc34_compat.c`, `test_dm1_v1_command_movement_sensor_timing_pc34_compat.c`.

Viewport/world core:

- `dm1_v1_viewport_3d_pc34_compat.c/.h` — F0128 draw order, PC34 wall/door/floor/ceiling zones, side-lane occlusion source evidence, wall-set parity flip/restore.
- `dm1_v1_creature_viewport_pc34_compat.c/.h` — viewport creature/object/projectile/explosion placement layer.
- `dm1_v1_viewport_floor_ceiling_items_pc34_compat.c/.h`, `dm1_v1_wall_ornament_pc34_compat.c/.h`, `dm1_v1_viewport_click_pc34_compat.c/.h` — supporting viewport geometry/interaction strata.
- Test/gate: `test_dm1_v1_viewport_3d_pc34_compat.c`, `tools/verify_v1_viewport_side_wall_occlusion_gate.py`.

Runtime evidence:

- `tools/pass210_dm1_v1_original_runtime_binding_guard.py` and `parity-evidence/pass210_dm1_v1_original_runtime_binding_guard.md` — guard the original-runtime binding claim. They intentionally classify as blocked until a decompressed FIRES runtime base/map/handoff exists.

## Exact remaining blockers

1. **Runtime binding blocker, hard stop for stock-original debugger claims:** N2 does not have a decompressed stock `FIRES` runtime image base, post-LZEXE transfer CS:IP, or TLINK `FIRES.MAP`. Only compressed loader evidence is present, so no source seam can be bound to stock runtime CS:IP yet. Use `pass210` as the guard; do not promote compressed offsets.
2. **Original route/control blocker:** the finish plan still requires rejecting the static no-party route/hash and capturing a real party-control route before pixel overlay parity claims. Movement/viewport source gates are useful, but not sufficient for original pixel parity.
3. **Movement resolver completeness:** Firestaff has source-locked command/timing/pipeline slices, but the full `MOVESENS.C:F0267` matrix is not yet one landed resolver: projectile impacts, teleporter scope/rotation, pit chain map changes, audible side effects, sensor remove/add, group-on-party-map behavior, and draw-while-falling must be integrated behind one source-shaped API before declaring 100% movement parity.
4. **Viewport full-world completeness:** F0128 draw order and side-wall occlusion are guarded, but the final world overlay still needs original-route frames for walls/floor/ceiling, door states/buttons/ornaments, wall/floor ornaments, items, creatures, projectiles, explosions, fields, and dynamic clipping/palettes.
5. **Merge hygiene:** current worker has pass210/runtime-evidence changes and a generated build directory; keep generated build output out of commits and land evidence/docs separately from behavior patches.

## Safe merge order

1. **Land runtime guard and plan only.** Commit `pass210` + this plan first. Gate: `python3 tools/pass210_dm1_v1_original_runtime_binding_guard.py`, `ctest -R dm1_v1_original_runtime_binding_guard`, `git diff --check`. This creates the hard guardrail without changing gameplay.
2. **Movement command/timing stabilization.** Land only `dm1_v1_movement_command_core_*`, `dm1_v1_movement_timing_*`, and their tests. Gate: `ctest -R 'dm1_v1_(movement_command_core|movement_timing|input_command_queue|command_movement_sensor_timing)'` plus the standalone movement command/timing binaries if needed.
3. **Movement resolver integration.** Add a single source-shaped `F0267` bridge for normal move, blocked move, stairs, teleporter, pit, and sensor edges. Gate: `ctest -R 'dm1_v1_(movement_core|movement_pipeline|sensor_trigger)'` and an evidence manifest mapping each branch to `CLIKMENU.C`/`MOVESENS.C` lines.
4. **Viewport draw-order/wall-set/occlusion integration.** Land F0128 draw-order and wall side occlusion before creature/item layers. Gate: `ctest -R dm1_v1_viewport_3d_pc34_compat`, `python3 tools/verify_v1_viewport_side_wall_occlusion_gate.py`.
5. **Viewport content layers.** Merge floor/ceiling/items, wall ornaments, door ornaments/buttons/states, then creature/projectile/explosion layers in that order. Gate each with a source evidence row and `ctest -R 'dm1_v1_(viewport_3d|creature_render|projectile_explosion_render)'`.
6. **Original-route evidence unlock.** Only after runtime/route blockers are resolved, capture original party-control frames and run overlay gates. Until then, label pixel claims as source-lock/probe parity, not original runtime pixel parity.
7. **Final consolidation.** Update `docs/parity/PARITY_MATRIX_DM1_V1.md`, `docs/parity/V1_BLOCKERS.md`, and finish-plan status after all gates pass; keep behavior commits separate from evidence-only commits.

## Required gates before declaring this lane green

Minimum local gates:

```sh
python3 tools/pass210_dm1_v1_original_runtime_binding_guard.py
cmake -S . -B build-pass210-check -DBUILD_TESTING=ON
ctest --test-dir build-pass210-check -R dm1_v1_original_runtime_binding_guard --output-on-failure
cmake --build build --target \
  test_dm1_v1_movement_command_core_pc34_compat \
  test_dm1_v1_movement_core_pc34_compat \
  test_dm1_v1_movement_pipeline_pc34_compat \
  test_dm1_v1_movement_timing_pc34_compat \
  test_dm1_v1_viewport_3d_pc34_compat
./build/test_dm1_v1_movement_command_core_pc34_compat
./build/test_dm1_v1_movement_core_pc34_compat
./build/test_dm1_v1_movement_pipeline_pc34_compat
./build/test_dm1_v1_movement_timing_pc34_compat
./build/test_dm1_v1_viewport_3d_pc34_compat
python3 tools/verify_v1_viewport_side_wall_occlusion_gate.py
git diff --check
```

Merge claim allowed after these gates: source-locked movement command/timing/draw-order/side-occlusion plan and runtime-binding guard are landed. Claim not allowed yet: stock-original runtime CS:IP binding or pixel-perfect original viewport parity.
