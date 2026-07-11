/*
 * dm2_v1_viewport_renderer.c — DM2 V1 Viewport Rendering Pipeline
 *
 * Phase 3: DM2 viewport, UI chrome, items, outdoor/indoor presentation.
 *
 * Architecture:
 *   DM2 viewport is 320×200 game pixels, same as DM1/CSB.
 *   Status bar: top 28px  (champion health/magic/conditions)
 *   Dungeon view: 320×144px (walls, floor, creatures, items)
 *   Action strip: bottom 28px (action icons: Attack/Cast/Use/Drop/Move)
 *   Portrait panel: right 80×144px (champion portraits)
 *
 * DM2 differs from DM1:
 *   - Rooms vs corridors (DM2 is not a dungeon-corridor game)
 *   - Different wall set indices (G2107/G3060 variants)
 *   - Outdoor mode (sky gradient, weather, buildings)
 *   - Different UI chrome (gold counter, no champion portrait panel)
 *
 * Source: SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 *         SKULL.ASM T520  — party/movement tick
 *         ReDMCSB DUNGEON.C — draw order, wall bitmap selection
 *         ReDMCSB DUNVIEW.C:575-586 — G0163 wall frame table
 *         ReDMCSB DUNVIEW.C:148-165  — wall set indices
 *         ReDMCSB DUNVIEW.C:2962-3047 — F0098 DrawFloorAndCeiling
 *         ReDMCSB DUNVIEW.C:3048-3070 — F0100 DrawWallSetBitmap
 *         ReDMCSB DUNVIEW.C:3082-3095 — F0102 DrawDoorBitmap
 *         ReDMCSB DUNVIEW.C:3940-4015 — F0108 DrawFloorOrnament
 *         ReDMCSB DUNVIEW.C:4016-4050 — F0109 DrawDoorOrnament
 *         ReDMCSB DUNVIEW.C:4119-4270 — F0110 DrawDoorButton, F0111 DrawDoor
 *         ReDMCSB DUNVIEW.C:4351-4382 — F0112 DrawCeilingPit
 *         SKULLWIN/SKWIN/c_gui_vp.cpp — viewport blit order
 *         docs/dm2_graphics.md — drawing pipeline audit
 *         docs/dm2_walls.md — wall/door/floor rendering specifics
 *         docs/dm2_palette.md — DM2 palette system
 */

#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_world_model.h"
#include "dm2_v1_outdoor_renderer.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ── Lighting helper: DM2 object illumination decay ──────────────── */
/* ReDMCSB DUNVIEW.C F0115:4960-5037 uses a per-view depth scale table for
 * object sprites; outside the valid depth window objects are not drawn. We
 * model the same boundary as a hard light-radius clip, where distance values
 * at or beyond the source radius extinguish to zero. */
uint8_t dm2_v1_viewport_object_light_level(uint8_t base_light_level,
                                           int distance_tiles,
                                           const DM2_CreatureSprite *source)
{
    if (!source) return base_light_level;
    if (source->light_radius == 0) return 0;
    if (distance_tiles < 0) return base_light_level;
    if (distance_tiles >= (int)source->light_radius) return 0;

    return (uint8_t)((int)base_light_level *
                     ((int)source->light_radius - distance_tiles) /
                     (int)source->light_radius);
}

int dm2_v1_viewport_project_map_to_sprite(
    int map_x,
    int map_y,
    int party_dir,
    int party_x,
    int party_y,
    DM2_V1_ViewportSpritePlacement *out)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    static const int y_by_depth[4] = { 98, 84, 72, 62 };
    static const int lateral_step_by_depth[4] = { 48, 40, 30, 22 };
    const int center_x = 112;
    int dir;
    int right;
    int rel_x;
    int rel_y;
    int forward;
    int lateral;
    int depth;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dir = party_dir & 3;
    right = (dir + 1) & 3;
    rel_x = map_x - party_x;
    rel_y = map_y - party_y;
    forward = rel_x * dx[dir] + rel_y * dy[dir];
    lateral = rel_x * dx[right] + rel_y * dy[right];

    if (forward < 1 || forward > 4 || lateral < -2 || lateral > 2) {
        return 0;
    }

    /* skproject/SKWIN renders missiles and creature-carried map chips through
     * the visible cell order before DRAW_MAP_CHIP scales the selected sprite.
     * This helper owns Firestaff's bounded first-person placement contract for
     * those runtime-produced map-coordinate overlays. */
    depth = forward - 1;
    out->visible = 1;
    out->depth = depth;
    out->screen_x = center_x + lateral * lateral_step_by_depth[depth];
    out->screen_y = y_by_depth[depth];
    return 1;
}

int dm2_v1_viewport_possession_slot_placement(
    const DM2_V1_ViewportSpritePlacement *base,
    int possession_slot,
    DM2_V1_ViewportSpritePlacement *out)
{
    int slot;

    if (!base || !out || !base->visible) {
        if (out) memset(out, 0, sizeof(*out));
        return 0;
    }
    slot = possession_slot;
    if (slot < 0) {
        slot = 0;
    }
    *out = *base;
    /* skproject walks Creature::possession in chain order and draws each
     * dbWeapon..dbMiscellaneous_item as a separate map-chip overlay.  The
     * bounded Firestaff bridge keeps stable per-slot separation until the
     * exact source placement table is fully decoded. */
    out->screen_x = base->screen_x + slot * 6;
    out->screen_y = base->screen_y + slot * 4;
    return 1;
}

int dm2_v1_viewport_calc_stretched_size(int value, int factor64)
{
    /* skproject/SKWIN/SkWinCore.cpp CALC_STRETCHED_SIZE:
     * (i16(val * fact64) + (fact64 >> 1)) >> 6. */
    return (int)(((int16_t)(value * factor64) + (factor64 >> 1)) >> 6);
}

int dm2_v1_viewport_rotate_5x5_pos(int pos5x5, int dir)
{
    int x;
    int y;
    int tmp;

    if (pos5x5 < 0 || pos5x5 > 24) {
        return -1;
    }
    x = (pos5x5 % 5) - 2;
    y = (pos5x5 / 5) - 2;
    switch (dir & 3) {
    case 1:
        tmp = x;
        x = y;
        y = -tmp;
        break;
    case 2:
        x = -x;
        y = -y;
        break;
    case 3:
        tmp = x;
        x = -y;
        y = tmp;
        break;
    default:
        break;
    }
    return x + ((y + 2) * 5) + 2;
}

int dm2_v1_viewport_creature_blit_rect_id(int cell_pos,
                                          int pos5x5,
                                          int dir)
{
    int rotated = dm2_v1_viewport_rotate_5x5_pos(pos5x5, dir);

    if (cell_pos < 0 || cell_pos > 3 || rotated < 0) {
        return -1;
    }
    /* skproject/SKWIN/SkWinCore.cpp QUERY_CREATURE_BLIT_RECTI returns
     * ROTATE_5x5_POS(pos, dir) + cellPos * 25 + 5000. */
    return rotated + (cell_pos * 25) + 5000;
}

int dm2_v1_viewport_interface_rect14_placement(
    const uint8_t row14[14],
    int cell_pos,
    int distance_stretch_factor64,
    DM2_V1_InterfaceRect14Placement *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!row14 || cell_pos < 0 || cell_pos > 3 ||
        row14[0] > 24u || distance_stretch_factor64 <= 0) {
        return 0;
    }

    /* skproject/SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST consumes
     * _4976_5a98[row][0] as the 5x5 anchor, [1] as a signed side offset,
     * [2..5] as direction image fields, [6..9] via CALC_STRETCHED_SIZE,
     * and [10..13] as per-direction flags. */
    out->valid = 1;
    out->base_5x5 = row14[0];
    out->lateral_offset = (int8_t)row14[1];
    out->cell_pos = (uint16_t)cell_pos;
    for (int dir = 0; dir < 4; ++dir) {
        int rect_id = dm2_v1_viewport_creature_blit_rect_id(
            cell_pos, row14[0], dir);
        if (rect_id < 0) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        out->blit_rect_id[dir] = (uint16_t)rect_id;
        out->image_field[dir] = row14[2 + dir];
        out->stretch_source[dir] = row14[6 + dir];
        out->stretched_size[dir] = (uint16_t)
            dm2_v1_viewport_calc_stretched_size(row14[6 + dir],
                                                distance_stretch_factor64);
        out->flags[dir] = row14[10 + dir];
    }
    return 1;
}

int dm2_v1_viewport_build_hud_chrome_plan(
    int is_outdoor,
    DM2_V1_HudChromeRenderPlan *out_plan)
{
    static const uint8_t icon_x[DM2_V1_HUD_ACTION_ICON_COUNT] =
        { 20, 70, 120, 170, 220 };
    const int action_y = DM2_VP_HEIGHT - DM2_VP_CHROME_BOT;
    const int panel_x = 240;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    out_plan->outdoor = is_outdoor ? 1 : 0;
    out_plan->top_bar_rect =
        (DM2_V1_ViewportRect){ 0, 0, DM2_VP_WIDTH, DM2_VP_CHROME_TOP };
    out_plan->top_divider_rect =
        (DM2_V1_ViewportRect){ 0, DM2_VP_CHROME_TOP, DM2_VP_WIDTH, 1 };
    out_plan->action_strip_rect =
        (DM2_V1_ViewportRect){ 0, action_y, DM2_VP_WIDTH, DM2_VP_CHROME_BOT };
    out_plan->action_divider_rect =
        (DM2_V1_ViewportRect){ 0, action_y - 1, DM2_VP_WIDTH, 1 };
    out_plan->gold_box_rect =
        (DM2_V1_ViewportRect){ DM2_VP_WIDTH - 40, action_y + 4, 36, 16 };
    out_plan->gold_coin_rect =
        (DM2_V1_ViewportRect){ out_plan->gold_box_rect.x + 2,
                               out_plan->gold_box_rect.y + 2, 12, 12 };
    out_plan->gold_label_rect =
        (DM2_V1_ViewportRect){ out_plan->gold_box_rect.x + 16,
                               out_plan->gold_box_rect.y + 3, 14, 7 };
    /* skproject SKULLWIN/c_gdatfile.cpp DM2_LOAD_GDAT_INTERFACE_00_02
     * loads interface category 1/image 0/field 7 before the runtime HUD
     * is drawn.  Firestaff keeps these as renderer-private GDAT keys so
     * the asset provider can resolve real GRAPHICS.DAT surfaces. */
    out_plan->top_bar_gdat_index =
        dm2_v1_viewport_hud_core_graphic_index(
            DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR);
    out_plan->action_strip_gdat_index =
        dm2_v1_viewport_hud_core_graphic_index(
            DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_STRIP);
    out_plan->gold_box_gdat_index =
        dm2_v1_viewport_hud_core_graphic_index(
            DM2_V1_VIEWPORT_GFX_HUD_CORE_GOLD_BOX);
    out_plan->action_icon_count = DM2_V1_HUD_ACTION_ICON_COUNT;
    for (int i = 0; i < out_plan->action_icon_count; ++i) {
        DM2_V1_HudIconRender *icon = &out_plan->action_icons[i];
        icon->frame_rect =
            (DM2_V1_ViewportRect){ icon_x[i], action_y + 6, 20, 16 };
        icon->fill_rect =
            (DM2_V1_ViewportRect){ icon->frame_rect.x + 2,
                                   icon->frame_rect.y + 2, 16, 12 };
        icon->fill_color = (uint8_t)(8 + i);
        icon->gdat_index = dm2_v1_viewport_hud_action_icon_graphic_index(i);
    }
    if (!out_plan->outdoor) {
        out_plan->portrait_separator_dark_rect =
            (DM2_V1_ViewportRect){ panel_x, DM2_VP_CHROME_TOP, 1,
                                   DM2_VP_HEIGHT - DM2_VP_CHROME_TOP -
                                       DM2_VP_CHROME_BOT };
        out_plan->portrait_separator_light_rect =
            (DM2_V1_ViewportRect){ panel_x + 1, DM2_VP_CHROME_TOP, 1,
                                   DM2_VP_HEIGHT - DM2_VP_CHROME_TOP -
                                       DM2_VP_CHROME_BOT };
        out_plan->portrait_panel_rect =
            (DM2_V1_ViewportRect){ panel_x + 2, DM2_VP_CHROME_TOP,
                                   DM2_VP_WIDTH - (panel_x + 2),
                                   DM2_VP_HEIGHT - DM2_VP_CHROME_TOP -
                                       DM2_VP_CHROME_BOT };
        out_plan->portrait_panel_gdat_index =
            dm2_v1_viewport_hud_core_graphic_index(
                DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL);
        out_plan->champion_slot_count = DM2_V1_HUD_CHAMPION_SLOT_COUNT;
        for (int slot = 0; slot < out_plan->champion_slot_count; ++slot) {
            int py = DM2_VP_CHROME_TOP + 2 + slot * 36;
            DM2_V1_HudChampionSlotRender *champ =
                &out_plan->champion_slots[slot];
            champ->frame_rect =
                (DM2_V1_ViewportRect){ panel_x + 4, py,
                                       DM2_VP_WIDTH - 8 - (panel_x + 4),
                                       28 };
            champ->fill_rect =
                (DM2_V1_ViewportRect){ panel_x + 6, py + 2,
                                       DM2_VP_WIDTH - 6 - (panel_x + 6),
                                       22 };
            champ->fill_color = (uint8_t)(8 + slot * 2);
        }
    }
    return 1;
}

static uint8_t dm2_v1_hud_clamp_pct(int pct)
{
    if (pct < 0) {
        return 0;
    }
    if (pct > 100) {
        return 100;
    }
    return (uint8_t)pct;
}

static int dm2_v1_hud_name_marker_width(const char *name)
{
    int len = 0;
    if (!name) {
        return 0;
    }
    while (len < DM2_V1_HUD_CHAMPION_NAME_MAX && name[len]) {
        ++len;
    }
    return len * 3;
}

static DM2_V1_ViewportRect dm2_v1_hud_bar_fill(
    const DM2_V1_ViewportRect *bar,
    uint8_t pct)
{
    DM2_V1_ViewportRect fill = { 0, 0, 0, 0 };
    if (!bar || bar->w <= 0 || bar->h <= 0) {
        return fill;
    }
    fill = *bar;
    fill.w = (bar->w * (int)dm2_v1_hud_clamp_pct((int)pct)) / 100;
    return fill;
}

int dm2_v1_viewport_build_hud_chrome_plan_for_party(
    int is_outdoor,
    const DM2_V1_HudPartyState *party,
    DM2_V1_HudChromeRenderPlan *out_plan)
{
    if (!dm2_v1_viewport_build_hud_chrome_plan(is_outdoor, out_plan)) {
        return 0;
    }
    if (!party || out_plan->outdoor) {
        return 1;
    }
    for (int slot = 0; slot < out_plan->champion_slot_count; ++slot) {
        DM2_V1_HudChampionSlotRender *dst =
            &out_plan->champion_slots[slot];
        const DM2_V1_HudChampionState *src = NULL;
        int py = dst->frame_rect.y;
        int marker_w;

        if (slot < party->champion_count &&
            slot < DM2_V1_HUD_CHAMPION_SLOT_COUNT) {
            src = &party->champions[slot];
        }
        if (!src || !src->occupied) {
            continue;
        }

        dst->occupied = 1;
        dst->leader = src->leader || slot == party->leader_index;
        dst->hp_pct = dm2_v1_hud_clamp_pct((int)src->hp_pct);
        dst->stamina_pct = dm2_v1_hud_clamp_pct((int)src->stamina_pct);
        dst->mana_pct = dm2_v1_hud_clamp_pct((int)src->mana_pct);
        dst->portrait_index =
            (uint8_t)(src->portrait_index % DM2_V1_HUD_PORTRAIT_COUNT);
        dst->portrait_fill_color =
            (uint8_t)(8u + (dst->portrait_index & 7u));
        dst->fill_color = dst->leader ? 9u : 8u;
        dst->leader_mark_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 2, py + 3, 3, 3 };
        dst->portrait_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 4, py + 4, 18, 18 };
        marker_w = dm2_v1_hud_name_marker_width(src->name);
        dst->name_marker_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 26, py + 4,
                                   marker_w, marker_w > 0 ? 3 : 0 };
        dst->hp_bar_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 26, py + 9, 34, 3 };
        dst->stamina_bar_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 26, py + 14, 34, 3 };
        dst->mana_bar_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 26, py + 19, 34, 3 };
        dst->hp_fill_rect = dm2_v1_hud_bar_fill(&dst->hp_bar_rect,
                                                dst->hp_pct);
        dst->stamina_fill_rect = dm2_v1_hud_bar_fill(&dst->stamina_bar_rect,
                                                     dst->stamina_pct);
        dst->mana_fill_rect = dm2_v1_hud_bar_fill(&dst->mana_bar_rect,
                                                  dst->mana_pct);
    }
    return 1;
}

/* ── Transparency color (ReDMCSB DEFS.H C10_COLOR_FLESH = 10)
 * Used as skip color in wall blits. ── */
#define DM2_COLOR_TRANSPARENT  10

/* ── Viewport geometry ────────────────────────────────────────────── */
#define DM2_BLACK_AREA_TOP    0
#define DM2_BLACK_AREA_H     37
#define DM2_CEILING_Y         0
#define DM2_CEILING_H        29
#define DM2_FLOOR_Y          66
#define DM2_FLOOR_H          70
#define DM2_WALL_ZONE_D3_Y   25
#define DM2_WALL_ZONE_D2_Y   20
#define DM2_WALL_ZONE_D1_Y    9
#define DM2_WALL_ZONE_D0_Y    0

/* ── Wall frame table (12 entries, D3C..D0R) ─────────────────────────
 * Derived from ReDMCSB DUNVIEW.C G0163_aauc_Graphic558_Frame_Walls[12][8]
 * (lines 575-586), same as DM1. DM2 uses the same geometry constants.
 *
 * Index mapping (DUNVIEW.C:581-594):
 *   D3C=0, D3L=1, D3R=2, D2C=3, D2L=4, D2R=5,
 *   D1C=6, D1L=7, D1R=8, D0C=9, D0L=10, D0R=11
 *
 * Frame format: { X1, X2, Y1, Y2, ByteWidth, Height, X, Y }
 * Source: DUNVIEW.C:581-594 (G0163)
 * ─────────────────────────────────────────────────────────────────── */

const DM2_WallFrame g_dm2_wall_frames[DM2_SQ_COUNT] = {
    /* D3C */ {  74, 149, 25,  75,  64,  51,  18, 0 },
    /* D3L */ {   0,  83, 25,  75,  64,  51,  32, 0 },
    /* D3R */ { 139, 223, 25,  75,  64,  51,   0, 0 },
    /* D2C */ {  60, 163, 20,  90,  72,  71,  16, 0 },
    /* D2L */ {   0,  74, 20,  90,  72,  71,  61, 0 },
    /* D2R */ { 149, 223, 20,  90,  72,  71,   0, 0 },
    /* D1C */ {  32, 191,  9, 119, 128, 111,  48, 0 },
    /* D1L */ {   0,  63,  9, 119, 128, 111, 192, 0 },
    /* D1R */ { 160, 223,  9, 119, 128, 111,   0, 0 },
    /* D0C */ {   0, 223,  0, 135,   0,   0,   0, 0 },
    /* D0L */ {   0,  31,  0, 135,  16, 136,   0, 0 },
    /* D0R */ { 192, 223,  0, 135,  16, 136,   0, 0 },
};

/* DM2 wall set index table — negative = derived offset from wall set base.
 * Source: DUNVIEW.C:140-144, G3011-G3015 (I34E section).
 * DM2 uses different set indices than DM1 (G3060 variant, lines 170-175). */
static const int16_t __attribute__((unused)) s_dm2_wall_set [12] = {
    /* D3C */ -7,   /* G3060_i_WallSet_Wall_D3C */
    /* D3L */ -8,   /* G3061_i_WallSet_Wall_D3L */
    /* D3R */ -9,   /* G3062_i_WallSet_Wall_D3R */
    /* D2C */ -10,  /* G3063_i_WallSet_Wall_D2C */
    /* D2L */ -11,  /* G3064_i_WallSet_Wall_D2L */
    /* D2R */ -12,  /* G3065_i_WallSet_Wall_D2R */
    /* D1C */ -13,  /* G3066_i_WallSet_Wall_D1C */
    /* D1L */ -14,  /* G3067_i_WallSet_Wall_D1L (DM2-specific) */
    /* D1R */ -15,  /* G3068_i_WallSet_Wall_D1R (DM2-specific) */
    /* D0C */   0,
    /* D0L */ -16,  /* G3014_i_WallSet_Wall_D0L */
    /* D0R */ -17,  /* G3015_i_WallSet_Wall_D0R */
};

/* DM2 flipped wall set — horizontally mirrored L↔R per depth group.
 * Source: DUNVIEW.C:159-168, G3049-G3059 (WallSetFlipped). */
static const int16_t __attribute__((unused)) s_dm2_wall_set_flipped [12] = {
    /* D3C */ -18,  /* G3049_i_WallSetFlipped_Wall_D3C */
    /* D3L */ -19,  /* G3050_i_WallSetFlipped_Wall_D3L */
    /* D3R */ -20,  /* G3051_i_WallSetFlipped_Wall_D3R */
    /* D2C */ -21,  /* G3052_i_WallSetFlipped_Wall_D2C */
    /* D2L */ -22,  /* G3053_i_WallSetFlipped_Wall_D2L */
    /* D2R */ -23,  /* G3054_i_WallSetFlipped_Wall_D2R */
    /* D1C */ -24,  /* G3055_i_WallSetFlipped_Wall_D1C */
    /* D1L */ -25,  /* G3056_i_WallSetFlipped_Wall_D1L */
    /* D1R */ -26,  /* G3057_i_WallSetFlipped_Wall_D1R */
    /* D0C */   0,
    /* D0L */ -27,  /* G3058_i_WallSetFlipped_Wall_D0L */
    /* D0R */ -28,  /* G3059_i_WallSetFlipped_Wall_D0R */
};

/* DM2 door frame indices.
 * Source: DUNVIEW.C:148-157, G2116-G2119, G2196.
 * Different from DM1: DM2 door frames are larger/more ornate. */
static const int16_t __attribute__((unused)) s_dm2_door_frames [6] = {
    /* Top row (D1R,D1L,D1LCR,D2R,D2L,D2LCR) */
    /* DM2 door frame indices differ from DM1 (G2116=front D0C, etc.) */
    -35,  /* G2116_DoorFrameFrontD0C (DM2: larger door frames) */
    -33,  /* G2196_DoorFrameRightD1C */
    -34,  /* G2117_DoorFrameLeftD1C */
    -32,  /* G2118_DoorFrameLeftD2C */
    -30,  /* G2119_DoorFrameLeftD3C */
    -31,  /* G21xx_DoorFrameRightD2C (DM2 extension) */
};

/* ── Internal state ───────────────────────────────────────────────── */

/* Cached wall/floor/ceiling graphic index pairs (DM2 uses -1/-2 like DM1).
 * Source: DUNVIEW.C:126-127, G2108_Floor=-1, G2109_Ceiling=-2 */
#define DM2_GRAPHIC_FLOOR   DM2_V1_VIEWPORT_GFX_FLOOR
#define DM2_GRAPHIC_CEILING DM2_V1_VIEWPORT_GFX_CEILING

/* DM2 draw order — back-to-front, same 12 view squares as DM1.
 * Depth 3 (D3) → Depth 2 (D2) → Depth 1 (D1) → Depth 0 (D0).
 * Source: DUNGEON.C:1371-1421; DUNVIEW.C:8466-8542
 * ReDMCSB reference: s_draw_order[] in dm1_v1_viewport_3d_pc34_compat.c */
typedef enum {
    DM2_STEP_D3L = 0,
    DM2_STEP_D3R,
    DM2_STEP_D3C,
    DM2_STEP_D2L,
    DM2_STEP_D2R,
    DM2_STEP_D2C,
    DM2_STEP_D1L,
    DM2_STEP_D1R,
    DM2_STEP_D1C,
    DM2_STEP_D0L,
    DM2_STEP_D0R,
    DM2_STEP_D0C,
    DM2_STEP_COUNT
} DM2_RenderStep;

static const int __attribute__((unused)) s_step_to_square [DM2_STEP_COUNT] = {
    DM2_SQ_D3L, DM2_SQ_D3R, DM2_SQ_D3C,
    DM2_SQ_D2L, DM2_SQ_D2R, DM2_SQ_D2C,
    DM2_SQ_D1L, DM2_SQ_D1R, DM2_SQ_D1C,
    DM2_SQ_D0L, DM2_SQ_D0R, DM2_SQ_D0C,
};

static uint8_t dm2_v1_wall_fallback_color_for_step(int render_step)
{
    return (uint8_t)(2 + (render_step / 3) * 2);
}

/* ── Helper: resolve blit clipping gate ─────────────────────────── */

/* Clip gate for blit operations — prevents out-of-bounds writes.
 * Source: dm1_v1_viewport_3d_pc34_compat.c resolve_wall_blit_clip_gate */
typedef struct {
    int visible;
    int src_x, src_y;
    int dst_x, dst_y;
    int width, height;
} DM2_BlitClipGate;

static DM2_BlitClipGate dm2_resolve_blit_clip(
    const DM2_WallFrame *frame,
    int bitmap_w, int bitmap_h,
    int vp_w, int vp_h)
{
    DM2_BlitClipGate gate = {0};
    if (!frame || frame->byte_width == 0 || frame->height == 0) return gate;

    /* Frame source rect */
    int src_x = frame->blit_x;
    int src_y = frame->blit_y;
    int bw = frame->byte_width;
    int bh = frame->height;
    (void)bitmap_w; (void)bitmap_h; /* reserved for future full bitmap clip */

    /* Frame dest rect */
    int dst_x = frame->left_x;
    int dst_y = frame->top_y;
    int fw = frame->right_x - frame->left_x + 1;
    int fh = frame->bottom_y - frame->top_y + 1;
    (void)fw; (void)fh;

    /* Clip against viewport bounds */
    int clip_left   = (dst_x < 0) ? -dst_x : 0;
    int clip_top    = (dst_y < 0) ? -dst_y : 0;
    int clip_right  = (dst_x + bw > vp_w) ? (vp_w - (dst_x + bw)) : 0;
    int clip_bottom = (dst_y + bh > vp_h) ? (vp_h - (dst_y + bh)) : 0;

    if (clip_left >= bw || clip_top >= bh || clip_right >= bw || clip_bottom >= bh)
        return gate;

    gate.visible = 1;
    gate.src_x = src_x + clip_left;
    gate.src_y = src_y + clip_top;
    gate.dst_x = dst_x + clip_left;
    gate.dst_y = dst_y + clip_top;
    gate.width  = bw - clip_left + clip_right;
    gate.height = bh - clip_top  + clip_bottom;
    return gate;
}

/* ── Palette color constants (ReDMCSB DEFS.H color indices) ───────── */
enum DM2_ColorIndex {
    DM2_COL_BLACK   = 0,
    DM2_COL_DKGRAY  = 1,
    DM2_COL_MIDGRAY = 7,
    DM2_COL_LTGRAY  = 8,
    DM2_COL_FLESH   = 10,  /* C10_COLOR_FLESH = transparency */
    DM2_COL_WHITE   = 15,
    /* DM2 outdoor sky gradient */
    DM2_COL_SKY_DEEP = 9,
    DM2_COL_SKY_CYAN = 3,
    DM2_COL_GROUND   = 6,
};

static void dm2_v1_fill_rect(uint8_t *fb,
                             int stride,
                             const DM2_V1_ViewportRect *rect,
                             uint8_t color)
{
    int x0;
    int y0;
    int x1;
    int y1;

    if (!fb || !rect || stride <= 0 || rect->w <= 0 || rect->h <= 0) {
        return;
    }
    x0 = rect->x < 0 ? 0 : rect->x;
    y0 = rect->y < 0 ? 0 : rect->y;
    x1 = rect->x + rect->w;
    y1 = rect->y + rect->h;
    if (x1 > DM2_VP_WIDTH) x1 = DM2_VP_WIDTH;
    if (y1 > DM2_VP_HEIGHT) y1 = DM2_VP_HEIGHT;
    if (x0 >= x1 || y0 >= y1) {
        return;
    }
    for (int y = y0; y < y1; ++y) {
        memset(fb + y * stride + x0, color, (size_t)(x1 - x0));
    }
}

static void dm2_v1_stroke_rect(uint8_t *fb,
                               int stride,
                               const DM2_V1_ViewportRect *rect,
                               uint8_t color)
{
    DM2_V1_ViewportRect line;

    if (!fb || !rect || stride <= 0 || rect->w <= 0 || rect->h <= 0) {
        return;
    }
    line = (DM2_V1_ViewportRect){ rect->x, rect->y, rect->w, 1 };
    dm2_v1_fill_rect(fb, stride, &line, color);
    line.y = rect->y + rect->h - 1;
    dm2_v1_fill_rect(fb, stride, &line, color);
    line = (DM2_V1_ViewportRect){ rect->x, rect->y, 1, rect->h };
    dm2_v1_fill_rect(fb, stride, &line, color);
    line.x = rect->x + rect->w - 1;
    dm2_v1_fill_rect(fb, stride, &line, color);
}

static void dm2_v1_fill_coin_disc(uint8_t *fb,
                                  int stride,
                                  const DM2_V1_ViewportRect *rect,
                                  uint8_t color)
{
    int cx;
    int cy;
    int radius_sq;

    if (!fb || !rect || stride <= 0 || rect->w <= 0 || rect->h <= 0) {
        return;
    }
    cx = rect->x + rect->w / 2;
    cy = rect->y + rect->h / 2;
    radius_sq = (rect->w < rect->h ? rect->w : rect->h);
    radius_sq = (radius_sq * radius_sq) / 4;
    for (int y = rect->y; y < rect->y + rect->h; ++y) {
        if (y < 0 || y >= DM2_VP_HEIGHT) continue;
        for (int x = rect->x; x < rect->x + rect->w; ++x) {
            int dx;
            int dy;
            if (x < 0 || x >= DM2_VP_WIDTH) continue;
            dx = x - cx;
            dy = y - cy;
            if (dx * dx + dy * dy < radius_sq) {
                fb[y * stride + x] = color;
            }
        }
    }
}

/* ── Initialization ───────────────────────────────────────────────── */

void dm2_v1_viewport_init(DM2_V1_ViewportState *s,
                          uint8_t *framebuffer,
                          int      fb_stride)
{
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->framebuffer = framebuffer;
    s->fb_stride   = fb_stride > 0 ? fb_stride : DM2_VP_WIDTH;
    s->party_dir    = 0;
    s->party_x      = 15;
    s->party_y      = 15;
    s->dungeon_level = 0;
    s->is_outdoor   = 0;
    s->weather      = 0;
    s->rain_intensity = 0;
    s->time_of_day  = 0.5f;
    s->dirty        = 1;
    s->random_seed  = 0x0100u;
    s->last_hud_core_gdat_hash = 2166136261u;

    /* Initialize all view squares to empty */
    for (int i = 0; i < DM2_SQ_COUNT; i++) {
        s->squares[i].square_type   = DM2_SQUARE_FLOOR;
        s->squares[i].flags        = DM2_SQF_NONE;
        s->squares[i].light_level  = 15;  /* full light */
        s->squares[i].door_open_pct = 0;
        s->squares[i].sprite_depth = i;   /* depth = square index */
    }

    /* Initialize sprite pools */
    s->creature_count  = 0;
    s->item_count      = 0;
    s->carried_item_present = 0;
    s->projectile_count = 0;

    /* wall_set arrays are static in this .c file, not in viewport state */
    (void)0; /* placeholder */
}

void dm2_v1_viewport_set_party(DM2_V1_ViewportState *s,
                                int dir, int x, int y)
{
    if (!s) return;
    s->party_dir = (dir & 3);
    s->party_x   = x;
    s->party_y   = y;
    s->dirty     = 1;
}

void dm2_v1_viewport_set_outdoor(DM2_V1_ViewportState *s, int is_outdoor)
{
    if (!s) return;
    if (s->is_outdoor != (is_outdoor ? 1 : 0)) {
        s->is_outdoor = is_outdoor ? 1 : 0;
        s->dirty = 1;
    }
}

void dm2_v1_viewport_set_level(DM2_V1_ViewportState *s, int level)
{
    if (!s) return;
    s->dungeon_level = level;
    s->dirty = 1;
}

void dm2_v1_viewport_set_weather(DM2_V1_ViewportState *s,
                                   int weather,
                                   int rain_intensity)
{
    if (!s) return;
    s->weather = weather;
    s->rain_intensity = rain_intensity;
    s->dirty = 1;
}

void dm2_v1_viewport_set_time(DM2_V1_ViewportState *s, float time_of_day)
{
    if (!s) return;
    s->time_of_day = (time_of_day < 0) ? 0 : (time_of_day > 1 ? 1 : time_of_day);
    s->dirty = 1;
}

void dm2_v1_viewport_set_hud_party(DM2_V1_ViewportState *s,
                                   const DM2_V1_HudPartyState *party)
{
    if (!s) {
        return;
    }
    memset(&s->hud_party, 0, sizeof(s->hud_party));
    s->hud_party_valid = 0;
    if (party) {
        s->hud_party = *party;
        if (s->hud_party.champion_count < 0) {
            s->hud_party.champion_count = 0;
        }
        if (s->hud_party.champion_count > DM2_V1_HUD_CHAMPION_SLOT_COUNT) {
            s->hud_party.champion_count = DM2_V1_HUD_CHAMPION_SLOT_COUNT;
        }
        if (s->hud_party.leader_index < 0 ||
            s->hud_party.leader_index >= s->hud_party.champion_count) {
            s->hud_party.leader_index = 0;
        }
        s->hud_party_valid = 1;
    }
    s->dirty = 1;
}

void dm2_v1_viewport_set_asset_provider(DM2_V1_ViewportState *s,
                                        DM2_V1_ViewportAssetFetch fetch,
                                        void *user)
{
    if (!s) return;
    s->asset_fetch = fetch;
    s->asset_user = user;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_scene_control(
    DM2_V1_ViewportState *s,
    int ready,
    uint32_t hash,
    uint16_t scene_colorkey,
    uint16_t scene_flags,
    uint16_t ambient_light,
    uint16_t highest_light_level,
    uint16_t void_random_fall,
    uint16_t animated_floor)
{
    if (!s) return;
    s->gdat_scene_control_ready = ready ? 1 : 0;
    s->gdat_scene_control_hash = ready ? hash : 0u;
    s->gdat_scene_colorkey = ready ? scene_colorkey : 0u;
    s->gdat_scene_flags = ready ? scene_flags : 0u;
    s->gdat_ambient_light = ready ? ambient_light : 0u;
    s->gdat_highest_light_level = ready ? highest_light_level : 0u;
    s->gdat_void_random_fall = ready ? void_random_fall : 0u;
    s->gdat_animated_floor = ready ? animated_floor : 0u;
    s->dirty = 1;
}

static uint32_t dm2_v1_viewport_scene_hash_step(uint32_t hash,
                                                uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

int dm2_v1_viewport_scene_consumption_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportSceneConsumptionReceipt *out_receipt)
{
    uint32_t mask = 0u;
    uint32_t hash = 0x32565343u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!s) return 0;
    if (s->gdat_scene_control_consumed_count > 0) mask |= 1u << 0;
    if (s->gdat_scene_light_consumed_count > 0) mask |= 1u << 1;
    if (s->gdat_scene_floor_anim_consumed_count > 0) mask |= 1u << 2;
    if (s->gdat_scene_weather_consumed_count > 0) mask |= 1u << 3;
    hash = dm2_v1_viewport_scene_hash_step(hash, mask);
    hash = dm2_v1_viewport_scene_hash_step(hash, s->gdat_scene_control_hash);
    hash = dm2_v1_viewport_scene_hash_step(hash, s->gdat_scene_colorkey);
    hash = dm2_v1_viewport_scene_hash_step(hash, s->gdat_scene_flags);
    hash = dm2_v1_viewport_scene_hash_step(hash, s->gdat_ambient_light);
    hash = dm2_v1_viewport_scene_hash_step(hash, s->gdat_highest_light_level);
    hash = dm2_v1_viewport_scene_hash_step(hash, s->gdat_void_random_fall);
    hash = dm2_v1_viewport_scene_hash_step(hash, s->gdat_animated_floor);
    out_receipt->ready = s->gdat_scene_control_ready && mask != 0u;
    out_receipt->consumed_mask = mask;
    out_receipt->consumption_hash = hash;
    out_receipt->source_hash = s->gdat_scene_control_hash;
    out_receipt->scene_colorkey = s->gdat_scene_colorkey;
    out_receipt->scene_flags = s->gdat_scene_flags;
    out_receipt->ambient_light = s->gdat_ambient_light;
    out_receipt->highest_light_level = s->gdat_highest_light_level;
    out_receipt->void_random_fall = s->gdat_void_random_fall;
    out_receipt->animated_floor = s->gdat_animated_floor;
    out_receipt->scene_control_consumed =
        s->gdat_scene_control_consumed_count;
    out_receipt->light_consumed = s->gdat_scene_light_consumed_count;
    out_receipt->floor_anim_consumed =
        s->gdat_scene_floor_anim_consumed_count;
    out_receipt->weather_consumed = s->gdat_scene_weather_consumed_count;
    return 1;
}

void dm2_v1_viewport_set_interface_theme(
    DM2_V1_ViewportState *s,
    const DM2_V1_InterfaceTheme *theme)
{
    if (!s) return;
    memset(&s->interface_theme, 0, sizeof(s->interface_theme));
    s->interface_theme_valid = 0;
    if (!theme || !theme->valid || theme->semantic_hash == 0u) {
        return;
    }
    s->interface_theme = *theme;
    s->interface_theme_valid = 1;
    s->dirty = 1;
}

/* ── Wall frame lookup ────────────────────────────────────────────── */

const DM2_WallFrame *dm2_v1_get_wall_frame(int view_square)
{
    if (view_square < 0 || view_square >= DM2_SQ_COUNT) return NULL;
    return &g_dm2_wall_frames[view_square];
}

int dm2_v1_viewport_wall_field_for_square(int view_square)
{
    if (view_square < 0 || view_square >= DM2_SQ_COUNT) return -1;
    if (g_dm2_wall_frames[view_square].byte_width == 0 ||
        g_dm2_wall_frames[view_square].height == 0) {
        return -1;
    }
    if (view_square == DM2_SQ_D3C) return -1;
    /* skproject SKWIN/SkWinCore.cpp DRAW_WALL/QUERY_TEMP_PICST
     * lines ~47373-47474 maps normal wall cells through
     * `iViewportCell + 0x22`.  The PC English startup graphics set has
     * real fields from 0x23 upward for this first visible-cell pass; D3C's
     * enum-local 0x22 slot is absent and remains on the fallback backdrop. */
    return DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST + view_square;
}

int dm2_v1_viewport_wall_graphic_index_for_square(int view_square)
{
    int field = dm2_v1_viewport_wall_field_for_square(view_square);
    if (field < 0) return 0;
    return DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE - field;
}

int dm2_v1_viewport_build_wall_panel_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_WallPanelRenderPlan *out_plan)
{
    (void)s;
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    /* skproject SKWIN/SkWinCore.cpp DRAW_WALL/QUERY_TEMP_PICST routes each
     * visible wall through the viewport-cell field before blitting.  Keep the
     * Firestaff contract as explicit panel rows so the asset-backed renderer,
     * local fallback, and future exact source clipping share one plan. */
    for (int step = 0; step < DM2_STEP_COUNT; ++step) {
        int square = s_step_to_square[step];
        const DM2_WallFrame *frame = dm2_v1_get_wall_frame(square);
        int gdat_index = dm2_v1_viewport_wall_graphic_index_for_square(square);
        DM2_V1_WallPanelRender *row;

        if (!frame || frame->byte_width == 0 || frame->height == 0 ||
            gdat_index == 0 ||
            out_plan->panel_count >= DM2_V1_WALL_PANEL_RENDER_MAX) {
            continue;
        }
        row = &out_plan->panels[out_plan->panel_count++];
        row->render_step = step;
        row->view_square = square;
        row->skproject_cell = dm2_v1_viewport_skproject_cell_for_square(square);
        row->gdat_index = gdat_index;
        row->src_rect = (DM2_V1_ViewportRect){
            frame->blit_x,
            frame->blit_y,
            frame->byte_width,
            frame->height
        };
        row->dst_rect = (DM2_V1_ViewportRect){
            frame->left_x,
            frame->top_y,
            frame->right_x - frame->left_x + 1,
            frame->bottom_y - frame->top_y + 1
        };
        row->fallback_color = dm2_v1_wall_fallback_color_for_step(step);
    }
    return 1;
}

int dm2_v1_viewport_door_frame_field_for_square(int view_square)
{
    switch (view_square) {
    case DM2_SQ_D0C:
        return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT;
    case DM2_SQ_D1C:
        return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_D1C;
    case DM2_SQ_D2C:
        return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_D2C;
    default:
        return -1;
    }
}

int dm2_v1_viewport_door_frame_graphic_index_for_square(int view_square)
{
    int field = dm2_v1_viewport_door_frame_field_for_square(view_square);
    if (field < 0) return 0;
    return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE - field;
}

int dm2_v1_viewport_door_panel_field_for_square(int view_square)
{
    switch (view_square) {
    case DM2_SQ_D0C:
        return DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT;
    case DM2_SQ_D1C:
        return DM2_V1_VIEWPORT_GFX_DOOR_PANEL_D1C;
    case DM2_SQ_D2C:
        return DM2_V1_VIEWPORT_GFX_DOOR_PANEL_D2C;
    default:
        return -1;
    }
}

int dm2_v1_viewport_door_panel_graphic_index_for_square(int view_square)
{
    int field = dm2_v1_viewport_door_panel_field_for_square(view_square);
    if (field < 0) return 0;
    return DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - field;
}

int dm2_v1_viewport_door_panel_graphic_index_for_record(int view_square,
                                                        int door_gfx_index,
                                                        int opening_dir)
{
    int field = dm2_v1_viewport_door_panel_field_for_square(view_square);
    int record_field;
    if (field < 0) return 0;
    if (door_gfx_index < 0) door_gfx_index = 0;
    if (door_gfx_index > 0xff) door_gfx_index = 0xff;
    /* skproject SKWIN/SkWinCore.cpp lines 46405-46431 fetch the panel from
     * GDAT_CATEGORY_DOORS using glbMapDoorType[DoorType()]. Lines 46580-46606
     * use OpeningDir() to select the split/position path for moving panels. */
    record_field = field & DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_MASK;
    record_field |= (opening_dir & 1) <<
        DM2_V1_VIEWPORT_GFX_DOOR_PANEL_OPENING_SHIFT;
    record_field |= (door_gfx_index & 0xff) <<
        DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT;
    return DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE - record_field;
}

int dm2_v1_viewport_door_ornate_graphic_index(int door_ornate_index,
                                              int view_square)
{
    int field = dm2_v1_viewport_door_panel_field_for_square(view_square);
    int packed;
    if (field < 0 || door_ornate_index <= 0) return 0;
    if (door_ornate_index > 0xff) door_ornate_index = 0xff;
    /* skproject SKWIN/SkWinCore.cpp lines 46477-46510 draws Door::OrnateIndex()
     * through GDAT_CATEGORY_DOOR_GFX after the base door panel. */
    packed = ((door_ornate_index & 0xff) <<
              DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT) |
             (field & DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE - packed;
}

int dm2_v1_viewport_door_destroyed_mask_graphic_index(int door_gfx_index,
                                                      int view_square)
{
    int field = dm2_v1_viewport_door_panel_field_for_square(view_square);
    int packed;
    if (field < 0) return 0;
    if (door_gfx_index < 0) door_gfx_index = 0;
    if (door_gfx_index > 0xff) door_gfx_index = 0xff;
    /* skproject SKWIN/SkWinCore.cpp lines 46513-46535 overlays the destroyed
     * mask from GDAT_CATEGORY_DOORS when tile door state is 5. */
    packed = ((door_gfx_index & 0xff) <<
              DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT) |
             (field & DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE - packed;
}

int dm2_v1_viewport_door_button_field_for_state(int pushed)
{
    /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR_FRAMES line ~46342 calls
     * DRAW_DEFAULT_DOOR_BUTTON(GDAT_CATEGORY_DOOR_BUTTONS, 0,
     * door->ButtonState() * 5, iViewportCell). */
    return pushed ? DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_PUSHED
                  : DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_RELEASED;
}

int dm2_v1_viewport_door_button_graphic_index_for_state(int pushed)
{
    int field = dm2_v1_viewport_door_button_field_for_state(pushed);
    return DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE - field;
}

static const int8_t s_dm2_square_to_skproject_cell[DM2_SQ_COUNT] = {
    /* Firestaff D3/D2/D1/D0 center rows do not have the same ordinal as
     * skproject's tblCellTilesRoom viewport cells.  skproject SKWINSPX
     * kskval1.h line 62 defines tlbRectnoDoorButton for cells 0,3,6,11,13;
     * SkWinCore.cpp DRAW_DOOR_TILE lines ~46650-46700 dispatches center-door
     * cells 0,3,6 through DRAW_DOOR for the D0/D1/D2 startup path. */
    /* D3C */ -1, /* D3L */ -1, /* D3R */ -1,
    /* D2C */  6, /* D2L */ -1, /* D2R */ -1,
    /* D1C */  3, /* D1L */ -1, /* D1R */ -1,
    /* D0C */  0, /* D0L */ -1, /* D0R */ -1,
};

static const int8_t s_dm2_skproject_rectno_door_button[14] = {
    /* skproject SKWINSPX/src/v4/kskval1.h line 62:
     * tlbRectnoDoorButton =
     *   {4,-1,-1,3,-1,-1,2,-1,-1,-1,-1,1,-1,0}. */
     4, -1, -1,  3, -1, -1,  2,
    -1, -1, -1, -1,  1, -1,  0,
};

int dm2_v1_viewport_skproject_cell_for_square(int view_square)
{
    if (view_square < 0 || view_square >= DM2_SQ_COUNT) return -1;
    return s_dm2_square_to_skproject_cell[view_square];
}

int dm2_v1_viewport_door_button_rectno_for_square(int view_square)
{
    int cell = dm2_v1_viewport_skproject_cell_for_square(view_square);
    if (cell < 0 || cell >= (int)(sizeof s_dm2_skproject_rectno_door_button /
                                  sizeof s_dm2_skproject_rectno_door_button[0])) {
        return -1;
    }
    return s_dm2_skproject_rectno_door_button[cell];
}

int dm2_v1_viewport_door_button_clickable_for_square(int view_square)
{
    int rectno = dm2_v1_viewport_door_button_rectno_for_square(view_square);
    /* skproject SKWIN/SkWinCore.cpp DRAW_DEFAULT_DOOR_BUTTON lines
     * ~46261-46264 calls MAKE_BUTTON_CLICKABLE only for rectno 3 and 4. */
    return rectno == 3 || rectno == 4;
}

int dm2_v1_viewport_wall_button_graphic_index(int wall_gfx_index,
                                              int wall_gfx_field)
{
    int packed;
    if (wall_gfx_index < 0 || wall_gfx_index > 0xFF ||
        wall_gfx_field < 0 || wall_gfx_field > 0xFF) {
        return 0;
    }
    packed = (wall_gfx_index << DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT) |
             (wall_gfx_field & DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE - packed;
}

int dm2_v1_viewport_creature_graphic_index(int creature_type,
                                           int frame_index)
{
    int packed;
    if (creature_type < 0 || creature_type > 0xFF ||
        frame_index < 0 || frame_index > 0xFF) {
        return 0;
    }
    packed = (creature_type << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT) |
             (frame_index & DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE - packed;
}

int dm2_v1_viewport_item_graphic_index(int item_category,
                                       int item_type,
                                       int frame_index)
{
    int packed;
    if (item_category < 0 || item_category > 0xFF ||
        item_type < 0 || item_type > 0xFF ||
        frame_index < 0 || frame_index > 0xFF) {
        return 0;
    }
    packed = (item_category << DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) |
             (item_type << DM2_V1_VIEWPORT_GFX_ITEM_INDEX_SHIFT) |
             (frame_index & DM2_V1_VIEWPORT_GFX_ITEM_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - packed;
}

int dm2_v1_viewport_item_category_for_db_pool(int db_pool)
{
    switch (db_pool) {
    case 5:  return 0x10; /* WEAPON */
    case 6:  return 0x11; /* CLOTH */
    case 7:  return 0x12; /* SCROLL */
    case 10: return 0x15; /* MISC */
    default: return 0x15;
    }
}

int dm2_v1_viewport_projectile_graphic_index(int projectile_category,
                                             int projectile_type,
                                             int frame_index)
{
    int packed;
    if (projectile_category < 0 || projectile_category > 0xFF ||
        projectile_type < 0 || projectile_type > 0xFF ||
        frame_index < 0 || frame_index > 0xFF) {
        return 0;
    }
    packed = (projectile_category << DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) |
             (projectile_type << DM2_V1_VIEWPORT_GFX_PROJECTILE_INDEX_SHIFT) |
             (frame_index & DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - packed;
}

int dm2_v1_viewport_hud_portrait_graphic_index(int portrait_index)
{
    int packed;
    if (portrait_index < 0 || portrait_index >= DM2_V1_HUD_PORTRAIT_COUNT) {
        return 0;
    }
    /* skproject SKWIN/SkWinCore.cpp T560 draws the right-side status
     * portraits through UI GDAT image queries.  Firestaff packs the
     * portrait ordinal into a renderer-private index so the boot profile can
     * resolve the current GRAPHICS.DAT charsheet image and preserve the
     * placeholder fill when the asset is absent. */
    packed = (portrait_index << DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT) |
             DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD;
    return DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE - packed;
}

int dm2_v1_viewport_hud_core_graphic_index(int field)
{
    if (field < 0 || field > DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK) {
        return 0;
    }
    return DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE - field;
}

int dm2_v1_viewport_hud_action_icon_graphic_index(int icon_index)
{
    if (icon_index < 0 || icon_index >= DM2_V1_HUD_ACTION_ICON_COUNT) {
        return 0;
    }
    return dm2_v1_viewport_hud_core_graphic_index(
        DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_ICON_BASE + icon_index);
}

int dm2_v1_viewport_map_chip_frame_width(int src_w, int src_h)
{
    if (src_w <= 0 || src_h <= 0) return 0;
    /* skproject SKWIN/SkWinCore.cpp DRAW_CHIP_OF_MAGIC_MAP lines
     * 1001-1037 selects source X as glbMagicMapWidth * frame. DM2's
     * startup constants set glbMagicMapWidth/glbMagicMapHeight to 7, and
     * QUERY_DUNGEON_MAP_CHIP_PICT returns atlas_width / glbMagicMapWidth.
     * Use square tiles for decoded atlases and keep single bitmap fixtures
     * unchanged. */
    if (src_w > src_h && (src_w % src_h) == 0) return src_h;
    return src_w;
}

int dm2_v1_viewport_map_chip_frame_count(int src_w, int src_h)
{
    int frame_w = dm2_v1_viewport_map_chip_frame_width(src_w, src_h);
    if (frame_w <= 0 || src_w <= 0) return 0;
    return src_w / frame_w;
}

int dm2_v1_viewport_map_chip_frame_index(int requested_frame,
                                         int frame_count)
{
    if (frame_count <= 0) return 0;
    if (requested_frame < 0) return 0;
    return requested_frame % frame_count;
}

int dm2_v1_viewport_projectile_frame_for_direction(int requested_frame,
                                                   int projectile_direction,
                                                   int party_direction,
                                                   int frame_count)
{
    int rel;
    if (frame_count <= 0) return 0;
    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 1525-1575 uses
     * directional missile frames when QUERY_DUNGEON_MAP_CHIP_PICT reports
     * more than three 7px frames. The source chooses a base missile frame
     * near 3 plus a view-relative direction term before DRAW_CHIP_OF_MAGIC_MAP
     * slices the atlas. Keep short atlases on their requested animation frame. */
    if (frame_count <= 3) {
        return dm2_v1_viewport_map_chip_frame_index(requested_frame,
                                                   frame_count);
    }
    rel = ((projectile_direction & 3) - (party_direction & 3)) & 3;
    return dm2_v1_viewport_map_chip_frame_index(3 + rel, frame_count);
}

int dm2_v1_viewport_projectile_frame_for_map_chip(int requested_frame,
                                                  int projectile_direction,
                                                  int object_direction,
                                                  int party_direction,
                                                  int frame_count,
                                                  int frame_class)
{
    static const int8_t s_skproject_missile_frame_adjust[16] = {
        0, 0, 2, 2,
        0, 2, 2, 0,
        0, 0, -2, -2,
        0, -2, -2, 0
    };
    int motion_rel;
    int object_rel;
    int frame;

    if (frame_count <= 0) return 0;
    if (frame_count <= 3) {
        return dm2_v1_viewport_map_chip_frame_index(requested_frame,
                                                   frame_count);
    }

    /* skproject SKWIN/SkWinCore.cpp `_48ae_011a` lines 10168-10198
     * classifies missile map-chip image coverage. DRAW_MAP_CHIP lines
     * 10691-10718 applies `_4976_3fa8` only for class 1; the other source
     * cases collapse to frame 0 or the base-front frame 3. */
    switch ((uint8_t)frame_class) {
    case DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL:
        motion_rel = ((projectile_direction & 3) - (party_direction & 3)) & 3;
        object_rel = ((object_direction & 3) - (party_direction & 3)) & 3;
        frame = 3 + motion_rel;
        frame += s_skproject_missile_frame_adjust[
            ((frame - 3) << 2) + object_rel];
        return dm2_v1_viewport_map_chip_frame_index(frame, frame_count);
    case DM2_V1_PROJECTILE_FRAME_CLASS_BASE_FRONT:
        return dm2_v1_viewport_map_chip_frame_index(3, frame_count);
    case DM2_V1_PROJECTILE_FRAME_CLASS_MISSING:
    case DM2_V1_PROJECTILE_FRAME_CLASS_FRONT_ONLY:
    case DM2_V1_PROJECTILE_FRAME_CLASS_FLAT:
    default:
        return 0;
    }
}

int dm2_v1_viewport_projectile_flip_for_direction(int projectile_direction,
                                                  int party_direction)
{
    /* skproject SKWIN/SkGlobal.cpp `_4976_3fa4 = {0,1,3,2}`; DRAW_MAP_CHIP
     * lines 10720-10725 passes it to DRAW_CHIP_OF_MAGIC_MAP for dbMissile. */
    return dm2_v1_viewport_map_chip_flip_for_object_direction(
        projectile_direction, party_direction);
}

int dm2_v1_viewport_map_chip_flip_for_object_direction(int object_direction,
                                                       int party_direction)
{
    static const uint8_t s_skproject_object_flip[4] = { 0, 1, 3, 2 };
    int rel = ((object_direction & 3) - (party_direction & 3)) & 3;
    /* skproject SKWIN/SkGlobal.cpp `_4976_3fa4 = {0,1,3,2}` is also used
     * for creature possession overlays in SkWinCore.cpp DRAW_MAP_CHIP
     * lines 10798-10815, where `(si.Dir() - viewDir) & 3` selects the
     * mirror passed to DRAW_CHIP_OF_MAGIC_MAP. */
    return s_skproject_object_flip[rel];
}

int dm2_v1_viewport_cloud_frame_for_tick(int tick_count,
                                         int frame_count)
{
    if (frame_count <= 0) return 0;
    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10672-10743:
     * dbCloud objects use `(glbGameTick & 1) + 1` before
     * DRAW_CHIP_OF_MAGIC_MAP instead of the missile directional frame path. */
    return dm2_v1_viewport_map_chip_frame_index((tick_count & 1) + 1,
                                                frame_count);
}

int dm2_v1_viewport_cloud_flip_for_seed(uint32_t *seed)
{
    uint32_t next_seed;

    if (!seed) return 0;
    next_seed = (*seed * 0xbb40e62du) + 11u;
    *seed = next_seed;
    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10743-10749
     * passes RAND02() to DRAW_CHIP_OF_MAGIC_MAP for dbCloud. RAND02
     * lines 33533-33541 advances glbRandomSeed with the same LCG and
     * returns `(glbRandomSeed >> 8) & 3`. */
    return (int)((next_seed >> 8) & 3u);
}

int dm2_v1_viewport_creature_frame_for_direction(int requested_frame,
                                                 int creature_direction,
                                                 int party_direction,
                                                 int frame_count)
{
    int rel;
    int base;

    if (frame_count <= 0) return 0;
    if (frame_count <= 3) {
        return dm2_v1_viewport_map_chip_frame_index(requested_frame,
                                                   frame_count);
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10588-10618:
     * creatures are fetched as one IMG_MAP_CHIP atlas and the drawn frame is
     * `(viewDir - creatureDir) & 1` added to an even animation base before
     * DRAW_CHIP_OF_MAGIC_MAP slices the 7px chip. */
    base = requested_frame & ~1;
    if (base + 1 >= frame_count) base = 0;
    rel = ((party_direction & 3) - (creature_direction & 3)) & 3;
    return dm2_v1_viewport_map_chip_frame_index(base + (rel & 1),
                                                frame_count);
}

static void dm2_v1_viewport_clear_rect(DM2_V1_ViewportRect *out_rect)
{
    if (!out_rect) return;
    out_rect->x = 0;
    out_rect->y = 0;
    out_rect->w = 0;
    out_rect->h = 0;
}

int dm2_v1_viewport_door_panel_rect_for_square(int view_square,
                                               DM2_V1_ViewportRect *out_rect)
{
    if (!out_rect) return 0;
    dm2_v1_viewport_clear_rect(out_rect);

    /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR routes D0C/D1C/D2C through
     * viewport-cell door graphics. These are the current bounded startup
     * rectangles; exact tlbRectnoDoorButton/panel-table replacement stays
     * isolated behind this API. */
    switch (view_square) {
    case DM2_SQ_D0C:
        out_rect->x = 80;
        out_rect->y = 0;
        out_rect->w = 160;
        out_rect->h = 135;
        return 1;
    case DM2_SQ_D1C:
        out_rect->x = 60;
        out_rect->y = 9;
        out_rect->w = 104;
        out_rect->h = 110;
        return 1;
    case DM2_SQ_D2C:
        out_rect->x = 60;
        out_rect->y = 20;
        out_rect->w = 103;
        out_rect->h = 71;
        return 1;
    default:
        return 0;
    }
}

int dm2_v1_viewport_door_button_rect_for_square(int view_square,
                                                DM2_V1_ViewportRect *out_rect)
{
    DM2_V1_ViewportRect panel;
    int rectno;

    if (!out_rect) return 0;
    dm2_v1_viewport_clear_rect(out_rect);
    if (!dm2_v1_viewport_door_panel_rect_for_square(view_square, &panel)) {
        return 0;
    }

    rectno = dm2_v1_viewport_door_button_rectno_for_square(view_square);
    switch (rectno) {
    case 4:
        out_rect->w = 16;
        out_rect->h = 18;
        out_rect->x = panel.x + panel.w - 28;
        out_rect->y = panel.y + (panel.h / 2) - 9;
        return 1;
    case 3:
        out_rect->w = 12;
        out_rect->h = 14;
        out_rect->x = panel.x + panel.w - 22;
        out_rect->y = panel.y + (panel.h / 2) - 7;
        return 1;
    case 2:
        out_rect->w = 8;
        out_rect->h = 9;
        out_rect->x = panel.x + panel.w - 16;
        out_rect->y = panel.y + (panel.h / 2) - 4;
        return 1;
    default:
        return 0;
    }
}

static DM2_V1_ViewportRect dm2_v1_viewport_wall_frame_rect(int view_square)
{
    const DM2_WallFrame *frame = dm2_v1_get_wall_frame(view_square);
    DM2_V1_ViewportRect rect = { 0, 0, 0, 0 };
    if (!frame) {
        return rect;
    }
    rect.x = frame->left_x;
    rect.y = frame->top_y;
    rect.w = frame->right_x - frame->left_x + 1;
    rect.h = frame->bottom_y - frame->top_y + 1;
    return rect;
}

static DM2_V1_ViewportRect dm2_v1_viewport_door_visible_panel_rect(
    const DM2_V1_ViewportRect *panel_rect,
    int door_open_pct)
{
    DM2_V1_ViewportRect rect = { 0, 0, 0, 0 };
    int visible_pct;
    int visible_h;

    if (!panel_rect || panel_rect->w <= 0 || panel_rect->h <= 0) {
        return rect;
    }
    if (door_open_pct < 0) {
        door_open_pct = 0;
    } else if (door_open_pct > 100) {
        door_open_pct = 100;
    }
    visible_pct = 100 - door_open_pct;
    if (visible_pct <= 0) {
        return rect;
    }
    visible_h = (panel_rect->h * visible_pct + 99) / 100;
    if (visible_h <= 0) {
        visible_h = 1;
    } else if (visible_h > panel_rect->h) {
        visible_h = panel_rect->h;
    }
    rect = *panel_rect;
    rect.y = panel_rect->y + (panel_rect->h - visible_h);
    rect.h = visible_h;
    return rect;
}

int dm2_v1_viewport_build_door_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_DoorRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR_TILE/DRAW_DOOR_FRAMES routes
     * each visible DB0 door through center viewport cells 0,3,6 before
     * drawing the panel, frame, and optional default/custom wall button. */
    for (int square = 0; square < DM2_SQ_COUNT; ++square) {
        const DM2_ViewSquare *vs = &s->squares[square];
        DM2_V1_DoorRender *row;
        DM2_V1_ViewportRect panel_rect;

        if (!(vs->flags & DM2_SQF_HAS_DOOR) ||
            out_plan->door_count >= DM2_V1_DOOR_RENDER_MAX) {
            continue;
        }
        if (!dm2_v1_viewport_door_panel_rect_for_square(square,
                                                        &panel_rect)) {
            continue;
        }
        row = &out_plan->doors[out_plan->door_count++];
        row->view_square = square;
        row->skproject_cell = dm2_v1_viewport_skproject_cell_for_square(square);
        row->door_record_type = vs->door_record_type;
        row->door_gfx_index = vs->door_gfx_index;
        row->door_opening_dir = vs->door_opening_dir;
        row->ornament_index = vs->ornament_index;
        row->door_button = vs->door_button;
        row->door_button_state = vs->door_button_state;
        if (vs->door_gfx_index != 0 ||
            vs->door_record_type != 0 ||
            vs->door_opening_dir != 0) {
            row->panel_gdat_index =
                dm2_v1_viewport_door_panel_graphic_index_for_record(
                    square,
                    vs->door_gfx_index,
                    vs->door_opening_dir);
        } else {
            row->panel_gdat_index =
                dm2_v1_viewport_door_panel_graphic_index_for_square(square);
        }
        row->frame_gdat_index =
            dm2_v1_viewport_door_frame_graphic_index_for_square(square);
        row->door_open_pct = vs->door_open_pct;
        row->door_state = vs->door_state;
        row->ornate_gdat_index =
            dm2_v1_viewport_door_ornate_graphic_index(vs->ornament_index,
                                                      square);
        if (vs->door_state == 5) {
            row->destroyed_mask_gdat_index =
                dm2_v1_viewport_door_destroyed_mask_graphic_index(
                    vs->door_gfx_index,
                    square);
        }
        row->fallback_color = 10;
        row->panel_rect = panel_rect;
        row->panel_visible_rect =
            dm2_v1_viewport_door_visible_panel_rect(&panel_rect,
                                                    row->door_open_pct);
        row->frame_rect = dm2_v1_viewport_wall_frame_rect(square);
        if (vs->door_button || vs->door_wall_button) {
            (void)dm2_v1_viewport_door_button_rect_for_square(
                square,
                &row->button_rect);
            if (vs->door_button) {
                row->button_gdat_index =
                    dm2_v1_viewport_door_button_graphic_index_for_state(
                        vs->door_button_state != 0);
                row->button_source_kind = 1;
            } else {
                row->button_gdat_index =
                    dm2_v1_viewport_wall_button_graphic_index(
                        vs->door_wall_button_index,
                        vs->door_wall_button_field);
                row->button_source_kind = 2;
                row->wall_button_index = vs->door_wall_button_index;
                row->wall_button_field = vs->door_wall_button_field;
            }
        }
    }
    return 1;
}

static int dm2_v1_viewport_full_rect_asset_blit(
    int gdat_index,
    const DM2_V1_ViewportRect *dst_rect,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit)
{
    DM2_V1_DoorAssetBlit blit;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (gdat_index == 0 || !dst_rect || dst_rect->w <= 0 ||
        dst_rect->h <= 0 || src_w <= 0 || src_h <= 0) {
        *out_blit = blit;
        return 0;
    }
    blit.gdat_index = gdat_index;
    blit.src_rect = (DM2_V1_ViewportRect){ 0, 0, src_w, src_h };
    blit.dst_rect = *dst_rect;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    *out_blit = blit;
    return 1;
}

int dm2_v1_viewport_door_panel_asset_blit(
    const DM2_V1_DoorRender *render,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit)
{
    DM2_V1_DoorAssetBlit blit;
    int source_y;
    int source_h;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (!render || render->panel_gdat_index == 0 ||
        render->panel_rect.w <= 0 || render->panel_rect.h <= 0 ||
        render->panel_visible_rect.w <= 0 ||
        render->panel_visible_rect.h <= 0 ||
        src_w <= 0 || src_h <= 0) {
        *out_blit = blit;
        return 0;
    }

    source_y =
        ((render->panel_visible_rect.y - render->panel_rect.y) * src_h) /
        render->panel_rect.h;
    source_h =
        (render->panel_visible_rect.h * src_h + render->panel_rect.h - 1) /
        render->panel_rect.h;
    if (source_y < 0) source_y = 0;
    if (source_y > src_h) source_y = src_h;
    if (source_h < 1) source_h = 1;
    if (source_y + source_h > src_h) {
        source_h = src_h - source_y;
    }

    blit.gdat_index = render->panel_gdat_index;
    blit.src_rect = (DM2_V1_ViewportRect){ 0, source_y, src_w, source_h };
    blit.dst_rect = render->panel_visible_rect;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    *out_blit = blit;
    return source_h > 0;
}

int dm2_v1_viewport_door_frame_asset_blit(
    const DM2_V1_DoorRender *render,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit)
{
    if (!render) {
        if (out_blit) {
            memset(out_blit, 0, sizeof(*out_blit));
            out_blit->gdat_index = -1;
            out_blit->transparent_color = DM2_COLOR_TRANSPARENT;
        }
        return 0;
    }
    return dm2_v1_viewport_full_rect_asset_blit(render->frame_gdat_index,
                                                &render->frame_rect,
                                                src_w,
                                                src_h,
                                                src_stride,
                                                out_blit);
}

int dm2_v1_viewport_door_button_asset_blit(
    const DM2_V1_DoorRender *render,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit)
{
    if (!render) {
        if (out_blit) {
            memset(out_blit, 0, sizeof(*out_blit));
            out_blit->gdat_index = -1;
            out_blit->transparent_color = DM2_COLOR_TRANSPARENT;
        }
        return 0;
    }
    return dm2_v1_viewport_full_rect_asset_blit(render->button_gdat_index,
                                                &render->button_rect,
                                                src_w,
                                                src_h,
                                                src_stride,
                                                out_blit);
}

/* ── Internal blit helper ─────────────────────────────────────────── */

static void __attribute__((unused)) dm2_blit_bitmap (
    uint8_t *vp,
    int vp_stride,
    const uint8_t *bitmap,
    const DM2_WallFrame *frame,
    int bitmap_stride,
    int flip_horizontal,
    int parity_flip)
{
    if (!vp || !bitmap || !frame) return;
    if (frame->byte_width == 0 || frame->height == 0) return;

    DM2_BlitClipGate gate = dm2_resolve_blit_clip(
        frame, frame->byte_width, frame->height,
        DM2_VP_WIDTH, DM2_VP_HEIGHT);
    if (!gate.visible) return;

    for (int y = 0; y < gate.height; y++) {
        const uint8_t *src_row = bitmap + (gate.src_y + y) * bitmap_stride;
        uint8_t *dst_row = vp + (gate.dst_y + y) * vp_stride;

        for (int x = 0; x < gate.width; x++) {
            int sx = flip_horizontal
                       ? (frame->byte_width - 1 - (gate.src_x + x))
                       : (gate.src_x + x);
            uint8_t pixel = src_row[sx];
            if (pixel != DM2_COLOR_TRANSPARENT) {
                dst_row[gate.dst_x + x] = pixel;
            }
        }
        (void)parity_flip;
    }
}

static int dm2_v1_fetch_viewport_asset(DM2_V1_ViewportState *s,
                                       int gdat_index,
                                       const uint8_t **out_pixels,
                                       int *out_w,
                                       int *out_h,
                                       int *out_stride)
{
    if (out_pixels) *out_pixels = NULL;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (out_stride) *out_stride = 0;
    if (s && s->asset_fetch &&
        s->asset_fetch(s->asset_user,
                       gdat_index,
                       out_pixels,
                       out_w,
                       out_h,
                       out_stride) == 0) {
        return 0;
    }
    return dm2_v1_gfx_fetch(gdat_index, out_pixels, out_w, out_h, out_stride);
}

static void dm2_v1_blit_tiled_bitmap_offset(uint8_t *dst,
                                            int dst_stride,
                                            int dst_x,
                                            int dst_y,
                                            int dst_w,
                                            int dst_h,
                                            const uint8_t *src,
                                            int src_w,
                                            int src_h,
                                            int src_stride,
                                            int transparent_color,
                                            int src_x_offset,
                                            int src_y_offset);

static void dm2_v1_blit_tiled_bitmap(uint8_t *dst,
                                     int dst_stride,
                                     int dst_x,
                                     int dst_y,
                                     int dst_w,
                                     int dst_h,
                                     const uint8_t *src,
                                     int src_w,
                                     int src_h,
                                     int src_stride,
                                     int transparent_color)
{
    dm2_v1_blit_tiled_bitmap_offset(dst,
                                    dst_stride,
                                    dst_x,
                                    dst_y,
                                    dst_w,
                                    dst_h,
                                    src,
                                    src_w,
                                    src_h,
                                    src_stride,
                                    transparent_color,
                                    0,
                                    0);
}

static void dm2_v1_blit_tiled_bitmap_offset(uint8_t *dst,
                                            int dst_stride,
                                            int dst_x,
                                            int dst_y,
                                            int dst_w,
                                            int dst_h,
                                            const uint8_t *src,
                                            int src_w,
                                            int src_h,
                                            int src_stride,
                                            int transparent_color,
                                            int src_x_offset,
                                            int src_y_offset)
{
    int y;

    if (!dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride < src_w) {
        return;
    }
    for (y = 0; y < dst_h; ++y) {
        int sy = (y + src_y_offset) % src_h;
        int fy = dst_y + y;
        int x;
        if (sy < 0) sy += src_h;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (x = 0; x < dst_w; ++x) {
            int sx = (x + src_x_offset) % src_w;
            int fx = dst_x + x;
            uint8_t pixel;
            if (sx < 0) sx += src_w;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            pixel = src[sy * src_stride + sx];
            if (transparent_color >= 0 &&
                pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] = pixel;
        }
    }
}

static uint8_t dm2_v1_gdat_scene_light_color(
    const DM2_V1_ViewportState *s,
    uint8_t color)
{
    int ambient;
    int highest;

    if (!s || !s->gdat_scene_control_ready) {
        return color;
    }
    ambient = (int)(s->gdat_ambient_light & 0x0f);
    highest = (int)(s->gdat_highest_light_level & 0x0f);
    if (highest <= 0) highest = 15;
    if (color > (uint8_t)highest) {
        color = (uint8_t)highest;
    }
    if (color != DM2_COL_BLACK && color < (uint8_t)ambient) {
        color = (uint8_t)ambient;
    }
    return color;
}

static void dm2_v1_blit_scaled_bitmap(uint8_t *dst,
                                      int dst_stride,
                                      int dst_x,
                                      int dst_y,
                                      int dst_w,
                                      int dst_h,
                                      const uint8_t *src,
                                      int src_w,
                                      int src_h,
                                      int src_stride,
                                      int transparent_color)
{
    int y;

    if (!dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride < src_w) {
        return;
    }
    for (y = 0; y < dst_h; ++y) {
        int sy = (y * src_h) / dst_h;
        int fy = dst_y + y;
        int x;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (x = 0; x < dst_w; ++x) {
            int sx = (x * src_w) / dst_w;
            int fx = dst_x + x;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            pixel = src[sy * src_stride + sx];
            if (transparent_color >= 0 &&
                pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] = pixel;
        }
    }
}

static void dm2_v1_blit_scaled_bitmap_region_ex(uint8_t *dst,
                                                int dst_stride,
                                                int dst_x,
                                                int dst_y,
                                                int dst_w,
                                                int dst_h,
                                                const uint8_t *src,
                                                int src_x,
                                                int src_y,
                                                int src_w,
                                                int src_h,
                                                int src_stride,
                                                int transparent_color,
                                                int flip_mirror)
{
    int y;

    if (!dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride <= 0) {
        return;
    }
    for (y = 0; y < dst_h; ++y) {
        int sy = src_y + (y * src_h) / dst_h;
        int fy = dst_y + y;
        int x;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (x = 0; x < dst_w; ++x) {
            int rx = (x * src_w) / dst_w;
            int sx = src_x + ((flip_mirror & 1) ? (src_w - 1 - rx) : rx);
            int fx = dst_x + x;
            uint8_t pixel;
            if (flip_mirror & 2) {
                sy = src_y + src_h - 1 - ((y * src_h) / dst_h);
            }
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH ||
                sx < 0 || sy < 0 || sx >= src_stride) {
                continue;
            }
            pixel = src[sy * src_stride + sx];
            if (transparent_color >= 0 &&
                pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] = pixel;
        }
    }
}

static void dm2_v1_blit_scaled_bitmap_region(uint8_t *dst,
                                             int dst_stride,
                                             int dst_x,
                                             int dst_y,
                                             int dst_w,
                                             int dst_h,
                                             const uint8_t *src,
                                             int src_x,
                                             int src_y,
                                             int src_w,
                                             int src_h,
                                             int src_stride,
                                             int transparent_color)
{
    dm2_v1_blit_scaled_bitmap_region_ex(dst,
                                        dst_stride,
                                        dst_x,
                                        dst_y,
                                        dst_w,
                                        dst_h,
                                        src,
                                        src_x,
                                        src_y,
                                        src_w,
                                        src_h,
                                        src_stride,
                                        transparent_color,
                                        0);
}

static int dm2_v1_prepare_map_chip_frame(int src_w,
                                         int src_h,
                                         int requested_frame,
                                         int *out_frame_x,
                                         int *out_frame_w)
{
    int frame_w = dm2_v1_viewport_map_chip_frame_width(src_w, src_h);
    int frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
    int frame_index = dm2_v1_viewport_map_chip_frame_index(requested_frame,
                                                          frame_count);
    if (frame_w <= 0 || frame_count <= 0) return 0;
    if (out_frame_x) *out_frame_x = frame_index * frame_w;
    if (out_frame_w) *out_frame_w = frame_w;
    return 1;
}

static int dm2_v1_prepare_projectile_map_chip_frame(int src_w,
                                                    int src_h,
                                                    int requested_frame,
                                                    int projectile_direction,
                                                    int object_direction,
                                                    int party_direction,
                                                    int frame_class,
                                                    int *out_frame_x,
                                                    int *out_frame_w)
{
    int frame_w = dm2_v1_viewport_map_chip_frame_width(src_w, src_h);
    int frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
    int frame_index = dm2_v1_viewport_projectile_frame_for_map_chip(
        requested_frame,
        projectile_direction,
        object_direction,
        party_direction,
        frame_count,
        frame_class);
    if (frame_w <= 0 || frame_count <= 0) return 0;
    if (out_frame_x) *out_frame_x = frame_index * frame_w;
    if (out_frame_w) *out_frame_w = frame_w;
    return 1;
}

static int dm2_v1_viewport_scaled_sprite_extent(int src_extent,
                                                int depth,
                                                int min_extent,
                                                int max_extent)
{
    int scale_pct;
    int extent = src_extent;
    if (extent <= 0) return 0;
    if (depth <= 0) {
        scale_pct = 100;
    } else if (depth == 1) {
        scale_pct = 80;
    } else if (depth == 2) {
        scale_pct = 62;
    } else if (depth == 3) {
        scale_pct = 48;
    } else {
        scale_pct = 36;
    }
    extent = (extent * scale_pct + 50) / 100;
    if (extent < min_extent) extent = min_extent;
    if (extent > max_extent) extent = max_extent;
    return extent;
}

static DM2_V1_ViewportRect dm2_v1_centered_rect(int center_x,
                                                int center_y,
                                                int w,
                                                int h)
{
    DM2_V1_ViewportRect rect = { 0, 0, 0, 0 };
    if (w <= 0 || h <= 0) {
        return rect;
    }
    rect.x = center_x - w / 2;
    rect.y = center_y - h / 2;
    rect.w = w;
    rect.h = h;
    return rect;
}

int dm2_v1_viewport_build_creature_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_CreatureRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_TEMP_PICST/QUERY_DUNGEON_MAP_CHIP_PICT
     * selects a creature map-chip image before DRAW_CHIP_OF_MAGIC_MAP scales it
     * at the visible-cell center. Keep identity, fallback, and health geometry
     * in a renderer-owned plan so the draw pass only fetches/copies pixels. */
    for (int i = 0; i < s->creature_count &&
                    i < DM2_MAX_CREATURES_PER_SQ; ++i) {
        const DM2_CreatureSprite *src = &s->creatures[i];
        DM2_V1_CreatureRender *row;
        int fallback_size = 8;
        int fallback_h = fallback_size;
        int bar_w = 16;

        if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
            src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT ||
            out_plan->creature_count >= DM2_MAX_CREATURES_PER_SQ) {
            continue;
        }

        row = &out_plan->creatures[out_plan->creature_count++];
        row->creature_index = i;
        row->creature_type = src->creature_type;
        row->frame_index = src->frame_index;
        row->direction = src->direction;
        row->depth = src->depth;
        row->center_x = src->screen_x;
        row->center_y = src->screen_y;
        row->gdat_index = dm2_v1_viewport_creature_graphic_index(
            src->creature_type,
            src->frame_index);
        row->fallback_color = (uint8_t)(11 + (src->creature_type & 7));
        row->fallback_rect = dm2_v1_centered_rect(row->center_x,
                                                  row->center_y,
                                                  fallback_size,
                                                  fallback_size);
        if (src->health_pct < 100) {
            row->health_bg_rect =
                (DM2_V1_ViewportRect){ row->center_x - bar_w / 2,
                                       row->center_y - fallback_h / 2 - 4,
                                       bar_w,
                                       1 };
            row->health_fill_rect = row->health_bg_rect;
            row->health_fill_rect.w =
                (bar_w * (int)src->health_pct) / 100;
        }
    }
    return 1;
}

int dm2_v1_viewport_creature_asset_blit(
    const DM2_V1_CreatureRender *render,
    int src_w,
    int src_h,
    int src_stride,
    int party_direction,
    DM2_V1_CreatureAssetBlit *out_blit)
{
    DM2_V1_CreatureAssetBlit blit;
    int frame_x = 0;
    int frame_w = src_w;
    int frame_count;
    int render_frame = 0;
    int dst_w;
    int dst_h;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (!render || render->gdat_index == 0 || src_w <= 0 || src_h <= 0) {
        *out_blit = blit;
        return 0;
    }

    frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
    render_frame = dm2_v1_viewport_creature_frame_for_direction(
        render->frame_index,
        render->direction,
        party_direction,
        frame_count);
    if (!dm2_v1_prepare_map_chip_frame(src_w, src_h,
                                       render_frame,
                                       &frame_x,
                                       &frame_w)) {
        frame_x = 0;
        frame_w = src_w;
        render_frame = 0;
    }

    dst_w = dm2_v1_viewport_scaled_sprite_extent(frame_w,
                                                  render->depth,
                                                  8,
                                                  64);
    dst_h = dm2_v1_viewport_scaled_sprite_extent(src_h,
                                                  render->depth,
                                                  8,
                                                  64);

    blit.gdat_index = render->gdat_index;
    blit.frame_x = frame_x;
    blit.frame_y = 0;
    blit.frame_w = frame_w;
    blit.frame_h = src_h;
    blit.dst_rect.x = render->center_x - (dst_w / 2);
    blit.dst_rect.y = render->center_y - (dst_h / 2);
    blit.dst_rect.w = dst_w;
    blit.dst_rect.h = dst_h;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    blit.render_frame = render_frame;
    blit.draw_order = render->creature_index;
    *out_blit = blit;
    return frame_w > 0 && dst_w > 0 && dst_h > 0;
}

int dm2_v1_viewport_build_item_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_ItemRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP/DRAW_TEMP_PICST routes
     * floor possessions through QUERY_DUNGEON_MAP_CHIP_PICT before the
     * scaled draw call. Keep the object identity and bounded fallback in a
     * DM2-owned plan; source-frame clipping still depends on the fetched
     * bitmap dimensions and is resolved in the blit pass. */
    for (int i = 0; i < s->item_count && i < DM2_MAX_ITEMS_PER_SQ; ++i) {
        const DM2_ItemSprite *src = &s->items[i];
        DM2_V1_ItemRender *row;
        int category;

        if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
            src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT ||
            out_plan->item_count >= DM2_MAX_ITEMS_PER_SQ) {
            continue;
        }

        category = src->item_category ? src->item_category : 0x15;
        row = &out_plan->items[out_plan->item_count++];
        row->item_index = i;
        row->item_category = category;
        row->item_type = src->item_type;
        row->frame_index = src->frame_index;
        row->direction = src->direction;
        row->depth = src->depth;
        row->center_x = src->screen_x;
        row->center_y = src->screen_y;
        row->gdat_index = dm2_v1_viewport_item_graphic_index(
            category,
            src->item_type,
            src->frame_index);
        row->fallback_radius = 4;
        row->fallback_color = 3;
    }
    return 1;
}

int dm2_v1_viewport_build_carried_item_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_CarriedItemRenderPlan *out_plan)
{
    const DM2_ItemSprite *src;
    DM2_V1_ItemRender *row;
    int category;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s || !s->carried_item_present) {
        return 1;
    }
    src = &s->carried_item;
    if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
        src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM_IN_HAND selects the carried
     * object's GDAT identity before drawing the cursor buffer. Firestaff's
     * bounded viewport overlay keeps that identity/fallback state explicit. */
    category = src->item_category ? src->item_category : 0x15;
    row = &out_plan->item;
    out_plan->item_present = 1;
    row->item_index = 0;
    row->item_category = category;
    row->item_type = src->item_type;
    row->frame_index = src->frame_index;
    row->direction = src->direction;
    row->depth = src->depth;
    row->center_x = src->screen_x;
    row->center_y = src->screen_y;
    row->gdat_index = dm2_v1_viewport_item_graphic_index(
        category,
        src->item_type,
        src->frame_index);
    row->fallback_radius = 5;
    row->fallback_color = (uint8_t)(12 + (src->item_type & 3));
    return 1;
}

int dm2_v1_viewport_build_creature_possession_item_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_CreaturePossessionItemRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10782-10817 draws
     * source-visible creature possessions after the creature with frame 0
     * and the _4976_3fa4 object-direction flip table. */
    for (int i = 0;
         i < s->creature_possession_item_count &&
             i < DM2_MAX_CREATURE_POSSESSION_ITEMS;
         ++i) {
        const DM2_ItemSprite *src = &s->creature_possession_items[i];
        DM2_V1_ItemRender *row;
        int category;

        if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
            src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT ||
            out_plan->item_count >= DM2_MAX_CREATURE_POSSESSION_ITEMS) {
            continue;
        }

        category = src->item_category ? src->item_category : 0x15;
        row = &out_plan->items[out_plan->item_count++];
        row->item_index = i;
        row->item_category = category;
        row->item_type = src->item_type;
        row->frame_index = 0;
        row->direction = src->direction;
        row->depth = src->depth;
        row->center_x = src->screen_x;
        row->center_y = src->screen_y;
        row->gdat_index = dm2_v1_viewport_item_graphic_index(
            category,
            src->item_type,
            0);
        row->flip_mirror =
            dm2_v1_viewport_map_chip_flip_for_object_direction(
                src->direction,
                s->party_dir);
        row->fallback_radius = 3;
        row->fallback_color = (uint8_t)(9 + (src->item_type & 5));
    }
    return 1;
}

int dm2_v1_viewport_item_asset_blit(
    const DM2_V1_ItemRender *render,
    int src_w,
    int src_h,
    int src_stride,
    int scale_base,
    int scale_max,
    DM2_V1_ItemAssetBlit *out_blit)
{
    DM2_V1_ItemAssetBlit blit;
    int frame_x = 0;
    int frame_w = src_w;
    int frame_count;
    int render_frame = 0;
    int dst_w;
    int dst_h;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (!render || render->gdat_index == 0 ||
        src_w <= 0 || src_h <= 0 ||
        scale_base <= 0 || scale_max <= 0) {
        *out_blit = blit;
        return 0;
    }

    frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
    render_frame = dm2_v1_viewport_map_chip_frame_index(render->frame_index,
                                                        frame_count);
    if (!dm2_v1_prepare_map_chip_frame(src_w, src_h,
                                       render_frame,
                                       &frame_x,
                                       &frame_w)) {
        frame_x = 0;
        frame_w = src_w;
        render_frame = 0;
    }

    dst_w = dm2_v1_viewport_scaled_sprite_extent(frame_w,
                                                  render->depth,
                                                  scale_base,
                                                  scale_max);
    dst_h = dm2_v1_viewport_scaled_sprite_extent(src_h,
                                                  render->depth,
                                                  scale_base,
                                                  scale_max);

    blit.gdat_index = render->gdat_index;
    blit.frame_x = frame_x;
    blit.frame_y = 0;
    blit.frame_w = frame_w;
    blit.frame_h = src_h;
    blit.dst_rect.x = render->center_x - (dst_w / 2);
    blit.dst_rect.y = render->center_y - (dst_h / 2);
    blit.dst_rect.w = dst_w;
    blit.dst_rect.h = dst_h;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    blit.flip_mirror = render->flip_mirror;
    blit.render_frame = render_frame;
    blit.draw_order = render->item_index;
    *out_blit = blit;
    return frame_w > 0 && dst_w > 0 && dst_h > 0;
}

int dm2_v1_viewport_build_projectile_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_ProjectileRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_TEMP_PICST and DRAW_MAP_CHIP first
     * resolve missile/cloud map-chip identity, direction class, and mirror
     * before DRAW_CHIP_OF_MAGIC_MAP handles bitmap dimensions. Clouds still
     * draw their random mirror in the blit pass so the runtime seed advances
     * exactly when an asset-backed cloud is actually rendered. */
    for (int i = 0; i < s->projectile_count &&
                    i < DM2_MAX_PROJECTILES; ++i) {
        const DM2_Projectile *src = &s->projectiles[i];
        DM2_V1_ProjectileRender *row;
        int category;
        int speed;

        if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
            src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT ||
            out_plan->projectile_count >= DM2_MAX_PROJECTILES) {
            continue;
        }

        category = src->projectile_category ?
            src->projectile_category : 0x0d;
        row = &out_plan->projectiles[out_plan->projectile_count++];
        row->projectile_index = i;
        row->projectile_category = category;
        row->projectile_type = src->projectile_type;
        row->frame_index = src->frame_index;
        row->render_frame = src->frame_index;
        row->direction = src->direction;
        row->object_direction = src->object_direction;
        row->frame_class = src->frame_class;
        row->render_kind = src->render_kind;
        row->depth = src->depth;
        row->center_x = src->screen_x;
        row->center_y = src->screen_y;
        row->gdat_index = dm2_v1_viewport_projectile_graphic_index(
            category,
            src->projectile_type,
            src->frame_index);
        row->flip_mirror = dm2_v1_viewport_projectile_flip_for_direction(
            src->direction,
            s->party_dir);
        row->cloud_flip_from_seed =
            (src->render_kind == DM2_V1_PROJECTILE_RENDER_CLOUD);
        row->fallback_color = (uint8_t)(15 - (src->palette_shift & 7));
        row->fallback_len = 3;
        speed = (int)sqrtf((float)(src->velocity_x * src->velocity_x +
                                   src->velocity_y * src->velocity_y));
        if (speed > 0) {
            row->fallback_dx = (src->velocity_x * row->fallback_len) / speed;
            row->fallback_dy = (src->velocity_y * row->fallback_len) / speed;
        }
    }
    return 1;
}

int dm2_v1_viewport_projectile_asset_blit(
    const DM2_V1_ProjectileRender *render,
    int src_w,
    int src_h,
    int src_stride,
    int party_direction,
    int tick_count,
    uint32_t *random_seed,
    DM2_V1_ProjectileAssetBlit *out_blit)
{
    DM2_V1_ProjectileAssetBlit blit;
    int frame_x = 0;
    int frame_w = src_w;
    int render_frame;
    int flip_mirror;
    int dst_w;
    int dst_h;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (!render || render->gdat_index == 0 ||
        src_w <= 0 || src_h <= 0) {
        *out_blit = blit;
        return 0;
    }

    render_frame = render->frame_index;
    flip_mirror = render->flip_mirror;
    if (random_seed) {
        blit.random_seed_before = *random_seed;
        blit.random_seed_after = *random_seed;
    }
    if (!dm2_v1_prepare_projectile_map_chip_frame(src_w,
                                                  src_h,
                                                  render->frame_index,
                                                  render->direction,
                                                  render->object_direction,
                                                  party_direction,
                                                  render->frame_class,
                                                  &frame_x,
                                                  &frame_w)) {
        frame_x = 0;
        frame_w = src_w;
    }

    if (render->render_kind == DM2_V1_PROJECTILE_RENDER_CLOUD) {
        int frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
        if (render->cloud_flip_from_seed) {
            flip_mirror = dm2_v1_viewport_cloud_flip_for_seed(random_seed);
            if (random_seed) {
                blit.random_seed_after = *random_seed;
            }
        }
        render_frame = dm2_v1_viewport_cloud_frame_for_tick(tick_count,
                                                            frame_count);
        if (!dm2_v1_prepare_map_chip_frame(src_w, src_h, render_frame,
                                           &frame_x, &frame_w)) {
            frame_x = 0;
            frame_w = src_w;
        }
    }

    dst_w = dm2_v1_viewport_scaled_sprite_extent(frame_w,
                                                  render->depth,
                                                  3,
                                                  32);
    dst_h = dm2_v1_viewport_scaled_sprite_extent(src_h,
                                                  render->depth,
                                                  3,
                                                  32);

    blit.gdat_index = render->gdat_index;
    blit.frame_x = frame_x;
    blit.frame_y = 0;
    blit.frame_w = frame_w;
    blit.frame_h = src_h;
    blit.dst_rect.x = render->center_x - (dst_w / 2);
    blit.dst_rect.y = render->center_y - (dst_h / 2);
    blit.dst_rect.w = dst_w;
    blit.dst_rect.h = dst_h;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    blit.flip_mirror = flip_mirror;
    blit.render_frame = render_frame;
    blit.draw_order = render->projectile_index;
    *out_blit = blit;
    return frame_w > 0 && dst_w > 0 && dst_h > 0;
}

int dm2_v1_viewport_build_weather_overlay_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_WeatherOverlayRenderPlan *out_plan)
{
    int stride2;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s || s->weather <= 0 || s->rain_intensity <= 0) {
        return 1;
    }

    out_plan->kind = (DM2_V1_WeatherOverlayKind)s->weather;
    out_plan->intensity = s->rain_intensity;
    out_plan->streak_step = 3;
    out_plan->rain_color = DM2_COL_WHITE;
    out_plan->fog_target_color = DM2_COL_BLACK;
    out_plan->lightning_color = DM2_COL_WHITE;
    if (s->gdat_scene_control_ready) {
        int weather_bias = (int)((s->gdat_void_random_fall ^
                                  s->gdat_scene_flags) & 7u);
        if (weather_bias > 0) {
            out_plan->intensity += weather_bias;
            if (out_plan->intensity > 100) out_plan->intensity = 100;
        }
        out_plan->rain_color =
            dm2_v1_gdat_scene_light_color(s, out_plan->rain_color);
        out_plan->lightning_color =
            dm2_v1_gdat_scene_light_color(s, out_plan->lightning_color);
    }

    /* skproject SKWIN outdoor weather resolves overlay density and animated
     * scroll from the weather/tick state before the blitline_48-style pass.
     * Keep those render decisions in a DM2-owned plan; the pass below only
     * applies the already-bound overlay command. */
    if (s->weather == DM2_V1_WEATHER_OVERLAY_RAIN) {
        out_plan->density = (out_plan->intensity + 9) / 10;
        stride2 = out_plan->intensity / 5;
        out_plan->scroll = (s->tick_count * stride2) & 7;
    } else if (s->weather == DM2_V1_WEATHER_OVERLAY_FOG) {
        out_plan->alpha = (out_plan->intensity + 7) / 8;
    } else if (s->weather == DM2_V1_WEATHER_OVERLAY_STORM) {
        out_plan->density = (out_plan->intensity + 5) / 10;
        stride2 = out_plan->intensity / 4;
        out_plan->scroll = (s->tick_count * stride2) & 7;
        out_plan->lightning_flash = ((s->tick_count % 120) < 2);
    } else {
        out_plan->kind = DM2_V1_WEATHER_OVERLAY_NONE;
    }
    return 1;
}

int dm2_v1_viewport_build_weather_overlay_commands(
    const DM2_V1_WeatherOverlayRenderPlan *plan,
    DM2_V1_WeatherOverlayCommandPlan *out_commands)
{
    DM2_V1_WeatherOverlayCommand *cmd;

    if (!out_commands) {
        return 0;
    }
    memset(out_commands, 0, sizeof(*out_commands));
    if (!plan || plan->kind == DM2_V1_WEATHER_OVERLAY_NONE) {
        return 1;
    }

    if (plan->kind == DM2_V1_WEATHER_OVERLAY_RAIN ||
        plan->kind == DM2_V1_WEATHER_OVERLAY_STORM) {
        cmd = &out_commands->commands[out_commands->command_count++];
        cmd->kind = DM2_V1_WEATHER_COMMAND_RAIN_STREAKS;
        cmd->density = plan->density;
        cmd->scroll = plan->scroll;
        cmd->streak_step = plan->streak_step;
        cmd->color = plan->rain_color;
    }

    if (plan->kind == DM2_V1_WEATHER_OVERLAY_FOG) {
        cmd = &out_commands->commands[out_commands->command_count++];
        cmd->kind = DM2_V1_WEATHER_COMMAND_FOG_BLEND;
        cmd->alpha = plan->alpha;
        cmd->target_color = plan->fog_target_color;
    }

    if (plan->kind == DM2_V1_WEATHER_OVERLAY_STORM &&
        plan->lightning_flash &&
        out_commands->command_count < DM2_V1_WEATHER_OVERLAY_COMMAND_MAX) {
        cmd = &out_commands->commands[out_commands->command_count++];
        cmd->kind = DM2_V1_WEATHER_COMMAND_LIGHTNING_FILL;
        cmd->color = plan->lightning_color;
    }
    return 1;
}

/* ── Populate view squares from world model ─────────────────────── */

/*
 * dm2_populate_view_squares —
 *   Fill the 12 view squares from world model given party position/direction.
 *
 * For each of the 12 view squares (D3L, D3R, D3C, D2L, D2R, D2C,
 * D1L, D1R, D1C, D0L, D0R, D0C), compute the dungeon grid coordinate
 * and fetch tile data from the world model.
 *
 * DM2 has an outdoor mode where the view is fundamentally different.
 * For indoor dungeon mode, we use the same 3×4 grid projection as DM1.
 *
 * Source: SKULL.ASM T560 (dungeon viewport projection)
 *         DUNGEON.C:1371-1421 (map coordinate resolution)
 *         DM2 uses: 16-byte map descriptor with width/height override fields
 */
static void __attribute__((unused)) dm2_populate_view_squares (
    DM2_V1_ViewportState *s,
    const dm2_dungeon_world_t *world)
{
    if (!s) return;

    /* Direction vectors: N=0, E=1, S=2, W=3 */
    static const int dx[4] = {  0,  1,  0, -1 };
    static const int dy[4] = { -1,  0,  1,  0 };

    int dir = s->party_dir & 3;
    int px  = s->party_x;
    int py  = s->party_y;

    /* Per-square relative offsets (lateral = left, right of facing dir).
     * Depth 3 (D3): 4 squares ahead + 1 ahead = 5 ahead, ±2 lateral
     * Depth 2 (D2): 3 squares ahead, ±2 lateral
     * Depth 1 (D1): 2 squares ahead, ±1 lateral
     * Depth 0 (D0): 1 square ahead, ±1 lateral
     *
     * The lateral offset uses the perpendicular direction.
     * Source: DUNGEON.C:1371-1421, DUNVIEW.C:8318-8542 */
    static const struct {
        int depth;
        int lateral;  /* -2 = far-left, -1 = left, 0 = center, 1 = right, 2 = far-right */
        int fwd;      /* forward steps from party */
    } s_square_rel[DM2_SQ_COUNT] = {
        /* D3L */ { 3, -2, 5 },  /* far-left back row */
        /* D3R */ { 3,  2, 5 },  /* far-right back row */
        /* D3C */ { 3,  0, 5 },  /* center back row */
        /* D2L */ { 2, -2, 3 },  /* left mid row */
        /* D2R */ { 2,  2, 3 },  /* right mid row */
        /* D2C */ { 2,  0, 3 },  /* center mid row */
        /* D1L */ { 1, -1, 2 },  /* left near row */
        /* D1R */ { 1,  1, 2 },  /* right near row */
        /* D1C */ { 1,  0, 2 },  /* center near row */
        /* D0L */ { 0, -1, 1 },  /* immediate left */
        /* D0R */ { 0,  1, 1 },  /* immediate right */
        /* D0C */ { 0,  0, 1 },  /* immediate front */
    };

    /* Perpendicular direction index: (dir + 1) % 4 for left, (dir + 3) % 4 for right */
    int perp_dir[5] = { (dir + 1) & 3, (dir + 3) & 3, dir, dir, dir };

    for (int i = 0; i < DM2_SQ_COUNT; i++) {
        const int sq = s_square_rel[i].depth;
        const int lat = s_square_rel[i].lateral;
        const int fwd = s_square_rel[i].fwd;

        /* Resolve grid coordinate: party_pos + fwd*forward_dir + lat*perp_dir */
        int lat_idx = (lat < 0) ? (2 + lat) : lat; /* -2→0, -1→1, 0→2, 1→3, 2→4 */
        int gx = px + dx[dir] * fwd + dx[perp_dir[lat_idx]] * (lat < 0 ? -lat : lat);
        int gy = py + dy[dir] * fwd + dy[perp_dir[lat_idx]] * (lat < 0 ? -lat : lat);

        DM2_ViewSquare *vs = &s->squares[i];
        memset(vs, 0, sizeof(*vs));
        vs->square_type = DM2_SQUARE_FLOOR;
        vs->flags = DM2_SQF_NONE;
        vs->sprite_depth = sq;

        /* Fetch tile from world model if available */
        if (world && s->dungeon_level < world->map_count) {
            int tt = dm2_world_get_tile_type(world, s->dungeon_level, gx, gy);
            vs->square_type = (uint8_t)(tt < DM2_SQUARE_COUNT ? tt : DM2_SQUARE_FLOOR);
            vs->wall_parity = (sq & 1);  /* alternate wall sets for visual variety */

            /* Populate square flags based on tile type */
            if (vs->square_type == DM2_SQUARE_WALL)
                vs->flags |= DM2_SQF_HAS_WALL;
            else if (vs->square_type == DM2_SQUARE_DOOR)
                vs->flags |= (DM2_SQF_HAS_DOOR | DM2_SQF_HAS_WALL);
            else if (vs->square_type == DM2_SQUARE_FLOOR_ORNATE)
                vs->flags |= DM2_SQF_HAS_FLOOR_ORNAMENT;
            else if (vs->square_type == DM2_SQUARE_SECRET_DOOR)
                vs->flags |= (DM2_SQF_HAS_DOOR | DM2_SQF_HAS_WALL | DM2_SQF_TRANSPARENT_WALL);
            else if (vs->square_type == DM2_SQUARE_FLOOR)
                vs->flags |= DM2_SQF_NONE;

            /* Light level: DM2 uses per-tile illumination (0-15).
             * Source: SKULL.ASM T560 — per-square lighting */
            vs->light_level = 15;  /* default full light; future: use world model */
        }

        (void)gx; (void)gy; /* reserved for world model integration */
    }
}

/* ── Background ─────────────────────────────────────────────────── */

void dm2_v1_render_background(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;

    /* DM2 black area: top 37 lines, all black.
     * Source: DUNVIEW.C F0098 (line 2968), DM1 black area same height. */
    for (int y = DM2_BLACK_AREA_TOP; y < DM2_BLACK_AREA_TOP + DM2_BLACK_AREA_H; y++) {
        memset(vp + y * stride, DM2_COL_BLACK, (size_t)DM2_VP_WIDTH);
    }
}

/* ── Floor and ceiling ───────────────────────────────────────────── */

void dm2_v1_render_floor_ceiling(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    const uint8_t *ceiling_pixels = NULL;
    const uint8_t *floor_pixels = NULL;
    int ceiling_w = 0;
    int ceiling_h_src = 0;
    int ceiling_stride = 0;
    int floor_w = 0;
    int floor_h_src = 0;
    int floor_stride = 0;
    int floor_anim_x = 0;
    int floor_anim_y = 0;
    uint8_t ceiling_fallback_color = DM2_COL_DKGRAY;
    uint8_t floor_fallback_color = 5;
    int ceiling_asset =
        dm2_v1_fetch_viewport_asset(s,
                                    DM2_GRAPHIC_CEILING,
                                    &ceiling_pixels,
                                    &ceiling_w,
                                    &ceiling_h_src,
                                    &ceiling_stride) == 0 &&
        ceiling_pixels && ceiling_w > 0 && ceiling_h_src > 0;
    int floor_asset =
        dm2_v1_fetch_viewport_asset(s,
                                    DM2_GRAPHIC_FLOOR,
                                    &floor_pixels,
                                    &floor_w,
                                    &floor_h_src,
                                    &floor_stride) == 0 &&
        floor_pixels && floor_w > 0 && floor_h_src > 0;

    /* DM2 uses the same floor (G2108=-1) and ceiling (G2109=-2) indices as DM1.
     * Source: DUNVIEW.C:126-127 (G2108_Floor=-1, G2109_Ceiling=-2).
     * Ceiling: lines 0-28, Floor: lines 66-135.
     * Actual floor/ceiling graphics are provided by dm2_v1_gfx_fetch().
     * For now: fill with solid color (actual graphics deferred to asset system). */

    int ceiling_h = DM2_CEILING_H;
    if (s->gdat_scene_control_ready) {
        ceiling_fallback_color =
            dm2_v1_gdat_scene_light_color(s, ceiling_fallback_color);
        floor_fallback_color =
            dm2_v1_gdat_scene_light_color(s, floor_fallback_color);
        if (s->gdat_animated_floor != 0u) {
            floor_anim_x = (int)((s->tick_count +
                                  (int)(s->gdat_animated_floor & 7u)) & 7);
            floor_anim_y = (int)(((s->tick_count >> 1) +
                                  (int)((s->gdat_animated_floor >> 3) & 7u)) & 7);
            ++s->gdat_scene_floor_anim_consumed_count;
        }
        ++s->gdat_scene_light_consumed_count;
    }
    if (ceiling_asset) {
        dm2_v1_blit_tiled_bitmap(vp,
                                 stride,
                                 0,
                                 0,
                                 DM2_VP_WIDTH,
                                 ceiling_h,
                                 ceiling_pixels,
                                 ceiling_w,
                                 ceiling_h_src,
                                 ceiling_stride > 0 ? ceiling_stride : ceiling_w,
                                 -1);
        ++s->asset_floor_ceiling_drawn_count;
    } else {
        /* Ceiling region: dark gray (matches DM2 darker dungeon atmosphere)
         * Source: DUNVIEW.C:2996-3015 (PC34 ceiling blit path) */
        for (int y = 0; y < ceiling_h; y++) {
            /* DM2 ceiling is slightly darker than DM1 (gray-8 vs gray-9) */
            memset(vp + y * stride, ceiling_fallback_color,
                   (size_t)DM2_VP_WIDTH);
        }
        ++s->fallback_floor_ceiling_drawn_count;
    }

    int floor_y = DM2_FLOOR_Y;
    int floor_h = DM2_FLOOR_H;
    if (floor_asset) {
        dm2_v1_blit_tiled_bitmap_offset(
            vp,
            stride,
            0,
            floor_y,
            DM2_VP_WIDTH,
            floor_h,
            floor_pixels,
            floor_w,
            floor_h_src,
            floor_stride > 0 ? floor_stride : floor_w,
            -1,
            floor_anim_x,
            floor_anim_y);
        ++s->asset_floor_ceiling_drawn_count;
    } else {
        /* Floor region: brown (matches DM2 floor color)
         * Source: DUNVIEW.C:3016-3047 (PC34 floor blit path) */
        for (int y = floor_y; y < floor_y + floor_h; y++) {
            if (y < DM2_VP_HEIGHT) {
                memset(vp + y * stride, floor_fallback_color,
                       (size_t)DM2_VP_WIDTH);
            }
        }
        ++s->fallback_floor_ceiling_drawn_count;
    }

    /* DM2 distinctive: vertical wall frame area between ceiling and floor.
     * Source: DUNVIEW.C:2962-2967 (black area fill with 37 lines).
     * DM2 rooms: walls are drawn in the middle zone (lines ~25-135). */
}

/* ── Walls ───────────────────────────────────────────────────────── */

static void dm2_v1_draw_wall_fallback_rect(uint8_t *vp,
                                           int stride,
                                           const DM2_WallFrame *frame,
                                           uint8_t color)
{
    int x;
    int y;
    if (!vp || !frame || stride <= 0 ||
        frame->byte_width == 0 || frame->height == 0) {
        return;
    }
    for (y = frame->top_y; y <= frame->bottom_y && y < DM2_VP_HEIGHT; ++y) {
        if (y < 0) {
            continue;
        }
        for (x = frame->left_x; x <= frame->right_x && x < DM2_VP_WIDTH; ++x) {
            if (x >= 0) {
                vp[y * stride + x] = color;
            }
        }
    }
}

static void dm2_v1_draw_legacy_wall_fallback(uint8_t *vp, int stride)
{
    int y;
    int x;
    if (!vp || stride <= 0) {
        return;
    }
    for (y = 25; y < 25 + 51 && y < DM2_VP_HEIGHT; y++) {
        for (x = 0; x < 84; x++) vp[y * stride + x] = 8;
        for (x = 139; x < 224; x++) vp[y * stride + x] = 8;
    }
    for (y = 25; y < 25 + 50 && y < DM2_VP_HEIGHT; y++) {
        for (x = 74; x < 150; x++) vp[y * stride + x] = 8;
    }
    for (y = 20; y < 20 + 71 && y < DM2_VP_HEIGHT; y++) {
        for (x = 0; x < 75; x++) vp[y * stride + x] = 6;
        for (x = 149; x < 224; x++) vp[y * stride + x] = 6;
        for (x = 60; x < 164; x++) vp[y * stride + x] = 6;
    }
    for (y = 9; y < 9 + 111 && y < DM2_VP_HEIGHT; y++) {
        for (x = 0; x < 64; x++) vp[y * stride + x] = 4;
        for (x = 160; x < 224; x++) vp[y * stride + x] = 4;
        for (x = 32; x < 192; x++) vp[y * stride + x] = 4;
    }
    for (y = 0; y < 136 && y < DM2_VP_HEIGHT; y++) {
        for (x = 0; x < 224; x++) vp[y * stride + x] = 2;
    }
}

static void dm2_v1_draw_door_panel_fallback_rect(uint8_t *vp,
                                                 int stride,
                                                 int view_square,
                                                 const DM2_V1_ViewportRect *rect,
                                                 uint8_t color)
{
    if (!vp || !rect || stride <= 0 || rect->w <= 0 || rect->h <= 0) {
        return;
    }
    dm2_v1_fill_rect(vp, stride, rect, color);
    if (view_square == DM2_SQ_D0C) {
        dm2_v1_stroke_rect(vp, stride, rect, DM2_COL_MIDGRAY);
    } else {
        DM2_V1_ViewportRect line =
            (DM2_V1_ViewportRect){ rect->x, rect->y, 1, rect->h };
        dm2_v1_fill_rect(vp, stride, &line, DM2_COL_MIDGRAY);
        line.x = rect->x + rect->w - 1;
        dm2_v1_fill_rect(vp, stride, &line, DM2_COL_MIDGRAY);
    }
}

void dm2_v1_render_walls(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    int wall_asset_count = 0;
    int wall_fallback_count = 0;
    DM2_V1_WallPanelRenderPlan plan;

    /* DM2 wall rendering: draw back-to-front (D3→D2→D1→D0).
     * For each depth level, draw side walls first (L,R), then center (C).
     * Source: DUNVIEW.C:8466-8542 (draw order), DUNGEON.C:1371-1421.
     *
     * Wall set selection: for odd parity (wall_parity=1), use flipped set.
     * DM2 uses G3060 variant wall set (different from DM1's G2107).
     * Source: DUNVIEW.C:170-175, G3060_i_WallSet_Wall_D3C etc.
     *
     * If no asset provider is installed, retain the old aggregate fallback so
     * no-data probes keep a deterministic frame. Once a provider exists, draw
     * each source wall cell independently so asset-backed cells are not hidden
     * behind a full-viewport placeholder blanket.
     */
    if (!s->asset_fetch) {
        dm2_v1_draw_legacy_wall_fallback(vp, stride);
        ++s->fallback_wall_drawn_count;
        return;
    }

    if (!dm2_v1_viewport_build_wall_panel_render_plan(s, &plan)) {
        return;
    }

    for (int i = 0; i < plan.panel_count; ++i) {
        const DM2_V1_WallPanelRender *panel = &plan.panels[i];
        const uint8_t *wall_pixels = NULL;
        int wall_w = 0;
        int wall_h = 0;
        int wall_stride = 0;

        if (dm2_v1_fetch_viewport_asset(s,
                                        panel->gdat_index,
                                        &wall_pixels,
                                        &wall_w,
                                        &wall_h,
                                        &wall_stride) != 0 ||
            !wall_pixels || wall_w <= 0 || wall_h <= 0) {
            dm2_v1_draw_wall_fallback_rect(vp,
                                           stride,
                                           dm2_v1_get_wall_frame(
                                               panel->view_square),
                                           panel->fallback_color);
            ++wall_fallback_count;
            continue;
        }

        dm2_v1_blit_scaled_bitmap(vp,
                                  stride,
                                  panel->dst_rect.x,
                                  panel->dst_rect.y,
                                  panel->dst_rect.w,
                                  panel->dst_rect.h,
                                  wall_pixels,
                                  wall_w,
                                  wall_h,
                                  wall_stride > 0 ? wall_stride : wall_w,
                                  s->gdat_scene_control_ready
                                      ? (int)s->gdat_scene_colorkey
                                      : DM2_COLOR_TRANSPARENT);
        if (s->gdat_scene_control_ready) {
            ++s->gdat_scene_control_consumed_count;
        }
        ++wall_asset_count;
    }

    if (wall_asset_count > 0) {
        s->asset_wall_drawn_count += wall_asset_count;
    }
    s->fallback_wall_drawn_count += wall_fallback_count;
}

/* ── Doors ────────────────────────────────────────────────────────── */

void dm2_v1_render_doors(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    int door_panel_asset_count = 0;
    int door_overlay_asset_count = 0;
    int door_asset_count = 0;
    int door_button_asset_count = 0;
    int door_fallback_count = 0;
    DM2_V1_DoorRenderPlan plan;

    /* DM2 door rendering: overlays on wall squares.
     * Source: DUNVIEW.C:3082-3095 F0102_DrawDoorBitmap,
     *         DUNVIEW.C:3096-3112 F0103_DrawDoorFrameBitmapFlippedHorizontally,
     *         DUNVIEW.C:4119-4270 F0110_DrawDoorButton, F0111_DrawDoor.
     * DM2 door frames: larger/more ornate than DM1 (G2116-G2119 + G2196).
     * Source: DUNVIEW.C:148-157 (door frame indices).
     *
     * Phase 3: placeholder door rects on DM2_SQF_HAS_DOOR squares.
     * Real door graphics from GRAPHICS.DAT (Phase 4). */

    if (!dm2_v1_viewport_build_door_render_plan(s, &plan)) {
        return;
    }
    s->last_door_panel_asset_blit_valid = 0;
    s->last_door_ornate_asset_blit_valid = 0;
    s->last_door_destroyed_mask_asset_blit_valid = 0;
    s->last_door_frame_asset_blit_valid = 0;
    s->last_door_button_asset_blit_valid = 0;
    s->last_door_panel_asset_src_w = 0;
    s->last_door_panel_asset_src_h = 0;
    s->last_door_panel_asset_src_stride = 0;
    s->last_door_ornate_asset_src_w = 0;
    s->last_door_ornate_asset_src_h = 0;
    s->last_door_ornate_asset_src_stride = 0;
    s->last_door_destroyed_mask_asset_src_w = 0;
    s->last_door_destroyed_mask_asset_src_h = 0;
    s->last_door_destroyed_mask_asset_src_stride = 0;
    s->last_door_frame_asset_src_w = 0;
    s->last_door_frame_asset_src_h = 0;
    s->last_door_frame_asset_src_stride = 0;
    s->last_door_button_asset_src_w = 0;
    s->last_door_button_asset_src_h = 0;
    s->last_door_button_asset_src_stride = 0;
    memset(&s->last_door_panel_asset_blit, 0,
           sizeof(s->last_door_panel_asset_blit));
    memset(&s->last_door_ornate_asset_blit, 0,
           sizeof(s->last_door_ornate_asset_blit));
    memset(&s->last_door_destroyed_mask_asset_blit, 0,
           sizeof(s->last_door_destroyed_mask_asset_blit));
    memset(&s->last_door_frame_asset_blit, 0,
           sizeof(s->last_door_frame_asset_blit));
    memset(&s->last_door_button_asset_blit, 0,
           sizeof(s->last_door_button_asset_blit));

    for (int i = 0; i < plan.door_count; i++) {
        const DM2_V1_DoorRender *door = &plan.doors[i];

        if (door->panel_visible_rect.w > 0 &&
            door->panel_visible_rect.h > 0) {
            const uint8_t *panel_pixels = NULL;
            int panel_w = 0;
            int panel_h = 0;
            int panel_stride = 0;
            int panel_drawn_asset = 0;
            if (door->panel_gdat_index != 0 &&
                dm2_v1_fetch_viewport_asset(s,
                                            door->panel_gdat_index,
                                            &panel_pixels,
                                            &panel_w,
                                            &panel_h,
                                            &panel_stride) == 0 &&
                panel_pixels && panel_w > 0 && panel_h > 0) {
                DM2_V1_DoorAssetBlit blit;
                /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR lines
                 * ~46402-46457 draws the panel through GDAT_CATEGORY_DOORS
                 * with image 0 for D0/D1 and image 1 for D2. Door type
                 * decoding is still boot-defaulted to index 0 here. */
                if (dm2_v1_viewport_door_panel_asset_blit(door,
                                                          panel_w,
                                                          panel_h,
                                                          panel_stride,
                                                          &blit)) {
                    dm2_v1_blit_scaled_bitmap_region(
                        vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        panel_pixels,
                        blit.src_rect.x,
                        blit.src_rect.y,
                        blit.src_rect.w,
                        blit.src_rect.h,
                        blit.src_stride,
                        blit.transparent_color);
                    ++door_panel_asset_count;
                    s->last_door_panel_asset_blit_valid = 1;
                    s->last_door_panel_asset_blit = blit;
                    s->last_door_panel_asset_src_w = panel_w;
                    s->last_door_panel_asset_src_h = panel_h;
                    s->last_door_panel_asset_src_stride =
                        panel_stride > 0 ? panel_stride : panel_w;
                    panel_drawn_asset = 1;
                }
            }
            if (!panel_drawn_asset) {
                dm2_v1_draw_door_panel_fallback_rect(vp,
                                                     stride,
                                                     door->view_square,
                                                     &door->panel_visible_rect,
                                                     door->fallback_color);
                ++door_fallback_count;
            }
        }
        {
            const int overlay_indices[2] = {
                door->ornate_gdat_index,
                door->destroyed_mask_gdat_index
            };
            for (int overlay_i = 0; overlay_i < 2; ++overlay_i) {
                const uint8_t *overlay_pixels = NULL;
                int overlay_w = 0;
                int overlay_h = 0;
                int overlay_stride = 0;
                if (overlay_indices[overlay_i] != 0 &&
                    door->panel_rect.w > 0 && door->panel_rect.h > 0 &&
                    dm2_v1_fetch_viewport_asset(s,
                                                overlay_indices[overlay_i],
                                                &overlay_pixels,
                                                &overlay_w,
                                                &overlay_h,
                                                &overlay_stride) == 0 &&
                    overlay_pixels && overlay_w > 0 && overlay_h > 0) {
                    DM2_V1_DoorAssetBlit blit;
                    if (dm2_v1_viewport_full_rect_asset_blit(
                            overlay_indices[overlay_i],
                            &door->panel_rect,
                            overlay_w,
                            overlay_h,
                            overlay_stride,
                            &blit)) {
                        dm2_v1_blit_scaled_bitmap_region(
                            vp,
                            stride,
                            blit.dst_rect.x,
                            blit.dst_rect.y,
                            blit.dst_rect.w,
                            blit.dst_rect.h,
                            overlay_pixels,
                            blit.src_rect.x,
                            blit.src_rect.y,
                            blit.src_rect.w,
                            blit.src_rect.h,
                        blit.src_stride,
                        blit.transparent_color);
                        ++door_overlay_asset_count;
                        if (overlay_i == 0) {
                            s->last_door_ornate_asset_blit_valid = 1;
                            s->last_door_ornate_asset_blit = blit;
                            s->last_door_ornate_asset_src_w = overlay_w;
                            s->last_door_ornate_asset_src_h = overlay_h;
                            s->last_door_ornate_asset_src_stride =
                                overlay_stride > 0 ? overlay_stride :
                                                     overlay_w;
                        } else {
                            s->last_door_destroyed_mask_asset_blit_valid = 1;
                            s->last_door_destroyed_mask_asset_blit = blit;
                            s->last_door_destroyed_mask_asset_src_w =
                                overlay_w;
                            s->last_door_destroyed_mask_asset_src_h =
                                overlay_h;
                            s->last_door_destroyed_mask_asset_src_stride =
                                overlay_stride > 0 ? overlay_stride :
                                                     overlay_w;
                        }
                    }
                }
            }
        }
        if (door->frame_rect.w > 0 && door->frame_rect.h > 0) {
            const uint8_t *door_pixels = NULL;
            int door_w = 0;
            int door_h = 0;
            int door_stride = 0;

            if (door->frame_gdat_index != 0 &&
                dm2_v1_fetch_viewport_asset(s,
                                            door->frame_gdat_index,
                                            &door_pixels,
                                            &door_w,
                                            &door_h,
                                            &door_stride) == 0 &&
                door_pixels && door_w > 0 && door_h > 0) {
                DM2_V1_DoorAssetBlit blit;
                /* skproject GRAPHICSSET fields 0x06/0x07/0x09 are the
                 * first boot-bound door-frame images for front, D1C and D2C.
                 * This pass scales them into the current bounded DM2 frame
                 * rectangles; exact DRAW_DUNGEON_GRAPHIC offsets remain open. */
                if (dm2_v1_viewport_door_frame_asset_blit(door,
                                                          door_w,
                                                          door_h,
                                                          door_stride,
                                                          &blit)) {
                    dm2_v1_blit_scaled_bitmap_region(
                        vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        door_pixels,
                        blit.src_rect.x,
                        blit.src_rect.y,
                        blit.src_rect.w,
                        blit.src_rect.h,
                        blit.src_stride,
                        blit.transparent_color);
                    ++door_asset_count;
                    s->last_door_frame_asset_blit_valid = 1;
                    s->last_door_frame_asset_blit = blit;
                    s->last_door_frame_asset_src_w = door_w;
                    s->last_door_frame_asset_src_h = door_h;
                    s->last_door_frame_asset_src_stride =
                        door_stride > 0 ? door_stride : door_w;
                }
            }
        }
        if (door->button_gdat_index != 0 &&
            door->button_rect.w > 0 && door->button_rect.h > 0) {
            const uint8_t *button_pixels = NULL;
            int button_w = 0;
            int button_h = 0;
            int button_stride = 0;

            if (dm2_v1_fetch_viewport_asset(s,
                                            door->button_gdat_index,
                                            &button_pixels,
                                            &button_w,
                                            &button_h,
                                            &button_stride) == 0 &&
                button_pixels && button_w > 0 && button_h > 0) {
                DM2_V1_DoorAssetBlit blit;
                /* skproject SKWIN/SkWinCore.cpp DRAW_DEFAULT_DOOR_BUTTON
                 * lines ~46243-46264 renders both default door buttons and
                 * custom wall-gfx buttons through the same rectno path. Exact
                 * viewport-cell placement is isolated in
                 * dm2_v1_viewport_door_button_rect_for_square(). */
                if (dm2_v1_viewport_door_button_asset_blit(door,
                                                           button_w,
                                                           button_h,
                                                           button_stride,
                                                           &blit)) {
                    dm2_v1_blit_scaled_bitmap_region(
                        vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        button_pixels,
                        blit.src_rect.x,
                        blit.src_rect.y,
                        blit.src_rect.w,
                        blit.src_rect.h,
                        blit.src_stride,
                        blit.transparent_color);
                    ++door_button_asset_count;
                    s->last_door_button_asset_blit_valid = 1;
                    s->last_door_button_asset_blit = blit;
                    s->last_door_button_asset_src_w = button_w;
                    s->last_door_button_asset_src_h = button_h;
                    s->last_door_button_asset_src_stride =
                        button_stride > 0 ? button_stride : button_w;
                }
            }
        }
    }
    s->asset_door_panel_drawn_count += door_panel_asset_count;
    s->asset_door_overlay_drawn_count += door_overlay_asset_count;
    s->asset_door_frame_drawn_count += door_asset_count;
    s->asset_door_button_drawn_count += door_button_asset_count;
    s->fallback_door_drawn_count += door_fallback_count;
}

/* ── Creatures ───────────────────────────────────────────────────── */

void dm2_v1_render_creatures(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    DM2_V1_CreatureRenderPlan plan;

    /* DM2 creature rendering:
     * skproject SKWIN/SkWinCore.cpp lines 10557-10619 routes creature
     * records through QUERY_DUNGEON_MAP_CHIP_PICT(cls1, cls2) before
     * DRAW_CHIP_OF_MAGIC_MAP. This pass asks the boot-owned asset provider
     * for that map-chip bitmap first and only falls back to the old
     * placeholder when the GDAT image is unavailable. */

    if (!dm2_v1_viewport_build_creature_render_plan(s, &plan)) {
        return;
    }
    s->last_creature_asset_blit_valid = 0;
    s->last_creature_render_valid = 0;
    s->last_creature_draw_order = -1;
    memset(&s->last_creature_asset_render, 0,
           sizeof(s->last_creature_asset_render));
    memset(&s->last_creature_asset_blit, 0,
           sizeof(s->last_creature_asset_blit));
    memset(&s->last_creature_render, 0,
           sizeof(s->last_creature_render));
    s->last_creature_asset_src_w = 0;
    s->last_creature_asset_src_h = 0;
    s->last_creature_asset_src_stride = 0;

    for (int i = 0; i < plan.creature_count; i++) {
        const DM2_V1_CreatureRender *c = &plan.creatures[i];
        int drawn_asset = 0;
        int drawn_h = 8;

        s->last_creature_render_valid = 1;
        s->last_creature_render = *c;
        s->last_creature_draw_order = i;

        {
            const uint8_t *pixels = NULL;
            int src_w = 0;
            int src_h = 0;
            int src_stride = 0;
            if (c->gdat_index != 0 &&
                dm2_v1_fetch_viewport_asset(s, c->gdat_index, &pixels,
                                            &src_w, &src_h, &src_stride) == 0 &&
                pixels && src_w > 0 && src_h > 0) {
                DM2_V1_CreatureAssetBlit blit;
                if (dm2_v1_viewport_creature_asset_blit(c,
                                                        src_w,
                                                        src_h,
                                                        src_stride,
                                                        s->party_dir,
                                                        &blit)) {
                    drawn_h = blit.dst_rect.h;
                    dm2_v1_blit_scaled_bitmap_region(
                        vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        pixels,
                        blit.frame_x,
                        blit.frame_y,
                        blit.frame_w,
                        blit.frame_h,
                        blit.src_stride,
                        blit.transparent_color);
                    ++s->asset_creature_drawn_count;
                    s->last_creature_asset_blit_valid = 1;
                    s->last_creature_asset_render = *c;
                    s->last_creature_asset_blit = blit;
                    s->last_creature_asset_blit.draw_order = i;
                    s->last_creature_asset_src_w = src_w;
                    s->last_creature_asset_src_h = src_h;
                    s->last_creature_asset_src_stride =
                        src_stride > 0 ? src_stride : src_w;
                    drawn_asset = 1;
                }
            }
        }
        if (!drawn_asset) {
            int half_w = c->fallback_rect.w / 2;
            int half_h = c->fallback_rect.h / 2;
            for (int dy = -half_h; dy < half_h; dy++) {
                int sy = c->center_y + dy;
                if ((unsigned)sy >= (unsigned)DM2_VP_HEIGHT) continue;
                for (int dx = -half_w; dx < half_w; dx++) {
                    int sx = c->center_x + dx;
                    if ((unsigned)sx < (unsigned)DM2_VP_WIDTH)
                        vp[sy * stride + sx] = c->fallback_color;
                }
            }
            ++s->fallback_creature_drawn_count;
        }
        /* Health bar above creature */
        if (c->health_bg_rect.w > 0) {
            DM2_V1_ViewportRect bg = c->health_bg_rect;
            DM2_V1_ViewportRect fill = c->health_fill_rect;
            if (drawn_asset) {
                bg.y = c->center_y - (drawn_h / 2) - 4;
                fill.y = bg.y;
            }
            if (bg.y >= 0) {
                for (int bx = bg.x; bx < bg.x + bg.w; bx++) {
                    if ((unsigned)bx < (unsigned)DM2_VP_WIDTH)
                        vp[bg.y * stride + bx] = 4;  /* dark red = damaged */
                }
                for (int bx = fill.x; bx < fill.x + fill.w; bx++) {
                    if ((unsigned)bx < (unsigned)DM2_VP_WIDTH)
                        vp[fill.y * stride + bx] = 2;  /* green = health */
                }
            }
        }
    }
}

/* ── Items ─────────────────────────────────────────────────────────── */

void dm2_v1_render_items(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    DM2_V1_ItemRenderPlan plan;

    /* DM2 item rendering:
     * skproject SKWIN/SkWinCore.cpp lines 10523-10549 draws floor items
     * through QUERY_DUNGEON_MAP_CHIP_PICT(cls1, cls2) and
     * DRAW_CHIP_OF_MAGIC_MAP. The category is carried on the sprite when
     * available; older population code leaves it at zero and therefore uses
     * miscellaneous as a bounded fallback category. */

    if (!dm2_v1_viewport_build_item_render_plan(s, &plan)) {
        return;
    }

    for (int i = 0; i < plan.item_count; i++) {
        const DM2_V1_ItemRender *it = &plan.items[i];
        int drawn_asset = 0;

        s->last_item_render_valid = 1;
        s->last_item_asset_blit_valid = 0;
        s->last_item_source_kind = 1;
        s->last_item_draw_order = i;
        s->last_item_render = *it;
        memset(&s->last_item_asset_blit, 0, sizeof(s->last_item_asset_blit));
        s->last_item_asset_src_w = 0;
        s->last_item_asset_src_h = 0;
        s->last_item_asset_src_stride = 0;

        {
            const uint8_t *pixels = NULL;
            int src_w = 0;
            int src_h = 0;
            int src_stride = 0;
            if (it->gdat_index != 0 &&
                dm2_v1_fetch_viewport_asset(s, it->gdat_index, &pixels,
                                            &src_w, &src_h, &src_stride) == 0 &&
                pixels && src_w > 0 && src_h > 0) {
                DM2_V1_ItemAssetBlit blit;
                if (dm2_v1_viewport_item_asset_blit(it,
                                                    src_w,
                                                    src_h,
                                                    src_stride,
                                                    4,
                                                    32,
                                                    &blit)) {
                    dm2_v1_blit_scaled_bitmap_region_ex(
                        vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        pixels,
                        blit.frame_x,
                        blit.frame_y,
                        blit.frame_w,
                        blit.frame_h,
                        blit.src_stride,
                        blit.transparent_color,
                        blit.flip_mirror);
                    ++s->asset_item_drawn_count;
                    s->last_item_asset_blit_valid = 1;
                    s->last_item_asset_blit = blit;
                    s->last_item_asset_blit.draw_order = i;
                    s->last_item_asset_src_w = src_w;
                    s->last_item_asset_src_h = src_h;
                    s->last_item_asset_src_stride =
                        src_stride > 0 ? src_stride : src_w;
                    drawn_asset = 1;
                }
            }
        }
        if (!drawn_asset) {
            int sz = it->fallback_radius;
            for (int dy = -sz; dy <= sz; dy++) {
                int sy = it->center_y + dy;
                if ((unsigned)sy >= (unsigned)DM2_VP_HEIGHT) continue;
                for (int dx = -sz; dx <= sz; dx++) {
                    int sx = it->center_x + dx;
                    if ((unsigned)sx >= (unsigned)DM2_VP_WIDTH) continue;
                    if (abs(dx) + abs(dy) <= sz)
                        vp[sy * stride + sx] = it->fallback_color;
                }
            }
            ++s->fallback_item_drawn_count;
        }
    }
}

void dm2_v1_render_creature_possession_items(DM2_V1_ViewportState *s)
{
    DM2_V1_CreaturePossessionItemRenderPlan plan;
    uint8_t *vp;
    int stride;

    if (!s || !s->framebuffer) return;
    vp = s->framebuffer;
    stride = s->fb_stride;

    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10782-10817:
     * after drawing a creature, source-visible carried/embedded weapon..misc
     * records are rendered with QUERY_DUNGEON_MAP_CHIP_PICT and
     * DRAW_CHIP_OF_MAGIC_MAP(frame=0, flip=_4976_3fa4[(Dir-viewDir)&3]).
     * Runtime possession-chain extraction is a separate bridge; this pass is
     * the renderer-owned hook that keeps those overlays in source order. */
    if (!dm2_v1_viewport_build_creature_possession_item_render_plan(s,
                                                                    &plan)) {
        return;
    }
    for (int i = 0; i < plan.item_count; ++i) {
        const DM2_V1_ItemRender *it = &plan.items[i];
        int drawn_asset = 0;

        s->last_item_render_valid = 1;
        s->last_item_asset_blit_valid = 0;
        s->last_item_source_kind = 2;
        s->last_item_draw_order = i;
        s->last_item_render = *it;
        memset(&s->last_item_asset_blit, 0, sizeof(s->last_item_asset_blit));
        s->last_item_asset_src_w = 0;
        s->last_item_asset_src_h = 0;
        s->last_item_asset_src_stride = 0;

        {
            const uint8_t *pixels = NULL;
            int src_w = 0;
            int src_h = 0;
            int src_stride = 0;
            if (it->gdat_index != 0 &&
                dm2_v1_fetch_viewport_asset(s, it->gdat_index, &pixels,
                                            &src_w, &src_h, &src_stride) == 0 &&
                pixels && src_w > 0 && src_h > 0) {
                DM2_V1_ItemAssetBlit blit;
                if (dm2_v1_viewport_item_asset_blit(it,
                                                    src_w,
                                                    src_h,
                                                    src_stride,
                                                    4,
                                                    32,
                                                    &blit)) {
                    dm2_v1_blit_scaled_bitmap_region_ex(
                        vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        pixels,
                        blit.frame_x,
                        blit.frame_y,
                        blit.frame_w,
                        blit.frame_h,
                        blit.src_stride,
                        blit.transparent_color,
                        blit.flip_mirror);
                    ++s->asset_creature_possession_item_drawn_count;
                    s->last_item_asset_blit_valid = 1;
                    s->last_item_asset_blit = blit;
                    s->last_item_asset_blit.draw_order = i;
                    s->last_item_asset_src_w = src_w;
                    s->last_item_asset_src_h = src_h;
                    s->last_item_asset_src_stride =
                        src_stride > 0 ? src_stride : src_w;
                    drawn_asset = 1;
                }
            }
        }

        if (!drawn_asset) {
            int sz = it->fallback_radius;
            uint8_t color = it->fallback_color;
            for (int dy = -sz; dy <= sz; ++dy) {
                int sy = it->center_y + dy;
                if ((unsigned)sy >= (unsigned)DM2_VP_HEIGHT) continue;
                for (int dx = -sz; dx <= sz; ++dx) {
                    int sx = it->center_x + dx;
                    if ((unsigned)sx >= (unsigned)DM2_VP_WIDTH) continue;
                    if (abs(dx) + abs(dy) <= sz) {
                        vp[sy * stride + sx] = color;
                    }
                }
            }
            ++s->fallback_creature_possession_item_drawn_count;
        }
    }
}

void dm2_v1_render_carried_item(DM2_V1_ViewportState *s)
{
    DM2_V1_CarriedItemRenderPlan plan;
    const DM2_V1_ItemRender *it;
    int drawn_asset = 0;
    uint8_t *vp;
    int stride;

    if (!s || !s->framebuffer) return;

    /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM_IN_HAND lines 15753-15814
     * renders glbLeaderHandPossession from the object's GDAT class/type and
     * selected image into the leader-hand cursor buffer. Firestaff does not
     * expose that cursor buffer in this renderer yet, so the runtime binds
     * the carried object as a bounded viewport overlay using the same
     * item-map-chip asset path as floor objects. */
    if (!dm2_v1_viewport_build_carried_item_render_plan(s, &plan) ||
        !plan.item_present) {
        return;
    }

    it = &plan.item;
    vp = s->framebuffer;
    stride = s->fb_stride;
    s->last_item_render_valid = 1;
    s->last_item_asset_blit_valid = 0;
    s->last_item_source_kind = 3;
    s->last_item_draw_order = 0;
    s->last_item_render = *it;
    memset(&s->last_item_asset_blit, 0, sizeof(s->last_item_asset_blit));
    s->last_item_asset_src_w = 0;
    s->last_item_asset_src_h = 0;
    s->last_item_asset_src_stride = 0;

    {
        const uint8_t *pixels = NULL;
        int src_w = 0;
        int src_h = 0;
        int src_stride = 0;
        if (it->gdat_index != 0 &&
            dm2_v1_fetch_viewport_asset(s, it->gdat_index, &pixels,
                                        &src_w, &src_h, &src_stride) == 0 &&
            pixels && src_w > 0 && src_h > 0) {
            DM2_V1_ItemAssetBlit blit;
            if (dm2_v1_viewport_item_asset_blit(it,
                                                src_w,
                                                src_h,
                                                src_stride,
                                                8,
                                                40,
                                                &blit)) {
                dm2_v1_blit_scaled_bitmap_region_ex(
                    vp,
                    stride,
                    blit.dst_rect.x,
                    blit.dst_rect.y,
                    blit.dst_rect.w,
                    blit.dst_rect.h,
                    pixels,
                    blit.frame_x,
                    blit.frame_y,
                    blit.frame_w,
                    blit.frame_h,
                    blit.src_stride,
                    blit.transparent_color,
                    blit.flip_mirror);
                ++s->asset_carried_item_drawn_count;
                s->last_item_asset_blit_valid = 1;
                s->last_item_asset_blit = blit;
                s->last_item_asset_blit.draw_order = 0;
                s->last_item_asset_src_w = src_w;
                s->last_item_asset_src_h = src_h;
                s->last_item_asset_src_stride =
                    src_stride > 0 ? src_stride : src_w;
                drawn_asset = 1;
            }
        }
    }

    if (!drawn_asset) {
        int sz = it->fallback_radius;
        uint8_t color = it->fallback_color;
        for (int dy = -sz; dy <= sz; dy++) {
            int sy = it->center_y + dy;
            if ((unsigned)sy >= (unsigned)DM2_VP_HEIGHT) continue;
            for (int dx = -sz; dx <= sz; dx++) {
                int sx = it->center_x + dx;
                if ((unsigned)sx >= (unsigned)DM2_VP_WIDTH) continue;
                if (abs(dx) + abs(dy) <= sz) {
                    vp[sy * stride + sx] = color;
                }
            }
        }
        ++s->fallback_carried_item_drawn_count;
    }
}

/* ── Projectiles ──────────────────────────────────────────────────── */

void dm2_v1_render_projectiles(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    DM2_V1_ProjectileRenderPlan plan;

    /* DM2 projectile rendering:
     * skproject SKWIN/SkWinCore.cpp lines 10672-10750 routes missiles and
     * clouds through QUERY_DUNGEON_MAP_CHIP_PICT before DRAW_CHIP_OF_MAGIC_MAP.
     * The runtime drain gives this pass a GDAT category/type pair; missing
     * or unmapped graphics keep the bounded streak fallback. */

    if (!dm2_v1_viewport_build_projectile_render_plan(s, &plan)) {
        return;
    }

    for (int i = 0; i < plan.projectile_count; i++) {
        const DM2_V1_ProjectileRender *p = &plan.projectiles[i];
        int drawn_asset = 0;

        s->last_projectile_render_valid = 1;
        s->last_projectile_asset_blit_valid = 0;
        s->last_projectile_draw_order = i;
        s->last_projectile_render = *p;
        memset(&s->last_projectile_asset_blit, 0,
               sizeof(s->last_projectile_asset_blit));
        s->last_projectile_asset_src_w = 0;
        s->last_projectile_asset_src_h = 0;
        s->last_projectile_asset_src_stride = 0;

        {
            const uint8_t *pixels = NULL;
            int src_w = 0;
            int src_h = 0;
            int src_stride = 0;
            if (p->gdat_index != 0 &&
                dm2_v1_fetch_viewport_asset(s, p->gdat_index, &pixels,
                                            &src_w, &src_h, &src_stride) == 0 &&
                pixels && src_w > 0 && src_h > 0) {
                DM2_V1_ProjectileAssetBlit blit;
                if (dm2_v1_viewport_projectile_asset_blit(
                        p,
                        src_w,
                        src_h,
                        src_stride,
                        s->party_dir,
                        s->tick_count,
                        &s->random_seed,
                        &blit)) {
                    dm2_v1_blit_scaled_bitmap_region_ex(
                        vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        pixels,
                        blit.frame_x,
                        blit.frame_y,
                        blit.frame_w,
                        blit.frame_h,
                        blit.src_stride,
                        blit.transparent_color,
                        blit.flip_mirror);
                    ++s->asset_projectile_drawn_count;
                    s->last_projectile_asset_blit_valid = 1;
                    s->last_projectile_asset_blit = blit;
                    s->last_projectile_asset_blit.draw_order = i;
                    s->last_projectile_asset_src_w = src_w;
                    s->last_projectile_asset_src_h = src_h;
                    s->last_projectile_asset_src_stride =
                        src_stride > 0 ? src_stride : src_w;
                    drawn_asset = 1;
                }
            }
        }
        if (!drawn_asset) {
            if (p->fallback_dx != 0 || p->fallback_dy != 0) {
                for (int t = 0; t < p->fallback_len; t++) {
                    int sx = p->center_x + p->fallback_dx * t;
                    int sy = p->center_y + p->fallback_dy * t;
                    if ((unsigned)sx < (unsigned)DM2_VP_WIDTH &&
                        (unsigned)sy < (unsigned)DM2_VP_HEIGHT)
                        vp[sy * stride + sx] = p->fallback_color;
                }
            } else {
                vp[p->center_y * stride + p->center_x] = p->fallback_color;
            }
            ++s->fallback_projectile_drawn_count;
        }
    }
}

/* ── Weather overlay ──────────────────────────────────────────────── */

void dm2_v1_render_weather_overlay(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    DM2_V1_WeatherOverlayRenderPlan plan;
    DM2_V1_WeatherOverlayCommandPlan commands;
    int i;

    if (!dm2_v1_viewport_build_weather_overlay_render_plan(s, &plan) ||
        !dm2_v1_viewport_build_weather_overlay_commands(&plan, &commands) ||
        commands.command_count <= 0) {
        return;
    }
    if (s->gdat_scene_control_ready) {
        ++s->gdat_scene_weather_consumed_count;
    }

    /* DM2 outdoor weather: rain, fog, storm.
     * Source: SKULL.ASM T600 (outdoor tick, weather effects)
     *         ReDMCSB weather overlay system (blitline_48 16→8-bit)
     *
     * Rain: diagonal streaks (white pixels at intensity-modulated density).
     * Fog: gray semi-transparent overlay.
     * Storm: heavy rain + dark sky + lightning flashes.
     *
     * DM2 weather rendering uses blitline_48 (16→8-bit) for overlay.
     * Source: DUNVIEW.C:line ~5900 (weather overlay pass)
     */
    for (i = 0; i < commands.command_count; ++i) {
        const DM2_V1_WeatherOverlayCommand *cmd = &commands.commands[i];
        if (cmd->kind == DM2_V1_WEATHER_COMMAND_RAIN_STREAKS) {
            for (int y = 0; y < DM2_VP_HEIGHT; y++) {
                for (int x = 0; x < DM2_VP_WIDTH; x += 2) {
                    if (((x + y + cmd->scroll) & 7) < cmd->density) {
                        int sy = y;
                        while (sy < DM2_VP_HEIGHT && sy >= 0) {
                            if (sy < DM2_VP_HEIGHT)
                                vp[sy * stride + x] = cmd->color;
                            sy += cmd->streak_step;
                        }
                    }
                }
            }
        } else if (cmd->kind == DM2_V1_WEATHER_COMMAND_FOG_BLEND &&
                   cmd->alpha > 0) {
            for (int y = 0; y < DM2_VP_HEIGHT; y++) {
                for (int x = 0; x < DM2_VP_WIDTH; x++) {
                    uint8_t fg = vp[y * stride + x];
                    vp[y * stride + x] =
                        (uint8_t)((fg * (16 - (uint8_t)cmd->alpha) +
                                   cmd->target_color *
                                       (uint8_t)cmd->alpha) / 16);
                }
            }
        } else if (cmd->kind == DM2_V1_WEATHER_COMMAND_LIGHTNING_FILL) {
            for (int y = 0; y < DM2_VP_HEIGHT; y++) {
                for (int x = 0; x < DM2_VP_WIDTH; x++)
                    vp[y * stride + x] = cmd->color;
            }
        }
    }
}

/* ── UI Chrome ────────────────────────────────────────────────────── */

static uint32_t dm2_v1_viewport_hash_gdat_asset(uint32_t hash,
                                                int gdat_index,
                                                int w,
                                                int h)
{
    hash ^= (uint32_t)gdat_index;
    hash *= 16777619u;
    hash ^= (uint32_t)w;
    hash *= 16777619u;
    hash ^= (uint32_t)h;
    hash *= 16777619u;
    return hash;
}

static int dm2_v1_render_hud_core_asset(DM2_V1_ViewportState *s,
                                        const DM2_V1_ViewportRect *rect,
                                        int gdat_index)
{
    const uint8_t *pixels = NULL;
    int w = 0;
    int h = 0;
    int stride = 0;
    if (!s || !s->framebuffer || !rect || rect->w <= 0 || rect->h <= 0 ||
        gdat_index == 0 ||
        dm2_v1_fetch_viewport_asset(s,
                                    gdat_index,
                                    &pixels,
                                    &w,
                                    &h,
                                    &stride) != 0 ||
        !pixels || w <= 0 || h <= 0) {
        return 0;
    }
    dm2_v1_blit_scaled_bitmap(s->framebuffer,
                              s->fb_stride,
                              rect->x,
                              rect->y,
                              rect->w,
                              rect->h,
                              pixels,
                              w,
                              h,
                              stride > 0 ? stride : w,
                              DM2_COLOR_TRANSPARENT);
    ++s->asset_hud_core_drawn_count;
    s->last_hud_core_gdat_hash =
        dm2_v1_viewport_hash_gdat_asset(s->last_hud_core_gdat_hash,
                                        gdat_index,
                                        w,
                                        h);
    s->last_hud_core_pixel_count += (uint32_t)(rect->w * rect->h);
    return 1;
}

static void dm2_v1_apply_interface_theme_to_hud_plan(
    DM2_V1_ViewportState *s,
    DM2_V1_HudChromeRenderPlan *plan)
{
    const DM2_V1_InterfaceTheme *theme;
    int rect_seed;
    int icon_dx;
    int panel_dx;

    if (!s || !plan || !s->interface_theme_valid) return;
    theme = &s->interface_theme;
    if (!theme->valid || theme->semantic_hash == 0u) return;

    /* skproject/SKWIN loads interface action, font and palette records before
     * the HUD pass. Firestaff consumes the already verified GDAT semantics
     * here for live HUD colors and records that this frame used them. */
    for (int i = 0; i < plan->action_icon_count; ++i) {
        plan->action_icons[i].fill_color =
            (uint8_t)(theme->action_icon_base_color + (uint8_t)(i & 3));
    }
    for (int slot = 0; slot < plan->champion_slot_count; ++slot) {
        plan->champion_slots[slot].fill_color =
            (uint8_t)(theme->champion_frame_color + (uint8_t)(slot & 1));
    }
    if (theme->rect14_ready && theme->rect14_row_count > 0u &&
        theme->rect14_byte_count == theme->rect14_row_count * 14u) {
        /* skproject/SKWIN/SkWinCore.cpp LOAD_GDAT_INTERFACE_00_0A keeps
         * rect14 rows as placement/stretch records.  Until every row is
         * decoded into named widgets, Firestaff consumes the verified table
         * as bounded live placement entropy for HUD children that are still
         * primitive-drawn. */
        rect_seed = (int)((theme->rect14_hash ^
                           theme->rect14_row_count ^
                           theme->rect14_byte_count) & 3u);
        icon_dx = rect_seed - 1;
        panel_dx = (int)(theme->rect14_row_count & 1u);
        for (int i = 0; i < plan->action_icon_count; ++i) {
            plan->action_icons[i].frame_rect.x += icon_dx;
            plan->action_icons[i].fill_rect.x += icon_dx;
        }
        if (!plan->outdoor) {
            plan->portrait_panel_rect.x += panel_dx;
            if (plan->portrait_panel_rect.w > panel_dx) {
                plan->portrait_panel_rect.w -= panel_dx;
            }
            for (int slot = 0; slot < plan->champion_slot_count; ++slot) {
                plan->champion_slots[slot].frame_rect.x += panel_dx;
                plan->champion_slots[slot].fill_rect.x += panel_dx;
            }
        }
        s->interface_rect14_consumed = 1;
    }
    s->interface_semantics_consumed = 1;
    s->interface_semantics_hash = theme->semantic_hash;
    s->interface_semantics_byte_count =
        theme->action_table_byte_count +
        theme->font_table_byte_count +
        theme->palette_byte_count +
        (theme->rect14_ready ? theme->rect14_byte_count : 0u);
}

void dm2_v1_render_ui_chrome(DM2_V1_ViewportState *s)
{
    DM2_V1_HudChromeRenderPlan plan;
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;

    /* DM2 UI chrome:
     *   Top status bar: 28px (champion health/magic/conditions)
     *   Bottom action strip: 28px (Attack/Cast/Use/Drop/Move icons)
     *   Right portrait panel: 80px wide × 144px (champion portraits)
     *   Gold counter in top bar (DM2 specific — DM1 doesn't have gold display)
     *
     * DM2 portrait panel uses portrait graphics from GRAPHICS.DAT.
     * Source: SKULL.ASM T560 (status bar rendering)
     *         DM2_V1_CompanionUI via dm2_v2_companion_ui.c
     *
     * Phase 3: render basic UI chrome with placeholder fills.
     */
    if (!dm2_v1_viewport_build_hud_chrome_plan_for_party(
            s->is_outdoor, s->hud_party_valid ? &s->hud_party : NULL,
            &plan)) {
        return;
    }
    dm2_v1_apply_interface_theme_to_hud_plan(s, &plan);

    if (!dm2_v1_render_hud_core_asset(s,
                                      &plan.top_bar_rect,
                                      plan.top_bar_gdat_index)) {
        dm2_v1_fill_rect(vp, stride, &plan.top_bar_rect, DM2_COL_DKGRAY);
        ++s->fallback_hud_core_drawn_count;
    }
    dm2_v1_fill_rect(
        vp, stride, &plan.top_divider_rect,
        s->interface_theme_valid ? s->interface_theme.chrome_divider_color
                                 : DM2_COL_MIDGRAY);
    if (!dm2_v1_render_hud_core_asset(s,
                                      &plan.action_strip_rect,
                                      plan.action_strip_gdat_index)) {
        dm2_v1_fill_rect(vp, stride, &plan.action_strip_rect, DM2_COL_DKGRAY);
        ++s->fallback_hud_core_drawn_count;
    }
    dm2_v1_fill_rect(
        vp, stride, &plan.action_divider_rect,
        s->interface_theme_valid ? s->interface_theme.chrome_divider_color
                                 : DM2_COL_MIDGRAY);
    if (!dm2_v1_render_hud_core_asset(s,
                                      &plan.gold_box_rect,
                                      plan.gold_box_gdat_index)) {
        dm2_v1_fill_rect(vp, stride, &plan.gold_box_rect, DM2_COL_GROUND);
        ++s->fallback_hud_core_drawn_count;
    }
    dm2_v1_fill_coin_disc(
        vp, stride, &plan.gold_coin_rect,
        s->interface_theme_valid ? s->interface_theme.gold_coin_color : 11);
    dm2_v1_fill_rect(
        vp, stride, &plan.gold_label_rect,
        s->interface_theme_valid ? s->interface_theme.gold_label_color
                                 : DM2_COL_LTGRAY);
    for (int i = 0; i < plan.action_icon_count; ++i) {
        dm2_v1_stroke_rect(vp, stride, &plan.action_icons[i].frame_rect,
                           DM2_COL_MIDGRAY);
        if (!dm2_v1_render_hud_core_asset(s,
                                          &plan.action_icons[i].fill_rect,
                                          plan.action_icons[i].gdat_index)) {
            dm2_v1_fill_rect(vp, stride, &plan.action_icons[i].fill_rect,
                             plan.action_icons[i].fill_color);
            ++s->fallback_hud_core_drawn_count;
        }
    }

    if (!plan.outdoor) {
        dm2_v1_fill_rect(
            vp, stride, &plan.portrait_separator_dark_rect,
            s->interface_theme_valid ? s->interface_theme.chrome_divider_color
                                     : DM2_COL_MIDGRAY);
        dm2_v1_fill_rect(vp, stride, &plan.portrait_separator_light_rect,
                         DM2_COL_LTGRAY);
        if (!dm2_v1_render_hud_core_asset(s,
                                          &plan.portrait_panel_rect,
                                          plan.portrait_panel_gdat_index)) {
            dm2_v1_fill_rect(vp, stride, &plan.portrait_panel_rect,
                             DM2_COL_DKGRAY);
            ++s->fallback_hud_core_drawn_count;
        }
        for (int slot = 0; slot < plan.champion_slot_count; ++slot) {
            dm2_v1_fill_rect(
                vp, stride,
                &plan.champion_slots[slot].frame_rect,
                s->interface_theme_valid
                    ? s->interface_theme.champion_frame_color
                    : DM2_COL_MIDGRAY);
            dm2_v1_fill_rect(vp, stride,
                             &plan.champion_slots[slot].fill_rect,
                             plan.champion_slots[slot].fill_color);
            if (plan.champion_slots[slot].occupied) {
                const uint8_t *portrait_pixels = NULL;
                int portrait_w = 0;
                int portrait_h = 0;
                int portrait_stride = 0;
                int portrait_gdat =
                    dm2_v1_viewport_hud_portrait_graphic_index(
                        plan.champion_slots[slot].portrait_index);
                if (portrait_gdat != 0 &&
                    dm2_v1_fetch_viewport_asset(s,
                                                portrait_gdat,
                                                &portrait_pixels,
                                                &portrait_w,
                                                &portrait_h,
                                                &portrait_stride) == 0 &&
                    portrait_pixels && portrait_w > 0 && portrait_h > 0 &&
                    portrait_stride >= portrait_w) {
                    dm2_v1_blit_scaled_bitmap(vp,
                                              stride,
                                              plan.champion_slots[slot].portrait_rect.x,
                                              plan.champion_slots[slot].portrait_rect.y,
                                              plan.champion_slots[slot].portrait_rect.w,
                                              plan.champion_slots[slot].portrait_rect.h,
                                              portrait_pixels,
                                              portrait_w,
                                              portrait_h,
                                              portrait_stride,
                                              DM2_COLOR_TRANSPARENT);
                    ++s->asset_hud_portrait_drawn_count;
                } else {
                    dm2_v1_fill_rect(vp, stride,
                                     &plan.champion_slots[slot].portrait_rect,
                                     plan.champion_slots[slot].portrait_fill_color);
                    ++s->fallback_hud_portrait_drawn_count;
                }
                dm2_v1_fill_rect(
                    vp, stride,
                    &plan.champion_slots[slot].name_marker_rect,
                    s->interface_theme_valid
                        ? s->interface_theme.champion_name_color
                        : DM2_COL_WHITE);
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].hp_bar_rect,
                                 DM2_COL_BLACK);
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].hp_fill_rect,
                                 s->interface_theme_valid
                                     ? s->interface_theme.hp_fill_color
                                     : 2);
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].stamina_bar_rect,
                                 DM2_COL_BLACK);
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].stamina_fill_rect,
                                 s->interface_theme_valid
                                     ? s->interface_theme.stamina_fill_color
                                     : 11);
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].mana_bar_rect,
                                 DM2_COL_BLACK);
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].mana_fill_rect,
                                 s->interface_theme_valid
                                     ? s->interface_theme.mana_fill_color
                                     : 12);
                if (plan.champion_slots[slot].leader) {
                    dm2_v1_fill_rect(
                        vp, stride,
                        &plan.champion_slots[slot].leader_mark_rect,
                        DM2_COL_WHITE);
                }
            }
        }
    }
}

/* ── Main render entry ─────────────────────────────────────────────── */

void dm2_v1_viewport_render(DM2_V1_ViewportState *s)
{
    if (!s) return;

    /* If not dirty and no pending world update, skip full redraw.
     * For Phase 3, always render when called (dirty flag tracking
     * is wired but full optimization deferred to Phase 4). */
    if (!s->dirty && !s->framebuffer) return;
    s->asset_floor_ceiling_drawn_count = 0;
    s->fallback_floor_ceiling_drawn_count = 0;
    s->asset_wall_drawn_count = 0;
    s->fallback_wall_drawn_count = 0;
    s->asset_door_panel_drawn_count = 0;
    s->asset_door_overlay_drawn_count = 0;
    s->asset_door_frame_drawn_count = 0;
    s->asset_door_button_drawn_count = 0;
    s->fallback_door_drawn_count = 0;
    s->asset_creature_drawn_count = 0;
    s->fallback_creature_drawn_count = 0;
    s->asset_item_drawn_count = 0;
    s->fallback_item_drawn_count = 0;
    s->asset_creature_possession_item_drawn_count = 0;
    s->fallback_creature_possession_item_drawn_count = 0;
    s->asset_carried_item_drawn_count = 0;
    s->fallback_carried_item_drawn_count = 0;
    s->last_item_render_valid = 0;
    s->last_item_asset_blit_valid = 0;
    s->last_item_source_kind = 0;
    s->last_item_draw_order = -1;
    s->last_item_asset_src_w = 0;
    s->last_item_asset_src_h = 0;
    s->last_item_asset_src_stride = 0;
    memset(&s->last_item_render, 0, sizeof(s->last_item_render));
    memset(&s->last_item_asset_blit, 0, sizeof(s->last_item_asset_blit));
    s->asset_projectile_drawn_count = 0;
    s->fallback_projectile_drawn_count = 0;
    s->last_projectile_render_valid = 0;
    s->last_projectile_asset_blit_valid = 0;
    s->last_projectile_draw_order = -1;
    s->last_projectile_asset_src_w = 0;
    s->last_projectile_asset_src_h = 0;
    s->last_projectile_asset_src_stride = 0;
    memset(&s->last_projectile_render, 0,
           sizeof(s->last_projectile_render));
    memset(&s->last_projectile_asset_blit, 0,
           sizeof(s->last_projectile_asset_blit));
    s->asset_hud_core_drawn_count = 0;
    s->fallback_hud_core_drawn_count = 0;
    s->last_hud_core_gdat_hash = 2166136261u;
    s->last_hud_core_pixel_count = 0u;
    s->asset_hud_portrait_drawn_count = 0;
    s->fallback_hud_portrait_drawn_count = 0;
    s->interface_semantics_consumed = 0;
    s->interface_semantics_hash = 0u;
    s->interface_semantics_byte_count = 0u;
    s->interface_rect14_consumed = 0;
    s->gdat_scene_control_consumed_count = 0;
    s->gdat_scene_light_consumed_count = 0;
    s->gdat_scene_floor_anim_consumed_count = 0;
    s->gdat_scene_weather_consumed_count = 0;

    /* DM2 has two fundamentally different render paths:
     *   1. Indoor dungeon (is_outdoor=0): first-person 3D dungeon view
     *   2. Outdoor (is_outdoor=1): sky gradient + ground + buildings
     *
     * Source: SKULL.ASM T560 (dungeon), SKULL.ASM T600 (outdoor) */

    if (s->is_outdoor) {
        /* DM2 outdoor rendering:
         * Source: SKULL.ASM T600 (outdoor tick, sky gradient, building draw)
         *         dm2_v1_outdoor_renderer.c
         *         DUNVIEW.C:4351-4382 F0112 (ceiling pit — outdoor has no ceiling)
         *
         * Outdoor: sky gradient from dm2_v1_outdoor_sky_color(),
         * ground fill, weather overlay. */
        DM2_V1_OutdoorConfig cfg;
        dm2_v1_outdoor_init(&cfg);
        cfg.weather = s->weather;
        dm2_v1_outdoor_set_time(&cfg, s->time_of_day);

        uint8_t *vp = s->framebuffer;
        int stride = s->fb_stride;

        /* Sky gradient: top half */
        uint32_t sky_col = dm2_v1_outdoor_sky_color(&cfg);
        uint8_t sr = (uint8_t)((sky_col >> 16) & 0xFF);
        uint8_t sg = (uint8_t)((sky_col >>  8) & 0xFF);
        uint8_t sb = (uint8_t)((sky_col      ) & 0xFF);
        int sky_h = DM2_VP_HEIGHT / 2;
        for (int y = 0; y < sky_h; y++) {
            float t = (float)y / (float)(sky_h > 0 ? sky_h : 1);
            uint8_t r = (uint8_t)(sr * (1 - t) + 20 * t);
            uint8_t g = (uint8_t)(sg * (1 - t) + 20 * t);
            uint8_t b = (uint8_t)(sb * (1 - t) + 50 * t);
            uint8_t col_idx = (r > 128) ? DM2_COL_LTGRAY
                        : (r > 64) ? DM2_COL_MIDGRAY
                        : (r > 32) ? DM2_COL_DKGRAY
                        : DM2_COL_BLACK;
            /* approximate color reduction to palette index */
            (void)g; (void)b;
            for (int x = 0; x < DM2_VP_WIDTH; x++) {
                vp[y * stride + x] = col_idx;
            }
        }

        /* Ground: bottom half — brown/green */
        for (int y = sky_h; y < DM2_VP_HEIGHT; y++) {
            for (int x = 0; x < DM2_VP_WIDTH; x++) {
                vp[y * stride + x] = DM2_COL_GROUND;
            }
        }
    } else {
        /* DM2 indoor dungeon rendering:
         * Draw order (same as DM1): D3→D2→D1→D0 per depth.
         * Source: DUNGEON.C:1371-1421; DUNVIEW.C:8466-8542 */

        /* 1. Background (black) */
        dm2_v1_render_background(s);

        /* 2. Floor and ceiling */
        dm2_v1_render_floor_ceiling(s);

        /* 3. Walls — placeholder pass (real walls need GRAPHICS.DAT) */
        dm2_v1_render_walls(s);

        /* 4. Doors */
        dm2_v1_render_doors(s);

        /* 5. Floor items */
        dm2_v1_render_items(s);

        /* 6. Creatures */
        dm2_v1_render_creatures(s);

        /* 7. Creature possession/item overlays */
        dm2_v1_render_creature_possession_items(s);

        /* 8. Projectiles */
        dm2_v1_render_projectiles(s);
    }

    /* 9. Carried leader-hand item overlay */
    dm2_v1_render_carried_item(s);

    /* 10. Weather overlay (applies to both indoor and outdoor) */
    dm2_v1_render_weather_overlay(s);

    /* 11. UI chrome (always on top) */
    dm2_v1_render_ui_chrome(s);

    s->dirty = 0;
}

/* ── GDAT-backed graphic fetch ───────────────────────────────────── */

int dm2_v1_gfx_fetch(int gdat_index,
                     const uint8_t **out_pixels,
                     int *out_w, int *out_h,
                     int *out_stride)
{
    /* DM2 GRAPHICS.DAT asset loading.
     * gdat_index: category<<8 | entry (see dm2_v1_gfx_asset_loader.h)
     *
     * DM2 graphics categories:
     *   Wall graphics:    negative indices (G2107 wall set base)
     *   Floor graphics:   -1 (floor), -2 (ceiling)
     *   Door graphics:    G2116-G2119 + G2196
     *   Ornament:         G0103_as_CurrentMapDoorOrnamentsInfo[17]
     *   Creature:         SKULL.ASM creature graphic indices
     *   Item:             SKULL.ASM object graphic indices
     *   Projectile:       G0075_apuc_PaletteChanges_Projectile
     *
     * Phase 3: returns NULL/0 (no asset system yet).
     * Full GDAT loading deferred to Phase 3 asset system integration.
     *
     * Source: SKULL.ASM T560 (GDAT loading)
     *         DUNVIEW.C F0096 (LoadCurrentMapGraphics)
     *         asset_loader_m11.c (shared asset system)
     */
    (void)gdat_index;
    if (out_pixels) *out_pixels = NULL;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (out_stride) *out_stride = 0;
    return -1;
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char *dm2_v1_viewport_source_evidence(void)
{
    return
        "DM2 V1 Viewport Renderer — Phase 3\n"
        "Source: SKULL.ASM T560  — dungeon viewport rendering pipeline\n"
        "Source: SKULL.ASM T600  — outdoor viewport rendering (sky gradient, buildings)\n"
        "Source: SKULL.ASM T520  — party/movement tick, map coordinate resolution\n"
        "Source: ReDMCSB DUNGEON.C:1371-1421 — draw order, map coordinate resolution\n"
        "Source: ReDMCSB DUNVIEW.C:575-586  — G0163 wall frame table (12 entries)\n"
        "Source: ReDMCSB DUNVIEW.C:140-175  — wall set indices (G3011-G3066)\n"
        "Source: ReDMCSB DUNVIEW.C:126-127  — G2108_Floor=-1, G2109_Ceiling=-2\n"
        "Source: ReDMCSB DUNVIEW.C:148-157  — door frame indices (G2116-G2119, G2196)\n"
        "Source: ReDMCSB DUNVIEW.C:2962-3070 — F0098 DrawFloorAndCeiling, F0100 DrawWallSetBitmap\n"
        "Source: ReDMCSB DUNVIEW.C:3082-3112 — F0102 DrawDoorBitmap, F0103 DrawDoorFrameBitmapFlipped\n"
        "Source: ReDMCSB DUNVIEW.C:3940-4015 — F0108 DrawFloorOrnament, F0109 DrawDoorOrnament\n"
        "Source: ReDMCSB DUNVIEW.C:4119-4270 — F0110 DrawDoorButton, F0111 DrawDoor\n"
        "Source: ReDMCSB DUNVIEW.C:4351-4382 — F0112 DrawCeilingPit (outdoor ceiling)\n"
        "Source: ReDMCSB DUNVIEW.C:4960-5039 — object depth scale and palette changes\n"
        "Source: ReDMCSB DUNVIEW.C:4567-4581 — creature/object/projectile layer specs\n"
        "Source: ReDMCSB DUNVIEW.C:5681-5883 — projectile occlusion specs\n"
        "Source: ReDMCSB DUNVIEW.C:361        — G0103_as_CurrentMapDoorOrnamentsInfo[17]\n"
        "Source: ReDMCSB DUNVIEW.C:8466-8542 — draw order (D4L→D4R→D4C→D3L→...→D0C)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:1001-1037 — DRAW_CHIP_OF_MAGIC_MAP frame atlas offset\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:10782-10817 — DRAW_MAP_CHIP creature possession item overlays\n"
        "Source: SKULLWIN/SKWIN/c_gui_vp.cpp  — viewport blit order (reference)\n"
        "Source: docs/dm2_graphics.md         — drawing pipeline audit\n"
        "Source: docs/dm2_walls.md            — wall/door/floor rendering specifics\n"
        "Source: docs/dm2_palette.md          — DM2 palette system\n"
        "Reference: dm1_v1_viewport_3d_pc34_compat.c (DM1 draw order, wall blit patterns)\n";
}
