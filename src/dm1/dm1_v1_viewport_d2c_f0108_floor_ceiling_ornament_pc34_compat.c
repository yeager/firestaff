#include "firestaff/dm1/v1/viewport/d2c_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_M603_VIEW_SQUARE_D2C = 6,
    DM1_M550_FIRST_THING = 2,
    DM1_M558_FLOOR_ORNAMENT_ORDINAL = 5,
    DM1_M583_VIEW_WALL_D2C_FRONT = 10,
    DM1_M592_VIEW_FLOOR_D2C = 6,
    DM1_C709_ZONE_WALL_D2C = 709,
    DM1_C705_ZONE_WALL_D3L = 705,
    DM1_C706_ZONE_WALL_D3R = 706,
    DM1_C865_ZONE_CEILING_PIT_D2C = 865,
    DM1_C1004_ZONE_WALL_ORNAMENT = 1004,
    DM1_C1500_ZONE_FLOOR_ORNAMENT = 1500,
    DM1_PC34_FLOOR_ZONE_STRIDE = 11,
    DM1_PC34_WALL_ORNAMENT_ZONE_STRIDE = 15,
    DM1_ORDER_DOOR_PASS1_BACKLEFT_BACKRIGHT = 0x0218,
    DM1_ORDER_DOOR_PASS2_FRONTLEFT_FRONTRIGHT = 0x0349,
    DM1_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT = 0x3421
};

/*
 * ReDMCSB source lock for this model:
 * - DUNVIEW.C F0108:3940-4011 owns floor-ornament ordinal clearing, the
 *   M592 D2C footprint flip branch, C10_COLOR_FLESH blit, and C1500 zone.
 * - DUNVIEW.C F0121:7244-7388 owns the D2C body: F0107 wall branch
 *   7289-7312; F0108 door-front call 7314 before F0115 pass 1 at 7315;
 *   open-route F0108/F0112/F0115/F0113 ordering at 7357-7386.
 * - DUNVIEW.C F0128:8511-8521 dispatches D2L, D2R, then D2C.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523 plus
 *   2666-2721 anchor square-aspect input, sensor M558, and first thing.
 */
static const char s_source_evidence[] =
    "DM1 V1 D2C F0108 contract-only source lock. ReDMCSB DUNVIEW.C "
    "F0108:3940-4011 owns nonzero floor-ornament ordinal dispatch, "
    "MASK0x8000_FOOTPRINTS recursion, M592_VIEW_FLOOR_D2C center-footprint "
    "flip at 3967-3980, C10_COLOR_FLESH transparent blits at 3989-4004, "
    "and PC34 C1500 + CoordinateSet * 11 + ViewFloor zone math at "
    "3998/4004. DUNVIEW.C F0121:7244-7388 is the D2C body: wall case "
    "7289-7312 calls F0107 at 7308 and returns, door-front calls F0108 at "
    "7314 before F0115 pass 1 at 7315, draws F0111 at 7336-7339, then "
    "uses pass 2 via 7341/7368; pit/corridor/teleporter/stairs-front use "
    "F0108 at 7357, F0112 ceiling pit at 7359-7365, F0115 at 7368, and "
    "teleporter F0113 at 7377-7386 after F0115. DUNVIEW.C F0128:8511-8521 "
    "dispatches D2L, D2R, then D2C. DUNVIEW.C F0107:3502-3938 is the "
    "separate wall-ornament branch with C1004 + CoordinateSet * 15 + "
    "ViewWall math. DUNVIEW.C F0115:4547-4581 and 4795-4800 anchor the "
    "ordered-cell nibble walk and L0175_i_DoorFrontViewDrawingPass two-pass "
    "door ordering. DUNGEON.C F0163:1769-1838, F0164:1840-1905, and "
    "F0172:2466-2523/2666-2721 anchor list and sensor aspect inputs. "
    "DEFS.H:2088,2547-2559,2669-2676,2678-2705,2739-2760,4045-4049,"
    "4188,4212,4223 anchor C10, M550..M558, cell orders, M575..M583, "
    "floor views, C705/C706/C709, D2C ceiling zones, and C1500.";

static const char s_disjointness_note[] =
    "This slice covers only the D2C F0108 floor+ceiling+ornament body for "
    "M603_VIEW_SQUARE_D2C/M592_VIEW_FLOOR_D2C. It intentionally excludes "
    "the D2C F0107 wall-ornament body, D0L/D0R and D3L/D3R F0108 siblings, "
    "F0111 door transparency, F0115 thing-pass pixel parity, chest/mirror "
    "runtime families, and CSB V1 lanes. Synthetic pixels stay inside the "
    "224x136 viewport and make no original DOS or real-asset parity claim.";

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> ((unsigned int)i * 8u)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

uint8_t dm1_v1_viewport_d2c_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D2C_F0108_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

int dm1_v1_viewport_d2c_f0108_floor_zone_pc34(
    int coordinate_set,
    int view_floor)
{
    return DM1_C1500_ZONE_FLOOR_ORNAMENT +
        coordinate_set * DM1_PC34_FLOOR_ZONE_STRIDE + view_floor;
}

int dm1_v1_viewport_d2c_f0108_f0107_wall_zone_pc34(
    int coordinate_set,
    int view_wall)
{
    return DM1_C1004_ZONE_WALL_ORNAMENT +
        coordinate_set * DM1_PC34_WALL_ORNAMENT_ZONE_STRIDE + view_wall;
}

static void fill_events(DM1_V1_D2CF0108ModelPc34 *m)
{
    static const DM1_V1_D2CF0108EventPc34 events[] = {
        { DM1_V1_D2C_F0108_EVENT_F0128_DISPATCH_PC34, 0, 8521,
          1, 1, 1, 1, "F0128 dispatches D2C after D2L/D2R",
          "DUNVIEW.C F0128:8511-8521" },
        { DM1_V1_D2C_F0108_EVENT_F0121_BODY_PC34, 1, 7244,
          1, 1, 1, 1, "F0121 D2C body",
          "DUNVIEW.C F0121:7244-7388" },
        { DM1_V1_D2C_F0108_EVENT_F0107_WALL_KEEP_OUT_PC34, 2, 7308,
          1, 0, 0, 0, "wall branch is F0107-only and returns",
          "DUNVIEW.C:7289-7312" },
        { DM1_V1_D2C_F0108_EVENT_F0108_FLOOR_PC34, 3, 7357,
          0, 1, 1, 1, "F0108 D2C floor ornament baseline",
          "DUNVIEW.C:7314 and 7357" },
        { DM1_V1_D2C_F0108_EVENT_F0112_CEILING_PC34, 4, 7359,
          0, 0, 1, 1, "F0112 ceiling pit after F0108",
          "DUNVIEW.C:7359-7365 before 7368" },
        { DM1_V1_D2C_F0108_EVENT_F0115_DOOR_PASS1_PC34, 5, 7315,
          0, 1, 0, 0, "door-front F0115 pass 1 after F0108",
          "DUNVIEW.C:7314-7315, F0115:4795-4800" },
        { DM1_V1_D2C_F0108_EVENT_F0111_DOOR_PC34, 6, 7336,
          0, 1, 0, 0, "F0111 door between F0115 passes",
          "DUNVIEW.C:7336-7339" },
        { DM1_V1_D2C_F0108_EVENT_F0115_DOOR_PASS2_PC34, 7, 7368,
          0, 1, 0, 0, "door-front F0115 pass 2 after F0111",
          "DUNVIEW.C:7341 and 7368" },
        { DM1_V1_D2C_F0108_EVENT_F0115_OPEN_PASS_PC34, 8, 7368,
          0, 0, 1, 1, "open-route F0115 after F0112",
          "DUNVIEW.C:7357-7368" },
        { DM1_V1_D2C_F0108_EVENT_F0113_FIELD_PC34, 9, 7377,
          0, 0, 0, 1, "teleporter field after F0115",
          "DUNVIEW.C:7377-7386" },
        { DM1_V1_D2C_F0108_EVENT_FRAMEBUFFER_PROBE_PC34, 10, 0,
          1, 1, 1, 1, "synthetic 320x200 framebuffer probe",
          "contract-only viewport probe" }
    };

    memcpy(m->events, events, sizeof(events));
}

static void fill_cell_orders(DM1_V1_D2CF0108ModelPc34 *m)
{
    m->cell_orders[0] = (DM1_V1_D2CF0108CellOrderPc34){
        DM1_ORDER_DOOR_PASS1_BACKLEFT_BACKRIGHT,
        { 8, 1, 2, 0 },
        3,
        1,
        "door pass 1 back-left/back-right",
        "DEFS.H:2669; DUNVIEW.C:7315; F0115:4561-4564/4795-4800"
    };
    m->cell_orders[1] = (DM1_V1_D2CF0108CellOrderPc34){
        DM1_ORDER_DOOR_PASS2_FRONTLEFT_FRONTRIGHT,
        { 9, 4, 3, 0 },
        3,
        2,
        "door pass 2 front-left/front-right",
        "DEFS.H:2672; DUNVIEW.C:7341/7368; F0115:4795-4800"
    };
    m->cell_orders[2] = (DM1_V1_D2CF0108CellOrderPc34){
        DM1_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT,
        { 1, 2, 4, 3 },
        4,
        0,
        "open route back-left/back-right/front-left/front-right",
        "DEFS.H:2676; DUNVIEW.C:7356/7368; F0115:4561-4564"
    };
}

static void fill_samples(DM1_V1_D2CF0108ModelPc34 *m)
{
    static const uint8_t before[] = { 0x20u, 0x21u, 0x22u, 0x23u, 0x24u, 0x25u, 0x26u, 0x27u };
    static const uint8_t source[] = { 0x51u, 10u, 0x52u, 10u, 0x53u, 0x54u, 10u, 0x55u };
    size_t i;

    for (i = 0; i < DM1_V1_D2C_F0108_SAMPLE_COUNT_PC34; ++i) {
        m->samples[i].before = before[i];
        m->samples[i].source = source[i];
        m->samples[i].after =
            dm1_v1_viewport_d2c_f0108_blend_c10_pc34(before[i], source[i]);
        m->samples[i].transparent_skip =
            source[i] == DM1_V1_D2C_F0108_C10_COLOR_FLESH_PC34;
    }
}

bool dm1_v1_viewport_d2c_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    int floor_flipped,
    DM1_V1_D2CF0108OrdinalPc34 *out)
{
    unsigned int cleared = floor_ornament_ordinal &
        ~DM1_V1_D2C_F0108_FOOTPRINT_MASK_PC34;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->input_ordinal = floor_ornament_ordinal;
    out->has_input_ordinal = floor_ornament_ordinal != 0u;
    out->footprint_flag_set =
        (floor_ornament_ordinal & DM1_V1_D2C_F0108_FOOTPRINT_MASK_PC34) != 0u;
    out->cleared_ordinal = cleared;
    out->primary_draws = cleared != 0u;
    out->primary_index = out->primary_draws ? (int)cleared - 1 : -1;
    out->recursive_footprints_draw = out->footprint_flag_set;
    out->recursive_footprints_index =
        out->footprint_flag_set ? DM1_V1_D2C_F0108_FOOTPRINT_INDEX_PC34 : -1;
    out->flips_on_d2c_when_floor_is_flipped =
        out->recursive_footprints_draw && floor_flipped;
    out->metadata_blit_count =
        (out->primary_draws ? 1 : 0) + (out->recursive_footprints_draw ? 1 : 0);
    return true;
}

bool dm1_v1_viewport_d2c_f0108_model_build_pc34(
    DM1_V1_D2CF0108ModelPc34 *out_model)
{
    DM1_V1_D2CF0108ModelPc34 *m = out_model;

    if (!m) return false;
    memset(m, 0, sizeof(*m));
    m->ok = 1;
    m->view_square_d2c = DM1_M603_VIEW_SQUARE_D2C;
    m->view_floor_d2c = DM1_M592_VIEW_FLOOR_D2C;
    m->first_thing_slot = DM1_M550_FIRST_THING;
    m->floor_ornament_slot = DM1_M558_FLOOR_ORNAMENT_ORDINAL;
    m->wall_zone_d2c = DM1_C709_ZONE_WALL_D2C;
    m->sibling_wall_zone_d3l = DM1_C705_ZONE_WALL_D3L;
    m->sibling_wall_zone_d3r = DM1_C706_ZONE_WALL_D3R;
    m->ceiling_zone_d2c_pc34 = DM1_C865_ZONE_CEILING_PIT_D2C;
    m->color_flesh = DM1_V1_D2C_F0108_C10_COLOR_FLESH_PC34;
    m->floor_zone_base = DM1_C1500_ZONE_FLOOR_ORNAMENT;
    m->f0108_start_line = 3940;
    m->f0108_end_line = 4011;
    m->f0121_start_line = 7244;
    m->f0121_end_line = 7388;
    m->f0128_d2c_update_line = 8520;
    m->f0128_d2c_draw_line = 8521;
    m->f0172_sensor_line = 2676;
    m->f0172_first_thing_line = 2721;
    m->zone_math = (DM1_V1_D2CF0108ZoneMathPc34){
        2,
        DM1_M592_VIEW_FLOOR_D2C,
        dm1_v1_viewport_d2c_f0108_floor_zone_pc34(2, DM1_M592_VIEW_FLOOR_D2C),
        DM1_M583_VIEW_WALL_D2C_FRONT,
        dm1_v1_viewport_d2c_f0108_f0107_wall_zone_pc34(
            2, DM1_M583_VIEW_WALL_D2C_FRONT),
        1
    };
    m->viewport = (DM1_V1_D2CF0108RectPc34){ 0, 0, 223, 135, "224x136 viewport" };
    m->ceiling_rect = (DM1_V1_D2CF0108RectPc34){ 48, 8, 175, 35, "D2C ceiling band" };
    m->floor_rect = (DM1_V1_D2CF0108RectPc34){ 32, 84, 191, 135, "D2C floor band" };
    m->ornament_rect = (DM1_V1_D2CF0108RectPc34){ 96, 103, 127, 127, "D2C F0108 ornament" };
    m->thing_rect = (DM1_V1_D2CF0108RectPc34){ 88, 88, 135, 123, "D2C F0115 thing pass sentinel" };
    m->field_rect = (DM1_V1_D2CF0108RectPc34){ 72, 40, 151, 119, "D2C F0113 teleporter field" };
    m->source_evidence = s_source_evidence;
    m->disjointness_note = s_disjointness_note;

    fill_events(m);
    fill_cell_orders(m);
    fill_samples(m);
    m->deterministic_hash = dm1_v1_viewport_d2c_f0108_hash_model_pc34(m);
    return true;
}

const DM1_V1_D2CF0108ModelPc34 *
dm1_v1_viewport_d2c_f0108_model_pc34(void)
{
    static DM1_V1_D2CF0108ModelPc34 model;
    static int initialized;

    if (!initialized) {
        (void)dm1_v1_viewport_d2c_f0108_model_build_pc34(&model);
        initialized = 1;
    }
    return &model;
}

const DM1_V1_D2CF0108EventPc34 *
dm1_v1_viewport_d2c_f0108_event_at_pc34(size_t index)
{
    const DM1_V1_D2CF0108ModelPc34 *model =
        dm1_v1_viewport_d2c_f0108_model_pc34();

    if (index >= DM1_V1_D2C_F0108_EVENT_COUNT_PC34) return NULL;
    return &model->events[index];
}

const DM1_V1_D2CF0108CellOrderPc34 *
dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(size_t index)
{
    const DM1_V1_D2CF0108ModelPc34 *model =
        dm1_v1_viewport_d2c_f0108_model_pc34();

    if (index >= DM1_V1_D2C_F0108_CELL_ORDER_COUNT_PC34) return NULL;
    return &model->cell_orders[index];
}

bool dm1_v1_viewport_d2c_f0108_initial_state_pc34(
    DM1_V1_D2CF0108ContextPc34 context,
    DM1_V1_D2CF0108StatePc34 *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->context = context;
    out->floor_ornament_ordinal = 3u;
    out->floor_flipped = 1;
    out->ceiling_pixel = 0x31u;
    out->floor_pixel = 0x32u;
    out->ornament_pixel = 0x43u;
    out->thing_pixel = 0x54u;
    out->field_pixel = 0x65u;
    return context == DM1_V1_D2C_F0108_CONTEXT_WALL_PC34 ||
        context == DM1_V1_D2C_F0108_CONTEXT_CORRIDOR_PC34 ||
        context == DM1_V1_D2C_F0108_CONTEXT_OPEN_PIT_PC34 ||
        context == DM1_V1_D2C_F0108_CONTEXT_TELEPORTER_PC34 ||
        context == DM1_V1_D2C_F0108_CONTEXT_DOOR_FRONT_PC34 ||
        context == DM1_V1_D2C_F0108_CONTEXT_STAIRS_FRONT_PC34;
}

static int rect_inside_viewport(const DM1_V1_D2CF0108RectPc34 *r)
{
    return r &&
        r->x1 >= 0 && r->y1 >= 0 &&
        r->x2 < DM1_V1_D2C_F0108_VIEWPORT_WIDTH_PC34 &&
        r->y2 < DM1_V1_D2C_F0108_VIEWPORT_HEIGHT_PC34 &&
        r->x1 <= r->x2 && r->y1 <= r->y2;
}

static size_t framebuffer_offset(int x, int y)
{
    return (size_t)y * (size_t)DM1_V1_D2C_F0108_FRAMEBUFFER_WIDTH_PC34 +
        (size_t)x;
}

static void draw_rect(
    uint8_t *framebuffer,
    const DM1_V1_D2CF0108RectPc34 *rect,
    uint8_t pixel,
    int use_c10,
    int *touched_pixels,
    int *transparent_skips)
{
    int y;

    if (!framebuffer || !rect) return;
    for (y = rect->y1; y <= rect->y2; ++y) {
        int x;
        for (x = rect->x1; x <= rect->x2; ++x) {
            uint8_t *dst = &framebuffer[framebuffer_offset(x, y)];
            if (use_c10 &&
                pixel == DM1_V1_D2C_F0108_C10_COLOR_FLESH_PC34) {
                ++(*transparent_skips);
            } else {
                *dst = dm1_v1_viewport_d2c_f0108_blend_c10_pc34(*dst, pixel);
                ++(*touched_pixels);
            }
        }
    }
}

static uint32_t hash_framebuffer(const uint8_t *framebuffer, size_t len)
{
    uint32_t h = 2166136261u;
    size_t i;

    for (i = 0; i < len; ++i) {
        h ^= framebuffer[i];
        h *= 16777619u;
    }
    return h;
}

static void draw_floor_and_ornament(
    uint8_t *framebuffer,
    const DM1_V1_D2CF0108ModelPc34 *model,
    const DM1_V1_D2CF0108StatePc34 *state,
    DM1_V1_D2CF0108ResultPc34 *out)
{
    DM1_V1_D2CF0108OrdinalPc34 ordinal;

    out->f0108_calls = 1;
    out->floor_zone = model->zone_math.f0108_zone;
    draw_rect(framebuffer, &model->floor_rect, state->floor_pixel, 1,
              &out->touched_pixels, &out->transparent_skips);
    (void)dm1_v1_viewport_d2c_f0108_decode_ordinal_pc34(
        state->floor_ornament_ordinal, state->floor_flipped, &ordinal);
    if (ordinal.primary_draws || ordinal.recursive_footprints_draw) {
        draw_rect(framebuffer, &model->ornament_rect, state->ornament_pixel, 1,
                  &out->touched_pixels, &out->transparent_skips);
    }
}

bool dm1_v1_viewport_d2c_f0108_compose_pc34(
    const DM1_V1_D2CF0108StatePc34 *state,
    DM1_V1_D2CF0108ResultPc34 *out)
{
    uint8_t framebuffer[
        DM1_V1_D2C_F0108_FRAMEBUFFER_WIDTH_PC34 *
        DM1_V1_D2C_F0108_FRAMEBUFFER_HEIGHT_PC34];
    const DM1_V1_D2CF0108ModelPc34 *model =
        dm1_v1_viewport_d2c_f0108_model_pc34();

    if (!state || !out) return false;
    memset(out, 0, sizeof(*out));
    memset(framebuffer, 0xee, sizeof(framebuffer));

    out->framebuffer_width = DM1_V1_D2C_F0108_FRAMEBUFFER_WIDTH_PC34;
    out->framebuffer_height = DM1_V1_D2C_F0108_FRAMEBUFFER_HEIGHT_PC34;
    out->viewport_width = DM1_V1_D2C_F0108_VIEWPORT_WIDTH_PC34;
    out->viewport_height = DM1_V1_D2C_F0108_VIEWPORT_HEIGHT_PC34;
    out->f0128_d2l_d2r_before_d2c = 1;
    out->f0128_d2c_before_d1_d0 = 1;
    out->terminal_depth_side_pair_correction = 1;
    out->thing_list_mutation_guard_ok = !state->mutate_thing_list;
    out->non_overlap_ok =
        !state->allow_f0107_wall_duplicate && !state->allow_f0111_only_route;
    out->f0107_contrast_zone = model->zone_math.f0107_zone;
    out->rejected_non_contract_state =
        state->mutate_thing_list ||
        state->allow_outside_viewport ||
        state->allow_f0107_wall_duplicate ||
        state->allow_f0111_only_route;
    if (out->rejected_non_contract_state) {
        out->ok = 0;
        return true;
    }
    if (!rect_inside_viewport(&model->viewport) ||
        !rect_inside_viewport(&model->ceiling_rect) ||
        !rect_inside_viewport(&model->floor_rect) ||
        !rect_inside_viewport(&model->ornament_rect) ||
        !rect_inside_viewport(&model->thing_rect) ||
        !rect_inside_viewport(&model->field_rect)) {
        out->ok = 0;
        return true;
    }

    switch (state->context) {
    case DM1_V1_D2C_F0108_CONTEXT_WALL_PC34:
        out->wall_f0107_calls = 1;
        break;
    case DM1_V1_D2C_F0108_CONTEXT_DOOR_FRONT_PC34:
        draw_floor_and_ornament(framebuffer, model, state, out);
        out->door_f0108_before_f0115_pass1 = 1;
        out->door_pass1_before_f0111 = 1;
        out->door_pass2_after_f0111 = 1;
        out->f0115_calls = 2;
        out->f0111_calls = 1;
        draw_rect(framebuffer, &model->thing_rect, state->thing_pixel, 1,
                  &out->touched_pixels, &out->transparent_skips);
        break;
    case DM1_V1_D2C_F0108_CONTEXT_OPEN_PIT_PC34:
        out->open_pit_still_draws_floor_ornament = 1;
        /* Fall through: ReDMCSB DUNVIEW.C F0121:7353-7357. */
    case DM1_V1_D2C_F0108_CONTEXT_CORRIDOR_PC34:
    case DM1_V1_D2C_F0108_CONTEXT_STAIRS_FRONT_PC34:
    case DM1_V1_D2C_F0108_CONTEXT_TELEPORTER_PC34:
        draw_floor_and_ornament(framebuffer, model, state, out);
        out->f0112_calls = 1;
        out->open_route_f0112_before_f0115 = 1;
        draw_rect(framebuffer, &model->ceiling_rect, state->ceiling_pixel, 1,
                  &out->touched_pixels, &out->transparent_skips);
        out->f0115_calls = 1;
        draw_rect(framebuffer, &model->thing_rect, state->thing_pixel, 1,
                  &out->touched_pixels, &out->transparent_skips);
        if (state->context == DM1_V1_D2C_F0108_CONTEXT_TELEPORTER_PC34) {
            out->f0113_calls = 1;
            out->teleporter_f0113_after_f0115 = 1;
            draw_rect(framebuffer, &model->field_rect, state->field_pixel, 1,
                      &out->touched_pixels, &out->transparent_skips);
        }
        break;
    default:
        out->rejected_non_contract_state = 1;
        out->ok = 0;
        return true;
    }

    out->ceiling_sample = framebuffer[framebuffer_offset(48, 8)];
    out->floor_sample = framebuffer[framebuffer_offset(40, 100)];
    out->ornament_sample = framebuffer[framebuffer_offset(100, 110)];
    out->thing_sample = framebuffer[framebuffer_offset(90, 90)];
    out->field_sample = framebuffer[framebuffer_offset(74, 42)];
    out->framebuffer_hash = hash_framebuffer(framebuffer, sizeof(framebuffer));
    out->deterministic_hash = hash_u32(model->deterministic_hash, out->framebuffer_hash);
    out->ok = 1;
    return true;
}

uint32_t dm1_v1_viewport_d2c_f0108_hash_model_pc34(
    const DM1_V1_D2CF0108ModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = hash_u32(h, (uint32_t)model->view_square_d2c);
    h = hash_u32(h, (uint32_t)model->view_floor_d2c);
    h = hash_u32(h, (uint32_t)model->first_thing_slot);
    h = hash_u32(h, (uint32_t)model->floor_ornament_slot);
    h = hash_u32(h, (uint32_t)model->wall_zone_d2c);
    h = hash_u32(h, (uint32_t)model->sibling_wall_zone_d3l);
    h = hash_u32(h, (uint32_t)model->sibling_wall_zone_d3r);
    h = hash_u32(h, (uint32_t)model->ceiling_zone_d2c_pc34);
    h = hash_u32(h, (uint32_t)model->f0108_start_line);
    h = hash_u32(h, (uint32_t)model->f0108_end_line);
    h = hash_u32(h, (uint32_t)model->f0121_start_line);
    h = hash_u32(h, (uint32_t)model->f0121_end_line);
    h = hash_u32(h, (uint32_t)model->f0128_d2c_update_line);
    h = hash_u32(h, (uint32_t)model->f0128_d2c_draw_line);
    h = hash_u32(h, (uint32_t)model->zone_math.f0108_zone);
    h = hash_u32(h, (uint32_t)model->zone_math.f0107_zone);
    for (i = 0; i < DM1_V1_D2C_F0108_EVENT_COUNT_PC34; ++i) {
        h = hash_u32(h, (uint32_t)model->events[i].kind);
        h = hash_u32(h, (uint32_t)model->events[i].order_index);
        h = hash_u32(h, (uint32_t)model->events[i].redmcsb_line);
        h = hash_u32(h, (uint32_t)model->events[i].expected_for_wall);
        h = hash_u32(h, (uint32_t)model->events[i].expected_for_door_front);
        h = hash_u32(h, (uint32_t)model->events[i].expected_for_open_route);
        h = hash_u32(h, (uint32_t)model->events[i].expected_for_teleporter);
    }
    for (i = 0; i < DM1_V1_D2C_F0108_CELL_ORDER_COUNT_PC34; ++i) {
        h = hash_u32(h, (uint32_t)model->cell_orders[i].order_value);
        h = hash_u32(h, (uint32_t)model->cell_orders[i].decoded_count);
        h = hash_u32(h, (uint32_t)model->cell_orders[i].door_front_pass);
    }
    return h;
}

uint32_t dm1_v1_viewport_d2c_f0108_deterministic_hash_pc34(void)
{
    return dm1_v1_viewport_d2c_f0108_model_pc34()->deterministic_hash;
}

const char *dm1_v1_viewport_d2c_f0108_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d2c_f0108_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
