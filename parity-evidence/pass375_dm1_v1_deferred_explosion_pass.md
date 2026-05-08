# pass375 DM1 V1 deferred explosion pass

Source lock: ReDMCSB `DUNVIEW.C` `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` draws projectiles inside the packed-cell loop, exits at `DUNVIEW.C:5915-5933`, then starts `/* Draw explosions */` and restarts `L0146_T_FirstThingToDraw` at `DUNVIEW.C:5916-5933` for an explosion-only pass.

Firestaff change: m11_draw_effect_cue() no longer draws explosions. Side-cell contents also defer explosions. `m11_draw_dm1_deferred_explosion_pass()` runs after all visible side and center object/creature/projectile contents have drawn, then calls the shared explosion cue/bitmap path for center and side cells.

This is the safe after-all-packed-cells explosion pass for current DM1 V1 movement/viewport/wall rendering. It preserves the existing source-backed explosion bitmap code while moving call placement to the ReDMCSB F0115 layer boundary.
