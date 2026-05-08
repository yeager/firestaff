# pass374 DM1 V1 F0115 explosion-pass blocker evidence

## ReDMCSB anchors

- `DUNVIEW.C:4547-4582` defines `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` and its packed-cell contract: objects are drawn for each ordered cell, creatures are drawn for the current cell, projectiles are drawn at the specified cell, then explosions run in a separate pass.
- `DUNVIEW.C:4819-4850` starts the per-cell object scan and only records projectile/explosion presence while scanning the square list.
- `DUNVIEW.C:5645-5683` is the projectile pass for the current processed cell; it restarts from `L0146_T_FirstThingToDraw` and draws only `C14_THING_TYPE_PROJECTILE` matching the current cell.
- `DUNVIEW.C:5915-5933` closes the packed-cell loop, starts `/* Draw explosions */`, then restarts the thing list again to draw `C15_THING_TYPE_EXPLOSION` after all ordered cells are complete.

## Local M11 state

- `m11_game_view.c:m11_draw_wall_contents()` calls `m11_draw_effect_cue()` as the top layer of a single open cell.
- `m11_game_view.c:m11_draw_effect_cue()` draws `cell->summary.projectiles > 0` and then `cell->summary.explosions > 0` in the same per-cell helper.
- This preserves projectile-before-explosion order inside one cell, but it still coalesces projectiles and explosions per open cell instead of deferring all explosions until after every packed view cell has had its object/creature/projectile passes.

## Result

Implementation is not safe in this pass: moving explosions out of `m11_draw_effect_cue()` needs a real deferred visible-cell schedule so D0/D1/D2/D3 ordering, side lanes, door passes, and fallback/headless probes all agree. The remaining blocker is now source-locked: replace the coalesced effect cue with an after-all-packed-cells explosion pass matching `DUNVIEW.C:5915-5933`.
