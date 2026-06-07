#include "dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_pc34_compat.h"

enum {
    DM1_F0115_ROUTE_PRESENT = 1,
    DM1_F0115_ROUTE_ABSENT = 0,
    DM1_F0115_NO_WALL = 0,
    DM1_F0115_C10_TRANSPARENCY = 1,       /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH */
    DM1_F0115_D1_DEPTH = 1,               /* ReDMCSB DUNVIEW.C:372 G2027[4/5] */
    DM1_F0115_D1L_VIEW_SQUARE = 4,        /* ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L */
    DM1_F0115_D1R_VIEW_SQUARE = 5,        /* ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R */
    DM1_F0115_D1L_LANE = -1,              /* ReDMCSB DUNVIEW.C:371 G2026[4] */
    DM1_F0115_D1R_LANE = 1,               /* ReDMCSB DUNVIEW.C:371 G2026[5] */
    DM1_F0115_D1L_ITEM_PROJECTILE_ROW = 9,/* ReDMCSB DUNVIEW.C:373 G2028[4] */
    DM1_F0115_D1R_ITEM_PROJECTILE_ROW = 10,/* ReDMCSB DUNVIEW.C:373 G2028[5] */
    DM1_F0115_D1L_CREATURE_ROW = 9,       /* ReDMCSB DUNVIEW.C:375 G2033[4] */
    DM1_F0115_D1R_CREATURE_ROW = 10,      /* ReDMCSB DUNVIEW.C:375 G2033[5] */
    DM1_F0115_D1L_EXPLOSION_ROW = 12,     /* ReDMCSB DUNVIEW.C:376 G2034[4] */
    DM1_F0115_D1R_EXPLOSION_ROW = 13,     /* ReDMCSB DUNVIEW.C:376 G2034[5] */
    DM1_F0115_ITEM_ZONE_BASE = 2500,      /* ReDMCSB DEFS.H:4228 C2500_ZONE_ */
    DM1_F0115_PROJECTILE_ZONE_BASE = 2900,/* ReDMCSB DEFS.H:4230 C2900_ZONE_ */
    DM1_F0115_EXPLOSION_ZONE_BASE = 3000, /* ReDMCSB DEFS.H:4232 C3000_ZONE_ */
    DM1_F0115_CREATURE_ZONE_BASE = 3200,  /* ReDMCSB DEFS.H:4236 C3200_ZONE_ */
    DM1_F0115_OBJECT_CREATURE_SHIFT_MASK = 0x8000,
    DM1_F0115_CELL_BACK_RIGHT = 2,        /* ReDMCSB DEFS.H:2644 C02_VIEW_CELL_BACK_RIGHT */
    DM1_F0115_CELL_BACK_LEFT = 3,         /* ReDMCSB DEFS.H:2645 C03_VIEW_CELL_BACK_LEFT */
    DM1_F0115_CELL_FRONT_LEFT = 0,        /* ReDMCSB DEFS.H:2642 C00_VIEW_CELL_FRONT_LEFT */
    DM1_F0115_CELL_FRONT_RIGHT = 1,       /* ReDMCSB DEFS.H:2643 C01_VIEW_CELL_FRONT_RIGHT */
    DM1_F0115_D1L_CORRIDOR_ORDER = 0x0032,/* ReDMCSB DEFS.H:2664; DUNVIEW.C:7523 */
    DM1_F0115_D1R_CORRIDOR_ORDER = 0x0041 /* ReDMCSB DEFS.H:2666; DUNVIEW.C:7691 */
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. "
    "DUNVIEW.C:1948-1961 declares F0122_DUNGEONVIEW_DrawSquareD1L and "
    "F0123_DUNGEONVIEW_DrawSquareD1R. DUNVIEW.C:6773-6793 is the view-depth-2 "
    "copy-protection dispatch that can patch through F0123_DUNGEONVIEW_DrawSquareD1R "
    "or F0122_DUNGEONVIEW_DrawSquareD1L. DUNVIEW.C:7391-7557 F0122 selects "
    "M607_VIEW_SQUARE_D1L and, for pit/teleporter/corridor/open-floor lanes, "
    "sets C0x0032 at 7523, skips F0107/F0111, then calls F0115 at 7536. "
    "DUNVIEW.C:7559-7725 F0123 mirrors the right lane, sets C0x0041 at 7691, "
    "skips F0107/F0111, then calls F0115 at 7704. DUNVIEW.C:4547-4581 "
    "F0115 describes per-cell object, creature, projectile, and end explosion "
    "passes. DUNVIEW.C:4806-4811 maps view lane/depth and G2028, 4923 gates "
    "items on L2476>=0 and cell match, 5075 binds C2500_ZONE_, 5201-5214 "
    "enters the creature pass, 5615-5617 binds C3200_ZONE_, 5668-5683 binds "
    "C2900_ZONE_ for projectiles, and 5916-5923/5998-5999 perform the end "
    "explosion pass using C3000_ZONE_. DUNVIEW.C:7873-7911 and 7925-7937 "
    "show the D1-ring F0115 order contrast: D1C uses back-left/back-right/"
    "front-left/front-right then a door end pass, while D1L/D1R side lanes "
    "use their two visible side cells. DEFS.H:2088 C10_COLOR_FLESH; "
    "DEFS.H:2596-2606 D0/D1/D2 view-square indices; DEFS.H:2642-2676 cell "
    "orders; DEFS.H:4228-4236 C2500/C2900/C3000/C3200 zones; "
    "DEFS.H:4250-4260 door-zone metadata anchors the no-F0111 contrast.";

static const DM1_V1_D1L2D1R2F0115ThingPassPc34 s_fixtures[] = {
    {
        DM1_V1_D1L2_D1R2_F0115_SIDE_D1L2_PC34,
        "D1L2 corridor/open-floor thing pass",
        1,
        1,
        DM1_F0115_C10_TRANSPARENCY,
        DM1_F0115_NO_WALL,
        "C2500 item / C2900 projectile / C3200 creature / C3000 explosion",
        DM1_F0115_ITEM_ZONE_BASE,
        DM1_F0115_PROJECTILE_ZONE_BASE,
        DM1_F0115_CREATURE_ZONE_BASE,
        DM1_F0115_EXPLOSION_ZONE_BASE,
        DM1_F0115_D1L_ITEM_PROJECTILE_ROW,
        DM1_F0115_D1L_CREATURE_ROW,
        DM1_F0115_D1L_EXPLOSION_ROW,
        DM1_F0115_D1L_VIEW_SQUARE,
        DM1_F0115_D1_DEPTH,
        DM1_F0115_D1L_LANE,
        DM1_F0115_D1L_CORRIDOR_ORDER,
        DM1_F0115_CELL_BACK_RIGHT,
        DM1_F0115_CELL_FRONT_RIGHT,
        2,
        1,
        1,
        1,
        1,
        1,
        "DUNVIEW.C:7391-7557 F0122_DUNGEONVIEW_DrawSquareD1L; 7523/7536",
        "DUNVIEW.C:4547-4581 F0115; 4806-4811/4923/5075/5615-5617/5668-5683/5998-5999",
        s_source_evidence
    },
    {
        DM1_V1_D1L2_D1R2_F0115_SIDE_D1R2_PC34,
        "D1R2 corridor/open-floor thing pass",
        1,
        1,
        DM1_F0115_C10_TRANSPARENCY,
        DM1_F0115_NO_WALL,
        "C2500 item / C2900 projectile / C3200 creature / C3000 explosion",
        DM1_F0115_ITEM_ZONE_BASE,
        DM1_F0115_PROJECTILE_ZONE_BASE,
        DM1_F0115_CREATURE_ZONE_BASE,
        DM1_F0115_EXPLOSION_ZONE_BASE,
        DM1_F0115_D1R_ITEM_PROJECTILE_ROW,
        DM1_F0115_D1R_CREATURE_ROW,
        DM1_F0115_D1R_EXPLOSION_ROW,
        DM1_F0115_D1R_VIEW_SQUARE,
        DM1_F0115_D1_DEPTH,
        DM1_F0115_D1R_LANE,
        DM1_F0115_D1R_CORRIDOR_ORDER,
        DM1_F0115_CELL_BACK_LEFT,
        DM1_F0115_CELL_FRONT_LEFT,
        2,
        1,
        1,
        1,
        1,
        1,
        "DUNVIEW.C:7559-7725 F0123_DUNGEONVIEW_DrawSquareD1R; 7691/7704",
        "DUNVIEW.C:4547-4581 F0115; 4806-4811/4923/5075/5615-5617/5668-5683/5998-5999",
        s_source_evidence
    }
};

static int s_initialized;

void dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_init_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0122/F0123 lines 7523/7691 seed fixed
     * contract metadata only; no user DUNGEON.DAT/GRAPHICS.DAT is read. */
    s_initialized = DM1_F0115_ROUTE_PRESENT;
}

size_t dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34(void)
{
    if (!s_initialized) dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_init_pc34();
    return sizeof(s_fixtures) / sizeof(s_fixtures[0]);
}

const DM1_V1_D1L2D1R2F0115ThingPassPc34 *
dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34()) {
        return 0;
    }
    return &s_fixtures[index];
}

const DM1_V1_D1L2D1R2F0115ThingPassPc34 *
dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34(); ++i) {
        if (s_fixtures[i].side == side) return &s_fixtures[i];
    }
    return 0;
}

int dm1_v1_viewport_d1l2_d1r2_f0115_item_zone_pc34(
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture || view_cell < 0 || view_cell > 3) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 line 5075 uses C2500 + row*4 + view cell,
     * ORed with MASK0x8000_SHIFT_OBJECTS_AND_CREATURES for item/object zones. */
    return (fixture->item_zone_base +
            (fixture->item_projectile_row * 4) +
            view_cell) | DM1_F0115_OBJECT_CREATURE_SHIFT_MASK;
}

int dm1_v1_viewport_d1l2_d1r2_f0115_projectile_zone_pc34(
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture || view_cell < 0 || view_cell > 3) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5668-5683 bind projectile zones to
     * C2900_ZONE_ + G2028[viewSquare] * 4 + view cell. */
    return fixture->projectile_zone_base +
           (fixture->item_projectile_row * 4) +
           view_cell;
}

int dm1_v1_viewport_d1l2_d1r2_f0115_creature_zone_pc34(
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture || view_cell < 0 || view_cell > 3) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5615-5617 bind creature zones to
     * C3200_ZONE_ + coordinateSet*65 + G2033[viewSquare]*5 + view cell.
     * This contract uses coordinateSet 0 to isolate the view-square row. */
    return (fixture->creature_zone_base +
            (fixture->creature_row * 5) +
            view_cell) | DM1_F0115_OBJECT_CREATURE_SHIFT_MASK;
}

int dm1_v1_viewport_d1l2_d1r2_f0115_explosion_zone_pc34(
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *fixture)
{
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5916-5923 and 5998-5999 run the
     * special end-of-things explosion pass at C3000_ZONE_ + G2034 row. */
    return fixture->explosion_zone_base + fixture->explosion_row;
}

const char *dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_source_evidence_pc34(void)
{
    return s_source_evidence;
}
