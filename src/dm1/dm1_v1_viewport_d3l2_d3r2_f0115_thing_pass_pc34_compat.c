#include "dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat.h"

#include <string.h>

enum {
    DM1_D3L2D3R2_ROUTE_PRESENT = 1,
    DM1_D3L2_VIEW_SQUARE = 14,          /* ReDMCSB DEFS.H:2610 C14_VIEW_SQUARE_D3L2 */
    DM1_D3R2_VIEW_SQUARE = 15,          /* ReDMCSB DEFS.H:2611 C15_VIEW_SQUARE_D3R2 */
    DM1_D3_DEPTH = 3,                   /* ReDMCSB DUNVIEW.C:372 G2027[14/15] */
    DM1_D3L2_LANE = -2,                 /* ReDMCSB DUNVIEW.C:371 G2026[14] stores 254 */
    DM1_D3R2_LANE = 2,                  /* ReDMCSB DUNVIEW.C:371 G2026[15] */
    DM1_D3L2_ITEM_ROW = 3,              /* ReDMCSB DUNVIEW.C:373 G2028[14] */
    DM1_D3R2_ITEM_ROW = 4,              /* ReDMCSB DUNVIEW.C:373 G2028[15] */
    DM1_D3L2_CREATURE_ROW = 3,          /* ReDMCSB DUNVIEW.C:375 G2033[14] */
    DM1_D3R2_CREATURE_ROW = 4,          /* ReDMCSB DUNVIEW.C:375 G2033[15] */
    DM1_D3L2_EXPLOSION_ROW = 6,         /* ReDMCSB DUNVIEW.C:376 G2034[14] */
    DM1_D3R2_EXPLOSION_ROW = 7,         /* ReDMCSB DUNVIEW.C:376 G2034[15] */
    DM1_D3L2_FIELD_ASPECT = 0,          /* ReDMCSB DUNVIEW.C:377 G2035[14] */
    DM1_D3R2_FIELD_ASPECT = 1,          /* ReDMCSB DUNVIEW.C:377 G2035[15] */
    DM1_D3L2_WALL_ZONE = 702,           /* ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2 */
    DM1_D3R2_WALL_ZONE = 703,           /* ReDMCSB DEFS.H:4043 C703_ZONE_WALL_D3R2 */
    DM1_WALL_ELEMENT = 0,               /* ReDMCSB square element C00_ELEMENT_WALL */
    DM1_CORRIDOR_ELEMENT = 1,           /* ReDMCSB square element C01_ELEMENT_CORRIDOR */
    DM1_PIT_ELEMENT = 2,                /* ReDMCSB square element C02_ELEMENT_PIT */
    DM1_TELEPORTER_ELEMENT = 5,         /* ReDMCSB square element C05_ELEMENT_TELEPORTER */
    DM1_DOOR_SIDE_ELEMENT = 16,         /* ReDMCSB square element C16_ELEMENT_DOOR_SIDE */
    DM1_DOOR_FRONT_ELEMENT = 17,        /* ReDMCSB square element C17_ELEMENT_DOOR_FRONT */
    DM1_STAIRS_SIDE_ELEMENT = 18,       /* ReDMCSB square element C18_ELEMENT_STAIRS_SIDE */
    DM1_D3L2_F0128_DRAW_ORDER = 3,      /* ReDMCSB DUNVIEW.C:8479-8482 */
    DM1_D3R2_F0128_DRAW_ORDER = 4,      /* ReDMCSB DUNVIEW.C:8483-8486 */
    DM1_D3L2_WALL_PREPASS_ORDER = 0,    /* ReDMCSB DUNVIEW.C:8446-8452 */
    DM1_D3R2_WALL_PREPASS_ORDER = 1,    /* ReDMCSB DUNVIEW.C:8454-8464 */
    DM1_D3L2_NORMAL_ORDER = 0x3421,     /* ReDMCSB DEFS.H:2676; DUNVIEW.C:6282 */
    DM1_D3R2_NORMAL_ORDER = 0x4312,     /* ReDMCSB DEFS.H:2677; DUNVIEW.C:6349 */
    DM1_D3L2_SIDE_ORDER = 0x0321,       /* ReDMCSB DEFS.H:2670; DUNVIEW.C:6267 */
    DM1_D3R2_SIDE_ORDER = 0x0412,       /* ReDMCSB DEFS.H:2673; DUNVIEW.C:6334 */
    DM1_D3L2_DOOR_PASS1_ORDER = 0x0218, /* ReDMCSB DEFS.H:2669; DUNVIEW.C:6271 */
    DM1_D3R2_DOOR_PASS1_ORDER = 0x0128, /* ReDMCSB DEFS.H:2668; DUNVIEW.C:6338 */
    DM1_D3L2_DOOR_PASS2_ORDER = 0x0349, /* ReDMCSB DEFS.H:2672; DUNVIEW.C:6273 */
    DM1_D3R2_DOOR_PASS2_ORDER = 0x0439  /* ReDMCSB DEFS.H:2675; DUNVIEW.C:6340 */
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. "
    "DUNVIEW.C:1943 declares F0674_F0128_sub in the same PC34 viewport "
    "source block. DUNVIEW.C:4547-4581 F0115 documents item, creature, "
    "projectile, and explosion passes; DUNVIEW.C:4794-4800 strips the "
    "door-front pass nibble before cell iteration; DUNVIEW.C:4923 and "
    "5075 cover item visibility and C2500 object-zone math; DUNVIEW.C:"
    "5180-5188 covers the requested F0115 box/blit anchor with C10 "
    "transparency; DUNVIEW.C:5211-5214 covers creature row rejection; "
    "DUNVIEW.C:5668-5675 covers projectile row/depth clipping; DUNVIEW.C:"
    "5920-5923 covers explosion rows and field-aspect fallback. DUNVIEW.C:"
    "579-580 maps the D3L2/D3R2 per-frame wall bitmaps through G0711 and "
    "G0712; this PC34 source does not use G0163[M602/M603] for D3L2/D3R2, "
    "because DEFS.H:2609 names M602 as D3R while DEFS.H:2610-2611 names "
    "C14/C15 as D3L2/D3R2. DUNVIEW.C:6235-6290 F0676_DrawD3L2 and "
    "6293-6357 F0677_DrawD3R2 classify the square with F0172, return for "
    "plain walls after F0107, run F0108/F0115 for side routes, run two "
    "F0115 passes around F0111 for front doors, and draw teleporter fields "
    "after the thing pass. DUNVIEW.C:8446-8464 performs the D3L2/D3R2 "
    "wall-set prepass, and DUNVIEW.C:8478-8486 F0128 dispatches the "
    "F0676/F0677 thing-pass route. DUNVIEW.C:371-377 maps view lane, "
    "depth, thing rows, explosion rows, and field aspects. DUNGEON.C:"
    "1769-1838 F0163 and 1840-1937 F0164 maintain square thing lists, "
    "and DUNGEON.C:2466-2589 F0172 classifies the square before viewport "
    "dispatch. DEFS.H:2642-2677 gives view cells and orders; DEFS.H:"
    "4042-4043 gives C702/C703 wall zones; DEFS.H:4228-4236 gives the "
    "F0115 object/projectile/creature/explosion zone families.";

static const DM1_V1_D3L2D3R2F0115RoutePc34 s_d3l2_routes[] = {
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_WALL_PC34,
        "D3L2 wall return before F0115",
        DM1_WALL_ELEMENT,
        0,
        0,
        0,
        0,
        0,
        0,
        -1,
        -1,
        -1,
        -1,
        "DUNVIEW.C:6253-6264 F0676 wall draws C702, F0107, then returns"
    },
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_SIDE_DOOR_OR_STAIRS_PC34,
        "D3L2 side door/stairs F0108 then F0115",
        DM1_DOOR_SIDE_ELEMENT,
        1,
        0,
        1,
        0,
        DM1_D3L2_SIDE_ORDER,
        3,
        0,
        1,
        2,
        -1,
        "DUNVIEW.C:6265-6268 and 6284-6286 F0676 order C0x0321"
    },
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS1_PC34,
        "D3L2 front door pass 1 before F0111",
        DM1_DOOR_FRONT_ELEMENT,
        1,
        0,
        1,
        1,
        DM1_D3L2_DOOR_PASS1_ORDER,
        2,
        0,
        1,
        -1,
        -1,
        "DUNVIEW.C:6270-6272 F0676 order C0x0218 before F0111"
    },
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS2_PC34,
        "D3L2 front door pass 2 after F0111",
        DM1_DOOR_FRONT_ELEMENT,
        0,
        1,
        1,
        2,
        DM1_D3L2_DOOR_PASS2_ORDER,
        2,
        3,
        2,
        -1,
        -1,
        "DUNVIEW.C:6272-6274 and 6285-6286 F0676 order C0x0349 after F0111"
    },
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34,
        "D3L2 corridor/pit/teleporter F0108 then F0115",
        DM1_CORRIDOR_ELEMENT,
        1,
        0,
        1,
        0,
        DM1_D3L2_NORMAL_ORDER,
        4,
        0,
        1,
        3,
        2,
        "DUNVIEW.C:6275-6286 F0676 order C0x3421 and BUG0_64 F0108"
    }
};

static const DM1_V1_D3L2D3R2F0115RoutePc34 s_d3r2_routes[] = {
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_WALL_PC34,
        "D3R2 wall return before F0115",
        DM1_WALL_ELEMENT,
        0,
        0,
        0,
        0,
        0,
        0,
        -1,
        -1,
        -1,
        -1,
        "DUNVIEW.C:6320-6331 F0677 wall draws C703, F0107, then returns"
    },
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_SIDE_DOOR_OR_STAIRS_PC34,
        "D3R2 side door/stairs F0108 then F0115",
        DM1_STAIRS_SIDE_ELEMENT,
        1,
        0,
        1,
        0,
        DM1_D3R2_SIDE_ORDER,
        3,
        1,
        0,
        3,
        -1,
        "DUNVIEW.C:6332-6335 and 6351-6353 F0677 order C0x0412"
    },
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS1_PC34,
        "D3R2 front door pass 1 before F0111",
        DM1_DOOR_FRONT_ELEMENT,
        1,
        0,
        1,
        1,
        DM1_D3R2_DOOR_PASS1_ORDER,
        2,
        1,
        0,
        -1,
        -1,
        "DUNVIEW.C:6337-6339 F0677 order C0x0128 before F0111"
    },
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS2_PC34,
        "D3R2 front door pass 2 after F0111",
        DM1_DOOR_FRONT_ELEMENT,
        0,
        1,
        1,
        2,
        DM1_D3R2_DOOR_PASS2_ORDER,
        2,
        2,
        3,
        -1,
        -1,
        "DUNVIEW.C:6339-6341 and 6352-6353 F0677 order C0x0439 after F0111"
    },
    {
        DM1_V1_D3L2_D3R2_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34,
        "D3R2 corridor/pit/teleporter F0108 then F0115",
        DM1_TELEPORTER_ELEMENT,
        1,
        0,
        1,
        0,
        DM1_D3R2_NORMAL_ORDER,
        4,
        1,
        0,
        2,
        3,
        "DUNVIEW.C:6342-6353 F0677 order C0x4312 and BUG0_64 F0108"
    }
};

static const DM1_V1_D3L2D3R2F0115ThingPassPc34 s_fixtures[] = {
    {
        DM1_V1_D3L2_D3R2_F0115_SIDE_D3L2_PC34,
        "D3L2 far-left side square F0115 thing pass",
        5,
        4,
        1,
        DM1_D3L2_VIEW_SQUARE,
        DM1_D3_DEPTH,
        DM1_D3L2_LANE,
        3,
        -2,
        DM1_D3L2_F0128_DRAW_ORDER,
        DM1_D3L2_WALL_PREPASS_ORDER,
        0,
        15,
        25,
        73,
        0,
        0,
        DM1_V1_D3L2_D3R2_F0115_SOURCE_WIDTH_PC34,
        DM1_V1_D3L2_D3R2_F0115_SOURCE_HEIGHT_PC34,
        DM1_D3L2_ITEM_ROW,
        DM1_D3L2_CREATURE_ROW,
        DM1_D3L2_EXPLOSION_ROW,
        DM1_D3L2_FIELD_ASPECT,
        DM1_D3L2_WALL_ZONE,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        s_d3l2_routes,
        sizeof(s_d3l2_routes) / sizeof(s_d3l2_routes[0]),
        "DUNVIEW.C:6235-6290 F0676_DrawD3L2; F0128 8478-8482 relative 3,-2",
        "DUNVIEW.C:4547-4581 F0115; 4923 depth-3 cell clip; 6286 call site",
        "DUNVIEW.C:579 G0711 frame, not G0163[M602/M603]",
        "DUNGEON.C:1769-1838 F0163; 1840-1937 F0164; 2466-2589 F0172",
        s_source_evidence
    },
    {
        DM1_V1_D3L2_D3R2_F0115_SIDE_D3R2_PC34,
        "D3R2 far-right side square F0115 thing pass",
        5,
        4,
        1,
        DM1_D3R2_VIEW_SQUARE,
        DM1_D3_DEPTH,
        DM1_D3R2_LANE,
        3,
        2,
        DM1_D3R2_F0128_DRAW_ORDER,
        DM1_D3R2_WALL_PREPASS_ORDER,
        208,
        223,
        25,
        73,
        0,
        0,
        DM1_V1_D3L2_D3R2_F0115_SOURCE_WIDTH_PC34,
        DM1_V1_D3L2_D3R2_F0115_SOURCE_HEIGHT_PC34,
        DM1_D3R2_ITEM_ROW,
        DM1_D3R2_CREATURE_ROW,
        DM1_D3R2_EXPLOSION_ROW,
        DM1_D3R2_FIELD_ASPECT,
        DM1_D3R2_WALL_ZONE,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        s_d3r2_routes,
        sizeof(s_d3r2_routes) / sizeof(s_d3r2_routes[0]),
        "DUNVIEW.C:6293-6357 F0677_DrawD3R2; F0128 8483-8486 relative 3,2",
        "DUNVIEW.C:4547-4581 F0115; 5668-5675 projectile clip; 6353 call site",
        "DUNVIEW.C:580 G0712 frame, not G0163[M602/M603]",
        "DUNGEON.C:1769-1838 F0163; 1840-1937 F0164; 2466-2589 F0172",
        s_source_evidence
    }
};

static int s_initialized;

void dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_init_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0676/F0677 lines 6235/6302 seed fixed
     * F0172-classified contract metadata only; no game data files are read. */
    s_initialized = DM1_D3L2D3R2_ROUTE_PRESENT;
}

size_t dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34(void)
{
    if (!s_initialized) dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_init_pc34();
    return sizeof(s_fixtures) / sizeof(s_fixtures[0]);
}

const DM1_V1_D3L2D3R2F0115ThingPassPc34 *
dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34()) {
        return 0;
    }
    return &s_fixtures[index];
}

const DM1_V1_D3L2D3R2F0115ThingPassPc34 *
dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34(); ++i) {
        if (s_fixtures[i].side == side) return &s_fixtures[i];
    }
    return 0;
}

const DM1_V1_D3L2D3R2F0115RoutePc34 *
dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_route_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int route_kind)
{
    size_t i;

    if (!fixture) return 0;
    for (i = 0; i < fixture->route_table_count; ++i) {
        if (fixture->routes[i].route_kind == route_kind) return &fixture->routes[i];
    }
    return 0;
}

int dm1_v1_viewport_d3l2_d3r2_f0115_decode_cell_pc34(
    unsigned int cell_order,
    int ordinal_index)
{
    unsigned int nibble;

    if (ordinal_index < 0 || ordinal_index >= 4) return -1;
    if ((cell_order & 0xFU) & 0x8U) {
        cell_order >>= 4;
    }
    nibble = (cell_order >> ((unsigned int)ordinal_index * 4U)) & 0xFU;
    if (nibble == 0U) return -1;
    return (int)nibble - 1;
}

int dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture || view_cell < 0 || view_cell > 3) return 0;
    /* ReDMCSB: DUNVIEW.C:4923 and :5672 clip depth-3 objects and
     * projectiles to cells greater than C01_VIEW_CELL_FRONT_RIGHT. */
    return fixture->view_depth == 3 ? (view_cell > 1) : 1;
}

int dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(fixture, view_cell)) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C:5075 C2500 + G2028 row * 4 + view cell. */
    return 2500 + (fixture->item_projectile_row * 4) + view_cell;
}

int dm1_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(fixture, view_cell)) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C:5683 C2900 + G2028 row * 4 + view cell. */
    return 2900 + (fixture->item_projectile_row * 4) + view_cell;
}

int dm1_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int coordinate_set,
    int view_cell)
{
    if (!fixture || coordinate_set < 0 || view_cell < 0 || view_cell > 4) return -1;
    if (fixture->creature_row < 0) return -1;
    /* ReDMCSB: DUNVIEW.C:5616 C3200 + coordinate set * 65 +
     * G2033 row * 5 + creature view cell. */
    return 3200 + (coordinate_set * 65) + (fixture->creature_row * 5) + view_cell;
}

int dm1_v1_viewport_d3l2_d3r2_f0115_explosion_zone_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture || fixture->explosion_row < 0 || view_cell < 0 || view_cell > 1) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C:6122 C3031 + G2034 explosion row * 2 + cell. */
    return 3031 + (fixture->explosion_row * 2) + view_cell;
}

uint8_t dm1_v1_viewport_d3l2_d3r2_f0115_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d3l2_d3r2_f0115_apply_pixel_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int viewport_x,
    int viewport_y,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D3L2D3R2F0115PixelPc34 *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->fixture = fixture;
    out->viewport_x = viewport_x;
    out->viewport_y = viewport_y;
    if (!fixture) return false;

    if (viewport_x < fixture->frame_viewport_x_first ||
        viewport_x > fixture->frame_viewport_x_last ||
        viewport_y < fixture->frame_viewport_y_first ||
        viewport_y > fixture->frame_viewport_y_last) {
        out->no_write_metadata = true;
        out->source_y_clipped =
            viewport_y < fixture->frame_viewport_y_first ||
            viewport_y > fixture->frame_viewport_y_last;
        return true;
    }
    if (!source || !viewport) return false;

    out->in_viewport_clip = true;
    out->source_x = fixture->frame_source_x_first +
        (viewport_x - fixture->frame_viewport_x_first);
    out->source_y = fixture->frame_source_y_first +
        (viewport_y - fixture->frame_viewport_y_first);
    if (out->source_y < 0 || out->source_y >= fixture->frame_height) {
        out->source_y_clipped = true;
        out->no_write_metadata = true;
        return true;
    }

    out->source_offset = (size_t)out->source_y *
        (size_t)DM1_V1_D3L2_D3R2_F0115_SOURCE_WIDTH_PC34 +
        (size_t)out->source_x;
    out->viewport_offset = (size_t)viewport_y *
        (size_t)DM1_V1_D3L2_D3R2_F0115_VIEWPORT_WIDTH_PC34 +
        (size_t)viewport_x;
    if (out->source_x < 0 ||
        out->source_x >= DM1_V1_D3L2_D3R2_F0115_SOURCE_WIDTH_PC34 ||
        out->source_offset >= source_len ||
        out->viewport_offset >= viewport_len) {
        return false;
    }

    out->destination_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip =
        out->source_pixel == DM1_V1_D3L2_D3R2_F0115_C10_COLOR_FLESH_PC34;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_d3l2_d3r2_f0115_blend_pixel_pc34(
            viewport[out->viewport_offset],
            out->source_pixel,
            DM1_V1_D3L2_D3R2_F0115_C10_COLOR_FLESH_PC34);
    out->destination_after = viewport[out->viewport_offset];
    return true;
}

const char *dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_source_evidence_pc34(void)
{
    return s_source_evidence;
}
