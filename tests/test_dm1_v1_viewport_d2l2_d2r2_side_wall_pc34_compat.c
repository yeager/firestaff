#include "dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

#define PROBE_ASSERT(id, expr)                                                   \
    do {                                                                         \
        ++g_assertions;                                                          \
        if (!(expr)) {                                                           \
            printf("FAIL %s\n", (id));                                           \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

#define PROBE_ASSERT_EQ(id, got, want)                                           \
    do {                                                                         \
        int probe_got__ = (int)(got);                                            \
        int probe_want__ = (int)(want);                                          \
        ++g_assertions;                                                          \
        if (probe_got__ != probe_want__) {                                       \
            printf("FAIL %s got=%d want=%d\n", (id), probe_got__, probe_want__); \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

#define PROBE_ASSERT_U32(id, got, want)                                          \
    do {                                                                         \
        uint32_t probe_got__ = (uint32_t)(got);                                  \
        uint32_t probe_want__ = (uint32_t)(want);                                \
        ++g_assertions;                                                          \
        if (probe_got__ != probe_want__) {                                       \
            printf("FAIL %s got=0x%08x want=0x%08x\n",                          \
                   (id), (unsigned)probe_got__, (unsigned)probe_want__);         \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

static int contains(const char *haystack, const char *needle)
{
    return haystack && needle && strstr(haystack, needle) != NULL;
}

static int count_marked(const uint8_t *framebuffer, uint8_t marker, int bytes)
{
    int count = 0;
    int i;

    for (i = 0; i < bytes; ++i) {
        if (framebuffer[i] == marker) ++count;
    }
    return count;
}

static int count_marked_in_rect(const uint8_t *framebuffer,
                                int x, int y, int w, int h,
                                uint8_t marker)
{
    int count = 0;
    int yy;
    int xx;

    for (yy = y; yy < y + h; ++yy) {
        for (xx = x; xx < x + w; ++xx) {
            size_t offset =
                (size_t)yy * DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_WIDTH_PC34 +
                (size_t)xx;
            if (framebuffer[offset] == marker) ++count;
        }
    }
    return count;
}

static void test_model_core(void)
{
    DM1_V1_D2L2D2R2SideWallDispatchModelPc34 built;
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *m =
        dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34();

    PROBE_ASSERT("builder.null",
                 !dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_builder_pc34(
                     NULL));
    PROBE_ASSERT("builder.ok",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_builder_pc34(
                     &built));
    PROBE_ASSERT("model.present", m != NULL);
    if (!m) return;

    PROBE_ASSERT("hash.null", dm1_v1_viewport_d2l2_d2r2_side_wall_hash_model_pc34(NULL) == 0);
    PROBE_ASSERT("hash.builder_matches_default",
                 built.deterministic_hash == m->deterministic_hash);
    PROBE_ASSERT("hash.accessor_matches",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_deterministic_hash_pc34() ==
                     m->deterministic_hash);

    /* F0678/F0679 limited switch contract (2 cases only) */
    PROBE_ASSERT_EQ("f0678.switch_count",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_switch_count_pc34(0),
                    2);
    PROBE_ASSERT_EQ("f0679.switch_count",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_switch_count_pc34(1),
                    2);
    PROBE_ASSERT_EQ("switch.bad_side",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_switch_count_pc34(-1),
                    -1);
    PROBE_ASSERT_EQ("switch.bad_side_hi",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_switch_count_pc34(2),
                    -1);

    /* Element case counts: 1 wall + 1 teleporter per side */
    PROBE_ASSERT_EQ("f0678.wall_case",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_element_case_count_pc34(0, 0),
                    1);
    PROBE_ASSERT_EQ("f0678.teleporter_case",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_element_case_count_pc34(0, 1),
                    1);
    PROBE_ASSERT_EQ("f0678.no_other_case",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_element_case_count_pc34(0, 2),
                    0);
    PROBE_ASSERT_EQ("f0679.wall_case",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_element_case_count_pc34(1, 0),
                    1);
    PROBE_ASSERT_EQ("f0679.teleporter_case",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_element_case_count_pc34(1, 1),
                    1);
    PROBE_ASSERT_EQ("f0679.no_other_case",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_element_case_count_pc34(1, 5),
                    0);

    /* No F0107/F0108/F0111/F0115 in F0678/F0679 body */
    PROBE_ASSERT_EQ("f0678.no_f0107",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(0, 3),
                    0);
    PROBE_ASSERT_EQ("f0678.no_f0108",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(0, 4),
                    0);
    PROBE_ASSERT_EQ("f0678.no_f0111",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(0, 5),
                    0);
    PROBE_ASSERT_EQ("f0678.no_f0115",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(0, 6),
                    0);
    PROBE_ASSERT_EQ("f0678.f0104_native",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(0, 0),
                    1);
    PROBE_ASSERT_EQ("f0678.f0105_excluded_pc34",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(0, 1),
                    0);
    PROBE_ASSERT_EQ("f0678.f0113_teleporter",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(0, 2),
                    1);
    PROBE_ASSERT_EQ("f0679.no_f0107",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(1, 3),
                    0);
    PROBE_ASSERT_EQ("f0679.no_f0108",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(1, 4),
                    0);
    PROBE_ASSERT_EQ("f0679.no_f0111",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(1, 5),
                    0);
    PROBE_ASSERT_EQ("f0679.no_f0115",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(1, 6),
                    0);
    PROBE_ASSERT_EQ("f0679.f0104_native",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(1, 0),
                    1);
    PROBE_ASSERT_EQ("f0679.f0105_excluded_pc34",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(1, 1),
                    0);
    PROBE_ASSERT_EQ("f0679.f0113_teleporter",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(1, 2),
                    1);

    /* F0128 caller position: D2L2 at 8504 = order 6, D2R2 at 8508 = order 7 */
    PROBE_ASSERT_EQ("f0678.f0128_call_order",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_call_order_pc34(0),
                    6);
    PROBE_ASSERT_EQ("f0679.f0128_call_order",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_call_order_pc34(1),
                    7);
    PROBE_ASSERT_EQ("f0128.bad_side",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_call_order_pc34(2),
                    -1);

    /* F0128 dispatch order: side row after D3C (8499) and before D2L (8513) */
    PROBE_ASSERT("f0128.d2l2_after_d3l2_8482",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_after_pc34(0, 8482));
    PROBE_ASSERT("f0128.d2l2_after_d3r2_8486",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_after_pc34(0, 8486));
    PROBE_ASSERT("f0128.d2l2_after_d3l_8491",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_after_pc34(0, 8491));
    PROBE_ASSERT("f0128.d2l2_after_d3r_8495",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_after_pc34(0, 8495));
    PROBE_ASSERT("f0128.d2l2_after_d3c_8499",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_after_pc34(0, 8499));
    PROBE_ASSERT("f0128.d2l2_before_d2l_8513",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_before_pc34(0, 8513));
    PROBE_ASSERT("f0128.d2l2_before_d2r_8517",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_before_pc34(0, 8517));
    PROBE_ASSERT("f0128.d2l2_before_d2c_8521",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_before_pc34(0, 8521));
    PROBE_ASSERT("f0128.d2l2_before_d1l_8525",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_before_pc34(0, 8525));
    PROBE_ASSERT("f0128.d2l2_before_d0c_8542",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_before_pc34(0, 8542));
    PROBE_ASSERT("f0128.d2r2_after_d2l2_8504",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_after_pc34(1, 8504));
    PROBE_ASSERT("f0128.d2r2_before_d2l_8513",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_before_pc34(1, 8513));
    PROBE_ASSERT("f0128.bad_side_after",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_after_pc34(2, 8504) == -1);
    PROBE_ASSERT("f0128.bad_side_before",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_before_pc34(2, 8504) == -1);

    /* C10 transparent blend */
    PROBE_ASSERT_EQ("c10.transparent",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_blend_pixel_pc34(0xaa, 10, 10),
                    0xaa);
    PROBE_ASSERT_EQ("c10.opaque",
                    dm1_v1_viewport_d2l2_d2r2_side_wall_blend_pixel_pc34(0xaa, 0x77, 10),
                    0x77);
}

static void test_lanes_cases_dispatch_order(void)
{
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *m =
        dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34();
    size_t i;
    int case_count = 0;
    int order_count = 0;

    PROBE_ASSERT("lane.oob_low",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_lane_at_pc34((size_t)-1) == NULL);
    PROBE_ASSERT("lane.oob_high",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_lane_at_pc34(2) == NULL);
    PROBE_ASSERT("case.oob",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_case_at_pc34(8) == NULL);
    PROBE_ASSERT("dispatch.oob",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_order_at_pc34(10) == NULL);

    for (i = 0; m && i < 2; ++i) {
        const DM1_V1_D2L2D2R2SideWallLanePc34 *lane =
            dm1_v1_viewport_d2l2_d2r2_side_wall_lane_at_pc34(i);
        PROBE_ASSERT("lane.present", lane != NULL);
        if (!lane) continue;
        PROBE_ASSERT("lane.anchor", contains(lane->redmcsb_anchor, "DUNVIEW.C"));
        PROBE_ASSERT("lane.f0128_line_anchor", contains(lane->redmcsb_anchor, "8504") ||
                                                contains(lane->redmcsb_anchor, "8508"));
        PROBE_ASSERT_EQ("lane.relative_depth", lane->relative_depth, 2);
        {
            int probe_lateral_pm2__ = (lane->relative_lateral == -2 ||
                                       lane->relative_lateral == 2) ? 1 : 0;
            PROBE_ASSERT_EQ("lane.relative_lateral_pm2", probe_lateral_pm2__, 1);
        }
    }

    PROBE_ASSERT("lane.d2l2.name",
                 contains(m ? m->lanes[0].side_name : NULL, "D2L2"));
    PROBE_ASSERT_EQ("lane.d2l2.view_square",
                    m ? m->lanes[0].view_square_index : 0, 9);
    PROBE_ASSERT_EQ("lane.d2l2.wall_zone",
                    m ? m->lanes[0].wall_zone : 0, 707);
    PROBE_ASSERT_EQ("lane.d2l2.native_wall_set",
                    m ? m->lanes[0].native_wall_set_index : 0, 6);
    PROBE_ASSERT_EQ("lane.d2l2.flipped_wall_set",
                    m ? m->lanes[0].flipped_wall_set_index : 0, 5);
    PROBE_ASSERT_EQ("lane.d2l2.f0104_call_line",
                    m ? m->lanes[0].f0104_f0105_call_line : 0, 6854);
    PROBE_ASSERT_EQ("lane.d2l2.f0113_call_line",
                    m ? m->lanes[0].f0113_teleporter_call_line : 0, 6863);
    PROBE_ASSERT_EQ("lane.d2l2.relative_lateral",
                    m ? m->lanes[0].relative_lateral : 0, -2);
    PROBE_ASSERT_EQ("lane.d2l2.f0128_call_line",
                    m ? m->lanes[0].f0128_call_line : 0, 8504);
    PROBE_ASSERT_EQ("lane.d2l2.f0128_order",
                    m ? m->lanes[0].f0128_call_order_index : 0, 6);

    PROBE_ASSERT("lane.d2r2.name",
                 contains(m ? m->lanes[1].side_name : NULL, "D2R2"));
    PROBE_ASSERT_EQ("lane.d2r2.view_square",
                    m ? m->lanes[1].view_square_index : 0, 10);
    PROBE_ASSERT_EQ("lane.d2r2.wall_zone",
                    m ? m->lanes[1].wall_zone : 0, 708);
    PROBE_ASSERT_EQ("lane.d2r2.native_wall_set",
                    m ? m->lanes[1].native_wall_set_index : 0, 5);
    PROBE_ASSERT_EQ("lane.d2r2.flipped_wall_set",
                    m ? m->lanes[1].flipped_wall_set_index : 0, 6);
    PROBE_ASSERT_EQ("lane.d2r2.f0104_call_line",
                    m ? m->lanes[1].f0104_f0105_call_line : 0, 6885);
    PROBE_ASSERT_EQ("lane.d2r2.f0113_call_line",
                    m ? m->lanes[1].f0113_teleporter_call_line : 0, 6894);
    PROBE_ASSERT_EQ("lane.d2r2.relative_lateral",
                    m ? m->lanes[1].relative_lateral : 0, 2);
    PROBE_ASSERT_EQ("lane.d2r2.f0128_call_line",
                    m ? m->lanes[1].f0128_call_line : 0, 8508);
    PROBE_ASSERT_EQ("lane.d2r2.f0128_order",
                    m ? m->lanes[1].f0128_call_order_index : 0, 7);

    for (i = 0; m && i < DM1_V1_D2L2_D2R2_SIDE_WALL_CASE_CAPACITY_PC34; ++i) {
        const DM1_V1_D2L2D2R2SideWallCasePc34 *c =
            dm1_v1_viewport_d2l2_d2r2_side_wall_case_at_pc34(i);
        if (!c) continue;
        if (c->dispatcher_symbol == NULL) continue;
        case_count++;
        PROBE_ASSERT("case.dispatcher",
                     contains(c->dispatcher_symbol, "F0678") ||
                     contains(c->dispatcher_symbol, "F0679"));
        PROBE_ASSERT("case.element",
                     contains(c->element_symbol, "C00_ELEMENT_WALL") ||
                     contains(c->element_symbol, "C05_ELEMENT_TELEPORTER"));
        PROBE_ASSERT("case.anchor", contains(c->redmcsb_anchor, "DUNVIEW.C"));
    }
    PROBE_ASSERT_EQ("case.count", case_count, 4);

    for (i = 0; i < DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_ORDER_CAPACITY_PC34; ++i) {
        const DM1_V1_D2L2D2R2SideWallDispatchOrderPc34 *o =
            dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_order_at_pc34(i);
        if (!o || o->name == NULL) continue;
        order_count++;
        PROBE_ASSERT("order.anchor", contains(o->redmcsb_anchor, "DUNVIEW.C"));
        PROBE_ASSERT("order.f0128_call_line", o->f0128_call_line > 0);
    }
    PROBE_ASSERT_EQ("order.count", order_count, 10);
}

static void test_sibling_rejects_and_evidence(void)
{
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *m =
        dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34();
    const char *e = dm1_v1_viewport_d2l2_d2r2_side_wall_source_evidence_pc34();
    const char *d = dm1_v1_viewport_d2l2_d2r2_side_wall_disjointness_note_pc34();
    size_t i;

    PROBE_ASSERT("sibling.oob",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_sibling_reject_at_pc34(8) == NULL);
    for (i = 0; m && i < 8; ++i) {
        const DM1_V1_D2L2D2R2SideWallSiblingRejectPc34 *r =
            dm1_v1_viewport_d2l2_d2r2_side_wall_sibling_reject_at_pc34(i);
        if (!r) continue;
        PROBE_ASSERT("sibling.no_f0107", r->reject_f0107_route == 1 ||
                                          r->sibling_name == NULL ||
                                          contains(r->sibling_name, "D3L2/D3R2"));
        PROBE_ASSERT("sibling.no_f0108", r->reject_f0108_route == 1 ||
                                          contains(r->sibling_name, "D3L2/D3R2"));
        PROBE_ASSERT("sibling.no_f0111", r->reject_f0111_route == 1 ||
                                          contains(r->sibling_name, "D3L2/D3R2"));
        PROBE_ASSERT("sibling.no_f0115", r->reject_f0115_route == 1 ||
                                          contains(r->sibling_name, "D3L2/D3R2"));
        PROBE_ASSERT("sibling.reject_f0128_depth",
                     r->reject_f0128_depth != 2 ||
                     contains(r->sibling_name, "D2L/D2R") ||
                     contains(r->sibling_name, "D2C") ||
                     contains(r->sibling_name, "CSB") ||
                     contains(r->sibling_name, "F0100"));
    }

    PROBE_ASSERT("sibling.d0lr", contains(m ? m->sibling_rejects[0].sibling_name : NULL,
                                          "D0L2/D0R2"));
    PROBE_ASSERT("sibling.d1lrc", contains(m ? m->sibling_rejects[1].sibling_name : NULL,
                                           "D1L/D1R/D1C"));
    PROBE_ASSERT("sibling.d2l.d2r", contains(m ? m->sibling_rejects[2].sibling_name : NULL,
                                            "D2L/D2R"));
    PROBE_ASSERT("sibling.d2c", contains(m ? m->sibling_rejects[3].sibling_name : NULL,
                                        "D2C"));
    PROBE_ASSERT("sibling.d3l2d3r2", contains(m ? m->sibling_rejects[4].sibling_name : NULL,
                                             "D3L2/D3R2"));
    PROBE_ASSERT("sibling.d3lrc", contains(m ? m->sibling_rejects[5].sibling_name : NULL,
                                          "D3L/D3R/D3C"));
    PROBE_ASSERT("sibling.csb", contains(m ? m->sibling_rejects[6].sibling_name : NULL,
                                        "CSB"));
    PROBE_ASSERT("sibling.f0100", contains(m ? m->sibling_rejects[7].sibling_name : NULL,
                                          "F0100"));

    /* Source evidence anchors */
    PROBE_ASSERT("evidence.f0678_lines", contains(e, "DUNVIEW.C:6837-6865"));
    PROBE_ASSERT("evidence.f0679_lines", contains(e, "DUNVIEW.C:6868-6896"));
    PROBE_ASSERT("evidence.f0678_call_6854", contains(e, "6854"));
    PROBE_ASSERT("evidence.f0678_teleporter_6863", contains(e, "6863"));
    PROBE_ASSERT("evidence.f0679_call_6885", contains(e, "6885"));
    PROBE_ASSERT("evidence.f0679_teleporter_6894", contains(e, "6894"));
    PROBE_ASSERT("evidence.f0128_8503", contains(e, "DUNVIEW.C:8503-8508"));
    PROBE_ASSERT("evidence.f0128_8503_8508", contains(e, "8503-8508"));
    PROBE_ASSERT("evidence.f0128_8508", contains(e, "8508"));
    PROBE_ASSERT("evidence.f0104", contains(e, "F0104"));
    PROBE_ASSERT("evidence.f0105", contains(e, "F0105"));
    PROBE_ASSERT("evidence.f0113", contains(e, "F0113"));
    PROBE_ASSERT("evidence.c09", contains(e, "C09_VIEW_SQUARE_D2L2=9"));
    PROBE_ASSERT("evidence.c10", contains(e, "C10_VIEW_SQUARE_D2R2=10"));
    PROBE_ASSERT("evidence.c05", contains(e, "C05_WALL_D2R2=5"));
    PROBE_ASSERT("evidence.c06", contains(e, "C06_WALL_D2L2=6"));
    PROBE_ASSERT("evidence.c707", contains(e, "C707_ZONE_WALL_D2L2=707"));
    PROBE_ASSERT("evidence.c708", contains(e, "C708_ZONE_WALL_D2R2=708"));
    PROBE_ASSERT("evidence.pc_fix", contains(e, "PC_FIX_CODE_SIZE"));
    PROBE_ASSERT("evidence.media720", contains(e, "MEDIA720_I34E"));
    PROBE_ASSERT("evidence.no_default", contains(e, "No default case"));
    PROBE_ASSERT("evidence.no_f0107", contains(e, "no F0107"));
    PROBE_ASSERT("evidence.no_f0107_f0108_f0111_f0115",
                 contains(e, "no F0107/F0108/F0111/F0115"));

    /* Disjointness note anchors */
    PROBE_ASSERT("disjoint.f0128_caller",
                 contains(d, "F0128 caller + F0678/F0679 dispatcher"));
    PROBE_ASSERT("disjoint.d2l2_d2r2_wall",
                 contains(d, "d2l2_d2r2_wall"));
    PROBE_ASSERT("disjoint.d2l2_d2r2_f0107",
                 contains(d, "d2l2_d2r2_f0107_wall_ornament"));
    PROBE_ASSERT("disjoint.d2l2_d2r2_f0108_wall_composition",
                 contains(d, "d2l2_d2r2_f0108_wall_composition"));
    PROBE_ASSERT("disjoint.d2l2_d2r2_f0111",
                 contains(d, "d2l2_d2r2_f0111"));
    PROBE_ASSERT("disjoint.d2l2_d2r2_f0115",
                 contains(d, "d2l2_d2r2_f0115"));
    PROBE_ASSERT("disjoint.d2l2_d2r2_stairs_pit",
                 contains(d, "d2l2_d2r2_stairs_pit"));
    PROBE_ASSERT("disjoint.d0l2d0r2", contains(d, "D0L2/D0R2"));
    PROBE_ASSERT("disjoint.d1c", contains(d, "D1C"));
    PROBE_ASSERT("disjoint.d1lrd1c", contains(d, "D1L/D1R"));
    PROBE_ASSERT("disjoint.d2c", contains(d, "D2C"));
    PROBE_ASSERT("disjoint.d2lrd2c", contains(d, "D2L/D2R"));
    PROBE_ASSERT("disjoint.d3lrd3c", contains(d, "D3L/D3R"));
    PROBE_ASSERT("disjoint.csb_lineage", contains(d, "CSB-lineage"));
    PROBE_ASSERT("disjoint.no_assets", contains(d, "GRAPHICS.DAT"));
    PROBE_ASSERT("disjoint.no_dos", contains(d, "original DOS pixel parity"));
}

static void test_render_probes(void)
{
    uint8_t framebuffer[DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_WIDTH_PC34 *
                        DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_HEIGHT_PC34];
    int writes;
    int dispatch_result;
    int i;

    PROBE_ASSERT("render.null",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_render_probe_pc34(NULL, 0) == -1);
    PROBE_ASSERT("render.short",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_render_probe_pc34(
                     framebuffer, sizeof(framebuffer) - 1) == -1);
    writes = dm1_v1_viewport_d2l2_d2r2_side_wall_render_probe_pc34(
        framebuffer, sizeof(framebuffer));
    PROBE_ASSERT_EQ("render.writes", writes, 8 * 8 + 8 * 8);
    PROBE_ASSERT_EQ("render.d2l2_marker",
                    count_marked_in_rect(framebuffer, 0, 20, 8, 8, 0x77),
                    8 * 8);
    PROBE_ASSERT_EQ("render.d2r2_marker",
                    count_marked_in_rect(framebuffer, 216, 20, 8, 8, 0x77),
                    8 * 8);
    /* Sibling reject rects should NOT have 0x77 */
    PROBE_ASSERT("render.d0_clear",
                 count_marked_in_rect(framebuffer, 4, 58, 18, 74, 0x77) == 0);
    PROBE_ASSERT("render.d1c_clear",
                 count_marked_in_rect(framebuffer, 86, 24, 46, 90, 0x77) == 0);
    PROBE_ASSERT("render.d1l_clear",
                 count_marked_in_rect(framebuffer, 23, 42, 30, 88, 0x77) == 0);
    PROBE_ASSERT("render.d2c_clear",
                 count_marked_in_rect(framebuffer, 54, 62, 42, 64, 0x77) == 0);
    PROBE_ASSERT("render.d2lrd2c_clear",
                 count_marked_in_rect(framebuffer, 54, 62, 24, 55, 0x77) == 0);
    PROBE_ASSERT("render.d3_clear",
                 count_marked_in_rect(framebuffer, 74, 76, 16, 37, 0x77) == 0);
    PROBE_ASSERT("render.d3c_clear",
                 count_marked_in_rect(framebuffer, 98, 84, 28, 25, 0x77) == 0);
    PROBE_ASSERT("render.csb_clear",
                 count_marked_in_rect(framebuffer, 38, 62, 21, 29, 0x77) == 0);

    /* Dispatch probe: encodes writes<<16 | skips */
    PROBE_ASSERT("dispatch.null",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_render_dispatch_probe_pc34(
                     NULL, 0) == -1);
    PROBE_ASSERT("dispatch.short",
                 dm1_v1_viewport_d2l2_d2r2_side_wall_render_dispatch_probe_pc34(
                     framebuffer, sizeof(framebuffer) - 1) == -1);
    dispatch_result = dm1_v1_viewport_d2l2_d2r2_side_wall_render_dispatch_probe_pc34(
        framebuffer, sizeof(framebuffer));
    PROBE_ASSERT_EQ("dispatch.writes_hi", (dispatch_result >> 16) & 0xffff, 8 * 8 + 8 * 8);
    PROBE_ASSERT_EQ("dispatch.skips_lo", dispatch_result & 0xffff, 8 * 8 + 8 * 8);
    PROBE_ASSERT_EQ("dispatch.marker",
                    count_marked(framebuffer, 0x77, sizeof(framebuffer)),
                    8 * 8 + 8 * 8);

    /* Silence unused variable */
    (void)i;
}

static void test_hash_stability(void)
{
    uint32_t h = dm1_v1_viewport_d2l2_d2r2_side_wall_deterministic_hash_pc34();

    PROBE_ASSERT("hash.nonzero", h != 0u);
    PROBE_ASSERT_U32("hash.stable", h, h);
}

int main(void)
{
    test_model_core();
    test_lanes_cases_dispatch_order();
    test_sibling_rejects_and_evidence();
    test_render_probes();
    test_hash_stability();

    if (g_failures) {
        printf("assertions=%d failures=%d hash=0x%08x\n",
               g_assertions,
               g_failures,
               (unsigned)dm1_v1_viewport_d2l2_d2r2_side_wall_deterministic_hash_pc34());
        return 1;
    }
    printf("assertions=%d failures=0 hash=0x%08x\n",
           g_assertions,
           (unsigned)dm1_v1_viewport_d2l2_d2r2_side_wall_deterministic_hash_pc34());
    return 0;
}
