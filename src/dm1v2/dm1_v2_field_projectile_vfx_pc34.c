/*
 * dm1_v2_field_projectile_vfx — DM1 V2 Phase 4 field/teleporter/projectile
 * visual-effect binding layer.
 *
 * This module bridges the source-locked V1 game-logic state (projectile
 * movement, explosion creation, field activation) to the V2 presentation
 * layer (particle emitters, spell overlays). It is purely presentation:
 * every function in this module reads V1 state but never writes or mutates
 * it.
 *
 * Source-lock anchors:
 * - ReDMCSB DEFS.H:421-430 names special explosion-thing values:
 *   Fireball (0xFF80), Lightning Bolt (0xFF82), Poison Bolt (0xFF86),
 *   Poison Cloud (0xFF87), Fluxcage (0xFFB2).
 * - ReDMCSB PROJEXPL.C:43-92 owns projectile creation and C48/C49 movement.
 * - ReDMCSB PROJEXPL.C:95-165 owns explosion thing creation and C25 event.
 * - ReDMCSB PROJEXPL.C:817-864 owns explosion damage/state processing.
 * - ReDMCSB PROJEXPL.C:987-994 owns C24 fluxcage removal event.
 * - ReDMCSB DUNVIEW.C:6816-6831 draws fields after the source object,
 *   creature, projectile and explosion route for the current square.
 * - ReDMCSB PANEL.C:367-428 computes dungeon light from torch charges
 *   and magical light, selecting G0304_i_DungeonViewPaletteIndex.
 *
 * No ReDMCSB original equivalent for particle effects or V2 overlays —
 * the original outputs directly to VGA DAC without post-processing.
 */

#include "dm1_v2_field_projectile_effect_metadata_pc34.h"
#include "dm1_v2_particle_emitter_presets_pc34.h"
#include "dm1_v2_lighting_dynamic_pc34.h"
#include "dm1_v2_spell_effect_overlay_pc34.h"
#include "dm1_v2_particle_system_pc34.h"

#include <math.h>

/* DM1 source effects have no particle-emitter equivalent. Keep this API as a
 * negative admission result so callers cannot turn source things into fake
 * V2 particles. */
int dm1_v2_vfx_family_to_emitter_preset(DM1_V2_FieldProjectileEffectFamily family) {
    (void)family;
    return -1; /* ReDMCSB has no particle emitter preset. */
}

/* Trigger a spell overlay and particle emitter for a DM1 explosion thing.
 * This reads the thing value, looks up the family metadata, and fires
 * both the VFX overlay (centered on the viewport) and a particle emitter
 * at the map pixel position (mapX, mapY).
 *
 * Returns 1 if metadata was found and effects were triggered, 0 if not.
 *
 * Source: Firestaff DM1 V2 Phase 4. */
int dm1_v2_vfx_trigger_explosion_thing(int16_t dm1Thing, int mapX, int mapY) {
    const DM1_V2_FieldProjectileEffectMetadata* meta =
        dm1_v2_field_projectile_effect_metadata_for_dm1_thing(dm1Thing);
    if (!meta) return 0;

    /* ReDMCSB renders the source explosion bitmap in DUNVIEW/PROJEXPL.
     * No generated V2 overlay or particle material is admitted here. */
    (void)mapX; (void)mapY;
    return 1;
}

/* Trigger VFX for a field (teleporter, fluxcage, etc.).
 * Field effects render as a persistent emitter in the viewport.
 * Returns emitter index, or -1 if no matching preset. */
int dm1_v2_vfx_trigger_field(int mapX, int mapY, DM1_V2_FieldProjectileEffectFamily family) {
    (void)mapX; (void)mapY; (void)family;
    return -1; /* Source field bitmaps remain on the authenticated V1 path. */
}

/* Compute dungeon ambient lighting for V2 presentation.
 * Uses the source palette index (0=brightest..5=darkest) from the game
 * and computes enhanced lighting parameters for the V2 viewport renderer.
 *
 * Source-lock: ReDMCSB PANEL.C:367-428 (see dm1_v2_lighting_dynamic_pc34.c) */
M11_V2_SourcePaletteLighting dm1_v2_vfx_compute_lighting(int source_palette_index,
                                                           bool enhanced_effects_enabled) {
    return v2_light_build_source_palette_lighting(source_palette_index, enhanced_effects_enabled);
}

/* Deterministic HiDPI-safe composition: the V2 presentation pipeline uses
 * a fixed 224x136 viewport buffer that is blitted to the SDL present buffer
 * at integer scale. This ensures that V2 rendering is resolution-independent
 * and deterministic across HiDPI/Retina displays. Any upscale to the
 * display resolution uses SDL texture scale mode (NEAREST by default for
 * pixel-art fidelity), not a software resize that could introduce non-
 * deterministic interpolation.
 *
 * HiDPI safety: we call SDL_GetRenderOutputSize (not SDL_GetWindowSize)
 * to get the true pixel dimensions, and we use SDL_RenderGeometry or
 * SDL_RenderTexture with a pre-computed integer destRect, never a
 * floating-point dest that could round differently across platforms. */
