#include "firestaff/dm1/v1/viewport/d2c_f0107_wall_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_M603_VIEW_SQUARE_D2C = 6,
    DM1_M550_FIRST_THING = 2,
    DM1_M551_RIGHT_WALL_ORNAMENT_ORDINAL = 4,
    DM1_M552_FRONT_WALL_ORNAMENT_ORDINAL = 5,
    DM1_M553_LEFT_WALL_ORNAMENT_ORDINAL = 6,
    DM1_M583_VIEW_WALL_D2C_FRONT = 10,
    DM1_M592_VIEW_FLOOR_D2C = 6,
    DM1_C09_WALL_D2C = 9,
    DM1_C709_ZONE_WALL_D2C = 709,
    DM1_C0X0000_CELL_ORDER_ALCOVE = 0x0000
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0121:7244-7388 is the local ReDMCSB D2C body; the wall
 *   branch draws C09_WALL_D2C/C709_ZONE_WALL_D2C and calls F0107 at line
 *   7308 with M552_FRONT_WALL_ORNAMENT_ORDINAL/M583_VIEW_WALL_D2C_FRONT.
 * - DUNVIEW.C F0107:3502-3938 owns zero-ordinal rejection, C1004 + set *
 *   C15 + view-wall zone math, alcove classification, and C10 blitting.
 * - DUNVIEW.C F0128:8503-8521 reaches D2C after D2L2/D2R2/D2L/D2R and
 *   before D1/D0; the requested pass774 label also names F0118:6888-6986,
 *   but this local ReDMCSB tree places the D2C source in F0121.
 * - DUNVIEW.C F0108:3940-4011 is the floor/ceiling/ornament contrast.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523
 *   anchor thing-list and square-aspect inputs.
 */
static const char s_source_evidence[] =
    "ReDMCSB source-lock: local DUNVIEW.C F0121:7244-7388 is the D2C body "
    "for M603_VIEW_SQUARE_D2C; the wall branch is 7289-7312, draws "
    "C09_WALL_D2C/C709_ZONE_WALL_D2C at 7299-7306, calls "
    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF at 7308 with "
    "M552_FRONT_WALL_ORNAMENT_ORDINAL and M583_VIEW_WALL_D2C_FRONT, sends "
    "alcoves to C0x0000_CELL_ORDER_ALCOVE at 7309-7310, and returns at "
    "7312. The pass774 request named DUNVIEW.C F0118:6888-6986 as the D2C "
    "wall-body label; this source tree shows that range as a neighboring "
    "line-map label, so the executable pins the actual D2C F0121 lines. "
    "DUNVIEW.C F0107:3502-3938 pins zero-ordinal rejection at 3571-3573, "
    "C1004 wall-ornament zone math at 3586-3587, alcove classification at "
    "3589/3933, and C10_COLOR_FLESH blit at 3922. DUNVIEW.C F0128:8503-8521 "
    "dispatches D2L2, D2R2, D2L, D2R, then D2C; D2C is before D1L/D1R/D1C "
    "and D0L/D0R/D0C at 8524-8542. DUNVIEW.C F0108:3940-4011 anchors the "
    "floor+ceiling+ornament baseline and M592_VIEW_FLOOR_D2C. DUNGEON.C "
    "F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523 anchor thing-list "
    "and sensor/square-aspect input. DEFS.H:2088 anchors C10_COLOR_FLESH; "
    "DEFS.H:2547-2559 anchors M550/M551/M552/M553 and C0..C5 ordinals; "
    "DEFS.H:2596-2611 anchors M603_VIEW_SQUARE_D2C=6; DEFS.H:2696-2711 "
    "anchors M583_VIEW_WALL_D2C_FRONT=10; DEFS.H:3432 anchors C09_WALL_D2C; "
    "DEFS.H:4045-4051 anchors sibling wall zones C705/C706 and C709.";

static const char s_disjointness_note[] =
    "D2C F0107 wall-ornament pass774 contract only. It covers the unique "
    "center-front D2C wall branch c == 0 && y == 0 and only the front "
    "M552/M583 F0107 call. It does not duplicate D0L/D0R F0107, D1C F0107, "
    "D2L/D2R F0107, D3L/D3R F0107, or the F0107 alcove helper slice. The "
    "synthetic framebuffer probe writes only inside its D2C probe box and is "
    "checked against all four sister slice boxes. No GRAPHICS.DAT reads and "
    "no original DOS pixel parity claim are made.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

uint8_t dm1_v1_viewport_d2c_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d2c_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

bool dm1_v1_viewport_d2c_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
    int ornament_index_c0_to_c5)
{
    return ornament_index_c0_to_c5 >= 0 &&
           ornament_index_c0_to_c5 < DM1_V1_D2C_F0107_ORDINAL_COUNT_PC34;
}

bool dm1_v1_viewport_d2c_f0107_wall_ornament_boxes_overlap_pc34(
    const DM1_V1_D2CF0107ProbeBoxPc34 *a,
    const DM1_V1_D2CF0107ProbeBoxPc34 *b)
{
    if (!a || !b) return false;
    return a->x < b->x + b->width &&
           b->x < a->x + a->width &&
           a->y < b->y + b->height &&
           b->y < a->y + a->height;
}

static void fill_steps(DM1_V1_D2CF0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D2CF0107StepPc34 steps[] = {
        { DM1_V1_D2C_F0107_STEP_F0128_D2C_DISPATCH_PC34, 0, 1,
          "F0128 updates and draws D2C", "DUNVIEW.C:8520-8521" },
        { DM1_V1_D2C_F0107_STEP_F0121_D2C_BODY_PC34, 1, 1,
          "F0121 D2C body", "DUNVIEW.C F0121:7244-7388" },
        { DM1_V1_D2C_F0107_STEP_WALL_BODY_PC34, 2, 1,
          "D2C wall body", "DUNVIEW.C:7289-7312" },
        { DM1_V1_D2C_F0107_STEP_F0107_FRONT_WALL_ORNAMENT_PC34, 3, 1,
          "F0107 M552/M583 front wall ornament", "DUNVIEW.C:7308" },
        { DM1_V1_D2C_F0107_STEP_F0115_ALCOVE_ONLY_PC34, 4, 1,
          "F0115 only when F0107 returns alcove", "DUNVIEW.C:7308-7310" },
        { DM1_V1_D2C_F0107_STEP_F0108_KEEP_OUT_PC34, 5, 0,
          "F0108 is not in the plain wall branch", "DUNVIEW.C:7312/7314" },
        { DM1_V1_D2C_F0107_STEP_F0111_KEEP_OUT_PC34, 6, 0,
          "F0111 is not in the plain wall branch", "DUNVIEW.C:7312/7336-7339" },
        { DM1_V1_D2C_F0107_STEP_SYNTHETIC_FRAMEBUFFER_PC34, 7, 1,
          "Synthetic 320x200 framebuffer probe", "DUNVIEW.C:7308 contract-only pass774 probe" }
    };
    memcpy(m->steps, steps, sizeof(steps));
}

static void fill_ordinals_pixels(DM1_V1_D2CF0107WallOrnamentModelPc34 *m)
{
    static const char *names[] = { "C0", "C1", "C2", "C3", "C4", "C5" };
    static const uint8_t before[] = { 0x21u, 0x22u, 0x23u, 0x24u, 0x25u, 0x26u };
    static const uint8_t source[] = { 10u, 0x31u, 10u, 0x32u, 0x33u, 10u };
    size_t i;

    for (i = 0; i < DM1_V1_D2C_F0107_ORDINAL_COUNT_PC34; ++i) {
        m->ordinals[i].ordinal_index_c0_to_c5 = (int)i;
        m->ordinals[i].sensor_ordinal = (int)i + 1;
        m->ordinals[i].aspect_slot = DM1_M552_FRONT_WALL_ORNAMENT_ORDINAL;
        m->ordinals[i].reaches_d2c_f0107 = 1;
        m->ordinals[i].accepted_by_f0107_body = 1;
        m->ordinals[i].name = names[i];
        m->ordinals[i].redmcsb_anchor =
            "DEFS.H C0..C5 wall-ornament ordinals; DUNGEON.C F0172 square aspect; DUNVIEW.C:7308 M552";

        m->pixels[i].before = before[i];
        m->pixels[i].source = source[i];
        m->pixels[i].after =
            dm1_v1_viewport_d2c_f0107_wall_ornament_blend_pixel_pc34(
                before[i], source[i], DM1_V1_D2C_F0107_C10_COLOR_FLESH_PC34);
        m->pixels[i].transparent_skip =
            source[i] == DM1_V1_D2C_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].writes_pixel =
            source[i] != DM1_V1_D2C_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].redmcsb_anchor = "DUNVIEW.C F0107:3922 C10_COLOR_FLESH blit";
    }
}

static void fill_probe_boxes(DM1_V1_D2CF0107WallOrnamentModelPc34 *m)
{
    m->d2c_probe_box = (DM1_V1_D2CF0107ProbeBoxPc34){
        "pass774 D2C F0107 synthetic probe", 108, 52, 8, 6,
        "DUNVIEW.C:7308 M552/M583 center-front wall ornament"
    };
    m->sister_boxes[0] = (DM1_V1_D2CF0107ProbeBoxPc34){
        "pass767 D0L/D0R F0107 sister", 12, 112, 24, 12,
        "D0L/D0R sister slice"
    };
    m->sister_boxes[1] = (DM1_V1_D2CF0107ProbeBoxPc34){
        "pass767 D1C F0107 sister", 96, 80, 18, 10,
        "D1C sister slice"
    };
    m->sister_boxes[2] = (DM1_V1_D2CF0107ProbeBoxPc34){
        "pass767 D2L/D2R F0107 sister", 44, 52, 24, 10,
        "D2L/D2R side-pair sister slice"
    };
    m->sister_boxes[3] = (DM1_V1_D2CF0107ProbeBoxPc34){
        "pass767 D3L/D3R F0107 sister", 160, 38, 28, 10,
        "D3L/D3R side-pair sister slice"
    };
}

bool dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D2CF0107WallOrnamentModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    out_model->framebuffer_width = DM1_V1_D2C_F0107_FRAMEBUFFER_WIDTH_PC34;
    out_model->framebuffer_height = DM1_V1_D2C_F0107_FRAMEBUFFER_HEIGHT_PC34;
    out_model->viewport_width = DM1_V1_D2C_F0107_VIEWPORT_WIDTH_PC34;
    out_model->viewport_height = DM1_V1_D2C_F0107_VIEWPORT_HEIGHT_PC34;
    out_model->viewport_x_first = 0;
    out_model->viewport_y_first = 0;
    out_model->viewport_x_last = DM1_V1_D2C_F0107_VIEWPORT_WIDTH_PC34 - 1;
    out_model->viewport_y_last = DM1_V1_D2C_F0107_VIEWPORT_HEIGHT_PC34 - 1;
    out_model->view_square_d2c = DM1_M603_VIEW_SQUARE_D2C;
    out_model->relative_depth = 2;
    out_model->relative_lateral = 0;
    out_model->c_coordinate = 0;
    out_model->y_coordinate = 0;
    out_model->view_wall_d2c_front = DM1_M583_VIEW_WALL_D2C_FRONT;
    out_model->wall_zone_d2c = DM1_C709_ZONE_WALL_D2C;
    out_model->wall_index_d2c = DM1_C09_WALL_D2C;
    out_model->floor_view_d2c = DM1_M592_VIEW_FLOOR_D2C;
    out_model->front_wall_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_ORDINAL;
    out_model->first_thing_slot = DM1_M550_FIRST_THING;
    out_model->f0128_update_line = 8520;
    out_model->f0128_draw_line = 8521;
    out_model->f0128_after_d2l_d2r = 1;
    out_model->f0128_before_d1_d0 = 1;
    out_model->body_function_start_line = 7244;
    out_model->body_function_end_line = 7388;
    out_model->wall_case_line = 7289;
    out_model->wall_draw_first_line = 7291;
    out_model->wall_draw_last_line = 7306;
    out_model->f0107_call_line = 7308;
    out_model->f0107_alcove_order_line = 7309;
    out_model->wall_case_return_line = 7312;
    out_model->f0107_zero_ordinal_returns_false =
        dm1_v1_viewport_d2c_f0107_wall_ornament_returns_alcove_pc34(0, true) ? 0 : 1;
    out_model->f0107_non_alcove_returns_false =
        dm1_v1_viewport_d2c_f0107_wall_ornament_returns_alcove_pc34(3, false) ? 0 : 1;
    out_model->f0107_alcove_returns_true =
        dm1_v1_viewport_d2c_f0107_wall_ornament_returns_alcove_pc34(3, true) ? 1 : 0;
    out_model->f0107_blit_uses_c10 = 1;
    out_model->c10_preserves_destination =
        dm1_v1_viewport_d2c_f0107_wall_ornament_blend_pixel_pc34(0x7au, 10u, 10u) == 0x7au;
    out_model->c0_to_c5_ordinals_pinned = 1;
    out_model->only_m552_reaches_d2c = 1;
    out_model->source_locked_contract_only = 1;
    out_model->no_original_dos_pixel_parity = 1;
    out_model->no_graphics_dat_reads = 1;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_steps(out_model);
    fill_ordinals_pixels(out_model);
    fill_probe_boxes(out_model);

    out_model->deterministic_hash =
        dm1_v1_viewport_d2c_f0107_wall_ornament_hash_model_pc34(out_model);
    return true;
}

uint32_t dm1_v1_viewport_d2c_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->framebuffer_width);
    h = fnv1a_u32(h, (uint32_t)model->framebuffer_height);
    h = fnv1a_u32(h, (uint32_t)model->viewport_width);
    h = fnv1a_u32(h, (uint32_t)model->viewport_height);
    h = fnv1a_u32(h, (uint32_t)model->view_square_d2c);
    h = fnv1a_u32(h, (uint32_t)model->relative_depth);
    h = fnv1a_u32(h, (uint32_t)model->relative_lateral);
    h = fnv1a_u32(h, (uint32_t)model->view_wall_d2c_front);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d2c);
    h = fnv1a_u32(h, (uint32_t)model->wall_index_d2c);
    h = fnv1a_u32(h, (uint32_t)model->front_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->f0128_update_line);
    h = fnv1a_u32(h, (uint32_t)model->f0128_draw_line);
    h = fnv1a_u32(h, (uint32_t)model->body_function_start_line);
    h = fnv1a_u32(h, (uint32_t)model->body_function_end_line);
    h = fnv1a_u32(h, (uint32_t)model->f0107_call_line);
    h = fnv1a_u32(h, (uint32_t)model->d2c_probe_box.x);
    h = fnv1a_u32(h, (uint32_t)model->d2c_probe_box.y);
    h = fnv1a_u32(h, (uint32_t)model->d2c_probe_box.width);
    h = fnv1a_u32(h, (uint32_t)model->d2c_probe_box.height);
    for (i = 0; i < DM1_V1_D2C_F0107_STEP_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->steps[i].step);
        h = fnv1a_u32(h, (uint32_t)model->steps[i].order_index);
        h = fnv1a_u32(h, (uint32_t)model->steps[i].expected_present);
    }
    for (i = 0; i < DM1_V1_D2C_F0107_ORDINAL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].ordinal_index_c0_to_c5);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].sensor_ordinal);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].aspect_slot);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].before);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].source);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].after);
    }
    return h;
}

const DM1_V1_D2CF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34(void)
{
    static DM1_V1_D2CF0107WallOrnamentModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_builder_pc34(&s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d2c_f0107_wall_ornament_deterministic_hash_pc34(void)
{
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1_V1_D2CF0107StepPc34 *
dm1_v1_viewport_d2c_f0107_wall_ornament_step_at_pc34(size_t index)
{
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2C_F0107_STEP_COUNT_PC34) ? NULL : &model->steps[index];
}

const DM1_V1_D2CF0107OrdinalPc34 *
dm1_v1_viewport_d2c_f0107_wall_ornament_ordinal_at_pc34(size_t index)
{
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2C_F0107_ORDINAL_COUNT_PC34) ? NULL : &model->ordinals[index];
}

bool dm1_v1_viewport_d2c_f0107_wall_ornament_probe_framebuffer_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_len,
    DM1_V1_D2CF0107FramebufferProbePc34 *out_probe)
{
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34();
    static const uint8_t source_pixels[] = {
        0x41u, 10u, 0x42u, 10u, 0x43u, 0x44u, 10u, 0x45u
    };
    uint32_t hash = 2166136261u;
    size_t required_len =
        (size_t)DM1_V1_D2C_F0107_FRAMEBUFFER_WIDTH_PC34 *
        (size_t)DM1_V1_D2C_F0107_FRAMEBUFFER_HEIGHT_PC34;
    int i;

    if (!framebuffer || !out_probe || !model || framebuffer_len < required_len) {
        return false;
    }
    memset(out_probe, 0, sizeof(*out_probe));
    for (i = 0; i < DM1_V1_D2C_F0107_SISTER_COUNT_PC34; ++i) {
        if (dm1_v1_viewport_d2c_f0107_wall_ornament_boxes_overlap_pc34(
                &model->d2c_probe_box, &model->sister_boxes[i])) {
            out_probe->d2c_probe_overlaps_sister = 1;
        }
    }
    for (i = 0; i < model->d2c_probe_box.width; ++i) {
        int x = model->d2c_probe_box.x + i;
        int y = model->d2c_probe_box.y + (i % model->d2c_probe_box.height);
        size_t offset =
            (size_t)y * (size_t)DM1_V1_D2C_F0107_FRAMEBUFFER_WIDTH_PC34 +
            (size_t)x;
        uint8_t before = framebuffer[offset];
        uint8_t source = source_pixels[i % (int)(sizeof(source_pixels) / sizeof(source_pixels[0]))];
        uint8_t after =
            dm1_v1_viewport_d2c_f0107_wall_ornament_blend_pixel_pc34(
                before, source, DM1_V1_D2C_F0107_C10_COLOR_FLESH_PC34);
        framebuffer[offset] = after;
        out_probe->transparent_skips += source == DM1_V1_D2C_F0107_C10_COLOR_FLESH_PC34;
        out_probe->writes += source != DM1_V1_D2C_F0107_C10_COLOR_FLESH_PC34;
        ++out_probe->touched_pixels;
        hash = fnv1a_u32(hash, (uint32_t)offset);
        hash = fnv1a_u32(hash, (uint32_t)before);
        hash = fnv1a_u32(hash, (uint32_t)source);
        hash = fnv1a_u32(hash, (uint32_t)after);
    }
    out_probe->framebuffer_hash = hash;
    return true;
}

const char *dm1_v1_viewport_d2c_f0107_wall_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d2c_f0107_wall_ornament_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
