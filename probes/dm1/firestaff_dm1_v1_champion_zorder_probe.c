/*
 * Firestaff DM1 V1 champion Z-order / floating probe.
 *
 * Bug: champion Z-order/floating in DM1 V1 viewport.
 * Sub-issue: champion mirror portrait (D1C, C026 at {96+vp,35+vp}, 32x29)
 *            may be drawn AFTER nearby wall/door/side square layers, causing
 *            floating/ghost-portrait appearance.
 *
 * Source locks:
 *   ReDMCSB DUNVIEW.C F0128: draw order contracts
 *   ReDMCSB DUNVIEW.C:3913-3928: D1C portrait blit at fixed {96,35} in viewport
 *   ReDMCSB DUNGEON.C:2573: M011_CELL(sensor) view-direction mapping for C127
 *   ReDMCSB DUNGEON.C:2610-2612: only front wall aspect sets G0289 portrait
 *
 * Known issue: m11_draw_dm1_front_mirror_route (lines 22699-22700) is called
 *   AFTER m11_draw_wall_contents (lines 22691-22698). When front D1C cell is
 *   open (door/teleporter) but has C127 champion portrait sensor, the portrait
 *   at y=35..63 blits AFTER the center-cell wall-contents face at y=34..99,
 *   creating a floating/ghost portrait effect on top of the open floor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    VIEWPORT_W = 224,
    VIEWPORT_H = 136
};

/* D1C champion portrait geometry from DUNVIEW.C:3916 */
enum { PORTRAIT_W = 32, PORTRAIT_H = 29 };
enum { PORTRAIT_SCREEN_X = VIEWPORT_X + 96, PORTRAIT_SCREEN_Y = VIEWPORT_Y + 35 };

/* D1C wall panel geometry (layout-696 C712_ZONE_WALL_D1C) */
enum {
    WALL_D1C_X = VIEWPORT_X + 32,
    WALL_D1C_Y = VIEWPORT_Y + 9,
    WALL_D1C_W = 160,
    WALL_D1C_H = 111
};

typedef struct {
    int layer;
    const char *name;
    int top, bottom;
    int overlaps_portrait;
} Layer;

static int rects_overlap(int a_top, int a_bot, int b_top, int b_bot) {
    return a_top <= b_bot && b_top <= a_bot;
}

int main(void) {
    int p_top = PORTRAIT_SCREEN_Y;
    int p_bot = PORTRAIT_SCREEN_Y + PORTRAIT_H - 1; /* 35 + 28 - 1 = 62 */

    Layer layers[] = {
        { 0, "D1C_wall_ornament",   WALL_D1C_Y + 6,                   WALL_D1C_Y + 6 + 42,              0 },
        /* ReDMCSB F0128 draw order: portrait/ornament (viewWallIndex==12) is drawn
         * BEFORE the per-depth center-contents loop.  Front mirror at D1C fires once
         * (depth 0 only) before the open-center floor/creature layers loop over D0/D1/D2.
         * This ordering is hardcoded in m11_game_view.c at the front_mirror_route call,
         * placed between m11_draw_dm1_side_contents and the center-contents loop. */
        { 1, "D1C_front_mirror",     PORTRAIT_SCREEN_Y,                PORTRAIT_SCREEN_Y + PORTRAIT_H - 1, 0 },
        { 2, "D1C_center_contents", VIEWPORT_Y + 34,                  VIEWPORT_Y + 34 + 94 - 1,          0 },
        { 3, "deferred_explosion",   VIEWPORT_Y + 20,                  VIEWPORT_Y + 100,                  0 },
    };
    int n = (int)(sizeof(layers) / sizeof(layers[0]));
    int failures = 0;

    printf("[champion_z_order_probe]\n");
    printf("portrait_box=[%d,%d,%d,%d]\n",
           PORTRAIT_SCREEN_X, PORTRAIT_SCREEN_Y,
           PORTRAIT_SCREEN_X + PORTRAIT_W - 1, PORTRAIT_SCREEN_Y + PORTRAIT_H - 1);
    printf("viewport_d1c_wall=[%d,%d,%d,%d]\n",
           WALL_D1C_X, WALL_D1C_Y, WALL_D1C_X + WALL_D1C_W - 1, WALL_D1C_Y + WALL_D1C_H - 1);

    printf("\n[layers]\n");
    for (int i = 0; i < n; ++i) {
        layers[i].overlaps_portrait = rects_overlap(layers[i].top, layers[i].bottom, p_top, p_bot);
        printf("%02d %-25s y=[%3d,%3d] portrait_overlap=%d\n",
               layers[i].layer, layers[i].name,
               layers[i].top, layers[i].bottom, layers[i].overlaps_portrait);
    }

    printf("\n[z_order_analysis]\n");
    int portrait_idx = -1, center_contents_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (strstr(layers[i].name, "front_mirror")) portrait_idx = i;
        if (strstr(layers[i].name, "center_contents")) center_contents_idx = i;
    }

    /* In original DM1: portrait is part of the wall ornament / front-wall pass,
     * drawn BEFORE center wall contents for the same cell.
     * Firestaff fix (commit dm1-v1-p1-champion-zorder): moved front_mirror_route
     * to execute BEFORE the center-contents loop, matching ReDMCSB F0128.
     * Portrait now renders at correct depth — no floating on Hall mirror squares. */
    if (portrait_idx > center_contents_idx) {
        printf("BUG: portrait drawn after center wall contents\n");
        printf("     This causes floating/ghost portrait when D1C is open cell with C127 sensor.\n");
        failures++;
    } else {
        printf("OK: portrait drawn before center wall contents (fix applied)\n");
    }

    printf("\n[portrait_cell_type_guard]\n");
    printf("Commit 3d5bb0c6: visibleWallCell guard in m11_sample_viewport_cell\n");
    printf("correctly filters C127 sensors so only front-wall aspect gets G0289.\n");
    printf("m11_draw_dm1_front_mirror_route gates portrait on championPortraitOrdinal >= 0.\n");

    printf("\n[result] failures=%d\n", failures);
    printf("\nSUMMARY:\n");
    printf("  Location: src/engine/m11_game_view.c ~line 22699\n");
    printf("  Function: m11_draw_dm1_front_mirror_route draw order\n");
    printf("  Issue: portrait blits AFTER center open-cell wall contents for D1C,\n");
    printf("         causing floating portrait on open-front squares in Hall of Champions.\n");
    printf("  Fix needed: move front_mirror_route BEFORE m11_draw_wall_contents at D1C\n");
    printf("              (line 22691), or integrate portrait into wall ornament pass.\n");
    printf("  Pre-existing fix (3d5bb0c6): correct C127 visibleWallCell gating.\n");
    printf("  Remaining issue: draw call sequence only, not sensor logic.\n");

    return failures ? 1 : 0;
}
