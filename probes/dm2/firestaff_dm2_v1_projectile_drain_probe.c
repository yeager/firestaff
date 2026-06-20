/*
 * firestaff_dm2_v1_projectile_drain_probe.c — DM2 V1 Projectile Drain Probe
 *
 * Exercises the dm2_v1_projectile_pc34_compat.c drain API: creates
 * synthetic projectiles, drains them into the M11-ready DrainedProjectile
 * array, and verifies framebuffer pixel coordinates are correct.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_render.cpp   - projectile draw routine
 *   ReDMCSB DUNGEON.C:2362-2387       - F0209 visible row/column
 *   ReDMCSB PROJEXPL.C:76-92          - F0212 projectile live
 */

#include "dm2_v1_projectile_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int errors = 0;
static int passed = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    fprintf(stderr, "=== DM2 V1 Projectile Drain Verification Probe ===\n");
    fprintf(stderr, "Source: skproject/SKULLWIN/c_render.cpp (projectile draw)\n");
    fprintf(stderr, "        ReDMCSB DUNGEON.C:2362-2387 (F0209 visible row/col)\n");
    fprintf(stderr, "        ReDMCSB PROJEXPL.C:76-92 (F0212 projectile live)\n\n");

    dm2_v1_projectile_reset_counters();
    dm2_v1_projectile_active_count();  /* ensure init */

    /* Invariant 1: empty drain returns 0. */
    DM2_V1_DrainedProjectile list[16];
    int n = dm2_v1_projectile_drain_to_m11(list, 16);
    PROBE_ASSERT(n == 0, "empty drain returns 0 (got %d)", n);

    /* Invariant 2: dispatch 3 synthetic projectiles with sensible coords. */
    int s1 = dm2_v1_projectile_dispatch_synthetic(PROJECTILE_CATEGORY_MAGICAL,
                                                    DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                                    5, 5, 0, 0);
    int s2 = dm2_v1_projectile_dispatch_synthetic(PROJECTILE_CATEGORY_KINETIC,
                                                    DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                                    10, 8, 0, 1);
    int s3 = dm2_v1_projectile_dispatch_synthetic(PROJECTILE_CATEGORY_MAGICAL,
                                                    DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING,
                                                    7, 4, 1, 2);
    PROBE_ASSERT(s1 >= 0 && s2 >= 0 && s3 >= 0,
                 "dispatched 3 projectiles (slots: %d, %d, %d)", s1, s2, s3);

    /* Invariant 3: active_count matches dispatched. */
    PROBE_ASSERT(dm2_v1_projectile_active_count() == 3,
                 "active_count = 3 (got %d)", dm2_v1_projectile_active_count());

    /* Invariant 4: drain returns 3 projectiles. */
    memset(list, 0, sizeof(list));
    n = dm2_v1_projectile_drain_to_m11(list, 16);
    PROBE_ASSERT(n == 3,
                 "drain returns 3 projectiles (got %d)", n);

    /* Invariant 5: drained entries have correct world coords. */
    int found_5_5 = 0, found_10_8 = 0, found_7_4 = 0;
    for (int i = 0; i < n; i++) {
        if (list[i].map_x == 5 && list[i].map_y == 5) found_5_5 = 1;
        if (list[i].map_x == 10 && list[i].map_y == 8) found_10_8 = 1;
        if (list[i].map_x == 7 && list[i].map_y == 4) found_7_4 = 1;
    }
    PROBE_ASSERT(found_5_5 && found_10_8 && found_7_4,
                 "all 3 world coordinates present in drained list");

    /* Invariant 6: pixel coords are computed correctly.
     * pixel_x = 32 + (map_x - map_y) * 32 + (cell_x * 16)
     * pixel_y = (map_x + map_y) * 8 + (cell_y * 8)
     * For cell=0: pixel_x = 32 + (map_x - map_y) * 32, pixel_y = (map_x + map_y) * 8
     */
    for (int i = 0; i < n; i++) {
        int expected_x = 32 + (list[i].map_x - list[i].map_y) * 32;
        int expected_y = (list[i].map_x + list[i].map_y) * 8;
        if (list[i].pixel_x != expected_x || list[i].pixel_y != expected_y) {
            fprintf(stderr, "  mismatch: world=(%d,%d) expected pixel=(%d,%d) got (%d,%d)\n",
                    list[i].map_x, list[i].map_y,
                    expected_x, expected_y, list[i].pixel_x, list[i].pixel_y);
            errors++;
            break;
        }
    }
    PROBE_ASSERT(errors == 0, "all pixel coords computed correctly");

    /* Invariant 7: pixel coords are within V1 viewport bounds (0..319, 0..199). */
    int in_bounds = 1;
    for (int i = 0; i < n; i++) {
        if (list[i].pixel_x < 0 || list[i].pixel_x > 319
         || list[i].pixel_y < 0 || list[i].pixel_y > 199) {
            in_bounds = 0;
            break;
        }
    }
    PROBE_ASSERT(in_bounds, "all pixel coords within V1 viewport bounds");

    /* Invariant 8: direction propagated correctly. */
    int dir_ok = 1;
    for (int i = 0; i < n; i++) {
        if (list[i].map_x == 5 && list[i].map_y == 5 && list[i].direction != 0) dir_ok = 0;
        if (list[i].map_x == 10 && list[i].map_y == 8 && list[i].direction != 1) dir_ok = 0;
        if (list[i].map_x == 7 && list[i].map_y == 4 && list[i].direction != 2) dir_ok = 0;
    }
    PROBE_ASSERT(dir_ok, "direction propagated from dispatch to drain");

    /* Invariant 9: drain with max_count=1 returns 1. */
    n = dm2_v1_projectile_drain_to_m11(list, 1);
    PROBE_ASSERT(n == 1, "drain with max_count=1 returns 1 (got %d)", n);

    /* Invariant 10: drain with NULL out_list returns 0. */
    n = dm2_v1_projectile_drain_to_m11(NULL, 16);
    PROBE_ASSERT(n == 0, "drain with NULL returns 0 (got %d)", n);

    /* Invariant 11: drain with max_count=0 returns 0. */
    n = dm2_v1_projectile_drain_to_m11(list, 0);
    PROBE_ASSERT(n == 0, "drain with max_count=0 returns 0 (got %d)", n);

    /* Invariant 12: source evidence. */
    const char *e = dm2_v1_projectile_source_evidence();
    PROBE_ASSERT(e != NULL && strstr(e, "SKULL.ASM:10620-10710") != NULL,
                 "source evidence mentions SKULL.ASM ranged combat");

    fprintf(stderr, "\n=== Summary ===\n");
    fprintf(stderr, "Active projectiles: %d\n", dm2_v1_projectile_active_count());
    fprintf(stderr, "Drained entries:    %d (of 3 dispatched)\n", n);
    fprintf(stderr, "\n%d/%d invariants PASS\n", passed, passed + errors);
    return (errors == 0) ? 0 : 1;
}
