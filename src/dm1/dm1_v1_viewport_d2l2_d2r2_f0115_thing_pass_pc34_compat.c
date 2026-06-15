#include "dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_pc34_compat.h"

enum {
    DM1_D2L2D2R2_ROUTE_PRESENT = 1,
    DM1_D2L2_VIEW_SQUARE = 9,          /* ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2 */
    DM1_D2R2_VIEW_SQUARE = 10,         /* ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2 */
    DM1_D2C_VIEW_SQUARE = 6,           /* ReDMCSB DEFS.H:2602 M603_VIEW_SQUARE_D2C */
    DM1_D2_DEPTH = 2,                  /* ReDMCSB DUNVIEW.C:372 G2027[9/10] */
    DM1_D2L2_LANE = -2,                /* ReDMCSB DUNVIEW.C:371 G2026[9] stores 254 */
    DM1_D2R2_LANE = 2,                 /* ReDMCSB DUNVIEW.C:371 G2026[10] */
    DM1_DISABLED_ROW = -1,             /* ReDMCSB DUNVIEW.C:373/375/376 entries 9/10 */
    DM1_D2L2_FIELD_ASPECT = 5,         /* ReDMCSB DUNVIEW.C:377 G2035[9] */
    DM1_D2R2_FIELD_ASPECT = 6,         /* ReDMCSB DUNVIEW.C:377 G2035[10] */
    DM1_D2L2_WALL_ZONE = 707,          /* ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2 */
    DM1_D2R2_WALL_ZONE = 708,          /* ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2 */
    DM1_D2C_NORMAL_ORDER = 0x3421,     /* ReDMCSB DEFS.H:2676; DUNVIEW.C:7368 */
    DM1_D2L2_DRAW_ORDER = 8,           /* ReDMCSB DUNVIEW.C:8503-8504 */
    DM1_D2R2_DRAW_ORDER = 9,           /* ReDMCSB DUNVIEW.C:8507-8508 */
    DM1_D2C_DRAW_ORDER = 12            /* ReDMCSB DUNVIEW.C:8520-8521 */
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. "
    "DUNVIEW.C:4547-4581 F0115 documents item, creature, projectile, and "
    "explosion passes; DUNVIEW.C:5180-5295 covers the F0115 box/blit and "
    "creature anchor block requested as the F0121/D2L2 box anchor, while "
    "DUNVIEW.C:5668-5671 covers the requested F0128/D2R2 projectile "
    "visibility anchor. DUNVIEW.C:6837-6865 F0678_DrawD2L2 calls "
    "F0172_DUNGEON_SetSquareAspect, handles only WALL and TELEPORTER, "
    "returns at 6862 for walls, and never calls F0115. DUNVIEW.C:6868-6896 "
    "F0679_DrawD2R2 mirrors that NOT-route and returns at 6893 for walls. "
    "DUNVIEW.C:8503-8508 F0128 dispatches D2L2 at relative offset 2,-2 "
    "before D2R2 at 2,2; DUNVIEW.C:8520-8521 dispatches D2C later. "
    "DUNVIEW.C:7244-7388 F0121_DUNGEONVIEW_DrawSquareD2C owns the D2C "
    "F0115 calls at 7315 and 7368, so D2L2/D2R2 must not borrow D2C's "
    "0x3421 F0115 order. DUNVIEW.C:371-377 maps view lane, depth, disabled "
    "G2028/G2033/G2034 thing rows, and G2035 field aspects for D2L2/D2R2. "
    "DUNGEON.C:1769-1838 F0163 and 1840-1937 F0164 maintain square thing "
    "lists, and DUNGEON.C:2466-2589 F0172 classifies the square before the "
    "D2L2/D2R2 switch. DEFS.H:2602-2606 gives D2C/D2L2/D2R2 view squares; "
    "DEFS.H:2642-2677 gives cell orders; DEFS.H:4047-4048 gives C707/C708 "
    "wall zones; DEFS.H:4228-4236 gives the F0115 zone families that remain "
    "unreached for the D2L2/D2R2 NOT-route.";

static const DM1_V1_D2L2D2R2F0115ThingPassPc34 s_fixtures[] = {
    {
        DM1_V1_D2L2_D2R2_F0115_SIDE_D2L2_PC34,
        "D2L2 far-left side square F0115 NOT-route",
        1,
        0,
        1,
        DM1_D2L2_VIEW_SQUARE,
        DM1_D2_DEPTH,
        DM1_D2L2_LANE,
        2,
        -2,
        DM1_D2L2_DRAW_ORDER,
        0,
        -1,
        -1,
        0,
        DM1_DISABLED_ROW,
        DM1_DISABLED_ROW,
        DM1_DISABLED_ROW,
        DM1_D2L2_FIELD_ASPECT,
        DM1_D2L2_WALL_ZONE,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        DM1_D2C_VIEW_SQUARE,
        DM1_D2C_NORMAL_ORDER,
        DM1_D2C_DRAW_ORDER,
        1,
        1,
        1,
        "DUNVIEW.C:6837-6865 F0678_DrawD2L2; F0128 8503-8504 relative 2,-2",
        "DUNVIEW.C:4547-4581 F0115; D2L2 has no call and disabled rows 373/375/376",
        "DUNGEON.C:1769-1838 F0163; 1840-1937 F0164; 2466-2589 F0172",
        s_source_evidence
    },
    {
        DM1_V1_D2L2_D2R2_F0115_SIDE_D2R2_PC34,
        "D2R2 far-right side square F0115 NOT-route",
        1,
        0,
        1,
        DM1_D2R2_VIEW_SQUARE,
        DM1_D2_DEPTH,
        DM1_D2R2_LANE,
        2,
        2,
        DM1_D2R2_DRAW_ORDER,
        0,
        -1,
        -1,
        0,
        DM1_DISABLED_ROW,
        DM1_DISABLED_ROW,
        DM1_DISABLED_ROW,
        DM1_D2R2_FIELD_ASPECT,
        DM1_D2R2_WALL_ZONE,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        DM1_D2C_VIEW_SQUARE,
        DM1_D2C_NORMAL_ORDER,
        DM1_D2C_DRAW_ORDER,
        1,
        1,
        1,
        "DUNVIEW.C:6868-6896 F0679_DrawD2R2; F0128 8507-8508 relative 2,2",
        "DUNVIEW.C:4547-4581 F0115; D2R2 has no call and disabled rows 373/375/376",
        "DUNGEON.C:1769-1838 F0163; 1840-1937 F0164; 2466-2589 F0172",
        s_source_evidence
    }
};

static int s_initialized;

void dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_init_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0678/F0679 lines 6846/6877 seed fixed
     * F0172-classified contract metadata only; no game data files are read. */
    s_initialized = DM1_D2L2D2R2_ROUTE_PRESENT;
}

size_t dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_count_pc34(void)
{
    if (!s_initialized) dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_init_pc34();
    return sizeof(s_fixtures) / sizeof(s_fixtures[0]);
}

const DM1_V1_D2L2D2R2F0115ThingPassPc34 *
dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_count_pc34()) {
        return 0;
    }
    return &s_fixtures[index];
}

const DM1_V1_D2L2D2R2F0115ThingPassPc34 *
dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_count_pc34(); ++i) {
        if (s_fixtures[i].side == side) return &s_fixtures[i];
    }
    return 0;
}

int dm1_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    (void)view_cell;
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 4923 and 5075 would bind C2500 only
     * inside F0115; F0678/F0679 never call it, and G2028[9/10] is -1. */
    return -1;
}

int dm1_v1_viewport_d2l2_d2r2_f0115_projectile_zone_pc34(
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    (void)view_cell;
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5668-5671 disable negative G2028
     * projectile rows; D2L2/D2R2 also never enter the F0115 dispatcher. */
    return -1;
}

int dm1_v1_viewport_d2l2_d2r2_f0115_creature_zone_pc34(
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    (void)view_cell;
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5211-5214 reject negative G2033 rows;
     * D2L2/D2R2 source routes keep that pass unreachable. */
    return -1;
}

int dm1_v1_viewport_d2l2_d2r2_f0115_explosion_zone_pc34(
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *fixture)
{
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5920-5923 read G2034; entries 9/10
     * are -1, and the D2L2/D2R2 dispatcher path does not call F0115. */
    return -1;
}

const char *dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_source_evidence_pc34(void)
{
    return s_source_evidence;
}
