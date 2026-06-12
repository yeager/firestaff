#include "firestaff/dm1/v1/viewport/d2c_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

#define EXPECT_TRUE(ID, EXPR, ANCHOR)                                      \
    do {                                                                    \
        ++g_assertions;                                                     \
        if (!(EXPR)) {                                                      \
            printf("FAIL %s anchor=%s\n", (ID), (ANCHOR));                 \
            ++g_failures;                                                   \
        } else {                                                            \
            printf("PASS %s anchor=%s\n", (ID), (ANCHOR));                 \
        }                                                                   \
    } while (0)

#define EXPECT_INT(ID, GOT, WANT, ANCHOR)                                  \
    do {                                                                    \
        int got_ = (GOT);                                                   \
        int want_ = (WANT);                                                 \
        ++g_assertions;                                                     \
        if (got_ != want_) {                                                \
            printf("FAIL %s got=%d want=%d anchor=%s\n",                   \
                   (ID), got_, want_, (ANCHOR));                            \
            ++g_failures;                                                   \
        } else {                                                            \
            printf("PASS %s == %d anchor=%s\n",                            \
                   (ID), want_, (ANCHOR));                                  \
        }                                                                   \
    } while (0)

#define EXPECT_U32(ID, GOT, WANT, ANCHOR)                                  \
    do {                                                                    \
        uint32_t got_ = (GOT);                                              \
        uint32_t want_ = (WANT);                                            \
        ++g_assertions;                                                     \
        if (got_ != want_) {                                                \
            printf("FAIL %s got=0x%08x want=0x%08x anchor=%s\n",           \
                   (ID), (unsigned)got_, (unsigned)want_, (ANCHOR));        \
            ++g_failures;                                                   \
        } else {                                                            \
            printf("PASS %s == 0x%08x anchor=%s\n",                        \
                   (ID), (unsigned)want_, (ANCHOR));                        \
        }                                                                   \
    } while (0)

#define EXPECT_CONTAINS(ID, HAYSTACK, NEEDLE, ANCHOR)                      \
    do {                                                                    \
        const char *haystack_ = (HAYSTACK);                                 \
        const char *needle_ = (NEEDLE);                                     \
        ++g_assertions;                                                     \
        if (!haystack_ || !needle_ || strstr(haystack_, needle_) == NULL) { \
            printf("FAIL %s missing=%s anchor=%s\n",                       \
                   (ID), needle_ ? needle_ : "(null)", (ANCHOR));          \
            ++g_failures;                                                   \
        } else {                                                            \
            printf("PASS %s contains=%s anchor=%s\n",                      \
                   (ID), needle_, (ANCHOR));                                \
        }                                                                   \
    } while (0)

static void expect_rect_inside(
    const char *prefix,
    const DM1_V1_D2CF0108RectPc34 *rect)
{
    char id[96];

    snprintf(id, sizeof(id), "%s.x1", prefix);
    EXPECT_TRUE(id, rect && rect->x1 >= 0, "224x136 viewport left bound");
    snprintf(id, sizeof(id), "%s.y1", prefix);
    EXPECT_TRUE(id, rect && rect->y1 >= 0, "224x136 viewport top bound");
    snprintf(id, sizeof(id), "%s.x2", prefix);
    EXPECT_TRUE(id, rect && rect->x2 < DM1_V1_D2C_F0108_VIEWPORT_WIDTH_PC34,
                "224x136 viewport right bound");
    snprintf(id, sizeof(id), "%s.y2", prefix);
    EXPECT_TRUE(id, rect && rect->y2 < DM1_V1_D2C_F0108_VIEWPORT_HEIGHT_PC34,
                "224x136 viewport bottom bound");
    snprintf(id, sizeof(id), "%s.ordered", prefix);
    EXPECT_TRUE(id, rect && rect->x1 <= rect->x2 && rect->y1 <= rect->y2,
                "non-empty viewport rect");
}

static void test_model_core(void)
{
    DM1_V1_D2CF0108ModelPc34 built;
    const DM1_V1_D2CF0108ModelPc34 *model =
        dm1_v1_viewport_d2c_f0108_model_pc34();

    EXPECT_INT("builder.null",
               dm1_v1_viewport_d2c_f0108_model_build_pc34(NULL), 0,
               "builder guard");
    EXPECT_INT("builder.ok",
               dm1_v1_viewport_d2c_f0108_model_build_pc34(&built), 1,
               "builder deterministic");
    EXPECT_TRUE("model.present", model != NULL, "model accessor");
    if (!model) return;

    EXPECT_U32("hash.builder", built.deterministic_hash,
               model->deterministic_hash, "model hash stable");
    EXPECT_U32("hash.accessor",
               dm1_v1_viewport_d2c_f0108_deterministic_hash_pc34(),
               model->deterministic_hash, "hash accessor stable");
    EXPECT_U32("hash.null", dm1_v1_viewport_d2c_f0108_hash_model_pc34(NULL),
               0u, "hash guard");

    EXPECT_INT("view_square.d2c", model->view_square_d2c, 6,
               "DEFS.H:2602 M603_VIEW_SQUARE_D2C");
    EXPECT_INT("view_floor.d2c", model->view_floor_d2c, 6,
               "DEFS.H:2756 M592_VIEW_FLOOR_D2C");
    EXPECT_INT("slot.first_thing", model->first_thing_slot, 2,
               "DEFS.H:2549 M550_FIRST_THING");
    EXPECT_INT("slot.floor_ornament", model->floor_ornament_slot, 5,
               "DEFS.H:2558 M558_FLOOR_ORNAMENT_ORDINAL");
    EXPECT_INT("zone.wall_d2c", model->wall_zone_d2c, 709,
               "DEFS.H:4049 C709_ZONE_WALL_D2C");
    EXPECT_INT("zone.wall_d3l", model->sibling_wall_zone_d3l, 705,
               "DEFS.H:4045 C705_ZONE_WALL_D3L");
    EXPECT_INT("zone.wall_d3r", model->sibling_wall_zone_d3r, 706,
               "DEFS.H:4046 C706_ZONE_WALL_D3R");
    EXPECT_INT("zone.ceiling_d2c", model->ceiling_zone_d2c_pc34, 865,
               "DEFS.H:4212 C865_ZONE_CEILING_PIT_D2C");
    EXPECT_INT("color.c10", model->color_flesh, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    EXPECT_INT("zone.floor_base", model->floor_zone_base, 1500,
               "DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT");
    EXPECT_INT("line.f0108.start", model->f0108_start_line, 3940,
               "DUNVIEW.C F0108");
    EXPECT_INT("line.f0108.end", model->f0108_end_line, 4011,
               "DUNVIEW.C F0108");
    EXPECT_INT("line.f0121.start", model->f0121_start_line, 7244,
               "DUNVIEW.C F0121");
    EXPECT_INT("line.f0121.end", model->f0121_end_line, 7388,
               "DUNVIEW.C F0121");
    EXPECT_INT("line.f0128.update", model->f0128_d2c_update_line, 8520,
               "DUNVIEW.C:8520");
    EXPECT_INT("line.f0128.draw", model->f0128_d2c_draw_line, 8521,
               "DUNVIEW.C:8521");
    EXPECT_INT("line.f0172.sensor", model->f0172_sensor_line, 2676,
               "DUNGEON.C:2676");
    EXPECT_INT("line.f0172.first_thing", model->f0172_first_thing_line, 2721,
               "DUNGEON.C:2721");
    EXPECT_INT("zone.f0108",
               dm1_v1_viewport_d2c_f0108_floor_zone_pc34(2, 6), 1528,
               "F0108 C1500 + set * 11 + viewFloor");
    EXPECT_INT("zone.f0107",
               dm1_v1_viewport_d2c_f0108_f0107_wall_zone_pc34(2, 10), 1044,
               "F0107 C1004 + set * 15 + viewWall");
    EXPECT_INT("zone.distinct", model->zone_math.source_locked_distinct, 1,
               "F0108 floor math distinct from F0107 wall math");

    expect_rect_inside("rect.viewport", &model->viewport);
    expect_rect_inside("rect.ceiling", &model->ceiling_rect);
    expect_rect_inside("rect.floor", &model->floor_rect);
    expect_rect_inside("rect.ornament", &model->ornament_rect);
    expect_rect_inside("rect.thing", &model->thing_rect);
    expect_rect_inside("rect.field", &model->field_rect);
}

static void test_evidence_strings(void)
{
    const char *e = dm1_v1_viewport_d2c_f0108_source_evidence_pc34();
    const char *d = dm1_v1_viewport_d2c_f0108_disjointness_note_pc34();

    EXPECT_CONTAINS("evidence.f0108", e, "DUNVIEW.C F0108:3940-4011",
                    "required F0108 source anchor");
    EXPECT_CONTAINS("evidence.f0121", e, "DUNVIEW.C F0121:7244-7388",
                    "required D2C body anchor");
    EXPECT_CONTAINS("evidence.f0128", e, "DUNVIEW.C F0128:8511-8521",
                    "required D2C dispatch anchor");
    EXPECT_CONTAINS("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938",
                    "required F0107 contrast anchor");
    EXPECT_CONTAINS("evidence.f0115", e, "DUNVIEW.C F0115:4547-4581",
                    "required F0115 anchor");
    EXPECT_CONTAINS("evidence.l0175", e, "L0175_i_DoorFrontViewDrawingPass",
                    "F0115 two-pass door ordering note");
    EXPECT_CONTAINS("evidence.f0163", e, "DUNGEON.C F0163:1769-1838",
                    "list append anchor");
    EXPECT_CONTAINS("evidence.f0164", e, "F0164:1840-1905",
                    "list walk/unlink anchor");
    EXPECT_CONTAINS("evidence.f0172", e, "F0172:2466-2523",
                    "square aspect anchor");
    EXPECT_CONTAINS("evidence.sensor", e, "2666-2721",
                    "sensor + first thing anchor");
    EXPECT_CONTAINS("evidence.c10", e, "DEFS.H:2088",
                    "C10 transparency source anchor");
    EXPECT_CONTAINS("evidence.m550", e, "M550..M558",
                    "aspect ordinal source anchor");
    EXPECT_CONTAINS("evidence.c705", e, "C705/C706/C709",
                    "wall-zone source anchor");
    EXPECT_CONTAINS("evidence.c1500", e, "C1500",
                    "floor-zone source anchor");
    EXPECT_CONTAINS("evidence.7357", e, "F0108 at 7357",
                    "open-route F0108 line");
    EXPECT_CONTAINS("evidence.7314", e, "F0108 at 7314",
                    "door-front F0108 line");
    EXPECT_CONTAINS("evidence.7377", e, "7377-7386 after F0115",
                    "F0113 after F0115 ordering");
    EXPECT_CONTAINS("disjoint.d2c", d, "D2C F0108",
                    "target lane note");
    EXPECT_CONTAINS("disjoint.f0107", d, "D2C F0107 wall-ornament",
                    "F0107 non-duplication");
    EXPECT_CONTAINS("disjoint.siblings", d, "D0L/D0R and D3L/D3R",
                    "sibling non-duplication");
    EXPECT_CONTAINS("disjoint.f0111", d, "F0111 door transparency",
                    "F0111 non-duplication");
    EXPECT_CONTAINS("disjoint.f0115", d, "F0115 thing-pass pixel parity",
                    "F0115 non-duplication");
    EXPECT_CONTAINS("disjoint.no_assets", d, "real-asset parity",
                    "asset-free contract");
}

static void test_event_table(void)
{
    size_t i;
    int wall_events = 0;
    int door_events = 0;
    int open_events = 0;
    int teleporter_events = 0;

    EXPECT_TRUE("event.bounds",
                dm1_v1_viewport_d2c_f0108_event_at_pc34(
                    DM1_V1_D2C_F0108_EVENT_COUNT_PC34) == NULL,
                "event accessor bounds");
    for (i = 0; i < DM1_V1_D2C_F0108_EVENT_COUNT_PC34; ++i) {
        const DM1_V1_D2CF0108EventPc34 *event =
            dm1_v1_viewport_d2c_f0108_event_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "event.%u.present", (unsigned)i);
        EXPECT_TRUE(id, event != NULL, "event accessor");
        if (!event) continue;
        snprintf(id, sizeof(id), "event.%u.order", (unsigned)i);
        EXPECT_INT(id, event->order_index, (int)i, "event order");
        snprintf(id, sizeof(id), "event.%u.kind", (unsigned)i);
        EXPECT_INT(id, (int)event->kind, (int)i, "event kind order");
        snprintf(id, sizeof(id), "event.%u.anchor", (unsigned)i);
        EXPECT_TRUE(id,
                    strstr(event->redmcsb_anchor, "DUNVIEW.C") != NULL ||
                    strstr(event->redmcsb_anchor, "contract-only") != NULL,
                    "event ReDMCSB anchor");
        snprintf(id, sizeof(id), "event.%u.name", (unsigned)i);
        EXPECT_TRUE(id, event->name != NULL && event->name[0] != '\0',
                    "event name");
        wall_events += event->expected_for_wall;
        door_events += event->expected_for_door_front;
        open_events += event->expected_for_open_route;
        teleporter_events += event->expected_for_teleporter;
    }
    EXPECT_INT("events.wall_count", wall_events, 4,
               "wall branch F0107 keep-out plus common dispatch/probe");
    EXPECT_INT("events.door_count", door_events, 7,
               "door-front F0108/F0115/F0111/F0115 ordering");
    EXPECT_INT("events.open_count", open_events, 6,
               "corridor/pit/stairs F0108/F0112/F0115 ordering");
    EXPECT_INT("events.teleporter_count", teleporter_events, 7,
               "teleporter adds F0113 after F0115");
}

static void test_cell_orders(void)
{
    size_t i;

    EXPECT_TRUE("cell_order.bounds",
                dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(
                    DM1_V1_D2C_F0108_CELL_ORDER_COUNT_PC34) == NULL,
                "cell-order accessor bounds");
    for (i = 0; i < DM1_V1_D2C_F0108_CELL_ORDER_COUNT_PC34; ++i) {
        const DM1_V1_D2CF0108CellOrderPc34 *order =
            dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(i);
        char id[96];
        int j;
        snprintf(id, sizeof(id), "order.%u.present", (unsigned)i);
        EXPECT_TRUE(id, order != NULL, "cell-order accessor");
        if (!order) continue;
        snprintf(id, sizeof(id), "order.%u.value_nonzero", (unsigned)i);
        EXPECT_TRUE(id, order->order_value != 0, "DEFS.H cell order");
        snprintf(id, sizeof(id), "order.%u.decoded_count", (unsigned)i);
        EXPECT_TRUE(id, order->decoded_count == 3 || order->decoded_count == 4,
                    "F0115 nibble walk");
        snprintf(id, sizeof(id), "order.%u.anchor", (unsigned)i);
        EXPECT_CONTAINS(id, order->redmcsb_anchor, "F0115",
                        "F0115 ordered-cell anchor");
        for (j = 0; j < order->decoded_count; ++j) {
            snprintf(id, sizeof(id), "order.%u.cell.%d", (unsigned)i, j);
            EXPECT_TRUE(id, order->decoded_cells[j] > 0,
                        "non-zero decoded nibble before terminator");
        }
    }
    EXPECT_INT("order.pass1.value",
               dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(0)->order_value,
               0x0218, "DEFS.H:2669");
    EXPECT_INT("order.pass1.door_pass",
               dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(0)->door_front_pass,
               1, "DUNVIEW.C F0115:4795-4800");
    EXPECT_INT("order.pass2.value",
               dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(1)->order_value,
               0x0349, "DEFS.H:2672");
    EXPECT_INT("order.pass2.door_pass",
               dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(1)->door_front_pass,
               2, "DUNVIEW.C F0115:4795-4800");
    EXPECT_INT("order.open.value",
               dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(2)->order_value,
               0x3421, "DEFS.H:2676");
    EXPECT_INT("order.open.door_pass",
               dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(2)->door_front_pass,
               0, "plain F0115 nibble walk");
}

static void check_ordinal(
    unsigned int ordinal,
    int floor_flipped,
    int primary_draws,
    int primary_index,
    int footprints,
    int footprint_index,
    int flipped)
{
    DM1_V1_D2CF0108OrdinalPc34 decoded;
    char id[96];

    EXPECT_INT("ordinal.decode.ok",
               dm1_v1_viewport_d2c_f0108_decode_ordinal_pc34(
                   ordinal, floor_flipped, &decoded),
               1, "DUNVIEW.C F0108:3959-4008");
    snprintf(id, sizeof(id), "ordinal.0x%04x.has", ordinal);
    EXPECT_INT(id, decoded.has_input_ordinal, ordinal != 0u,
               "F0108 nonzero ordinal gate");
    snprintf(id, sizeof(id), "ordinal.0x%04x.flag", ordinal);
    EXPECT_INT(id, decoded.footprint_flag_set, footprints,
               "MASK0x8000_FOOTPRINTS");
    snprintf(id, sizeof(id), "ordinal.0x%04x.primary", ordinal);
    EXPECT_INT(id, decoded.primary_draws, primary_draws,
               "F0108 cleared ordinal primary draw");
    snprintf(id, sizeof(id), "ordinal.0x%04x.primary_index", ordinal);
    EXPECT_INT(id, decoded.primary_index, primary_index,
               "F0108 pre-decrement index");
    snprintf(id, sizeof(id), "ordinal.0x%04x.footprint", ordinal);
    EXPECT_INT(id, decoded.recursive_footprints_draw, footprints,
               "F0108 recursive footprint draw");
    snprintf(id, sizeof(id), "ordinal.0x%04x.footprint_index", ordinal);
    EXPECT_INT(id, decoded.recursive_footprints_index, footprint_index,
               "C15_FLOOR_ORNAMENT_FOOTPRINTS");
    snprintf(id, sizeof(id), "ordinal.0x%04x.flip", ordinal);
    EXPECT_INT(id, decoded.flips_on_d2c_when_floor_is_flipped, flipped,
               "M592 D2C center-footprint flip branch");
    snprintf(id, sizeof(id), "ordinal.0x%04x.blit_count", ordinal);
    EXPECT_INT(id, decoded.metadata_blit_count,
               (primary_draws ? 1 : 0) + (footprints ? 1 : 0),
               "metadata blit count");
}

static void test_ordinal_decode_and_c10(void)
{
    const DM1_V1_D2CF0108ModelPc34 *model =
        dm1_v1_viewport_d2c_f0108_model_pc34();
    size_t i;
    int c10_skips = 0;

    EXPECT_INT("ordinal.null",
               dm1_v1_viewport_d2c_f0108_decode_ordinal_pc34(1u, 0, NULL),
               0, "decode guard");
    check_ordinal(0u, 0, 0, -1, 0, -1, 0);
    check_ordinal(1u, 0, 1, 0, 0, -1, 0);
    check_ordinal(3u, 0, 1, 2, 0, -1, 0);
    check_ordinal(0x8000u, 1, 0, -1, 1, 15, 1);
    check_ordinal(0x8003u, 1, 1, 2, 1, 15, 1);
    check_ordinal(0x8003u, 0, 1, 2, 1, 15, 0);

    EXPECT_INT("blend.c10",
               dm1_v1_viewport_d2c_f0108_blend_c10_pc34(0xaau, 10u),
               0xaa, "DEFS.H:2088 C10 transparency");
    EXPECT_INT("blend.opaque",
               dm1_v1_viewport_d2c_f0108_blend_c10_pc34(0xaau, 0x4bu),
               0x4b, "opaque F0108 pixel writes");
    for (i = 0; model && i < DM1_V1_D2C_F0108_SAMPLE_COUNT_PC34; ++i) {
        const DM1_V1_D2CF0108PixelSamplePc34 *sample = &model->samples[i];
        char id[96];
        snprintf(id, sizeof(id), "sample.%u.after", (unsigned)i);
        EXPECT_INT(id, sample->after,
                   dm1_v1_viewport_d2c_f0108_blend_c10_pc34(
                       sample->before, sample->source),
                   "F0108 C10 blend sample");
        snprintf(id, sizeof(id), "sample.%u.xor", (unsigned)i);
        EXPECT_INT(id,
                   sample->transparent_skip +
                   (sample->source != DM1_V1_D2C_F0108_C10_COLOR_FLESH_PC34),
                   1, "skip/write exclusive");
        c10_skips += sample->transparent_skip;
    }
    EXPECT_INT("sample.c10_skip_count", c10_skips, 3,
               "three synthetic C10 samples");
}

static void check_compose(
    DM1_V1_D2CF0108ContextPc34 context,
    int want_f0108,
    int want_f0112,
    int want_f0115,
    int want_f0111,
    int want_f0113,
    int want_f0107)
{
    DM1_V1_D2CF0108StatePc34 state;
    DM1_V1_D2CF0108ResultPc34 result;
    char id[128];

    EXPECT_INT("state.init",
               dm1_v1_viewport_d2c_f0108_initial_state_pc34(context, &state),
               1, "supported D2C context");
    state.floor_ornament_ordinal = 0x8003u;
    EXPECT_INT("compose.okcall",
               dm1_v1_viewport_d2c_f0108_compose_pc34(&state, &result),
               1, "compose call");
    snprintf(id, sizeof(id), "compose.%d.ok", (int)context);
    EXPECT_INT(id, result.ok, 1, "contract state accepted");
    snprintf(id, sizeof(id), "compose.%d.fbw", (int)context);
    EXPECT_INT(id, result.framebuffer_width, 320, "320x200 framebuffer");
    snprintf(id, sizeof(id), "compose.%d.fbh", (int)context);
    EXPECT_INT(id, result.framebuffer_height, 200, "320x200 framebuffer");
    snprintf(id, sizeof(id), "compose.%d.vpw", (int)context);
    EXPECT_INT(id, result.viewport_width, 224, "224x136 viewport");
    snprintf(id, sizeof(id), "compose.%d.vph", (int)context);
    EXPECT_INT(id, result.viewport_height, 136, "224x136 viewport");
    snprintf(id, sizeof(id), "compose.%d.f0128.d2_before_d2c", (int)context);
    EXPECT_INT(id, result.f0128_d2l_d2r_before_d2c, 1,
               "DUNVIEW.C:8511-8521 D2 pair before D2C");
    snprintf(id, sizeof(id), "compose.%d.f0128.d2c_before_d1", (int)context);
    EXPECT_INT(id, result.f0128_d2c_before_d1_d0, 1,
               "DUNVIEW.C:8521 before D1/D0");
    snprintf(id, sizeof(id), "compose.%d.f0108", (int)context);
    EXPECT_INT(id, result.f0108_calls, want_f0108, "DUNVIEW.C:7314/7357");
    snprintf(id, sizeof(id), "compose.%d.f0112", (int)context);
    EXPECT_INT(id, result.f0112_calls, want_f0112, "DUNVIEW.C:7359-7365");
    snprintf(id, sizeof(id), "compose.%d.f0115", (int)context);
    EXPECT_INT(id, result.f0115_calls, want_f0115, "DUNVIEW.C:7315/7368");
    snprintf(id, sizeof(id), "compose.%d.f0111", (int)context);
    EXPECT_INT(id, result.f0111_calls, want_f0111, "DUNVIEW.C:7336-7339");
    snprintf(id, sizeof(id), "compose.%d.f0113", (int)context);
    EXPECT_INT(id, result.f0113_calls, want_f0113, "DUNVIEW.C:7377-7386");
    snprintf(id, sizeof(id), "compose.%d.f0107", (int)context);
    EXPECT_INT(id, result.wall_f0107_calls, want_f0107, "DUNVIEW.C:7308");
    snprintf(id, sizeof(id), "compose.%d.terminal_depth", (int)context);
    EXPECT_INT(id, result.terminal_depth_side_pair_correction, 1,
               "D2C is after the D2L/D2R side pair");
    snprintf(id, sizeof(id), "compose.%d.thing_guard", (int)context);
    EXPECT_INT(id, result.thing_list_mutation_guard_ok, 1,
               "DUNGEON.C F0163/F0164 not mutated");
    snprintf(id, sizeof(id), "compose.%d.non_overlap", (int)context);
    EXPECT_INT(id, result.non_overlap_ok, 1, "disjoint route guard");
    snprintf(id, sizeof(id), "compose.%d.zone.f0108", (int)context);
    EXPECT_INT(id, result.floor_zone, want_f0108 ? 1528 : 0,
               "F0108 C1500 zone math");
    snprintf(id, sizeof(id), "compose.%d.zone.f0107", (int)context);
    EXPECT_INT(id, result.f0107_contrast_zone, 1044,
               "F0107 contrast zone math");
    snprintf(id, sizeof(id), "compose.%d.hash", (int)context);
    EXPECT_TRUE(id, result.framebuffer_hash != 0u, "framebuffer hash");
}

static void test_compose_contexts(void)
{
    check_compose(DM1_V1_D2C_F0108_CONTEXT_WALL_PC34,
                  0, 0, 0, 0, 0, 1);
    check_compose(DM1_V1_D2C_F0108_CONTEXT_DOOR_FRONT_PC34,
                  1, 0, 2, 1, 0, 0);
    check_compose(DM1_V1_D2C_F0108_CONTEXT_CORRIDOR_PC34,
                  1, 1, 1, 0, 0, 0);
    check_compose(DM1_V1_D2C_F0108_CONTEXT_OPEN_PIT_PC34,
                  1, 1, 1, 0, 0, 0);
    check_compose(DM1_V1_D2C_F0108_CONTEXT_TELEPORTER_PC34,
                  1, 1, 1, 0, 1, 0);
    check_compose(DM1_V1_D2C_F0108_CONTEXT_STAIRS_FRONT_PC34,
                  1, 1, 1, 0, 0, 0);
}

static void test_specific_ordering_and_rejections(void)
{
    DM1_V1_D2CF0108StatePc34 state;
    DM1_V1_D2CF0108ResultPc34 result;

    (void)dm1_v1_viewport_d2c_f0108_initial_state_pc34(
        DM1_V1_D2C_F0108_CONTEXT_DOOR_FRONT_PC34, &state);
    EXPECT_INT("door.compose",
               dm1_v1_viewport_d2c_f0108_compose_pc34(&state, &result),
               1, "door compose");
    EXPECT_INT("door.f0108_before_pass1", result.door_f0108_before_f0115_pass1,
               1, "DUNVIEW.C:7314 before 7315");
    EXPECT_INT("door.pass1_before_f0111", result.door_pass1_before_f0111,
               1, "F0115 pass 1 before F0111");
    EXPECT_INT("door.pass2_after_f0111", result.door_pass2_after_f0111,
               1, "F0115 pass 2 after F0111");

    (void)dm1_v1_viewport_d2c_f0108_initial_state_pc34(
        DM1_V1_D2C_F0108_CONTEXT_OPEN_PIT_PC34, &state);
    EXPECT_INT("pit.compose",
               dm1_v1_viewport_d2c_f0108_compose_pc34(&state, &result),
               1, "pit compose");
    EXPECT_INT("pit.floor_over_open_pit",
               result.open_pit_still_draws_floor_ornament, 1,
               "BUG0_64 DUNVIEW.C:7357");
    EXPECT_INT("pit.f0112_before_f0115", result.open_route_f0112_before_f0115,
               1, "DUNVIEW.C:7359-7368");

    (void)dm1_v1_viewport_d2c_f0108_initial_state_pc34(
        DM1_V1_D2C_F0108_CONTEXT_TELEPORTER_PC34, &state);
    EXPECT_INT("teleporter.compose",
               dm1_v1_viewport_d2c_f0108_compose_pc34(&state, &result),
               1, "teleporter compose");
    EXPECT_INT("teleporter.f0113_after_f0115",
               result.teleporter_f0113_after_f0115, 1,
               "DUNVIEW.C:7377-7386 after 7368");

    (void)dm1_v1_viewport_d2c_f0108_initial_state_pc34(
        DM1_V1_D2C_F0108_CONTEXT_CORRIDOR_PC34, &state);
    state.mutate_thing_list = true;
    EXPECT_INT("reject.mutate.compose",
               dm1_v1_viewport_d2c_f0108_compose_pc34(&state, &result),
               1, "mutation guard call");
    EXPECT_INT("reject.mutate.flag", result.rejected_non_contract_state, 1,
               "DUNGEON.C F0163/F0164 mutation excluded");
    EXPECT_INT("reject.mutate.ok", result.ok, 0, "contract-only guard");

    state.mutate_thing_list = false;
    state.allow_outside_viewport = true;
    EXPECT_INT("reject.viewport.compose",
               dm1_v1_viewport_d2c_f0108_compose_pc34(&state, &result),
               1, "viewport guard call");
    EXPECT_INT("reject.viewport.flag", result.rejected_non_contract_state, 1,
               "224x136 viewport only");
    EXPECT_INT("reject.viewport.ok", result.ok, 0, "contract-only guard");

    state.allow_outside_viewport = false;
    state.allow_f0107_wall_duplicate = true;
    EXPECT_INT("reject.f0107.compose",
               dm1_v1_viewport_d2c_f0108_compose_pc34(&state, &result),
               1, "F0107 duplication guard call");
    EXPECT_INT("reject.f0107.flag", result.rejected_non_contract_state, 1,
               "D2C F0107 wall-ornament excluded");
    EXPECT_INT("reject.f0107.ok", result.ok, 0, "contract-only guard");

    state.allow_f0107_wall_duplicate = false;
    state.allow_f0111_only_route = true;
    EXPECT_INT("reject.f0111.compose",
               dm1_v1_viewport_d2c_f0108_compose_pc34(&state, &result),
               1, "F0111 duplication guard call");
    EXPECT_INT("reject.f0111.flag", result.rejected_non_contract_state, 1,
               "F0111-only route excluded");
    EXPECT_INT("reject.f0111.ok", result.ok, 0, "contract-only guard");
}

int main(void)
{
    uint32_t hash;

    test_model_core();
    test_evidence_strings();
    test_event_table();
    test_cell_orders();
    test_ordinal_decode_and_c10();
    test_compose_contexts();
    test_specific_ordering_and_rejections();

    hash = dm1_v1_viewport_d2c_f0108_deterministic_hash_pc34();
    EXPECT_U32("hash.stable", hash, 0x3a2502a1u,
               "deterministic model hash");
    printf("assertions=%d failures=%d hash=0x%08x\n",
           g_assertions, g_failures, (unsigned)hash);
    return g_failures ? 1 : 0;
}
