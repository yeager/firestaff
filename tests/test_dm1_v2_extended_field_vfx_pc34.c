/*
 * test_dm1_v2_extended_field_vfx_pc34 — DM1 V2 Phase 4 extended field VFX.
 *
 * Tests the extended field VFX layer (dm1_v2_extended_field_vfx_pc34)
 * that covers dungeon element types beyond fireball/lightning/poison:
 *   - Teleporters
 *   - Pits
 *   - Stairs (front/side)
 *   - Fake walls
 *   - Floor ornaments
 *   - Unknown field fallback
 *
 * Source-lock anchors:
 * - ReDMCSB DEFS.H:922-941 M034_SQUARE_TYPE defines raw square type fields.
 * - ReDMCSB DUNGEON.C:2199-2250 element-type mapping.
 * - ReDMCSB DUNVIEW.C:6816-6831 field draw order in viewport composition.
 *
 * Source: Firestaff DM1 V2 Phase 4 followup.
 */

#include "dm1_v2_extended_field_vfx_pc34.h"
#include "dm1_v2_particle_system_pc34.h"
#include "dm1_v2_particle_emitter_presets_pc34.h"

#include <stdint.h>
#include <stdio.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

int main(void) {
    int emitter;

    /*
     * ReDMCSB anchors:
     * DEFS.H:922-941 M034_SQUARE_TYPE, DUNGEON.C:2177-2250 element mapping,
     * DUNVIEW.C:6816-6831 field draw order.
     */

    /* Test emitter preset mapping for extended families */
    CHECK(dm1_v2_extended_vfx_family_to_emitter_preset(
        DM1_V2_EFFECT_FAMILY_TELEPORTER) >= 0);
    CHECK(dm1_v2_extended_vfx_family_to_emitter_preset(
        DM1_V2_EFFECT_FAMILY_PIT) >= 0);
    CHECK(dm1_v2_extended_vfx_family_to_emitter_preset(
        DM1_V2_EFFECT_FAMILY_STAIRS) >= 0);
    CHECK(dm1_v2_extended_vfx_family_to_emitter_preset(
        DM1_V2_EFFECT_FAMILY_FLOOR_ORNAMENT) >= 0);
    CHECK(dm1_v2_extended_vfx_family_to_emitter_preset(
        DM1_V2_EFFECT_FAMILY_FAKEWALL) >= 0);
    /* Unrecognised family returns -1 */
    CHECK(dm1_v2_extended_vfx_family_to_emitter_preset(
        (DM1_V2_ExtendedFieldEffectFamily)999) == -1);

    /* Initialise particle system so emitter creation succeeds */
    v2_particle_init();

    /* Teleporters trigger MAGIC_SPARKLE preset */
    emitter = dm1_v2_extended_vfx_trigger_field(4, 5,
        DM1_V2_EFFECT_FAMILY_TELEPORTER);
    CHECK(emitter >= 0);

    /* Pits trigger DUST_PUFF preset */
    emitter = dm1_v2_extended_vfx_trigger_field(8, 3,
        DM1_V2_EFFECT_FAMILY_PIT);
    CHECK(emitter >= 0);

    /* Stairs trigger DUST_PUFF preset */
    emitter = dm1_v2_extended_vfx_trigger_field(2, 7,
        DM1_V2_EFFECT_FAMILY_STAIRS);
    CHECK(emitter >= 0);

    /* Fake walls trigger MAGIC_SPARKLE preset */
    emitter = dm1_v2_extended_vfx_trigger_field(10, 1,
        DM1_V2_EFFECT_FAMILY_FAKEWALL);
    CHECK(emitter >= 0);

    /* Floor ornaments trigger DUST_PUFF preset */
    emitter = dm1_v2_extended_vfx_trigger_field(15, 15,
        DM1_V2_EFFECT_FAMILY_FLOOR_ORNAMENT);
    CHECK(emitter >= 0);

    /* Unknown family returns -1 */
    CHECK(dm1_v2_extended_vfx_trigger_field(4, 5,
        (DM1_V2_ExtendedFieldEffectFamily)999) == -1);

    /* Deterministic unknown fallback always returns 0 (success) */
    CHECK(dm1_v2_extended_vfx_trigger_unknown_fallback(0, 0) == 0);
    CHECK(dm1_v2_extended_vfx_trigger_unknown_fallback(-1, -1) == 0);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_extended_field_vfx_pc34: ok");
    return 0;
}