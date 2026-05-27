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
    CSB_V1_NO_VIEW_WALL = -1
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

const char *csb_v1_viewport_source_evidence(void) {
    return
        "ReDMCSB WIP20210206 Toolchains/Common/Source/DUNVIEW.C:\n"
        "  6226-6353 F0676/F0677 back-wall D3L2/D3R2 element routing\n"
        "  6254-6263 F0676 D3L2 wall panel then F0107 right-wall ornament route\n"
        "  6321-6330 F0677 D3R2 wall panel then F0107 left-wall ornament route\n"
        "  6837-6896 F0678/F0679 near-wall D2L2/D2R2 element routing\n"
        "  6848-6865 F0678 and 6877-6896 F0679 return for walls without F0107\n"
        "  8318-8542 F0128 shared viewport draw sequence\n"
        "  G0711/G0712 back-wall frame descriptors (lines 579-580)\n"
        "  G2107 WallSet bitmap indices (lines ~183)\n"
        "  G3048 WallSetFlipped (lines 277-295)\n"
        "ReDMCSB DEFS.H:2696-2697 C00_VIEW_WALL_D3L2_RIGHT / C01_VIEW_WALL_D3R2_LEFT\n"
        "CSBWin/Viewport.cpp: 7290 lines viewport rendering\n"
        "CSBWin/Graphics.cpp: 3186 lines asset cache\n"
        "CSBWin/CSBCode.cpp:26 CustomBackgrounds\n"
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack (prison door)\n";
}
