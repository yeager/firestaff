#ifndef FIRESTAFF_DM1_V2_FIELD_PROJECTILE_VFX_PC34_H
#define FIRESTAFF_DM1_V2_FIELD_PROJECTILE_VFX_PC34_H

/*
 * dm1_v2_field_projectile_vfx — DM1 V2 Phase 4 field/teleporter/projectile
 * VFX binding layer.
 *
 * Source-lock anchors:
 * - ReDMCSB DEFS.H:421-430: Fireball (0xFF80), Lightning (0xFF82),
 *   Poison Bolt (0xFF86), Poison Cloud (0xFF87), Fluxcage (0xFFB2).
 * - ReDMCSB PROJEXPL.C:43-92: projectile creation/movement.
 * - ReDMCSB PROJEXPL.C:95-165: explosion thing creation/C25 event.
 * - ReDMCSB PROJEXPL.C:817-864: explosion damage processing.
 * - ReDMCSB PROJEXPL.C:987-994: C24 fluxcage removal.
 * - ReDMCSB DUNVIEW.C:6816-6831: field draw order.
 * - ReDMCSB PANEL.C:367-428: dungeon palette/lighting selection.
 *
 * No ReDMCSB original equivalent for particle effects or V2 overlays.
 * All functions are presentation-only and non-mutating.
 */

#include <stdint.h>

#include "dm1_v2_field_projectile_effect_metadata_pc34.h"
#include "dm1_v2_lighting_dynamic_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Map a DM1 V2 effect family to its V2 particle emitter preset.
 * Returns -1 when the family has no V2 presentation emitter.
 *
 * Source: Firestaff DM1 V2 Phase 4; family inputs are source-locked through
 * ReDMCSB DEFS.H:421-430 and PROJEXPL.C:95-166. */
int dm1_v2_vfx_family_to_emitter_preset(
    DM1_V2_FieldProjectileEffectFamily family);

/* Trigger VFX for a DM1 explosion thing at the given dungeon tile.
 * Fires both the spell overlay and particle emitter.
 * Returns 1 if the thing was recognised, 0 otherwise.
 *
 * Source: Firestaff DM1 V2 Phase 4. */
int dm1_v2_vfx_trigger_explosion_thing(int16_t dm1Thing, int mapX, int mapY);

/* Trigger VFX for a dungeon field (teleporter, fluxcage) at tile position.
 * Returns emitter index, or -1 if no matching preset.
 *
 * Source: Firestaff DM1 V2 Phase 4. */
int dm1_v2_vfx_trigger_field(int mapX, int mapY,
    DM1_V2_FieldProjectileEffectFamily family);

/* Compute V2 source-palette lighting from the source palette index
 * (0=brightest, 5=darkest). The enhanced_effects_enabled flag gates
 * V2-only additive light-map effects; source lighting is always mirrored.
 *
 * Source: ReDMCSB PANEL.C:367-428 via dm1_v2_lighting_dynamic_pc34.c */
M11_V2_SourcePaletteLighting dm1_v2_vfx_compute_lighting(
    int source_palette_index,
    bool enhanced_effects_enabled);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_FIELD_PROJECTILE_VFX_PC34_H */
