#include "dm1_v2_field_projectile_vfx_pc34.h"
#include "dm1_v2_particle_system_pc34.h"
#include "dm1_v2_spell_effect_overlay_pc34.h"

#include <stdint.h>
#include <stdio.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static void reset_vfx_state(void) {
    v2_spell_overlay_init();
    v2_particle_init();
}

static void check_explosion_trigger(int16_t thing, M11_V2_SpellVFX expected) {
    M11_V2_SpellOverlay snap;

    reset_vfx_state();
    CHECK(dm1_v2_vfx_trigger_explosion_thing(thing, 12, 7) == 1);

    snap = v2_spell_overlay_snapshot();
    CHECK(snap.active);
    CHECK(snap.type == expected);
    CHECK(snap.progress == 0.0f);
    CHECK(snap.speed == 1.0f);
}

int main(void) {
    M11_V2_SpellOverlay snap;
    M11_V2_SourcePaletteLighting lighting;
    int emitter;

    /*
     * ReDMCSB anchors for this V2 presentation-only binding:
     * DEFS.H:421-430 names the special explosion thing values, PROJEXPL.C:95-166
     * creates/stores explosion things, DUNVIEW.C:6816-6831 keeps field drawing
     * after source object/creature/projectile routes, and PANEL.C:367-428 owns
     * the canonical source palette selection that V2 mirrors here.
     */
    CHECK(dm1_v2_vfx_family_to_emitter_preset(DM1_V2_EFFECT_FAMILY_FIREBALL) >= 0);
    CHECK(dm1_v2_vfx_family_to_emitter_preset(DM1_V2_EFFECT_FAMILY_LIGHTNING) >= 0);
    CHECK(dm1_v2_vfx_family_to_emitter_preset(DM1_V2_EFFECT_FAMILY_POISON) >= 0);
    CHECK(dm1_v2_vfx_family_to_emitter_preset(DM1_V2_EFFECT_FAMILY_FLUXCAGE_FIELD) >= 0);
    CHECK(dm1_v2_vfx_family_to_emitter_preset((DM1_V2_FieldProjectileEffectFamily)99) == -1);

    check_explosion_trigger((int16_t)0xFF80, VFX_FIREBALL_BURST);
    check_explosion_trigger((int16_t)0xFF82, VFX_LIGHTNING_BOLT);
    check_explosion_trigger((int16_t)0xFF86, VFX_POISON_CLOUD);
    check_explosion_trigger((int16_t)0xFF87, VFX_POISON_CLOUD);

    reset_vfx_state();
    CHECK(dm1_v2_vfx_trigger_explosion_thing((int16_t)0x1234, 12, 7) == 0);
    snap = v2_spell_overlay_snapshot();
    CHECK(!snap.active);

    reset_vfx_state();
    CHECK(dm1_v2_vfx_trigger_explosion_thing((int16_t)0xFFB2, 4, 5) == 1);
    snap = v2_spell_overlay_snapshot();
    CHECK(!snap.active);

    reset_vfx_state();
    emitter = dm1_v2_vfx_trigger_field(4, 5, DM1_V2_EFFECT_FAMILY_FLUXCAGE_FIELD);
    CHECK(emitter >= 0);
    CHECK(dm1_v2_vfx_trigger_field(4, 5, (DM1_V2_FieldProjectileEffectFamily)99) == -1);

    lighting = dm1_v2_vfx_compute_lighting(0, true);
    CHECK(lighting.source_palette_index == 0);
    CHECK(lighting.enhanced_effects_enabled);
    CHECK(!lighting.deterministic_fallback);

    lighting = dm1_v2_vfx_compute_lighting(-1, true);
    CHECK(lighting.source_palette_index == 5);
    CHECK(!lighting.enhanced_effects_enabled);
    CHECK(lighting.deterministic_fallback);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_field_projectile_vfx_pc34: ok");
    return 0;
}
