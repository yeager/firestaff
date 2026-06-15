/*
 * dm1_v2_extended_field_vfx_pc34 — DM1 V2 Phase 4 extended field VFX.
 *
 * Extends the base field/projectile VFX layer (dm1_v2_field_projectile_vfx_pc34)
 * to cover dungeon element types beyond fireball/lightning/poison/fluxcage:
 *   - Pits (DM1_V2_ELEMENT_PIT)
 *   - Stairs (DM1_V2_ELEMENT_STAIRS_FRONT / SIDE)
 *   - Teleporters (DM1_V2_ELEMENT_TELEPORTER)
 *   - Fake walls (DM1_V2_ELEMENT_WALL where fakewall is open)
 *
 * These are visual-only effects that run alongside the base VFX binding.
 * The V1 source (ReDMCSB DUNGEON.C:2199-2210, DUNGEON.C:2238-2246) only
 * marks their presence in the square; no original particle/VFX equivalent
 * exists, so V2 is adding presentation-only effects here.
 *
 * Source-lock anchors (V1 element types):
 * - ReDMCSB DEFS.H:922-941 M034_SQUARE_TYPE defines raw square type fields.
 * - ReDMCSB DUNGEON.C:2199-2210 open fake walls become corridor.
 * - ReDMCSB DUNGEON.C:2238-2239 stairs raw type becomes side/front aspect.
 * - ReDMCSB DUNGEON.C:2246-2250 teleporter raw type (DEFS.H:927).
 * - ReDMCSB DUNGEON.C:2177-2185 pit raw type (DEFS.H:925).
 * - ReDMCSB DUNVIEW.C:6816-6831 field draw order in viewport composition.
 *
 * No ReDMCSB original equivalent for particle/VFX on these elements —
 * this module is V2 presentation-only enhancement.
 */

#ifndef FIRESTAFF_DM1_V2_EXTENDED_FIELD_VFX_PC34_H
#define FIRESTAFF_DM1_V2_EXTENDED_FIELD_VFX_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Extended field effect families for dungeon element types beyond
 * the standard projectile/explosion effects. These complete the V2
 * field VFX coverage for DUNVIEW.C:6816-6831. */
typedef enum {
    DM1_V2_EFFECT_FAMILY_TELEPORTER = 100,
    DM1_V2_EFFECT_FAMILY_PIT,
    DM1_V2_EFFECT_FAMILY_STAIRS,
    DM1_V2_EFFECT_FAMILY_FAKEWALL,
    DM1_V2_EFFECT_FAMILY_FLOOR_ORNAMENT,
    DM1_V2_EFFECT_FAMILY_UNKNOWN_FIELD
} DM1_V2_ExtendedFieldEffectFamily;

/* Map an extended field family to its V2 particle emitter preset.
 * Returns -1 when the family has no V2 presentation emitter.
 *
 * Source: Firestaff DM1 V2 Phase 4 followup. */
int dm1_v2_extended_vfx_family_to_emitter_preset(
    DM1_V2_ExtendedFieldEffectFamily family);

/* Trigger a VFX effect for an extended dungeon element at map position.
 * This is called by the viewport renderer when rendering D3C/D2C/D1C/D0C
 * squares that contain pits, stairs, teleporters, or fake walls.
 *
 * Returns emitter index on success, -1 if no preset matched.
 *
 * Source: Firestaff DM1 V2 Phase 4 followup. */
int dm1_v2_extended_vfx_trigger_field(
    int mapX,
    int mapY,
    DM1_V2_ExtendedFieldEffectFamily family);

/* Deterministic fallback for unknown field types.
 * Always succeeds (returns 0) so callers can ignore failure silently.
 *
 * Source: Firestaff DM1 V2 Phase 4 followup. */
int dm1_v2_extended_vfx_trigger_unknown_fallback(int mapX, int mapY);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_EXTENDED_FIELD_VFX_PC34_H */