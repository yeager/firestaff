#include "firestaff/dm1/v1/viewport/d3c_f0107_wall_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

#define PROBE_ASSERT(ID, EXPR, ANCHOR)                                      \
    do {                                                                     \
        ++g_assertions;                                                      \
        if (!(EXPR)) {                                                       \
            printf("FAIL %s anchor=%s\n", (ID), (ANCHOR));                  \
            ++g_failures;                                                    \
        } else {                                                             \
            printf("PASS %s anchor=%s\n", (ID), (ANCHOR));                  \
        }                                                                    \
    } while (0)

#define PROBE_ASSERT_INT(ID, GOT, WANT, ANCHOR)                             \
    do {                                                                     \
        int probe_got_ = (GOT);                                              \
        int probe_want_ = (WANT);                                            \
        ++g_assertions;                                                      \
        if (probe_got_ != probe_want_) {                                     \
            printf("FAIL %s got=%d want=%d anchor=%s\n",                    \
                   (ID), probe_got_, probe_want_, (ANCHOR));                \
            ++g_failures;                                                    \
        } else {                                                             \
            printf("PASS %s == %d anchor=%s\n",                            \
                   (ID), probe_want_, (ANCHOR));                            \
        }                                                                    \
    } while (0)

#define PROBE_ASSERT_U32(ID, GOT, WANT, ANCHOR)                             \
    do {                                                                     \
        uint32_t probe_got_ = (GOT);                                         \
        uint32_t probe_want_ = (WANT);                                       \
        ++g_assertions;                                                      \
        if (probe_got_ != probe_want_) {                                     \
            printf("FAIL %s got=0x%08x want=0x%08x anchor=%s\n",            \
                   (ID), (unsigned)probe_got_, (unsigned)probe_want_,        \
                   (ANCHOR));                                                \
            ++g_failures;                                                    \
        } else {                                                             \
            printf("PASS %s == 0x%08x anchor=%s\n",                        \
                   (ID), (unsigned)probe_want_, (ANCHOR));                  \
        }                                                                    \
    } while (0)

#define PROBE_ASSERT_CONTAINS(ID, HAYSTACK, NEEDLE, ANCHOR)                 \
    do {                                                                     \
        const char *probe_haystack_ = (HAYSTACK);                            \
        const char *probe_needle_ = (NEEDLE);                                \
        ++g_assertions;                                                      \
        if (!probe_haystack_ || !probe_needle_ ||                            \
            strstr(probe_haystack_, probe_needle_) == NULL) {                \
            printf("FAIL %s missing=%s anchor=%s\n",                        \
                   (ID), probe_needle_ ? probe_needle_ : "(null)",          \
                   (ANCHOR));                                                \
            ++g_failures;                                                    \
        } else {                                                             \
            printf("PASS %s contains=%s anchor=%s\n",                       \
                   (ID), probe_needle_, (ANCHOR));                          \
        }                                                                    \
    } while (0)

static int count_changed_pixels_in_box(
    const uint8_t *framebuffer,
    const DM1_V1_D3CF0107RejectedContractPc34 *box)
{
    int x;
    int y;
    int changed = 0;

    if (!framebuffer || !box) return -1;
    for (y = box->y; y < box->y + box->height; ++y) {
        for (x = box->x; x < box->x + box->width; ++x) {
            size_t offset =
                (size_t)y * (size_t)DM1_V1_D3C_F0107_FRAMEBUFFER_WIDTH_PC34 +
                (size_t)x;
            changed += framebuffer[offset] != 0xeeu;
        }
    }
    return changed;
}

static void test_core_model(void)
{
    DM1_V1_D3CF0107WallOrnamentModelPc34 built;
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();

    PROBE_ASSERT_INT("builder.null",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_builder_pc34(NULL),
                     0, "builder guard");
    PROBE_ASSERT_INT("builder.ok",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_builder_pc34(&built),
                     1, "builder deterministic");
    PROBE_ASSERT("model.present", model != NULL, "model accessor");
    if (!model) return;

    PROBE_ASSERT_INT("hash.null",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_hash_model_pc34(NULL),
                     0, "hash guard");
    PROBE_ASSERT_U32("hash.builder", built.deterministic_hash,
                     model->deterministic_hash, "builder hash stable");
    PROBE_ASSERT_U32("hash.accessor",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_deterministic_hash_pc34(),
                     model->deterministic_hash, "hash accessor stable");

    PROBE_ASSERT_INT("framebuffer.width", model->framebuffer_width, 320,
                     "320x200 framebuffer contract");
    PROBE_ASSERT_INT("framebuffer.height", model->framebuffer_height, 200,
                     "320x200 framebuffer contract");
    PROBE_ASSERT_INT("viewport.width", model->viewport_width, 224,
                     "224x136 viewport contract");
    PROBE_ASSERT_INT("viewport.height", model->viewport_height, 136,
                     "224x136 viewport contract");
    PROBE_ASSERT_INT("viewport.x_last", model->viewport_x_last, 223,
                     "224x136 viewport bounds");
    PROBE_ASSERT_INT("viewport.y_last", model->viewport_y_last, 135,
                     "224x136 viewport bounds");
    PROBE_ASSERT_INT("view_square.d3c", model->view_square_d3c, 11,
                     "DEFS.H:2607 M600_VIEW_SQUARE_D3C");
    PROBE_ASSERT_INT("relative.depth", model->relative_depth, 3,
                     "DUNVIEW.C:8498 F0128 relative depth");
    PROBE_ASSERT_INT("relative.lateral", model->relative_lateral, 0,
                     "DUNVIEW.C:8498 F0128 relative lateral");
    PROBE_ASSERT_INT("position.c", model->c_coordinate, 0,
                     "D3C center-front c == 0");
    PROBE_ASSERT_INT("position.y", model->y_coordinate, 0,
                     "D3C center-front y == 0");
    PROBE_ASSERT_INT("view_wall.d3c_front", model->view_wall_d3c_front, 5,
                     "DEFS.H:2701 M578_VIEW_WALL_D3C_FRONT");
    PROBE_ASSERT_INT("wall_zone.d3c", model->wall_zone_d3c, 704,
                     "DEFS.H:4044 C704_ZONE_WALL_D3C");
    PROBE_ASSERT_INT("wall_index.d3c", model->wall_index_d3c, 14,
                     "DEFS.H:3437 C14_WALL_D3C");
    PROBE_ASSERT_INT("floor_view.d3c", model->floor_view_d3c, 3,
                     "DEFS.H:2753 M589_VIEW_FLOOR_D3C");
    PROBE_ASSERT_INT("slot.m552", model->front_wall_ornament_slot, 5,
                     "DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL");
    PROBE_ASSERT_INT("slot.m550", model->first_thing_slot, 2,
                     "DEFS.H:2549 M550_FIRST_THING");
    PROBE_ASSERT_INT("requested.c715_mismatch", model->requested_wall_zone_note_is_mismatch, 1,
                     "DEFS.H:4044 C704 D3C; 4055 C715 D0C");
}

static void test_d3c_dispatch_and_body(void)
{
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int present_count = 0;
    int keepout_count = 0;

    PROBE_ASSERT("step.bounds",
                 dm1_v1_viewport_d3c_f0107_wall_ornament_step_at_pc34(
                     DM1_V1_D3C_F0107_STEP_COUNT_PC34) == NULL,
                 "step accessor bounds");
    for (i = 0; i < DM1_V1_D3C_F0107_STEP_COUNT_PC34; ++i) {
        const DM1_V1_D3CF0107StepPc34 *step =
            dm1_v1_viewport_d3c_f0107_wall_ornament_step_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "step.%u.present", (unsigned)i);
        PROBE_ASSERT(id, step != NULL, "step accessor");
        if (!step) continue;
        present_count += step->expected_present ? 1 : 0;
        keepout_count += step->expected_present ? 0 : 1;
        snprintf(id, sizeof(id), "step.%u.order", (unsigned)i);
        PROBE_ASSERT_INT(id, step->order_index, (int)i, "step order");
        snprintf(id, sizeof(id), "step.%u.anchor", (unsigned)i);
        PROBE_ASSERT_CONTAINS(id, step->redmcsb_anchor, "DUNVIEW.C", "step anchor");
    }
    PROBE_ASSERT_INT("steps.present_count", present_count, 7,
                     "D3L/D3R/D3C/F0118/wall/F0107/probe present");
    PROBE_ASSERT_INT("steps.keepout_count", keepout_count, 1,
                     "F0108 wall-branch keepout");
    PROBE_ASSERT_INT("f0128.d3l", model ? model->f0128_d3l_draw_line : 0, 8491,
                     "DUNVIEW.C:8491");
    PROBE_ASSERT_INT("f0128.d3r", model ? model->f0128_d3r_draw_line : 0, 8495,
                     "DUNVIEW.C:8495");
    PROBE_ASSERT_INT("f0128.d3c", model ? model->f0128_d3c_draw_line : 0, 8499,
                     "DUNVIEW.C:8499");
    PROBE_ASSERT_INT("f0128.after_d3l_d3r",
                     model ? model->f0128_after_d3l_d3r : 0, 1,
                     "DUNVIEW.C:8491->8495->8499");
    PROBE_ASSERT_INT("f0128.before_d2", model ? model->f0128_before_d2 : 0, 1,
                     "D3C before D2 dispatch tail");
    PROBE_ASSERT_INT("body.start", model ? model->body_function_start_line : 0, 6642,
                     "DUNVIEW.C F0118");
    PROBE_ASSERT_INT("body.end", model ? model->body_function_end_line : 0, 6763,
                     "DUNVIEW.C F0118");
    PROBE_ASSERT_INT("wall.case", model ? model->wall_case_line : 0, 6697,
                     "DUNVIEW.C:6697");
    PROBE_ASSERT_INT("wall.draw_first", model ? model->wall_draw_first_line : 0, 6707,
                     "DUNVIEW.C:6707");
    PROBE_ASSERT_INT("wall.draw_last", model ? model->wall_draw_last_line : 0, 6714,
                     "DUNVIEW.C:6714");
    PROBE_ASSERT_INT("f0107.call", model ? model->f0107_call_line : 0, 6716,
                     "DUNVIEW.C:6716");
    PROBE_ASSERT_INT("f0107.alcove_order",
                     model ? model->f0107_alcove_order_line : 0, 6717,
                     "DUNVIEW.C:6717");
    PROBE_ASSERT_INT("wall.return", model ? model->wall_case_return_line : 0, 6720,
                     "DUNVIEW.C:6720");
    PROBE_ASSERT_INT("f0108.contrast", model ? model->f0108_contrast_line : 0, 6722,
                     "DUNVIEW.C:6722 door branch contrasts wall branch");
}

static void test_ordinals_c10_and_zone_math(void)
{
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int accepted = 0;
    int reaches = 0;
    int skips = 0;
    int writes = 0;

    PROBE_ASSERT_INT("model.zero_false",
                     model ? model->f0107_zero_ordinal_returns_false : 0, 1,
                     "DUNVIEW.C:3571-3573");
    PROBE_ASSERT_INT("model.non_alcove_false",
                     model ? model->f0107_non_alcove_returns_false : 0, 1,
                     "DUNVIEW.C:3589/3933");
    PROBE_ASSERT_INT("model.alcove_true",
                     model ? model->f0107_alcove_returns_true : 0, 1,
                     "DUNVIEW.C:3933");
    PROBE_ASSERT_INT("model.f0107_c10", model ? model->f0107_blit_uses_c10 : 0, 1,
                     "DUNVIEW.C:3922");
    PROBE_ASSERT_INT("model.c10_preserve",
                     model ? model->c10_preserves_destination : 0, 1,
                     "DEFS.H:2088");
    PROBE_ASSERT_INT("model.c0_c5", model ? model->c0_to_c5_ordinals_pinned : 0, 1,
                     "C0..C5 ordinals");
    PROBE_ASSERT_INT("model.only_m552",
                     model ? model->only_m552_reaches_d3c : 0, 1,
                     "DUNVIEW.C:6716 M552-only D3C F0107");
    PROBE_ASSERT_INT("zone.base", model ? model->wall_ornament_zone_base : 0, 1004,
                     "DEFS.H:4222 C1004_ZONE_WALL_ORNAMENT");
    PROBE_ASSERT_INT("zone.stride", model ? model->wall_ornament_zone_stride : 0, 15,
                     "DUNVIEW.C:3586-3587 C15 zone stride");
    PROBE_ASSERT_INT("zone.view_wall", model ? model->view_wall_d3c_front : 0, 5,
                     "DEFS.H:2701 M578");
    PROBE_ASSERT_INT("zone.d3c", model ? model->wall_ornament_zone_d3c_front : 0, 1039,
                     "C1004 + CoordinateSet*15 + M578");
    PROBE_ASSERT_INT("zone.helper",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_zone_pc34(2, 5),
                     1039, "zone helper");

    PROBE_ASSERT("ordinal.bounds",
                 dm1_v1_viewport_d3c_f0107_wall_ornament_ordinal_at_pc34(
                     DM1_V1_D3C_F0107_ORDINAL_COUNT_PC34) == NULL,
                 "ordinal accessor bounds");
    for (i = 0; model && i < DM1_V1_D3C_F0107_ORDINAL_COUNT_PC34; ++i) {
        const DM1_V1_D3CF0107OrdinalPc34 *ordinal =
            dm1_v1_viewport_d3c_f0107_wall_ornament_ordinal_at_pc34(i);
        const DM1_V1_D3CF0107PixelPc34 *pixel = &model->pixels[i];
        char id[96];
        snprintf(id, sizeof(id), "ordinal.%u.present", (unsigned)i);
        PROBE_ASSERT(id, ordinal != NULL, "ordinal accessor");
        if (!ordinal) continue;
        accepted += ordinal->accepted_by_f0107_body;
        reaches += ordinal->reaches_d3c_f0107;
        snprintf(id, sizeof(id), "ordinal.%u.index", (unsigned)i);
        PROBE_ASSERT_INT(id, ordinal->ordinal_index_c0_to_c5, (int)i,
                         "C0..C5 ordinal index");
        snprintf(id, sizeof(id), "ordinal.%u.sensor", (unsigned)i);
        PROBE_ASSERT_INT(id, ordinal->sensor_ordinal, (int)i + 1,
                         "one-based F0107 ordinal");
        snprintf(id, sizeof(id), "ordinal.%u.slot", (unsigned)i);
        PROBE_ASSERT_INT(id, ordinal->aspect_slot, 5,
                         "M552 D3C front wall ornament slot");
        snprintf(id, sizeof(id), "ordinal.%u.view_wall", (unsigned)i);
        PROBE_ASSERT_INT(id, ordinal->view_wall, 5,
                         "M578 D3C front view wall");
        snprintf(id, sizeof(id), "ordinal.%u.accepts", (unsigned)i);
        PROBE_ASSERT_INT(id,
                         dm1_v1_viewport_d3c_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
                             (int)i),
                         1, "C0..C5 accepted");
        snprintf(id, sizeof(id), "ordinal.%u.anchor", (unsigned)i);
        PROBE_ASSERT_CONTAINS(id, ordinal->redmcsb_anchor, "DUNVIEW.C:6716",
                              "ordinal source anchor");
        snprintf(id, sizeof(id), "pixel.%u.after", (unsigned)i);
        PROBE_ASSERT_INT(id, pixel->after,
                         dm1_v1_viewport_d3c_f0107_wall_ornament_blend_pixel_pc34(
                             pixel->before, pixel->source, 10),
                         "C10 blend");
        snprintf(id, sizeof(id), "pixel.%u.xor", (unsigned)i);
        PROBE_ASSERT_INT(id, pixel->transparent_skip + pixel->writes_pixel, 1,
                         "skip/write exclusive");
        skips += pixel->transparent_skip;
        writes += pixel->writes_pixel;
    }
    PROBE_ASSERT_INT("ordinal.accepted_count", accepted, 6,
                     "C0..C5 accepted at D3C M552");
    PROBE_ASSERT_INT("ordinal.reaches_count", reaches, 6,
                     "C0..C5 reach D3C F0107 body");
    PROBE_ASSERT_INT("ordinal.reject_low",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_accepts_sensor_ordinal_pc34(-1),
                     0, "ordinal lower bound");
    PROBE_ASSERT_INT("ordinal.reject_high",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_accepts_sensor_ordinal_pc34(6),
                     0, "ordinal upper bound");
    PROBE_ASSERT_INT("alcove.zero",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_returns_alcove_pc34(0, true),
                     0, "zero ordinal");
    PROBE_ASSERT_INT("alcove.no",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_returns_alcove_pc34(5, false),
                     0, "non-alcove");
    PROBE_ASSERT_INT("alcove.yes",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_returns_alcove_pc34(5, true),
                     1, "alcove");
    PROBE_ASSERT_INT("blend.c10",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_blend_pixel_pc34(0xaa, 10, 10),
                     0xaa, "C10 preserves destination");
    PROBE_ASSERT_INT("blend.opaque",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_blend_pixel_pc34(0xaa, 0x51, 10),
                     0x51, "opaque ornament pixel writes");
    PROBE_ASSERT_INT("pixels.skip_count", skips, 2, "two C10 skips");
    PROBE_ASSERT_INT("pixels.write_count", writes, 4, "four opaque writes");
}

static void test_rejected_sibling_contracts(void)
{
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int rejects = 0;

    PROBE_ASSERT("rejected.bounds",
                 dm1_v1_viewport_d3c_f0107_wall_ornament_rejected_contract_at_pc34(
                     DM1_V1_D3C_F0107_REJECTED_CONTRACT_COUNT_PC34) == NULL,
                 "rejected accessor bounds");
    for (i = 0; model && i < DM1_V1_D3C_F0107_REJECTED_CONTRACT_COUNT_PC34; ++i) {
        const DM1_V1_D3CF0107RejectedContractPc34 *r =
            dm1_v1_viewport_d3c_f0107_wall_ornament_rejected_contract_at_pc34(i);
        char id[96];
        int same_cell;
        int same_zone;
        int same_view_wall;
        int same_aspect;

        snprintf(id, sizeof(id), "reject.%u.present", (unsigned)i);
        PROBE_ASSERT(id, r != NULL, "rejected accessor");
        if (!r) continue;
        same_cell = r->relative_depth == model->relative_depth &&
                    r->relative_lateral == model->relative_lateral;
        same_zone = r->wall_zone_first == model->wall_zone_d3c ||
                    r->wall_zone_second == model->wall_zone_d3c;
        same_view_wall = r->view_wall_first == model->view_wall_d3c_front ||
                         r->view_wall_second == model->view_wall_d3c_front;
        same_aspect = r->width * 7 == r->height * 5;
        rejects += !same_cell && !same_zone && !same_view_wall && !same_aspect;
        snprintf(id, sizeof(id), "reject.%u.not_same_cell", (unsigned)i);
        PROBE_ASSERT_INT(id, same_cell, 0, "cell position rejects sibling contract");
        snprintf(id, sizeof(id), "reject.%u.not_same_zone", (unsigned)i);
        PROBE_ASSERT_INT(id, same_zone, 0, "wall carrier zone rejects sibling contract");
        snprintf(id, sizeof(id), "reject.%u.not_same_view_wall", (unsigned)i);
        PROBE_ASSERT_INT(id, same_view_wall, 0, "view-wall ordinal rejects sibling contract");
        snprintf(id, sizeof(id), "reject.%u.not_same_aspect", (unsigned)i);
        PROBE_ASSERT_INT(id, same_aspect, 0, "probe aspect ratio rejects sibling contract");
        snprintf(id, sizeof(id), "reject.%u.anchor", (unsigned)i);
        PROBE_ASSERT_CONTAINS(id, r->redmcsb_anchor, "sibling", "sibling anchor");
    }
    PROBE_ASSERT_INT("reject.count", rejects,
                     DM1_V1_D3C_F0107_REJECTED_CONTRACT_COUNT_PC34,
                     "all sibling/helper contracts rejected");
}

static void test_synthetic_framebuffer_probe(void)
{
    uint8_t framebuffer[DM1_V1_D3C_F0107_FRAMEBUFFER_WIDTH_PC34 *
                        DM1_V1_D3C_F0107_FRAMEBUFFER_HEIGHT_PC34];
    DM1_V1_D3CF0107FramebufferProbePc34 probe;
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    size_t i;

    memset(framebuffer, 0xee, sizeof(framebuffer));
    PROBE_ASSERT_INT("probe.apply",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_probe_framebuffer_pc34(
                         framebuffer, sizeof(framebuffer), &probe),
                     1, "synthetic framebuffer probe");
    PROBE_ASSERT_INT("probe.touched", probe.touched_pixels, 7,
                     "D3C synthetic probe pixels");
    PROBE_ASSERT_INT("probe.writes", probe.writes, 5,
                     "opaque synthetic pixels");
    PROBE_ASSERT_INT("probe.skips", probe.transparent_skips, 2,
                     "C10 synthetic pixels");
    PROBE_ASSERT_INT("probe.overlap_flag", probe.d3c_probe_overlaps_rejected_contract, 0,
                     "D3C probe disjoint from rejected sibling boxes");
    for (i = 0; model && i < DM1_V1_D3C_F0107_REJECTED_CONTRACT_COUNT_PC34; ++i) {
        char id[96];
        snprintf(id, sizeof(id), "probe.reject.%u.overlap", (unsigned)i);
        PROBE_ASSERT_INT(id,
                         dm1_v1_viewport_d3c_f0107_wall_ornament_boxes_overlap_pc34(
                             122, 36, 5, 7, &model->rejected[i]),
                         0, "rejected synthetic boxes are disjoint");
        snprintf(id, sizeof(id), "probe.reject.%u.unchanged", (unsigned)i);
        PROBE_ASSERT_INT(id,
                         count_changed_pixels_in_box(framebuffer, &model->rejected[i]),
                         0, "rejected synthetic boxes unchanged");
    }
    PROBE_ASSERT("probe.hash.nonzero", probe.framebuffer_hash != 0u,
                 "framebuffer hash exists");
    PROBE_ASSERT_U32("probe.hash.stable", probe.framebuffer_hash, 0x2f525784u,
                     "deterministic synthetic framebuffer hash");
}

static void test_evidence_and_contract(void)
{
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    const char *e = dm1_v1_viewport_d3c_f0107_wall_ornament_source_evidence_pc34();
    const char *d = dm1_v1_viewport_d3c_f0107_wall_ornament_disjointness_note_pc34();

    PROBE_ASSERT_INT("contract.source_locked",
                     model ? model->source_locked_contract_only : 0, 1,
                     "contract-only source lock");
    PROBE_ASSERT_INT("contract.no_dos",
                     model ? model->no_original_dos_pixel_parity : 0, 1,
                     "no original DOS pixel parity claim");
    PROBE_ASSERT_INT("contract.no_assets",
                     model ? model->no_graphics_dat_reads : 0, 1,
                     "no GRAPHICS.DAT reads");

    PROBE_ASSERT_CONTAINS("evidence.f0118", e, "DUNVIEW.C F0118:6642-6763",
                          "D3C body");
    PROBE_ASSERT_CONTAINS("evidence.f0107_call", e, "DUNVIEW.C:6716",
                          "F0107 wall-ornament call");
    PROBE_ASSERT_CONTAINS("evidence.m552", e, "M552_FRONT_WALL_ORNAMENT_ORDINAL",
                          "M552 source anchor");
    PROBE_ASSERT_CONTAINS("evidence.m578", e, "M578_VIEW_WALL_D3C_FRONT",
                          "M578 source anchor");
    PROBE_ASSERT_CONTAINS("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938",
                          "F0107 wall-ornament dispatch");
    PROBE_ASSERT_CONTAINS("evidence.f0108", e, "DUNVIEW.C F0108:3940-4011",
                          "F0108 baseline");
    PROBE_ASSERT_CONTAINS("evidence.f0128", e, "DUNVIEW.C F0128:8491-8499",
                          "F0128 D3 order");
    PROBE_ASSERT_CONTAINS("evidence.f0163", e, "F0163:1769-1838",
                          "DUNGEON.C F0163");
    PROBE_ASSERT_CONTAINS("evidence.f0164", e, "F0164:1840-1905",
                          "DUNGEON.C F0164");
    PROBE_ASSERT_CONTAINS("evidence.f0172", e, "F0172:2466-2523",
                          "DUNGEON.C F0172");
    PROBE_ASSERT_CONTAINS("evidence.c10", e, "DEFS.H:2088",
                          "C10 source anchor");
    PROBE_ASSERT_CONTAINS("evidence.c704", e, "C704_ZONE_WALL_D3C",
                          "D3C wall-zone source anchor");
    PROBE_ASSERT_CONTAINS("evidence.c715", e, "C715_ZONE_WALL_D0C",
                          "requested C715 mismatch recorded");
    PROBE_ASSERT_CONTAINS("disjoint.d0", d, "D0L/D0R",
                          "D0 sibling disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.d1c", d, "D1C",
                          "D1C sibling disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.d1lr", d, "D1L/D1R",
                          "D1 side-pair sibling disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.d2c", d, "D2C",
                          "D2C sibling disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.d2lr", d, "D2L/D2R",
                          "D2 side-pair sibling disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.d3lr", d, "D3L/D3R",
                          "D3 side-pair sibling disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.alcove", d, "F0107 alcove/ordinal",
                          "alcove helper disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.no_assets", d, "GRAPHICS.DAT",
                          "asset-free");
    PROBE_ASSERT_U32("hash.stable",
                     dm1_v1_viewport_d3c_f0107_wall_ornament_deterministic_hash_pc34(),
                     0x45876648u, "deterministic model hash");
}

int main(void)
{
    uint32_t hash;

    test_core_model();
    test_d3c_dispatch_and_body();
    test_ordinals_c10_and_zone_math();
    test_rejected_sibling_contracts();
    test_synthetic_framebuffer_probe();
    test_evidence_and_contract();

    hash = dm1_v1_viewport_d3c_f0107_wall_ornament_deterministic_hash_pc34();
    printf("assertions=%d failures=%d hash=0x%08x\n",
           g_assertions, g_failures, (unsigned)hash);
    return g_failures ? 1 : 0;
}
