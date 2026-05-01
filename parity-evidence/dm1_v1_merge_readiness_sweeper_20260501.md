# DM1 V1 merge-readiness sweeper — N2 — 2026-05-01

Host: N2 / `Firestaff-Worker-VM` (`/home/trv2/work/firestaff`).  Main integration branch inspected: `sync/n2-dm1-v1-20260428` at `35827f9` (`Lock DM1 viewport side-lane occlusion`), 70 commits ahead of `origin/main`.

## ReDMCSB source audit first

Source root audited: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

Verifier-relevant anchors:

- `DUNVIEW.C:371-377` defines the PC view-square mapping tables (`G2026_ac_ViewSquareIndexToViewLane`, `G2027_ac_ViewSquareIndexToViewDepth`, `G2035_ac_ViewSquareIndexToFieldAspectIndex`) used by viewport lane/depth/field gates.
- `DUNVIEW.C:436-465` defines wall/door/stair/pit frame tables including `G0163_aauc_Graphic558_Frame_Walls`, door-frame frames, and `G0188_aauc_Graphic558_FieldAspects`.
- `DUNVIEW.C:5732-5854` handles projectile aspect direction, flips, scale, and left-edge crop math before blitting projectiles/explosions; this is the source basis for projectile viewport clipping/cadence gates.
- `DUNVIEW.C:7604-7705` draws `D1R`: wall case returns after wall/ornament draw; door-front draws floor ornament, objects/projectiles pass 1, door frame/door, then objects/projectiles pass 2; pit/corridor path draws floor ornament, ceiling pit, then dynamic contents. This is the core D1 side-lane occlusion/order pattern.
- `DUNVIEW.C:8318-8542` is `F0128_DUNGEONVIEW_Draw_CPSF`: clears click boxes, sets flipped walls, draws D3 side blockers, D4 objects, then D3L/D3R/D3C, D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C in source order.
- `MOVESENS.C:194-315` documents projectile-impact checks when party/groups move, including intermediary step checks and destination-square projectile checks (`F0266_MOVE_IsKilledByProjectileImpact`).
- `MOVESENS.C:316-565` is `F0267_MOVE_GetMoveResult_CPSCE`: party/group/projectile movement, projectile pre-impact check (`432-437`), teleporter chain (`469-535`), pit/fall redraw and level transition (`538-565`).
- `COMMAND.C:2095-2156` drains the command queue, defers movement while `G0310_i_DisabledMovementTicks` or matching projectile movement cooldown is active, unlocks queue/pending click, then dispatches turns to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and moves to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKVIEW.C:348-496` normalizes dungeon-view click coordinates by viewport origin, routes door-button/wall-ornament, leader-hand throw/grab/drop, front-wall sensor/knock, alcove drops, and redraws changed object icons.
- `GROUP.C:22-25` exposes movement blocker state flags for walls/stairs/pits/fakewalls/fluxcages/teleporters, groups, doors, and party; `GROUP.C:769-930` (`F0190_GROUP_GetDamageCreatureOutcome`) updates group cell/direction packing and active-group data on death during movement/fall/projectile interaction.

## Current branch state

`git status --short` on `/home/trv2/work/firestaff`: clean before this report.

Ahead commits include the large DM1 V1 lane, not fine-tuning, already integrated into `sync/n2-dm1-v1-20260428`:

- Movement/input/timing: `2ff73dd`, `f675d4a`, `142d85e`, `e8633b0`, `d766d79`, `62e19d7`, `3db3b0d`, `1e7aaef`, `07fc711`.
- Viewport/draw-stack/walls/projectiles: `1fba19c`, `d7d5c08`, `1db7d27`, `35297e3`, `646fa63`, `5447407`, `2b02a04`, `fc98c93`, `119c83d`, `1d00ddd`, `1fb05d0`, `d7697e6`, `0e27489`, `3db94d6`, `059dc16`, `669b51f`, `ab76d78`, `35827f9`.
- Evidence/probe infrastructure: `bea0256`, `6e51e49`, `8cbe1f7`, `424bc09`, `8960853`, `748d73c`, `c4003b0`, plus the pass175/pass179 evidence commits.

## Worktrees/branches inspected

All listed worktrees matching DM1/V1/move/view/wall/door/projectile/occlusion were clean. The assignment-mentioned worktrees were clean:

- `worker/dm1-v1-door-state-redraw-probe-1777588821` `b8033c5`
- `worker/dm1-v1-front-wall-door-depth-20260501-0116` `065e65f`
- `worker/dm1-v1-front-wall-orn-vis-20260501-0354` `ac5964d`
- `worker/dm1-v1-input-command-queue-20260430-2225` `82c55ea`
- `worker/dm1-v1-move-collision-timing-20260430-201504` `e37166b`
- `worker/dm1-v1-object-source-row-clipping-20260430-211020` `6acaa54`
- `worker/dm1-v1-projectile-wall-occlusion-20260501-0216` `4d607db`
- `worker/dm1-v1-wall-ornament-motion-occ-20260501-0327` `ee7ff17`

Additional high-signal unmerged branches ahead of `HEAD`:

- `worker/dm1-v1-projectile-viewport-1777629142` `7bf4e6f` — merge-tree clean, but would delete current movement timing/pass179 evidence files if merged naively; cherry-pick/surgical merge only.
- `worker/n2-dm1v1-movement-core-source-20260501-104724` `8035e12` — merge-tree clean, but also deletes pass179 artifacts and current timing command test; cherry-pick the movement-wall original route fixture only if still needed.
- `worker/dm1-v1-side-content-zorder-20260501-1241` `2f8b837` — merge-tree clean, modifies `m11_game_view.c`, `m11_game_view.h`, probe, CMake; likely best next large-lane merge after fixing current occlusion gates.
- `n2-dm1v1-movement-runtime-heavy-20260501083047` `e5ba7a3` — merge-tree clean but broad non-DM1 churn (`release.yml`, README, M12, packaging deletions). Do not merge in DM1 viewport lane without isolating runtime probe commits.
- `dm1-v1-wall-blocker-occlusion-20260501-044412` `611432b` — merge-tree clean but broad legacy branch with many deletions and unrelated M12/release changes; superseded by current side-lane/center-door gates.
- `worker/original-overlay-capture-unblock-20260430-1156` `3d2463c` — merge-tree clean but very broad; only harvest pass177 source-gate pieces if needed.
- `worker/dm1-v1-front-wall-orn-vis-20260501-0354` `ac5964d` — merge-tree reports conflict in `CMakeLists.txt`; not first in merge order.

## Gates run

Build:

- `cmake --build build -j2` passed (`BUILD_EXIT:0`), log: `/tmp/firestaff_build_20260501_sweeper.log`.

Focused DM1/V1 movement/viewport/wall gate run:

- Command: `ctest --test-dir build --output-on-failure -R "(dm1_v1|v1_viewport|m11_game_view|m11_viewport_state|m11_turn_viewport_orientation|touch_movement)"`
- Result: 27/29 passed; failing:
  - `v1_viewport_side_wall_occlusion_gate`: `FAIL: missing function body for m11_dm1_side_lane_wall_clear_before_depth`
  - `v1_viewport_center_door_occlusion_gate`: `FAIL: Firestaff D3R door-button side-lane occlusion: missing '!m11_dm1_side_lane_wall_clear_for_rel(cells, 3, 1)'`
- Log: `/tmp/firestaff_ctest_dm1v1_sweeper.log`.

Full CTest:

- Result: 41/47 passed; failing:
  - `v1_viewport_side_wall_occlusion_gate`
  - `v1_viewport_center_door_occlusion_gate`
  - `v1_inventory_panel_open_redmcsb_gate`
  - `v1_inventory_toggle_redmcsb_gate`
  - `v1_inventory_chest_actionhand_redmcsb_gate`
  - `v1_status_refresh_order_redmcsb_gate`
- The DM1 movement/source-lock gates passed: `dm1_v1_movement_source_lock`, `dm1_v1_entry_movement_viewport_source_lock`, `dm1_v1_stairs_pits_viewport_source_lock`, `touch_movement_viewport_source_lock`, `dm1_v1_input_command_queue_source_lock`, `dm1_v1_movement_timing_source_lock`, `dm1_v1_projectile_movement_interlock_source_lock`.
- Full log: `/tmp/firestaff_ctest_all_sweeper.log`.

## Recommended merge order

1. Keep `sync/n2-dm1-v1-20260428` as integration base; do **not** merge broad runtime/release/M12 branches into this lane.
2. Retire current viewport blockers first on the integration branch:
   - implement/restore `m11_dm1_side_lane_wall_clear_before_depth`,
   - add the `!m11_dm1_side_lane_wall_clear_for_rel(cells, 3, 1)` D3R center-door/button occlusion guard,
   - rerun focused DM1/V1 gate set.
3. After those two pass, merge or cherry-pick `worker/dm1-v1-side-content-zorder-20260501-1241` (`2f8b837`) because it is the next coherent viewport draw-order lane and merge-tree is clean.
4. Then harvest projectile viewport work from `worker/dm1-v1-projectile-viewport-1777629142` (`7bf4e6f`) surgically; do not accept its deletions of current movement timing/pass179 files.
5. Defer `n2-dm1v1-movement-runtime-heavy-20260501083047`, `dm1-v1-wall-blocker-occlusion-20260501-044412`, and `worker/original-overlay-capture-unblock-20260430-1156` until DM1 lane blockers are green; they are too broad for big-lane merge without unrelated churn review.
6. `worker/dm1-v1-front-wall-orn-vis-20260501-0354` has a `CMakeLists.txt` conflict against current integration; merge only after the occlusion gate fixes, with manual CMake reconciliation.

## Remaining blockers

- Two active DM1 viewport occlusion blockers prevent merge-ready green:
  - missing `m11_dm1_side_lane_wall_clear_before_depth` body,
  - missing D3R `m11_dm1_side_lane_wall_clear_for_rel(cells, 3, 1)` guard for center-door/button occlusion.
- Four inventory/status gates are still red but outside the requested movement/viewport/walls sweep; they should not block the DM1 wall lane unless Daniel wants full-suite green before merge.
- No missing tools/libs observed in the build/probe gates. The only exact failures were verifier assertions above.
