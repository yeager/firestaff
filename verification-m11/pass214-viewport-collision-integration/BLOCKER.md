# Viewport/collision-specific blockers

1. **Viewport renderer is structural, not dungeon-data complete.**
   `dm1_viewport_3d_draw_frame()` records party position/direction, parity flip, floor/ceiling dirty state, and present cadence, but the source comment still says actual square-type queries require integration with dungeon data. This blocks visual parity for wall/door/pit/teleporter collision outcomes.

2. **Blocked movement has no viewport invalidation hook.**
   Current pipeline returns `viewportDirty=0` for closed-door and group-blocked movement. That is plausible for unchanged party pose, but any future bump animation, sound-timed door feedback, or viewport shake must be wired explicitly rather than inferred from movement state.

3. **Group collision is resolved in the movement core, not the collision query.**
   `DM1_V1_Collision_CheckStep()` records wall/door/fake-wall/bounds, but group blocking is currently only surfaced by `result.core.blockedByGroup`. A single collision API cannot yet explain every pipeline block reason.

4. **Cooldown/cadence remains synthetic in this probe.**
   Accepted steps set disabled movement ticks, so the probe drains cooldown between route commands and records `cooldown_drained`. A true frame-by-frame entrance/runtime capture still needs Pass211 movement-capture blocker removed before this can be compared against live movement frames.

5. **No GRAPHICS.DAT-backed draw validation here.**
   The probe intentionally writes TSV/JSON only and creates no PNG/PPM files. It proves state sync and draw/present decisions, not pixel parity.
