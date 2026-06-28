/*
 * test_dm2_v1_lighting_falloff_boundary.c — DM2 V1 object lighting gate
 *
 * Covers exactly one deterministic runtime rule:
 * - source light falls off linearly by integer division inside the radius.
 * - distance_tiles == source->light_radius must extinguish to 0.
 *
 * Source: ReDMCSB DUNVIEW.C:4960-5039 — object depth scale and
 *         palette-change selection before object bitmap draw.
 */

#include "dm2_v1_viewport_renderer.h"
#include <stdio.h>
#include <string.h>

static int s_tests_run = 0;
static int s_tests_passed = 0;

#define CHECK(name_, cond_) do { \
    printf("  %s...\n", name_); \
    s_tests_run++; \
    if (cond_) { s_tests_passed++; printf("    PASS\n"); } \
    else      { printf("    FAIL\n"); } \
} while (0)

int main(void)
{
    printf("=== DM2 V1 Lighting/Palette Runtime Gate ===\n\n");

    DM2_CreatureSprite source = { 0 };
    source.light_radius = 4;
    CHECK("distance 0 keeps deterministic base brightness",
          dm2_v1_viewport_object_light_level(15, 0, &source) == 15);
    CHECK("distance 1 has deterministic integer falloff",
          dm2_v1_viewport_object_light_level(15, 1, &source) == 11);
    CHECK("distance 2 has deterministic integer falloff",
          dm2_v1_viewport_object_light_level(15, 2, &source) == 7);
    CHECK("distance 3 is the last lit tile before boundary",
          dm2_v1_viewport_object_light_level(15, 3, &source) == 3);
    CHECK("at boundary distance == radius is 0",
          dm2_v1_viewport_object_light_level(15, 4, &source) == 0);
    CHECK("beyond radius clamps to 0",
          dm2_v1_viewport_object_light_level(15, 8, &source) == 0);
    source.light_radius = 0;
    CHECK("zero-radius light source stays dark",
          dm2_v1_viewport_object_light_level(15, 0, &source) == 0);
    CHECK("null source keeps base tile light",
          dm2_v1_viewport_object_light_level(7, 4, NULL) == 7);

    {
        const char *e = dm2_v1_viewport_source_evidence();
        CHECK("source evidence cites DUNVIEW object draw path",
              e != NULL && strstr(e, "DUNVIEW.C:4960-5039") != NULL);
        CHECK("source evidence cites DM2 palette documentation",
              e != NULL && strstr(e, "docs/dm2_palette.md") != NULL);
    }

    printf("\nDM2 V1 Lighting/Palette Runtime Gate: %d/%d passed\n",
           s_tests_passed, s_tests_run);
    return (s_tests_passed == s_tests_run) ? 0 : 1;
}
