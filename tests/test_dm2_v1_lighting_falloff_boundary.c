/*
 * test_dm2_v1_lighting_falloff_boundary.c — DM2 V1 object lighting boundary test
 *
 * Covers exactly one boundary rule:
 * - distance_tiles == source->light_radius must extinguish to 0.
 */

#include "dm2_v1_viewport_renderer.h"
#include <stdio.h>

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
    printf("=== DM2 V1 Lighting Falloff Boundary ===\n\n");

    DM2_CreatureSprite source = { 0 };
    source.light_radius = 4;
    CHECK("at boundary distance == radius is 0",
          dm2_v1_viewport_object_light_level(15, 4, &source) == 0);
    CHECK("within boundary distance < radius remains lit",
          dm2_v1_viewport_object_light_level(15, 3, &source) > 0);
    CHECK("beyond radius clamps to 0",
          dm2_v1_viewport_object_light_level(15, 8, &source) == 0);
    CHECK("null source keeps base tile light",
          dm2_v1_viewport_object_light_level(7, 4, NULL) == 7);

    printf("\nDM2 V1 Lighting Falloff Boundary: %d/%d passed\n", s_tests_passed, s_tests_run);
    return (s_tests_passed == s_tests_run) ? 0 : 1;
}
