#include "dm1_v1_viewport_f0107_wall_ornament_alcove_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void check_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void check_contains(const char *id, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)",
               anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

#define CHECK_INT(id, got, want, anchor) check_int((id), (got), (want), (anchor))
#define CHECK_TRUE(id, got, anchor) check_int((id), (got) ? 1 : 0, 1, (anchor))
#define CHECK_FALSE(id, got, anchor) check_int((id), (got) ? 1 : 0, 0, (anchor))
#define CHECK_CONTAINS(id, haystack, needle, anchor) \
    check_contains((id), (haystack), (needle), (anchor))

static int decide(int ordinal_slot, int wall_cell_code, int coordinate_set, int wall_set)
{
    return dm1_v1_viewport_f0107_wall_ornament_alcove_decide(
               ordinal_slot, wall_cell_code, coordinate_set, wall_set) ? 1 : 0;
}

static void test_table_contract(void)
{
    size_t count = 0;
    const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *cases =
        dm1_v1_viewport_f0107_wall_ornament_alcove_cases_pc34(&count);

    CHECK_TRUE("cases.pointer", cases != NULL,
               "DUNVIEW.C:3502-3938 F0107 contract fixture");
    CHECK_INT("cases.count", (int)count, 12,
              "eight side-wall entries plus front/mirror rejects");
    CHECK_TRUE("cases.null_count_safe",
               dm1_v1_viewport_f0107_wall_ornament_alcove_cases_pc34(NULL) != NULL,
               "NULL-safe fixture count");
    CHECK_TRUE("case0.anchor",
               cases && cases[0].redmcsb_anchor &&
                   strstr(cases[0].redmcsb_anchor, "DUNVIEW.C:6263") != NULL,
               "DUNVIEW.C:6263 C00/M551 call site");
    CHECK_TRUE("case_last.reject",
               cases && cases[11].returns_alcove == false,
               "DUNVIEW.C:6432-6433/M554 reject evidence");
}

static void test_cell_code_coordinate_mapping(void)
{
    CHECK_INT("cell.C00.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(
                  DM1_V1_F0107_WALL_CELL_C00_D3L2_RIGHT_PC34),
              DM1_V1_F0107_COORDINATE_D3L2_RIGHT_PC34,
              "DUNVIEW.C:6263; DEFS.H:2696 C00_VIEW_WALL_D3L2_RIGHT");
    CHECK_INT("cell.C01.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(
                  DM1_V1_F0107_WALL_CELL_C01_D3R2_LEFT_PC34),
              DM1_V1_F0107_COORDINATE_D3R2_LEFT_PC34,
              "DUNVIEW.C:6330; DEFS.H:2697 C01_VIEW_WALL_D3R2_LEFT");
    CHECK_INT("cell.M575.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(
                  DM1_V1_F0107_WALL_CELL_M575_D3L_RIGHT_PC34),
              DM1_V1_F0107_COORDINATE_D3L_RIGHT_PC34,
              "DUNVIEW.C:6432; DEFS.H:2698 M575_VIEW_WALL_D3L_RIGHT");
    CHECK_INT("cell.M576.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(
                  DM1_V1_F0107_WALL_CELL_M576_D3R_LEFT_PC34),
              DM1_V1_F0107_COORDINATE_D3R_LEFT_PC34,
              "DUNVIEW.C:6568; DEFS.H:2699 M576_VIEW_WALL_D3R_LEFT");
    CHECK_INT("cell.M580.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(
                  DM1_V1_F0107_WALL_CELL_M580_D2L_RIGHT_PC34),
              DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34,
              "DUNVIEW.C:6968; DEFS.H:2703 M580_VIEW_WALL_D2L_RIGHT");
    CHECK_INT("cell.M581.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(
                  DM1_V1_F0107_WALL_CELL_M581_D2R_LEFT_PC34),
              DM1_V1_F0107_COORDINATE_D2R_LEFT_PC34,
              "DUNVIEW.C:7119; DEFS.H:2704 M581_VIEW_WALL_D2R_LEFT");
    CHECK_INT("cell.M585.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(
                  DM1_V1_F0107_WALL_CELL_M585_D1L_RIGHT_PC34),
              DM1_V1_F0107_COORDINATE_D1L_RIGHT_PC34,
              "DUNVIEW.C:7459; DEFS.H:2708 M585_VIEW_WALL_D1L_RIGHT");
    CHECK_INT("cell.M586.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(
                  DM1_V1_F0107_WALL_CELL_M586_D1R_LEFT_PC34),
              DM1_V1_F0107_COORDINATE_D1R_LEFT_PC34,
              "DUNVIEW.C:7627; DEFS.H:2709 M586_VIEW_WALL_D1R_LEFT");
    CHECK_INT("cell.front.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(
                  DM1_V1_F0107_WALL_CELL_M582_D2L_FRONT_PC34),
              DM1_V1_F0107_COORDINATE_BACK_WALL_FRONT_PC34,
              "DUNVIEW.C:6969; DEFS.H:2705 M582_VIEW_WALL_D2L_FRONT");
    CHECK_INT("cell.unknown.coord",
              dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(99),
              DM1_V1_F0107_COORDINATE_UNKNOWN_PC34,
              "unknown wall-cell reject");
}

static void test_side_alcove_contract(void)
{
    CHECK_INT("M551.C00.alcove",
              decide(DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_C00_D3L2_RIGHT_PC34,
                     DM1_V1_F0107_COORDINATE_D3L2_RIGHT_PC34,
                     DM1_V1_F0107_WALLSET_C11_D3L2_PC34),
              1, "DUNVIEW.C:6263 calls F0107(M551,C00)");
    CHECK_INT("M553.C01.alcove",
              decide(DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_C01_D3R2_LEFT_PC34,
                     DM1_V1_F0107_COORDINATE_D3R2_LEFT_PC34,
                     DM1_V1_F0107_WALLSET_C10_D3R2_PC34),
              1, "DUNVIEW.C:6330 calls F0107(M553,C01)");
    CHECK_INT("M551.D3L.alcove",
              decide(DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M575_D3L_RIGHT_PC34,
                     DM1_V1_F0107_COORDINATE_D3L_RIGHT_PC34,
                     DM1_V1_F0107_WALLSET_C13_D3L_PC34),
              1, "DUNVIEW.C:6432 calls F0107(M551,M575)");
    CHECK_INT("M553.D3R.alcove",
              decide(DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M576_D3R_LEFT_PC34,
                     DM1_V1_F0107_COORDINATE_D3R_LEFT_PC34,
                     DM1_V1_F0107_WALLSET_C12_D3R_PC34),
              1, "DUNVIEW.C:6568 calls F0107(M553,M576)");
    CHECK_INT("M551.D2L.alcove",
              decide(DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M580_D2L_RIGHT_PC34,
                     DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34,
                     DM1_V1_F0107_WALLSET_C08_D2L_PC34),
              1, "DUNVIEW.C:6968 calls F0107(M551,M580)");
    CHECK_INT("M553.D2R.alcove",
              decide(DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M581_D2R_LEFT_PC34,
                     DM1_V1_F0107_COORDINATE_D2R_LEFT_PC34,
                     DM1_V1_F0107_WALLSET_C07_D2R_PC34),
              1, "DUNVIEW.C:7119 calls F0107(M553,M581)");
    CHECK_INT("M551.D1L.alcove",
              decide(DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M585_D1L_RIGHT_PC34,
                     DM1_V1_F0107_COORDINATE_D1L_RIGHT_PC34,
                     DM1_V1_F0107_WALLSET_C03_D1L_PC34),
              1, "DUNVIEW.C:7459 calls F0107(M551,M585)");
    CHECK_INT("M553.D1R.alcove",
              decide(DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M586_D1R_LEFT_PC34,
                     DM1_V1_F0107_COORDINATE_D1R_LEFT_PC34,
                     DM1_V1_F0107_WALLSET_C02_D1R_PC34),
              1, "DUNVIEW.C:7627 calls F0107(M553,M586)");
}

static void test_front_and_mirror_reject_contract(void)
{
    CHECK_INT("M552.D2L.front.no_alcove",
              decide(DM1_V1_F0107_SLOT_M552_FRONT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M582_D2L_FRONT_PC34,
                     DM1_V1_F0107_COORDINATE_BACK_WALL_FRONT_PC34,
                     DM1_V1_F0107_WALLSET_C08_D2L_PC34),
              0, "DUNVIEW.C:6969-6971 front F0107 is an order gate");
    CHECK_INT("M552.D2R.front.no_alcove",
              decide(DM1_V1_F0107_SLOT_M552_FRONT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M584_D2R_FRONT_PC34,
                     DM1_V1_F0107_COORDINATE_BACK_WALL_FRONT_PC34,
                     DM1_V1_F0107_WALLSET_C07_D2R_PC34),
              0, "DUNVIEW.C:7120-7122 front F0107 is an order gate");
    CHECK_INT("M552.D1C.front.no_alcove",
              decide(DM1_V1_F0107_SLOT_M552_FRONT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M587_D1C_FRONT_PC34,
                     DM1_V1_F0107_COORDINATE_BACK_WALL_FRONT_PC34,
                     DM1_V1_F0107_WALLSET_C03_D1L_PC34),
              0, "DUNVIEW.C:7842 front F0107 is an order gate");
    CHECK_INT("M554.mirror_front.no_alcove",
              decide(DM1_V1_F0107_SLOT_M554_MIRROR_FRONT_WALL_PC34,
                     DM1_V1_F0107_WALL_CELL_M582_D2L_FRONT_PC34,
                     DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34,
                     DM1_V1_F0107_WALLSET_C08_D2L_PC34),
              0, "DUNVIEW.C:6432-6433 and 6968-6969 use M551/M552/M553 only");
    CHECK_INT("M551.wrong_coordinate.no_alcove",
              decide(DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M580_D2L_RIGHT_PC34,
                     DM1_V1_F0107_COORDINATE_D2R_LEFT_PC34,
                     DM1_V1_F0107_WALLSET_C08_D2L_PC34),
              0, "DUNVIEW.C:3578 coordinate-set selected by view-wall code");
    CHECK_INT("M553.wrong_wallset.no_alcove",
              decide(DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M581_D2R_LEFT_PC34,
                     DM1_V1_F0107_COORDINATE_D2R_LEFT_PC34,
                     DM1_V1_F0107_WALLSET_C08_D2L_PC34),
              0, "DEFS.H:3430-3431 C07/C08 wall-set split");
}

static void test_null_safety_and_repeatability(void)
{
    const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *entry =
        dm1_v1_viewport_f0107_wall_ornament_alcove_case_at_pc34(4);
    int first;
    int second;

    CHECK_TRUE("case_at.valid", entry != NULL,
               "fixture entry for DUNVIEW.C:6968 M580");
    CHECK_TRUE("case_at.invalid.null",
               dm1_v1_viewport_f0107_wall_ornament_alcove_case_at_pc34(99) == NULL,
               "NULL-safe fixture lookup");
    CHECK_FALSE("decide_case.null.false",
                dm1_v1_viewport_f0107_wall_ornament_alcove_decide_case_pc34(NULL),
                "NULL-safe helper wrapper");
    CHECK_TRUE("decide_case.entry.matches",
               dm1_v1_viewport_f0107_wall_ornament_alcove_decide_case_pc34(entry),
               "DUNVIEW.C:3933 returns stable BOOLEAN");

    first = decide(DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
                   DM1_V1_F0107_WALL_CELL_M580_D2L_RIGHT_PC34,
                   DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34,
                   DM1_V1_F0107_WALLSET_C08_D2L_PC34);
    second = decide(DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
                    DM1_V1_F0107_WALL_CELL_M580_D2L_RIGHT_PC34,
                    DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34,
                    DM1_V1_F0107_WALLSET_C08_D2L_PC34);
    CHECK_INT("repeat.same_tuple", second, first,
              "DUNVIEW.C:3502-3938 pure tuple decision is stable");
    CHECK_INT("unknown.cell.false",
              decide(DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34, 99,
                     DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34,
                     DM1_V1_F0107_WALLSET_C08_D2L_PC34),
              0, "unknown wall-cell rejects before F0107 contract decision");
    CHECK_INT("unknown.coordinate.false",
              decide(DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
                     DM1_V1_F0107_WALL_CELL_M580_D2L_RIGHT_PC34,
                     DM1_V1_F0107_COORDINATE_UNKNOWN_PC34,
                     DM1_V1_F0107_WALLSET_C08_D2L_PC34),
              0, "unknown coordinate-set rejects before F0107 contract decision");
}

static void test_d1r_side_ornament_pixel_contract(void)
{
    uint8_t source[DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_WIDTH_PC34 *
                   DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_F0107_WALL_ORNAMENT_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_HEIGHT_PC34];
    DM1_V1_F0107WallOrnamentPixelResultPc34 out;
    DM1_V1_F0107WallOrnamentPixelInputPc34 input = {
        7,
        0,
        0,
        DM1_V1_F0107_WALL_ORNAMENT_C10_COLOR_FLESH_PC34
    };

    memset(source, DM1_V1_F0107_WALL_ORNAMENT_C10_COLOR_FLESH_PC34,
           sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[0] = 0x34;
    source[1] = DM1_V1_F0107_WALL_ORNAMENT_C10_COLOR_FLESH_PC34;
    source[DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_WIDTH_PC34 + 7] = 0x6a;

    CHECK_TRUE("pixel.d1r.apply",
               dm1_v1_viewport_f0107_wall_ornament_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out),
               "DUNVIEW.C:7627 -> F0107, DUNVIEW.C:3907/3910 C10 blit");
    CHECK_TRUE("pixel.d1r.route_valid", out.route_valid,
               "case 7 is M553/M586 D1R-left route");
    CHECK_TRUE("pixel.d1r.alcove", out.returns_alcove,
               "DUNVIEW.C:3933 returns F0149 alcove boolean");
    CHECK_TRUE("pixel.d1r.draw_attempted", out.draw_attempted,
               "DUNVIEW.C:3907/3910 draw before return");
    CHECK_TRUE("pixel.d1r.in_clip", out.in_clip,
               "synthetic D1R side ornament clip");
    CHECK_INT("pixel.d1r.source_x", out.source_x, 0,
              "synthetic coordinate-set x=0");
    CHECK_INT("pixel.d1r.source_y", out.source_y, 0,
              "synthetic coordinate-set y=0");
    CHECK_TRUE("pixel.d1r.writes", out.writes_pixel,
               "DUNVIEW.C:3907/3910 opaque ornament pixel writes");
    CHECK_INT("pixel.d1r.value", out.pixel_after, 0x34,
              "deterministic ornament source pixel");

    input.viewport_x = 1;
    CHECK_TRUE("pixel.d1r.c10.apply",
               dm1_v1_viewport_f0107_wall_ornament_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out),
               "DEFS.H:2088 C10_COLOR_FLESH");
    CHECK_TRUE("pixel.d1r.c10.skip", out.transparent_skip,
               "DUNVIEW.C:3907/3910 C10 transparent blit");
    CHECK_FALSE("pixel.d1r.c10.no_write", out.writes_pixel,
                "DUNVIEW.C:3907/3910 C10 transparent blit");
    CHECK_INT("pixel.d1r.c10.preserved", out.pixel_after, 0xee,
              "transparent ornament pixel preserves viewport");

    input.row = 1;
    input.viewport_x = 7;
    CHECK_TRUE("pixel.d1r.edge.apply",
               dm1_v1_viewport_f0107_wall_ornament_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out),
               "synthetic coordinate-set right edge");
    CHECK_INT("pixel.d1r.edge.source_offset", (int)out.source_offset,
              DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_WIDTH_PC34 + 7,
              "row-local source offset");
    CHECK_INT("pixel.d1r.edge.value", out.pixel_after, 0x6a,
              "right-edge ornament pixel writes");

    input.viewport_x = 8;
    CHECK_TRUE("pixel.d1r.after_edge.apply",
               dm1_v1_viewport_f0107_wall_ornament_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out),
               "synthetic coordinate-set no-write after edge");
    CHECK_TRUE("pixel.d1r.after_edge.no_write", out.no_write_metadata,
               "clipped ornament metadata");
    CHECK_FALSE("pixel.d1r.after_edge.in_clip", out.in_clip,
                "clipped ornament metadata");

    CHECK_INT("blend.ornament.c10",
              dm1_v1_viewport_f0107_wall_ornament_blend_pixel_pc34(0x44, 10, 10),
              0x44, "DUNVIEW.C:3907/3910 C10 skip");
    CHECK_INT("blend.ornament.opaque",
              dm1_v1_viewport_f0107_wall_ornament_blend_pixel_pc34(0x44, 0x55, 10),
              0x55, "DUNVIEW.C:3907/3910 opaque write");
    CHECK_FALSE("pixel.invalid.null_out",
                dm1_v1_viewport_f0107_wall_ornament_apply_pixel_pc34(
                    &input, source, sizeof(source), viewport, sizeof(viewport), NULL),
                "NULL-safe pixel result");
}

static void test_source_evidence(void)
{
    const char *e =
        dm1_v1_viewport_f0107_wall_ornament_alcove_source_evidence_pc34();

    CHECK_CONTAINS("evidence.f0107", e, "DUNVIEW.C:3502-3938",
                   "F0107 body source-lock");
    CHECK_CONTAINS("evidence.return", e, "line 3933",
                   "DUNVIEW.C:3933 returns L0096_B_IsAlcove");
    CHECK_CONTAINS("evidence.c00_c01", e, "DUNVIEW.C:6263/6330",
                   "DUNVIEW.C:6263/6330 C00/C01 call sites");
    CHECK_CONTAINS("evidence.d2", e, "DUNVIEW.C:6968/7119",
                   "DUNVIEW.C:6968/7119 D2 side call sites");
    CHECK_CONTAINS("evidence.d1", e, "DUNVIEW.C:7459/7627",
                   "DUNVIEW.C:7459/7627 D1 side call sites");
    CHECK_CONTAINS("evidence.defs_slots", e, "DEFS.H:2551-2554",
                   "DEFS.H M551/M552/M553/M554 slots");
    CHECK_CONTAINS("evidence.defs_walls", e, "DEFS.H:2696-2710",
                   "DEFS.H C00/C01/M575-M587 wall cells");
    CHECK_CONTAINS("evidence.draw_direct", e, "DUNVIEW.C:3907",
                   "F0107 direct C10 blit");
    CHECK_CONTAINS("evidence.draw_zone", e, "DUNVIEW.C:3922",
                   "F0107 zone C10 blit");
    CHECK_CONTAINS("evidence.c10", e, "DEFS.H:2088",
                   "C10 transparent ornament pixels");
}

int main(void)
{
    test_table_contract();
    test_cell_code_coordinate_mapping();
    test_side_alcove_contract();
    test_front_and_mirror_reject_contract();
    test_null_safety_and_repeatability();
    test_d1r_side_ornament_pixel_contract();
    test_source_evidence();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_f0107_wall_ornament_alcove_pc34_compat "
               "assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_f0107_wall_ornament_alcove_pc34_compat "
           "assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
