# pass363 DM1 V1 F0115 thing-layer source lock / blocker narrowing

Status: `PASS_DM1_V1_F0115_THING_LAYER_SOURCE_LOCK_BLOCKER_NARROWED`

Scope: movement/viewport/walls only. This pass does not change renderer behavior and makes no pixel-parity claim.

## ReDMCSB anchors

Primary source root: `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

- `DUNVIEW.C:4547-4582` — `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`; source comment gives the per-cell loop: objects, one creature, projectiles, then explosions after cell processing.
- `DUNVIEW.C:4819-4860` — object pass begins with `/* Draw objects */`, defers group/projectile/explosion things, and draws visible objects for the current view cell.
- `DUNVIEW.C:5195-5202` — D4 guard then `/* Draw creatures */`; creature draw follows object processing for the current cell.
- `DUNVIEW.C:5679-5683` — projectile pass restarts the square thing list and draws only projectile objects for the current cell.
- `DUNVIEW.C:5915-5933` — after packed view cells are exhausted, `/* Draw explosions */` restarts the thing list and draws explosion things.

## Firestaff evidence

- `dm1_v1_viewport_3d_pc34_compat.c` exposes `s_thing_layers[]` with four source-locked F0115 phases: objects, creatures, projectiles, explosions.
- `dm1_v1_viewport_3d_pc34_compat.h` records the contract: object/creature/projectile phases repeat per packed view cell; explosion phase runs after all cells.
- `m11_game_view.c:m11_draw_wall_contents` still draws `m11_draw_effect_cue()` as the local top content layer for a single open cell.

## Blocker narrowed

The source-locked metadata and tests now distinguish ReDMCSB F0115’s two scopes:

1. **per packed view cell:** objects → creatures → projectiles
2. **after all packed cells:** explosions / fluxcage

That means the remaining pixel-parity risk is not the wall draw order table itself; it is the M11 local open-cell effect path coalescing projectiles and explosions inside `m11_draw_effect_cue()` instead of scheduling an after-all-cells explosion pass matching `DUNVIEW.C:5915-5933`.
