#include "firestaff/dm1/v1/viewport/d0l2_d0r2_f0107_wall_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_M610_VIEW_SQUARE_D0L = 1,
    DM1_M611_VIEW_SQUARE_D0R = 2,
    DM1_C716_ZONE_WALL_D0L = 716,
    DM1_C717_ZONE_WALL_D0R = 717,
    DM1_C870_ZONE_CEILING_PIT_D0L = 870,
    DM1_C872_ZONE_CEILING_PIT_D0R = 872,
    DM1_C1004_ZONE_WALL_ORNAMENT = 1004,
    DM1_WALL_ORNAMENT_ZONE_STRIDE = 15,
    DM1_D0_WALL_ORNAMENT_COORDINATE_SET = 0,
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M551_RIGHT_WALL_ORNAMENT_SLOT = 4,
    DM1_M552_FRONT_WALL_ORNAMENT_SLOT = 5,
    DM1_M553_LEFT_WALL_ORNAMENT_SLOT = 6,
    DM1_M585_VIEW_WALL_D1L_RIGHT = 12,
    DM1_M586_VIEW_WALL_D1R_LEFT = 13,
    DM1_M587_VIEW_WALL_D1C_FRONT = 14,
    DM1_D0L_THING_ORDER = 0x0002,
    DM1_D0R_THING_ORDER = 0x0001
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0125:7960-8062 and F0126:8064-8162 are the D0L/D0R
 *   side-pair dispatch bodies reached from F0128:8536-8541. Their WALL and
 *   STAIRS_SIDE cases return before the open-square tail; PIT, CORRIDOR,
 *   DOOR_SIDE, and TELEPORTER fall through the shared ceiling/thing/field
 *   tail. DOOR_FRONT is not a D0L/D0R element in these bodies.
 * - DUNVIEW.C F0107:3502-3938 owns wall-ornament ordinal zero short-circuit,
 *   C1004 + coordinateSet * 15 + viewWall zone math, the alcove boolean, and
 *   the C10 transparent blit.
 * - DEFS.H:2088/2549-2553/2698-2710/4045-4057 anchor C10, M550..M553,
 *   F0107 view-wall ordinals, and the nearby wall-zone family.
 */
static const char s_source_evidence[] =
    "Source-locked contract-only DM1 V1 D0L2/D0R2 F0107 wall-ornament gate; "
    "no real-asset bitmap parity and no game-data load. ReDMCSB DUNVIEW.C "
    "F0107:3502-3938 is the only wall-ornament/alcove helper: ordinal zero "
    "returns false at 3571-3573, non-zero ordinals are decremented at 3575, "
    "PC34 zone math is C1004 + CoordinateSet * C15 + ViewWall at 3586-3587, "
    "F0149 classifies alcoves at 3589, C10_COLOR_FLESH is the transparent "
    "blit color at 3922, and the alcove boolean returns at 3933. ReDMCSB "
    "DUNVIEW.C F0125:7960-8062 and F0126:8064-8162 are the D0L/D0R dispatch "
    "bodies reached by F0128:8536-8541; C18 stairs-side returns at 7988/8092, "
    "C00 wall returns at 8038/8144, C01 corridor/C02 pit/C16 door-side/C05 "
    "teleporter continue through the shared open-square tail, and no C17 "
    "door-front element is handled there. F0128 follows the earlier "
    "F0116/F0117 wall-composition pair and dispatches D0L before D0R. "
    "DEFS.H:2088 binds C10; DEFS.H:2549-2553 binds M550/M551/M552/M553; "
    "DEFS.H:2698-2710 binds the wall-ornament view-wall ordinal band; "
    "DEFS.H:4045-4057 binds the wall-zone family. This gate is byte-stable, "
    "uses a deterministic seed, and only asserts a source contract.";

static const char s_disjointness_note[] =
    "D0L2/D0R2 F0107 wall-ornament source-lock contract only. It is "
    "disjoint from D0L/D0R F0107 by the terminal side-pair contract, from "
    "D1L2/D1R2 and D2L2/D2R2 wall-composition gates by relative depth and "
    "the F0125/F0126 body matrix, and from D2L2/D2R2 F0107 by the absence "
    "of F0119/F0120 carrier calls. It does not claim original DOS pixel "
    "parity and reads no GRAPHICS.DAT.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

int dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall)
{
    if (coordinate_set < 0 || view_wall < 0) return -1;
    return DM1_C1004_ZONE_WALL_ORNAMENT +
           coordinate_set * DM1_WALL_ORNAMENT_ZONE_STRIDE + view_wall;
}

uint8_t dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

static void fill_lanes(DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *m)
{
    m->lanes[0] = (DM1_V1_D0L2D0R2F0107LanePc34){
        DM1_V1_D0L2_D0R2_F0107_SIDE_D0L2_PC34,
        "D0L2",
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        8536,
        8537,
        7960,
        8062,
        DM1_M610_VIEW_SQUARE_D0L,
        0,
        -1,
        DM1_C716_ZONE_WALL_D0L,
        DM1_C870_ZONE_CEILING_PIT_D0L,
        DM1_M550_FIRST_THING_SLOT,
        DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
        DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
        DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
        8005,
        DM1_D0L_THING_ORDER,
        1,
        0,
        "DUNVIEW.C F0125:7960-8062; F0128:8536-8537"
    };
    m->lanes[1] = (DM1_V1_D0L2D0R2F0107LanePc34){
        DM1_V1_D0L2_D0R2_F0107_SIDE_D0R2_PC34,
        "D0R2",
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        8540,
        8541,
        8064,
        8162,
        DM1_M611_VIEW_SQUARE_D0R,
        0,
        1,
        DM1_C717_ZONE_WALL_D0R,
        DM1_C872_ZONE_CEILING_PIT_D0R,
        DM1_M550_FIRST_THING_SLOT,
        DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
        DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
        DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
        8115,
        DM1_D0R_THING_ORDER,
        2,
        1,
        "DUNVIEW.C F0126:8064-8162; F0128:8540-8541"
    };
}

static void fill_routes(DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D0L2D0R2F0107ElementRoutePc34 routes[] = {
        { DM1_V1_D0L2_D0R2_F0107_ELEMENT_WALL_PC34, "WALL", 1, 1, 0, 0, 0, 1,
          8007, 8038, 8117, 8144,
          "DUNVIEW.C F0125:8007-8038; F0126:8117-8144 wall return before tail" },
        { DM1_V1_D0L2_D0R2_F0107_ELEMENT_CORRIDOR_PC34, "CORRIDOR", 1, 0, 1, 1, 0, 1,
          7999, 8006, 8103, 8116,
          "DUNVIEW.C F0125:7999-8006; F0126:8103-8116 corridor open tail" },
        { DM1_V1_D0L2_D0R2_F0107_ELEMENT_PIT_PC34, "PIT", 1, 0, 1, 1, 0, 1,
          7989, 8006, 8093, 8116,
          "DUNVIEW.C F0125:7989-8006; F0126:8093-8116 pit fall-through" },
        { DM1_V1_D0L2_D0R2_F0107_ELEMENT_TELEPORTER_PC34, "TELEPORTER", 1, 0, 1, 1, 1, 1,
          8001, 8061, 8105, 8161,
          "DUNVIEW.C F0125:8001-8061; F0126:8105-8161 teleporter field tail" },
        { DM1_V1_D0L2_D0R2_F0107_ELEMENT_DOOR_SIDE_PC34, "DOOR_SIDE", 1, 0, 1, 1, 0, 1,
          8000, 8006, 8104, 8116,
          "DUNVIEW.C F0125:8000-8006; F0126:8104-8116 door-side open tail" },
        { DM1_V1_D0L2_D0R2_F0107_ELEMENT_DOOR_FRONT_PC34, "DOOR_FRONT", 0, 1, 0, 0, 0, 1,
          0, 0, 0, 0,
          "DUNVIEW.C F0125/F0126 have no C17 door-front case" },
        { DM1_V1_D0L2_D0R2_F0107_ELEMENT_STAIRS_SIDE_PC34, "STAIRS_SIDE", 1, 1, 0, 0, 0, 1,
          7978, 7988, 8082, 8092,
          "DUNVIEW.C F0125:7978-7988; F0126:8082-8092 stairs return before tail" }
    };

    memcpy(m->routes, routes, sizeof(routes));
}

static void fill_calls(DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *m)
{
    static const int elements[] = {
        DM1_V1_D0L2_D0R2_F0107_ELEMENT_CORRIDOR_PC34,
        DM1_V1_D0L2_D0R2_F0107_ELEMENT_PIT_PC34,
        DM1_V1_D0L2_D0R2_F0107_ELEMENT_DOOR_SIDE_PC34,
        DM1_V1_D0L2_D0R2_F0107_ELEMENT_TELEPORTER_PC34
    };
    size_t side;
    size_t i;
    size_t call = 0;

    for (side = 0; side < DM1_V1_D0L2_D0R2_F0107_SIDE_COUNT_PC34; ++side) {
        for (i = 0; i < sizeof(elements) / sizeof(elements[0]); ++i) {
            const int view_wall = side == 0 ?
                DM1_M585_VIEW_WALL_D1L_RIGHT : DM1_M586_VIEW_WALL_D1R_LEFT;
            m->calls[call] = (DM1_V1_D0L2D0R2F0107CallPc34){
                (int)call,
                (int)side,
                elements[i],
                DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
                "M552_FRONT_WALL_ORNAMENT_ORDINAL",
                view_wall,
                side == 0 ? "M585_VIEW_WALL_D1L_RIGHT" : "M586_VIEW_WALL_D1R_LEFT",
                DM1_D0_WALL_ORNAMENT_COORDINATE_SET,
                dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(
                    DM1_D0_WALL_ORNAMENT_COORDINATE_SET, view_wall),
                1,
                1,
                1,
                side == 0 ?
                    "DUNVIEW.C F0125 open-tail candidate; F0107:3502-3938" :
                    "DUNVIEW.C F0126 open-tail candidate; F0107:3502-3938"
            };
            ++call;
        }
    }
}

static void fill_ordinals_pixels(DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *m)
{
    static const uint8_t before[] = { 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u, 0x38u };
    static const uint8_t source[] = { 10u, 0x61u, 0x62u, 10u, 0x63u, 10u, 0x64u, 0x65u };
    size_t i;

    for (i = 0; i < DM1_V1_D0L2_D0R2_F0107_ORDINAL_COUNT_PC34; ++i) {
        m->ordinals[i].ordinal_index_c0_to_c5 = (int)i;
        m->ordinals[i].sensor_ordinal = (int)i + 1;
        m->ordinals[i].accepted_at_d0l2_front = 1;
        m->ordinals[i].accepted_at_d0r2_front = 1;
        m->ordinals[i].decremented_index = (int)i;
        m->ordinals[i].source_slot_m552 = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
        m->ordinals[i].redmcsb_anchor =
            "DEFS.H M552 front wall ordinal; DUNVIEW.C F0107:3571-3575 C0..C5 ordinal path";
    }

    for (i = 0; i < DM1_V1_D0L2_D0R2_F0107_PIXEL_COUNT_PC34; ++i) {
        m->pixels[i].before = before[i];
        m->pixels[i].source = source[i];
        m->pixels[i].after =
            dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_blend_pixel_pc34(
                before[i], source[i], DM1_V1_D0L2_D0R2_F0107_C10_COLOR_FLESH_PC34);
        m->pixels[i].transparent_skip =
            source[i] == DM1_V1_D0L2_D0R2_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].writes_pixel = !m->pixels[i].transparent_skip;
        m->pixels[i].side = (int)(i & 1u);
        m->pixels[i].element = i < 4u ?
            DM1_V1_D0L2_D0R2_F0107_ELEMENT_CORRIDOR_PC34 :
            DM1_V1_D0L2_D0R2_F0107_ELEMENT_TELEPORTER_PC34;
        m->pixels[i].redmcsb_anchor =
            "DUNVIEW.C F0107:3922 C10_COLOR_FLESH transparent blit";
    }
}

uint32_t dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!model) return 0u;
    hash = fnv1a_u32(hash, (uint32_t)model->deterministic_seed);
    hash = fnv1a_u32(hash, (uint32_t)model->f0128_d0l2_before_d0r2);
    hash = fnv1a_u32(hash, (uint32_t)model->direct_f0107_call_count);
    hash = fnv1a_u32(hash, (uint32_t)model->f0107_candidate_call_count);
    for (i = 0; i < DM1_V1_D0L2_D0R2_F0107_SIDE_COUNT_PC34; ++i) {
        hash = fnv1a_u32(hash, (uint32_t)model->lanes[i].view_square);
        hash = fnv1a_u32(hash, (uint32_t)model->lanes[i].wall_zone);
        hash = fnv1a_u32(hash, (uint32_t)model->lanes[i].thing_pass_line);
    }
    for (i = 0; i < DM1_V1_D0L2_D0R2_F0107_ELEMENT_COUNT_PC34; ++i) {
        hash = fnv1a_u32(hash, (uint32_t)model->routes[i].element);
        hash = fnv1a_u32(hash, (uint32_t)model->routes[i].returns_before_tail);
        hash = fnv1a_u32(hash, (uint32_t)model->routes[i].has_thing_pass_tail);
    }
    for (i = 0; i < DM1_V1_D0L2_D0R2_F0107_CALL_COUNT_PC34; ++i) {
        hash = fnv1a_u32(hash, (uint32_t)model->calls[i].side);
        hash = fnv1a_u32(hash, (uint32_t)model->calls[i].element);
        hash = fnv1a_u32(hash, (uint32_t)model->calls[i].zone);
    }
    for (i = 0; i < DM1_V1_D0L2_D0R2_F0107_PIXEL_COUNT_PC34; ++i) {
        hash = fnv1a_u32(hash, model->pixels[i].before);
        hash = fnv1a_u32(hash, model->pixels[i].source);
        hash = fnv1a_u32(hash, model->pixels[i].after);
    }
    return hash;
}

bool dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    out_model->framebuffer_width = DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_WIDTH_PC34;
    out_model->framebuffer_height = DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_HEIGHT_PC34;
    out_model->viewport_width = DM1_V1_D0L2_D0R2_F0107_VIEWPORT_WIDTH_PC34;
    out_model->viewport_height = DM1_V1_D0L2_D0R2_F0107_VIEWPORT_HEIGHT_PC34;
    out_model->c10_transparent_color = DM1_V1_D0L2_D0R2_F0107_C10_COLOR_FLESH_PC34;
    out_model->wall_ornament_zone_base = DM1_C1004_ZONE_WALL_ORNAMENT;
    out_model->wall_ornament_zone_stride = DM1_WALL_ORNAMENT_ZONE_STRIDE;
    out_model->wall_ornament_coordinate_set = DM1_D0_WALL_ORNAMENT_COORDINATE_SET;
    out_model->d0l2_front_wall_zone =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(
            DM1_D0_WALL_ORNAMENT_COORDINATE_SET, DM1_M585_VIEW_WALL_D1L_RIGHT);
    out_model->d0r2_front_wall_zone =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(
            DM1_D0_WALL_ORNAMENT_COORDINATE_SET, DM1_M586_VIEW_WALL_D1R_LEFT);
    out_model->m550_first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    out_model->m551_right_wall_ornament_slot = DM1_M551_RIGHT_WALL_ORNAMENT_SLOT;
    out_model->m552_front_wall_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    out_model->m553_left_wall_ornament_slot = DM1_M553_LEFT_WALL_ORNAMENT_SLOT;
    out_model->f0128_d0l2_before_d0r2 = 1;
    out_model->f0128_after_f0116_f0117_wall_composition = 1;
    out_model->terminal_depth_side_pair_correction = 1;
    out_model->direct_f0107_call_count = 0;
    out_model->f0107_candidate_call_count = DM1_V1_D0L2_D0R2_F0107_CALL_COUNT_PC34;
    out_model->zero_ordinal_returns_false = 1;
    out_model->non_alcove_returns_false = 1;
    out_model->alcove_returns_true = 1;
    out_model->c0_to_c5_ordinals_pinned = 1;
    out_model->c10_transparent_preserves_destination = 1;
    out_model->field_level_byte_stability = 1;
    out_model->deterministic_seed = 0x1070d02;
    out_model->source_locked_contract_only = 1;
    out_model->no_original_dos_pixel_parity = 1;
    out_model->no_graphics_dat_reads = 1;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_lanes(out_model);
    fill_routes(out_model);
    fill_calls(out_model);
    fill_ordinals_pixels(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_hash_model_pc34(out_model);
    return true;
}

const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34(void)
{
    static DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 model;
    static int initialized;

    if (!initialized) {
        (void)dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_builder_pc34(&model);
        initialized = 1;
    }
    return &model;
}

uint32_t dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_deterministic_hash_pc34(void)
{
    return dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34()
        ->deterministic_hash;
}

const DM1_V1_D0L2D0R2F0107LanePc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_lane_at_pc34(size_t index)
{
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34();
    if (index >= DM1_V1_D0L2_D0R2_F0107_SIDE_COUNT_PC34) return NULL;
    return &model->lanes[index];
}

const DM1_V1_D0L2D0R2F0107ElementRoutePc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_route_at_pc34(size_t index)
{
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34();
    if (index >= DM1_V1_D0L2_D0R2_F0107_ELEMENT_COUNT_PC34) return NULL;
    return &model->routes[index];
}

const DM1_V1_D0L2D0R2F0107CallPc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_call_at_pc34(size_t index)
{
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34();
    if (index >= DM1_V1_D0L2_D0R2_F0107_CALL_COUNT_PC34) return NULL;
    return &model->calls[index];
}

const DM1_V1_D0L2D0R2F0107OrdinalPc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_ordinal_at_pc34(size_t index)
{
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34();
    if (index >= DM1_V1_D0L2_D0R2_F0107_ORDINAL_COUNT_PC34) return NULL;
    return &model->ordinals[index];
}

int dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_render_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size)
{
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int writes = 0;
    const size_t required =
        (size_t)DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_WIDTH_PC34 *
        (size_t)DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_HEIGHT_PC34;

    if (!framebuffer || framebuffer_size < required) return -1;
    memset(framebuffer, 0, required);
    for (i = 0; i < DM1_V1_D0L2_D0R2_F0107_PIXEL_COUNT_PC34; ++i) {
        const size_t x = i < 4u ? i : (size_t)DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_WIDTH_PC34 - i;
        const size_t y = 20u + i;
        const size_t offset =
            y * (size_t)DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_WIDTH_PC34 + x;
        framebuffer[offset] =
            dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_blend_pixel_pc34(
                model->pixels[i].before,
                model->pixels[i].source,
                DM1_V1_D0L2_D0R2_F0107_C10_COLOR_FLESH_PC34);
        if (framebuffer[offset] == model->pixels[i].after) ++writes;
    }
    return writes;
}

const char *dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
