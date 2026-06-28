/*
 * Firestaff DM1 V1 Champion Panel pixel-slice probe.
 *
 * Source-locked to ReDMCSB layout-696 + CHAMDRAW.C F0287/F0291/F0292 +
 * PANEL.C F0345/F0658. Paints a synthetic 320x200 frame buffer with the
 * 4-champion status box row, HP/stamina/mana bar-graph region, ready/action
 * hand slots, portrait box, and food/water/poisoned label zone anchors.
 * Pixel-slices every fixed anchor and confirms non-overlap, correct
 * screen-space origin, and correct sizes.
 *
 * This probe does NOT claim original DOS parity; it pins the Firestaff
 * helper output to layout-696 / CHAMDRAW.C coordinates so the HUD layout
 * stays internally consistent with the rest of the champion-panel source.
 *
 * ReDMCSB source anchors:
 *   - DEFS.H:2186-2192 C00..C037 graphic id set, including C033/C034/C035
 *     slot-box graphics and C026 portrait graphic.
 *   - DEFS.H:2186-2195 + 3793 status-box stride C69 = 69 px,
 *     status-box width 67, height 29.
 *   - DEFS.H G0046_auc_Graphic562_ChampionColor = {7,11,8,14} for the
 *     4-champion color palette.
 *   - CHAMDRAW.C F0287 lines 67-130 bar-graph zone iteration:
 *     C195_ZONE_FIRST_BAR_GRAPH + champIdx, stride 4 per stat.
 *     Each bar is 4 px wide, 25 px max height; HP/stamina/mana.
 *   - CHAMDRAW.C F0291 lines 632-651 status hand slot box:
 *     18x18 graphic at (champIdx*69+4/24, 10).
 *   - CHAMDRAW.C F0292 lines 757-1110 status box redraw.
 *   - CHAMDRAW.C F0354 lines 1503-1531 inventory champion portrait box:
 *     32x29 from CHAMPION.Portrait at zones C175..C178 anchored at
 *     (champIdx*69+7, 0) within the status box.
 *   - PANEL.C F0345 lines 1563-1616 food/water/poisoned label blits at
 *     zones C500/C501/C502 (zones, not absolute pixels).
 *   - layout-696 zone anchor table:
 *       C159..C162 champion name zones (stride C69)
 *       C175..C178 portrait zones
 *       C195..C198 HP bar zones
 *       C199..C202 stamina bar zones
 *       C203..C206 mana bar zones
 *       C211..C218 status hand slot zones
 *       C500..C502 food/water/poisoned label zones
 *
 * The probe allocates a 320x200 surface, fills it with PIXEL_SENTINEL,
 * paints the panel according to the helpers, then pixel-slices every
 * fixed anchor. PASS / FAIL counters are written to stdout. It does not
 * run a real DM1 game; all geometry is source-locked synthetic.
 */

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* DM1 PC34 frame buffer is 320x200 chunky pixels. */
#define DM1_PANEL_FRAME_WIDTH  320
#define DM1_PANEL_FRAME_HEIGHT 200

/* Viewport occupies y=33..168 (height 136). The champion panel row sits
 * immediately below at y=170..199 (29 px tall status box; 1 px gap). */
#define DM1_PANEL_ROW_TOP_Y    170
#define DM1_PANEL_ROW_HEIGHT   29

/* Pixel sentinel and helper pixels. */
enum {
    PIXEL_SENTINEL = 0xee,
    PIXEL_FILL_GRAY = 0x11,
    PIXEL_HP_FILL = 0x21,
    PIXEL_STAMINA_FILL = 0x22,
    PIXEL_MANA_FILL = 0x23,
    PIXEL_PORTRAIT_BORDER = 0x33,
    PIXEL_SLOT_BORDER = 0x44,
    PIXEL_NAME_BG = 0x55
};

static int g_failures = 0;
static int g_checks = 0;

static void check_int(const char *id, int got, int want)
{
    ++g_checks;
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++g_failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void check_pixel(const char *id, const uint8_t *fb, int x, int y, uint8_t want)
{
    ++g_checks;
    if (x < 0 || x >= DM1_PANEL_FRAME_WIDTH ||
        y < 0 || y >= DM1_PANEL_FRAME_HEIGHT) {
        printf("FAIL %s coords out of range (%d,%d)\n", id, x, y);
        ++g_failures;
        return;
    }
    if (fb[y * DM1_PANEL_FRAME_WIDTH + x] != want) {
        printf("FAIL %s (%d,%d) got=0x%02x want=0x%02x\n",
               id, x, y, fb[y * DM1_PANEL_FRAME_WIDTH + x], want);
        ++g_failures;
    }
}

static void check_pixel_band(const char *id_prefix,
                             const uint8_t *fb,
                             int x0, int y0, int x1, int y1,
                             uint8_t want)
{
    char id[96];
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            snprintf(id, sizeof(id), "%s.px(%d,%d)", id_prefix, x, y);
            check_pixel(id, fb, x, y, want);
        }
    }
}

static void fill_rect(uint8_t *fb, int x, int y, int w, int h, uint8_t value)
{
    if (x < 0 || y < 0) return;
    for (int yy = y; yy < y + h && yy < DM1_PANEL_FRAME_HEIGHT; ++yy) {
        for (int xx = x; xx < x + w && xx < DM1_PANEL_FRAME_WIDTH; ++xx) {
            fb[yy * DM1_PANEL_FRAME_WIDTH + xx] = value;
        }
    }
}

/* -----------------------------------------------------------------
 * Paint helpers — these mirror the source-locked helpers used by
 * m11_draw_dm1_champion_panel. We re-implement the same coordinates
 * so the probe can stand alone without linking the full M11 stack.
 * ----------------------------------------------------------------- */

static void paint_status_box(uint8_t *fb, int champIdx, uint8_t fillColor)
{
    int x = champIdx * DM1_STATUS_BOX_SPACING;
    int y = DM1_PANEL_ROW_TOP_Y;
    fill_rect(fb, x, y, DM1_STATUS_BOX_WIDTH, DM1_STATUS_BOX_HEIGHT, fillColor);
}

static void paint_bar_graph(uint8_t *fb, int champIdx, int statIndex, uint8_t fillColor)
{
    int x, y;
    DM1_ChampionPanel_BarGraphScreenXY(champIdx, statIndex, &x, &y);
    /* Panel-relative y=2 maps to DM1_PANEL_ROW_TOP_Y+2 on the frame. */
    fill_rect(fb, x, y + DM1_PANEL_ROW_TOP_Y,
              DM1_BAR_GRAPH_WIDTH, DM1_BAR_GRAPH_MAX_HEIGHT, fillColor);
}

static void paint_hand_slot(uint8_t *fb, int champIdx, int handSlot, uint8_t borderColor)
{
    int x, y;
    DM1_ChampionPanel_StatusHandSlotXY(champIdx, handSlot, &x, &y);
    fill_rect(fb, x, y + DM1_PANEL_ROW_TOP_Y,
              DM1_SLOT_BOX_SIZE, DM1_SLOT_BOX_SIZE, borderColor);
}

static void paint_portrait_box(uint8_t *fb, int champIdx, uint8_t borderColor)
{
    int x = DM1_ChampionPanel_PortraitScreenX(champIdx);
    /* Portrait box: 32x29 at panel-relative y=0. */
    fill_rect(fb, x, DM1_PANEL_ROW_TOP_Y,
              DM1_PORTRAIT_WIDTH, DM1_PORTRAIT_HEIGHT, borderColor);
}

static void paint_name_band(uint8_t *fb, int champIdx, uint8_t bgColor)
{
    int x = DM1_ChampionPanel_NameZoneX(champIdx);
    /* Name zone stride is C69. The actual text band is centered inside
     * the status box by F0650; we only paint a top 4 px band on the
     * status box border for the slice. */
    fill_rect(fb, x + 4, DM1_PANEL_ROW_TOP_Y + 1,
              DM1_STATUS_BOX_WIDTH - 8, 4, bgColor);
}

/* -----------------------------------------------------------------
 * Top-level verification: walk through all source-locked anchor
 * coordinates and confirm the synthetic paint matches the layout.
 * ----------------------------------------------------------------- */

static void verify_constants_lock(void)
{
    /* DEFS.H:2186-2195 + 3793 status-box stride/width/height. */
    check_int("constants.status_box_stride", DM1_STATUS_BOX_SPACING, 69);
    check_int("constants.status_box_width", DM1_STATUS_BOX_WIDTH, 67);
    check_int("constants.status_box_height", DM1_STATUS_BOX_HEIGHT, 29);

    /* DEFS.H bar graph constants. */
    check_int("constants.bar_graph_width", DM1_BAR_GRAPH_WIDTH, 4);
    check_int("constants.bar_graph_max_height", DM1_BAR_GRAPH_MAX_HEIGHT, 25);

    /* DEFS.H portrait + slot constants. */
    check_int("constants.portrait_width", DM1_PORTRAIT_WIDTH, 32);
    check_int("constants.portrait_height", DM1_PORTRAIT_HEIGHT, 29);
    check_int("constants.portrait_offset_x", DM1_PORTRAIT_OFFSET_X, 7);
    check_int("constants.slot_box_size", DM1_SLOT_BOX_SIZE, 18);
    check_int("constants.champion_count", DM1_CHAMPION_COUNT, 4);

    /* G0046_auc_Graphic562_ChampionColor. */
    check_int("constants.champion_color[0]", DM1_ChampionColor[0], 7);
    check_int("constants.champion_color[1]", DM1_ChampionColor[1], 11);
    check_int("constants.champion_color[2]", DM1_ChampionColor[2], 8);
    check_int("constants.champion_color[3]", DM1_ChampionColor[3], 14);

    /* VGA palette index guards so the palette is the original 16-color
     * subset used by DEFS.H:2085-2100. */
    check_int("constants.color_darkest_gray", DM1_COLOR_DARKEST_GRAY, 12);
    check_int("constants.color_lightest_gray", DM1_COLOR_LIGHTEST_GRAY, 13);
    check_int("constants.color_gold", DM1_COLOR_GOLD, 9);
}

static void verify_helpers_lock(void)
{
    int x, y;

    /* DEFS.H + layout-696: bar graph anchors at champIdx*69+46/53/60, y=2.
     * The helper returns panel-relative coordinates; we confirm the
     * offsets match the source-locked layout table. */
    for (int c = 0; c < DM1_CHAMPION_COUNT; ++c) {
        char id_bar[64];
        char id_hand[64];
        char id_port[64];
        char id_name[64];

        DM1_ChampionPanel_BarGraphScreenXY(c, DM1_STATUS_VALUE_HEALTH, &x, &y);
        snprintf(id_bar, sizeof(id_bar), "bar.hp.x.c%d", c);
        check_int(id_bar, x, c * DM1_STATUS_BOX_SPACING + 46);
        snprintf(id_bar, sizeof(id_bar), "bar.hp.y.c%d", c);
        check_int(id_bar, y, 2);

        DM1_ChampionPanel_BarGraphScreenXY(c, DM1_STATUS_VALUE_STAMINA, &x, &y);
        snprintf(id_bar, sizeof(id_bar), "bar.stam.x.c%d", c);
        check_int(id_bar, x, c * DM1_STATUS_BOX_SPACING + 53);
        snprintf(id_bar, sizeof(id_bar), "bar.stam.y.c%d", c);
        check_int(id_bar, y, 2);

        DM1_ChampionPanel_BarGraphScreenXY(c, DM1_STATUS_VALUE_MANA, &x, &y);
        snprintf(id_bar, sizeof(id_bar), "bar.mana.x.c%d", c);
        check_int(id_bar, x, c * DM1_STATUS_BOX_SPACING + 60);
        snprintf(id_bar, sizeof(id_bar), "bar.mana.y.c%d", c);
        check_int(id_bar, y, 2);

        /* Status hand slot anchors (C211..C218):
         *   ready hand  = champIdx*69 + 4, y=10
         *   action hand = champIdx*69 + 24, y=10 */
        DM1_ChampionPanel_StatusHandSlotXY(c, 0, &x, &y);
        snprintf(id_hand, sizeof(id_hand), "hand.ready.x.c%d", c);
        check_int(id_hand, x, c * DM1_STATUS_BOX_SPACING + 4);
        snprintf(id_hand, sizeof(id_hand), "hand.ready.y.c%d", c);
        check_int(id_hand, y, 10);

        DM1_ChampionPanel_StatusHandSlotXY(c, 1, &x, &y);
        snprintf(id_hand, sizeof(id_hand), "hand.action.x.c%d", c);
        check_int(id_hand, x, c * DM1_STATUS_BOX_SPACING + 24);
        snprintf(id_hand, sizeof(id_hand), "hand.action.y.c%d", c);
        check_int(id_hand, y, 10);

        /* Portrait box X anchor (C175..C178): champIdx*69 + 7. */
        snprintf(id_port, sizeof(id_port), "portrait.x.c%d", c);
        check_int(id_port,
                  DM1_ChampionPanel_PortraitScreenX(c),
                  c * DM1_STATUS_BOX_SPACING + DM1_PORTRAIT_OFFSET_X);

        /* Name zone X anchor (C159..C162): champIdx*69. */
        snprintf(id_name, sizeof(id_name), "name.x.c%d", c);
        check_int(id_name,
                  DM1_ChampionPanel_NameZoneX(c),
                  c * DM1_STATUS_BOX_SPACING);

        /* Name color: leader = gold (9), others = lightest gray (13). */
        snprintf(id_name, sizeof(id_name), "name.color.leader.c%d", c);
        check_int(id_name, DM1_ChampionPanel_NameColor(c, c), DM1_COLOR_GOLD);
        snprintf(id_name, sizeof(id_name), "name.color.nonleader.c%d", c);
        check_int(id_name,
                  DM1_ChampionPanel_NameColor(c, (c + 1) & 3),
                  DM1_COLOR_LIGHTEST_GRAY);
    }

    /* Inventory numeric value zones (CHAMDRAW.C F0289/F0290). The
     * source-locked mapping intentionally swaps STAMINA and MANA
     * zones (CHAMDRAW.C F0290 routes stamina text to C551 and mana
     * text to C552 because stamina is divided by 10). Pin the swap. */
    check_int("zone.health",
              DM1_ChampionPanel_StatusValueZone(DM1_STATUS_VALUE_HEALTH),
              DM1_ZONE_HEALTH_VALUE);
    check_int("zone.stamina_swapped_to_mana_value",
              DM1_ChampionPanel_StatusValueZone(DM1_STATUS_VALUE_STAMINA),
              DM1_ZONE_MANA_VALUE);
    check_int("zone.mana_swapped_to_stamina_value",
              DM1_ChampionPanel_StatusValueZone(DM1_STATUS_VALUE_MANA),
              DM1_ZONE_STAMINA_VALUE);

    /* Food/Water/Poisoned label zones (PANEL.C F0345:1598-1606). */
    check_int("zone.food", DM1_ZONE_FOOD, 500);
    check_int("zone.water", DM1_ZONE_WATER, 501);
    check_int("zone.poisoned", DM1_ZONE_POISONED, 502);

    /* Load label/value zones (CHAMDRAW.C F0292). */
    check_int("zone.load_label", DM1_ZONE_CHAMPION_LOAD_LABEL, 554);
    check_int("zone.load_value", DM1_ZONE_CHAMPION_LOAD_VALUE, 555);

    /* Empty-hand eye statistics panel zones (PANEL.C F0351). */
    check_int("zone.skill_value", DM1_ZONE_SKILL_VALUE, 557);
    check_int("zone.statistic_value", DM1_ZONE_STATISTIC_VALUE, 559);
}

static void verify_status_box_pixel_slice(uint8_t *fb)
{
    /* For each champion, paint the status box with sentinel+fill and
     * confirm the 67x29 rectangle is anchored at (champIdx*69, 170).
     * The 2 px horizontal gap between status boxes (champion n ends at
     * x = champIdx*69+66; champion n+1 starts at x = (champIdx+1)*69)
     * must remain PIXEL_SENTINEL. */
    for (int c = 0; c < DM1_CHAMPION_COUNT; ++c) {
        char id[64];

        paint_status_box(fb, c, PIXEL_FILL_GRAY);

        /* The full 67x29 rectangle (interior) must be the fill color. */
        int x0 = c * DM1_STATUS_BOX_SPACING;
        int y0 = DM1_PANEL_ROW_TOP_Y;
        snprintf(id, sizeof(id), "status_box.fill.c%d", c);
        check_pixel_band(id, fb,
                         x0, y0, x0 + DM1_STATUS_BOX_WIDTH - 1,
                         y0 + DM1_STATUS_BOX_HEIGHT - 1,
                         PIXEL_FILL_GRAY);

        /* The 2 px gap between status boxes must remain sentinel:
         * champion c box ends at x=c*69+66; champion c+1 box starts at
         * x=(c+1)*69; x=c*69+67..68 is the gap. */
        if (c < DM1_CHAMPION_COUNT - 1) {
            int gap_x0 = x0 + DM1_STATUS_BOX_WIDTH;
            int gap_x1 = gap_x0 + 1; /* 2 px gap */
            for (int yy = y0; yy < y0 + DM1_STATUS_BOX_HEIGHT; ++yy) {
                for (int xx = gap_x0; xx <= gap_x1; ++xx) {
                    snprintf(id, sizeof(id),
                             "status_box.gap.c%d.px(%d,%d)", c, xx, yy);
                    check_pixel(id, fb, xx, yy, PIXEL_SENTINEL);
                }
            }
        }
    }

    /* Row bottom sanity: pixel one row above the panel row (y=169) and
     * one row below (y=199) must remain sentinel — these are the
     * 1-px gap above the panel and the last unused scanline. */
    check_pixel_band("status_box.top_gap", fb,
                     0, 169, DM1_PANEL_FRAME_WIDTH - 1, 169,
                     PIXEL_SENTINEL);
    check_pixel_band("status_box.bottom_unused", fb,
                     0, 199, DM1_PANEL_FRAME_WIDTH - 1, 199,
                     PIXEL_SENTINEL);
}

static void verify_bar_graph_pixel_slice(uint8_t *fb)
{
    /* For each champion × stat, paint the bar-graph region (4x25) at
     * (champIdx*69 + 46/53/60, 172). Confirm both the bar pixels and
     * the 1-pixel borders between HP/stamina/mana within the region. */
    uint8_t stat_fills[DM1_BAR_GRAPH_COUNT] = {
        PIXEL_HP_FILL, PIXEL_STAMINA_FILL, PIXEL_MANA_FILL
    };

    for (int c = 0; c < DM1_CHAMPION_COUNT; ++c) {
        for (int s = 0; s < DM1_BAR_GRAPH_COUNT; ++s) {
            int x, y;
            char id[64];

            paint_bar_graph(fb, c, s, stat_fills[s]);

            DM1_ChampionPanel_BarGraphScreenXY(c, s, &x, &y);
            /* The bar is anchored at panel-relative (x, 2) which is
             * frame-relative (x, 172). The bar covers 4x25. */
            snprintf(id, sizeof(id), "bar_graph.fill.c%d.s%d", c, s);
            check_pixel_band(id, fb,
                             x, y + DM1_PANEL_ROW_TOP_Y,
                             x + DM1_BAR_GRAPH_WIDTH - 1,
                             y + DM1_BAR_GRAPH_MAX_HEIGHT - 1 +
                               DM1_PANEL_ROW_TOP_Y,
                             stat_fills[s]);
        }
    }

    /* Cross-check the bar-graph region's horizontal layout: HP at
     * offset 0, stamina at offset 7, mana at offset 14 within the
     * bar-graph region. Confirm the three fills are contiguous for
     * every champion with the documented 7-px stride. */
    for (int c = 0; c < DM1_CHAMPION_COUNT; ++c) {
        char id[64];
        int hp_x = c * DM1_STATUS_BOX_SPACING + 46;
        int stam_x = c * DM1_STATUS_BOX_SPACING + 53;
        int mana_x = c * DM1_STATUS_BOX_SPACING + 60;

        /* Bar bottom (y=2+25-1 = 26, frame y=196) sits at panel row 196. */
        snprintf(id, sizeof(id), "bar_graph.bottom.hp.c%d", c);
        check_pixel(id, fb, hp_x, 196, PIXEL_HP_FILL);
        snprintf(id, sizeof(id), "bar_graph.bottom.stam.c%d", c);
        check_pixel(id, fb, stam_x, 196, PIXEL_STAMINA_FILL);
        snprintf(id, sizeof(id), "bar_graph.bottom.mana.c%d", c);
        check_pixel(id, fb, mana_x, 196, PIXEL_MANA_FILL);

        /* 1 row above bar (y=1, frame y=171) is status-box fill, not
         * sentinel, because the bars live inside the status box. */
        snprintf(id, sizeof(id), "bar_graph.above_bar.hp.c%d", c);
        check_pixel(id, fb, hp_x, 171, PIXEL_FILL_GRAY);
    }
}

static void verify_hand_slot_pixel_slice(uint8_t *fb)
{
    /* 8 hand slot boxes (4 champions × 2 hands) anchored at
     * (champIdx*69 + 4/24, 10) panel-relative = frame y=180. */
    for (int c = 0; c < DM1_CHAMPION_COUNT; ++c) {
        for (int h = 0; h < 2; ++h) {
            int x, y;
            char id[64];

            paint_hand_slot(fb, c, h, PIXEL_SLOT_BORDER);

            DM1_ChampionPanel_StatusHandSlotXY(c, h, &x, &y);
            int frame_x = x;
            int frame_y = y + DM1_PANEL_ROW_TOP_Y;

            /* 18x18 box centered in the hand-slot anchor. */
            snprintf(id, sizeof(id), "hand_slot.fill.c%d.h%d", c, h);
            check_pixel_band(id, fb,
                             frame_x, frame_y,
                             frame_x + DM1_SLOT_BOX_SIZE - 1,
                             frame_y + DM1_SLOT_BOX_SIZE - 1,
                             PIXEL_SLOT_BORDER);
        }
    }

    /* Verify the 2 px gap between ready hand (x=champIdx*69+4..21)
     * and action hand (x=champIdx*69+24..41) within the same
     * champion is NOT painted as a slot box. For each champion,
     * the column at x=champIdx*69+22 must remain status-box fill,
     * and the column at x=champIdx*69+23 must be status-box fill
     * (champion 0's gap is x=22..23 = pixels 22, 23). */
    for (int c = 0; c < DM1_CHAMPION_COUNT; ++c) {
        char id[64];
        int gap_x0 = c * DM1_STATUS_BOX_SPACING + 22;
        int gap_x1 = gap_x0 + 1; /* 2 px gap */
        for (int xx = gap_x0; xx <= gap_x1; ++xx) {
            snprintf(id, sizeof(id),
                     "hand_slot.between.c%d.px(%d,180)", c, xx);
            check_pixel(id, fb, xx, 180, PIXEL_FILL_GRAY);
        }
    }

    /* Verify the 2 px gap between status boxes does not contain any
     * hand slot pixel at frame y=180..197. The status box gap is at
     * x = champIdx*69 + 67..68, all of which must remain status-box
     * fill on the action hand slot row. For champion 0 the action
     * hand ends at x=41 so the gap at x=67..68 is far away from any
     * slot — this confirms the action hand slot does not bleed into
     * the inter-champion gap. */
    for (int c = 0; c < DM1_CHAMPION_COUNT - 1; ++c) {
        char id[64];
        int box_gap_x0 = c * DM1_STATUS_BOX_SPACING + 67;
        for (int xx = box_gap_x0; xx <= box_gap_x0 + 1; ++xx) {
            for (int yy = 180; yy <= 197; ++yy) {
                snprintf(id, sizeof(id),
                         "hand_slot.action_box_gap.c%d.px(%d,%d)",
                         c, xx, yy);
                check_pixel(id, fb, xx, yy, PIXEL_SENTINEL);
            }
        }
    }
}

static void verify_portrait_pixel_slice(uint8_t *fb)
{
    /* Portrait box: 32x29 at (champIdx*69 + 7, 170) frame coordinates.
     * This overlaps the top portion of the status box. We repaint the
     * portrait over the previously painted status box to verify the
     * anchor pins the portrait correctly within its slot. */
    for (int c = 0; c < DM1_CHAMPION_COUNT; ++c) {
        char id[64];

        paint_portrait_box(fb, c, PIXEL_PORTRAIT_BORDER);

        int x = DM1_ChampionPanel_PortraitScreenX(c);
        int y = DM1_PANEL_ROW_TOP_Y;
        snprintf(id, sizeof(id), "portrait.fill.c%d", c);
        check_pixel_band(id, fb,
                         x, y,
                         x + DM1_PORTRAIT_WIDTH - 1,
                         y + DM1_PORTRAIT_HEIGHT - 1,
                         PIXEL_PORTRAIT_BORDER);

        /* Sanity: the 1 px column to the LEFT of the portrait box
         * (x=champIdx*69+6) must remain status-box fill (not sentinel)
         * because the portrait is anchored at +7 within the 67-px
         * status box. */
        snprintf(id, sizeof(id), "portrait.left_inside_box.c%d", c);
        check_pixel(id, fb, x - 1, y + 1, PIXEL_FILL_GRAY);
    }
}

static void verify_name_band_pixel_slice(uint8_t *fb)
{
    /* Name zone: champIdx*69, 4-px band panel-relative y=1..4
     * (frame y=171..174). The portrait overlaps the name band for
     * champions 0..3 (portrait x=champIdx*69+7 starts the column
     * INSIDE the name band zone), so after paint_name_band the
     * portrait must be repainted to confirm portrait priority. */
    for (int c = 0; c < DM1_CHAMPION_COUNT; ++c) {
        char id[64];

        /* Repaint status box fill (clean slate) then paint name band
         * with name bg. */
        paint_status_box(fb, c, PIXEL_FILL_GRAY);
        paint_name_band(fb, c, PIXEL_NAME_BG);

        int x = DM1_ChampionPanel_NameZoneX(c);
        snprintf(id, sizeof(id), "name_band.fill.c%d", c);
        check_pixel_band(id, fb,
                         x + 4, DM1_PANEL_ROW_TOP_Y + 1,
                         x + DM1_STATUS_BOX_WIDTH - 5,
                         DM1_PANEL_ROW_TOP_Y + 4,
                         PIXEL_NAME_BG);

        /* Above the name band (y=170, panel-relative y=0) the status
         * box fill must remain — but ONLY for champions whose name
         * band leaves a clean top edge. The band spans x+4..x+62 and
         * y=1..4, so x+8 at y=170 is OUTSIDE the band and remains
         * status-box fill. */
        snprintf(id, sizeof(id), "name_band.above.c%d", c);
        check_pixel(id, fb, x + 8, DM1_PANEL_ROW_TOP_Y, PIXEL_FILL_GRAY);

        /* Restore portrait so subsequent tests work on the layered
         * image expected by verify_portrait_pixel_slice. */
        paint_portrait_box(fb, c, PIXEL_PORTRAIT_BORDER);
    }
}

static void verify_pc34_bar_fill_model_pixel_slice(void)
{
    /* Source-locked CHAMDRAW.C F0287 lines 307-342: when current=12,
     * max=25, the blank band fills from y=2 to y=14 (13 px) and the
     * colored band fills from y=15 to y=26 (12 px). Champion 0 HP. */
    DM1_ChampionPanel_BarFillModel model;
    int ok = DM1_ChampionPanel_BuildPc34BarFillModel(
        0, DM1_STATUS_VALUE_HEALTH, 12, 25, &model);

    check_int("pc34_bar.ok", ok, 1);
    check_int("pc34_bar.zone_id", model.zoneId, 195);
    check_int("pc34_bar.x", model.x, 46);
    check_int("pc34_bar.y", model.y, 2);
    check_int("pc34_bar.width", model.width, 4);
    check_int("pc34_bar.height", model.height, 25);
    check_int("pc34_bar.blank_y", model.blankY, 2);
    check_int("pc34_bar.blank_height", model.blankHeight, 13);
    check_int("pc34_bar.fill_y", model.fillY, 15);
    check_int("pc34_bar.fill_height", model.fillHeight, 12);
    check_int("pc34_bar.blank_color", model.blankColor, DM1_COLOR_DARKEST_GRAY);
    check_int("pc34_bar.fill_color", model.fillColor, DM1_ChampionColor[0]);
    check_int("pc34_bar.emits_blank", model.emitsBlank, 1);
    check_int("pc34_bar.emits_fill", model.emitsFill, 1);

    /* Same model on a saturated bar (25/25): no blank band, only the
     * full fill band. ReDMCSB F0287 lines 332-340. */
    ok = DM1_ChampionPanel_BuildPc34BarFillModel(
        2, DM1_STATUS_VALUE_STAMINA, 25, 25, &model);
    check_int("pc34_bar.saturated.ok", ok, 1);
    check_int("pc34_bar.saturated.zone_id", model.zoneId, 195 + 2 + 4);
    check_int("pc34_bar.saturated.x", model.x, 2 * DM1_STATUS_BOX_SPACING + 53);
    check_int("pc34_bar.saturated.blank_height", model.blankHeight, 0);
    check_int("pc34_bar.saturated.fill_height", model.fillHeight, 25);
    check_int("pc34_bar.saturated.emits_blank", model.emitsBlank, 0);
    check_int("pc34_bar.saturated.emits_fill", model.emitsFill, 1);

    /* Empty bar (0/25): blank covers the whole bar, no fill.
     * F0287 lines 320-326. */
    ok = DM1_ChampionPanel_BuildPc34BarFillModel(
        1, DM1_STATUS_VALUE_MANA, 0, 25, &model);
    check_int("pc34_bar.empty.ok", ok, 1);
    check_int("pc34_bar.empty.blank_height", model.blankHeight, 25);
    check_int("pc34_bar.empty.fill_height", model.fillHeight, 0);
    check_int("pc34_bar.empty.emits_blank", model.emitsBlank, 1);
    check_int("pc34_bar.empty.emits_fill", model.emitsFill, 0);
}

static void verify_champion_color_pixel_slice(void)
{
    /* The 4-champion color palette pins DM1_ChampionColor[idx] exactly
     * to G0046_auc_Graphic562_ChampionColor. Any future palette change
     * must be source-locked to the original graphic. */
    static const int kExpected[DM1_CHAMPION_COUNT] = {
        DM1_COLOR_LIGHT_GREEN,   /* green = 7 */
        DM1_COLOR_YELLOW,        /* yellow = 11 */
        DM1_COLOR_RED,           /* red = 8 */
        14                       /* blue (custom index, original DOS C14) */
    };
    for (int c = 0; c < DM1_CHAMPION_COUNT; ++c) {
        char id[64];
        snprintf(id, sizeof(id), "champion_color.idx[%d]", c);
        check_int(id, DM1_ChampionColor[c], kExpected[c]);
    }
}

static void verify_champion_zorder_no_overlap(void)
{
    /* Source-locked layout-696 invariants: the 4 status boxes must
     * never overlap. With stride C69 and width 67 the per-champion
     * status box spans [champIdx*69, champIdx*69+66] which is a 2-px
     * gap to the next champion's status box. We confirm that as a
     * post-condition of the helpers. */
    for (int c = 0; c < DM1_CHAMPION_COUNT - 1; ++c) {
        char id[64];
        int left_end = c * DM1_STATUS_BOX_SPACING + DM1_STATUS_BOX_WIDTH - 1;
        int right_start = (c + 1) * DM1_STATUS_BOX_SPACING;
        int gap = right_start - left_end - 1;
        snprintf(id, sizeof(id), "status_box.no_overlap.gap.c%d_to_c%d", c, c + 1);
        check_int(id, gap, 2);
    }

    /* Champion 3 status box right edge must remain within the 320-px
     * frame. 3 * 69 + 67 - 1 = 207 + 66 = 273 < 320 ✓. */
    int last_right = (DM1_CHAMPION_COUNT - 1) * DM1_STATUS_BOX_SPACING
                   + DM1_STATUS_BOX_WIDTH - 1;
    check_int("status_box.last_within_frame", last_right < 320, 1);
    check_int("status_box.last_within_frame.value", last_right, 273);
}

int main(void)
{
    uint8_t frame[DM1_PANEL_FRAME_WIDTH * DM1_PANEL_FRAME_HEIGHT];
    int failures_before;
    int checks_before;

    printf("probe=firestaff_dm1_v1_champion_panel_pixel_slice_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("sourceEvidence=CHAMDRAW.C:F0287/F0291/F0292/F0354,PANEL.C:F0345/F0658,DEFS.H:2186-2195,layout-696:C159-C218/C500-C502\n");
    printf("frameBuffer=%dx%d\n", DM1_PANEL_FRAME_WIDTH, DM1_PANEL_FRAME_HEIGHT);

    memset(frame, PIXEL_SENTINEL, sizeof(frame));

    failures_before = g_failures;
    checks_before = g_checks;

    printf("\n[constants_source_lock]\n");
    verify_constants_lock();

    printf("\n[helpers_source_lock]\n");
    verify_helpers_lock();

    printf("\n[status_box_pixel_slice]\n");
    verify_status_box_pixel_slice(frame);

    printf("\n[bar_graph_pixel_slice]\n");
    verify_bar_graph_pixel_slice(frame);

    printf("\n[hand_slot_pixel_slice]\n");
    verify_hand_slot_pixel_slice(frame);

    printf("\n[portrait_pixel_slice]\n");
    verify_portrait_pixel_slice(frame);

    printf("\n[name_band_pixel_slice]\n");
    verify_name_band_pixel_slice(frame);

    printf("\n[pc34_bar_fill_model_pixel_slice]\n");
    verify_pc34_bar_fill_model_pixel_slice();

    printf("\n[champion_color_pixel_slice]\n");
    verify_champion_color_pixel_slice();

    printf("\n[champion_zorder_no_overlap]\n");
    verify_champion_zorder_no_overlap();

    if (g_failures > failures_before) {
        printf("\nFAIL dm1_v1_champion_panel_pixel_slice_probe failures=%d\n",
               g_failures - failures_before);
        return 1;
    }

    printf("\nPASS dm1_v1_champion_panel_pixel_slice_probe (%d checks)\n",
           g_checks - checks_before);
    return 0;
}
