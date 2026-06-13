#include "firestaff/dm1/v1/viewport/d3l2_d3r2_f0108_wall_composition_pc34_compat.h"

#include <string.h>

enum {
    DM1_C14_VIEW_SQUARE_D3L2 = 14,
    DM1_C15_VIEW_SQUARE_D3R2 = 15,
    DM1_C00_VIEW_FLOOR_D3L2 = 0,
    DM1_C01_VIEW_FLOOR_D3R2 = 1,
    DM1_C00_VIEW_WALL_D3L2_RIGHT = 0,
    DM1_C01_VIEW_WALL_D3R2_LEFT = 1,
    DM1_C702_ZONE_WALL_D3L2 = 702,
    DM1_C703_ZONE_WALL_D3R2 = 703,
    DM1_C704_ZONE_WALL_D3C = 704,
    DM1_C716_ZONE_WALL_D0L = 716,
    DM1_C717_ZONE_WALL_D0R = 717,
    DM1_M550_FIRST_THING = 2,
    DM1_M551_RIGHT_WALL_ORNAMENT_ORDINAL = 4,
    DM1_M552_FRONT_WALL_ORNAMENT_ORDINAL = 5,
    DM1_M553_LEFT_WALL_ORNAMENT_ORDINAL = 6,
    DM1_M556_DOOR_STATE = 3,
    DM1_M557_DOOR_THING_INDEX = 4,
    DM1_M558_FLOOR_ORNAMENT_ORDINAL = 5,
    DM1_C3700_ZONE_DOOR_D3L2 = 3700,
    DM1_C3710_ZONE_DOOR_D3R2 = 3710
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0676:6226-6291 and F0677:6293-6358 are the D3L2/D3R2
 *   second side-pair bodies reached from F0128:8478-8508. They sit in the
 *   D3 row before D3L/D3R via F0116:6361-6480 and F0117:6500-6622.
 * - DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 are the native and
 *   horizontally flipped bitmap helpers used by the wall, stair, and pit
 *   branches; both keep the C10 transparency convention that the route
 *   model below pins without claiming real-asset pixel parity.
 * - DUNVIEW.C F0107:3502-3938 is the wall-ornament branch reached by
 *   F0676/F0677 wall squares. The wall branch returns before F0108, so this
 *   gate records it as a keepout rather than duplicating F0107 coverage.
 * - DUNVIEW.C F0108:3940-4011 owns nonzero floor-ornament ordinals,
 *   MASK0x8000_FOOTPRINTS clearing and recursion, PC34 C1500 + set * 11
 *   + view-floor zone math, and C10_COLOR_FLESH transparent blitting.
 * - DUNVIEW.C F0111:4218-4339 owns door drawing; this gate only pins that
 *   its C10 transparent blit participates between F0115 pass 1 and pass 2.
 * - DUNVIEW.C F0115:4794-4800 computes the two door-front drawing passes
 *   from the low nibble, and F0115:5180-5188 shows C10 transparency for
 *   item/object/projectile blits.
 * - DUNGEON.C F0163:1769-1838 and F0164:1840-1905 are mutation anchors;
 *   DUNGEON.C F0172:2466-2523 starts square-aspect classification.
 * - DEFS.H:2088, 2443, 2450, 2582-2583, 2603-2604, 2610-2611, 2662,
 *   2676-2677, 4139-4153, and 4197-4198 anchor C10, D2/D3 view squares,
 *   cell-order constants, D3L2/D3R2 stair/pit zones, and nearby helpers.
 */
static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0676:6226-6291 and DUNVIEW.C F0677:6293-6358 "
    "define the D3L2/D3R2 second side-pair bodies. DUNVIEW.C "
    "F0128:8478-8508 dispatches D3L2/D3R2 before D3L/D3R/D3C and D2L2/D2R2. "
    "DUNVIEW.C F0116:6361-6480 and DUNVIEW.C F0117:6500-6622 are the "
    "neighboring D3L/D3R side-wall contrast and are not claimed here. "
    "DUNVIEW.C F0104:3113-3156 and DUNVIEW.C F0105:3185-3247 anchor native "
    "and flipped C10 bitmap helpers. DUNVIEW.C F0107:3502-3938 is the wall "
    "ornament branch reached by D3L2/D3R2 wall squares and kept disjoint. "
    "DUNVIEW.C F0108:3940-4011 pins floor ornament "
    "ordinal handling, MASK0x8000_FOOTPRINTS recursion, C10_COLOR_FLESH "
    "transparency, and PC34 zone math. DUNVIEW.C F0111:4218-4339 is only "
    "claimed here for its C10 transparent door layer in the D3L2/D3R2 "
    "door-front sequence at F0676:6270-6273 and F0677:6337-6340. "
    "DUNVIEW.C F0115:4794-4800 pins L0175_i_DoorFrontViewDrawingPass two "
    "passes and F0115:5180-5188 pins C10 item/object transparency. "
    "DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523 "
    "anchor thing-list and square-aspect inputs. DEFS.H:2088 anchors "
    "C10_COLOR_FLESH, DEFS.H:2443 and DEFS.H:2450 anchor D3L-source stair bitmap "
    "ordinals, DEFS.H:2582-2583 and DEFS.H:2603-2604 keep D2L/D2R view "
    "squares separate from this D3L2/D3R2 gate, DEFS.H:2610-2611 anchors "
    "C14/C15 view squares, DEFS.H:2662 and DEFS.H:2676-2677 anchor cell "
    "orders, DEFS.H:4139-4153 anchors C800/C801/C813/C814 stair zones, "
    "and DEFS.H:4197-4198 anchors C850/C851 pit zones.";

static const char s_disjointness_note[] =
    "Contract-only D3L2/D3R2 F0108 wall-composition gate. It is disjoint "
    "from pass777 D3C F0107 (commit 83db35b76) by row-order slot, lateral "
    "-2/+2 versus center 0, wall zones C702/C703 versus C704, and by "
    "claiming F0108/F0111/F0115 composition rather than the D3C F0107 "
    "front-wall ornament. It is disjoint from D0L2/D0R2 F0107 (commit "
    "cc6b81b59) by depth 3 versus terminal depth 0, F0676/F0677 versus "
    "F0125/F0126, and C702/C703/C3700/C3710 versus C716/C717.";

static uint64_t fnv1a_u64(uint64_t hash, uint64_t value)
{
    int i;

    for (i = 0; i < 8; ++i) {
        hash ^= (value >> (unsigned)(i * 8)) & 0xffu;
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

uint8_t dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1V1D3L2D3R2F0108WallCompositionOrdinalPc34 *out)
{
    unsigned int cleared;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->input_ordinal = floor_ornament_ordinal;
    out->primary_index = -1;
    out->recursive_footprints_index = -1;
    out->has_input_ordinal = floor_ornament_ordinal != 0u;
    if (!out->has_input_ordinal) return true;

    out->footprint_flag_set =
        (floor_ornament_ordinal &
         DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal &
        ~DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_ordinal = out->footprint_flag_set ? cleared : floor_ornament_ordinal;
        out->primary_index = (int)out->primary_ordinal - 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index =
            DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FOOTPRINT_INDEX_PC34;
        out->recursive_footprints_ordinal =
            (unsigned int)DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FOOTPRINT_INDEX_PC34 + 1u;
    }
    return true;
}

bool dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pixel_case_pc34(
    DM1V1D3L2D3R2F0108WallCompositionPixelPc34 *pixel)
{
    uint8_t out;

    if (!pixel) return false;
    out = pixel->before;
    pixel->f0108_transparent =
        pixel->f0108_source == DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34;
    out = dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
        out, pixel->f0108_source,
        DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34);
    pixel->after_f0108 = out;

    pixel->pass1_transparent =
        pixel->pass1_source == DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34;
    out = dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
        out, pixel->pass1_source,
        DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34);
    pixel->after_pass1 = out;

    if (pixel->door_front_sequence) {
        pixel->f0111_transparent =
            pixel->f0111_source == DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34;
        out = dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
            out, pixel->f0111_source,
            DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34);
        pixel->after_f0111 = out;

        pixel->pass2_transparent =
            pixel->pass2_source == DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34;
        out = dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
            out, pixel->pass2_source,
            DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34);
        pixel->after_pass2 = out;
    } else {
        pixel->after_f0111 = out;
        pixel->after_pass2 = out;
    }
    return true;
}

static void fill_specs(DM1V1D3L2D3R2F0108WallCompositionModelPc34 *m)
{
    m->specs[0] = (DM1V1D3L2D3R2F0108WallCompositionSpecPc34){
        DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_D3L2_PC34,
        "D3L2", "F0676_DrawD3L2",
        3, -2, 0, 8481, 8482, 6226, 6291,
        DM1_C14_VIEW_SQUARE_D3L2, DM1_C00_VIEW_FLOOR_D3L2,
        DM1_C702_ZONE_WALL_D3L2, DM1_C00_VIEW_WALL_D3L2_RIGHT,
        DM1_M551_RIGHT_WALL_ORNAMENT_ORDINAL, DM1_M558_FLOOR_ORNAMENT_ORDINAL,
        DM1_M556_DOOR_STATE, DM1_M557_DOOR_THING_INDEX, DM1_C3700_ZONE_DOOR_D3L2,
        0x3421u, 0x0321u, 0x0218u, 0x0349u,
        6284, 6270, 6271, 6272, 6286,
        false, true, true, true, true, true, true,
        "DUNVIEW.C F0676:6226-6291",
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0111:4218-4339 and F0676:6272",
        "DUNVIEW.C F0115:4794-4800/5180-5188 and F0676:6271/6286",
        "DUNVIEW.C F0128:8478-8499",
        "DEFS.H:2534-2561/2696-2701/4040-4048/4249-4251"
    };
    m->specs[1] = (DM1V1D3L2D3R2F0108WallCompositionSpecPc34){
        DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_D3R2_PC34,
        "D3R2", "F0677_DrawD3R2",
        3, 2, 1, 8485, 8486, 6293, 6358,
        DM1_C15_VIEW_SQUARE_D3R2, DM1_C01_VIEW_FLOOR_D3R2,
        DM1_C703_ZONE_WALL_D3R2, DM1_C01_VIEW_WALL_D3R2_LEFT,
        DM1_M553_LEFT_WALL_ORNAMENT_ORDINAL, DM1_M558_FLOOR_ORNAMENT_ORDINAL,
        DM1_M556_DOOR_STATE, DM1_M557_DOOR_THING_INDEX, DM1_C3710_ZONE_DOOR_D3R2,
        0x4312u, 0x0412u, 0x0128u, 0x0439u,
        6351, 6337, 6338, 6339, 6353,
        true, true, true, true, true, true, true,
        "DUNVIEW.C F0677:6293-6358",
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0111:4218-4339 and F0677:6339",
        "DUNVIEW.C F0115:4794-4800/5180-5188 and F0677:6338/6353",
        "DUNVIEW.C F0128:8478-8499",
        "DEFS.H:2534-2561/2696-2701/4040-4048/4249-4251"
    };
}

static DM1V1D3L2D3R2F0108WallCompositionRoutePc34 make_route(
    const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *s,
    DM1V1D3L2D3R2F0108WallCompositionElementPc34 element,
    const char *name,
    int route_index,
    int start_line,
    int end_line,
    bool wall_f0107,
    bool f0108,
    bool pass1,
    bool f0111,
    bool pass2,
    bool returns_after_wall,
    bool field_tail,
    unsigned int cell_order,
    const char *anchor)
{
    DM1V1D3L2D3R2F0108WallCompositionRoutePc34 route;

    memset(&route, 0, sizeof(route));
    route.side = s->side;
    route.element = element;
    route.element_name = name;
    route.route_index = route_index;
    route.route_start_line = start_line;
    route.route_end_line = end_line;
    route.supported_by_f067x = true;
    route.calls_wall_f0107 = wall_f0107;
    route.calls_f0108 = f0108;
    route.calls_f0115_pass1 = pass1;
    route.calls_f0111 = f0111;
    route.calls_f0115_pass2 = pass2;
    route.returns_after_wall = returns_after_wall;
    route.field_tail_after_teleporter = field_tail;
    route.d3c_f0107_keepout = true;
    route.d0l2_d0r2_f0107_keepout = true;
    route.cell_order = cell_order;
    route.redmcsb_anchor = anchor;
    return route;
}

static void fill_routes(DM1V1D3L2D3R2F0108WallCompositionModelPc34 *m)
{
    const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *l = &m->specs[0];
    const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *r = &m->specs[1];
    int i = 0;

    m->routes[i++] = make_route(l, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_WALL_PC34,
        "WALL", 0, 6253, 6264, true, false, false, false, false, true, false, 0u,
        "DUNVIEW.C F0676:6253-6264 wall branch calls F0107 then returns");
    m->routes[i++] = make_route(l, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_CORRIDOR_PC34,
        "CORRIDOR", 1, 6280, 6286, false, true, false, false, true, false, false, l->open_cell_order,
        "DUNVIEW.C F0676:6280-6286 corridor F0108 then F0115 tail");
    m->routes[i++] = make_route(l, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_PIT_PC34,
        "PIT", 2, 6275, 6286, false, true, false, false, true, false, false, l->open_cell_order,
        "DUNVIEW.C F0676:6275-6286 pit fall-through to F0108/F0115");
    m->routes[i++] = make_route(l, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_TELEPORTER_PC34,
        "TELEPORTER", 3, 6279, 6290, false, true, false, false, true, false, true, l->open_cell_order,
        "DUNVIEW.C F0676:6279-6290 teleporter F0108/F0115/field tail");
    m->routes[i++] = make_route(l, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_DOOR_SIDE_PC34,
        "DOOR_SIDE", 4, 6265, 6286, false, true, false, false, true, false, false, l->side_cell_order,
        "DUNVIEW.C F0676:6265-6286 door-side order then F0108/F0115");
    m->routes[i++] = make_route(l, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_STAIRS_SIDE_PC34,
        "STAIRS_SIDE", 5, 6265, 6286, false, true, false, false, true, false, false, l->side_cell_order,
        "DUNVIEW.C F0676:6265-6286 stairs-side order then F0108/F0115");
    m->routes[i++] = make_route(l, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_STAIRS_FRONT_PC34,
        "STAIRS_FRONT", 6, 6237, 6286, false, true, false, false, true, false, false, l->open_cell_order,
        "DUNVIEW.C F0676:6237-6286 stairs-front jumps to F0108/F0115");
    m->routes[i++] = make_route(l, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_DOOR_FRONT_PC34,
        "DOOR_FRONT", 7, 6269, 6286, false, true, true, true, true, false, false, l->door_pass2_cell_order,
        "DUNVIEW.C F0676:6269-6286 door-front F0108/pass1/F0111/pass2");

    m->routes[i++] = make_route(r, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_WALL_PC34,
        "WALL", 8, 6320, 6331, true, false, false, false, false, true, false, 0u,
        "DUNVIEW.C F0677:6320-6331 wall branch calls F0107 then returns");
    m->routes[i++] = make_route(r, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_CORRIDOR_PC34,
        "CORRIDOR", 9, 6347, 6353, false, true, false, false, true, false, false, r->open_cell_order,
        "DUNVIEW.C F0677:6347-6353 corridor F0108 then F0115 tail");
    m->routes[i++] = make_route(r, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_PIT_PC34,
        "PIT", 10, 6342, 6353, false, true, false, false, true, false, false, r->open_cell_order,
        "DUNVIEW.C F0677:6342-6353 pit fall-through to F0108/F0115");
    m->routes[i++] = make_route(r, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_TELEPORTER_PC34,
        "TELEPORTER", 11, 6346, 6356, false, true, false, false, true, false, true, r->open_cell_order,
        "DUNVIEW.C F0677:6346-6356 teleporter F0108/F0115/field tail");
    m->routes[i++] = make_route(r, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_DOOR_SIDE_PC34,
        "DOOR_SIDE", 12, 6332, 6353, false, true, false, false, true, false, false, r->side_cell_order,
        "DUNVIEW.C F0677:6332-6353 door-side order then F0108/F0115");
    m->routes[i++] = make_route(r, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_STAIRS_SIDE_PC34,
        "STAIRS_SIDE", 13, 6332, 6353, false, true, false, false, true, false, false, r->side_cell_order,
        "DUNVIEW.C F0677:6332-6353 stairs-side order then F0108/F0115");
    m->routes[i++] = make_route(r, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_STAIRS_FRONT_PC34,
        "STAIRS_FRONT", 14, 6304, 6353, false, true, false, false, true, false, false, r->open_cell_order,
        "DUNVIEW.C F0677:6304-6353 stairs-front jumps to F0108/F0115");
    m->routes[i++] = make_route(r, DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_DOOR_FRONT_PC34,
        "DOOR_FRONT", 15, 6336, 6353, false, true, true, true, true, false, false, r->door_pass2_cell_order,
        "DUNVIEW.C F0677:6336-6353 door-front F0108/pass1/F0111/pass2");
}

static void fill_pixels(DM1V1D3L2D3R2F0108WallCompositionModelPc34 *m)
{
    static const uint8_t before[] = {
        0x20u, 0x21u, 0x22u, 0x23u, 0x24u, 0x25u, 0x26u, 0x27u,
        0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u
    };
    static const uint8_t f0108[] = {
        0x40u, 10u, 0x41u, 10u, 0x42u, 0x43u, 10u, 0x44u,
        0x50u, 10u, 0x51u, 10u, 0x52u, 0x53u, 10u, 0x54u
    };
    static const uint8_t pass1[] = {
        10u, 10u, 0x45u, 0x46u, 10u, 0x47u, 0x48u, 0x49u,
        10u, 10u, 0x55u, 0x56u, 10u, 0x57u, 0x58u, 0x59u
    };
    static const uint8_t door[] = {
        10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u,
        10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u
    };
    static const uint8_t pass2[] = {
        10u, 10u, 10u, 10u, 10u, 10u, 10u, 0x4bu,
        10u, 10u, 10u, 10u, 10u, 10u, 10u, 0x5bu
    };
    size_t i;

    for (i = 0; i < DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_PIXEL_COUNT_PC34; ++i) {
        DM1V1D3L2D3R2F0108WallCompositionPixelPc34 pixel;
        memset(&pixel, 0, sizeof(pixel));
        pixel.pixel_index = (int)i;
        pixel.side = i < 8u ?
            DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_D3L2_PC34 :
            DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_D3R2_PC34;
        pixel.element = (i % 8u) == 7u ?
            DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_DOOR_FRONT_PC34 :
            DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_CORRIDOR_PC34;
        pixel.before = before[i];
        pixel.f0108_source = f0108[i];
        pixel.pass1_source = pass1[i];
        pixel.f0111_source = door[i];
        pixel.pass2_source = pass2[i];
        pixel.door_front_sequence =
            pixel.element == DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_DOOR_FRONT_PC34;
        pixel.redmcsb_anchor = pixel.door_front_sequence ?
            "DUNVIEW.C F0676/F0677 door-front F0108/pass1/F0111/pass2 C10 flow" :
            "DUNVIEW.C F0676/F0677 open-row F0108 then F0115 C10 flow";
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pixel_case_pc34(&pixel);
        m->pixels[i] = pixel;
    }
}

static void fill_rejected(DM1V1D3L2D3R2F0108WallCompositionModelPc34 *m)
{
    m->rejected[0] = (DM1V1D3L2D3R2F0108WallCompositionRejectedPc34){
        "D3C F0107 pass777 keepout",
        3, 0, DM1_C704_ZONE_WALL_D3C, DM1_C704_ZONE_WALL_D3C, 11, 11, 1,
        "D3C is center-lateral, C704, F0118/F0107, and commit 83db35b76 already pins it."
    };
    m->rejected[1] = (DM1V1D3L2D3R2F0108WallCompositionRejectedPc34){
        "D0L2 F0107 cc6b81b59 keepout",
        0, -1, DM1_C716_ZONE_WALL_D0L, DM1_C716_ZONE_WALL_D0L, 1, 1, 1,
        "D0L2 is terminal depth 0, F0125, C716, and commit cc6b81b59 already pins it."
    };
    m->rejected[2] = (DM1V1D3L2D3R2F0108WallCompositionRejectedPc34){
        "D0R2 F0107 cc6b81b59 keepout",
        0, 1, DM1_C717_ZONE_WALL_D0R, DM1_C717_ZONE_WALL_D0R, 2, 2, 1,
        "D0R2 is terminal depth 0, F0126, C717, and commit cc6b81b59 already pins it."
    };
}

uint64_t dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_hash_model_pc34(
    const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *model)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;

    if (!model) return 0u;
    hash = fnv1a_u64(hash, (uint64_t)model->framebuffer_width);
    hash = fnv1a_u64(hash, (uint64_t)model->framebuffer_height);
    hash = fnv1a_u64(hash, (uint64_t)model->viewport_width);
    hash = fnv1a_u64(hash, (uint64_t)model->viewport_height);
    hash = fnv1a_u64(hash, (uint64_t)model->c10_transparent_color);
    hash = fnv1a_u64(hash, (uint64_t)model->d3_row_depth);
    hash = fnv1a_u64(hash, (uint64_t)model->d3l2_f0128_order);
    hash = fnv1a_u64(hash, (uint64_t)model->d3r2_f0128_order);
    hash = fnv1a_u64(hash, (uint64_t)model->d3c_f0128_order);
    hash = fnv1a_u64(hash, (uint64_t)model->door_front_view_drawing_pass_first);
    hash = fnv1a_u64(hash, (uint64_t)model->door_front_view_drawing_pass_second);
    for (i = 0; i < DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_COUNT_PC34; ++i) {
        const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *s = &model->specs[i];
        hash = fnv1a_u64(hash, (uint64_t)s->side);
        hash = fnv1a_u64(hash, (uint64_t)s->f0128_draw_line);
        hash = fnv1a_u64(hash, (uint64_t)s->view_square);
        hash = fnv1a_u64(hash, (uint64_t)s->view_floor);
        hash = fnv1a_u64(hash, (uint64_t)s->wall_zone);
        hash = fnv1a_u64(hash, (uint64_t)s->wall_view);
        hash = fnv1a_u64(hash, (uint64_t)s->door_zone);
        hash = fnv1a_u64(hash, (uint64_t)s->open_cell_order);
        hash = fnv1a_u64(hash, (uint64_t)s->door_pass1_cell_order);
        hash = fnv1a_u64(hash, (uint64_t)s->door_pass2_cell_order);
    }
    for (i = 0; i < DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ROUTE_COUNT_PC34; ++i) {
        const DM1V1D3L2D3R2F0108WallCompositionRoutePc34 *r = &model->routes[i];
        hash = fnv1a_u64(hash, (uint64_t)r->side);
        hash = fnv1a_u64(hash, (uint64_t)r->element);
        hash = fnv1a_u64(hash, (uint64_t)r->calls_f0108);
        hash = fnv1a_u64(hash, (uint64_t)r->calls_f0111);
        hash = fnv1a_u64(hash, (uint64_t)r->returns_after_wall);
        hash = fnv1a_u64(hash, (uint64_t)r->cell_order);
    }
    for (i = 0; i < DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_PIXEL_COUNT_PC34; ++i) {
        const DM1V1D3L2D3R2F0108WallCompositionPixelPc34 *p = &model->pixels[i];
        hash = fnv1a_u64(hash, (uint64_t)p->before);
        hash = fnv1a_u64(hash, (uint64_t)p->f0108_source);
        hash = fnv1a_u64(hash, (uint64_t)p->pass1_source);
        hash = fnv1a_u64(hash, (uint64_t)p->f0111_source);
        hash = fnv1a_u64(hash, (uint64_t)p->pass2_source);
        hash = fnv1a_u64(hash, (uint64_t)p->after_pass2);
    }
    for (i = 0; i < DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_REJECTED_COUNT_PC34; ++i) {
        const DM1V1D3L2D3R2F0108WallCompositionRejectedPc34 *r = &model->rejected[i];
        hash = fnv1a_u64(hash, (uint64_t)r->relative_depth);
        hash = fnv1a_u64(hash, (uint64_t)r->relative_lateral);
        hash = fnv1a_u64(hash, (uint64_t)r->wall_zone_first);
        hash = fnv1a_u64(hash, (uint64_t)r->wall_zone_last);
        hash = fnv1a_u64(hash, (uint64_t)r->f0107_owner_commit_pinned);
    }
    return hash;
}

bool dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_builder_pc34(
    DM1V1D3L2D3R2F0108WallCompositionModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    out_model->framebuffer_width = DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FRAMEBUFFER_WIDTH_PC34;
    out_model->framebuffer_height = DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FRAMEBUFFER_HEIGHT_PC34;
    out_model->viewport_width = DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_VIEWPORT_WIDTH_PC34;
    out_model->viewport_height = DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_VIEWPORT_HEIGHT_PC34;
    out_model->c10_transparent_color = DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34;
    out_model->d3_row_depth = 3;
    out_model->d3l2_f0128_order = 0;
    out_model->d3r2_f0128_order = 1;
    out_model->d3l_f0128_order = 2;
    out_model->d3r_f0128_order = 3;
    out_model->d3c_f0128_order = 4;
    out_model->d2l2_f0128_order = 5;
    out_model->d2r2_f0128_order = 6;
    out_model->d3c_f0107_pass777_commit_pinned = 1;
    out_model->d0l2_d0r2_f0107_cc6b81b59_commit_pinned = 1;
    out_model->door_front_view_drawing_pass_first = 1;
    out_model->door_front_view_drawing_pass_second = 2;
    out_model->source_locked_contract_only = 1;
    out_model->no_real_asset_bitmap_parity = 1;
    out_model->no_game_data_load = 1;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_specs(out_model);
    fill_routes(out_model);
    fill_pixels(out_model);
    fill_rejected(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_hash_model_pc34(out_model);
    return true;
}

const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_pc34(void)
{
    static DM1V1D3L2D3R2F0108WallCompositionModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_builder_pc34(&s_model);
        s_init = 1;
    }
    return &s_model;
}

uint64_t dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_deterministic_hash_pc34(void)
{
    const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *model =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_spec_at_pc34(size_t index)
{
    const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *model =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_pc34();
    if (!model || index >= DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_COUNT_PC34) {
        return NULL;
    }
    return &model->specs[index];
}

const DM1V1D3L2D3R2F0108WallCompositionRoutePc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_route_at_pc34(size_t index)
{
    const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *model =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_pc34();
    if (!model || index >= DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ROUTE_COUNT_PC34) {
        return NULL;
    }
    return &model->routes[index];
}

const DM1V1D3L2D3R2F0108WallCompositionPixelPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pixel_at_pc34(size_t index)
{
    const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *model =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_pc34();
    if (!model || index >= DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_PIXEL_COUNT_PC34) {
        return NULL;
    }
    return &model->pixels[index];
}

const DM1V1D3L2D3R2F0108WallCompositionRejectedPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_rejected_at_pc34(size_t index)
{
    const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *model =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_pc34();
    if (!model || index >= DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_REJECTED_COUNT_PC34) {
        return NULL;
    }
    return &model->rejected[index];
}

const char *dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
