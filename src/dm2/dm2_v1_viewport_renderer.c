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
        dst->portrait_type_source_bound = src->portrait_type_source_bound;
        dst->state_source_bound = src->state_source_bound;
        dst->portrait_fill_color =
            (uint8_t)(8u + (dst->portrait_index & 7u));
        memcpy(dst->name, src->name, sizeof(dst->name));
        dst->name[DM2_V1_HUD_CHAMPION_NAME_MAX] = '\0';
        dst->fill_color = dst->leader ? 9u : 8u;
        dst->leader_mark_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 2, py + 3, 3, 3 };
        dst->portrait_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 4, py + 4, 18, 18 };
        marker_w = dm2_v1_hud_name_marker_width(src->name);
        dst->name_marker_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 26, py + 2,
                                   marker_w, marker_w > 0 ? 6 : 0 };
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

int dm2_v1_viewport_scene_material_graphic_index(int graphicsset_index,
                                                  int material_field)
{
    if (graphicsset_index < 0 || graphicsset_index > 0xff ||
        (material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR &&
         material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING)) {
        return 0;
    }
    return DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_BASE -
           (graphicsset_index << 8) - material_field;
}

int dm2_v1_viewport_scene_material_graphic_address(int gdat_index,
                                                    int *out_graphicsset_index,
                                                    int *out_material_field)
{
    int packed = DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_BASE - gdat_index;
    int material_field = packed & 0xff;
    int graphicsset_index = (packed >> 8) & 0xff;

    if (!out_graphicsset_index || !out_material_field || packed < 0 ||
        packed > 0x0f01 ||
        gdat_index > DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_BASE ||
        (material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR &&
         material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING)) {
        return 0;
    }
    *out_graphicsset_index = graphicsset_index;
    *out_material_field = material_field;
    return 1;
}

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

void dm2_v1_viewport_set_g1_first_map_runtime(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1FirstMapRuntimeReceipt *receipt)
{
    if (!s || !receipt || !receipt->committed ||
        !receipt->incomplete_world || receipt->object_count != 0 ||
        receipt->blocked_record_reads != 0) {
        return;
    }
    s->g1_first_map_runtime = *receipt;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_map0_teleporter_transition(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1TeleporterTransitionReceipt *receipt)
{
    if (!s || !receipt || !receipt->committed ||
        !receipt->incomplete_world || receipt->source_map != 0 ||
        receipt->generic_record_reads != 0 ||
        receipt->blocked_record_reads != 0) {
        return;
    }
    s->g1_map0_teleporter_transition = *receipt;
    s->dirty = 1;
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

void dm2_v1_viewport_set_hud_hand_action_source(
    DM2_V1_ViewportState *s,
    const DM2_V1_HudHandActionSource *source)
{
    DM2_V1_HudHandActionSource accepted;
    int expected_entry;
    int expected_rectno;

    if (!s) return;
    memset(&s->hud_hand_action_source, 0, sizeof(s->hud_hand_action_source));
    if (!source || !source->valid || source->player_index >=
        DM2_V1_HUD_CHAMPION_SLOT_COUNT || source->possession_index > 1u ||
        source->left_or_right > 1u || source->player_position > 3u ||
        source->party_direction > 3u ||
        source->map_load_token == 0u || source->scene_control_hash == 0u ||
        source->palette_hash == 0u ||
        source->destination_rect.x < 0 || source->destination_rect.y < 0 ||
        source->destination_rect.w <= 0 || source->destination_rect.h <= 0 ||
        source->destination_rect.x + source->destination_rect.w > DM2_VP_WIDTH ||
        source->destination_rect.y + source->destination_rect.h > DM2_VP_HEIGHT) {
        s->dirty = 1;
        return;
    }
    expected_entry = ((int)source->possession_index << 1) +
        (int)source->left_or_right + 2;
    expected_rectno = (source->possession_index == 1u ? 0x46 : 0x4a) +
        (((int)source->player_position + 4 -
          (int)source->party_direction) & 3);
    if (source->gdat_category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
        source->gdat_subcategory != 4u ||
        source->gdat_entry != (uint8_t)expected_entry ||
        source->rectno != (uint8_t)expected_rectno) {
        s->dirty = 1;
        return;
    }
    accepted = *source;
    s->hud_hand_action_source = accepted;
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

void dm2_v1_viewport_set_door_surface_view_provider(
    DM2_V1_ViewportState *s,
    DM2_V1_ViewportDoorSurfaceViewFetch fetch,
    void *user)
{
    if (!s) return;
    s->door_surface_view_fetch = fetch;
    s->door_surface_view_user = user;
    s->dirty = 1;
}

void dm2_v1_viewport_set_source_materials_required(
    DM2_V1_ViewportState *s, int required)
{
    if (!s) return;
    s->source_materials_required = required ? 1 : 0;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_scene_control(
    DM2_V1_ViewportState *s,
    int ready,
    int graphicsset_index,
    uint32_t hash,
    uint16_t scene_colorkey,
    uint16_t scene_flags,
    uint16_t ambient_light,
    uint16_t highest_light_level,
    uint16_t void_random_fall,
    uint16_t animated_floor,
    uint16_t scene_rain,
    uint16_t misty_map,
    uint16_t thunder_position,
    uint16_t ambient_darkness)
{
    int control_changed;

    if (!s) return;
    control_changed = s->gdat_scene_control_ready != (ready ? 1 : 0) ||
        s->gdat_scene_control_hash != (ready ? hash : 0u);
    s->gdat_scene_control_ready = ready ? 1 : 0;
    s->gdat_scene_material_index = ready && graphicsset_index >= 0 &&
        graphicsset_index <= 0xff ? graphicsset_index : 0;
    s->gdat_scene_control_hash = ready ? hash : 0u;
    s->gdat_scene_colorkey = ready ? scene_colorkey : 0u;
    s->gdat_scene_flags = ready ? scene_flags : 0u;
    s->gdat_ambient_light = ready ? ambient_light : 0u;
    s->gdat_highest_light_level = ready ? highest_light_level : 0u;
    s->gdat_void_random_fall = ready ? void_random_fall : 0u;
    s->gdat_animated_floor = ready ? animated_floor : 0u;
    s->gdat_scene_rain = ready ? scene_rain : 0u;
    s->gdat_misty_map = ready ? misty_map : 0u;
    s->gdat_thunder_position = ready ? thunder_position : 0u;
    s->gdat_ambient_darkness = ready ? ambient_darkness : 0u;
    if (control_changed) {
        /* A new active GRAPHICSSET receipt cannot inherit a previous
         * map-local static-light consumer bind. */
        s->gdat_static_light_map_load_token = 0u;
        s->gdat_static_light_scene_control_hash = 0u;
        s->gdat_static_light_control_owned = 0;
        s->gdat_static_ambient_light_map_load_token = 0u;
        s->gdat_static_ambient_light_scene_control_hash = 0u;
        s->gdat_static_ambient_light_control_owned = 0;
        s->gdat_static_ambient_darkness_map_load_token = 0u;
        s->gdat_static_ambient_darkness_scene_control_hash = 0u;
        s->gdat_static_ambient_darkness_control_owned = 0;
        s->gdat_static_scene_flags_map_load_token = 0u;
        s->gdat_static_scene_flags_scene_control_hash = 0u;
        s->gdat_static_scene_flags_control_owned = 0;
        s->gdat_static_scene_colorkey_map_load_token = 0u;
        s->gdat_static_scene_colorkey_scene_control_hash = 0u;
        s->gdat_static_scene_colorkey_control_owned = 0;
        s->gdat_static_scene_floor_material_map_load_token = 0u;
        s->gdat_static_scene_floor_material_scene_control_hash = 0u;
        s->gdat_static_scene_floor_material_owned = 0;
        s->gdat_static_scene_ceiling_material_map_load_token = 0u;
        s->gdat_static_scene_ceiling_material_scene_control_hash = 0u;
        s->gdat_static_scene_ceiling_material_owned = 0;
        s->gdat_static_scene_wall_material_map_load_token = 0u;
        s->gdat_static_scene_wall_material_scene_control_hash = 0u;
        s->gdat_static_scene_wall_material_owned = 0;
        s->gdat_static_scene_wall_material_mask = 0u;
        s->gdat_static_scene_wall_material_view_square = 0u;
        s->gdat_static_scene_wall_material_field = 0u;
        s->gdat_static_scene_door_frame_material_map_load_token = 0u;
        s->gdat_static_scene_door_frame_material_scene_control_hash = 0u;
        s->gdat_static_scene_door_frame_material_owned = 0;
        s->gdat_static_scene_door_frame_d1c_material_map_load_token = 0u;
        s->gdat_static_scene_door_frame_d1c_material_scene_control_hash = 0u;
        s->gdat_static_scene_door_frame_d1c_material_owned = 0;
        s->gdat_static_scene_door_frame_d2c_material_map_load_token = 0u;
        s->gdat_static_scene_door_frame_d2c_material_scene_control_hash = 0u;
        s->gdat_static_scene_door_frame_d2c_material_owned = 0;
        memset(&s->gdat_static_scene_record, 0,
               sizeof(s->gdat_static_scene_record));
    }
    dm2_v1_viewport_scene_light_control(
        s->gdat_highest_light_level,
        s->gdat_ambient_darkness,
        &s->gdat_scene_light_floor,
        &s->gdat_scene_light_search_depth,
        &s->gdat_scene_light_recompute_enabled);
    s->dirty = 1;
}

void dm2_v1_viewport_set_scene_map_load_token(
    DM2_V1_ViewportState *s, uint32_t source_map_load_token)
{
    if (!s) return;
    s->gdat_scene_map_load_token = source_map_load_token;
    memset(&s->gdat_static_scene_record, 0,
           sizeof(s->gdat_static_scene_record));
    s->gdat_static_light_map_load_token = 0u;
    s->gdat_static_light_scene_control_hash = 0u;
    s->gdat_static_light_control_owned = 0;
    s->gdat_static_ambient_light_map_load_token = 0u;
    s->gdat_static_ambient_light_scene_control_hash = 0u;
    s->gdat_static_ambient_light_control_owned = 0;
    s->gdat_static_ambient_darkness_map_load_token = 0u;
    s->gdat_static_ambient_darkness_scene_control_hash = 0u;
    s->gdat_static_ambient_darkness_control_owned = 0;
    s->gdat_static_scene_flags_map_load_token = 0u;
    s->gdat_static_scene_flags_scene_control_hash = 0u;
    s->gdat_static_scene_flags_control_owned = 0;
    s->gdat_static_scene_colorkey_map_load_token = 0u;
    s->gdat_static_scene_colorkey_scene_control_hash = 0u;
    s->gdat_static_scene_colorkey_control_owned = 0;
    s->gdat_static_scene_floor_material_map_load_token = 0u;
    s->gdat_static_scene_floor_material_scene_control_hash = 0u;
    s->gdat_static_scene_floor_material_owned = 0;
    s->gdat_static_scene_ceiling_material_map_load_token = 0u;
    s->gdat_static_scene_ceiling_material_scene_control_hash = 0u;
    s->gdat_static_scene_ceiling_material_owned = 0;
    s->gdat_static_scene_wall_material_map_load_token = 0u;
    s->gdat_static_scene_wall_material_scene_control_hash = 0u;
    s->gdat_static_scene_wall_material_owned = 0;
    s->gdat_static_scene_wall_material_mask = 0u;
    s->gdat_static_scene_wall_material_view_square = 0u;
    s->gdat_static_scene_wall_material_field = 0u;
    s->gdat_static_scene_door_frame_material_map_load_token = 0u;
    s->gdat_static_scene_door_frame_material_scene_control_hash = 0u;
    s->gdat_static_scene_door_frame_material_owned = 0;
    s->gdat_static_scene_door_frame_d1c_material_map_load_token = 0u;
    s->gdat_static_scene_door_frame_d1c_material_scene_control_hash = 0u;
    s->gdat_static_scene_door_frame_d1c_material_owned = 0;
    s->gdat_static_scene_door_frame_d2c_material_map_load_token = 0u;
    s->gdat_static_scene_door_frame_d2c_material_scene_control_hash = 0u;
    s->gdat_static_scene_door_frame_d2c_material_owned = 0;
    memset(&s->floor_gfx_viewport_ownership, 0,
           sizeof(s->floor_gfx_viewport_ownership));
    s->dirty = 1;
}

int dm2_v1_viewport_bind_static_graphicsset_scene_record(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    DM2_V1_GraphicsSetStaticSceneReceipt *record;

    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready || s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    record = &s->gdat_static_scene_record;
    memset(record, 0, sizeof(*record));
    record->valid = 1;
    record->map_load_token = source_map_load_token;
    record->scene_control_hash = source_scene_control_hash;
    record->graphicsset = (uint8_t)s->gdat_scene_material_index;
    record->scene_colorkey = s->gdat_scene_colorkey;
    record->scene_flags = s->gdat_scene_flags;
    record->ambient_light = s->gdat_ambient_light;
    record->highest_light_level = s->gdat_highest_light_level;
    record->ambient_darkness = s->gdat_ambient_darkness;
    record->material_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    record->floor_field = DM2_GDAT_GFXSET_FLOOR;
    record->ceiling_field = DM2_GDAT_GFXSET_CEIL;
    record->door_frame_front_d1_field = DM2_GDAT_GFXSET_DOOR_FRAME_FRONT_D1;
    record->door_frame_d1c_field = DM2_GDAT_GFXSET_DOOR_FRAME_D1C;
    record->door_frame_d2c_field = DM2_GDAT_GFXSET_DOOR_FRAME_D2C;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_light_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_light_map_load_token = source_map_load_token;
    s->gdat_static_light_scene_control_hash = source_scene_control_hash;
    s->gdat_static_light_control_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_ambient_light_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_ambient_light_map_load_token = source_map_load_token;
    s->gdat_static_ambient_light_scene_control_hash = source_scene_control_hash;
    s->gdat_static_ambient_light_control_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_ambient_darkness_map_load_token = source_map_load_token;
    s->gdat_static_ambient_darkness_scene_control_hash =
        source_scene_control_hash;
    s->gdat_static_ambient_darkness_control_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_flags_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_scene_flags_map_load_token = source_map_load_token;
    s->gdat_static_scene_flags_scene_control_hash = source_scene_control_hash;
    s->gdat_static_scene_flags_control_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_colorkey_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_scene_colorkey_map_load_token = source_map_load_token;
    s->gdat_static_scene_colorkey_scene_control_hash =
        source_scene_control_hash;
    s->gdat_static_scene_colorkey_control_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_floor_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_scene_floor_material_map_load_token = source_map_load_token;
    s->gdat_static_scene_floor_material_scene_control_hash =
        source_scene_control_hash;
    s->gdat_static_scene_floor_material_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_wall_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash,
    int view_square)
{
    int field;

    field = dm2_v1_viewport_wall_field_for_square(view_square);
    if (!s || field < 0 || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready || s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_scene_wall_material_map_load_token = source_map_load_token;
    s->gdat_static_scene_wall_material_scene_control_hash =
        source_scene_control_hash;
    s->gdat_static_scene_wall_material_owned = 1;
    s->gdat_static_scene_wall_material_mask |=
        (uint16_t)(1u << (unsigned)view_square);
    s->gdat_static_scene_wall_material_view_square = (uint8_t)view_square;
    s->gdat_static_scene_wall_material_field = (uint8_t)field;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_all_wall_materials(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    uint16_t wall_mask = 0u;

    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready || s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    /* skproject/SKULLWIN/c_gui_vp.cpp DM2_DRAW_WALL selects
     * GRAPHICSSET[viewportCell + 0x22] separately for every visible panel.
     * Validate the whole drawable set before publishing any ownership so a
     * source-required frame cannot be authorized by one convenient panel. */
    for (int view_square = 0; view_square < DM2_SQ_COUNT; ++view_square) {
        if (dm2_v1_viewport_wall_field_for_square(view_square) >= 0) {
            wall_mask |= (uint16_t)(1u << (unsigned)view_square);
        }
    }
    if (wall_mask == 0u) {
        return 0;
    }
    s->gdat_static_scene_wall_material_map_load_token = source_map_load_token;
    s->gdat_static_scene_wall_material_scene_control_hash =
        source_scene_control_hash;
    s->gdat_static_scene_wall_material_owned = 1;
    s->gdat_static_scene_wall_material_mask = wall_mask;
    s->gdat_static_scene_wall_material_view_square = DM2_SQ_D1C;
    s->gdat_static_scene_wall_material_field =
        (uint8_t)dm2_v1_viewport_wall_field_for_square(DM2_SQ_D1C);
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_ceiling_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_scene_ceiling_material_map_load_token =
        source_map_load_token;
    s->gdat_static_scene_ceiling_material_scene_control_hash =
        source_scene_control_hash;
    s->gdat_static_scene_ceiling_material_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_door_frame_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_scene_door_frame_material_map_load_token =
        source_map_load_token;
    s->gdat_static_scene_door_frame_material_scene_control_hash =
        source_scene_control_hash;
    s->gdat_static_scene_door_frame_material_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_door_frame_d1c_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_scene_door_frame_d1c_material_map_load_token =
        source_map_load_token;
    s->gdat_static_scene_door_frame_d1c_material_scene_control_hash =
        source_scene_control_hash;
    s->gdat_static_scene_door_frame_d1c_material_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_door_frame_d2c_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    if (!s || source_map_load_token == 0u ||
        source_map_load_token != s->gdat_scene_map_load_token ||
        !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        source_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    s->gdat_static_scene_door_frame_d2c_material_map_load_token =
        source_map_load_token;
    s->gdat_static_scene_door_frame_d2c_material_scene_control_hash =
        source_scene_control_hash;
    s->gdat_static_scene_door_frame_d2c_material_owned = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_set_floor_gfx_viewport_ownership(
    DM2_V1_ViewportState *s,
    const DM2_V1_FloorGfxViewportOwnershipReceipt *ownership)
{
    if (!s || !ownership || !ownership->valid || !ownership->viewport_owned ||
        ownership->map_load_token == 0u ||
        ownership->map_load_token != s->gdat_scene_map_load_token ||
        ownership->gdat_category != DM2_GDAT_CATEGORY_FLOOR_GFX) {
        return 0;
    }
    s->floor_gfx_viewport_ownership = *ownership;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_floor_gfx_render_plan_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportFloorGfxRenderPlanReceipt *out_receipt)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    scene = s ? &s->gdat_static_scene_record : NULL;
    if (!s || !s->gdat_scene_control_ready ||
        s->gdat_scene_control_hash == 0u ||
        !scene || !scene->valid ||
        scene->map_load_token != s->gdat_scene_map_load_token ||
        scene->scene_control_hash != s->gdat_scene_control_hash ||
        !s->floor_gfx_viewport_ownership.valid ||
        !s->floor_gfx_viewport_ownership.viewport_owned ||
        s->gdat_scene_map_load_token == 0u ||
        s->floor_gfx_viewport_ownership.map_load_token !=
            s->gdat_scene_map_load_token) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->static_scene_control_owned = 1;
    out_receipt->static_light_control_owned = 1;
    out_receipt->static_ambient_light_control_owned = 1;
    out_receipt->static_ambient_darkness_control_owned = 1;
    out_receipt->static_scene_flags_control_owned = 1;
    out_receipt->static_scene_colorkey_control_owned = 1;
    out_receipt->static_scene_floor_material_owned = 1;
    out_receipt->static_scene_ceiling_material_owned = 1;
    out_receipt->static_scene_door_frame_material_owned = 1;
    out_receipt->static_scene_door_frame_d1c_material_owned = 1;
    out_receipt->static_scene_door_frame_d2c_material_owned = 1;
    out_receipt->map_load_token = scene->map_load_token;
    out_receipt->gdat_category =
        s->floor_gfx_viewport_ownership.gdat_category;
    out_receipt->floor_ornate_source_index =
        s->floor_gfx_viewport_ownership.floor_ornate_source_index;
    out_receipt->animated_frame_route =
        s->floor_gfx_viewport_ownership.animated_frame_route;
    out_receipt->scene_control_hash = scene->scene_control_hash;
    out_receipt->scene_colorkey = scene->scene_colorkey;
    out_receipt->scene_flags = scene->scene_flags;
    out_receipt->outdoor_scene =
        (scene->scene_flags & DM2_V1_GDAT_SCENE_FLAG_OUTDOOR) != 0u;
    out_receipt->scene_floor_material_category = scene->material_category;
    out_receipt->scene_floor_material_graphicsset = scene->graphicsset;
    out_receipt->scene_floor_material_field = scene->floor_field;
    out_receipt->scene_ceiling_material_category = scene->material_category;
    out_receipt->scene_ceiling_material_graphicsset = scene->graphicsset;
    out_receipt->scene_ceiling_material_field = scene->ceiling_field;
    out_receipt->scene_door_frame_material_category = scene->material_category;
    out_receipt->scene_door_frame_material_graphicsset = scene->graphicsset;
    out_receipt->scene_door_frame_material_field =
        scene->door_frame_front_d1_field;
    out_receipt->scene_door_frame_d1c_material_category = scene->material_category;
    out_receipt->scene_door_frame_d1c_material_graphicsset = scene->graphicsset;
    out_receipt->scene_door_frame_d1c_material_field =
        scene->door_frame_d1c_field;
    out_receipt->scene_door_frame_d2c_material_category = scene->material_category;
    out_receipt->scene_door_frame_d2c_material_graphicsset = scene->graphicsset;
    out_receipt->scene_door_frame_d2c_material_field =
        scene->door_frame_d2c_field;
    out_receipt->ambient_light = scene->ambient_light;
    out_receipt->highest_light_level = scene->highest_light_level;
    out_receipt->ambient_darkness = scene->ambient_darkness;
    return 1;
}

void dm2_v1_viewport_scene_light_control(uint16_t highest_light_level,
                                         uint16_t ambient_darkness,
                                         uint8_t *out_light_floor,
                                         uint8_t *out_search_depth,
                                         int *out_recompute_enabled)
{
    uint8_t floor = highest_light_level > 5u ? 5u :
        (uint8_t)highest_light_level;
    uint8_t depth = ambient_darkness > 8u ? 8u :
        (uint8_t)ambient_darkness;

    if (out_light_floor) *out_light_floor = floor;
    if (out_search_depth) *out_search_depth = depth;
    if (out_recompute_enabled) *out_recompute_enabled = depth != 0u;
}

void dm2_v1_viewport_set_gdat_interface_palette(
    DM2_V1_ViewportState *s,
    int ready,
    uint32_t hash,
    const uint8_t palette16[16])
{
    if (!s) return;
    s->gdat_interface_palette_ready = ready && hash != 0u && palette16;
    s->gdat_interface_palette_hash =
        s->gdat_interface_palette_ready ? hash : 0u;
    if (s->gdat_interface_palette_ready) {
        memcpy(s->gdat_interface_palette16, palette16,
               sizeof(s->gdat_interface_palette16));
    } else {
        memset(s->gdat_interface_palette16, 0,
               sizeof(s->gdat_interface_palette16));
    }
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_interface_font(
    DM2_V1_ViewportState *s,
    const uint8_t *rows,
    uint32_t hash)
{
    if (!s) return;
    s->gdat_interface_font_rows = rows;
    s->gdat_interface_font_hash = rows && hash != 0u ? hash : 0u;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_creature_map_chip_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt)
{
    if (!s) return;
    s->g1_creature_map_chip_materials =
        receipt && receipt->valid ? receipt : NULL;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_wall_gfx_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1TextWallGfxRuntimeReceipt *text_receipt,
    const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *actuator_receipt)
{
    if (!s) return;
    s->g1_text_wall_gfx_materials =
        text_receipt && text_receipt->valid ? text_receipt : NULL;
    s->g1_actuator_wall_gfx_materials =
        actuator_receipt && actuator_receipt->valid ? actuator_receipt : NULL;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_interface_hud_layout(
    DM2_V1_ViewportState *s,
    const DM2_V1_InterfaceHudLayout *layout)
{
    if (!s) return;
    s->gdat_interface_hud_layout = layout && layout->valid ? layout : NULL;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_interface_rect14(
    DM2_V1_ViewportState *s,
    const uint8_t *rows,
    uint32_t row_count,
    uint32_t hash)
{
    if (!s) return;
    /* skproject/SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST consumes the
     * LOAD_GDAT_INTERFACE_00_0A table only after runtime has checked its
     * host receipt. An empty or unhashed buffer is never a drawable owner. */
    s->gdat_interface_rect14_rows = rows && row_count > 0u && hash != 0u
        ? rows : NULL;
    s->gdat_interface_rect14_row_count =
        s->gdat_interface_rect14_rows ? row_count : 0u;
    s->gdat_interface_rect14_hash =
        s->gdat_interface_rect14_rows ? hash : 0u;
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
    return dm2_v1_viewport_wall_graphic_index_for_graphicsset(
        DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET, view_square);
}

int dm2_v1_viewport_wall_graphic_index_for_graphicsset(int graphicsset_index,
                                                        int view_square)
{
    int field = dm2_v1_viewport_wall_field_for_square(view_square);

    if (field < 0 || graphicsset_index < 0 || graphicsset_index > 0xff) {
        return 0;
    }
    /* ReDMCSB lineage: SkWinCore.cpp DRAW_WALL lines 47466-47474 queries
     * GDAT_CATEGORY_GRAPHICSSET with the live iMapGfx for each wall cell. */
    if (graphicsset_index == DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET) {
        return DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE - field;
    }
    return DM2_V1_VIEWPORT_GFX_WALL_GRAPHICSSET_BASE -
           (graphicsset_index << 8) - field;
}

int dm2_v1_viewport_wall_graphic_address(int gdat_index,
                                         int *out_graphicsset_index,
                                         int *out_field)
{
    int packed;
    int graphicsset_index;
    int field;

    if (!out_graphicsset_index || !out_field) return 0;
    if (gdat_index <= DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE -
                          DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST &&
        gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE) {
        field = DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE - gdat_index;
        *out_graphicsset_index = DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
        *out_field = field;
        return 1;
    }
    packed = DM2_V1_VIEWPORT_GFX_WALL_GRAPHICSSET_BASE - gdat_index;
    graphicsset_index = (packed >> 8) & 0xff;
    field = packed & 0xff;
    if (packed < 0 || packed > 0xff3f ||
        gdat_index > DM2_V1_VIEWPORT_GFX_WALL_GRAPHICSSET_BASE ||
        field < DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST || field >= 0x40) {
        return 0;
    }
    *out_graphicsset_index = graphicsset_index;
    *out_field = field;
    return 1;
}

int dm2_v1_viewport_build_wall_panel_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_WallPanelRenderPlan *out_plan)
{
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
        int graphicsset_index = s && s->gdat_scene_control_ready
            ? s->gdat_scene_material_index
            : DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
        int gdat_index = dm2_v1_viewport_wall_graphic_index_for_graphicsset(
            graphicsset_index, square);
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

int dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
    int graphicsset_index, int view_square)
{
    int field = dm2_v1_viewport_door_frame_field_for_square(view_square);

    if (field < 0 || graphicsset_index < 0 || graphicsset_index > 0xff) {
        return 0;
    }
    if (graphicsset_index == DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET) {
        return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE - field;
    }
    return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_GRAPHICSSET_BASE -
        (graphicsset_index << 8) - field;
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

int dm2_v1_viewport_creature_field_graphic_index(int creature_type,
                                                 int image_field)
{
    int packed;
    if (creature_type < 0 || creature_type > 0xff ||
        image_field < 0 || image_field > 0xff) return 0;
    packed = (creature_type << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT) |
             (image_field & DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE - packed;
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

int dm2_v1_viewport_hud_hand_action_graphic_index(int possession_index,
                                                   int left_or_right)
{
    int entry;

    if (possession_index < 0 || possession_index > 1 ||
        left_or_right < 0 || left_or_right > 1) {
        return 0;
    }
    entry = (possession_index << 1) + left_or_right + 2;
    return DM2_V1_VIEWPORT_GFX_HUD_HAND_ACTION_BASE - entry;
}

int dm2_v1_viewport_hud_hand_action_graphic_address(
    int gdat_index,
    int *out_possession_index,
    int *out_left_or_right,
    int *out_entry)
{
    int entry = DM2_V1_VIEWPORT_GFX_HUD_HAND_ACTION_BASE - gdat_index;

    if (entry < 2 || entry > 5) {
        return 0;
    }
    if (out_possession_index) *out_possession_index = (entry - 2) >> 1;
    if (out_left_or_right) *out_left_or_right = (entry - 2) & 1;
    if (out_entry) *out_entry = entry;
    return 1;
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
        row->frame_gdat_index = dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
            s && s->gdat_scene_control_ready
                ? s->gdat_scene_material_index
                : DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET,
            square);
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
                row->wall_button_x = vs->door_wall_button_x;
                row->wall_button_y = vs->door_wall_button_y;
                row->wall_button_object_id = vs->door_wall_button_object_id;
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

int dm2_v1_viewport_last_original_material_gate_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_OriginalMaterialGateReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!s || !s->last_original_material_gate.valid) return 0;
    *out_receipt = s->last_original_material_gate;
    return 1;
}

int dm2_v1_viewport_last_original_door_surface_request(
    const DM2_V1_ViewportState *s,
    DM2_V1_OriginalDoorSurfaceRequest *out_request)
{
    if (!out_request) return 0;
    memset(out_request, 0, sizeof(*out_request));
    if (!s || !s->last_original_door_surface_request.valid) return 0;
    *out_request = s->last_original_door_surface_request;
    return 1;
}

int dm2_v1_viewport_last_original_door_surface_binding(
    const DM2_V1_ViewportState *s,
    DM2_V1_OriginalDoorSurfaceBinding *out_binding)
{
    if (!out_binding) return 0;
    memset(out_binding, 0, sizeof(*out_binding));
    if (!s || !s->last_original_door_surface_binding.valid) return 0;
    *out_binding = s->last_original_door_surface_binding;
    return 1;
}

int dm2_v1_viewport_last_original_door_opening_frame_request(
    const DM2_V1_ViewportState *s,
    DM2_V1_OriginalDoorOpeningFrameRequest *out_request)
{
    if (!out_request) return 0;
    memset(out_request, 0, sizeof(*out_request));
    if (!s || !s->last_original_door_opening_frame_request.valid) return 0;
    *out_request = s->last_original_door_opening_frame_request;
    return 1;
}

int dm2_v1_viewport_last_original_door_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportDoorPresentationCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_original_door_presentation_command.valid) return 0;
    *out_command = s->last_original_door_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_hud_top_bar_material_request(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudMaterialRequest *out_request)
{
    if (!out_request) return 0;
    memset(out_request, 0, sizeof(*out_request));
    if (!s || !s->last_hud_top_bar_material_request.valid) return 0;
    *out_request = s->last_hud_top_bar_material_request;
    return 1;
}

int dm2_v1_viewport_last_hud_top_bar_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudPresentationCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_hud_top_bar_presentation_command.valid) return 0;
    *out_command = s->last_hud_top_bar_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_hud_status_panel_material_request(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudMaterialRequest *out_request)
{
    if (!out_request) return 0;
    memset(out_request, 0, sizeof(*out_request));
    if (!s || !s->last_hud_status_panel_material_request.valid) return 0;
    *out_request = s->last_hud_status_panel_material_request;
    return 1;
}

int dm2_v1_viewport_last_hud_status_panel_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudPresentationCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_hud_status_panel_presentation_command.valid) return 0;
    *out_command = s->last_hud_status_panel_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_hud_hand_action_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudPresentationCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_hud_hand_action_presentation_command.valid) return 0;
    *out_command = s->last_hud_hand_action_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_dungeon_floor_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportDungeonMaterialCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_dungeon_floor_presentation_command.valid) return 0;
    *out_command = s->last_dungeon_floor_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_dungeon_ceiling_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportDungeonMaterialCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_dungeon_ceiling_presentation_command.valid) return 0;
    *out_command = s->last_dungeon_ceiling_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_dungeon_wall_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportDungeonMaterialCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_dungeon_wall_presentation_command.valid) return 0;
    *out_command = s->last_dungeon_wall_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_scene_control_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportSceneControlCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_scene_control_presentation_command.valid) return 0;
    *out_command = s->last_scene_control_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_creature_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportCreatureMaterialCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_creature_presentation_command.valid) return 0;
    *out_command = s->last_creature_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_item_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportItemMaterialCommand *out_command)
{
    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->last_item_presentation_command.valid) return 0;
    *out_command = s->last_item_presentation_command;
    return 1;
}

int dm2_v1_viewport_last_frame_composition_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportFrameCompositionReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!s || !s->last_frame_composition.valid) return 0;
    *out_receipt = s->last_frame_composition;
    return 1;
}

int dm2_v1_viewport_last_m11_frame_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportM11FrameReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!s || !s->last_m11_frame_receipt.valid) return 0;
    *out_receipt = s->last_m11_frame_receipt;
    return 1;
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

static void __attribute__((unused)) dm2_v1_blit_tiled_bitmap(uint8_t *dst,
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
        int sy = y % src_h;
        int fy = dst_y + y;
        int x;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (x = 0; x < dst_w; ++x) {
            int sx = x % src_w;
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

static void __attribute__((unused)) dm2_v1_blit_scaled_bitmap(uint8_t *dst,
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

/* skproject/SKWIN initializes dtPalIRGB and dtPalette16 before indexed GDAT
 * material is drawn. Material pixels below 16 are logical colours and must
 * reach the framebuffer through that real logical-to-IRGB index table. */
static uint8_t dm2_v1_material_palette_color(DM2_V1_ViewportState *s,
                                             uint8_t logical_color,
                                             int *consumed_count)
{
    if (!s || !s->gdat_interface_palette_ready || logical_color >= 16u) {
        return logical_color;
    }
    if (consumed_count) ++*consumed_count;
    return s->gdat_interface_palette16[logical_color];
}

static void dm2_v1_blit_scaled_material_bitmap(DM2_V1_ViewportState *s,
                                                uint8_t *dst,
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
                                                int *consumed_count)
{
    int y;
    if (!s || !dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride < src_w) return;
    for (y = 0; y < dst_h; ++y) {
        int fy = dst_y + y;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < dst_w; ++x) {
            int fx = dst_x + x;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            pixel = src[((y * src_h) / dst_h) * src_stride +
                        ((x * src_w) / dst_w)];
            if (transparent_color >= 0 && pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] =
                dm2_v1_material_palette_color(s, pixel, consumed_count);
        }
    }
}

static void dm2_v1_blit_tiled_material_bitmap(DM2_V1_ViewportState *s,
                                               uint8_t *dst,
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
                                               int *consumed_count)
{
    int y;
    if (!s || !dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride < src_w) return;
    for (y = 0; y < dst_h; ++y) {
        int fy = dst_y + y;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < dst_w; ++x) {
            int fx = dst_x + x;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            pixel = src[(y % src_h) * src_stride + (x % src_w)];
            if (transparent_color >= 0 && pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] =
                dm2_v1_material_palette_color(s, pixel, consumed_count);
        }
    }
}

static void dm2_v1_blit_scaled_material_bitmap_region(
    DM2_V1_ViewportState *s, uint8_t *dst, int dst_stride, int dst_x,
    int dst_y, int dst_w, int dst_h, const uint8_t *src, int src_x,
    int src_y, int src_w, int src_h, int src_stride, int transparent_color,
    int *consumed_count)
{
    int y;
    if (!s || !dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride <= 0) return;
    for (y = 0; y < dst_h; ++y) {
        int fy = dst_y + y;
        int sy = src_y + (y * src_h) / dst_h;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < dst_w; ++x) {
            int fx = dst_x + x;
            int sx = src_x + (x * src_w) / dst_w;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH || sx < 0 || sy < 0 ||
                sx >= src_stride) continue;
            pixel = src[sy * src_stride + sx];
            if (transparent_color >= 0 && pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] =
                dm2_v1_material_palette_color(s, pixel, consumed_count);
        }
    }
}

/* skproject/SKWIN/SkWinCore.cpp DRAW_DOOR_FRAMES binds the active
 * dtPalette16 before it presents GRAPHICSSET door frames. The source-required
 * path must consume that exact indexed view, never the viewport's mutable
 * palette state. Preflight every U4 index before touching the destination. */
static int dm2_v1_blit_door_presentation_command(
    uint8_t *dst,
    int dst_stride,
    const DM2_V1_ViewportDoorPresentationCommand *command,
    int *consumed_count)
{
    const DM2_V1_OriginalDoorSurfaceBinding *binding;
    const DM2_V1_ViewportRect *src_rect;
    const DM2_V1_ViewportRect *dst_rect;

    if (!dst || !command || !command->valid || !command->u4_pixels ||
        !command->palette16 || dst_stride <= 0 ||
        command->palette_stride != 16 || command->format != DM2_IMG_FMT_U4 ||
        command->scene_colorkey >= 16u ||
        command->colorkey_palette_index !=
            command->palette16[command->scene_colorkey]) {
        return 0;
    }
    binding = &command->opening_frame.material;
    src_rect = &command->source_rect;
    dst_rect = &command->destination_rect;
    if (!binding->valid || binding->request.decoded_pixels != command->u4_pixels ||
        binding->request.palette16 != command->palette16 ||
        binding->request.format != DM2_IMG_FMT_U4 ||
        binding->request.stride < binding->request.width ||
        binding->request.scene_colorkey != command->scene_colorkey ||
        binding->palette_stride != command->palette_stride ||
        binding->colorkey_palette_index != command->colorkey_palette_index ||
        src_rect->x < 0 || src_rect->y < 0 || src_rect->w <= 0 ||
        src_rect->h <= 0 || src_rect->x + src_rect->w > binding->request.width ||
        src_rect->y + src_rect->h > binding->request.height ||
        dst_rect->w <= 0 || dst_rect->h <= 0 ||
        src_rect->x != binding->blit.src_rect.x ||
        src_rect->y != binding->blit.src_rect.y ||
        src_rect->w != binding->blit.src_rect.w ||
        src_rect->h != binding->blit.src_rect.h ||
        dst_rect->x != binding->blit.dst_rect.x ||
        dst_rect->y != binding->blit.dst_rect.y ||
        dst_rect->w != binding->blit.dst_rect.w ||
        dst_rect->h != binding->blit.dst_rect.h ||
        binding->blit.transparent_color != (int)command->scene_colorkey) {
        return 0;
    }
    for (int y = 0; y < src_rect->h; ++y) {
        for (int x = 0; x < src_rect->w; ++x) {
            if (command->u4_pixels[(src_rect->y + y) *
                                       binding->request.stride +
                                   src_rect->x + x] >= 16u) {
                return 0;
            }
        }
    }
    for (int y = 0; y < dst_rect->h; ++y) {
        int fy = dst_rect->y + y;
        int sy = src_rect->y + (y * src_rect->h) / dst_rect->h;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < dst_rect->w; ++x) {
            int fx = dst_rect->x + x;
            int sx = src_rect->x + (x * src_rect->w) / dst_rect->w;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            pixel = command->u4_pixels[sy * binding->request.stride + sx];
            if (pixel == command->scene_colorkey) continue;
            dst[fy * dst_stride + fx] = command->palette16[pixel];
            if (consumed_count) ++*consumed_count;
        }
    }
    return 1;
}

/* skproject/SKWIN/SkWinCore.cpp routes map-chip and HUD sprites through the
 * same dtPalIRGB/dtPalette16 binding as dungeon materials. Keep this as a
 * distinct primitive so ordinary fallback pixels cannot be counted as GDAT
 * palette consumption. */
static void dm2_v1_blit_scaled_material_bitmap_region_ex(
    DM2_V1_ViewportState *s, uint8_t *dst, int dst_stride, int dst_x,
    int dst_y, int dst_w, int dst_h, const uint8_t *src, int src_x,
    int src_y, int src_w, int src_h, int src_stride, int transparent_color,
    int flip_mirror, int *consumed_count)
{
    if (!s || !dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride <= 0) return;
    for (int y = 0; y < dst_h; ++y) {
        int fy = dst_y + y;
        int sy = src_y + ((flip_mirror & 2)
            ? src_h - 1 - ((y * src_h) / dst_h) : (y * src_h) / dst_h);
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < dst_w; ++x) {
            int rx = (x * src_w) / dst_w;
            int sx = src_x + ((flip_mirror & 1) ? src_w - 1 - rx : rx);
            int fx = dst_x + x;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH || sx < 0 || sy < 0 ||
                sx >= src_stride) continue;
            pixel = src[sy * src_stride + sx];
            if (transparent_color >= 0 && pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] =
                dm2_v1_material_palette_color(s, pixel, consumed_count);
        }
    }
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
        row->source_kind = src->source_kind;
        row->frame_index = src->frame_index;
        row->direction = src->direction;
        row->depth = src->depth;
        row->center_x = src->screen_x;
        row->center_y = src->screen_y;
        row->gdat_index = dm2_v1_viewport_creature_graphic_index(
            src->creature_type,
            src->frame_index);
        if (s->gdat_interface_rect14_rows &&
            src->frame_index < s->gdat_interface_rect14_row_count) {
            const uint8_t *rect14 = s->gdat_interface_rect14_rows +
                ((size_t)src->frame_index * 14u);
            int relative_direction = (s->party_dir - src->direction) & 3;
            uint8_t image_field = rect14[2 + relative_direction];

            /* skproject SkWinCore.cpp QUERY_CREATURE_PICST (32CB:28C7)
             * selects the creature dtImage field from this row. */
            if (rect14[0] <= 24u && image_field != 0xffu) {
                row->gdat_index = dm2_v1_viewport_creature_field_graphic_index(
                    src->creature_type, image_field);
                row->rect14_applied = 1;
                row->rect14_scale64 = rect14[6 + relative_direction];
                row->rect14_lateral_offset = (int8_t)rect14[1];
                row->rect14_flip_mirror = rect14[10 + relative_direction] & 1u;
            }
        }
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
    if (render->rect14_applied) {
        int scale64 = render->rect14_scale64;
        if (scale64 <= 0) scale64 = 64;
        frame_count = 1;
        dst_w = dm2_v1_viewport_calc_stretched_size(src_w, scale64);
        dst_h = dm2_v1_viewport_calc_stretched_size(src_h, scale64);
    } else {
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
    }

    blit.gdat_index = render->gdat_index;
    blit.frame_x = frame_x;
    blit.frame_y = 0;
    blit.frame_w = frame_w;
    blit.frame_h = src_h;
    blit.dst_rect.x = render->center_x - (dst_w / 2);
    blit.dst_rect.y = render->center_y - (dst_h / 2);
    if (render->rect14_applied && render->rect14_lateral_offset != 0) {
        int offset = render->rect14_lateral_offset;
        int relative_direction = (party_direction - render->direction) & 3;
        if (relative_direction == 0) {
            blit.dst_rect.x += dm2_v1_viewport_calc_stretched_size(-7, offset);
        } else if (relative_direction == 2) {
            blit.dst_rect.x += dm2_v1_viewport_calc_stretched_size(7, offset);
        } else {
            blit.dst_rect.y += dm2_v1_viewport_calc_stretched_size(-64, offset);
        }
    }
    blit.dst_rect.w = dst_w;
    blit.dst_rect.h = dst_h;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    blit.flip_mirror = render->rect14_applied ? render->rect14_flip_mirror : 0;
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

    /* skproject SKWIN outdoor weather resolves overlay density and animated
     * scroll from the weather/tick state before the blitline_48-style pass.
     * Keep those render decisions in a DM2-owned plan; the pass below only
     * applies the already-bound overlay command. */
    if (s->weather == DM2_V1_WEATHER_OVERLAY_RAIN) {
        out_plan->density = (s->rain_intensity + 9) / 10;
        stride2 = s->rain_intensity / 5;
        out_plan->scroll = (s->tick_count * stride2) & 7;
    } else if (s->weather == DM2_V1_WEATHER_OVERLAY_FOG) {
        out_plan->alpha = (s->rain_intensity + 7) / 8;
    } else if (s->weather == DM2_V1_WEATHER_OVERLAY_STORM) {
        out_plan->density = (s->rain_intensity + 5) / 10;
        stride2 = s->rain_intensity / 4;
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

static int dm2_v1_viewport_ceiling_material_owned(
    const DM2_V1_ViewportState *s)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;

    if (!s || !s->source_materials_required ||
        !s->gdat_scene_control_ready || s->gdat_scene_map_load_token == 0u ||
        s->gdat_scene_control_hash == 0u ||
        !s->gdat_interface_palette_ready ||
        s->gdat_interface_palette_hash == 0u ||
        !s->gdat_static_scene_ceiling_material_owned ||
        s->gdat_static_scene_ceiling_material_map_load_token !=
            s->gdat_scene_map_load_token ||
        s->gdat_static_scene_ceiling_material_scene_control_hash !=
            s->gdat_scene_control_hash || !s->gdat_static_light_control_owned ||
        s->gdat_static_light_map_load_token != s->gdat_scene_map_load_token ||
        s->gdat_static_light_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    scene = &s->gdat_static_scene_record;
    return scene->valid && scene->map_load_token == s->gdat_scene_map_load_token &&
        scene->scene_control_hash == s->gdat_scene_control_hash &&
        scene->graphicsset == (uint8_t)s->gdat_scene_material_index &&
        scene->material_category == DM2_GDAT_CATEGORY_GRAPHICSSET &&
        scene->ceiling_field == DM2_GDAT_GFXSET_CEIL;
}

static int dm2_v1_viewport_floor_material_owned(
    const DM2_V1_ViewportState *s)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;

    if (!s || !s->source_materials_required ||
        !s->gdat_scene_control_ready || s->gdat_scene_map_load_token == 0u ||
        s->gdat_scene_control_hash == 0u ||
        !s->gdat_interface_palette_ready ||
        s->gdat_interface_palette_hash == 0u ||
        !s->gdat_static_scene_floor_material_owned ||
        s->gdat_static_scene_floor_material_map_load_token !=
            s->gdat_scene_map_load_token ||
        s->gdat_static_scene_floor_material_scene_control_hash !=
            s->gdat_scene_control_hash || !s->gdat_static_light_control_owned ||
        s->gdat_static_light_map_load_token != s->gdat_scene_map_load_token ||
        s->gdat_static_light_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    scene = &s->gdat_static_scene_record;
    return scene->valid && scene->map_load_token == s->gdat_scene_map_load_token &&
        scene->scene_control_hash == s->gdat_scene_control_hash &&
        scene->graphicsset == (uint8_t)s->gdat_scene_material_index &&
        scene->material_category == DM2_GDAT_CATEGORY_GRAPHICSSET &&
        scene->floor_field == DM2_GDAT_GFXSET_FLOOR;
}

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
    int ceiling_gdat_index = dm2_v1_viewport_scene_material_graphic_index(
        s->gdat_scene_material_index,
        DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
    int floor_gdat_index = dm2_v1_viewport_scene_material_graphic_index(
        s->gdat_scene_material_index,
        DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);
    int ceiling_source_owned = !s->source_materials_required ||
        dm2_v1_viewport_ceiling_material_owned(s);
    int ceiling_asset = ceiling_source_owned &&
        dm2_v1_fetch_viewport_asset(s,
                                    ceiling_gdat_index,
                                    &ceiling_pixels,
                                    &ceiling_w,
                                    &ceiling_h_src,
                                    &ceiling_stride) == 0 &&
        ceiling_pixels && ceiling_w > 0 && ceiling_h_src > 0;
    int floor_source_owned = !s->source_materials_required ||
        dm2_v1_viewport_floor_material_owned(s);
    int floor_asset = floor_source_owned &&
        dm2_v1_fetch_viewport_asset(s,
                                    floor_gdat_index,
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
    if (ceiling_asset) {
        dm2_v1_blit_tiled_material_bitmap(s,
                                 vp,
                                 stride,
                                 0,
                                 0,
                                 DM2_VP_WIDTH,
                                 ceiling_h,
                                 ceiling_pixels,
                                 ceiling_w,
                                 ceiling_h_src,
                                 ceiling_stride > 0 ? ceiling_stride : ceiling_w,
                                 -1,
                                 &s->gdat_material_palette_floor_ceiling_consumed_count);
        ++s->asset_floor_ceiling_drawn_count;
        ++s->gdat_scene_material_consumed_count;
        if (s->source_materials_required) {
            DM2_V1_ViewportDungeonMaterialCommand command;

            memset(&command, 0, sizeof(command));
            command.gdat_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
            command.graphicsset = (uint8_t)s->gdat_scene_material_index;
            command.field = DM2_GDAT_GFXSET_CEIL;
            command.gdat_index = ceiling_gdat_index;
            command.map_load_token = s->gdat_scene_map_load_token;
            command.scene_control_hash = s->gdat_scene_control_hash;
            command.palette_hash = s->gdat_interface_palette_hash;
            command.indexed_pixels = ceiling_pixels;
            command.palette16 = s->gdat_interface_palette16;
            command.palette_stride = 16;
            command.width = ceiling_w;
            command.height = ceiling_h_src;
            command.stride = ceiling_stride > 0 ? ceiling_stride : ceiling_w;
            command.transparent_color = -1;
            command.source_rect = (DM2_V1_ViewportRect){
                0, 0, ceiling_w, ceiling_h_src };
            command.destination_rect = (DM2_V1_ViewportRect){
                0, 0, DM2_VP_WIDTH, ceiling_h };
            command.valid = command.indexed_pixels && command.palette16 &&
                command.palette_hash != 0u && command.palette_stride == 16 &&
                command.width > 0 && command.height > 0 &&
                command.stride >= command.width && ceiling_source_owned;
            if (command.valid) {
                s->last_dungeon_ceiling_presentation_command = command;
            }
        }
    } else {
        if (s->source_materials_required) {
            ++s->blocked_material_draw_count;
            s->blocked_material_mask |=
                DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING;
        } else {
            /* Ceiling region: dark gray (matches DM2 darker dungeon atmosphere)
             * Source: DUNVIEW.C:2996-3015 (PC34 ceiling blit path) */
            for (int y = 0; y < ceiling_h; y++) {
                /* DM2 ceiling is slightly darker than DM1 (gray-8 vs gray-9) */
                memset(vp + y * stride, DM2_COL_DKGRAY, (size_t)DM2_VP_WIDTH);
            }
            ++s->fallback_floor_ceiling_drawn_count;
        }
    }

    int floor_y = DM2_FLOOR_Y;
    int floor_h = DM2_FLOOR_H;
    if (floor_asset) {
        dm2_v1_blit_tiled_material_bitmap(s,
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
                                 &s->gdat_material_palette_floor_ceiling_consumed_count);
        ++s->asset_floor_ceiling_drawn_count;
        ++s->gdat_scene_material_consumed_count;
        if (s->source_materials_required) {
            DM2_V1_ViewportDungeonMaterialCommand command;

            memset(&command, 0, sizeof(command));
            command.gdat_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
            command.graphicsset = (uint8_t)s->gdat_scene_material_index;
            command.field = DM2_GDAT_GFXSET_FLOOR;
            command.gdat_index = floor_gdat_index;
            command.map_load_token = s->gdat_scene_map_load_token;
            command.scene_control_hash = s->gdat_scene_control_hash;
            command.palette_hash = s->gdat_interface_palette_hash;
            command.indexed_pixels = floor_pixels;
            command.palette16 = s->gdat_interface_palette16;
            command.palette_stride = 16;
            command.width = floor_w;
            command.height = floor_h_src;
            command.stride = floor_stride > 0 ? floor_stride : floor_w;
            command.transparent_color = -1;
            command.source_rect = (DM2_V1_ViewportRect){
                0, 0, floor_w, floor_h_src };
            command.destination_rect = (DM2_V1_ViewportRect){
                0, floor_y, DM2_VP_WIDTH, floor_h };
            command.valid = command.indexed_pixels && command.palette16 &&
                command.palette_hash != 0u && command.palette_stride == 16 &&
                command.width > 0 && command.height > 0 &&
                command.stride >= command.width && floor_source_owned;
            if (command.valid) {
                s->last_dungeon_floor_presentation_command = command;
            }
        }
    } else {
        if (s->source_materials_required) {
            ++s->blocked_material_draw_count;
            s->blocked_material_mask |=
                DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING;
        } else {
            /* Floor region: brown (matches DM2 floor color)
             * Source: DUNVIEW.C:3016-3047 (PC34 floor blit path) */
            for (int y = floor_y; y < floor_y + floor_h; y++) {
                if (y < DM2_VP_HEIGHT) {
                    memset(vp + y * stride, 5, (size_t)DM2_VP_WIDTH);  /* brown */
                }
            }
            ++s->fallback_floor_ceiling_drawn_count;
        }
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

static void dm2_v1_block_source_material(DM2_V1_ViewportState *s,
                                         uint32_t material_mask)
{
    if (!s || !s->source_materials_required) {
        return;
    }
    ++s->blocked_material_draw_count;
    s->blocked_material_mask |= material_mask;
}

static int dm2_v1_viewport_wall_material_owned(
    const DM2_V1_ViewportState *s)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;
    int expected_field;

    if (!s || !s->source_materials_required ||
        !s->gdat_scene_control_ready || s->gdat_scene_map_load_token == 0u ||
        s->gdat_scene_control_hash == 0u ||
        !s->gdat_interface_palette_ready ||
        s->gdat_interface_palette_hash == 0u ||
        s->gdat_scene_colorkey > 15u ||
        !s->gdat_static_scene_wall_material_owned ||
        s->gdat_static_scene_wall_material_map_load_token !=
            s->gdat_scene_map_load_token ||
        s->gdat_static_scene_wall_material_scene_control_hash !=
            s->gdat_scene_control_hash || !s->gdat_static_light_control_owned ||
        s->gdat_static_light_map_load_token != s->gdat_scene_map_load_token ||
        s->gdat_static_light_scene_control_hash != s->gdat_scene_control_hash) {
        return 0;
    }
    expected_field = dm2_v1_viewport_wall_field_for_square(
        s->gdat_static_scene_wall_material_view_square);
    scene = &s->gdat_static_scene_record;
    return expected_field >= 0 &&
        expected_field == s->gdat_static_scene_wall_material_field &&
        scene->valid && scene->map_load_token == s->gdat_scene_map_load_token &&
        scene->scene_control_hash == s->gdat_scene_control_hash &&
        scene->graphicsset == (uint8_t)s->gdat_scene_material_index &&
        scene->material_category == DM2_GDAT_CATEGORY_GRAPHICSSET;
}

static int dm2_v1_viewport_wall_material_owned_for_square(
    const DM2_V1_ViewportState *s,
    int view_square)
{
    int expected_field;

    if (view_square < 0 || view_square >= DM2_SQ_COUNT ||
        !dm2_v1_viewport_wall_material_owned(s)) {
        return 0;
    }
    expected_field = dm2_v1_viewport_wall_field_for_square(view_square);
    return expected_field >= 0 &&
        (s->gdat_static_scene_wall_material_mask &
         (uint16_t)(1u << (unsigned)view_square)) != 0u;
}

/* skproject/SKWIN/SkWinCore.cpp DRAW_DOOR_FRAMES 46277-46334 reads the
 * current GRAPHICSSET plus glbSceneColorKey before it draws either side of a
 * door frame. Keep every gate visible to the production caller: the boot
 * provider has already established U4 format facts when its fetch succeeds. */
static void dm2_v1_source_door_frame_gate_receipt(
    const DM2_V1_ViewportState *s,
    const DM2_V1_DoorRender *door,
    DM2_V1_OriginalMaterialGateReceipt *out_receipt)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;
    int expected_index = 0;

    if (!out_receipt) return;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!s || !door) return;
    out_receipt->valid = 1;
    out_receipt->original_material_required = s->source_materials_required;
    out_receipt->gdat_index = door->frame_gdat_index;
    out_receipt->map_load_token = s->gdat_scene_map_load_token;
    out_receipt->scene_control_hash = s->gdat_scene_control_hash;
    out_receipt->scene_colorkey = s->gdat_scene_colorkey;
    out_receipt->palette_ready = s->gdat_interface_palette_ready;
    out_receipt->colorkey_ready = s->gdat_scene_colorkey <= 15u;
    scene = &s->gdat_static_scene_record;
    if (s->gdat_scene_control_ready && s->gdat_scene_control_hash != 0u &&
        s->gdat_scene_map_load_token != 0u && scene->valid &&
        scene->map_load_token == s->gdat_scene_map_load_token &&
        scene->scene_control_hash == s->gdat_scene_control_hash &&
        scene->graphicsset == (uint8_t)s->gdat_scene_material_index &&
        scene->material_category == DM2_GDAT_CATEGORY_GRAPHICSSET) {
        out_receipt->scene_record_ready = 1;
        expected_index =
            dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
                scene->graphicsset, door->view_square);
        out_receipt->active_graphicsset_frame =
            expected_index != 0 && door->frame_gdat_index == expected_index;
    }
}

static int dm2_v1_wall_button_receipt_matches(
    const DM2_V1_ViewportState *s,
    const DM2_V1_DoorRender *door)
{
    int i;

    if (!s || !door || door->button_source_kind != 2 ||
        door->wall_button_field != 1) {
        return 0;
    }
    if (s->g1_text_wall_gfx_materials &&
        s->g1_text_wall_gfx_materials->map == s->dungeon_level) {
        const DM2_V1_G1TextWallGfxRuntimeReceipt *receipt =
            s->g1_text_wall_gfx_materials;
        for (i = 0; i < receipt->material_count; ++i) {
            const DM2_V1_G1TextWallGfxMaterial *material =
                &receipt->materials[i];
            if (material->x == door->wall_button_x &&
                material->y == door->wall_button_y &&
                material->object_id == door->wall_button_object_id &&
                material->wall_gfx_index == (uint8_t)door->wall_button_index) {
                return 1;
            }
        }
    }
    if (s->g1_actuator_wall_gfx_materials &&
        s->g1_actuator_wall_gfx_materials->map == s->dungeon_level) {
        const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *receipt =
            s->g1_actuator_wall_gfx_materials;
        for (i = 0; i < receipt->material_count; ++i) {
            const DM2_V1_G1ActuatorWallGfxMaterial *material =
                &receipt->materials[i];
            if (material->x == door->wall_button_x &&
                material->y == door->wall_button_y &&
                material->object_id == door->wall_button_object_id &&
                material->wall_gfx_index == (uint8_t)door->wall_button_index) {
                return 1;
            }
        }
    }
    return 0;
}

void dm2_v1_render_walls(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    int wall_asset_count = 0;
    int wall_fallback_count = 0;
    DM2_V1_WallPanelRenderPlan plan;

    memset(&s->last_dungeon_wall_presentation_command, 0,
           sizeof(s->last_dungeon_wall_presentation_command));
    s->last_dungeon_wall_material_required_mask = 0u;
    s->last_dungeon_wall_material_consumed_mask = 0u;

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
        if (s->source_materials_required) {
            ++s->blocked_material_draw_count;
            s->blocked_material_mask |= DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL;
            return;
        }
        dm2_v1_draw_legacy_wall_fallback(vp, stride);
        ++s->fallback_wall_drawn_count;
        return;
    }

    if (!dm2_v1_viewport_build_wall_panel_render_plan(s, &plan)) {
        return;
    }

    for (int i = 0; i < plan.panel_count; ++i) {
        const int view_square = plan.panels[i].view_square;

        s->last_dungeon_wall_material_required_mask |=
            (uint16_t)(1u << (unsigned)view_square);
        /* skproject DRAW_WALL queries GRAPHICSSET with the live
         * MapGraphicsStyle.  The default set is only a data-free renderer
         * convenience; it must not substitute for any missing source-owned
         * panel in a source-required frame. */
        if (s->source_materials_required &&
            !dm2_v1_viewport_wall_material_owned_for_square(s, view_square)) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
            return;
        }
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
            if (s->source_materials_required) {
                ++s->blocked_material_draw_count;
                s->blocked_material_mask |=
                    DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL;
            } else {
                dm2_v1_draw_wall_fallback_rect(vp,
                                               stride,
                                               dm2_v1_get_wall_frame(
                                                   panel->view_square),
                                               panel->fallback_color);
                ++wall_fallback_count;
            }
            continue;
        }

        dm2_v1_blit_scaled_material_bitmap(s,
                                  vp,
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
                                      : DM2_COLOR_TRANSPARENT,
                                  &s->gdat_material_palette_wall_consumed_count);
        if (s->gdat_scene_control_ready) {
            ++s->gdat_scene_control_consumed_count;
        }
        if (s->source_materials_required) {
            int graphicsset_index = 0;
            int field = 0;
            DM2_V1_ViewportDungeonMaterialCommand command;

            if (!dm2_v1_viewport_wall_graphic_address(
                    panel->gdat_index, &graphicsset_index, &field) ||
                graphicsset_index != s->gdat_scene_material_index ||
                field != dm2_v1_viewport_wall_field_for_square(
                    panel->view_square)) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
                continue;
            }
            memset(&command, 0, sizeof(command));
            command.gdat_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
            command.graphicsset = (uint8_t)graphicsset_index;
            command.field = (uint8_t)field;
            command.gdat_index = panel->gdat_index;
            command.map_load_token = s->gdat_scene_map_load_token;
            command.scene_control_hash = s->gdat_scene_control_hash;
            command.palette_hash = s->gdat_interface_palette_hash;
            command.indexed_pixels = wall_pixels;
            command.palette16 = s->gdat_interface_palette16;
            command.palette_stride = 16;
            command.width = wall_w;
            command.height = wall_h;
            command.stride = wall_stride > 0 ? wall_stride : wall_w;
            command.transparent_color = (int)s->gdat_scene_colorkey;
            command.colorkey_palette_index =
                command.palette16[command.transparent_color];
            command.source_rect = (DM2_V1_ViewportRect){ 0, 0, wall_w, wall_h };
            command.destination_rect = panel->dst_rect;
            command.valid = command.indexed_pixels && command.palette16 &&
                command.palette_hash != 0u && command.palette_stride == 16 &&
                command.width > 0 && command.height > 0 &&
                command.stride >= command.width &&
                command.transparent_color >= 0 &&
                command.transparent_color < command.palette_stride &&
                command.colorkey_palette_index ==
                    command.palette16[command.transparent_color];
            if (command.valid) {
                s->last_dungeon_wall_presentation_command = command;
                s->last_dungeon_wall_material_consumed_mask |=
                    (uint16_t)(1u << (unsigned)panel->view_square);
            }
        }
        ++wall_asset_count;
    }

    if (s->source_materials_required &&
        s->last_dungeon_wall_material_consumed_mask !=
            s->last_dungeon_wall_material_required_mask) {
        memset(&s->last_dungeon_wall_presentation_command, 0,
               sizeof(s->last_dungeon_wall_presentation_command));
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
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
        int ornate_drawn_asset = 0;
        int destroyed_mask_drawn_asset = 0;
        int frame_drawn_asset = 0;
        int button_drawn_asset = 0;

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
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
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
                        blit.transparent_color,
                        0,
                        &s->gdat_sprite_palette_consumed_count);
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
                if (s->source_materials_required) {
                    /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR resolves the
                     * selected DOORS image before painting the panel. */
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                } else {
                    dm2_v1_draw_door_panel_fallback_rect(
                        vp, stride, door->view_square,
                        &door->panel_visible_rect, door->fallback_color);
                    ++door_fallback_count;
                }
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
                        /* skproject/SKULLWIN/c_gui_vp.cpp DM2_DRAW_DOOR
                         * draws ornate and destroyed-door overlays through
                         * the active GDAT palette just like the base door
                         * panel.  Keeping these pixels on the raw blitter
                         * made real logical indices bypass dtPalette16. */
                        dm2_v1_blit_scaled_material_bitmap_region(
                            s, vp,
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
                            blit.transparent_color,
                            &s->gdat_sprite_palette_consumed_count);
                        ++door_overlay_asset_count;
                        if (overlay_i == 0) {
                            ornate_drawn_asset = 1;
                            s->last_door_ornate_asset_blit_valid = 1;
                            s->last_door_ornate_asset_blit = blit;
                            s->last_door_ornate_asset_src_w = overlay_w;
                            s->last_door_ornate_asset_src_h = overlay_h;
                            s->last_door_ornate_asset_src_stride =
                                overlay_stride > 0 ? overlay_stride :
                                                     overlay_w;
                        } else {
                            destroyed_mask_drawn_asset = 1;
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
                if (overlay_indices[overlay_i] != 0 &&
                    s->source_materials_required &&
                    ((overlay_i == 0 &&
                      !ornate_drawn_asset) ||
                     (overlay_i == 1 &&
                      !destroyed_mask_drawn_asset))) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                }
            }
        }
        if (door->frame_rect.w > 0 && door->frame_rect.h > 0) {
            const uint8_t *door_pixels = NULL;
            int door_w = 0;
            int door_h = 0;
            int door_stride = 0;
            DM2_V1_OriginalMaterialGateReceipt gate;
            DM2_V1_BootViewportSurfaceView surface_view;
            DM2_V1_OriginalDoorSurfaceRequest request;
            DM2_V1_OriginalDoorSurfaceBinding binding;
            DM2_V1_OriginalDoorOpeningFrameRequest opening_frame_request;
            DM2_V1_ViewportDoorPresentationCommand presentation_command;
            int source_frame_facts;
            int surface_view_ready = 0;
            int binding_ready = 0;
            int presentation_command_ready = 0;

            dm2_v1_source_door_frame_gate_receipt(s, door, &gate);
            source_frame_facts = gate.scene_record_ready &&
                gate.palette_ready && gate.colorkey_ready &&
                gate.active_graphicsset_frame;

            if ((!s->source_materials_required || source_frame_facts) &&
                door->frame_gdat_index != 0 &&
                dm2_v1_fetch_viewport_asset(s,
                                            door->frame_gdat_index,
                                            &door_pixels,
                                            &door_w,
                                            &door_h,
                                            &door_stride) == 0 &&
                door_pixels && door_w > 0 && door_h > 0) {
                DM2_V1_DoorAssetBlit blit;
                memset(&surface_view, 0, sizeof(surface_view));
                if (!s->source_materials_required) {
                    surface_view_ready = 1;
                } else if (s->door_surface_view_fetch &&
                           s->door_surface_view_fetch(
                               s->door_surface_view_user,
                               door->frame_gdat_index,
                               &surface_view) &&
                           surface_view.pixels == door_pixels &&
                           surface_view.width == door_w &&
                           surface_view.height == door_h &&
                           surface_view.stride == door_stride &&
                           surface_view.format == DM2_IMG_FMT_U4) {
                    surface_view_ready = 1;
                }
                gate.decoded_format_gate_ready = surface_view_ready;
                /* skproject GRAPHICSSET fields 0x06/0x07/0x09 are the
                 * first boot-bound door-frame images for front, D1C and D2C.
                 * This pass scales them into the current bounded DM2 frame
                 * rectangles; exact DRAW_DUNGEON_GRAPHIC offsets remain open. */
                if (surface_view_ready &&
                    dm2_v1_viewport_door_frame_asset_blit(door,
                                                          door_w,
                                                          door_h,
                                                          door_stride,
                                                          &blit)) {
                    if (source_frame_facts) {
                        blit.transparent_color = (int)gate.scene_colorkey;
                    }
                    if (!s->source_materials_required) {
                        binding_ready = 1;
                    } else if (source_frame_facts) {
                        /* skproject/SKWIN/SkWinCore.cpp DRAW_DOOR_FRAMES
                         * lines 46311-46334 consumes a GRAPHICSSET U4 image
                         * with the active interface palette and transparency
                         * index. Keep that borrowed GDAT view whole here. */
                        memset(&request, 0, sizeof(request));
                        request.valid = 1;
                        request.gdat_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
                        request.graphicsset =
                            (uint8_t)s->gdat_scene_material_index;
                        request.field = (uint8_t)
                            dm2_v1_viewport_door_frame_field_for_square(
                                door->view_square);
                        request.gdat_index = door->frame_gdat_index;
                        request.map_load_token = gate.map_load_token;
                        request.scene_control_hash = gate.scene_control_hash;
                        request.palette_hash = s->gdat_interface_palette_hash;
                        request.scene_colorkey = gate.scene_colorkey;
                        request.decoded_pixels = surface_view.pixels;
                        request.palette16 = s->gdat_interface_palette16;
                        request.width = surface_view.width;
                        request.height = surface_view.height;
                        request.stride = surface_view.stride;
                        request.format = surface_view.format;
                        memset(&binding, 0, sizeof(binding));
                        binding.request = request;
                        binding.blit = blit;
                        binding.palette_stride = 16;
                        binding.colorkey_palette_index =
                            request.palette16[request.scene_colorkey];
                        binding.valid = request.palette_hash != 0u &&
                            request.scene_colorkey < 16u &&
                            request.format == DM2_IMG_FMT_U4 &&
                            request.stride >= request.width &&
                            blit.gdat_index == request.gdat_index &&
                            blit.src_rect.x == 0 && blit.src_rect.y == 0 &&
                            blit.src_rect.w == request.width &&
                            blit.src_rect.h == request.height &&
                            blit.src_stride == request.stride &&
                            blit.transparent_color ==
                                (int)request.scene_colorkey;
                        binding_ready = binding.valid;
                        memset(&opening_frame_request, 0,
                               sizeof(opening_frame_request));
                        opening_frame_request.view_square = door->view_square;
                        opening_frame_request.skproject_cell = door->skproject_cell;
                        opening_frame_request.door_state = door->door_state;
                        opening_frame_request.door_open_pct = door->door_open_pct;
                        opening_frame_request.door_opening_dir =
                            door->door_opening_dir;
                        opening_frame_request.opening_visible_rect =
                            door->panel_visible_rect;
                        opening_frame_request.frame_rect = door->frame_rect;
                        opening_frame_request.source_rect = binding.blit.src_rect;
                        opening_frame_request.destination_rect = binding.blit.dst_rect;
                        opening_frame_request.material = binding;
                        opening_frame_request.valid = binding.valid &&
                            opening_frame_request.frame_rect.x ==
                                opening_frame_request.destination_rect.x &&
                            opening_frame_request.frame_rect.y ==
                                opening_frame_request.destination_rect.y &&
                            opening_frame_request.frame_rect.w ==
                                opening_frame_request.destination_rect.w &&
                            opening_frame_request.frame_rect.h ==
                                opening_frame_request.destination_rect.h &&
                            opening_frame_request.source_rect.x == 0 &&
                            opening_frame_request.source_rect.y == 0 &&
                            opening_frame_request.source_rect.w == request.width &&
                            opening_frame_request.source_rect.h == request.height &&
                            opening_frame_request.door_open_pct <= 100u;
                        memset(&presentation_command, 0,
                               sizeof(presentation_command));
                        presentation_command.opening_frame = opening_frame_request;
                        presentation_command.u4_pixels = request.decoded_pixels;
                        presentation_command.palette16 = request.palette16;
                        presentation_command.palette_stride =
                            binding.palette_stride;
                        presentation_command.scene_colorkey =
                            (uint8_t)request.scene_colorkey;
                        presentation_command.colorkey_palette_index =
                            binding.colorkey_palette_index;
                        presentation_command.format = request.format;
                        presentation_command.source_rect =
                            opening_frame_request.source_rect;
                        presentation_command.destination_rect =
                            opening_frame_request.destination_rect;
                        presentation_command.valid = opening_frame_request.valid &&
                            presentation_command.u4_pixels &&
                            presentation_command.palette16 &&
                            presentation_command.palette_stride == 16 &&
                            presentation_command.scene_colorkey < 16u &&
                            presentation_command.colorkey_palette_index ==
                                presentation_command.palette16[
                                    presentation_command.scene_colorkey] &&
                            presentation_command.format == DM2_IMG_FMT_U4 &&
                            presentation_command.source_rect.w == request.width &&
                            presentation_command.source_rect.h == request.height &&
                            presentation_command.destination_rect.x ==
                                binding.blit.dst_rect.x &&
                            presentation_command.destination_rect.y ==
                                binding.blit.dst_rect.y &&
                            presentation_command.destination_rect.w ==
                                binding.blit.dst_rect.w &&
                            presentation_command.destination_rect.h ==
                                binding.blit.dst_rect.h;
                        presentation_command_ready = presentation_command.valid;
                    }
                    if (s->source_materials_required &&
                        !presentation_command_ready) {
                        binding_ready = 0;
                    }
                    if (binding_ready) {
                        const DM2_V1_ViewportDoorPresentationCommand *command =
                            s->source_materials_required
                                ? &presentation_command : NULL;
                        int presented = 0;
                        if (command) {
                            presented = dm2_v1_blit_door_presentation_command(
                                vp, stride, command,
                                &s->gdat_material_palette_door_frame_consumed_count);
                        } else {
                            dm2_v1_blit_scaled_material_bitmap_region(
                                s, vp,
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
                                blit.transparent_color,
                                &s->gdat_material_palette_door_frame_consumed_count);
                            presented = 1;
                        }
                        if (presented) {
                            ++door_asset_count;
                            s->last_door_frame_asset_blit_valid = 1;
                            s->last_door_frame_asset_blit = blit;
                            s->last_door_frame_asset_src_w = door_w;
                            s->last_door_frame_asset_src_h = door_h;
                            s->last_door_frame_asset_src_stride =
                                door_stride > 0 ? door_stride : door_w;
                            frame_drawn_asset = 1;
                        }
                    }
                }
            }
            gate.accepted = !s->source_materials_required ||
                (source_frame_facts && gate.decoded_format_gate_ready &&
                 frame_drawn_asset);
            s->last_original_material_gate = gate;
            if (s->source_materials_required && gate.accepted &&
                source_frame_facts && frame_drawn_asset) {
                DM2_V1_OriginalDoorSurfaceRequest *request =
                    &s->last_original_door_surface_request;
                DM2_V1_OriginalDoorOpeningFrameRequest *stored_opening_request =
                    &s->last_original_door_opening_frame_request;
                *request = binding.request;
                s->last_original_door_surface_binding = binding;
                *stored_opening_request = opening_frame_request;
                s->last_original_door_presentation_command =
                    presentation_command;
            }
            if (s->source_materials_required &&
                !frame_drawn_asset) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
            }
        }
        if (door->button_gdat_index != 0 &&
            door->button_rect.w > 0 && door->button_rect.h > 0) {
            const uint8_t *button_pixels = NULL;
            int button_w = 0;
            int button_h = 0;
            int button_stride = 0;
            int wall_button_material_bound =
                door->button_source_kind != 2 ||
                dm2_v1_wall_button_receipt_matches(s, door);

            /* skproject DRAW_DEFAULT_DOOR_BUTTON reaches the custom button
             * through the current WALL_GFX owner. Do not let the generic
             * view-square helper pick a same-numbered GDAT image unless the
             * direct DB2/DB3 receipt proves that ownership. */
            if ((!s->source_materials_required || wall_button_material_bound) &&
                dm2_v1_fetch_viewport_asset(s,
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
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
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
                        blit.transparent_color,
                        0,
                        &s->gdat_sprite_palette_consumed_count);
                    ++door_button_asset_count;
                    s->last_door_button_asset_blit_valid = 1;
                    s->last_door_button_asset_blit = blit;
                    s->last_door_button_asset_src_w = button_w;
                    s->last_door_button_asset_src_h = button_h;
                    s->last_door_button_asset_src_stride =
                        button_stride > 0 ? button_stride : button_w;
                    button_drawn_asset = 1;
                }
            }
            if (s->source_materials_required &&
                (!wall_button_material_bound || !button_drawn_asset)) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
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
                if (c->source_kind == 2 &&
                    s->g1_creature_map_chip_materials &&
                    !dm2_v1_g1_creature_map_chip_matches_decoded_material(
                        s->g1_creature_map_chip_materials,
                        c->creature_type, src_w, src_h)) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CREATURE);
                    continue;
                }
                if (dm2_v1_viewport_creature_asset_blit(c,
                                                        src_w,
                                                        src_h,
                                                        src_stride,
                                                        s->party_dir,
                                                        &blit)) {
                    drawn_h = blit.dst_rect.h;
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
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
                        blit.flip_mirror,
                        &s->gdat_sprite_palette_consumed_count);
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
                    if (c->rect14_applied) {
                        /* Count only a rendered source-selected row, not a
                         * parsed table or a speculative creature plan. */
                        ++s->gdat_interface_rect14_consumed_count;
                    }
                    if (s->source_materials_required) {
                        DM2_V1_ViewportCreatureMaterialCommand command;

                        memset(&command, 0, sizeof(command));
                        command.gdat_category = DM2_GDAT_CATEGORY_CREATURES;
                        command.creature_type = c->creature_type;
                        command.field = DM2_GDAT_IMG_MAP_CHIP;
                        command.gdat_index = c->gdat_index;
                        command.map_load_token = s->gdat_scene_map_load_token;
                        command.scene_control_hash = s->gdat_scene_control_hash;
                        command.palette_hash = s->gdat_interface_palette_hash;
                        command.indexed_pixels = pixels;
                        command.palette16 = s->gdat_interface_palette16;
                        command.palette_stride = 16;
                        command.width = src_w;
                        command.height = src_h;
                        command.stride = src_stride > 0 ? src_stride : src_w;
                        command.transparent_color = blit.transparent_color;
                        command.colorkey_palette_index =
                            command.transparent_color >= 0 &&
                            command.transparent_color < 16
                                ? command.palette16[command.transparent_color]
                                : 0u;
                        command.source_rect = (DM2_V1_ViewportRect){
                            blit.frame_x, blit.frame_y,
                            blit.frame_w, blit.frame_h };
                        command.destination_rect = blit.dst_rect;
                        command.valid = s->last_scene_control_presentation_command.valid &&
                            command.map_load_token != 0u &&
                            command.scene_control_hash != 0u &&
                            command.palette_hash != 0u && command.indexed_pixels &&
                            command.palette16 && command.palette_stride == 16 &&
                            command.width > 0 && command.height > 0 &&
                            command.stride >= command.width &&
                            command.transparent_color >= 0 &&
                            command.transparent_color < 16 &&
                            command.colorkey_palette_index ==
                                command.palette16[command.transparent_color] &&
                            command.source_rect.w > 0 && command.source_rect.h > 0 &&
                            command.source_rect.x + command.source_rect.w <=
                                command.width &&
                            command.source_rect.y + command.source_rect.h <=
                                command.height;
                        if (command.valid) {
                            s->last_creature_presentation_command = command;
                        } else {
                            dm2_v1_block_source_material(
                                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CREATURE);
                        }
                    }
                }
            }
        }
        if (!drawn_asset) {
            if (s->source_materials_required) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CREATURE);
            } else {
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
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
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
                        blit.flip_mirror,
                        &s->gdat_sprite_palette_consumed_count);
                    ++s->asset_item_drawn_count;
                    s->last_item_asset_blit_valid = 1;
                    s->last_item_asset_blit = blit;
                    s->last_item_asset_blit.draw_order = i;
                    s->last_item_asset_src_w = src_w;
                    s->last_item_asset_src_h = src_h;
                    s->last_item_asset_src_stride =
                        src_stride > 0 ? src_stride : src_w;
                    drawn_asset = 1;
                    if (s->source_materials_required) {
                        DM2_V1_ViewportItemMaterialCommand command;

                        /* skproject SkWinCore.cpp DRAW_ITEM routes a record's
                         * cls1/cls2 map chip through DRAW_CHIP_OF_MAGIC_MAP. */
                        memset(&command, 0, sizeof(command));
                        command.gdat_category = (uint8_t)it->item_category;
                        command.item_type = (uint8_t)it->item_type;
                        command.field = DM2_GDAT_IMG_MAP_CHIP;
                        command.gdat_index = it->gdat_index;
                        command.map_load_token = s->gdat_scene_map_load_token;
                        command.scene_control_hash = s->gdat_scene_control_hash;
                        command.palette_hash = s->gdat_interface_palette_hash;
                        command.indexed_pixels = pixels;
                        command.palette16 = s->gdat_interface_palette16;
                        command.palette_stride = 16;
                        command.width = src_w;
                        command.height = src_h;
                        command.stride = src_stride > 0 ? src_stride : src_w;
                        command.transparent_color = blit.transparent_color;
                        command.colorkey_palette_index =
                            command.transparent_color >= 0 &&
                            command.transparent_color < 16
                                ? command.palette16[command.transparent_color]
                                : 0u;
                        command.source_rect = (DM2_V1_ViewportRect){
                            blit.frame_x, blit.frame_y,
                            blit.frame_w, blit.frame_h };
                        command.destination_rect = blit.dst_rect;
                        command.valid =
                            s->last_scene_control_presentation_command.valid &&
                            command.map_load_token != 0u &&
                            command.scene_control_hash != 0u &&
                            command.palette_hash != 0u &&
                            command.indexed_pixels && command.palette16 &&
                            command.palette_stride == 16 && command.width > 0 &&
                            command.height > 0 &&
                            command.stride >= command.width &&
                            command.transparent_color >= 0 &&
                            command.transparent_color < 16 &&
                            command.colorkey_palette_index ==
                                command.palette16[command.transparent_color] &&
                            command.source_rect.w > 0 &&
                            command.source_rect.h > 0 &&
                            command.source_rect.x + command.source_rect.w <=
                                command.width &&
                            command.source_rect.y + command.source_rect.h <=
                                command.height;
                        if (command.valid) {
                            s->last_item_presentation_command = command;
                        } else {
                            dm2_v1_block_source_material(
                                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM);
                        }
                    }
                }
            }
        }
        if (!drawn_asset) {
            if (s->source_materials_required) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM);
            } else {
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
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
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
                        blit.flip_mirror,
                        &s->gdat_sprite_palette_consumed_count);
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
            if (s->source_materials_required) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_POSSESSION);
            } else {
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
                dm2_v1_blit_scaled_material_bitmap_region_ex(
                    s, vp,
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
                    blit.flip_mirror,
                    &s->gdat_sprite_palette_consumed_count);
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
        if (s->source_materials_required) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CARRIED_ITEM);
        } else {
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
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
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
                        blit.flip_mirror,
                        &s->gdat_sprite_palette_consumed_count);
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
            if (s->source_materials_required) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_PROJECTILE);
            } else {
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
}

/* ── Weather overlay ──────────────────────────────────────────────── */

void dm2_v1_render_weather_overlay(DM2_V1_ViewportState *s)
{
    /* skproject's weather pass is a source-material blitline route. The
     * GRAPHICSSET words only select/control it; they are not pixels. Until a
     * source-backed weather image address is proven, do not fabricate rain,
     * fog, or lightning over a real GDAT material frame. */
    (void)s;
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

static uint8_t dm2_v1_hud_palette_color(DM2_V1_ViewportState *s,
                                        uint8_t logical_color)
{
    if (!s || !s->gdat_interface_palette_ready || logical_color >= 16u) {
        return logical_color;
    }
    ++s->gdat_interface_palette_consumed_count;
    return s->gdat_interface_palette16[logical_color];
}

static int dm2_v1_blit_hud_presentation_command(
    uint8_t *dst,
    int dst_stride,
    const DM2_V1_ViewportHudPresentationCommand *command,
    int *consumed_count)
{
    const DM2_V1_ViewportHudMaterialRequest *material;

    if (!dst || !command || !command->valid || !command->indexed_pixels ||
        !command->palette16 || dst_stride <= 0) {
        return 0;
    }
    material = &command->material;
    if (!material->valid || material->indexed_pixels != command->indexed_pixels ||
        material->palette16 != command->palette16 ||
        material->palette_hash == 0u || material->palette_entry_count != 16 ||
        material->width <= 0 || material->height <= 0 ||
        material->stride < material->width ||
        material->transparent_color != command->transparent_color ||
        material->transparent_color < 0 || material->transparent_color > 255 ||
        material->colorkey_palette_index !=
            command->palette16[material->transparent_color] ||
        command->source_rect.x != material->source_rect.x ||
        command->source_rect.y != material->source_rect.y ||
        command->source_rect.w != material->source_rect.w ||
        command->source_rect.h != material->source_rect.h ||
        command->destination_rect.x != material->destination_rect.x ||
        command->destination_rect.y != material->destination_rect.y ||
        command->destination_rect.w != material->destination_rect.w ||
        command->destination_rect.h != material->destination_rect.h ||
        command->source_rect.x < 0 || command->source_rect.y < 0 ||
        command->source_rect.w <= 0 || command->source_rect.h <= 0 ||
        command->source_rect.x + command->source_rect.w > material->width ||
        command->source_rect.y + command->source_rect.h > material->height ||
        command->destination_rect.w <= 0 || command->destination_rect.h <= 0) {
        return 0;
    }
    for (int y = 0; y < command->destination_rect.h; ++y) {
        int fy = command->destination_rect.y + y;
        int sy = command->source_rect.y +
            (y * command->source_rect.h) / command->destination_rect.h;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < command->destination_rect.w; ++x) {
            int fx = command->destination_rect.x + x;
            int sx = command->source_rect.x +
                (x * command->source_rect.w) / command->destination_rect.w;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            pixel = command->indexed_pixels[sy * material->stride + sx];
            if (pixel == (uint8_t)command->transparent_color) continue;
            dst[fy * dst_stride + fx] = pixel < 16u
                ? command->palette16[pixel] : pixel;
            if (pixel < 16u && consumed_count) ++*consumed_count;
        }
    }
    return 1;
}

static int dm2_v1_render_hud_source_font(
    DM2_V1_ViewportState *s,
    const DM2_V1_ViewportRect *rect,
    const char *text,
    uint8_t foreground,
    uint8_t background)
{
    int glyph_count = 0;

    if (!s || !s->framebuffer || !rect || !text || !text[0] ||
        !s->gdat_interface_font_rows || s->gdat_interface_font_hash == 0u) {
        return 0;
    }
    /* skproject/SKWIN/SkWinCore.cpp QUERY_FONT expands each dt07/0 byte
     * into three pixels in the order 0x10, 0x04, 0x01 for six rows. */
    for (int glyph = 0; text[glyph] && glyph < DM2_V1_HUD_CHAMPION_NAME_MAX;
         ++glyph) {
        unsigned char character = (unsigned char)text[glyph];
        if (character >= 128u || rect->x + glyph * 3 >= rect->x + rect->w) {
            break;
        }
        for (int row = 0; row < 6; ++row) {
            uint8_t bits = s->gdat_interface_font_rows[row * 128 + character];
            for (int column = 0; column < 3; ++column) {
                DM2_V1_ViewportRect pixel = {
                    rect->x + glyph * 3 + column, rect->y + row, 1, 1
                };
                uint8_t color = (bits & (0x10u >> (column * 2)))
                    ? dm2_v1_hud_palette_color(s, foreground)
                    : dm2_v1_hud_palette_color(s, background);
                dm2_v1_fill_rect(s->framebuffer, s->fb_stride, &pixel, color);
            }
        }
        ++glyph_count;
    }
    s->gdat_interface_font_consumed_count += glyph_count;
    return glyph_count > 0;
}

static int dm2_v1_render_hud_core_asset(DM2_V1_ViewportState *s,
                                        const DM2_V1_ViewportRect *rect,
                                        int gdat_index)
{
    const uint8_t *pixels = NULL;
    int w = 0;
    int h = 0;
    int stride = 0;
    int hud_request_field = -1;
    if (!s || !s->framebuffer || !rect || rect->w <= 0 || rect->h <= 0 ||
        gdat_index == 0 ||
        (s->source_materials_required &&
         (!s->gdat_interface_palette_ready ||
          s->gdat_interface_palette_hash == 0u)) ||
        dm2_v1_fetch_viewport_asset(s,
                                    gdat_index,
                                    &pixels,
                                    &w,
                                    &h,
                                    &stride) != 0 ||
        !pixels || w <= 0 || h <= 0) {
        return 0;
    }
    if (s->source_materials_required &&
        gdat_index == dm2_v1_viewport_hud_core_graphic_index(
            DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR)) {
        hud_request_field = DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR;
    } else if (s->source_materials_required &&
               gdat_index == dm2_v1_viewport_hud_core_graphic_index(
                   DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL)) {
        hud_request_field = DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL;
    }
    if (hud_request_field >= 0) {
        DM2_V1_ViewportHudMaterialRequest request;
        DM2_V1_ViewportHudPresentationCommand command;

        memset(&request, 0, sizeof(request));
        request.gdat_index = gdat_index;
        request.field = (uint8_t)hud_request_field;
        request.indexed_pixels = pixels;
        request.palette16 = s->gdat_interface_palette16;
        request.palette_hash = s->gdat_interface_palette_hash;
        request.palette_entry_count = 16;
        request.width = w;
        request.height = h;
        request.stride = stride > 0 ? stride : w;
        request.transparent_color = DM2_COLOR_TRANSPARENT;
        request.colorkey_palette_index =
            request.palette16[request.transparent_color];
        request.source_rect = (DM2_V1_ViewportRect){ 0, 0, w, h };
        request.destination_rect = *rect;
        request.valid = request.palette_hash != 0u && request.width > 0 &&
            request.height > 0 && request.stride >= request.width;
        memset(&command, 0, sizeof(command));
        command.material = request;
        command.indexed_pixels = request.indexed_pixels;
        command.palette16 = request.palette16;
        command.transparent_color = request.transparent_color;
        command.source_rect = request.source_rect;
        command.destination_rect = request.destination_rect;
        command.valid = request.valid && command.indexed_pixels &&
            command.palette16 && command.transparent_color >= 0 &&
            command.transparent_color < request.palette_entry_count &&
            request.colorkey_palette_index ==
                command.palette16[command.transparent_color];
        if (!dm2_v1_blit_hud_presentation_command(
                s->framebuffer, s->fb_stride, &command,
                &s->gdat_interface_palette_consumed_count)) {
            return 0;
        }
        if (hud_request_field == DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR) {
            s->last_hud_top_bar_material_request = request;
            s->last_hud_top_bar_presentation_command = command;
        } else {
            /* skproject DRAW_CHAMPION_PICTURE enters through the static
             * INTERFACE_GENERAL status-panel before champion imagery. */
            s->last_hud_status_panel_material_request = request;
            s->last_hud_status_panel_presentation_command = command;
        }
    } else {
        dm2_v1_blit_scaled_material_bitmap(s,
                                  s->framebuffer,
                                  s->fb_stride,
                                  rect->x,
                                  rect->y,
                                  rect->w,
                                  rect->h,
                                  pixels,
                                  w,
                                  h,
                                  stride > 0 ? stride : w,
                                  DM2_COLOR_TRANSPARENT,
                                  /* skproject LOAD_GDAT_INTERFACE_00_02
                                   * initializes the INTERFACE_GENERAL palette
                                   * used by these chrome images.  Keep its
                                   * consumption distinct from map-chip sprite
                                   * palettes in the frame ownership receipt. */
                                  &s->gdat_interface_palette_consumed_count);
    }
    /* skproject binds dtPalIRGB/dtPalette16 before selecting the chrome
     * bitmap.  Count that source palette binding even if this particular
     * image contains no logical index below 16 (where the pixel loop above
     * would otherwise have no individual remap to count). */
    if (s->gdat_interface_palette_ready) {
        ++s->gdat_interface_palette_consumed_count;
    }
    ++s->asset_hud_core_drawn_count;
    s->last_hud_core_gdat_hash =
        dm2_v1_viewport_hash_gdat_asset(s->last_hud_core_gdat_hash,
                                        gdat_index,
                                        w,
                                        h);
    s->last_hud_core_pixel_count += (uint32_t)(rect->w * rect->h);
    return 1;
}

/* ReDMCSB/skproject SKWINSPX/src/v4/skguidrw.cpp DRAW_HAND_ACTION_ICONS
 * (0x29EE:026C) selects INTERFACE_GENERAL/4 entry
 * (possession<<1)+side+2 and expands the matching 0x46/0x4a rectangle
 * before DRAW_ICON_PICT_ENTRY. The source receipt supplies that rectangle;
 * do not replace it with Firestaff strip geometry. */
static int dm2_v1_render_hud_hand_action_asset(DM2_V1_ViewportState *s)
{
    const DM2_V1_HudHandActionSource *source;
    DM2_V1_ViewportHudMaterialRequest request;
    DM2_V1_ViewportHudPresentationCommand command;
    const uint8_t *pixels = NULL;
    int width = 0;
    int height = 0;
    int stride = 0;
    int gdat_index;

    if (!s || !s->framebuffer) return 0;
    source = &s->hud_hand_action_source;
    if (!source->valid || !s->gdat_interface_palette_ready ||
        s->gdat_interface_palette_hash == 0u || !s->hud_party_valid ||
        source->player_index >= (uint8_t)s->hud_party.champion_count ||
        !s->hud_party.champions[source->player_index].occupied ||
        source->map_load_token != s->gdat_scene_map_load_token ||
        source->scene_control_hash != s->gdat_scene_control_hash ||
        source->palette_hash != s->gdat_interface_palette_hash ||
        !s->gdat_static_scene_record.valid ||
        s->gdat_static_scene_record.map_load_token != source->map_load_token ||
        s->gdat_static_scene_record.scene_control_hash !=
            source->scene_control_hash || !s->gdat_static_light_control_owned ||
        s->gdat_static_light_map_load_token != source->map_load_token ||
        s->gdat_static_light_scene_control_hash !=
            source->scene_control_hash) {
        return 0;
    }
    gdat_index = dm2_v1_viewport_hud_hand_action_graphic_index(
        source->possession_index, source->left_or_right);
    if (gdat_index == 0 ||
        dm2_v1_fetch_viewport_asset(s, gdat_index, &pixels, &width, &height,
                                    &stride) != 0 ||
        !pixels || width <= 0 || height <= 0 || stride < width) {
        return 0;
    }
    memset(&request, 0, sizeof(request));
    request.gdat_index = gdat_index;
    request.gdat_category = source->gdat_category;
    request.gdat_subcategory = source->gdat_subcategory;
    request.gdat_entry = source->gdat_entry;
    request.field = source->gdat_entry;
    request.indexed_pixels = pixels;
    request.palette16 = s->gdat_interface_palette16;
    request.palette_hash = s->gdat_interface_palette_hash;
    request.palette_entry_count = 16;
    request.width = width;
    request.height = height;
    request.stride = stride;
    request.transparent_color = DM2_COLOR_TRANSPARENT;
    request.colorkey_palette_index =
        request.palette16[request.transparent_color];
    request.source_rect = (DM2_V1_ViewportRect){ 0, 0, width, height };
    request.destination_rect = source->destination_rect;
    request.valid = request.width > 0 && request.height > 0 &&
        request.stride >= request.width && request.palette_hash != 0u &&
        request.destination_rect.w > 0 && request.destination_rect.h > 0;
    memset(&command, 0, sizeof(command));
    command.material = request;
    command.indexed_pixels = request.indexed_pixels;
    command.palette16 = request.palette16;
    command.transparent_color = request.transparent_color;
    command.source_rect = request.source_rect;
    command.destination_rect = request.destination_rect;
    command.valid = request.valid && command.indexed_pixels &&
        command.palette16 && command.transparent_color >= 0 &&
        command.transparent_color < request.palette_entry_count &&
        request.colorkey_palette_index ==
            command.palette16[command.transparent_color];
    if (!dm2_v1_blit_hud_presentation_command(
            s->framebuffer, s->fb_stride, &command,
            &s->gdat_interface_palette_consumed_count)) {
        return 0;
    }
    s->last_hud_hand_action_material_request = request;
    s->last_hud_hand_action_presentation_command = command;
    return 1;
}

void dm2_v1_render_ui_chrome(DM2_V1_ViewportState *s)
{
    DM2_V1_HudChromeRenderPlan plan;
    if (!s || !s->framebuffer) return;
    memset(&s->last_hud_top_bar_material_request, 0,
           sizeof(s->last_hud_top_bar_material_request));
    memset(&s->last_hud_top_bar_presentation_command, 0,
           sizeof(s->last_hud_top_bar_presentation_command));
    memset(&s->last_hud_status_panel_material_request, 0,
           sizeof(s->last_hud_status_panel_material_request));
    memset(&s->last_hud_status_panel_presentation_command, 0,
           sizeof(s->last_hud_status_panel_presentation_command));
    memset(&s->last_hud_hand_action_material_request, 0,
           sizeof(s->last_hud_hand_action_material_request));
    memset(&s->last_hud_hand_action_presentation_command, 0,
           sizeof(s->last_hud_hand_action_presentation_command));
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
     * skproject/SKWIN/SkWinCore.cpp DRAW_CHAMPION_PICTURE (12866-12880)
     * draws decoded CHAMPIONS pixels, then DRAW_PLAYER_3STAT_HEALTH_BAR
     * (12885-12947) overlays live champion state. A real GDAT profile must
     * therefore leave missing static HUD material untouched while retaining
     * the dynamic state overlays below.
     */
    if (!dm2_v1_viewport_build_hud_chrome_plan_for_party(
            s->is_outdoor, s->hud_party_valid ? &s->hud_party : NULL,
            &plan)) {
        return;
    }
    if (!plan.outdoor && s->gdat_interface_hud_layout) {
        /* skproject _098d_1208 expands the original 640-wide dt04/0 rect
         * table. Firestaff's indexed game surface is 320 wide, so consume
         * the source rectangles through the matching half-resolution path. */
        for (int slot = 0; slot < plan.champion_slot_count; ++slot) {
            const DM2_V1_InterfaceHudLayout *layout = s->gdat_interface_hud_layout;
            DM2_V1_HudChampionSlotRender *champ = &plan.champion_slots[slot];
            champ->portrait_rect = (DM2_V1_ViewportRect){
                layout->portrait[slot].x / 2, layout->portrait[slot].y / 2,
                layout->portrait[slot].w / 2, layout->portrait[slot].h / 2 };
            champ->name_marker_rect = (DM2_V1_ViewportRect){
                layout->name[slot].x / 2, layout->name[slot].y / 2,
                layout->name[slot].w / 2, layout->name[slot].h / 2 };
            champ->hp_bar_rect = (DM2_V1_ViewportRect){
                layout->status[slot][0].x / 2, layout->status[slot][0].y / 2,
                layout->status[slot][0].w / 2, layout->status[slot][0].h / 2 };
            champ->stamina_bar_rect = (DM2_V1_ViewportRect){
                layout->status[slot][1].x / 2, layout->status[slot][1].y / 2,
                layout->status[slot][1].w / 2, layout->status[slot][1].h / 2 };
            champ->mana_bar_rect = (DM2_V1_ViewportRect){
                layout->status[slot][2].x / 2, layout->status[slot][2].y / 2,
                layout->status[slot][2].w / 2, layout->status[slot][2].h / 2 };
            champ->hp_fill_rect = dm2_v1_hud_bar_fill(&champ->hp_bar_rect, champ->hp_pct);
            champ->stamina_fill_rect = dm2_v1_hud_bar_fill(&champ->stamina_bar_rect, champ->stamina_pct);
            champ->mana_fill_rect = dm2_v1_hud_bar_fill(&champ->mana_bar_rect, champ->mana_pct);
        }
    }

    if (!dm2_v1_render_hud_core_asset(s,
                                      &plan.top_bar_rect,
                                      plan.top_bar_gdat_index)) {
        if (s->source_materials_required) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        } else {
            dm2_v1_fill_rect(vp, stride, &plan.top_bar_rect,
                             dm2_v1_hud_palette_color(s, DM2_COL_DKGRAY));
            ++s->fallback_hud_core_drawn_count;
        }
    }
    if (!s->source_materials_required) {
        dm2_v1_fill_rect(vp, stride, &plan.top_divider_rect,
                         dm2_v1_hud_palette_color(s, DM2_COL_MIDGRAY));
    }
    if (!dm2_v1_render_hud_core_asset(s,
                                      &plan.action_strip_rect,
                                      plan.action_strip_gdat_index)) {
        if (s->source_materials_required) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        } else {
            dm2_v1_fill_rect(vp, stride, &plan.action_strip_rect,
                             dm2_v1_hud_palette_color(s, DM2_COL_DKGRAY));
            ++s->fallback_hud_core_drawn_count;
        }
    }
    if (!s->source_materials_required) {
        dm2_v1_fill_rect(vp, stride, &plan.action_divider_rect,
                         dm2_v1_hud_palette_color(s, DM2_COL_MIDGRAY));
    }
    if (!dm2_v1_render_hud_core_asset(s,
                                      &plan.gold_box_rect,
                                      plan.gold_box_gdat_index)) {
        if (s->source_materials_required) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        } else {
            dm2_v1_fill_rect(vp, stride, &plan.gold_box_rect,
                             dm2_v1_hud_palette_color(s, DM2_COL_GROUND));
            ++s->fallback_hud_core_drawn_count;
        }
    }
    if (!s->source_materials_required) {
        dm2_v1_fill_coin_disc(vp, stride, &plan.gold_coin_rect, 11);
        dm2_v1_fill_rect(vp, stride, &plan.gold_label_rect,
                         dm2_v1_hud_palette_color(s, DM2_COL_LTGRAY));
    }
    for (int i = 0; i < plan.action_icon_count; ++i) {
        if (!s->source_materials_required) {
            dm2_v1_stroke_rect(vp, stride, &plan.action_icons[i].frame_rect,
                               dm2_v1_hud_palette_color(s, DM2_COL_MIDGRAY));
        }
        if (!dm2_v1_render_hud_core_asset(s,
                                          &plan.action_icons[i].fill_rect,
                                          plan.action_icons[i].gdat_index)) {
            if (s->source_materials_required) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
            } else {
                dm2_v1_fill_rect(vp, stride, &plan.action_icons[i].fill_rect,
                                 dm2_v1_hud_palette_color(
                                     s, plan.action_icons[i].fill_color));
                ++s->fallback_hud_core_drawn_count;
            }
        }
    }
    if (!plan.outdoor) {
        if (!s->source_materials_required) {
            dm2_v1_fill_rect(vp, stride, &plan.portrait_separator_dark_rect,
                             dm2_v1_hud_palette_color(s, DM2_COL_MIDGRAY));
            dm2_v1_fill_rect(vp, stride, &plan.portrait_separator_light_rect,
                             dm2_v1_hud_palette_color(s, DM2_COL_LTGRAY));
        }
        if (!dm2_v1_render_hud_core_asset(s,
                                          &plan.portrait_panel_rect,
                                          plan.portrait_panel_gdat_index)) {
            if (s->source_materials_required) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
            } else {
                dm2_v1_fill_rect(vp, stride, &plan.portrait_panel_rect,
                                 dm2_v1_hud_palette_color(s, DM2_COL_DKGRAY));
                ++s->fallback_hud_core_drawn_count;
            }
        }
        for (int slot = 0; slot < plan.champion_slot_count; ++slot) {
            if (!s->source_materials_required) {
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].frame_rect,
                                 dm2_v1_hud_palette_color(s, DM2_COL_MIDGRAY));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].fill_rect,
                                 dm2_v1_hud_palette_color(
                                     s, plan.champion_slots[slot].fill_color));
            }
            if (plan.champion_slots[slot].occupied) {
                int source_state_bound =
                    !s->source_materials_required ||
                    (plan.champion_slots[slot].state_source_bound &&
                     s->gdat_interface_hud_layout &&
                     s->gdat_interface_palette_ready &&
                     s->gdat_interface_font_rows &&
                     s->gdat_interface_font_hash != 0u);
                const uint8_t *portrait_pixels = NULL;
                int portrait_w = 0;
                int portrait_h = 0;
                int portrait_stride = 0;
                int portrait_gdat =
                    dm2_v1_viewport_hud_portrait_graphic_index(
                        plan.champion_slots[slot].portrait_index);
                /* DRAW_CHAMPION_PICTURE uses glbChampionSquad.HeroType(),
                 * not Firestaff's session-tail portrait ordinal. Until the
                 * original save/session parser binds that field, a real-data
                 * profile must not select a CHAMPIONS image by inference. */
                if (s->source_materials_required &&
                    !plan.champion_slots[slot].portrait_type_source_bound) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT);
                } else if (portrait_gdat != 0 &&
                    dm2_v1_fetch_viewport_asset(s,
                                                portrait_gdat,
                                                &portrait_pixels,
                                                &portrait_w,
                                                &portrait_h,
                                                &portrait_stride) == 0 &&
                    portrait_pixels && portrait_w > 0 && portrait_h > 0 &&
                    portrait_stride >= portrait_w) {
                    dm2_v1_blit_scaled_material_bitmap(s,
                                              vp,
                                              stride,
                                              plan.champion_slots[slot].portrait_rect.x,
                                              plan.champion_slots[slot].portrait_rect.y,
                                              plan.champion_slots[slot].portrait_rect.w,
                                              plan.champion_slots[slot].portrait_rect.h,
                                              portrait_pixels,
                                              portrait_w,
                                              portrait_h,
                                              portrait_stride,
                                              DM2_COLOR_TRANSPARENT,
                                              &s->gdat_interface_palette_consumed_count);
                    if (s->gdat_interface_palette_ready) {
                        ++s->gdat_interface_palette_consumed_count;
                    }
                    ++s->asset_hud_portrait_drawn_count;
                } else {
                    if (s->source_materials_required) {
                        dm2_v1_block_source_material(
                            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT);
                    } else {
                        dm2_v1_fill_rect(vp, stride,
                                         &plan.champion_slots[slot].portrait_rect,
                                         plan.champion_slots[slot].portrait_fill_color);
                        ++s->fallback_hud_portrait_drawn_count;
                    }
                }
                if (!source_state_bound) {
                    /* skproject DRAW_CHAMPION_PICTURE and
                     * DRAW_PLAYER_3STAT_HEALTH_BAR consume
                     * glbChampionSquad fields. Firestaff's session snapshot
                     * is not original-save provenance, so it cannot paint
                     * source-coloured names, bars, or leader state. */
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
                    continue;
                }
                if (!dm2_v1_render_hud_source_font(
                        s, &plan.champion_slots[slot].name_marker_rect,
                        plan.champion_slots[slot].name,
                        DM2_COL_WHITE, DM2_COL_BLACK)) {
                    if (s->source_materials_required) {
                        dm2_v1_block_source_material(
                            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
                    } else {
                        dm2_v1_fill_rect(
                            vp, stride,
                            &plan.champion_slots[slot].name_marker_rect,
                            dm2_v1_hud_palette_color(s, DM2_COL_WHITE));
                    }
                }
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].hp_bar_rect,
                                 dm2_v1_hud_palette_color(
                                     s, DM2_COL_BLACK));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].hp_fill_rect,
                                 dm2_v1_hud_palette_color(s, 2));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].stamina_bar_rect,
                                 dm2_v1_hud_palette_color(
                                     s, DM2_COL_BLACK));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].stamina_fill_rect,
                                 dm2_v1_hud_palette_color(s, 11));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].mana_bar_rect,
                                 dm2_v1_hud_palette_color(
                                     s, DM2_COL_BLACK));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].mana_fill_rect,
                                 dm2_v1_hud_palette_color(s, 12));
                if (plan.champion_slots[slot].leader) {
                    dm2_v1_fill_rect(
                        vp, stride,
                        &plan.champion_slots[slot].leader_mark_rect,
                        dm2_v1_hud_palette_color(s, DM2_COL_WHITE));
                }
            }
        }
        /* skproject DRAW_HAND_ACTION_ICONS follows the static interface
         * material path. Keep the hand command after the top-bar and
         * champion/status-panel receipts in this bounded stage-11 pass. */
        if (s->hud_hand_action_source.valid &&
            !dm2_v1_render_hud_hand_action_asset(s) &&
            s->source_materials_required) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        }
    }
}

/* skproject/SKWIN/SkWinCore.cpp CHECK_RECOMPUTE_LIGHT (30416-30439) binds
 * map-local GRAPHICSSET light before later dungeon draws, including
 * DRAW_DOOR_FRAMES. Keep the composition gate token- and hash-bound. */
static int dm2_v1_viewport_frame_light_palette_owned(
    const DM2_V1_ViewportState *s)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;

    if (!s || !s->gdat_scene_control_ready ||
        s->gdat_scene_map_load_token == 0u ||
        s->gdat_scene_control_hash == 0u ||
        !s->gdat_interface_palette_ready ||
        s->gdat_interface_palette_hash == 0u) {
        return 0;
    }
    scene = &s->gdat_static_scene_record;
    return scene->valid &&
        scene->map_load_token == s->gdat_scene_map_load_token &&
        scene->scene_control_hash == s->gdat_scene_control_hash &&
        s->gdat_static_light_control_owned &&
        s->gdat_static_light_map_load_token == s->gdat_scene_map_load_token &&
        s->gdat_static_light_scene_control_hash ==
            s->gdat_scene_control_hash;
}

static int dm2_v1_viewport_scene_control_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportSceneControlCommand *out_command)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;

    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->source_materials_required ||
        !dm2_v1_viewport_frame_light_palette_owned(s) ||
        s->gdat_scene_colorkey > 15u ||
        !s->gdat_static_ambient_darkness_control_owned ||
        s->gdat_static_ambient_darkness_map_load_token !=
            s->gdat_scene_map_load_token ||
        s->gdat_static_ambient_darkness_scene_control_hash !=
            s->gdat_scene_control_hash ||
        !s->gdat_static_scene_colorkey_control_owned ||
        s->gdat_static_scene_colorkey_map_load_token !=
            s->gdat_scene_map_load_token ||
        s->gdat_static_scene_colorkey_scene_control_hash !=
            s->gdat_scene_control_hash) {
        return 0;
    }
    scene = &s->gdat_static_scene_record;
    if (!scene->valid || scene->map_load_token != s->gdat_scene_map_load_token ||
        scene->scene_control_hash != s->gdat_scene_control_hash ||
        scene->graphicsset != (uint8_t)s->gdat_scene_material_index ||
        scene->material_category != DM2_GDAT_CATEGORY_GRAPHICSSET ||
        scene->ambient_darkness != s->gdat_ambient_darkness) {
        return 0;
    }
    out_command->gdat_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    out_command->graphicsset = scene->graphicsset;
    out_command->field = DM2_GDAT_GFXSET_AMBIANT_DARKNESS;
    out_command->map_load_token = s->gdat_scene_map_load_token;
    out_command->scene_control_hash = s->gdat_scene_control_hash;
    out_command->palette_hash = s->gdat_interface_palette_hash;
    out_command->scene_colorkey = s->gdat_scene_colorkey;
    out_command->colorkey_palette_index =
        s->gdat_interface_palette16[out_command->scene_colorkey];
    out_command->ambient_darkness = s->gdat_ambient_darkness;
    out_command->light_floor = s->gdat_scene_light_floor;
    out_command->walk_path_depth = s->gdat_scene_light_search_depth;
    out_command->light_check_enabled = s->gdat_scene_light_recompute_enabled;
    out_command->valid = out_command->palette_hash != 0u &&
        out_command->colorkey_palette_index ==
            s->gdat_interface_palette16[out_command->scene_colorkey];
    return out_command->valid;
}

/* ── Main render entry ─────────────────────────────────────────────── */

static int dm2_v1_viewport_build_m11_frame_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportM11FrameReceipt *out_receipt);

void dm2_v1_viewport_render(DM2_V1_ViewportState *s)
{
    if (!s) return;

    /* If not dirty and no pending world update, skip full redraw.
     * For Phase 3, always render when called (dirty flag tracking
     * is wired but full optimization deferred to Phase 4). */
    if (!s->dirty && !s->framebuffer) return;
    s->asset_floor_ceiling_drawn_count = 0;
    s->fallback_floor_ceiling_drawn_count = 0;
    s->blocked_material_draw_count = 0;
    s->blocked_material_mask = 0u;
    s->asset_outdoor_sky_drawn_count = 0;
    s->asset_outdoor_ground_drawn_count = 0;
    s->asset_wall_drawn_count = 0;
    s->fallback_wall_drawn_count = 0;
    s->asset_door_panel_drawn_count = 0;
    s->asset_door_overlay_drawn_count = 0;
    s->asset_door_frame_drawn_count = 0;
    s->asset_door_button_drawn_count = 0;
    s->fallback_door_drawn_count = 0;
    memset(&s->last_original_material_gate, 0,
           sizeof(s->last_original_material_gate));
    memset(&s->last_original_door_surface_request, 0,
           sizeof(s->last_original_door_surface_request));
    memset(&s->last_original_door_surface_binding, 0,
           sizeof(s->last_original_door_surface_binding));
    memset(&s->last_original_door_opening_frame_request, 0,
           sizeof(s->last_original_door_opening_frame_request));
    memset(&s->last_original_door_presentation_command, 0,
           sizeof(s->last_original_door_presentation_command));
    memset(&s->last_hud_top_bar_material_request, 0,
           sizeof(s->last_hud_top_bar_material_request));
    memset(&s->last_hud_top_bar_presentation_command, 0,
           sizeof(s->last_hud_top_bar_presentation_command));
    memset(&s->last_hud_status_panel_material_request, 0,
           sizeof(s->last_hud_status_panel_material_request));
    memset(&s->last_hud_status_panel_presentation_command, 0,
           sizeof(s->last_hud_status_panel_presentation_command));
    memset(&s->last_hud_hand_action_material_request, 0,
           sizeof(s->last_hud_hand_action_material_request));
    memset(&s->last_hud_hand_action_presentation_command, 0,
           sizeof(s->last_hud_hand_action_presentation_command));
    memset(&s->last_dungeon_floor_presentation_command, 0,
           sizeof(s->last_dungeon_floor_presentation_command));
    memset(&s->last_dungeon_ceiling_presentation_command, 0,
           sizeof(s->last_dungeon_ceiling_presentation_command));
    memset(&s->last_dungeon_wall_presentation_command, 0,
           sizeof(s->last_dungeon_wall_presentation_command));
    s->last_dungeon_wall_material_required_mask = 0u;
    s->last_dungeon_wall_material_consumed_mask = 0u;
    memset(&s->last_scene_control_presentation_command, 0,
           sizeof(s->last_scene_control_presentation_command));
    memset(&s->last_creature_presentation_command, 0,
           sizeof(s->last_creature_presentation_command));
    memset(&s->last_item_presentation_command, 0,
           sizeof(s->last_item_presentation_command));
    memset(&s->last_frame_composition, 0,
           sizeof(s->last_frame_composition));
    memset(&s->last_m11_frame_receipt, 0,
           sizeof(s->last_m11_frame_receipt));
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
    s->gdat_interface_palette_consumed_count = 0;
    s->gdat_interface_font_consumed_count = 0;
    s->gdat_material_palette_floor_ceiling_consumed_count = 0;
    s->gdat_material_palette_wall_consumed_count = 0;
    s->gdat_material_palette_door_frame_consumed_count = 0;
    s->gdat_scene_light_consumed_count = 0;
    s->gdat_scene_material_consumed_count = 0;
    s->gdat_scene_weather_consumed_count = 0;
    s->gdat_sprite_palette_consumed_count = 0;
    if (s->gdat_scene_control_ready) {
        /* skproject/SKWIN/SkWinCore.cpp CHECK_RECOMPUTE_LIGHT (30416-30439)
         * binds GRAPHICSSET darkness to the active map before viewport work.
         * Preserve the original level scale and bounded walk-path depth here;
         * actual palette selection remains blocked until its source table is
         * decoded rather than inventing a screen-darkening overlay. */
        dm2_v1_viewport_scene_light_control(
            s->gdat_highest_light_level,
            s->gdat_ambient_darkness,
            &s->gdat_scene_light_floor,
            &s->gdat_scene_light_search_depth,
            &s->gdat_scene_light_recompute_enabled);
        if (s->gdat_ambient_light != 0u ||
            s->gdat_scene_light_floor != 0u ||
            s->gdat_scene_light_search_depth != 0u) {
            ++s->gdat_scene_light_consumed_count;
        }
    }
    s->last_frame_composition.indoor_viewport = !s->is_outdoor;
    s->last_frame_composition.scene_record_owned =
        s->gdat_static_scene_record.valid &&
        s->gdat_static_scene_record.map_load_token ==
            s->gdat_scene_map_load_token &&
        s->gdat_static_scene_record.scene_control_hash ==
            s->gdat_scene_control_hash;
    s->last_frame_composition.scene_light_owned =
        dm2_v1_viewport_frame_light_palette_owned(s);
    s->last_frame_composition.palette_owned =
        s->gdat_interface_palette_ready &&
        s->gdat_interface_palette_hash != 0u;
    s->last_frame_composition.map_load_token = s->gdat_scene_map_load_token;
    s->last_frame_composition.scene_control_hash = s->gdat_scene_control_hash;
    s->last_frame_composition.palette_hash = s->gdat_interface_palette_hash;
    s->last_frame_composition.light_floor = s->gdat_scene_light_floor;
    s->last_frame_composition.light_search_depth =
        s->gdat_scene_light_search_depth;
    s->last_hud_core_gdat_hash = 2166136261u;
    s->last_hud_core_pixel_count = 0u;
    s->asset_hud_portrait_drawn_count = 0;
    s->fallback_hud_portrait_drawn_count = 0;

    /* DM2 has two fundamentally different render paths:
     *   1. Indoor dungeon (is_outdoor=0): first-person 3D dungeon view
     *   2. Outdoor (is_outdoor=1): sky gradient + ground + buildings
     *
     * Source: SKULL.ASM T560 (dungeon), SKULL.ASM T600 (outdoor) */

    if (s->is_outdoor) {
        /* DM2 outdoor rendering:
         * Source: SKULL.ASM T600 (outdoor tick, sky and ground draw)
         *         skproject/SKWIN/SkWinCore.cpp GRAPHICSSET material route
         *
         * Do not substitute a generated gradient for source material.  The
         * active map GRAPHICSSET already supplies the ceiling/floor GDAT
         * records used by the scene; route them through the same palette
         * binding as the indoor viewport before weather and HUD are layered. */
        uint8_t *vp = s->framebuffer;
        int stride = s->fb_stride;
        const uint8_t *sky_pixels = NULL;
        const uint8_t *ground_pixels = NULL;
        int sky_w = 0;
        int sky_h_src = 0;
        int sky_stride = 0;
        int ground_w = 0;
        int ground_h_src = 0;
        int ground_stride = 0;
        int sky_h = DM2_VP_HEIGHT / 2;
        int sky_gdat_index = dm2_v1_viewport_scene_material_graphic_index(
            s->gdat_scene_material_index,
            DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
        int ground_gdat_index = dm2_v1_viewport_scene_material_graphic_index(
            s->gdat_scene_material_index,
            DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);
        int sky_asset =
            dm2_v1_fetch_viewport_asset(s,
                                        sky_gdat_index,
                                        &sky_pixels,
                                        &sky_w,
                                        &sky_h_src,
                                        &sky_stride) == 0 &&
            sky_pixels && sky_w > 0 && sky_h_src > 0;
        int ground_asset =
            dm2_v1_fetch_viewport_asset(s,
                                        ground_gdat_index,
                                        &ground_pixels,
                                        &ground_w,
                                        &ground_h_src,
                                        &ground_stride) == 0 &&
            ground_pixels && ground_w > 0 && ground_h_src > 0;

        if (sky_asset) {
            dm2_v1_blit_tiled_material_bitmap(
                s, vp, stride, 0, 0, DM2_VP_WIDTH, sky_h, sky_pixels,
                sky_w, sky_h_src,
                sky_stride > 0 ? sky_stride : sky_w, -1,
                &s->gdat_material_palette_floor_ceiling_consumed_count);
            ++s->asset_floor_ceiling_drawn_count;
            ++s->asset_outdoor_sky_drawn_count;
            ++s->gdat_scene_material_consumed_count;
        }
        if (ground_asset) {
            dm2_v1_blit_tiled_material_bitmap(
                s, vp, stride, 0, sky_h, DM2_VP_WIDTH,
                DM2_VP_HEIGHT - sky_h, ground_pixels, ground_w,
                ground_h_src,
                ground_stride > 0 ? ground_stride : ground_w, -1,
                &s->gdat_material_palette_floor_ceiling_consumed_count);
            ++s->asset_floor_ceiling_drawn_count;
            ++s->asset_outdoor_ground_drawn_count;
            ++s->gdat_scene_material_consumed_count;
        }
    } else {
        /* DM2 indoor dungeon rendering:
         * Draw order (same as DM1): D3→D2→D1→D0 per depth.
         * Source: DUNGEON.C:1371-1421; DUNVIEW.C:8466-8542 */

        /* 1. Background (black) */
        dm2_v1_render_background(s);

        /* 2. Floor and ceiling */
        dm2_v1_render_floor_ceiling(s);
        if (s->source_materials_required &&
            s->last_dungeon_ceiling_presentation_command.valid) {
            s->last_frame_composition.dungeon_ceiling_presentation_stage = 1;
            s->last_frame_composition.dungeon_ceiling_command_consumed = 1;
            s->last_frame_composition.dungeon_ceiling_command =
                s->last_dungeon_ceiling_presentation_command;
        }
        if (s->source_materials_required &&
            s->last_dungeon_floor_presentation_command.valid) {
            s->last_frame_composition.dungeon_floor_presentation_stage = 2;
            s->last_frame_composition.dungeon_floor_command_consumed = 1;
            s->last_frame_composition.dungeon_floor_command =
                s->last_dungeon_floor_presentation_command;
        }

        /* 3. Walls — placeholder pass (real walls need GRAPHICS.DAT) */
        dm2_v1_render_walls(s);
        if (s->source_materials_required &&
            s->last_dungeon_wall_presentation_command.valid) {
            s->last_frame_composition.dungeon_wall_presentation_stage = 3;
            s->last_frame_composition.dungeon_wall_command_consumed = 1;
            s->last_frame_composition.dungeon_wall_command =
                s->last_dungeon_wall_presentation_command;
            s->last_frame_composition.dungeon_wall_material_required_mask =
                s->last_dungeon_wall_material_required_mask;
            s->last_frame_composition.dungeon_wall_material_consumed_mask =
                s->last_dungeon_wall_material_consumed_mask;
        }

        /* 4. Doors. skproject's map light/palette bind precedes this pass;
         * a required source door cannot be composed without that ownership. */
        if (s->source_materials_required &&
            !s->last_frame_composition.scene_light_owned) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
        } else {
            dm2_v1_render_doors(s);
            if (s->source_materials_required &&
                s->last_original_door_presentation_command.valid) {
                s->last_frame_composition.door_presentation_stage = 4;
                s->last_frame_composition.door_command_consumed = 1;
                s->last_frame_composition.door_command =
                    s->last_original_door_presentation_command;
            }
        }

        if (s->source_materials_required) {
            DM2_V1_ViewportSceneControlCommand command;

            if (dm2_v1_viewport_scene_control_command(s, &command)) {
                s->last_scene_control_presentation_command = command;
                s->last_frame_composition.scene_control_presentation_stage = 5;
                s->last_frame_composition.scene_control_command_consumed = 1;
                s->last_frame_composition.scene_control_command = command;
            } else {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_SCENE_CONTROL);
            }
        }

        /* 6. Creatures */
        dm2_v1_render_creatures(s);
        if (s->source_materials_required &&
            s->last_creature_presentation_command.valid) {
            s->last_frame_composition.creature_presentation_stage = 6;
            s->last_frame_composition.creature_command_consumed = 1;
            s->last_frame_composition.creature_command =
                s->last_creature_presentation_command;
        }

        /* 7. Floor objects/items */
        dm2_v1_render_items(s);
        if (s->source_materials_required &&
            s->last_item_presentation_command.valid) {
            s->last_frame_composition.item_presentation_stage = 7;
            s->last_frame_composition.item_command_consumed = 1;
            s->last_frame_composition.item_command =
                s->last_item_presentation_command;
        }

        /* 8. Creature possession/item overlays */
        dm2_v1_render_creature_possession_items(s);

        /* 9. Projectiles */
        dm2_v1_render_projectiles(s);
    }

    /* Carried leader-hand item overlay */
    dm2_v1_render_carried_item(s);

    /* Weather overlay (applies to both indoor and outdoor) */
    dm2_v1_render_weather_overlay(s);

    /* Stage 11: UI chrome (always on top) */
    dm2_v1_render_ui_chrome(s);
    if (!s->is_outdoor) {
        s->last_frame_composition.hud_presentation_stage = 11;
        if (s->source_materials_required &&
            s->last_hud_top_bar_material_request.valid) {
            s->last_frame_composition.hud_top_bar_material_consumed = 1;
            s->last_frame_composition.hud_top_bar_request =
                s->last_hud_top_bar_material_request;
        }
        if (s->source_materials_required &&
            s->last_hud_top_bar_presentation_command.valid) {
            s->last_frame_composition.hud_top_bar_command_consumed = 1;
            s->last_frame_composition.hud_top_bar_command =
                s->last_hud_top_bar_presentation_command;
            s->last_frame_composition.hud_top_bar_order = 1;
        }
        if (s->source_materials_required &&
            s->last_hud_status_panel_material_request.valid) {
            s->last_frame_composition.hud_status_panel_material_consumed = 1;
            s->last_frame_composition.hud_status_panel_request =
                s->last_hud_status_panel_material_request;
        }
        if (s->source_materials_required &&
            s->last_hud_status_panel_presentation_command.valid) {
            s->last_frame_composition.hud_status_panel_command_consumed = 1;
            s->last_frame_composition.hud_status_panel_command =
                s->last_hud_status_panel_presentation_command;
            s->last_frame_composition.hud_status_panel_order = 2;
        }
        if (s->source_materials_required &&
            s->hud_hand_action_source.valid &&
            s->last_hud_hand_action_material_request.valid) {
            s->last_frame_composition.hud_hand_action_command_consumed = 1;
            s->last_frame_composition.hud_hand_action_request =
                s->last_hud_hand_action_material_request;
            s->last_frame_composition.hud_hand_action_order = 3;
        }
        if (s->source_materials_required &&
            s->hud_hand_action_source.valid &&
            s->last_hud_hand_action_presentation_command.valid) {
            s->last_frame_composition.hud_hand_action_command =
                s->last_hud_hand_action_presentation_command;
        }
        s->last_frame_composition.valid =
            !s->source_materials_required ||
            (s->last_frame_composition.scene_light_owned &&
             s->last_frame_composition.dungeon_ceiling_command_consumed &&
             s->last_frame_composition.dungeon_ceiling_command.valid &&
             s->last_frame_composition.dungeon_ceiling_presentation_stage <
                 s->last_frame_composition.dungeon_floor_presentation_stage &&
             s->last_frame_composition.dungeon_floor_command_consumed &&
             s->last_frame_composition.dungeon_floor_command.valid &&
             s->last_frame_composition.dungeon_floor_presentation_stage <
                 s->last_frame_composition.hud_presentation_stage &&
             s->last_frame_composition.dungeon_wall_command_consumed &&
             s->last_frame_composition.dungeon_wall_command.valid &&
             s->last_frame_composition.dungeon_wall_material_required_mask !=
                 0u &&
             s->last_frame_composition.dungeon_wall_material_required_mask ==
                 s->last_frame_composition.dungeon_wall_material_consumed_mask &&
             s->last_frame_composition.dungeon_floor_presentation_stage <
                 s->last_frame_composition.dungeon_wall_presentation_stage &&
             s->last_frame_composition.dungeon_wall_presentation_stage <
                 s->last_frame_composition.hud_presentation_stage &&
             s->last_frame_composition.scene_control_command_consumed &&
             s->last_frame_composition.scene_control_command.valid &&
             s->last_frame_composition.dungeon_wall_presentation_stage <
                 s->last_frame_composition.scene_control_presentation_stage &&
             s->last_frame_composition.scene_control_presentation_stage <
                 s->last_frame_composition.hud_presentation_stage &&
             (!s->last_creature_render_valid ||
              (s->last_frame_composition.creature_command_consumed &&
               s->last_frame_composition.creature_command.valid &&
               s->last_frame_composition.scene_control_presentation_stage <
                   s->last_frame_composition.creature_presentation_stage &&
               s->last_frame_composition.creature_presentation_stage <
                   s->last_frame_composition.hud_presentation_stage)) &&
             (!s->item_count ||
              (s->last_frame_composition.item_command_consumed &&
               s->last_frame_composition.item_command.valid &&
               s->last_frame_composition.creature_presentation_stage <
                   s->last_frame_composition.item_presentation_stage &&
               s->last_frame_composition.item_presentation_stage <
                   s->last_frame_composition.hud_presentation_stage)) &&
             s->last_frame_composition.hud_top_bar_material_consumed &&
             s->last_frame_composition.hud_top_bar_command_consumed &&
             (s->is_outdoor ||
              (s->last_frame_composition.hud_status_panel_material_consumed &&
               s->last_frame_composition.hud_status_panel_command_consumed)) &&
             (!s->hud_hand_action_source.valid ||
             (s->last_frame_composition.hud_hand_action_command_consumed &&
               s->last_frame_composition.hud_hand_action_command.valid &&
               s->last_frame_composition.hud_top_bar_order <
                   s->last_frame_composition.hud_status_panel_order &&
                   s->last_frame_composition.hud_status_panel_order <
                   s->last_frame_composition.hud_hand_action_order)));
    }

    (void)dm2_v1_viewport_build_m11_frame_receipt(
        s, &s->last_m11_frame_receipt);

    s->dirty = 0;
}

/* ── GDAT-backed graphic fetch ───────────────────────────────────── */

static int dm2_v1_viewport_build_m11_frame_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportM11FrameReceipt *out_receipt)
{
    const DM2_V1_ViewportFrameCompositionReceipt *c;
    int door_required = 0;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!s) return 0;
    for (int i = 0; i < DM2_SQ_COUNT; ++i) {
        if (s->squares[i].flags & DM2_SQF_HAS_DOOR) {
            door_required = 1;
            break;
        }
    }

    c = &s->last_frame_composition;
    out_receipt->source_materials_required = s->source_materials_required;
    out_receipt->map_load_token = c->map_load_token;
    out_receipt->scene_control_hash = c->scene_control_hash;
    out_receipt->palette_hash = c->palette_hash;
    out_receipt->composition = *c;

    /* skproject's indoor draw sequence consumes geometry, door, map chips
     * and interface GDAT under one active map/palette context. M11 receives
     * nothing if any requested source route is absent or belongs to another
     * scene generation. */
    out_receipt->valid =
        s->source_materials_required && c->valid && c->indoor_viewport &&
        c->scene_record_owned &&
        c->scene_light_owned && c->palette_owned &&
        c->map_load_token != 0u && c->scene_control_hash != 0u &&
        c->palette_hash != 0u &&
        c->dungeon_ceiling_command_consumed &&
        c->dungeon_ceiling_command.valid &&
        c->dungeon_ceiling_command.map_load_token == c->map_load_token &&
        c->dungeon_ceiling_command.scene_control_hash ==
            c->scene_control_hash &&
        c->dungeon_ceiling_command.palette_hash == c->palette_hash &&
        c->dungeon_floor_command_consumed && c->dungeon_floor_command.valid &&
        c->dungeon_floor_command.map_load_token == c->map_load_token &&
        c->dungeon_floor_command.scene_control_hash == c->scene_control_hash &&
        c->dungeon_floor_command.palette_hash == c->palette_hash &&
        c->dungeon_wall_command_consumed && c->dungeon_wall_command.valid &&
        c->dungeon_wall_material_required_mask != 0u &&
        c->dungeon_wall_material_required_mask ==
            c->dungeon_wall_material_consumed_mask &&
        c->dungeon_wall_command.map_load_token == c->map_load_token &&
        c->dungeon_wall_command.scene_control_hash == c->scene_control_hash &&
        c->dungeon_wall_command.palette_hash == c->palette_hash &&
        c->scene_control_command_consumed && c->scene_control_command.valid &&
        c->scene_control_command.map_load_token == c->map_load_token &&
        c->scene_control_command.scene_control_hash == c->scene_control_hash &&
        c->scene_control_command.palette_hash == c->palette_hash &&
        (!door_required ||
         (c->door_command_consumed && c->door_command.valid &&
          c->door_command.opening_frame.material.valid &&
          c->door_command.opening_frame.material.request.map_load_token ==
              c->map_load_token &&
          c->door_command.opening_frame.material.request.scene_control_hash ==
              c->scene_control_hash &&
          c->door_command.opening_frame.material.request.palette_hash ==
              c->palette_hash)) &&
        (!s->creature_count ||
         (c->creature_command_consumed && c->creature_command.valid &&
          c->creature_command.map_load_token == c->map_load_token &&
          c->creature_command.scene_control_hash == c->scene_control_hash &&
          c->creature_command.palette_hash == c->palette_hash)) &&
        (!s->item_count ||
         (c->item_command_consumed && c->item_command.valid &&
          c->item_command.map_load_token == c->map_load_token &&
          c->item_command.scene_control_hash == c->scene_control_hash &&
          c->item_command.palette_hash == c->palette_hash)) &&
        c->hud_top_bar_material_consumed && c->hud_top_bar_request.valid &&
        c->hud_top_bar_request.palette_hash == c->palette_hash &&
        c->hud_top_bar_command_consumed && c->hud_top_bar_command.valid &&
        c->hud_status_panel_material_consumed &&
        c->hud_status_panel_request.valid &&
        c->hud_status_panel_request.palette_hash == c->palette_hash &&
        c->hud_status_panel_command_consumed &&
        c->hud_status_panel_command.valid &&
        (!s->hud_hand_action_source.valid ||
         (c->hud_hand_action_command_consumed &&
          c->hud_hand_action_request.valid &&
          c->hud_hand_action_request.palette_hash == c->palette_hash &&
          c->hud_hand_action_command.valid));
    out_receipt->m11_consume_frame = out_receipt->valid;
    return out_receipt->valid;
}

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
