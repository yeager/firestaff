#include "csb_v2_lighting_dynamic.h"

#include <stdio.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    CSB_V2_SourcePaletteLighting plan;
    uint8_t r = 255u, g = 255u, b = 255u;

    csb_v2_light_init();
    plan = csb_v2_light_build_source_palette_lighting(2, true);
    check(plan.sourcePaletteIndex == 2u && plan.sourceLightAmountFloor == 50u &&
              plan.darknessPercent == 50u && !plan.enhancedEffectsEnabled,
          "source palette is preserved without host enhancement");
    plan = csb_v2_light_build_source_palette_lighting(-1, true);
    check(plan.sourcePaletteIndex == 5u && plan.deterministicFallback &&
              !plan.enhancedEffectsEnabled,
          "invalid source palette fails closed");
    check(csb_v2_light_add_source(1.0f, 2.0f, 3.0f, 255u,
                                  1u, 2u, 3u, 1) == -1,
          "product rejects host RGB light source");
    csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_CHAOS_SURGE, 1.0f, 1.0f);
    check(!csb_v2_light_event_is_active() &&
              csb_v2_light_event_current_type() == CSB_V2_LIGHT_EVENT_NORMAL,
          "product rejects host DSA light event");
    csb_v2_light_set_ambient(1.0f);
    csb_v2_light_set_dungeon_level(5);
    csb_v2_light_tick(10.0f);
    csb_v2_light_compute_map();
    csb_v2_light_get_tile(0, 0, &r, &g, &b);
    check(r == 0u && g == 0u && b == 0u &&
              csb_v2_light_get_ambient() == 0.0f &&
              csb_v2_light_get_dungeon_level() == 0,
          "product lighting stays transparent after host inputs");
    check(csb_v2_light_source_evidence() != NULL,
          "product source boundary names the original owners");

    if (failures != 0) {
        fprintf(stderr, "%d failures\n", failures);
        return 1;
    }
    printf("CSB V2 lighting runtime gate: 6 checks passed\n");
    return 0;
}
