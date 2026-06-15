#include "csb_p4_lighting_metadata.h"
#include "csb_v2_vfx_particles.h"
#include "csb_v2_lighting_dynamic.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ================================================================
 * CSB Phase 4 — Lighting Metadata & VFX Binding Tests
 *
 * Tests:
 * 1. Torch metadata: charge-to-type and type-to-intensity mappings
 * 2. Palette light-percent table completeness
 * 3. Spell projectile metadata lookup (all known spell ids)
 * 4. Spell category → VFX type mapping
 * 5. VFX binding gate decisions with various phase configs
 * 6. Projectile VFX fire / field VFX add / chaos trigger integration
 * 7. Binding tick expiry and active counts
 * ================================================================ */

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

#define CHECK_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        fprintf(stderr, "FAIL %s:%d: %s (%ld != %ld)\n", \
                __FILE__, __LINE__, msg, (long)(a), (long)(b)); \
        ++failures; \
    } \
} while (0)

/* ================================================================
 * Test 1: Torch metadata
 * ================================================================ */

static void test_torch_metadata(void) {
    /* Charge count 0 → no torch */
    CHECK_EQ(csb_p4_charge_count_to_torch_type(0),
             CSB_P4_TORCH_TYPE_NONE, "charge 0 → no torch");

    /* Charge counts 1-3 → normal torch */
    CHECK_EQ(csb_p4_charge_count_to_torch_type(1), CSB_P4_TORCH_TYPE_NORMAL, "charge 1 → normal");
    CHECK_EQ(csb_p4_charge_count_to_torch_type(3), CSB_P4_TORCH_TYPE_NORMAL, "charge 3 → normal");

    /* Charge counts 4-7 → bright torch */
    CHECK_EQ(csb_p4_charge_count_to_torch_type(4), CSB_P4_TORCH_TYPE_BRIGHT, "charge 4 → bright");
    CHECK_EQ(csb_p4_charge_count_to_torch_type(7), CSB_P4_TORCH_TYPE_BRIGHT, "charge 7 → bright");

    /* Charge counts 8-15 → magical torch */
    CHECK_EQ(csb_p4_charge_count_to_torch_type(8),  CSB_P4_TORCH_TYPE_MAGICAL, "charge 8 → magical");
    CHECK_EQ(csb_p4_charge_count_to_torch_type(15), CSB_P4_TORCH_TYPE_MAGICAL, "charge 15 → magical");

    /* Clamp at boundaries */
    CHECK_EQ(csb_p4_charge_count_to_torch_type(-1),  CSB_P4_TORCH_TYPE_NONE,  "charge -1 clamped");
    CHECK_EQ(csb_p4_charge_count_to_torch_type(100), CSB_P4_TORCH_TYPE_MAGICAL, "charge 100 clamped");

    /* Torch type → intensity */
    CHECK_EQ(csb_p4_torch_type_to_intensity(CSB_P4_TORCH_TYPE_NONE),    0,   "none intensity");
    CHECK_EQ(csb_p4_torch_type_to_intensity(CSB_P4_TORCH_TYPE_NORMAL),  140, "normal intensity");
    CHECK_EQ(csb_p4_torch_type_to_intensity(CSB_P4_TORCH_TYPE_BRIGHT),  200, "bright intensity");
    CHECK_EQ(csb_p4_torch_type_to_intensity(CSB_P4_TORCH_TYPE_MAGICAL), 255, "magical intensity");
    CHECK_EQ(csb_p4_torch_type_to_intensity(99), 0, "unknown type → 0");
}

/* ================================================================
 * Test 2: Palette light-percent table
 * ================================================================ */

static void test_palette_light_table(void) {
    /* Palette 0 (brightest dungeon depth) → 99% */
    CHECK_EQ(csb_p4_k_palette_index_to_light_percent[0], 99, "palette 0 = 99%");

    /* Palette 5 (darkest) → 0% */
    CHECK_EQ(csb_p4_k_palette_index_to_light_percent[5], 0, "palette 5 = 0%");

    /* Monotonically decreasing */
    int i;
    for (i = 0; i < 5; i++) {
        CHECK(csb_p4_k_palette_index_to_light_percent[i] >
              csb_p4_k_palette_index_to_light_percent[i + 1]);
    }

    /* Light power table: 16 entries, increasing */
    for (i = 0; i < 15; i++) {
        CHECK(csb_p4_k_light_power_to_percent[i] <=
              csb_p4_k_light_power_to_percent[i + 1]);
    }
    CHECK_EQ(csb_p4_k_light_power_to_percent[15], 100, "max power = 100%");
    CHECK_EQ(csb_p4_k_light_power_to_percent[0], 0, "min power = 0%");
}

/* ================================================================
 * Test 3: Spell projectile metadata lookup
 * ================================================================ */

static void test_spell_metadata_lookup(void) {
    const CSB_P4_SpellProjectileMetadata *meta;

    /* Spell id 1 = fire */
    meta = csb_p4_get_spell_projectile_metadata(1);
    CHECK(meta != NULL);
    CHECK_EQ(meta->category, CSB_P4_SPELL_CAT_FIRE, "spell 1 category");
    CHECK_EQ(meta->vfx_type, CSB_V2_VFX_FIRE, "spell 1 VFX type");
    CHECK_EQ(meta->has_trail, 1, "fire has trail");
    CHECK_EQ(meta->has_field_on_hit, 1, "fire has field on hit");

    /* Spell id 6 = lightning */
    meta = csb_p4_get_spell_projectile_metadata(6);
    CHECK(meta != NULL);
    CHECK_EQ(meta->category, CSB_P4_SPELL_CAT_LIGHTNING, "spell 6 category");
    CHECK_EQ(meta->vfx_type, CSB_V2_VFX_LIGHTNING, "spell 6 VFX");
    CHECK_EQ(meta->speed_tiles_per_sec, 8, "lightning speed");

    /* Spell id 11 = chaos */
    meta = csb_p4_get_spell_projectile_metadata(11);
    CHECK(meta != NULL);
    CHECK_EQ(meta->category, CSB_P4_SPELL_CAT_CHAOS, "spell 11 category");
    CHECK_EQ(meta->vfx_type, CSB_V2_VFX_CHAOS_MIST, "chaos VFX");
    CHECK_EQ(meta->has_field_on_hit, 1, "chaos has field on hit");

    /* Unknown spell id → fallback arcane */
    meta = csb_p4_get_spell_projectile_metadata(9999);
    CHECK(meta != NULL);
    CHECK_EQ(meta->category, CSB_P4_SPELL_CAT_ARCANE, "unknown → arcane");
    CHECK_EQ(meta->vfx_type, CSB_V2_VFX_MAGICAL_GLOW, "fallback VFX");

    /* Spell id 0 = none */
    meta = csb_p4_get_spell_projectile_metadata(0);
    CHECK(meta != NULL);
    CHECK_EQ(meta->category, CSB_P4_SPELL_CAT_NONE, "spell 0 = none");
}

/* ================================================================
 * Test 4: Spell category → VFX type
 * ================================================================ */

static void test_category_to_vfx(void) {
    CHECK_EQ(csb_p4_spell_category_to_vfx_type(CSB_P4_SPELL_CAT_FIRE),
             CSB_V2_VFX_FIRE, "fire cat → FIRE VFX");
    CHECK_EQ(csb_p4_spell_category_to_vfx_type(CSB_P4_SPELL_CAT_LIGHTNING),
             CSB_V2_VFX_LIGHTNING, "lightning cat → LIGHTNING VFX");
    CHECK_EQ(csb_p4_spell_category_to_vfx_type(CSB_P4_SPELL_CAT_CHAOS),
             CSB_V2_VFX_CHAOS_MIST, "chaos cat → CHAOS_MIST VFX");
    CHECK_EQ(csb_p4_spell_category_to_vfx_type(CSB_P4_SPELL_CAT_MAGICAL),
             CSB_V2_VFX_MAGICAL_GLOW, "magical cat → GLOW VFX");
    CHECK_EQ(csb_p4_spell_category_to_vfx_type(CSB_P4_SPELL_CAT_DARKNESS),
             CSB_V2_VFX_SMOKE, "darkness cat → SMOKE VFX");
    CHECK_EQ(csb_p4_spell_category_to_vfx_type(CSB_P4_SPELL_CAT_NONE),
             CSB_V2_VFX_NONE, "none cat → NONE VFX");

    /* Light check */
    CHECK(csb_p4_spell_category_has_light(CSB_P4_SPELL_CAT_FIRE));
    CHECK(csb_p4_spell_category_has_light(CSB_P4_SPELL_CAT_CHAOS));
    CHECK(!csb_p4_spell_category_has_light(CSB_P4_SPELL_CAT_NONE));
    CHECK(!csb_p4_spell_category_has_light(CSB_P4_SPELL_CAT_HEALING));
}

/* ================================================================
 * Test 5: VFX binding gate decisions
 * ================================================================ */

static void test_vfx_gates(void) {
    CSB_V2_PhaseGateConfig cfg_on;
    CSB_V2_PhaseGateConfig cfg_off;

    cfg_on.v2PresentationEnabled = 1;
    cfg_on.v2ConfigPersistenceEnabled = 0;
    cfg_off.v2PresentationEnabled = 0;
    cfg_off.v2ConfigPersistenceEnabled = 0;

    /* All gates off when V2 is disabled */
    CHECK(!csb_p4_vfx_gate_field_enabled(&cfg_off));
    CHECK(!csb_p4_vfx_gate_projectile_enabled(&cfg_off));
    CHECK(!csb_p4_vfx_gate_chaos_enabled(&cfg_off));
    CHECK(!csb_p4_vfx_gate_any_enabled(&cfg_off));

    /* All gates on when V2 is enabled */
    CHECK(csb_p4_vfx_gate_field_enabled(&cfg_on));
    CHECK(csb_p4_vfx_gate_projectile_enabled(&cfg_on));
    CHECK(csb_p4_vfx_gate_chaos_enabled(&cfg_on));
    CHECK(csb_p4_vfx_gate_any_enabled(&cfg_on));

    /* NULL config → off */
    CHECK(!csb_p4_vfx_gate_field_enabled(NULL));
    CHECK(!csb_p4_vfx_gate_projectile_enabled(NULL));
    CHECK(!csb_p4_vfx_gate_chaos_enabled(NULL));
}

/* ================================================================
 * Test 6: Projectile VFX fire / field VFX add / chaos trigger
 * ================================================================ */

static void test_binding_fire_projectile(void) {
    CSB_V2_PhaseGateConfig cfg;
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;

    csb_p4_binding_init();
    csb_p4_binding_reset();
    csb_v2_vfx_init();

    /* Fire a fireball (spell id 1) */
    int bix = csb_p4_binding_fire_projectile(&cfg, 1,
        5.0f, 5.0f, 10.0f, 10.0f);
    CHECK_EQ(bix >= 0, 1, "fireball binding returned valid index");

    /* Try to fire with V2 disabled → returns -1 */
    cfg.v2PresentationEnabled = 0;
    bix = csb_p4_binding_fire_projectile(&cfg, 1, 5.0f, 5.0f, 10.0f, 10.0f);
    CHECK_EQ(bix, -1, "fireball blocked when V2 disabled");
    cfg.v2PresentationEnabled = 1;

    /* Place a field VFX (spell id 8 = magical) */
    int fbx = csb_p4_binding_add_field(&cfg, 8, 7, 7);
    CHECK_EQ(fbx >= 0, 1, "field binding returned valid index");

    /* Chaos trigger for fire family → triggers light event */
    csb_v2_light_init();
    csb_v2_light_event_tick(0.0f);
    CHECK(!csb_v2_light_event_is_active());
    csb_p4_binding_trigger_chaos(&cfg, 1);  /* fire spell */
    CHECK(csb_v2_light_event_is_active());
    CHECK_EQ(csb_v2_light_event_current_type(),
             CSB_V2_LIGHT_EVENT_MAGICAL_PULSE, "fire → pulse");

    /* Chaos trigger for chaos family → chaos surge */
    csb_v2_light_event_tick(1.0f); /* expire previous */
    csb_p4_binding_trigger_chaos(&cfg, 11); /* chaos spell */
    CHECK(csb_v2_light_event_is_active());
    CHECK_EQ(csb_v2_light_event_current_type(),
             CSB_V2_LIGHT_EVENT_CHAOS_SURGE, "chaos → surge");

    /* Torch light source */
    int light_idx = csb_p4_binding_add_torch_light(&cfg, 5.0f, 5.0f,
        CSB_P4_TORCH_TYPE_BRIGHT);
    CHECK_EQ(light_idx >= 0, 1, "torch light added");

    /* Torch light blocked when V2 disabled */
    cfg.v2PresentationEnabled = 0;
    light_idx = csb_p4_binding_add_torch_light(&cfg, 5.0f, 5.0f,
        CSB_P4_TORCH_TYPE_MAGICAL);
    CHECK_EQ(light_idx, -1, "torch light blocked when V2 disabled");
}

/* ================================================================
 * Test 7: Binding tick expiry and active counts
 * ================================================================ */

static void test_binding_tick_and_counts(void) {
    CSB_V2_PhaseGateConfig cfg;
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;

    csb_p4_binding_reset();
    csb_v2_vfx_init();

    CHECK_EQ(csb_p4_binding_active_projectile_count(), 0, "no active proj");
    CHECK_EQ(csb_p4_binding_active_field_count(), 0, "no active field");

    /* Fire a lightning bolt */
    int bix = csb_p4_binding_fire_projectile(&cfg, 6,
        3.0f, 3.0f, 6.0f, 6.0f);
    CHECK_EQ(bix >= 0, 1, "lightning fired");
    CHECK_EQ(csb_p4_binding_active_projectile_count(), 1, "one active proj");

    /* Add a field */
    int fbx = csb_p4_binding_add_field(&cfg, 8, 4, 4);
    CHECK_EQ(fbx >= 0, 1, "field added");
    CHECK_EQ(csb_p4_binding_active_field_count(), 1, "one active field");

    /* Any binding active? */
    CHECK(csb_p4_binding_any_active());

    /* Tick with large dt to expire the projectile
     * (lightning is fast, expires quickly) */
    csb_p4_binding_tick(10.0f);

    /* After a long tick, the VFX should have expired.
     * The projectile (speed 8 tiles/s, distance ~4.2 tiles) expires in < 1s.
     * After 10s it should be gone. */
    CHECK_EQ(csb_p4_binding_active_projectile_count(), 0, "proj expired");

    /* Reset */
    csb_p4_binding_reset();
    CHECK_EQ(csb_p4_binding_active_projectile_count(), 0, "no active proj after reset");
    CHECK_EQ(csb_p4_binding_active_field_count(), 0, "no active field after reset");
    CHECK(!csb_p4_binding_any_active());
}

/* ================================================================
 * Test 8: Source evidence strings
 * ================================================================ */

static void test_source_evidence(void) {
    const char *ev;

    ev = csb_p4_lighting_metadata_source_evidence();
    CHECK(strstr(ev, "DATA.C:263") != NULL);
    CHECK(strstr(ev, "DATA.C:359") != NULL);
    CHECK(strstr(ev, "DATA.C:360") != NULL);
    CHECK(strstr(ev, "CASTER.C") != NULL);
}

/* ================================================================
 * main
 * ================================================================ */

int main(void) {
    test_torch_metadata();
    test_palette_light_table();
    test_spell_metadata_lookup();
    test_category_to_vfx();
    test_vfx_gates();
    test_binding_fire_projectile();
    test_binding_tick_and_counts();
    test_source_evidence();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("test_csb_p4_lighting: ok");
    return 0;
}
