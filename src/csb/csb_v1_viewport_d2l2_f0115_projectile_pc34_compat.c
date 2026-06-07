#include "csb_v1_viewport_d2l2_f0115_projectile_pc34_compat.h"

enum {
    CSB_D2L2_VIEW_SQUARE = 9,          /* ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2 */
    CSB_D2R2_VIEW_SQUARE = 10,         /* ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2 */
    CSB_D2_VIEW_DEPTH = 2,             /* ReDMCSB DUNVIEW.C:372 G2027[9/10] */
    CSB_D2L2_VIEW_LANE = 254,          /* ReDMCSB DUNVIEW.C:371 G2026[9] == -2 as unsigned char */
    CSB_D2R2_VIEW_LANE = 2,            /* ReDMCSB DUNVIEW.C:371 G2026[10] */
    CSB_D2L2_FIELD_ASPECT = 5,         /* ReDMCSB DUNVIEW.C:377 G2035[9] */
    CSB_D2R2_FIELD_ASPECT = 6,         /* ReDMCSB DUNVIEW.C:377 G2035[10] */
    CSB_D2L2_FIELD_ZONE = 707,         /* ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2 */
    CSB_D2R2_FIELD_ZONE = 708,         /* ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2 */
    CSB_MISSING_ROW = -1,              /* ReDMCSB DUNVIEW.C:373/375/376 G2028/G2033/G2034[9/10] */
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_PROJECTILE_ZONE_BASE = 2900,   /* ReDMCSB DEFS.H:4230 C2900_ZONE_ */
    CSB_PROJECTILE_ZONE_STRIDE = 4,    /* ReDMCSB DUNVIEW.C:5683 row * 4 + ViewCell */
    CSB_OBJECT_ZONE_BASE = 2500,       /* ReDMCSB DEFS.H:4228 C2500_ZONE_ */
    CSB_OBJECT_ZONE_STRIDE = 4,        /* ReDMCSB DUNVIEW.C:5075 row * 4 + ViewCell */
    CSB_CREATURE_ZONE_BASE = 3200,     /* ReDMCSB DEFS.H:4236 C3200_ZONE_ */
    CSB_CREATURE_COORD_STRIDE = 65,    /* ReDMCSB DUNVIEW.C:5616 CoordinateSet * 65 */
    CSB_CREATURE_CELL_STRIDE = 5,      /* ReDMCSB DUNVIEW.C:5616 row * 5 + ViewCell */
    CSB_SHIFT_MASK = 0x8000,           /* ReDMCSB DEFS.H:3517 MASK0x8000_SHIFT_OBJECTS_AND_CREATURES */
    CSB_EXPLOSION_REBIRTH1_ZONE = 3000,/* ReDMCSB DEFS.H:4232 C3000_ZONE_ */
    CSB_EXPLOSION_REBIRTH2_ZONE = 3007,/* ReDMCSB DEFS.H:4233 C3007_ZONE_ */
    CSB_EXPLOSION_CENTERED_ZONE = 3014,/* ReDMCSB DEFS.H:4234 C3014_ZONE_ */
    CSB_EXPLOSION_SIDE_ZONE = 3031,    /* ReDMCSB DEFS.H:4235 C3031_ZONE_ */
    CSB_EXPLOSION_SIDE_STRIDE = 2,     /* ReDMCSB DUNVIEW.C:6121 row * 2 + ViewCell */
    CSB_PROJECTILE_DERIVED_NONE = -1,  /* ReDMCSB DUNVIEW.C:5859 CM1_DERIVED_BITMAP_NONE */
    CSB_TRANSPARENT_COLOR = 10         /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH */
};

static const char s_source_evidence[] =
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 order draws objects, creatures, "
    "projectiles, then explosions. DUNVIEW.C:371-377 maps C09/C10 D2L2/D2R2 "
    "to depth 2 but G2028/G2033/G2034 rows -1 and G2035 field aspects 5/6. "
    "F0115:4806-4811 loads G2028 for object rows; 4923 gates weapon..junk on "
    "L2476>=0 and cell match; 5071-5079 uses C2500_ZONE_ | MASK0x8000 plus "
    "row*4+ViewCell and pile shifts; 5109 blits via F0791 with C10. "
    "F0115:5201-5214 records group markers but rejects G2033<0; 5615-5627 "
    "uses C3200_ZONE_ | MASK0x8000 + CoordinateSet*65 + row*5 + ViewCell. "
    "F0115:5668-5683 maps projectiles through G2028, rejects rows <0, "
    "suppresses depth3 front cells and depth0 back cells, restarts the thing "
    "list, requires C14 projectile and cell match, then computes C2900_ZONE_ "
    "+ row*4+ViewCell; 5859 sets CM1_DERIVED_BITMAP_NONE and 5881-5882 uses "
    "F0791 with C10. F0115:5915-5933 restarts explosions, 5920-5924 loads "
    "G2034/G2035, 5948 rejects rebirth when row <0, 5998-5999/6094-6096/"
    "6106-6107/6121-6122 route C3000/C3007/C3014/C3031, 6192-6193 blits "
    "with C10, and 6202-6219 defers fluxcage to F0113. F0678/F0679 "
    "DUNVIEW.C:6847-6865/6878-6896 expose no D2L2/D2R2 F0115 or F0111 door "
    "front route; only teleporter fields call F0113 with C707/C708.";

static const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec s_routes[] = {
    {
        CSB_D2L2_VIEW_SQUARE,
        CSB_D2L2_VIEW_SQUARE,
        CSB_D2_VIEW_DEPTH,
        CSB_D2L2_VIEW_LANE,
        CSB_D2L2_FIELD_ASPECT,
        CSB_D2L2_FIELD_ZONE,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_MISSING_ROW,
        CSB_PROJECTILE_ZONE_BASE,
        CSB_PROJECTILE_ZONE_STRIDE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_PROJECTILE_DERIVED_NONE,
        CSB_ROUTE_PRESENT,
        CSB_TRANSPARENT_COLOR,
        CSB_MISSING_ROW,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_CREATURE_ZONE_BASE,
        CSB_CREATURE_COORD_STRIDE,
        CSB_CREATURE_CELL_STRIDE,
        CSB_SHIFT_MASK,
        CSB_MISSING_ROW,
        CSB_OBJECT_ZONE_BASE,
        CSB_OBJECT_ZONE_STRIDE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_MISSING_ROW,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_EXPLOSION_REBIRTH1_ZONE,
        CSB_EXPLOSION_REBIRTH2_ZONE,
        CSB_EXPLOSION_CENTERED_ZONE,
        CSB_EXPLOSION_SIDE_ZONE,
        CSB_EXPLOSION_SIDE_STRIDE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_D2L2_FIELD_ZONE,
        "DUNVIEW.C F0678_DrawD2L2 / F0115 route gate",
        s_source_evidence
    },
    {
        CSB_D2R2_VIEW_SQUARE,
        CSB_D2R2_VIEW_SQUARE,
        CSB_D2_VIEW_DEPTH,
        CSB_D2R2_VIEW_LANE,
        CSB_D2R2_FIELD_ASPECT,
        CSB_D2R2_FIELD_ZONE,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_MISSING_ROW,
        CSB_PROJECTILE_ZONE_BASE,
        CSB_PROJECTILE_ZONE_STRIDE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_PROJECTILE_DERIVED_NONE,
        CSB_ROUTE_PRESENT,
        CSB_TRANSPARENT_COLOR,
        CSB_MISSING_ROW,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_CREATURE_ZONE_BASE,
        CSB_CREATURE_COORD_STRIDE,
        CSB_CREATURE_CELL_STRIDE,
        CSB_SHIFT_MASK,
        CSB_MISSING_ROW,
        CSB_OBJECT_ZONE_BASE,
        CSB_OBJECT_ZONE_STRIDE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_MISSING_ROW,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_EXPLOSION_REBIRTH1_ZONE,
        CSB_EXPLOSION_REBIRTH2_ZONE,
        CSB_EXPLOSION_CENTERED_ZONE,
        CSB_EXPLOSION_SIDE_ZONE,
        CSB_EXPLOSION_SIDE_STRIDE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_D2R2_FIELD_ZONE,
        "DUNVIEW.C F0679_DrawD2R2 / F0115 route gate",
        s_source_evidence
    }
};

size_t M11_GameView_D2L2F0115ProjectileRouteSpecCount(void)
{
    return sizeof(s_routes) / sizeof(s_routes[0]);
}

const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *
M11_GameView_D2L2F0115ProjectileRouteSpecAt(size_t index)
{
    if (index >= M11_GameView_D2L2F0115ProjectileRouteSpecCount()) return NULL;
    return &s_routes[index];
}

const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *
M11_GameView_D2L2F0115ProjectileRouteSpecForSquare(int view_square)
{
    for (size_t i = 0; i < M11_GameView_D2L2F0115ProjectileRouteSpecCount(); ++i) {
        if (s_routes[i].view_square == view_square) return &s_routes[i];
    }
    return NULL;
}

int M11_GameView_D2L2F0115ProjectileZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    unsigned char view_cell)
{
    if (!spec || view_cell > 3 || spec->projectile_g2028_row < 0) return -1;
    if (spec->view_depth == 3 && spec->projectile_suppresses_depth3_front_cells &&
        view_cell <= 1) return -1;
    if (spec->view_depth == 0 && spec->projectile_suppresses_depth0_back_cells &&
        view_cell >= 2) return -1;
    return spec->projectile_zone_base +
           (spec->projectile_g2028_row * spec->projectile_zone_cell_stride) +
           view_cell;
}

int M11_GameView_D2L2F0115ProjectileObjectZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    unsigned char view_cell)
{
    if (!spec || view_cell > 3 || spec->object_g2028_row < 0) return -1;
    return (spec->object_zone_base | spec->shift_objects_and_creatures_mask) +
           (spec->object_g2028_row * spec->object_zone_cell_stride) +
           view_cell;
}

int M11_GameView_D2L2F0115ProjectileCreatureZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    int coordinate_set,
    unsigned char view_cell)
{
    if (!spec || coordinate_set < 0 || view_cell > 4 || spec->creature_g2033_row < 0) {
        return -1;
    }
    return (spec->creature_zone_base | spec->shift_objects_and_creatures_mask) +
           (coordinate_set * spec->creature_coordinate_set_stride) +
           (spec->creature_g2033_row * spec->creature_zone_cell_stride) +
           view_cell;
}

int M11_GameView_D2L2F0115ProjectileExplosionRebirthStep1Zone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec)
{
    if (!spec || spec->explosion_g2034_row < 0) return -1;
    return spec->explosion_rebirth_step1_zone_base + spec->explosion_g2034_row;
}

int M11_GameView_D2L2F0115ProjectileExplosionRebirthStep2Zone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec)
{
    if (!spec || spec->explosion_g2034_row < 0) return -1;
    return spec->explosion_rebirth_step2_zone_base + spec->explosion_g2034_row;
}

int M11_GameView_D2L2F0115ProjectileExplosionCenteredZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec)
{
    if (!spec || spec->explosion_g2034_row < 0) return -1;
    return spec->explosion_centered_zone_base + spec->explosion_g2034_row;
}

int M11_GameView_D2L2F0115ProjectileExplosionSideZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    unsigned char view_cell)
{
    if (!spec || view_cell > 1 || spec->explosion_g2034_row < 0) return -1;
    return spec->explosion_side_zone_base +
           (spec->explosion_g2034_row * spec->explosion_side_zone_cell_stride) +
           view_cell;
}

int M11_GameView_D2L2F0115ProjectileTeleporterFieldZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec)
{
    if (!spec || !spec->f0113_teleporter_route) return -1;
    return spec->field_zone;
}

int M11_GameView_D2L2F0115ProjectileApplySyntheticC10Blit(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride < width || destination_stride < width ||
        width <= 0 || height <= 0) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C F0115 lines 5881-5882 and 6192-6193 dispatch
     * MEDIA709 bitmap routes through F0791 with C10_COLOR_FLESH transparency.
     * D2L2/D2R2 compute no live zone because G2028/G2034 rows are -1; this
     * helper isolates only the C10 synthetic blit contract for the gate test. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->projectile_transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

const char *M11_GameView_D2L2F0115ProjectileSourceEvidence(void)
{
    return s_source_evidence;
}
