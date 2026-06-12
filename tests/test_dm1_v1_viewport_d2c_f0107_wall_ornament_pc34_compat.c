#include "firestaff/dm1/v1/viewport/d2c_f0107_wall_ornament_pc34_compat.h"

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
    const DM1_V1_D2CF0107ProbeBoxPc34 *box)
{
    int x;
    int y;
    int changed = 0;

    if (!framebuffer || !box) return -1;
    for (y = box->y; y < box->y + box->height; ++y) {
        for (x = box->x; x < box->x + box->width; ++x) {
            size_t offset =
                (size_t)y * (size_t)DM1_V1_D2C_F0107_FRAMEBUFFER_WIDTH_PC34 +
                (size_t)x;
            changed += framebuffer[offset] != 0xeeu;
        }
    }
    return changed;
}

static void test_core_model(void)
{
    DM1_V1_D2CF0107WallOrnamentModelPc34 built;
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34();

    PROBE_ASSERT_INT("builder.null",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_builder_pc34(NULL),
                     0, "builder guard");
    PROBE_ASSERT_INT("builder.ok",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_builder_pc34(&built),
                     1, "builder deterministic");
    PROBE_ASSERT("model.present", model != NULL, "model accessor");
    if (!model) return;

    PROBE_ASSERT_INT("hash.null",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_hash_model_pc34(NULL),
                     0, "hash guard");
    PROBE_ASSERT_U32("hash.builder", built.deterministic_hash,
                     model->deterministic_hash, "builder hash stable");
    PROBE_ASSERT_U32("hash.accessor",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_deterministic_hash_pc34(),
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
    PROBE_ASSERT_INT("view_square.d2c", model->view_square_d2c, 6,
                     "DEFS.H:2602 M603_VIEW_SQUARE_D2C");
    PROBE_ASSERT_INT("relative.depth", model->relative_depth, 2,
                     "DUNVIEW.C:8520 F0128 relative depth");
    PROBE_ASSERT_INT("relative.lateral", model->relative_lateral, 0,
                     "DUNVIEW.C:8520 F0128 relative lateral");
    PROBE_ASSERT_INT("position.c", model->c_coordinate, 0,
                     "D2C center-front c == 0");
    PROBE_ASSERT_INT("position.y", model->y_coordinate, 0,
                     "D2C center-front y == 0");
    PROBE_ASSERT_INT("view_wall.d2c_front", model->view_wall_d2c_front, 10,
                     "DEFS.H:2706 M583_VIEW_WALL_D2C_FRONT");
    PROBE_ASSERT_INT("wall_zone.d2c", model->wall_zone_d2c, 709,
                     "DEFS.H:4049 C709_ZONE_WALL_D2C");
    PROBE_ASSERT_INT("wall_index.d2c", model->wall_index_d2c, 9,
                     "DEFS.H:3432 C09_WALL_D2C");
    PROBE_ASSERT_INT("floor_view.d2c", model->floor_view_d2c, 6,
                     "DEFS.H:2756 M592_VIEW_FLOOR_D2C");
    PROBE_ASSERT_INT("slot.m552", model->front_wall_ornament_slot, 5,
                     "DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL");
    PROBE_ASSERT_INT("slot.m550", model->first_thing_slot, 2,
                     "DEFS.H:2549 M550_FIRST_THING");
}

static void test_d2c_dispatch_and_body(void)
{
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int present_count = 0;
    int keepout_count = 0;

    PROBE_ASSERT("step.bounds",
                 dm1_v1_viewport_d2c_f0107_wall_ornament_step_at_pc34(
                     DM1_V1_D2C_F0107_STEP_COUNT_PC34) == NULL,
                 "step accessor bounds");
    for (i = 0; i < DM1_V1_D2C_F0107_STEP_COUNT_PC34; ++i) {
        const DM1_V1_D2CF0107StepPc34 *step =
            dm1_v1_viewport_d2c_f0107_wall_ornament_step_at_pc34(i);
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
    PROBE_ASSERT_INT("steps.present_count", present_count, 6,
                     "D2C present steps plus synthetic probe");
    PROBE_ASSERT_INT("steps.keepout_count", keepout_count, 2,
                     "F0108/F0111 keepouts");
    PROBE_ASSERT_INT("f0128.update", model ? model->f0128_update_line : 0, 8520,
                     "DUNVIEW.C:8520");
    PROBE_ASSERT_INT("f0128.draw", model ? model->f0128_draw_line : 0, 8521,
                     "DUNVIEW.C:8521");
    PROBE_ASSERT_INT("f0128.after_d2_side_pair",
                     model ? model->f0128_after_d2l_d2r : 0, 1,
                     "DUNVIEW.C:8512-8521");
    PROBE_ASSERT_INT("f0128.before_d1_d0",
                     model ? model->f0128_before_d1_d0 : 0, 1,
                     "DUNVIEW.C:8521 before 8524-8542");
    PROBE_ASSERT_INT("body.start", model ? model->body_function_start_line : 0, 7244,
                     "DUNVIEW.C F0121");
    PROBE_ASSERT_INT("body.end", model ? model->body_function_end_line : 0, 7388,
                     "DUNVIEW.C F0121");
    PROBE_ASSERT_INT("wall.case", model ? model->wall_case_line : 0, 7289,
                     "DUNVIEW.C:7289");
    PROBE_ASSERT_INT("wall.draw_first", model ? model->wall_draw_first_line : 0, 7291,
                     "DUNVIEW.C:7291");
    PROBE_ASSERT_INT("wall.draw_last", model ? model->wall_draw_last_line : 0, 7306,
                     "DUNVIEW.C:7306");
    PROBE_ASSERT_INT("f0107.call", model ? model->f0107_call_line : 0, 7308,
                     "DUNVIEW.C:7308");
    PROBE_ASSERT_INT("f0107.alcove_order",
                     model ? model->f0107_alcove_order_line : 0, 7309,
                     "DUNVIEW.C:7309");
    PROBE_ASSERT_INT("wall.return", model ? model->wall_case_return_line : 0, 7312,
                     "DUNVIEW.C:7312");
}

static void test_ordinals_c10_and_alcove(void)
{
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34();
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
                     model ? model->only_m552_reaches_d2c : 0, 1,
                     "DUNVIEW.C:7308 M552-only D2C F0107");

    PROBE_ASSERT("ordinal.bounds",
                 dm1_v1_viewport_d2c_f0107_wall_ornament_ordinal_at_pc34(
                     DM1_V1_D2C_F0107_ORDINAL_COUNT_PC34) == NULL,
                 "ordinal accessor bounds");
    for (i = 0; model && i < DM1_V1_D2C_F0107_ORDINAL_COUNT_PC34; ++i) {
        const DM1_V1_D2CF0107OrdinalPc34 *ordinal =
            dm1_v1_viewport_d2c_f0107_wall_ornament_ordinal_at_pc34(i);
        const DM1_V1_D2CF0107PixelPc34 *pixel = &model->pixels[i];
        char id[96];
        snprintf(id, sizeof(id), "ordinal.%u.present", (unsigned)i);
        PROBE_ASSERT(id, ordinal != NULL, "ordinal accessor");
        if (!ordinal) continue;
        accepted += ordinal->accepted_by_f0107_body;
        reaches += ordinal->reaches_d2c_f0107;
        snprintf(id, sizeof(id), "ordinal.%u.index", (unsigned)i);
        PROBE_ASSERT_INT(id, ordinal->ordinal_index_c0_to_c5, (int)i,
                         "C0..C5 ordinal index");
        snprintf(id, sizeof(id), "ordinal.%u.sensor", (unsigned)i);
        PROBE_ASSERT_INT(id, ordinal->sensor_ordinal, (int)i + 1,
                         "one-based F0107 ordinal");
        snprintf(id, sizeof(id), "ordinal.%u.slot", (unsigned)i);
        PROBE_ASSERT_INT(id, ordinal->aspect_slot, 5,
                         "M552 D2C front wall ornament slot");
        snprintf(id, sizeof(id), "ordinal.%u.accepts", (unsigned)i);
        PROBE_ASSERT_INT(id,
                         dm1_v1_viewport_d2c_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
                             (int)i),
                         1, "C0..C5 accepted");
        snprintf(id, sizeof(id), "ordinal.%u.anchor", (unsigned)i);
        PROBE_ASSERT_CONTAINS(id, ordinal->redmcsb_anchor, "DUNVIEW.C:7308",
                              "ordinal source anchor");
        snprintf(id, sizeof(id), "pixel.%u.after", (unsigned)i);
        PROBE_ASSERT_INT(id, pixel->after,
                         dm1_v1_viewport_d2c_f0107_wall_ornament_blend_pixel_pc34(
                             pixel->before, pixel->source, 10),
                         "C10 blend");
        snprintf(id, sizeof(id), "pixel.%u.xor", (unsigned)i);
        PROBE_ASSERT_INT(id, pixel->transparent_skip + pixel->writes_pixel, 1,
                         "skip/write exclusive");
        skips += pixel->transparent_skip;
        writes += pixel->writes_pixel;
    }
    PROBE_ASSERT_INT("ordinal.accepted_count", accepted, 6,
                     "C0..C5 accepted at D2C M552");
    PROBE_ASSERT_INT("ordinal.reaches_count", reaches, 6,
                     "C0..C5 reach D2C F0107 body");
    PROBE_ASSERT_INT("ordinal.reject_low",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_accepts_sensor_ordinal_pc34(-1),
                     0, "ordinal lower bound");
    PROBE_ASSERT_INT("ordinal.reject_high",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_accepts_sensor_ordinal_pc34(6),
                     0, "ordinal upper bound");
    PROBE_ASSERT_INT("alcove.zero",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_returns_alcove_pc34(0, true),
                     0, "zero ordinal");
    PROBE_ASSERT_INT("alcove.no",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_returns_alcove_pc34(5, false),
                     0, "non-alcove");
    PROBE_ASSERT_INT("alcove.yes",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_returns_alcove_pc34(5, true),
                     1, "alcove");
    PROBE_ASSERT_INT("blend.c10",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_blend_pixel_pc34(0xaa, 10, 10),
                     0xaa, "C10 preserves destination");
    PROBE_ASSERT_INT("blend.opaque",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_blend_pixel_pc34(0xaa, 0x51, 10),
                     0x51, "opaque ornament pixel writes");
    PROBE_ASSERT_INT("pixels.skip_count", skips, 3, "three C10 skips");
    PROBE_ASSERT_INT("pixels.write_count", writes, 3, "three opaque writes");
}

static void test_synthetic_framebuffer_probe(void)
{
    uint8_t framebuffer[DM1_V1_D2C_F0107_FRAMEBUFFER_WIDTH_PC34 *
                        DM1_V1_D2C_F0107_FRAMEBUFFER_HEIGHT_PC34];
    DM1_V1_D2CF0107FramebufferProbePc34 probe;
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34();
    size_t i;

    memset(framebuffer, 0xee, sizeof(framebuffer));
    PROBE_ASSERT_INT("probe.apply",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_probe_framebuffer_pc34(
                         framebuffer, sizeof(framebuffer), &probe),
                     1, "synthetic framebuffer probe");
    PROBE_ASSERT_INT("probe.touched", probe.touched_pixels, 8,
                     "D2C synthetic probe pixels");
    PROBE_ASSERT_INT("probe.writes", probe.writes, 5,
                     "opaque synthetic pixels");
    PROBE_ASSERT_INT("probe.skips", probe.transparent_skips, 3,
                     "C10 synthetic pixels");
    PROBE_ASSERT_INT("probe.overlap_flag", probe.d2c_probe_overlaps_sister, 0,
                     "D2C probe disjoint from sister boxes");
    PROBE_ASSERT_INT("probe.d2c_changed",
                     count_changed_pixels_in_box(framebuffer, model ? &model->d2c_probe_box : NULL),
                     5, "only opaque D2C pixels changed");
    for (i = 0; model && i < DM1_V1_D2C_F0107_SISTER_COUNT_PC34; ++i) {
        char id[96];
        snprintf(id, sizeof(id), "probe.sister.%u.overlap", (unsigned)i);
        PROBE_ASSERT_INT(id,
                         dm1_v1_viewport_d2c_f0107_wall_ornament_boxes_overlap_pc34(
                             &model->d2c_probe_box, &model->sister_boxes[i]),
                         0, "sister synthetic boxes are disjoint");
        snprintf(id, sizeof(id), "probe.sister.%u.unchanged", (unsigned)i);
        PROBE_ASSERT_INT(id,
                         count_changed_pixels_in_box(framebuffer, &model->sister_boxes[i]),
                         0, "sister synthetic boxes unchanged");
    }
    PROBE_ASSERT("probe.hash.nonzero", probe.framebuffer_hash != 0u,
                 "framebuffer hash exists");
    PROBE_ASSERT_U32("probe.hash.stable", probe.framebuffer_hash, 0xa35e3204u,
                     "deterministic synthetic framebuffer hash");
}

static void test_evidence_and_contract(void)
{
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34();
    const char *e = dm1_v1_viewport_d2c_f0107_wall_ornament_source_evidence_pc34();
    const char *d = dm1_v1_viewport_d2c_f0107_wall_ornament_disjointness_note_pc34();

    PROBE_ASSERT_INT("contract.source_locked",
                     model ? model->source_locked_contract_only : 0, 1,
                     "contract-only source lock");
    PROBE_ASSERT_INT("contract.no_dos",
                     model ? model->no_original_dos_pixel_parity : 0, 1,
                     "no original DOS pixel parity claim");
    PROBE_ASSERT_INT("contract.no_assets",
                     model ? model->no_graphics_dat_reads : 0, 1,
                     "no GRAPHICS.DAT reads");

    PROBE_ASSERT_CONTAINS("evidence.f0121", e, "DUNVIEW.C F0121:7244-7388",
                          "actual local D2C body");
    PROBE_ASSERT_CONTAINS("evidence.requested_f0118", e,
                          "DUNVIEW.C F0118:6888-6986",
                          "requested pass774 label recorded");
    PROBE_ASSERT_CONTAINS("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938",
                          "F0107 wall-ornament dispatch");
    PROBE_ASSERT_CONTAINS("evidence.f0128", e, "DUNVIEW.C F0128:8503-8521",
                          "F0128 D2C dispatch body");
    PROBE_ASSERT_CONTAINS("evidence.f0108", e, "DUNVIEW.C F0108:3940-4011",
                          "F0108 baseline");
    PROBE_ASSERT_CONTAINS("evidence.f0163", e, "F0163:1769-1838",
                          "DUNGEON.C F0163");
    PROBE_ASSERT_CONTAINS("evidence.f0164", e, "F0164:1840-1905",
                          "DUNGEON.C F0164");
    PROBE_ASSERT_CONTAINS("evidence.f0172", e, "F0172:2466-2523",
                          "DUNGEON.C F0172");
    PROBE_ASSERT_CONTAINS("evidence.c10", e, "DEFS.H:2088",
                          "C10 source anchor");
    PROBE_ASSERT_CONTAINS("evidence.c0_c5", e, "C0..C5 ordinals",
                          "ornament ordinal source anchor");
    PROBE_ASSERT_CONTAINS("evidence.c705_c706", e, "C705/C706",
                          "sister wall-zone source anchor");
    PROBE_ASSERT_CONTAINS("disjoint.d2c", d, "c == 0 && y == 0",
                          "D2C unique center-front");
    PROBE_ASSERT_CONTAINS("disjoint.d0", d, "D0L/D0R F0107",
                          "D0 sister disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.d1c", d, "D1C F0107",
                          "D1C sister disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.d2lr", d, "D2L/D2R F0107",
                          "D2 side-pair sister disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.d3lr", d, "D3L/D3R F0107",
                          "D3 side-pair sister disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.alcove", d, "F0107 alcove helper",
                          "alcove helper disjoint");
    PROBE_ASSERT_CONTAINS("disjoint.no_assets", d, "GRAPHICS.DAT",
                          "asset-free");
    PROBE_ASSERT_U32("hash.stable",
                     dm1_v1_viewport_d2c_f0107_wall_ornament_deterministic_hash_pc34(),
                     0xb41bf6abu, "deterministic model hash");
}

int main(void)
{
    uint32_t hash;

    test_core_model();
    test_d2c_dispatch_and_body();
    test_ordinals_c10_and_alcove();
    test_synthetic_framebuffer_probe();
    test_evidence_and_contract();

    hash = dm1_v1_viewport_d2c_f0107_wall_ornament_deterministic_hash_pc34();
    printf("assertions=%d failures=%d hash=0x%08x\n",
           g_assertions, g_failures, (unsigned)hash);
    return g_failures ? 1 : 0;
}
