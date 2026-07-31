#include "csb_p4_lighting_metadata.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; } } while (0)

int main(void) {
    CSB_V2_PhaseGateConfig cfg = { 1, 1 };
    int i;

    /* These three tables are direct ReDMCSB DATA.C material. */
    CHECK(csb_p4_charge_count_to_torch_type(0) == CSB_P4_TORCH_TYPE_NONE);
    CHECK(csb_p4_charge_count_to_torch_type(4) == CSB_P4_TORCH_TYPE_BRIGHT);
    CHECK(csb_p4_charge_count_to_torch_type(15) == CSB_P4_TORCH_TYPE_MAGICAL);
    CHECK(csb_p4_k_light_power_to_percent[0] == 0);
    CHECK(csb_p4_k_light_power_to_percent[15] == 100);
    CHECK(csb_p4_k_palette_index_to_light_percent[0] == 99);
    CHECK(csb_p4_k_palette_index_to_light_percent[5] == 0);
    for (i = 0; i < 15; ++i) {
        CHECK(csb_p4_k_light_power_to_percent[i] <=
              csb_p4_k_light_power_to_percent[i + 1]);
    }

    /* The old modern VFX mapping had invented spell ids and render values. */
    CHECK(csb_p4_torch_type_to_intensity(CSB_P4_TORCH_TYPE_MAGICAL) == 0);
    CHECK(csb_p4_get_spell_projectile_metadata(0) == NULL);
    CHECK(csb_p4_get_spell_projectile_metadata(1) == NULL);
    CHECK(csb_p4_get_spell_projectile_metadata(9999) == NULL);
    CHECK(csb_p4_spell_category_to_vfx_type(CSB_P4_SPELL_CAT_FIRE) == CSB_V2_VFX_NONE);
    CHECK(!csb_p4_spell_category_has_light(CSB_P4_SPELL_CAT_CHAOS));
    CHECK(!csb_p4_vfx_gate_field_enabled(&cfg));
    CHECK(!csb_p4_vfx_gate_projectile_enabled(&cfg));
    CHECK(!csb_p4_vfx_gate_chaos_enabled(&cfg));
    CHECK(!csb_p4_vfx_gate_any_enabled(&cfg));
    CHECK(!csb_p4_vfx_gate_field_enabled(NULL));

    /* The retired binding must stay inert even if an accidental caller passes
     * a permissive-looking phase config. */
    csb_p4_binding_init();
    CHECK(csb_p4_binding_fire_projectile(&cfg, 1, 0.0f, 0.0f,
                                         1.0f, 1.0f) == -1);
    CHECK(csb_p4_binding_add_field(&cfg, 1, 0, 0) == -1);
    csb_p4_binding_trigger_chaos(&cfg, 1);
    CHECK(csb_p4_binding_add_torch_light(&cfg, 0.0f, 0.0f,
                                         CSB_P4_TORCH_TYPE_NORMAL) == -1);
    csb_p4_binding_tick(1.0f);
    CHECK(csb_p4_binding_active_projectile_count() == 0);
    CHECK(csb_p4_binding_active_field_count() == 0);
    CHECK(!csb_p4_binding_any_active());

    CHECK(strstr(csb_p4_lighting_metadata_source_evidence(), "DATA.C:263") != NULL);
    CHECK(strstr(csb_p4_lighting_metadata_source_evidence(), "DATA.C:359") != NULL);
    if (failures) return 1;
    puts("test_csb_p4_lighting: ok");
    return 0;
}
