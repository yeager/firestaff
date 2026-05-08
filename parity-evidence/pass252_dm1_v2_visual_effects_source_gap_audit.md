# Pass 252 — DM1 V2 visual/effects source-gap audit

Date: 2026-05-06
Worker: N2 / Firestaff-Worker-VM
Scope: DM1 V2 completion matrix, next largest verified gap

## Why this lane

`parity-evidence/verification/dm1_v2_completion_matrix.json` currently inventories 32 V2 modules. The largest category without a dedicated current CTest/source-lock owner is `visual/effects` with five modules:

- `dm1_v2_damage_numbers_pc34.c`
- `dm1_v2_particle_emitter_presets_pc34.c`
- `dm1_v2_particle_system_pc34.c`
- `dm1_v2_spell_effect_overlay_pc34.c`
- `dm1_v2_weather_fx_pc34.c`

The current matrix gate lists V2 gates for runtime shell, movement viewport, HUD overlay, lighting, viewport occlusion, and upscale validation, but no `dm1_v2_*effect*`, `dm1_v2_particle*`, `dm1_v2_damage*`, or `dm1_v2_weather*` gate. This makes the effects lane the next largest verified gap by module count.

This pass intentionally does not claim visual/effects parity. It turns the broad Phase 5 effects gap into source-anchored sub-gates.

## Local source references audited

Primary local reference root:

`<redmcsb-source>/Toolchains/Common/Source/`

Relevant ReDMCSB anchors:

1. `DUNVIEW.C:1303-1324` — projectile/explosion aspect tables.
   - `G0210_as_Graphic558_ProjectileAspects` defines bitmap-relative indices, dimensions, and `GraphicInfo` for arrows, thrown weapons, lightning, fireball, slime, poison dart, and poison-cloud/poison-bolt aspects.
   - `G0211_as_Graphic558_ExplosionAspects` defines the native explosion dimensions for fire, spell, poison, and death/smoke-style explosions.
2. `DUNVIEW.C:1541-1571` — distance scale tables.
   - `G0215_auc_Graphic558_ProjectileScales` and `G0216_auc_Graphic558_ExplosionBaseScales` define source distance scaling. V2 particles may be modernized, but their trigger positions and scale buckets must be derived from these source buckets or from an explicitly documented V2-only overlay layer.
3. `DUNVIEW.C:1871-1926` — explosion coordinates.
   - Center, split front-left/front-right, and rebirth explosion coordinate tables bind visible positions to DM1 view-square/cell depth.
4. `DUNVIEW.C:4476-4520` — `F0114_DUNGEONVIEW_GetExplosionBitmap`.
   - Native/derived explosion bitmap retrieval, scale clamp, and smoke palette-change handling are source-owned rendering behavior.
5. `DUNVIEW.C:4547-4582`, `4838-4850`, `5200-5208` — `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`.
   - Draw order is explicit: note projectiles/explosions while iterating objects, draw objects, draw creature(s), then projectiles, then explosions/fluxcage. V2 overlays must not reorder gameplay-visible projectile/explosion layers unless declared as V2-only decoration above the source layer.
6. `DUNGEON.C:1165-1224` — `F0142_DUNGEON_GetProjectileAspect`.
   - Explosion thing IDs map to negative projectile-aspect ordinals: fireball, slime, lightning bolt, poison bolt/cloud, or default explosion.
7. `TIMELINE.C:1033-1132`, `1865-1874` — launcher creation and event dispatch.
   - Wall projectile-launcher sensors create one or two projectile events with source cell/direction/energy rules; timeline dispatch routes events 48/49 to projectile processing and event 25 to explosion processing.
8. `GROUP.C:1706-1770` — creature projectile selection.
   - Creature attack behavior selects fireball, harm-non-material, lightning, poison cloud, open-door, or slime explosion things based on creature type/random choice, then calls `F0212_PROJECTILE_Create`.
9. `MOVESENS.C:113-160`, `195-302` — projectile movement interactions.
   - Teleporter rotation, levitation handling for projectiles/explosions, and intermediary cell impact checks are gameplay semantics. V2 visual effects must observe these semantics rather than inventing independent hit timing.

## Current Firestaff V2 effects state

The five V2 effects modules are currently presentation scaffolds, not source-locked gameplay/effect implementations:

- `dm1_v2_damage_numbers_pc34.c` renders floating numeric popups with local lifetime/velocity/digit-font policy. DM1 V1 has no source damage-number surface; this must stay a V2-only accessibility/presentation overlay and must be gated to prove it does not feed back into combat/projectile timing.
- `dm1_v2_particle_emitter_presets_pc34.c` uses invented preset names and rates (`SPELL_FIREBALL`, `SPELL_POISON`, `MAGIC_SPARKLE`, etc.). These can be decorative only until bound to source projectile/explosion aspect IDs and event triggers.
- `dm1_v2_particle_system_pc34.c` uses `rand()`-driven generic particles. This is not deterministic source behavior and cannot be a parity owner without a seeded/tested V2-only overlay contract.
- `dm1_v2_spell_effect_overlay_pc34.c` maps broad VFX names to full-frame color/intensity formulas. Several names (`HEAL_GLOW`, `FREEZE_FLASH`, `SHIELD_PULSE`, `DISPEL_WAVE`) are not direct DM1 projectile/explosion thing IDs and must not be treated as source effects.
- `dm1_v2_weather_fx_pc34.c` is environmental decoration (`FOG`, `DUST`, `DRIP`, etc.). DM1 dungeon view fog/lighting/distance treatment belongs with the source viewport/lighting lane unless this remains explicitly V2-only decoration.

## Required source-locked sub-gates before completion credit

1. **Aspect binding gate**
   - Add a V2 effect catalog that maps source thing/projectile/explosion IDs to V2 effect IDs.
   - Minimum source IDs: fireball, slime, lightning bolt, poison bolt/cloud, default explosion, smoke/death, rebirth step 1/2, and projectile object aspects.
   - Source anchors: `DUNGEON.C:1165-1224`, `DUNVIEW.C:1303-1324`.

2. **Distance/cell placement gate**
   - Test that V2 effect spawn positions and scale buckets derive from source view-square/cell tables, not normalized full-frame math alone.
   - Source anchors: `DUNVIEW.C:1541-1571`, `1871-1926`.

3. **Draw-order gate**
   - Test the source order: floor/objects first, creature pass, projectile pass, explosion/fluxcage pass. V2 particles may be an additional overlay only after source-owned layers have been preserved.
   - Source anchor: `DUNVIEW.C:4547-4582`, `4838-4850`, `5200-5208`.

4. **Timeline trigger gate**
   - Test that V2 effects are driven by projectile/explosion events created by existing source-compatible movement/timeline modules, not by independent visual timers.
   - Source anchors: `TIMELINE.C:1033-1132`, `1865-1874`, `GROUP.C:1706-1770`.

5. **No gameplay feedback gate**
   - Test that `damage_numbers`, decorative particles, and weather/fog overlays do not change movement, collision, projectile impact, sensor activation, combat outcome, save state, or V1 behavior.
   - Source anchors: `MOVESENS.C:113-160`, `195-302`.

## Completion impact

This pass narrows the DM1 V2 completion matrix from “Phase 5 creature/item/spell/effect render gates are absent” to a concrete, source-locked checklist for the largest ungated V2 category. It adds no runtime behavior and claims no visual parity. The next implementation pass should start with the aspect-binding gate and should add a dedicated `dm1_v2_effects_source_lock` CTest owner before granting any effects completion credit.
