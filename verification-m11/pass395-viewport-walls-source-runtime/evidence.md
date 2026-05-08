# Pass395 DM1 V1 viewport walls source/runtime lock

## Parity result
PASS — source/runtime lock now covers wall square draw order, occlusion, door two-pass ordering, F0115 object/creature/projectile/explosion handoff, and the post-command next-redraw path.

## ReDMCSB-first anchors
- `COMMAND.C:2045-2156` — `F0380_COMMAND_ProcessQueue_CPSC` pops eligible queued commands and dispatches turn/move mutations.
- `GAMELOOP.C:55-90` — the next main-loop iteration redraws `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)` from the post-command party tuple.
- `DUNVIEW.C:8318-8610` — far-to-near visible-square replay and final viewport present.
- `DUNVIEW.C:6421-7843` — wall case occlusion via wall draw + return, with front-alcove F0115 exception.
- `DUNVIEW.C:6440-6816` — door-front rear contents pass, door draw, front contents pass.
- `DUNVIEW.C:4547-5938` — F0115 object/creature/projectile per-cell passes and global explosion pass.
- `DRAWVIEW.C:709-722` — viewport present requests blit and waits for vertical blank.

## Firestaff locks added
- `dm1_v1_viewport_3d_pc34_compat.[ch]`: `DM1_ViewportPostCommandRedrawSpec` metadata and source evidence.
- `test_dm1_v1_viewport_3d_pc34_compat.c`: runtime assertions for the post-command redraw contract.
- `scripts/verify_pass395_dm1_v1_viewport_walls_source_runtime_lock.py`: ReDMCSB-first verifier/probe with optional runtime test execution.

## Gates run
- `python3 -m py_compile scripts/verify_pass395_dm1_v1_viewport_walls_source_runtime_lock.py`
- `scripts/verify_pass395_dm1_v1_viewport_walls_source_runtime_lock.py`
- `cmake -S . -B build -DBUILD_TESTING=ON`
- `cmake --build build --target test_dm1_v1_viewport_3d_pc34_compat -j2`
- `./build/test_dm1_v1_viewport_3d_pc34_compat`
- `scripts/verify_pass395_dm1_v1_viewport_walls_source_runtime_lock.py --run-runtime`
