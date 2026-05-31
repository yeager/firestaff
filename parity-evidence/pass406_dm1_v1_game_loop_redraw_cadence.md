# Pass406 — DM1 V1 game-loop redraw/cadence gate

Status: `PASS406_DM1_V1_GAME_LOOP_REDRAW_CADENCE_SOURCE_LOCKED`

## ReDMCSB-first source audit
- `GAMELOOP.C:35-219` / `F0002_MAIN_GameLoop_CPSDF` — sets the V1 wait budget, redraws the dungeon view from current party state, advances game time, decrements movement/projectile cooldowns, then processes queued commands in the input wait loop.
- `COMMAND.C:2045-2829` / `F0380_COMMAND_ProcessQueue_CPSC` — locks the queue, gates movement on `G0310_i_DisabledMovementTicks` / `G0311_i_ProjectileDisabledMovementTicks`, dequeues, unlocks, and dispatches turn/move commands.
- `DUNVIEW.C:8318-8618` / `F0128_DUNGEONVIEW_Draw_CPSF` — draws the viewport from supplied direction/map coordinates and ends by requesting a viewport draw.
- `DRAWVIEW.C:709-722` / `F0097_DUNGEONVIEW_DrawViewport` — sets `G0324_B_DrawViewportRequested` and waits for vblank.

## Firestaff executable guard
- `m11_game_view.c:6477-6649` / `m11_apply_dm1_v1_pipeline_tick` — enqueues the route command, ages old cooldowns before processing, processes one pipeline tick, publishes game tick/hash, and returns redraw/dequeue state.
- `dm1_v1_movement_pipeline_pc34_compat.c:244-443` / `DM1_V1_MovementPipeline_ProcessOneTickPc34Compat` — applies command/movement/post-move/timing before publishing `viewportDirty` provenance.
- `main_loop_m11.c:2305-2317` — records whether input redraw followed a viewport-dirty pipeline result before calling `M11_GameView_Draw`.

## Gates run
- `build/test_m11_v1_turning_presentation_pc34_compat`
- `build/test_dm1_v1_movement_pipeline_pc34_compat`
- `git diff --check` (also run inside this verifier)

## Scope guard
- This is a source-locked executable verifier/probe. It does not claim original DOSBox/FIRES.EXE breakpoint parity or pixel-perfect viewport parity.

Manifest: `parity-evidence/verification/pass406_dm1_v1_game_loop_redraw_cadence/manifest.json`
