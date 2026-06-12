#include "firestaff/dm1/v1/viewport/d3l_d3r_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_u32_nonzero(const char *id, uint32_t value, const char *anchor)
{
    ++g_assertions;
    if (value == 0u) {
        printf("FAIL %s got=0x%08x anchor=%s\n", id, (unsigned)value, anchor);
        ++g_failures;
    } else {
        printf("PASS %s != 0 hash=0x%08x anchor=%s\n",
               id, (unsigned)value, anchor);
    }
}

static void expect_contains(const char *id,
                            const char *haystack,
                            const char *needle,
                            const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id,
               needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void test_specs_and_anchors(void)
{
    const DM1_V1_D3LD3RF0108SpecPc34 *d3l =
        dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_for_side_pc34(
            DM1_V1_D3L_D3R_F0108_SIDE_D3L_PC34);
    const DM1_V1_D3LD3RF0108SpecPc34 *d3r =
        dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_for_side_pc34(
            DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34);
    const char *e = dm1_v1_viewport_d3l_d3r_f0108_source_evidence_pc34();

    expect_int("spec.count",
               (int)dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_count_pc34(),
               2, "D3L/D3R pair only");
    expect_int("spec.bounds",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_at_pc34(2) == NULL,
               1, "bounds guard");
    expect_int("spec.d3l.present", d3l != NULL, 1, "D3L spec");
    expect_int("spec.d3r.present", d3r != NULL, 1, "D3R spec");
    if (!d3l || !d3r) return;

    expect_int("d3l.dispatch_order", d3l->f0128_dispatch_order, 0,
               "DUNVIEW.C:8491 D3L first");
    expect_int("d3r.dispatch_order", d3r->f0128_dispatch_order, 1,
               "DUNVIEW.C:8495 D3R second");
    expect_int("d3l.view_square", d3l->owner_view_square, 12,
               "DEFS.H:2608 M601_VIEW_SQUARE_D3L");
    expect_int("d3r.view_square", d3r->owner_view_square, 13,
               "DEFS.H:2609 M602_VIEW_SQUARE_D3R");
    expect_int("d3l.view_floor", d3l->view_floor, 2,
               "DEFS.H:2752 M588_VIEW_FLOOR_D3L");
    expect_int("d3r.view_floor", d3r->view_floor, 4,
               "DEFS.H:2754 M590_VIEW_FLOOR_D3R");
    expect_int("d3l.floor_zone", d3l->floor_zone, 1502,
               "DUNVIEW.C:3998 C1500 + 0*C11 + M588");
    expect_int("d3r.floor_zone", d3r->floor_zone, 1504,
               "DUNVIEW.C:3998 C1500 + 0*C11 + M590");
    expect_int("d3l.right_side_flip", d3l->right_side_flip, 0,
               "DUNVIEW.C:3980 excludes D3L");
    expect_int("d3r.right_side_flip", d3r->right_side_flip, 1,
               "DUNVIEW.C:3980 includes M590_VIEW_FLOOR_D3R");
    expect_int("d3l.wall_zone", d3l->wall_zone, 705,
               "DEFS.H:4045 C705_ZONE_WALL_D3L");
    expect_int("d3r.wall_zone", d3r->wall_zone, 706,
               "DEFS.H:4046 C706_ZONE_WALL_D3R");
    expect_int("d3l.side_view_wall", d3l->side_view_wall, 2,
               "DEFS.H:2698 M575_VIEW_WALL_D3L_RIGHT");
    expect_int("d3r.side_view_wall", d3r->side_view_wall, 3,
               "DEFS.H:2699 M576_VIEW_WALL_D3R_LEFT");
    expect_int("d3l.front_view_wall", d3l->front_view_wall, 4,
               "DEFS.H:2700 M577_VIEW_WALL_D3L_FRONT");
    expect_int("d3r.front_view_wall", d3r->front_view_wall, 6,
               "DEFS.H:2702 M579_VIEW_WALL_D3R_FRONT");
    expect_int("d3l.wall_zone.side",
               dm1_v1_viewport_d3l_d3r_f0108_wall_ornament_zone_pc34(2, 2),
               1028, "DUNVIEW.C:3586-3587 C1004 + 2*C11 + M575");
    expect_int("d3r.wall_zone.front",
               dm1_v1_viewport_d3l_d3r_f0108_wall_ornament_zone_pc34(2, 6),
               1032, "DUNVIEW.C:3586-3587 C1004 + 2*C11 + M579");
    expect_int("d3l.order.open", d3l->corridor_order, 0x3421,
               "DUNVIEW.C:6476 C0x3421");
    expect_int("d3r.order.open", d3r->corridor_order, 0x4312,
               "DUNVIEW.C:6618 C0x4312");
    expect_int("d3l.order.door_pass1", d3l->door_pass1_order, 0x0218,
               "DUNVIEW.C:6444");
    expect_int("d3r.order.door_pass1", d3r->door_pass1_order, 0x0128,
               "DUNVIEW.C:6580");
    expect_int("d3l.order.door_pass2", d3l->door_pass2_order, 0x0349,
               "DUNVIEW.C:6459");
    expect_int("d3r.order.door_pass2", d3r->door_pass2_order, 0x0439,
               "DUNVIEW.C:6601");

    expect_contains("evidence.f0108", e, "DUNVIEW.C F0108:3940-4011",
                    "F0108 baseline");
    expect_contains("evidence.f0116", e, "F0116:6361-6498",
                    "D3L body carries F0108");
    expect_contains("evidence.f0117", e, "F0117:6500-6640",
                    "D3R body carries F0108");
    expect_contains("evidence.f0128", e, "F0128:8491-8517",
                    "D3L then D3R then D3C then D2 pair");
    expect_contains("evidence.f0163", e, "F0163:1769-1838",
                    "thing-list link boundary");
    expect_contains("evidence.f0164", e, "F0164:1840-1905",
                    "thing-list unlink boundary");
    expect_contains("evidence.f0172", e, "F0172:2466-2523",
                    "square aspect");
    expect_contains("evidence.defs", e, "M575..M579",
                    "wall positions");
    expect_contains("evidence.zones", e, "C1004, C1500",
                    "zone math");
}

static void test_compose_branches(void)
{
    static const DM1_V1_D3LD3RF0108ContextPc34 contexts[] = {
        DM1_V1_D3L_D3R_F0108_CONTEXT_WALL_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_CORRIDOR_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_OPEN_PIT_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_TELEPORTER_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_SIDE_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_FRONT_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_STAIRS_SIDE_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_STAIRS_FRONT_PC34
    };
    const DM1_V1_D3LD3RF0108SidePc34 sides[] = {
        DM1_V1_D3L_D3R_F0108_SIDE_D3L_PC34,
        DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34
    };
    size_t side_index;

    for (side_index = 0; side_index < 2; ++side_index) {
        size_t context_index;
        for (context_index = 0; context_index < sizeof(contexts) / sizeof(contexts[0]); ++context_index) {
            DM1_V1_D3LD3RF0108StatePc34 state;
            DM1_V1_D3LD3RF0108ResultPc34 result;
            char id[96];
            const int is_wall =
                contexts[context_index] == DM1_V1_D3L_D3R_F0108_CONTEXT_WALL_PC34;

            snprintf(id, sizeof(id), "compose.%u.%u.init",
                     (unsigned)side_index, (unsigned)context_index);
            expect_int(id,
                       dm1_v1_viewport_d3l_d3r_f0108_initial_state_pc34(
                           sides[side_index], contexts[context_index], &state),
                       1, "all D3L/D3R F0116/F0117 branch contexts");
            snprintf(id, sizeof(id), "compose.%u.%u.ok",
                     (unsigned)side_index, (unsigned)context_index);
            expect_int(id,
                       dm1_v1_viewport_d3l_d3r_f0108_compose_pc34(&state, &result),
                       1, "synthetic 320x200 framebuffer probe");
            expect_int("compose.framebuffer.width", result.framebuffer_width, 320,
                       "synthetic framebuffer width");
            expect_int("compose.framebuffer.height", result.framebuffer_height, 200,
                       "synthetic framebuffer height");
            expect_int("compose.viewport.width", result.viewport_width, 224,
                       "viewport width");
            expect_int("compose.viewport.height", result.viewport_height, 136,
                       "viewport height");
            expect_int("compose.ceiling", result.ceiling_base_calls, 1,
                       "F0128 floor/ceiling baseline");
            expect_int("compose.floor", result.floor_base_calls, 1,
                       "F0128 floor/ceiling baseline");
            expect_int("compose.dispatch", result.f0128_d3l_then_d3r_then_d3c, 1,
                       "DUNVIEW.C:8491-8499");
            expect_int("compose.d2_later", result.terminal_depth_d2_pair_drawn_later, 1,
                       "DUNVIEW.C:8513/8517 later D2 side pair");
            expect_int("compose.d2_sample", result.d2_later_sample, 0x7d,
                       "later D2 marker overwrites after D3 side pair");
            expect_int("compose.non_overlap", result.non_overlap_ok, 1,
                       "pass770 non-overlap marker");
            if (is_wall) {
                expect_int("compose.wall_body", result.wall_body_calls, 1,
                           "F0116/F0117 wall body C705/C706");
                expect_int("compose.wall_no_f0108", result.floor_ornament_calls, 0,
                           "wall branch returns after F0107 wall ornaments");
            } else {
                expect_int("compose.f0108", result.floor_ornament_calls, 1,
                           "F0108 carried by D3L/D3R body");
                expect_int("compose.footprints", result.footprint_recursions, 1,
                           "DUNVIEW.C:4006-4008 footprint recursion");
                expect_int("compose.c10", result.c10_transparent_blits > 0, 1,
                           "C10_COLOR_FLESH transparent blit");
                expect_int("compose.thing_pass", result.front_thing_pass_calls, 1,
                           "F0115 after F0108");
            }
            if (contexts[context_index] == DM1_V1_D3L_D3R_F0108_CONTEXT_OPEN_PIT_PC34) {
                expect_int("compose.open_pit_floor", result.open_pit_still_draws_floor_ornament,
                           1, "BUG0_64 F0108 over open pits");
            }
            if (contexts[context_index] == DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_FRONT_PC34) {
                expect_int("compose.door_rear_pass", result.rear_thing_pass_calls, 1,
                           "D3 door-front rear F0115 pass");
            }
        }
    }
}

static void test_helpers_and_non_overlap(void)
{
    DM1_V1_D3LD3RF0108OrdinalPc34 ordinal;
    DM1_V1_D3LD3RF0108StatePc34 rejected;
    DM1_V1_D3LD3RF0108ResultPc34 result;
    const char *marker =
        dm1_v1_viewport_d3l_d3r_f0108_non_overlap_marker_pc34();

    expect_int("decode.footprint",
               dm1_v1_viewport_d3l_d3r_f0108_decode_ordinal_pc34(0x8004u, &ordinal),
               1, "DUNVIEW.C:3960-3965");
    expect_int("decode.primary_index", ordinal.primary_index, 3,
               "ordinal-to-index after M009_CLEAR");
    expect_int("decode.footprint_index", ordinal.recursive_footprints_index, 15,
               "DEFS.H C15_FLOOR_ORNAMENT_FOOTPRINTS");
    expect_int("decode.metadata_blits", ordinal.metadata_blit_count, 2,
               "primary + recursive footprint metadata blits");
    expect_int("blend.transparent",
               dm1_v1_viewport_d3l_d3r_f0108_blend_c10_pc34(0xaa, 10),
               0xaa, "C10_COLOR_FLESH preserves destination");
    expect_int("blend.opaque",
               dm1_v1_viewport_d3l_d3r_f0108_blend_c10_pc34(0xaa, 0x51),
               0x51, "opaque pixel writes");
    expect_int("floor_zone.invalid",
               dm1_v1_viewport_d3l_d3r_f0108_floor_zone_pc34(-1, 2),
               -1, "zone guard");
    expect_int("wall_zone.invalid",
               dm1_v1_viewport_d3l_d3r_f0108_wall_ornament_zone_pc34(2, -1),
               -1, "zone guard");

    expect_contains("nonoverlap.d0c", marker, "D0C",
                    "does not duplicate D0C F0108 gate");
    expect_contains("nonoverlap.d0lr", marker, "D0L/D0R",
                    "does not duplicate D0L/D0R F0108 gate");
    expect_contains("nonoverlap.d0l2r2", marker, "D0L2/D0R2",
                    "does not duplicate D0L2/D0R2 F0108 gate");
    expect_contains("nonoverlap.d1c", marker, "D1C",
                    "does not duplicate D1C F0108 gate");
    expect_contains("nonoverlap.d1lr", marker, "D1L/D1R",
                    "does not duplicate D1L/D1R F0108 gate");
    expect_contains("nonoverlap.d1l2r2", marker, "D1L2/D1R2",
                    "does not duplicate D1L2/D1R2 F0108 gate");
    expect_contains("nonoverlap.d2lr", marker, "D2L/D2R",
                    "does not duplicate D2L/D2R F0108 gate");
    expect_contains("nonoverlap.d2l2r2", marker, "D2L2/D2R2",
                    "does not duplicate D2L2/D2R2 F0108 gate");
    expect_contains("nonoverlap.d3c", marker, "D3C",
                    "does not duplicate D3C F0108 gate");
    expect_contains("nonoverlap.f0107", marker, "D3L/D3R F0107",
                    "does not duplicate D3L/D3R F0107 wall-ornament gate");

    expect_int("reject.init",
               dm1_v1_viewport_d3l_d3r_f0108_initial_state_pc34(
                   DM1_V1_D3L_D3R_F0108_SIDE_D3L_PC34,
                   DM1_V1_D3L_D3R_F0108_CONTEXT_CORRIDOR_PC34,
                   &rejected),
               1, "rejection fixture");
    rejected.allow_sibling_f0108_overlap = true;
    expect_int("reject.sibling_overlap",
               dm1_v1_viewport_d3l_d3r_f0108_compose_pc34(&rejected, &result),
               0, "explicit sibling F0108 non-overlap self-assert");
    rejected.allow_sibling_f0108_overlap = false;
    rejected.allow_f0107_wall_ornament_duplicate = true;
    expect_int("reject.f0107_duplicate",
               dm1_v1_viewport_d3l_d3r_f0108_compose_pc34(&rejected, &result),
               0, "explicit F0107 non-overlap self-assert");
}

int main(void)
{
    const DM1_V1_D3LD3RF0108SelfTestResultPc34 *result;
    int ok;

    test_specs_and_anchors();
    test_compose_branches();
    test_helpers_and_non_overlap();

    ok = run_dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_self_test();
    result = dm1_v1_viewport_d3l_d3r_f0108_last_self_test_result_pc34();
    expect_int("selftest.ok", ok, 1, "library self-test");
    expect_int("selftest.failures", result ? result->failures : 1, 0,
               "library self-test failures");
    expect_int("selftest.d3l_floor", result ? result->d3l_floor_calls : 0, 7,
               "D3L non-wall contexts exercise F0108");
    expect_int("selftest.d3r_floor", result ? result->d3r_floor_calls : 0, 7,
               "D3R non-wall contexts exercise F0108");
    expect_int("selftest.wall_bodies", result ? result->wall_body_calls : 0, 2,
               "D3L/D3R wall branches exercise C705/C706 carriers");
    expect_int("selftest.non_overlap_assertions",
               result ? result->non_overlap_assertions : 0, 3,
               "non-overlap guards");
    expect_u32_nonzero("selftest.hash",
                       result ? result->deterministic_hash : 0u,
                       "deterministic branch hash");

    if (g_failures) {
        printf("FAIL test_dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_pc34_compat "
               "assertions=%d failures=%d hash=0x%08x\n",
               g_assertions, g_failures,
               result ? (unsigned)result->deterministic_hash : 0u);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_pc34_compat "
           "assertions=%d failures=0 d3l_floor=%d d3r_floor=%d wall_bodies=%d "
           "footprint_recursions=%d ceilings=%d thing_passes=%d "
           "mutation_rejections=%d non_overlap=%d hash=0x%08x\n",
           g_assertions + (result ? result->assertions : 0),
           result ? result->d3l_floor_calls : 0,
           result ? result->d3r_floor_calls : 0,
           result ? result->wall_body_calls : 0,
           result ? result->footprint_recursions : 0,
           result ? result->ceiling_calls : 0,
           result ? result->thing_pass_calls : 0,
           result ? result->mutation_rejections : 0,
           result ? result->non_overlap_assertions : 0,
           result ? (unsigned)result->deterministic_hash : 0u);
    return 0;
}
