#include "firestaff/dm1/v1/viewport/d3c_f0107_wall_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_M600_VIEW_SQUARE_D3C = 11,
    DM1_M550_FIRST_THING = 2,
    DM1_M552_FRONT_WALL_ORNAMENT_ORDINAL = 5,
    DM1_M578_VIEW_WALL_D3C_FRONT = 5,
    DM1_M589_VIEW_FLOOR_D3C = 3,
    DM1_C14_WALL_D3C = 14,
    DM1_C704_ZONE_WALL_D3C = 704,
    DM1_C1004_ZONE_WALL_ORNAMENT = 1004,
    DM1_WALL_ORNAMENT_ZONE_STRIDE = 15
};

/*
 * ReDMCSB: DUNVIEW.C F0118:6642-6763 is the D3C body. Its wall branch
 * draws C14_WALL_D3C/C704_ZONE_WALL_D3C, then calls F0107 at line 6716
 * with L0206[M552_FRONT_WALL_ORNAMENT_ORDINAL] and
 * M578_VIEW_WALL_D3C_FRONT. F0107:3502-3938 owns C10 transparent wall
 * ornament blitting and the C1004 + CoordinateSet * C15 + ViewWall zone.
 */
static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0118:6642-6763 is the D3C draw body. "
    "The C00_ELEMENT_WALL branch starts at 6697, draws the D3C wall through "
    "C14_WALL_D3C/C704_ZONE_WALL_D3C at 6707-6714, calls "
    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF at DUNVIEW.C:6716 with "
    "L0206_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL] and "
    "M578_VIEW_WALL_D3C_FRONT, assigns C0x0000_CELL_ORDER_ALCOVE at 6717 "
    "for alcoves, and returns at 6720 for non-alcove walls. DUNVIEW.C "
    "F0107:3502-3938 rejects zero ordinals at 3571-3573/3936, computes "
    "C1004_ZONE_WALL_ORNAMENT + CoordinateSet*C15 + ViewWall at 3586-3587, "
    "classifies alcoves at 3589, and blits the wall ornament with "
    "C10_COLOR_FLESH at 3922 before returning the alcove boolean at 3933. "
    "DUNVIEW.C F0108:3940-4011 is the floor+ceiling baseline contrast; the "
    "D3C wall branch does not call it, while the D3C door-front branch calls "
    "F0108 at 6722. DUNVIEW.C F0128:8491-8499 dispatches D3L, then D3R, "
    "then D3C. DUNGEON.C F0163:1769-1838, F0164:1840-1905, and "
    "F0172:2466-2523 anchor thing-list and square-aspect inputs. "
    "DEFS.H:2088 defines C10_COLOR_FLESH; DEFS.H:2547-2554 defines "
    "M550/M551/M552/M553; DEFS.H:2607 defines M600_VIEW_SQUARE_D3C=11; "
    "DEFS.H:2701 defines M578_VIEW_WALL_D3C_FRONT=5; DEFS.H:3437 defines "
    "C14_WALL_D3C; DEFS.H:4044 defines C704_ZONE_WALL_D3C; DEFS.H:4055 "
    "defines C715_ZONE_WALL_D0C, not D3C; DEFS.H:4222 defines C1004.";

static const char s_disjointness_note[] =
    "D3C F0107 wall-ornament pass777 contract only. It covers the unique "
    "center far-depth D3C wall branch, relative cell depth 3/lateral 0, "
    "wall carrier C704, and front M552/M578 F0107 call. It rejects the "
    "existing D0L/D0R, D1C, D1L/D1R, D2C, D2L/D2R, D3L/D3R, and generic "
    "F0107 alcove/ordinal contracts by cell position, carrier wall zones, "
    "view-wall ordinals, and synthetic probe aspect ratio. It is asset-free, "
    "does not read GRAPHICS.DAT, and makes no original DOS pixel parity claim.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

bool dm1_v1_viewport_d3c_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

bool dm1_v1_viewport_d3c_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
    int ornament_index_c0_to_c5)
{
    return ornament_index_c0_to_c5 >= 0 &&
           ornament_index_c0_to_c5 < DM1_V1_D3C_F0107_ORDINAL_COUNT_PC34;
}

int dm1_v1_viewport_d3c_f0107_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall)
{
    if (coordinate_set < 0 || view_wall < 0) return -1;
    return DM1_C1004_ZONE_WALL_ORNAMENT +
           coordinate_set * DM1_WALL_ORNAMENT_ZONE_STRIDE + view_wall;
}

uint8_t dm1_v1_viewport_d3c_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d3c_f0107_wall_ornament_boxes_overlap_pc34(
    int ax,
    int ay,
    int aw,
    int ah,
    const DM1_V1_D3CF0107RejectedContractPc34 *b)
{
    if (!b) return false;
    return ax < b->x + b->width &&
           b->x < ax + aw &&
           ay < b->y + b->height &&
           b->y < ay + ah;
}

static void fill_steps(DM1_V1_D3CF0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D3CF0107StepPc34 steps[] = {
        { DM1_V1_D3C_F0107_STEP_F0128_D3L_DISPATCH_PC34, 0, 1,
          "F0128 draws D3L before D3C", "DUNVIEW.C:8491" },
        { DM1_V1_D3C_F0107_STEP_F0128_D3R_DISPATCH_PC34, 1, 1,
          "F0128 draws D3R before D3C", "DUNVIEW.C:8495" },
        { DM1_V1_D3C_F0107_STEP_F0128_D3C_DISPATCH_PC34, 2, 1,
          "F0128 draws D3C after D3L/D3R", "DUNVIEW.C:8499" },
        { DM1_V1_D3C_F0107_STEP_F0118_D3C_BODY_PC34, 3, 1,
          "F0118 D3C body", "DUNVIEW.C F0118:6642-6763" },
        { DM1_V1_D3C_F0107_STEP_WALL_BODY_PC34, 4, 1,
          "D3C wall branch", "DUNVIEW.C:6697-6720" },
        { DM1_V1_D3C_F0107_STEP_F0107_FRONT_WALL_ORNAMENT_PC34, 5, 1,
          "F0107 M552/M578 front wall ornament", "DUNVIEW.C:6716" },
        { DM1_V1_D3C_F0107_STEP_F0108_KEEP_OUT_PC34, 6, 0,
          "F0108 is separate from the D3C wall-ornament path", "DUNVIEW.C:6720/6722; F0108:3940-4011" },
        { DM1_V1_D3C_F0107_STEP_SYNTHETIC_FRAMEBUFFER_PC34, 7, 1,
          "Synthetic 320x200 framebuffer probe", "DUNVIEW.C:6716 pass777 D3C contract-only probe" }
    };
    memcpy(m->steps, steps, sizeof(steps));
}

static void fill_ordinals_pixels(DM1_V1_D3CF0107WallOrnamentModelPc34 *m)
{
    static const char *names[] = { "C0", "C1", "C2", "C3", "C4", "C5" };
    static const uint8_t before[] = { 0x61u, 0x62u, 0x63u, 0x64u, 0x65u, 0x66u };
    static const uint8_t source[] = { 0x71u, 10u, 0x72u, 0x73u, 10u, 0x74u };
    size_t i;

    for (i = 0; i < DM1_V1_D3C_F0107_ORDINAL_COUNT_PC34; ++i) {
        m->ordinals[i].ordinal_index_c0_to_c5 = (int)i;
        m->ordinals[i].sensor_ordinal = (int)i + 1;
        m->ordinals[i].aspect_slot = DM1_M552_FRONT_WALL_ORNAMENT_ORDINAL;
        m->ordinals[i].view_wall = DM1_M578_VIEW_WALL_D3C_FRONT;
        m->ordinals[i].accepted_by_f0107_body = 1;
        m->ordinals[i].reaches_d3c_f0107 = 1;
        m->ordinals[i].name = names[i];
        m->ordinals[i].redmcsb_anchor =
            "DEFS.H C0..C5 wall-ornament ordinals; DUNGEON.C F0172 square aspect; DUNVIEW.C:6716 M552/M578";

        m->pixels[i].before = before[i];
        m->pixels[i].source = source[i];
        m->pixels[i].after =
            dm1_v1_viewport_d3c_f0107_wall_ornament_blend_pixel_pc34(
                before[i], source[i], DM1_V1_D3C_F0107_C10_COLOR_FLESH_PC34);
        m->pixels[i].transparent_skip =
            source[i] == DM1_V1_D3C_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].writes_pixel =
            source[i] != DM1_V1_D3C_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].redmcsb_anchor = "DUNVIEW.C F0107:3922 C10_COLOR_FLESH blit";
    }
}

static void fill_rejected_contracts(DM1_V1_D3CF0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D3CF0107RejectedContractPc34 rejected[] = {
        { "D0L/D0R F0107 keepout", 0, -1, 716, 717, -1, -1, 12, 112, 24, 12,
          "D0L/D0R sibling: C716/C717, no direct F0107 wall call" },
        { "D1C F0107 keepout", 1, 0, 712, 712, 14, 14, 96, 80, 18, 10,
          "D1C sibling: M587/C712" },
        { "D1L/D1R F0107 keepout", 1, -1, 713, 714, 12, 13, 72, 74, 22, 9,
          "D1L/D1R sibling: M585/M586 and C713/C714" },
        { "D2C F0107 keepout", 2, 0, 709, 709, 10, 10, 108, 52, 8, 6,
          "D2C sibling: M583/C709" },
        { "D2L/D2R F0107 keepout", 2, -1, 710, 711, 7, 11, 44, 52, 24, 10,
          "D2L/D2R sibling: M580/M581/M582/M584 and C710/C711" },
        { "D3L/D3R F0107 keepout", 3, -1, 705, 706, 2, 6, 160, 38, 28, 10,
          "D3L/D3R sibling: M575/M576/M577/M579 and C705/C706" },
        { "F0107 alcove/ordinal helper keepout", -1, -1, -1, -1, -1, -1, 188, 116, 13, 5,
          "Generic F0107 helper sibling has no D3C cell position, wall carrier, or probe aspect" }
    };
    memcpy(m->rejected, rejected, sizeof(rejected));
}

bool dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D3CF0107WallOrnamentModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    out_model->framebuffer_width = DM1_V1_D3C_F0107_FRAMEBUFFER_WIDTH_PC34;
    out_model->framebuffer_height = DM1_V1_D3C_F0107_FRAMEBUFFER_HEIGHT_PC34;
    out_model->viewport_width = DM1_V1_D3C_F0107_VIEWPORT_WIDTH_PC34;
    out_model->viewport_height = DM1_V1_D3C_F0107_VIEWPORT_HEIGHT_PC34;
    out_model->viewport_x_first = 0;
    out_model->viewport_y_first = 0;
    out_model->viewport_x_last = DM1_V1_D3C_F0107_VIEWPORT_WIDTH_PC34 - 1;
    out_model->viewport_y_last = DM1_V1_D3C_F0107_VIEWPORT_HEIGHT_PC34 - 1;
    out_model->view_square_d3c = DM1_M600_VIEW_SQUARE_D3C;
    out_model->relative_depth = 3;
    out_model->relative_lateral = 0;
    out_model->c_coordinate = 0;
    out_model->y_coordinate = 0;
    out_model->view_wall_d3c_front = DM1_M578_VIEW_WALL_D3C_FRONT;
    out_model->wall_zone_d3c = DM1_C704_ZONE_WALL_D3C;
    out_model->wall_index_d3c = DM1_C14_WALL_D3C;
    out_model->floor_view_d3c = DM1_M589_VIEW_FLOOR_D3C;
    out_model->front_wall_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_ORDINAL;
    out_model->first_thing_slot = DM1_M550_FIRST_THING;
    out_model->requested_wall_zone_note_is_mismatch = 1;
    out_model->f0128_d3l_draw_line = 8491;
    out_model->f0128_d3r_draw_line = 8495;
    out_model->f0128_d3c_draw_line = 8499;
    out_model->f0128_after_d3l_d3r = 1;
    out_model->f0128_before_d2 = 1;
    out_model->body_function_start_line = 6642;
    out_model->body_function_end_line = 6763;
    out_model->wall_case_line = 6697;
    out_model->wall_draw_first_line = 6707;
    out_model->wall_draw_last_line = 6714;
    out_model->f0107_call_line = 6716;
    out_model->f0107_alcove_order_line = 6717;
    out_model->wall_case_return_line = 6720;
    out_model->f0108_contrast_line = 6722;
    out_model->f0107_zero_ordinal_returns_false =
        dm1_v1_viewport_d3c_f0107_wall_ornament_returns_alcove_pc34(0, true) ? 0 : 1;
    out_model->f0107_non_alcove_returns_false =
        dm1_v1_viewport_d3c_f0107_wall_ornament_returns_alcove_pc34(3, false) ? 0 : 1;
    out_model->f0107_alcove_returns_true =
        dm1_v1_viewport_d3c_f0107_wall_ornament_returns_alcove_pc34(3, true) ? 1 : 0;
    out_model->f0107_blit_uses_c10 = 1;
    out_model->c10_preserves_destination =
        dm1_v1_viewport_d3c_f0107_wall_ornament_blend_pixel_pc34(0x7au, 10u, 10u) == 0x7au;
    out_model->c0_to_c5_ordinals_pinned = 1;
    out_model->only_m552_reaches_d3c = 1;
    out_model->wall_ornament_zone_base = DM1_C1004_ZONE_WALL_ORNAMENT;
    out_model->wall_ornament_zone_stride = DM1_WALL_ORNAMENT_ZONE_STRIDE;
    out_model->wall_ornament_coordinate_set = 2;
    out_model->wall_ornament_zone_d3c_front =
        dm1_v1_viewport_d3c_f0107_wall_ornament_zone_pc34(
            out_model->wall_ornament_coordinate_set, DM1_M578_VIEW_WALL_D3C_FRONT);
    out_model->source_locked_contract_only = 1;
    out_model->no_original_dos_pixel_parity = 1;
    out_model->no_graphics_dat_reads = 1;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_steps(out_model);
    fill_ordinals_pixels(out_model);
    fill_rejected_contracts(out_model);

    out_model->deterministic_hash =
        dm1_v1_viewport_d3c_f0107_wall_ornament_hash_model_pc34(out_model);
    return true;
}

uint32_t dm1_v1_viewport_d3c_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->framebuffer_width);
    h = fnv1a_u32(h, (uint32_t)model->framebuffer_height);
    h = fnv1a_u32(h, (uint32_t)model->viewport_width);
    h = fnv1a_u32(h, (uint32_t)model->viewport_height);
    h = fnv1a_u32(h, (uint32_t)model->view_square_d3c);
    h = fnv1a_u32(h, (uint32_t)model->relative_depth);
    h = fnv1a_u32(h, (uint32_t)model->relative_lateral);
    h = fnv1a_u32(h, (uint32_t)model->view_wall_d3c_front);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d3c);
    h = fnv1a_u32(h, (uint32_t)model->wall_index_d3c);
    h = fnv1a_u32(h, (uint32_t)model->front_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d3l_draw_line);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d3r_draw_line);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d3c_draw_line);
    h = fnv1a_u32(h, (uint32_t)model->body_function_start_line);
    h = fnv1a_u32(h, (uint32_t)model->body_function_end_line);
    h = fnv1a_u32(h, (uint32_t)model->f0107_call_line);
    h = fnv1a_u32(h, (uint32_t)model->wall_ornament_zone_d3c_front);
    for (i = 0; i < DM1_V1_D3C_F0107_STEP_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->steps[i].step);
        h = fnv1a_u32(h, (uint32_t)model->steps[i].order_index);
        h = fnv1a_u32(h, (uint32_t)model->steps[i].expected_present);
    }
    for (i = 0; i < DM1_V1_D3C_F0107_ORDINAL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].ordinal_index_c0_to_c5);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].sensor_ordinal);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].view_wall);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].before);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].source);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].after);
    }
    for (i = 0; i < DM1_V1_D3C_F0107_REJECTED_CONTRACT_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->rejected[i].relative_depth);
        h = fnv1a_u32(h, (uint32_t)model->rejected[i].relative_lateral);
        h = fnv1a_u32(h, (uint32_t)model->rejected[i].wall_zone_first);
        h = fnv1a_u32(h, (uint32_t)model->rejected[i].view_wall_first);
        h = fnv1a_u32(h, (uint32_t)model->rejected[i].width);
        h = fnv1a_u32(h, (uint32_t)model->rejected[i].height);
    }
    return h;
}

const DM1_V1_D3CF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34(void)
{
    static DM1_V1_D3CF0107WallOrnamentModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_builder_pc34(&s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d3c_f0107_wall_ornament_deterministic_hash_pc34(void)
{
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1_V1_D3CF0107StepPc34 *
dm1_v1_viewport_d3c_f0107_wall_ornament_step_at_pc34(size_t index)
{
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D3C_F0107_STEP_COUNT_PC34) ? NULL : &model->steps[index];
}

const DM1_V1_D3CF0107OrdinalPc34 *
dm1_v1_viewport_d3c_f0107_wall_ornament_ordinal_at_pc34(size_t index)
{
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D3C_F0107_ORDINAL_COUNT_PC34) ? NULL : &model->ordinals[index];
}

const DM1_V1_D3CF0107RejectedContractPc34 *
dm1_v1_viewport_d3c_f0107_wall_ornament_rejected_contract_at_pc34(size_t index)
{
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D3C_F0107_REJECTED_CONTRACT_COUNT_PC34) ? NULL : &model->rejected[index];
}

bool dm1_v1_viewport_d3c_f0107_wall_ornament_probe_framebuffer_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_len,
    DM1_V1_D3CF0107FramebufferProbePc34 *out_probe)
{
    const DM1_V1_D3CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3c_f0107_wall_ornament_default_model_pc34();
    static const uint8_t source_pixels[] = {
        0x81u, 10u, 0x82u, 0x83u, 10u, 0x84u, 0x85u
    };
    uint32_t hash = 2166136261u;
    size_t required_len =
        (size_t)DM1_V1_D3C_F0107_FRAMEBUFFER_WIDTH_PC34 *
        (size_t)DM1_V1_D3C_F0107_FRAMEBUFFER_HEIGHT_PC34;
    int probe_x = 122;
    int probe_y = 36;
    int probe_w = 5;
    int probe_h = 7;
    int i;

    if (!framebuffer || !out_probe || !model || framebuffer_len < required_len) {
        return false;
    }
    memset(out_probe, 0, sizeof(*out_probe));
    for (i = 0; i < DM1_V1_D3C_F0107_REJECTED_CONTRACT_COUNT_PC34; ++i) {
        if (dm1_v1_viewport_d3c_f0107_wall_ornament_boxes_overlap_pc34(
                probe_x, probe_y, probe_w, probe_h, &model->rejected[i])) {
            out_probe->d3c_probe_overlaps_rejected_contract = 1;
        }
    }
    for (i = 0; i < probe_h; ++i) {
        int x = probe_x + (i % probe_w);
        int y = probe_y + i;
        size_t offset =
            (size_t)y * (size_t)DM1_V1_D3C_F0107_FRAMEBUFFER_WIDTH_PC34 +
            (size_t)x;
        uint8_t before = framebuffer[offset];
        uint8_t source = source_pixels[i % (int)(sizeof(source_pixels) / sizeof(source_pixels[0]))];
        uint8_t after =
            dm1_v1_viewport_d3c_f0107_wall_ornament_blend_pixel_pc34(
                before, source, DM1_V1_D3C_F0107_C10_COLOR_FLESH_PC34);
        framebuffer[offset] = after;
        out_probe->transparent_skips += source == DM1_V1_D3C_F0107_C10_COLOR_FLESH_PC34;
        out_probe->writes += source != DM1_V1_D3C_F0107_C10_COLOR_FLESH_PC34;
        ++out_probe->touched_pixels;
        hash = fnv1a_u32(hash, (uint32_t)offset);
        hash = fnv1a_u32(hash, (uint32_t)before);
        hash = fnv1a_u32(hash, (uint32_t)source);
        hash = fnv1a_u32(hash, (uint32_t)after);
    }
    out_probe->framebuffer_hash = hash;
    return true;
}

const char *dm1_v1_viewport_d3c_f0107_wall_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d3c_f0107_wall_ornament_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
