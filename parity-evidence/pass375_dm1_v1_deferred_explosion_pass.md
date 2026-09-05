# pass375 DM1 V1 deferred explosion pass

Source lock: ReDMCSB `DUNVIEW.C` `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` draws projectiles inside the packed-cell loop, exits at `DUNVIEW.C:5915-5933`, then starts `/* Draw explosions */` and restarts `L0146_T_FirstThingToDraw` at `DUNVIEW.C:5916-5933` for an explosion-only pass.

Firestaff change: m11_draw_effect_cue() no longer draws explosions. Each
scheduler-owned F0115 callback now finishes its packed-cell object, creature,
and projectile work and then restarts the square's C15 list. Door squares do
this in both source partitions around F0111. The former once-per-frame global
replay is no longer called. Normal explosions use the authenticated PC 3.4
item-696 C3014/C3031 anchors and real GRAPHICS.DAT material.

This is the safe after-all-packed-cells explosion pass within each F0115
transaction. It preserves the source-backed explosion bitmap code while
matching the ReDMCSB per-call layer boundary.
