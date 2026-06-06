/*
 * CSB V1 Viewport Rendering — pc34 compat implementation
 *
 * Source-locked to ReDMCSB WIP20210206, Toolchains/Common/Source/:
 *   DUNVIEW.C:6226-6353 F0676/F0677 (CSB back-wall D3L2/D3R2)
 *   DUNVIEW.C:6837-6896 F0678/F0679 (CSB near-wall D2L2/D2R2)
 *   DUNVIEW.C:8318-8542 F0128 (shared DM1/CSB viewport draw core)
 *
 * CSB differences from DM1:
 *   - D3L2/D3R2 back-wall positions render all 8 element types
 *     (WALL, TELEPORTER, STAIRS_FRONT, PIT, CORRIDOR, DOOR_SIDE,
 *      DOOR_FRONT, STAIRS_SIDE)
 *   - D2L2/D2R2 four-sided decoration positions render WALL only
 *     (no stairs, pits, floor ornaments, creatures, items, projectiles)
 *   - Four-sided wall decoration rules differ from DM1 corridor sides
 *   - Custom backgrounds (CSBWin/CSBCode.cpp:26 CustomBackgrounds)
 *   - CSB wall set index selection per current map
 *
 * Reference: CSBWin/Viewport.cpp (7290 lines) · CSBWin/Graphics.cpp (3186 lines)
 *   CSBWin/CSBCode.cpp:26 CustomBackgrounds · CSBWin/CSBCode.cpp:9196
 */

#include "csb_v1_viewport_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include <string.h>

enum {
    CSB_V1_ORNAMENT_SLOT_RIGHT = 1, /* M551_RIGHT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_ORNAMENT_SLOT_LEFT = 3,  /* M553_LEFT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_VIEW_WALL_D3L2_RIGHT = 0,
    CSB_V1_VIEW_WALL_D3R2_LEFT = 1,
    CSB_V1_NO_ORNAMENT_SLOT = -1,
    CSB_V1_NO_VIEW_WALL = -1,
    CSB_V1_VIEW_FLOOR_D3L2 = 0, /* C00_VIEW_FLOOR_D3L2 */
    CSB_V1_VIEW_FLOOR_D3R2 = 1, /* C01_VIEW_FLOOR_D3R2 */
    CSB_V1_CELL_ORDER_D3L2_DOORPASS1 = 0x0218,
    CSB_V1_CELL_ORDER_D3L2_DOORPASS2 = 0x0349,
    CSB_V1_CELL_ORDER_D3R2_DOORPASS1 = 0x0128,
    CSB_V1_CELL_ORDER_D3R2_DOORPASS2 = 0x0439,
    CSB_V1_CELL_ORDER_D3L2_CORRIDOR = 0x3421,
    CSB_V1_CELL_ORDER_D3R2_CORRIDOR = 0x4312,
    CSB_V1_CELL_ORDER_D3L2_SIDE = 0x0321,
    CSB_V1_CELL_ORDER_D3R2_SIDE = 0x0412,
    CSB_V1_ZONE_DOOR_D3L2 = 3700,
    CSB_V1_ZONE_DOOR_D3R2 = 3710,
    CSB_V1_REDMCSB_VIEW_SQUARE_D3L2 = 14, /* C14_VIEW_SQUARE_D3L2 */
    CSB_V1_REDMCSB_VIEW_SQUARE_D3R2 = 15, /* C15_VIEW_SQUARE_D3R2 */
    CSB_V1_VIEW_DEPTH_D3 = 3,
    CSB_V1_OBJECT_ROW_D3L2 = 3, /* G2028_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_OBJECT_ROW_D3R2 = 4, /* G2028_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL = 3,
    CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL = 4,
    CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED = 1,
    CSB_V1_DOOR_PANEL_PARENT_D3L2 = 129,
    CSB_V1_DOOR_PANEL_PARENT_D3R2 = 130,
    CSB_V1_DOOR_PANEL_CLIP_D3 = 126,
    CSB_V1_DOOR_PANEL_NATIVE_W_D3 = 48,
    CSB_V1_DOOR_PANEL_NATIVE_H_D3 = 41,
    CSB_V1_DOOR_PANEL_CLIPPED_H_D3 = 40,
    CSB_V1_DOOR_PANEL_D3L2_X = 24,
    CSB_V1_DOOR_PANEL_D3R2_X = 88,
    CSB_V1_DOOR_PANEL_D3_Y = 28,
    CSB_V1_DOOR_ORNAMENT_D3LCR = 0, /* C0_VIEW_DOOR_ORNAMENT_D3LCR */
    CSB_V1_DOOR_TRANSPARENT_COLOR = 10 /* C10_COLOR_FLESH */
};

static const CSB_V1_ViewportWallOrnamentRouteSpec s_wall_ornament_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        DM1_PC34_ZONE_WALL_D3L2,
        1,
        CSB_V1_ORNAMENT_SLOT_RIGHT,
        CSB_V1_VIEW_WALL_D3L2_RIGHT,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6254-6263 wall panel then F0107(M551_RIGHT_WALL_ORNAMENT_ORDINAL, C00_VIEW_WALL_D3L2_RIGHT); DEFS.H:2696"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        DM1_PC34_ZONE_WALL_D3R2,
        1,
        CSB_V1_ORNAMENT_SLOT_LEFT,
        CSB_V1_VIEW_WALL_D3R2_LEFT,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6321-6330 wall panel then F0107(M553_LEFT_WALL_ORNAMENT_ORDINAL, C01_VIEW_WALL_D3R2_LEFT); DEFS.H:2697"
    },
    {
        (int)DM1_VIEW_SQUARE_D2L2,
        DM1_PC34_ZONE_WALL_D2L2,
        0,
        CSB_V1_NO_ORNAMENT_SLOT,
        CSB_V1_NO_VIEW_WALL,
        "F0678_DrawD2L2",
        "DUNVIEW.C:6848-6865 wall case returns without F0107; teleporter field is the only non-wall draw"
    },
    {
        (int)DM1_VIEW_SQUARE_D2R2,
        DM1_PC34_ZONE_WALL_D2R2,
        0,
        CSB_V1_NO_ORNAMENT_SLOT,
        CSB_V1_NO_VIEW_WALL,
        "F0679_DrawD2R2",
        "DUNVIEW.C:6877-6896 wall case returns without F0107; teleporter field is the only non-wall draw"
    },
};

/* ReDMCSB: DUNVIEW.C F0676/F0677 lines 6270-6286 and 6337-6353.
 * The CSB-only D3L2/D3R2 routes call F0108 before the rear F0115 pass,
 * call F0111 for door-front panels, then finish with the front F0115 pass.
 * Pit cases fall through to the same F0108 call, preserving BUG0_64. */
static const CSB_V1_ViewportFloorOrnamentRouteSpec s_floor_ornament_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        1,
        1,
        1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS2,
        CSB_V1_ZONE_DOOR_D3L2,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108 door-front floor ornament; 6271 F0115 pass1; 6272 F0111 C3700_ZONE_DOOR_D3L2; 6282-6286 pit/corridor F0108 then F0115"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        1,
        1,
        1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS2,
        CSB_V1_ZONE_DOOR_D3R2,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108 door-front floor ornament; 6338 F0115 pass1; 6339 F0111 C3710_ZONE_DOOR_D3R2; 6349-6353 pit/corridor F0108 then F0115"
    },
};

/* ReDMCSB: DUNVIEW.C F0676 lines 6270-6286 and F0677 lines 6337-6353.
 * CSB's extra D3L2/D3R2 squares route the same F0115 stack as DM1: objects
 * first, a creature after object cells, projectiles after creatures, and
 * explosions after all cells. Door-front squares split F0115 around F0111:
 * rear cells before the door panel/frame, front cells after it. */
static const CSB_V1_ViewportThingPassOrderSpec s_thing_pass_order_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        0,
        1,
        2,
        3,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS2,
        CSB_V1_CELL_ORDER_D3L2_CORRIDOR,
        CSB_V1_CELL_ORDER_D3L2_SIDE,
        0,
        1,
        2,
        3,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108; 6271 F0115 door rear cells 0x0218; 6272 F0111 door; 6284 F0108 pit/corridor BUG0_64; 6286 F0115 final/corridor/side; F0115:4567-4581 objects/creatures/projectiles, 5915-5933 explosions"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        0,
        1,
        2,
        3,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS2,
        CSB_V1_CELL_ORDER_D3R2_CORRIDOR,
        CSB_V1_CELL_ORDER_D3R2_SIDE,
        0,
        1,
        2,
        3,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108; 6338 F0115 door rear cells 0x0128; 6339 F0111 door; 6351 F0108 pit/corridor BUG0_64; 6353 F0115 final/corridor/side; F0115:4567-4581 objects/creatures/projectiles, 5915-5933 explosions"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4806-4811 and 4923.
 * For the PC34/I34E path, D3L2/D3R2 map through G2027/G2028 to depth 3
 * rows 3/4.  The object predicate accepts only weapon..junk things on the
 * processed cell; depth 3 squares suppress front-left/front-right cells by
 * requiring AL0126_i_ViewCell > C01_VIEW_CELL_FRONT_RIGHT. */
static const CSB_V1_ViewportObjectVisibilitySpec s_object_visibility_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3L2,
        1,
        1,
        1,
        0,
        CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL,
        CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL,
        "F0676_DrawD3L2",
        "DUNVIEW.C:371-373 G2026/G2027/G2028; 4806-4811 loads lane/depth/object row; 4923 F0115 weapon..junk, L2476>=0, cell match, depth3 front-cell suppression"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3R2,
        1,
        1,
        1,
        0,
        CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL,
        CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL,
        "F0677_DrawD3R2",
        "DUNVIEW.C:371-373 G2026/G2027/G2028; 4806-4811 loads lane/depth/object row; 4923 F0115 weapon..junk, L2476>=0, cell match, depth3 front-cell suppression"
    },
};

/* ReDMCSB: DUNVIEW.C F0111 lines 4218-4337, F0676/F0677 lines
 * 6271-6273 and 6338-6340, DEFS.H lines 4250-4251, COORD.C lines
 * 1545-1565 and 781-807.  The CSB-only far door panels reuse the
 * D3 native 48x41 door bitmap but clip it through COORD.C record 126's
 * 48x40 viewport sub-zone.  F0111 skips state 0, shifts zone ids by
 * door state for partially-open doors, and blits with C10 transparency. */
static const CSB_V1_ViewportDoorPanelBlitSpec s_door_panel_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_ZONE_DOOR_D3L2,
        CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED,
        CSB_V1_DOOR_PANEL_PARENT_D3L2,
        CSB_V1_DOOR_PANEL_D3L2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_PANEL_CLIP_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_NATIVE_H_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_CLIPPED_H_D3,
        CSB_V1_DOOR_PANEL_D3L2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_ORNAMENT_D3LCR,
        1,
        1,
        6,
        3,
        CSB_V1_DOOR_TRANSPARENT_COLOR,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6271 F0115 rear pass; 6272 F0111(... C3700_ZONE_DOOR_D3L2); 6273-6286 F0115 front pass. F0111:4248 skips C0 open, 4298-4321 shifts zone by state/horizontal halves, 4331 F0791 blits with C10. DEFS.H:4250; COORD.C:1548-1565 records 120/126/129 and 788-797 zone 3700..3709."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_ZONE_DOOR_D3R2,
        CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED,
        CSB_V1_DOOR_PANEL_PARENT_D3R2,
        CSB_V1_DOOR_PANEL_D3R2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_PANEL_CLIP_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_NATIVE_H_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_CLIPPED_H_D3,
        CSB_V1_DOOR_PANEL_D3R2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_ORNAMENT_D3LCR,
        1,
        1,
        6,
        3,
        CSB_V1_DOOR_TRANSPARENT_COLOR,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6338 F0115 rear pass; 6339 F0111(... C3710_ZONE_DOOR_D3R2); 6340-6353 F0115 front pass. F0111:4248 skips C0 open, 4298-4321 shifts zone by state/horizontal halves, 4331 F0791 blits with C10. DEFS.H:4251; COORD.C:1548-1565 records 120/126/130 and 798-807 zone 3710..3719."
    },
};

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->ambient_color = 0xFF000000; /* black ambient */
    cfg->viewport_pixels = NULL;
    cfg->viewport_stride = 320;
    cfg->dungeon_grid = NULL;
    cfg->dungeon_width = 0;
    cfg->dungeon_height = 0;
}

void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set) {
    if (cfg) cfg->wall_set_index = set;
}

void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id) {
    if (cfg) cfg->custom_background = bg_id;
}

void csb_v1_viewport_set_dungeon_grid(CSB_V1_ViewportConfig *cfg,
                                       const uint8_t *grid,
                                       int width, int height) {
    if (!cfg) return;
    cfg->dungeon_grid = grid;
    cfg->dungeon_width = width;
    cfg->dungeon_height = height;
}

/* csb_v1_viewport_render_frame — integration entry point
 *
 * Renders the CSB dungeon view by delegating to the shared DM1 V1 viewport
 * engine (dm1_viewport_3d_draw_frame).  CSB config provides:
 *   - viewport_pixels + stride: the pixel buffer to draw into
 *   - dungeon_grid/width/height: square type data for element routing
 *   - wall_set_index: selects which GRAPHICS.DAT wall set to use
 *
 * When viewport_pixels is NULL, this is a no-op (allows staged integration).
 *
 * Source: CSBWin/Viewport.cpp F0128 passthrough; ReDMCSB DUNVIEW.C F0128
 */
void csb_v1_viewport_render_frame(CSB_V1_ViewportConfig *cfg,
                                   int party_dir,
                                   int party_x,
                                   int party_y)
{
    if (!cfg) return;

    /* Guard: no viewport buffer means not yet initialised */
    if (!cfg->viewport_pixels) return;

    /* Set up a DM1 viewport state backed by our CSB pixel buffer.
     * We share the exact same pixel format (320×200 indexed, viewport
     * sub-region 224×136 at screen row 33) so the DM1 draw primitives
     * work without modification. */
    DM1_Viewport3DState vp;
    memset(&vp, 0, sizeof(vp));
    vp.viewport_pixels = cfg->viewport_pixels;
    vp.viewport_stride = cfg->viewport_stride > 0 ? cfg->viewport_stride : 320;
    vp.floor_area = cfg->viewport_pixels +
                    DM1_VIEWPORT_FLOOR_Y * vp.viewport_stride;
    vp.floor_ceiling_dirty = true;

    /* Wire dungeon grid for CSB back-wall rendering (D3L2/D3R2/D2L2/D2R2).
     * The dungeon grid enables element-specific routing for CSB four-sided
     * wall decoration: walls, doors, stairs, pits, teleporters, corridors.
     * When dungeon_grid is NULL, CSB back-wall rendering falls back to
     * the generic wall-drawing path (same as DM1).
     *
     * Grid layout: dungeon_grid[y * dungeon_width + x] = raw dungeon cell.
     * Raw cell: low 5 bits = element type (0=WALL, 1=CORRIDOR, 2=PIT,
     * 3=STAIRS, 4=DOOR, 5=TELEPORTER, 6=FAKEWALL).
     *
     * Source: ReDMCSB DUNVIEW.C:6226-6353 F0676/F0677; 6837-6896 F0678/F0679 */
    vp.dungeon_grid   = cfg->dungeon_grid;
    vp.dungeon_width  = cfg->dungeon_width;
    vp.dungeon_height = cfg->dungeon_height;

    /* Wall set index — CSB may use a different wall set than DM1.
     * ReDMCSB DUNVIEW.C F0096 loads wall set based on current map index.
     * Here we accept the index from config; 0 = default CSB wall set. */
    dm1_viewport_3d_load_wall_set(&vp, cfg->wall_set_index, 0);

    /* Main draw call — mirrors ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF.
     * Draws wall frames for D4 far objects, then D3L/R/C → D2 → D1 → D0
     * back-to-front with correct depth occlusion and parity flip.
     *
     * The CSB-specific elements (back-walls D3L2/D3R2, near-walls D2L2/D2R2)
     * are handled by dm1_viewport_3d_draw_csb_back_wall and
     * dm1_viewport_3d_draw_csb_near_wall, which are invoked from the
     * dm1_viewport_3d_draw_frame wall loop for D3L2/D3R2/D2L2/D2R2 positions.
     *
     * Custom background rendering — TODO (pass604).
     * CSBWin/CSBCode.cpp:26 CustomBackgrounds is a CSB-specific feature
     * that replaces the standard floor/ceiling rendering with a
     * per-map custom backdrop.  The background ID (cfg->custom_background)
     * indexes into the CSB custom background table.  This is distinct from
     * the DM1 floor/ceiling rendering which uses G2108/G2109 bitmap indices.
     * ReDMCSB does not have a clear reference for this in DUNVIEW.C;
     * it is specific to the CSBWin implementation.
     * Source: CSBWin/CSBCode.cpp:26 CustomBackgrounds (CSB custom feature) */

    dm1_viewport_3d_draw_frame(&vp, party_dir, party_x, party_y);
}

size_t csb_v1_viewport_wall_ornament_route_spec_count(void)
{
    return sizeof(s_wall_ornament_routes) / sizeof(s_wall_ornament_routes[0]);
}

const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_route_spec_count()) return NULL;
    return &s_wall_ornament_routes[index];
}

const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_wall_ornament_route_spec_count(); ++i) {
        if (s_wall_ornament_routes[i].view_square == view_square) {
            return &s_wall_ornament_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_floor_ornament_route_spec_count(void)
{
    return sizeof(s_floor_ornament_routes) / sizeof(s_floor_ornament_routes[0]);
}

const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec(size_t index)
{
    if (index >= csb_v1_viewport_floor_ornament_route_spec_count()) return NULL;
    return &s_floor_ornament_routes[index];
}

const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_floor_ornament_route_spec_count(); ++i) {
        if (s_floor_ornament_routes[i].view_square == view_square) {
            return &s_floor_ornament_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_thing_pass_order_spec_count(void)
{
    return sizeof(s_thing_pass_order_routes) / sizeof(s_thing_pass_order_routes[0]);
}

const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec(size_t index)
{
    if (index >= csb_v1_viewport_thing_pass_order_spec_count()) return NULL;
    return &s_thing_pass_order_routes[index];
}

const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_thing_pass_order_spec_count(); ++i) {
        if (s_thing_pass_order_routes[i].view_square == view_square) {
            return &s_thing_pass_order_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_object_visibility_spec_count(void)
{
    return sizeof(s_object_visibility_routes) / sizeof(s_object_visibility_routes[0]);
}

const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec(size_t index)
{
    if (index >= csb_v1_viewport_object_visibility_spec_count()) return NULL;
    return &s_object_visibility_routes[index];
}

const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_object_visibility_spec_count(); ++i) {
        if (s_object_visibility_routes[i].view_square == view_square) {
            return &s_object_visibility_routes[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_object_visibility_allows_cell(const CSB_V1_ViewportObjectVisibilitySpec *spec,
                                                  unsigned char cell_ordinal)
{
    if (!spec) return 0;
    if (cell_ordinal < spec->first_visible_cell_ordinal) return 0;
    if (cell_ordinal > spec->last_visible_cell_ordinal) return 0;
    return 1;
}

size_t csb_v1_viewport_door_panel_blit_spec_count(void)
{
    return sizeof(s_door_panel_blits) / sizeof(s_door_panel_blits[0]);
}

const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_door_panel_blit_spec_count()) return NULL;
    return &s_door_panel_blits[index];
}

const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_door_panel_blit_spec_count(); ++i) {
        if (s_door_panel_blits[i].view_square == view_square) {
            return &s_door_panel_blits[i];
        }
    }
    return NULL;
}

const char *csb_v1_viewport_source_evidence(void) {
    return
        "ReDMCSB WIP20210206 Toolchains/Common/Source/DUNVIEW.C:\n"
        "  6226-6353 F0676/F0677 back-wall D3L2/D3R2 element routing\n"
        "  6254-6263 F0676 D3L2 wall panel then F0107 right-wall ornament route\n"
        "  6270-6286 F0676 D3L2 F0108 floor ornament, F0115 pass1/pass2, F0111 door panel route\n"
        "  6321-6330 F0677 D3R2 wall panel then F0107 left-wall ornament route\n"
        "  6337-6353 F0677 D3R2 F0108 floor ornament, F0115 pass1/pass2, F0111 door panel route\n"
        "  4567-4581 F0115 draws objects, then creatures, then projectiles per processed view cell\n"
        "  4806-4811 F0115 maps PC34 view square to lane/depth/object visibility rows\n"
        "  4923 F0115 filters weapon..junk objects by visible row, matching cell, and D3/D0 cell gates\n"
        "  5915-5933 F0115 restarts for explosions after all processed view cells\n"
        "  3940-4008 F0108 floor ornament bitmap/index/flip dispatch\n"
        "  4218-4337 F0111 door bitmap, ornament, state, zone shift, and C10 transparent blit dispatch\n"
        "  6837-6896 F0678/F0679 near-wall D2L2/D2R2 element routing\n"
        "  6848-6865 F0678 and 6877-6896 F0679 return for walls without F0107\n"
        "  8318-8542 F0128 shared viewport draw sequence\n"
        "  DEFS.H:4250-4251 C3700_ZONE_DOOR_D3L2 / C3710_ZONE_DOOR_D3R2\n"
        "  COORD.C:1548-1565 D3 48x41 native door bitmap and 48x40 clip records; 788-807 far door zones\n"
        "  G0711/G0712 back-wall frame descriptors (lines 579-580)\n"
        "  G2107 WallSet bitmap indices (lines ~183)\n"
        "  G3048 WallSetFlipped (lines 277-295)\n"
        "ReDMCSB DEFS.H:2696-2697 C00_VIEW_WALL_D3L2_RIGHT / C01_VIEW_WALL_D3R2_LEFT\n"
        "CSBWin/Viewport.cpp: 7290 lines viewport rendering\n"
        "CSBWin/Graphics.cpp: 3186 lines asset cache\n"
        "CSBWin/CSBCode.cpp:26 CustomBackgrounds\n"
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack (prison door)\n";
}
