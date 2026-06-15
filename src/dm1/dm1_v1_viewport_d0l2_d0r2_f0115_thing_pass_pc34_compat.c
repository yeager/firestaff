#include "dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat.h"

enum {
    DM1_D0L2D0R2_ROUTE_PRESENT = 1,
    DM1_D0L2_VIEW_SQUARE = 1,          /* ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L */
    DM1_D0R2_VIEW_SQUARE = 2,          /* ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R */
    DM1_D0_DEPTH = 0,                  /* ReDMCSB DUNVIEW.C:372 G2027[1/2] */
    DM1_D0L2_LANE = -1,                /* ReDMCSB DUNVIEW.C:371 G2026[1] */
    DM1_D0R2_LANE = 1,                 /* ReDMCSB DUNVIEW.C:371 G2026[2] */
    DM1_G2028_DISABLED = -1,           /* ReDMCSB DUNVIEW.C:373 G2028[1/2] */
    DM1_D0L2_CREATURE_ROW = 11,        /* ReDMCSB DUNVIEW.C:375 G2033[1] */
    DM1_D0R2_CREATURE_ROW = 12,        /* ReDMCSB DUNVIEW.C:375 G2033[2] */
    DM1_D0L2_EXPLOSION_ROW = 15,       /* ReDMCSB DUNVIEW.C:376 G2034[1] */
    DM1_D0R2_EXPLOSION_ROW = 16,       /* ReDMCSB DUNVIEW.C:376 G2034[2] */
    DM1_D0L2_FIELD_ASPECT = 14,        /* ReDMCSB DUNVIEW.C:377 G2035[1] */
    DM1_D0R2_FIELD_ASPECT = 15,        /* ReDMCSB DUNVIEW.C:377 G2035[2] */
    DM1_D0L2_WALL_ZONE = 716,          /* ReDMCSB DEFS.H:4056 C716_ZONE_WALL_D0L */
    DM1_D0R2_WALL_ZONE = 717,          /* ReDMCSB DEFS.H:4057 C717_ZONE_WALL_D0R */
    DM1_FLUXCAGE_FIELD_ZONE_BASE = 702,/* ReDMCSB DUNVIEW.C:6219 C702 + G2035 row */
    DM1_NO_D0_DOOR_ZONE = -1,          /* ReDMCSB DEFS.H:4250-4260 has no D0 door zone. */
    DM1_D0L2_CEILING_ZONE = 870,       /* ReDMCSB DUNVIEW.C:8003 C870_ZONE_CEILING_PIT_D0L */
    DM1_D0R2_CEILING_ZONE = 872,       /* ReDMCSB DUNVIEW.C:8113 C872_ZONE_CEILING_PIT_D0R */
    DM1_CELL_BACK_RIGHT = 2,           /* ReDMCSB DEFS.H:2644 C02_VIEW_CELL_BACK_RIGHT */
    DM1_CELL_BACK_LEFT = 3,            /* ReDMCSB DEFS.H:2645 C03_VIEW_CELL_BACK_LEFT */
    DM1_CELL_FRONT_LEFT = 0,           /* ReDMCSB DEFS.H:2642 C00_VIEW_CELL_FRONT_LEFT */
    DM1_CELL_FRONT_RIGHT = 1,          /* ReDMCSB DEFS.H:2643 C01_VIEW_CELL_FRONT_RIGHT */
    DM1_D0L2_ORDER = 0x0002,           /* ReDMCSB DEFS.H:2660; DUNVIEW.C:8005 */
    DM1_D0R2_ORDER = 0x0001,           /* ReDMCSB DEFS.H:2659; DUNVIEW.C:8115 */
    DM1_ITEM_ZONE_BASE = 2500,         /* ReDMCSB DEFS.H:4228 C2500_ZONE_ */
    DM1_PROJECTILE_ZONE_BASE = 2900,   /* ReDMCSB DEFS.H:4230 C2900_ZONE_ */
    DM1_CENTER_EXPLOSION_BASE = 3014,  /* ReDMCSB DEFS.H:4234 C3014_ZONE_ */
    DM1_SIDE_EXPLOSION_BASE = 3031,    /* ReDMCSB DEFS.H:4235 C3031_ZONE_ */
    DM1_CREATURE_ZONE_BASE = 3200,     /* ReDMCSB DEFS.H:4236 C3200_ZONE_ */
    DM1_C10_COLOR_FLESH = 10,          /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH */
    DM1_OBJECT_CREATURE_SHIFT = 0x8000
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. "
    "DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L and "
    "DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R route the near "
    "side lanes. DUNVIEW.C:8003-8005 draws D0L ceiling pit before calling "
    "F0115 with M610_VIEW_SQUARE_D0L and C0x0002_CELL_ORDER_BACKRIGHT; "
    "DUNVIEW.C:8113-8115 mirrors D0R with M611_VIEW_SQUARE_D0R and "
    "C0x0001_CELL_ORDER_BACKLEFT. DUNVIEW.C:8536-8541 F0128 dispatches "
    "D0L then D0R after relative movement offsets 0,-1 and 0,1. "
    "DUNVIEW.C:4547-4581 F0115 describes the thing pass; lines 4806-4812 "
    "load view lane/depth and G2028/G2029 rows; lines 4923 and 5668-5671 "
    "skip item/projectile drawing when G2028 is negative for D0L/D0R; "
    "lines 5211-5214 keep creature rows from G2033, line 5295 restricts "
    "D0L quarter creatures to BACKRIGHT and D0R to BACKLEFT, and "
    "5615-5617 binds C3200 creature zones. DUNVIEW.C:5920-5923 and "
    "6107/6122 bind explosion rows from G2034/C3014/C3031. "
    "DUNVIEW.C:6006-6015 defers fluxcage explosions, and "
    "DUNVIEW.C:6199-6219 draws them after normal explosions only when "
    "G2035 is valid, the door-front pass is not 1, and endgame suppression "
    "is not active; C702 + G2035 maps D0L/D0R fluxcages to C716/C717. "
    "DUNVIEW.C:8050-8059 and 8150-8159 draw teleporter fields after the "
    "F0115 route using G2035 and C716/C717 wall zones. DEFS.H:2596-2606 "
    "view-square indices; DEFS.H:2642-2660 cell ordinals/orders; "
    "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:4056-4057 wall zones; "
    "DEFS.H:4250-4260 door-zone table has no D0 door-front zone because "
    "D0L/D0R take the side-square F0115 path. DUNVIEW.C:3940-4008 F0108 "
    "is the C10-transparent floor-ornament stage used by deeper door-side "
    "callers; this D0L/D0R source-lock composes a synthetic F0108 pixel "
    "before F0115 without claiming a direct D0 F0108 call. "
    "DUNGEON.C:1769-1838 F0163 and 1840-1905 F0164 are mutating link/"
    "unlink anchors not called by draw; DUNGEON.C:2466-2523 F0172 supplies "
    "the square aspect consumed by F0125/F0126. DEFS.H:4228-4236 thing-zone bases.";

static const DM1_V1_D0L2D0R2F0115ThingPassPc34 s_fixtures[] = {
    {
        DM1_V1_D0L2_D0R2_F0115_SIDE_D0L2_PC34,
        "D0L2 near-left corridor/open-floor thing pass",
        1,
        1,
        1,
        0,
        DM1_D0L2_VIEW_SQUARE,
        DM1_D0_DEPTH,
        DM1_D0L2_LANE,
        DM1_D0L2_ORDER,
        DM1_CELL_BACK_RIGHT,
        1,
        DM1_G2028_DISABLED,
        DM1_D0L2_CREATURE_ROW,
        DM1_D0L2_EXPLOSION_ROW,
        DM1_D0L2_FIELD_ASPECT,
        DM1_D0L2_WALL_ZONE,
        DM1_FLUXCAGE_FIELD_ZONE_BASE + DM1_D0L2_FIELD_ASPECT,
        1,
        1,
        1,
        DM1_NO_D0_DOOR_ZONE,
        DM1_D0L2_CEILING_ZONE,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        "DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L; 8003/8005/8059",
        "DUNVIEW.C:4547-4581 F0115; 4923/5211/5295/5615-5617/5668-5671/6107/6122/6199-6219",
        "DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172",
        "DEFS.H:2088 C10; 2596-2606 view squares; 2642-2660 cells; 4056/4250-4260 zones",
        s_source_evidence
    },
    {
        DM1_V1_D0L2_D0R2_F0115_SIDE_D0R2_PC34,
        "D0R2 near-right corridor/open-floor thing pass",
        1,
        1,
        1,
        0,
        DM1_D0R2_VIEW_SQUARE,
        DM1_D0_DEPTH,
        DM1_D0R2_LANE,
        DM1_D0R2_ORDER,
        DM1_CELL_BACK_LEFT,
        1,
        DM1_G2028_DISABLED,
        DM1_D0R2_CREATURE_ROW,
        DM1_D0R2_EXPLOSION_ROW,
        DM1_D0R2_FIELD_ASPECT,
        DM1_D0R2_WALL_ZONE,
        DM1_FLUXCAGE_FIELD_ZONE_BASE + DM1_D0R2_FIELD_ASPECT,
        1,
        1,
        1,
        DM1_NO_D0_DOOR_ZONE,
        DM1_D0R2_CEILING_ZONE,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        "DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R; 8113/8115/8159",
        "DUNVIEW.C:4547-4581 F0115; 4923/5211/5295/5615-5617/5668-5671/6107/6122/6199-6219",
        "DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172",
        "DEFS.H:2088 C10; 2596-2606 view squares; 2642-2660 cells; 4057/4250-4260 zones",
        s_source_evidence
    }
};

static int s_initialized;

void dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0125/F0126 lines 8005/8115 seed fixed
     * contract metadata only; no DUNGEON.DAT/GRAPHICS.DAT is read. */
    s_initialized = DM1_D0L2D0R2_ROUTE_PRESENT;
}

size_t dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_count_pc34(void)
{
    if (!s_initialized) dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34();
    return sizeof(s_fixtures) / sizeof(s_fixtures[0]);
}

size_t dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34(void)
{
    return dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_count_pc34();
}

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_count_pc34()) {
        return 0;
    }
    return &s_fixtures[index];
}

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(size_t index)
{
    return dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_at_pc34(index);
}

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_for_side_pc34(int side)
{
    size_t i;

    for (i = 0;
         i < dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_count_pc34();
         ++i) {
        if (s_fixtures[i].side == side) return &s_fixtures[i];
    }
    return 0;
}

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(int side)
{
    return dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_for_side_pc34(side);
}

int dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_decode_cell_order_pc34(
    unsigned int order,
    int ordinal)
{
    unsigned int shift;
    unsigned int cell;

    /* ReDMCSB: DUNVIEW.C F0115 lines 4547-4581 document low-to-high
     * nibble cell-order consumers; DEFS.H lines 2656-2660 define the D0
     * BACKLEFT/BACKRIGHT order constants. */
    if (ordinal < 0 || ordinal > 3) return -1;
    shift = (unsigned int)ordinal * 4u;
    cell = (order >> shift) & 0x0fu;
    if (cell == 0u || cell == 8u || cell == 9u) return -1;
    return (int)cell - 1;
}

uint8_t dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    /* ReDMCSB: DUNVIEW.C F0108 lines 3988-4004 and F0115 object blits use
     * DEFS.H line 2088 C10_COLOR_FLESH as the transparent pixel. */
    return source_pixel == (uint8_t)DM1_C10_COLOR_FLESH ?
        destination_pixel : source_pixel;
}

int dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_compose_pixel_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *spec,
    uint8_t base_pixel,
    uint8_t f0108_floor_pixel,
    uint8_t f0115_thing_pixel,
    DM1_V1_D0L2D0R2F0115ThingPassTracePc34 *out_trace)
{
    DM1_V1_D0L2D0R2F0115ThingPassTracePc34 trace;

    if (!spec || !out_trace) return -1;
    /* ReDMCSB: DUNVIEW.C F0108 lines 3940-4008 is the C10-transparent
     * floor-ornament stage; DUNVIEW.C F0125/F0126 lines 8005/8115 then
     * hand the D0 side square to F0115, whose lines 4547-4581 preserve
     * ordered thing-pass composition. */
    trace.ok = 1;
    trace.f0108_calls = spec->f0108_floor_stage_before_f0115;
    trace.f0115_calls = spec->f0115_call_count;
    trace.f0108_transparent = f0108_floor_pixel == (uint8_t)DM1_C10_COLOR_FLESH;
    trace.f0115_transparent = f0115_thing_pixel == (uint8_t)DM1_C10_COLOR_FLESH;
    trace.after_f0108 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_blend_pixel_pc34(
            base_pixel, f0108_floor_pixel);
    trace.after_f0115 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_blend_pixel_pc34(
            trace.after_f0108, f0115_thing_pixel);
    *out_trace = trace;
    return 0;
}

int dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_is_draw_mutating_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *spec)
{
    if (!spec) return -1;
    /* ReDMCSB: DUNGEON.C F0163 lines 1769-1838 and F0164 lines 1840-1905
     * mutate thing lists; the D0L/D0R draw contract consumes F0172 square
     * aspect data at DUNGEON.C lines 2466-2523 and must not link/unlink. */
    return !(spec->f0163_not_called_by_draw && spec->f0164_not_called_by_draw);
}

int dm1_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    (void)view_cell;
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 line 4923 rejects D0L/D0R items because
     * G2028[1] and G2028[2] are -1 (DUNVIEW.C:373). */
    return fixture->item_projectile_row < 0 ? -1 :
        ((DM1_ITEM_ZONE_BASE + fixture->item_projectile_row * 4 + view_cell) |
         DM1_OBJECT_CREATURE_SHIFT);
}

int dm1_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    (void)view_cell;
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5668-5671 disable D0L/D0R projectile
     * drawing through the same negative G2028 row used for items. */
    return fixture->item_projectile_row < 0 ? -1 :
        DM1_PROJECTILE_ZONE_BASE + fixture->item_projectile_row * 4 + view_cell;
}

int dm1_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture || view_cell != fixture->f0115_first_cell) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5295 and 5615-5617 gate D0L quarter
     * creatures to BACKRIGHT, D0R to BACKLEFT, then bind C3200 + row*5 + cell. */
    return (DM1_CREATURE_ZONE_BASE +
            fixture->creature_row * 5 +
            view_cell) | DM1_OBJECT_CREATURE_SHIFT;
}

int dm1_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture)
{
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5920-5923 and 6107 use C3014 plus
     * G2034[viewSquare] for centered explosion zones. */
    return DM1_CENTER_EXPLOSION_BASE + fixture->explosion_row;
}

int dm1_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture || (view_cell != DM1_CELL_FRONT_LEFT &&
                     view_cell != DM1_CELL_FRONT_RIGHT)) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C F0115 lines 6110-6122 uses C3031 plus
     * G2034[viewSquare]*2 plus the front-left/front-right explosion cell. */
    return DM1_SIDE_EXPLOSION_BASE + fixture->explosion_row * 2 + view_cell;
}

int dm1_v1_viewport_d0l2_d0r2_f0115_fluxcage_field_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int door_front_pass,
    int endgame_suppressed)
{
    if (!fixture || fixture->field_aspect_index < 0 ||
        door_front_pass == 1 || endgame_suppressed) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C F0115 lines 6006-6015 records fluxcage
     * explosions, then lines 6199-6219 draw the field after other
     * explosion blits as C702_ZONE_WALL_D3L2 + G2035[viewSquare]. */
    return DM1_FLUXCAGE_FIELD_ZONE_BASE + fixture->field_aspect_index;
}

const char *dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_evidence_pc34(void)
{
    return s_source_evidence;
}
