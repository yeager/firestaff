#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat.h"

enum {
    DM1_C00_ELEMENT_WALL = 0,
    DM1_C01_ELEMENT_CORRIDOR = 1,
    DM1_C02_ELEMENT_PIT = 2,
    DM1_C05_ELEMENT_TELEPORTER = 5,
    DM1_C16_ELEMENT_DOOR_SIDE = 16,
    DM1_C17_ELEMENT_DOOR_FRONT = 17,
    DM1_C18_ELEMENT_STAIRS_SIDE = 18,
    DM1_C19_ELEMENT_STAIRS_FRONT = 19,
    DM1_C10_COLOR_FLESH = 10,             /* ReDMCSB DEFS.H:2088 */
    DM1_M550_FIRST_THING_PC34 = 2,        /* ReDMCSB DEFS.H:2547-2549 */
    DM1_M550_FIRST_THING_MEDIA020 = 1,    /* ReDMCSB DEFS.H:2535-2536 */
    DM1_M607_VIEW_SQUARE_D1L = 4,         /* ReDMCSB DEFS.H:2600 */
    DM1_M608_VIEW_SQUARE_D1R = 5,         /* ReDMCSB DEFS.H:2601 */
    DM1_C00_VIEW_CELL_FRONT_LEFT = 0,     /* ReDMCSB DEFS.H:2642 */
    DM1_C01_VIEW_CELL_FRONT_RIGHT = 1,    /* ReDMCSB DEFS.H:2643 */
    DM1_C02_VIEW_CELL_BACK_RIGHT = 2,     /* ReDMCSB DEFS.H:2644 */
    DM1_C03_VIEW_CELL_BACK_LEFT = 3,      /* ReDMCSB DEFS.H:2645 */
    DM1_MASK0x0008_DOOR_FRONT = 0x0008,   /* ReDMCSB DEFS.H:2657 */
    DM1_C0x0028_DOORPASS1_BACKRIGHT = 0x0028, /* ReDMCSB DEFS.H:2663 */
    DM1_C0x0032_BACKRIGHT_FRONTRIGHT = 0x0032,/* ReDMCSB DEFS.H:2664 */
    DM1_C0x0039_DOORPASS2_FRONTRIGHT = 0x0039,/* ReDMCSB DEFS.H:2665 */
    DM1_C0x0041_BACKLEFT_FRONTLEFT = 0x0041,  /* ReDMCSB DEFS.H:2666 */
    DM1_C0x0049_DOORPASS2_FRONTLEFT = 0x0049,/* ReDMCSB DEFS.H:2667 */
    DM1_C0x0018_DOORPASS1_BACKLEFT = 0x0018, /* ReDMCSB DEFS.H:2661 */
    DM1_M630_ZONE_DOOR_D1L = 3780,        /* ReDMCSB DEFS.H:4258 */
    DM1_M632_ZONE_DOOR_D1R = 3800         /* ReDMCSB DEFS.H:4260 */
};

static const char s_source_evidence[] =
    "DM1 V1 D1L/D1R F0115 source-lock gate; contract-only, no real "
    "GRAPHICS.DAT/DUNGEON.DAT load and no original-DOS pixel parity claim. "
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 thing pass comments define the object, "
    "creature, projectile, explosion sequence and low-to-high nibble order. "
    "DUNVIEW.C:4794-4800 strips a door-front marker nibble, computes pass "
    "(order & 1)+1, and shifts the remaining cell order before drawing. "
    "DUNVIEW.C:4920-4923 clips only depth 3 front cells and depth 0 back cells, "
    "so D1 depth=1 keeps front and back view cells. DUNVIEW.C:5180-5188 blits "
    "object/projectile-as-object pixels with C10_COLOR_FLESH transparency. "
    "DUNVIEW.C:5208-5214 resolves G2033 creature rows, and DUNVIEW.C:5668-5674 "
    "resolves G2028 projectile rows with the same depth-3 front-cell clip. "
    "DUNVIEW.C:7494/7506/7508/7536 pin D1L door pass1, F0111 door body, "
    "door pass2, and corridor/pit/teleporter F0115. DUNVIEW.C:7662/7674/"
    "7676/7704 pin the D1R mirror. DUNVIEW.C:8524-8529 dispatches D1L then "
    "D1R from F0128 at relative (1,-1) and (1,+1). DEFS.H:2088 C10, "
    "2535-2549 M550_FIRST_THING, 2596-2611 view squares, 2642-2677 cell "
    "orders, 4045-4046 wall-zone contrast, 4139-4153 D1 zone band, and "
    "4258/4260 D1L/D1R door zones anchor this DM1-only contract. DUNGEON.C:"
    "1769-1838 F0163, 1840-1905 F0164, and 2466-2523 F0172 anchor square "
    "first-thing and square-aspect provenance.";

static const DM1V1D1LD1RF0115LanePc34Data s_lanes[] = {
    {
        DM1_V1_D1L_D1R_F0115_LANE_D1L_PC34,
        "D1L",
        1,
        -1,
        1,
        -1,
        8524,
        8525,
        DM1_M607_VIEW_SQUARE_D1L,
        -1,
        1,
        9,
        9,
        12,
        DM1_M630_ZONE_DOOR_D1L,
        DM1_C10_COLOR_FLESH,
        DM1_M550_FIRST_THING_PC34,
        DM1_M550_FIRST_THING_MEDIA020,
        1,
        1,
        1,
        3,
        {
            {
                DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS1_PC34,
                "D1L door-front pass 1 before F0111",
                7494,
                7506,
                0,
                DM1_C0x0028_DOORPASS1_BACKRIGHT,
                8,
                1,
                0x0002,
                0,
                0,
                0,
                1,
                { DM1_C02_VIEW_CELL_BACK_RIGHT, -1 },
                1,
                { DM1_C01_VIEW_CELL_FRONT_RIGHT, -1 },
                "C0x0028_CELL_ORDER_DOORPASS1_BACKRIGHT",
                "ReDMCSB DUNVIEW.C:7494; F0115:4794-4800"
            },
            {
                DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS2_PC34,
                "D1L door-front pass 2 after F0111",
                7536,
                7506,
                7508,
                DM1_C0x0039_DOORPASS2_FRONTRIGHT,
                9,
                2,
                0x0003,
                0,
                0,
                0,
                1,
                { DM1_C01_VIEW_CELL_FRONT_RIGHT, -1 },
                1,
                { DM1_C02_VIEW_CELL_BACK_RIGHT, -1 },
                "C0x0039_CELL_ORDER_DOORPASS2_FRONTRIGHT",
                "ReDMCSB DUNVIEW.C:7508/7536; F0115:4794-4800"
            },
            {
                DM1_V1_D1L_D1R_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34,
                "D1L corridor/pit/teleporter pass",
                7536,
                0,
                7523,
                DM1_C0x0032_BACKRIGHT_FRONTRIGHT,
                0,
                0,
                DM1_C0x0032_BACKRIGHT_FRONTRIGHT,
                1,
                1,
                1,
                2,
                { DM1_C02_VIEW_CELL_BACK_RIGHT, DM1_C01_VIEW_CELL_FRONT_RIGHT },
                2,
                { DM1_C01_VIEW_CELL_FRONT_RIGHT, DM1_C02_VIEW_CELL_BACK_RIGHT },
                "C0x0032_CELL_ORDER_BACKRIGHT_FRONTRIGHT",
                "ReDMCSB DUNVIEW.C:7510-7536; F0115:4547-4581"
            }
        },
        {
            { DM1_C00_VIEW_CELL_FRONT_LEFT, 1, 0, 1, DM1_M550_FIRST_THING_PC34 + 0 },
            { DM1_C01_VIEW_CELL_FRONT_RIGHT, 2, 1, 1, DM1_M550_FIRST_THING_PC34 + 1 },
            { DM1_C02_VIEW_CELL_BACK_RIGHT, 3, 2, 1, DM1_M550_FIRST_THING_PC34 + 2 },
            { DM1_C03_VIEW_CELL_BACK_LEFT, 4, 3, 1, DM1_M550_FIRST_THING_PC34 + 3 }
        },
        "ReDMCSB DUNVIEW.C:8524-8525 F0128 relative (1,-1) D1L dispatch",
        "ReDMCSB DUNVIEW.C:4547-4581, 4794-4800, 4920-4923, 5180-5188"
    },
    {
        DM1_V1_D1L_D1R_F0115_LANE_D1R_PC34,
        "D1R",
        1,
        1,
        1,
        1,
        8528,
        8529,
        DM1_M608_VIEW_SQUARE_D1R,
        1,
        1,
        10,
        10,
        13,
        DM1_M632_ZONE_DOOR_D1R,
        DM1_C10_COLOR_FLESH,
        DM1_M550_FIRST_THING_PC34,
        DM1_M550_FIRST_THING_MEDIA020,
        1,
        1,
        1,
        3,
        {
            {
                DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS1_PC34,
                "D1R door-front pass 1 before F0111",
                7662,
                7674,
                0,
                DM1_C0x0018_DOORPASS1_BACKLEFT,
                8,
                1,
                0x0001,
                0,
                0,
                0,
                1,
                { DM1_C03_VIEW_CELL_BACK_LEFT, -1 },
                1,
                { DM1_C00_VIEW_CELL_FRONT_LEFT, -1 },
                "C0x0018_CELL_ORDER_DOORPASS1_BACKLEFT",
                "ReDMCSB DUNVIEW.C:7662; F0115:4794-4800"
            },
            {
                DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS2_PC34,
                "D1R door-front pass 2 after F0111",
                7704,
                7674,
                7676,
                DM1_C0x0049_DOORPASS2_FRONTLEFT,
                9,
                2,
                0x0004,
                0,
                0,
                0,
                1,
                { DM1_C00_VIEW_CELL_FRONT_LEFT, -1 },
                1,
                { DM1_C03_VIEW_CELL_BACK_LEFT, -1 },
                "C0x0049_CELL_ORDER_DOORPASS2_FRONTLEFT",
                "ReDMCSB DUNVIEW.C:7676/7704; F0115:4794-4800"
            },
            {
                DM1_V1_D1L_D1R_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34,
                "D1R corridor/pit/teleporter pass",
                7704,
                0,
                7691,
                DM1_C0x0041_BACKLEFT_FRONTLEFT,
                0,
                0,
                DM1_C0x0041_BACKLEFT_FRONTLEFT,
                1,
                1,
                1,
                2,
                { DM1_C03_VIEW_CELL_BACK_LEFT, DM1_C00_VIEW_CELL_FRONT_LEFT },
                2,
                { DM1_C00_VIEW_CELL_FRONT_LEFT, DM1_C03_VIEW_CELL_BACK_LEFT },
                "C0x0041_CELL_ORDER_BACKLEFT_FRONTLEFT",
                "ReDMCSB DUNVIEW.C:7678-7704; F0115:4547-4581"
            }
        },
        {
            { DM1_C00_VIEW_CELL_FRONT_LEFT, 1, 0, 1, DM1_M550_FIRST_THING_PC34 + 0 },
            { DM1_C01_VIEW_CELL_FRONT_RIGHT, 2, 1, 1, DM1_M550_FIRST_THING_PC34 + 1 },
            { DM1_C02_VIEW_CELL_BACK_RIGHT, 3, 2, 1, DM1_M550_FIRST_THING_PC34 + 2 },
            { DM1_C03_VIEW_CELL_BACK_LEFT, 4, 3, 1, DM1_M550_FIRST_THING_PC34 + 3 }
        },
        "ReDMCSB DUNVIEW.C:8528-8529 F0128 relative (1,+1) D1R dispatch",
        "ReDMCSB DUNVIEW.C:4547-4581, 4794-4800, 4920-4923, 5180-5188"
    }
};

static int s_initialized;

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;
    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

void dm1_v1_viewport_d1l_d1r_f0115_thing_pass_init_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8524-8529 dispatch D1L before D1R;
     * this model records source contracts only and loads no game assets. */
    s_initialized = 1;
}

size_t dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_count_pc34(void)
{
    if (!s_initialized) dm1_v1_viewport_d1l_d1r_f0115_thing_pass_init_pc34();
    return sizeof(s_lanes) / sizeof(s_lanes[0]);
}

const DM1V1D1LD1RF0115LanePc34Data *
dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_count_pc34()) {
        return 0;
    }
    return &s_lanes[index];
}

const DM1V1D1LD1RF0115LanePc34Data *
dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_for_view_square_pc34(
    int view_square)
{
    size_t i;
    for (i = 0; i < dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_count_pc34(); ++i) {
        if (s_lanes[i].view_square == view_square) {
            return &s_lanes[i];
        }
    }
    return 0;
}

const DM1V1D1LD1RF0115RoutePc34 *
dm1_v1_viewport_d1l_d1r_f0115_thing_pass_route_pc34(
    const DM1V1D1LD1RF0115LanePc34Data *lane,
    int route_kind)
{
    size_t i;
    if (!lane) return 0;
    for (i = 0; i < lane->route_count; ++i) {
        if (lane->routes[i].route_kind == route_kind) {
            return &lane->routes[i];
        }
    }
    return 0;
}

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(
    int element_type)
{
    return element_type == DM1_C01_ELEMENT_CORRIDOR ||
           element_type == DM1_C02_ELEMENT_PIT ||
           element_type == DM1_C05_ELEMENT_TELEPORTER ||
           element_type == DM1_C16_ELEMENT_DOOR_SIDE ||
           element_type == DM1_C17_ELEMENT_DOOR_FRONT;
}

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_rejects_element_pc34(
    int element_type)
{
    return !dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(
        element_type);
}

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_view_cell_pc34(
    int view_cell)
{
    return view_cell >= DM1_C00_VIEW_CELL_FRONT_LEFT &&
           view_cell <= DM1_C03_VIEW_CELL_BACK_LEFT;
}

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_depth1_keeps_cell_pc34(
    int view_cell)
{
    if (!dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_view_cell_pc34(view_cell)) {
        return 0;
    }
    /* ReDMCSB: DUNVIEW.C F0115 line 4923 only clips depth 3 front cells
     * and depth 0 back cells; D1 view_depth=1 keeps all four cells. */
    return 1;
}

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_order_pc34(
    const DM1V1D1LD1RF0115LanePc34Data *lane,
    int raw_cell_order)
{
    size_t i;
    if (!lane || raw_cell_order == 0) return 0;
    for (i = 0; i < lane->route_count; ++i) {
        if (lane->routes[i].raw_cell_order == raw_cell_order) {
            return 1;
        }
    }
    return 0;
}

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_door_pass_from_order_pc34(
    int raw_cell_order)
{
    if ((raw_cell_order & DM1_MASK0x0008_DOOR_FRONT) == 0) return 0;
    /* ReDMCSB: DUNVIEW.C F0115 lines 4794-4796, pass = (order & 1) + 1. */
    return (raw_cell_order & 0x0001) + 1;
}

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_strip_door_order_pc34(
    int raw_cell_order)
{
    if ((raw_cell_order & DM1_MASK0x0008_DOOR_FRONT) == 0) return raw_cell_order;
    /* ReDMCSB: DUNVIEW.C F0115 line 4796 removes the door-front nibble. */
    return raw_cell_order >> 4;
}

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_first_thing_slot_pc34(
    const DM1V1D1LD1RF0115LanePc34Data *lane,
    int view_cell)
{
    if (!lane || !dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_view_cell_pc34(view_cell)) {
        return -1;
    }
    return lane->m550_first_thing_square_aspect_slot + view_cell;
}

uint32_t dm1_v1_viewport_d1l_d1r_f0115_thing_pass_hash_pc34(void)
{
    uint32_t hash = 2166136261u;
    size_t i;
    size_t j;
    size_t k;

    for (i = 0; i < dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_count_pc34(); ++i) {
        const DM1V1D1LD1RF0115LanePc34Data *lane = &s_lanes[i];
        hash = fnv1a_u32(hash, (uint32_t)lane->lane);
        hash = fnv1a_u32(hash, (uint32_t)lane->relative_depth);
        hash = fnv1a_u32(hash, (uint32_t)lane->relative_lateral);
        hash = fnv1a_u32(hash, (uint32_t)lane->f0128_update_line);
        hash = fnv1a_u32(hash, (uint32_t)lane->f0128_dispatch_line);
        hash = fnv1a_u32(hash, (uint32_t)lane->view_square);
        hash = fnv1a_u32(hash, (uint32_t)lane->view_lane);
        hash = fnv1a_u32(hash, (uint32_t)lane->view_depth);
        hash = fnv1a_u32(hash, (uint32_t)lane->item_projectile_row);
        hash = fnv1a_u32(hash, (uint32_t)lane->creature_row);
        hash = fnv1a_u32(hash, (uint32_t)lane->explosion_row);
        hash = fnv1a_u32(hash, (uint32_t)lane->door_zone);
        hash = fnv1a_u32(hash, (uint32_t)lane->c10_transparent_color);
        hash = fnv1a_u32(hash, (uint32_t)lane->m550_first_thing_square_aspect_slot);
        for (j = 0; j < lane->route_count; ++j) {
            const DM1V1D1LD1RF0115RoutePc34 *route = &lane->routes[j];
            hash = fnv1a_u32(hash, (uint32_t)route->route_kind);
            hash = fnv1a_u32(hash, (uint32_t)route->caller_line);
            hash = fnv1a_u32(hash, (uint32_t)route->f0111_door_line);
            hash = fnv1a_u32(hash, (uint32_t)route->order_assign_line);
            hash = fnv1a_u32(hash, (uint32_t)route->raw_cell_order);
            hash = fnv1a_u32(hash, (uint32_t)route->door_front_marker_nibble);
            hash = fnv1a_u32(hash, (uint32_t)route->door_front_pass);
            hash = fnv1a_u32(hash, (uint32_t)route->stripped_cell_order);
            hash = fnv1a_u32(hash, (uint32_t)route->source_named_cell_count);
            for (k = 0; k < route->source_named_cell_count; ++k) {
                hash = fnv1a_u32(hash, (uint32_t)route->source_named_cells[k]);
            }
            hash = fnv1a_u32(hash, (uint32_t)route->decoded_cell_count);
            for (k = 0; k < route->decoded_cell_count; ++k) {
                hash = fnv1a_u32(hash, (uint32_t)route->decoded_cells[k]);
            }
        }
        for (j = 0; j < 4; ++j) {
            hash = fnv1a_u32(hash, (uint32_t)lane->depth1_cells[j].source_named_cell);
            hash = fnv1a_u32(hash, (uint32_t)lane->depth1_cells[j].ordinal_low_to_high);
            hash = fnv1a_u32(hash, (uint32_t)lane->depth1_cells[j].decoded_view_cell_index);
            hash = fnv1a_u32(hash, (uint32_t)lane->depth1_cells[j].kept_by_depth1_clip);
            hash = fnv1a_u32(hash, (uint32_t)lane->depth1_cells[j].first_thing_square_aspect_slot);
        }
    }
    return hash;
}

const char *dm1_v1_viewport_d1l_d1r_f0115_thing_pass_source_evidence_pc34(void)
{
    return s_source_evidence;
}
