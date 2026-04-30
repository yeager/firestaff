# Pass177 viewport ReDMCSB draw-stack gate

Narrow source gate for the N2 viewport/world visuals lane.

This pass source-locks Firestaff's DM1 V1 open-cell viewport stack to ReDMCSB's
`F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` contract:
objects are drawn during the first per-cell pass while creatures, projectiles,
and explosions are deferred into later passes; Firestaff keeps floor ornaments,
floor items, creatures, and projectile/effect cues in that visible order after
front wall/door ornaments.

It is a source-shape verifier, not a pixel parity claim.
