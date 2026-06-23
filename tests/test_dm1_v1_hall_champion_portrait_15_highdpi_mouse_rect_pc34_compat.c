/*
 * test_dm1_v1_hall_champion_portrait_15_highdpi_mouse_rect_pc34_compat.c
 *
 * DM1 V1 Hall of Champions portrait ordinal 15 (MOPHUS, "THE HEALER")
 * high-DPI / window-scale mouse-rect verification gate.
 *
 * Source evidence:
 *   - DUNGEON.C:2573 C127 sensor cell match against view dir
 *   - DUNGEON.C:2608-2612 G0289 champion portrait ordinal
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall =
 *     {96, 127, 35, 63}
 *   - DUNVIEW.C:3913-3928 C026 portrait blit at the D1C portrait
 *     rect (96, 35, 32, 29) in viewport coords
 *   - DUNVIEW.C:3916-3919 C026 atlas 8 cols x 3 rows of 32x29
 *     portraits; ordinal 15 -> (15 & 7 = 7) * 32 = 224 srcX,
 *     (15 >> 3 = 1) * 29 = 29 srcY
 *   - COORD.C:1693-1749 PC34 viewport origin and portrait dims
 *   - DEFS.H:821-826 M027/M028 portrait-grid macro math
 *   - DEFS.H:3979-3982 I34E M664/M665/M666 -> layout-696 zones
 *     570/571/573 (panel.resurrect / panel.reincarnate / panel.cancel)
 *   - COMMAND.C:228-233 / 508-511 maps C160/C161/C162 to
 *     M664/M665/M666 (panel buttons), and to M11_GameView_Confirm
 *     MirrorCandidate(0/1) and M11_GameView_CancelMirrorCandidate
 *     in m11_game_view.c:8756-8771
 *   - COMMAND.C:1379-1439 F0358_COMMAND_GetCommandFromMouseInput_CPSC
 *     matches normalized 320x200 screen coords against the active
 *     primary/secondary mouse tables, with the C160/C161/C162 panel
 *     boxes handled before the generic dispatch in m11_game_view.c:8747-8771
 *   - COMMAND.C:1641-1644 source-orders primary mouse input before
 *     secondary mouse input
 *   - TOUCHCLICK_Compat_NormalizeLetterboxedPointPc34Compat is the
 *     HiDPI scaled-mouse inverse for both the screen and the
 *     viewport letterbox paths
 *   - M11_Render_MapPointToFramebuffer is the SDL render framebuffer
 *     map used for HiDPI pixel -> 320x200 source coords
 *
 * Test scope (narrow HiDPI regression for ordinal 15):
 *
 *   The candidate panel is the source-locked overlay drawn on top
 *   of the D1C portrait rect when a Hall of Champions mirror sensor
 *   produces a candidate.  Once the panel is live, the source-locked
 *   COMMAND.C C160/C161/C162 panel boxes are routed in
 *   m11_game_view.c:8755-8771 BEFORE any generic viewport dispatch:
 *
 *     m11_point_in_rect(x, y, 104, 86, 55, 57) -> C160 resurrect
 *     m11_point_in_rect(x, y, 163, 86, 55, 57) -> C161 reincarnate
 *     m11_point_in_rect(x, y, 104, 146, 114, 11) -> C162 cancel
 *
 *   These boxes are in the 320x200 source framebuffer coord space.
 *   On macOS Retina / HiDPI the SDL mouse event arrives in pixel
 *   (drawable) coords, e.g. window=1920x1080 framebuffer=960x540
 *   or window=2560x1440 framebuffer=1280x720 etc.  The
 *   TOUCHCLICK_Compat_NormalizeLetterboxedPointPc34Compat family
 *   maps those physical pixels back into either the screen-relative
 *   320x200 coords (for the source-ordered primary/secondary scan)
 *   or the viewport-relative 224x136 coords (for the panel and
 *   inventory slot subtables).
 *
 *   The HiDPI mouse-rect regression is therefore twofold:
 *
 *     1) The viewport-letterbox inverse MUST land a physical-pixel
 *        click on the source-locked panel.resurrect / panel
 *        .reincarnate / panel.cancel boxes when the click is taken
 *        from the screen-letterbox math, AND vice versa (the screen
 *        inverse lands inside the source screen-relative panel
 *        boxes (104..158, 86..142), (163..217, 86..142), and
 *        (104..217, 146..156)).
 *
 *     2) Off-by-one source-point clicks just outside the panel box
 *        MUST NOT dispatch to the panel buttons.  A regression that
 *        widens the panel hit-test by one source pixel would let a
 *        click one pixel left of panel.resurrect route to C160
 *        (resurrect) instead of the surrounding panel-deadzone.
 *        This is exactly the bug class the existing HiDPI chest-slot
 *        gate locks for chest_1..chest_8.
 *
 *   This gate is the panel-button counterpart to the chest-slot
 *   HiDPI gate.  It locks the panel.resurrect / panel.reincarnate /
 *   panel.cancel box geometry under HiDPI scaling for the ordinal
 *   15 candidate-panel state, which is the panel route variant the
 *   task spec calls highdpi_mouse_rect.  It does NOT relitigate
 *   pixel parity of the D1C portrait rect itself (that contract is
 *   covered by:
 *     - firestaff_dm1_v1_hall_of_champions_portrait_15_cancel_reopen
 *       _portrait_rect_position_runtime_probe
 *     - firestaff_dm1_v1_champion_mirror_ordinal_15_west_negative
 *       _portrait_rect_position_runtime_probe
 *     - firestaff_dm1_v1_hoc_mophus_ordinal15_unreachable_probe
 *   ).
 *
 *   The gate is also disjoint from the existing
 *   test_dm1_v1_hidpi_chest_slot_hit_zone_pc34_compat, which only
 *   exercises chest_1 / chest_8 HiDPI scaling.  The panel boxes
 *   live in a different M11 zone layout (M664/M665/M666 in DEFS.H
 *   3979-3982 -> layout-696 zones 570/571/573) and are dispatched
 *   by the C160/C161/C162 source-locked route before any chest slot
 *   subtable scan.
 *
 *   Disjoint from existing portrait-15 probes:
 *     - portrait_15_cancel_reopen: portrait_rect_position +
 *       select/cancel/select cycle; no HiDPI scaled mouse input.
 *     - ordinal_15_west_negative: portrait_rect_position +
 *       corridor west_negative; no panel dispatch.
 *     - hoc_mophus_ordinal15_unreachable: ordinal 15 east_walkpath
 *       slice; no panel dispatch.
 *
 *   Source-viewport invariant: the viewport rect is (0, 33, 224,
 *   136) per COORD.C; HiDPI scaling must not bleed into the source
 *   view (the inverse normalize is computed against the live
 *   surface, but the source viewport extents are locked).
 */

#include "render_sdl_m11.h"
#include "touch_click_zone_matrix_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_pass = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++g_failures; \
    } else { \
        ++g_pass; \
    } \
} while (0)

static int dispatch_scaled_viewport(int physicalX,
                                    int physicalY,
                                    int surfaceW,
                                    int surfaceH,
                                    TouchClickDispatchPc34Compat* outDispatch) {
    return TOUCHCLICK_Compat_MapScaledViewportPointToDispatch(physicalX,
                                                              physicalY,
                                                              surfaceW,
                                                              surfaceH,
                                                              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                                              outDispatch);
}

static int dispatch_scaled_screen(int physicalX,
                                  int physicalY,
                                  int surfaceW,
                                  int surfaceH,
                                  TouchClickDispatchPc34Compat* outDispatch) {
    return TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(physicalX,
                                                            physicalY,
                                                            surfaceW,
                                                            surfaceH,
                                                            TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                                            outDispatch);
}

/*
 * Panel button source geometry (COMMAND.C:228-233, 508-511, DEFS.H
 * 3979-3982, layout-696 C569..C573, m11_game_view.c:8755-8771):
 *
 *   panel.resurrect   viewport (104, 53, 55, 57)  screen (104, 86, 55, 57)
 *                     commandId=160 zoneIndex=570
 *   panel.reincarnate viewport (163, 53, 55, 57)  screen (163, 86, 55, 57)
 *                     commandId=161 zoneIndex=571
 *   panel.cancel      viewport (104,113,114,11)   screen (104,146,114, 11)
 *                     commandId=162 zoneIndex=573
 *
 * Centers:
 *   panel.resurrect:   vp (131, 81)  sc (131, 114)
 *   panel.reincarnate: vp (190, 81)  sc (190, 114)
 *   panel.cancel:      vp (161, 118) sc (161, 151)
 *
 * Pre-computed physical pixels per surface for the viewport path:
 *
 *   surface       | vp res | vp rei | vp can
 *   --------------|--------|--------|---------
 *   1280 x 720    |  (741, 429) | (1053, 429) |  (899, 625)
 *   1920 x 1080   | (1111, 644) | (1580, 644) | (1349, 938)
 *   2560 x 1440   | (1481, 858) | (2106, 858) | (1799,1250)
 *   1512 x 982    |  (885, 579) | (1283, 579) | (1087, 829)
 *
 * Pre-computed physical pixels per surface for the screen path:
 *
 *   surface       | sc res | sc rei | sc can
 *   --------------|--------|--------|---------
 *   1280 x 720    |  (536, 411) |  (748, 411) |  (644, 544)
 *   1920 x 1080   |  (804, 616) | (1122, 616) |  (966, 816)
 *   2560 x 1440   | (1072, 821) | (1496, 821) | (1288,1088)
 *   1512 x 982    |  (619, 557) |  (898, 557) |  (761, 732)
 *
 * The "vp" column maps via the viewport-letterbox (224x136) used
 * by MapScaledViewportPointToDispatch. The "sc" column maps via the
 * screen-letterbox (320x200) used by MapScaledScreenPointToDispatch
 * and M11_Render_MapPointToFramebuffer. The two letterboxes use
 * different drawW/drawH/drawX/drawY, so the surface pixel for the
 * same source point differs.
 */
static void expect_resurrect_viewport(int surfaceW, int surfaceH,
                                      int physicalX, int physicalY) {
    TouchClickDispatchPc34Compat dispatch;
    CHECK(dispatch_scaled_viewport(physicalX, physicalY,
                                   surfaceW, surfaceH, &dispatch) == 1);
    CHECK(dispatch.commandId == 160u);
    CHECK(dispatch.zoneIndex == 570u);
    CHECK(dispatch.coordMode == TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT);
    CHECK(strcmp(dispatch.groupName, "panel.resurrect") == 0);
    /* MapScaledViewportPointToDispatch promotes viewport-local to
     * screen-local by adding sourceViewportY=33. The dispatched
     * screenX/screenY is therefore the screen-local point inside
     * the panel.resurrect box [104..158, 86..142]. */
    CHECK(dispatch.screenX >= 104 && dispatch.screenX <= 158);
    CHECK(dispatch.screenY >= 86 && dispatch.screenY <= 142);
}

static void expect_reincarnate_viewport(int surfaceW, int surfaceH,
                                        int physicalX, int physicalY) {
    TouchClickDispatchPc34Compat dispatch;
    CHECK(dispatch_scaled_viewport(physicalX, physicalY,
                                   surfaceW, surfaceH, &dispatch) == 1);
    CHECK(dispatch.commandId == 161u);
    CHECK(dispatch.zoneIndex == 571u);
    CHECK(dispatch.coordMode == TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT);
    CHECK(strcmp(dispatch.groupName, "panel.reincarnate") == 0);
    CHECK(dispatch.screenX >= 163 && dispatch.screenX <= 217);
    CHECK(dispatch.screenY >= 86 && dispatch.screenY <= 142);
}

static void expect_cancel_viewport(int surfaceW, int surfaceH,
                                   int physicalX, int physicalY) {
    TouchClickDispatchPc34Compat dispatch;
    CHECK(dispatch_scaled_viewport(physicalX, physicalY,
                                   surfaceW, surfaceH, &dispatch) == 1);
    CHECK(dispatch.commandId == 162u);
    CHECK(dispatch.zoneIndex == 573u);
    CHECK(dispatch.coordMode == TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT);
    CHECK(strcmp(dispatch.groupName, "panel.cancel") == 0);
    CHECK(dispatch.screenX >= 104 && dispatch.screenX <= 217);
    CHECK(dispatch.screenY >= 146 && dispatch.screenY <= 156);
}

/*
 * Screen path: MapScaledScreenPointToDispatch uses the source-ordered
 * primary/secondary scan (COMMAND.C:1641-1644) and the panel.resurrect /
 * panel.reincarnate / panel.cancel zones are VIEWPORT_RELATIVE, so the
 * primary scan does not hit them directly.  The same physical pixel
 * that hits panel.resurrect via the viewport path therefore routes
 * through the primary scan to one of the screen-relative zones that
 * overlap the same screen pixel (e.g. viewport.dungeon at C080 if the
 * pixel is inside the dungeon viewport band, or no dispatch if it is
 * inside the panel's screen dead-zone).  We assert the dispatch does
 * not reach panel.resurrect through the screen path - that is the
 * source-locked COMMAND.C:1641-1644 ordering: panel boxes are
 * viewport-table subroute, not screen-table primary route.
 *
 * The screen path on these surfaces lands the in-panel physical
 * pixel inside the dungeon viewport (since panel.resurrect's screen
 * box (104..158, 86..142) overlaps the viewport (0,33,224,136)
 * source coords plus viewportY=33 -> screen (104..158, 86..142) is
 * inside the screen band [0..319, 33..168]).  Therefore the screen
 * path should dispatch to viewport.dungeon (C080) at zoneIndex 7
 * (COMMAND.C:228-233 / 508-511's primary scan hits C080 via the
 * C080 source-relative screen-relative zone at (0,33,224,136)).
 *
 * To keep this test focused on the HiDPI mouse-rect contract, we
 * only assert that the screen path does NOT dispatch to
 * panel.resurrect / panel.reincarnate / panel.cancel (the boxes are
 * viewport-relative by design).  We do not assert what it does
 * dispatch to - that depends on which other source-ordered zones
 * overlap the pixel and is the responsibility of the touch-pointer
 * integration test.
 */
static void expect_screen_path_not_to_panel(int surfaceW, int surfaceH,
                                            int physicalX, int physicalY) {
    TouchClickDispatchPc34Compat dispatch;
    if (dispatch_scaled_screen(physicalX, physicalY,
                               surfaceW, surfaceH, &dispatch) == 1) {
        CHECK(strcmp(dispatch.groupName, "panel.resurrect") != 0);
        CHECK(strcmp(dispatch.groupName, "panel.reincarnate") != 0);
        CHECK(strcmp(dispatch.groupName, "panel.cancel") != 0);
        /* The screen path uses COORD_SCREEN_RELATIVE and the panel
         * boxes are COORD_VIEWPORT_RELATIVE; the source-ordered
         * primary scan only consults screen-relative zones. */
        CHECK(dispatch.coordMode != TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT);
    }
}

/*
 * Verify M11_Render_MapPointToFramebuffer (SDL render framebuffer map)
 * lands inside the source screen panel boxes from a scaled logical
 * window click.  The screen letterbox is 320x200, the same as the
 * panel source rect coords.  Resurrect center screen (131, 114),
 * reincarnate center screen (190, 114), cancel center screen (161,
 * 151).
 */
static void expect_resurrect_fb_in_box(int surfaceW, int surfaceH,
                                       int logicalX, int logicalY) {
    int fbX = -1;
    int fbY = -1;
    int rc = M11_Render_MapPointToFramebuffer(logicalX,
                                              logicalY,
                                              surfaceW,
                                              surfaceH,
                                              M11_FB_WIDTH,
                                              M11_FB_HEIGHT,
                                              M11_SCALE_FIT,
                                              0,
                                              M11_DISPLAY_ASPECT_CONTENT,
                                              &fbX,
                                              &fbY);
    CHECK(rc == 1);
    CHECK(fbX >= 104 && fbX <= 158);
    CHECK(fbY >= 86 && fbY <= 142);
}

static void expect_reincarnate_fb_in_box(int surfaceW, int surfaceH,
                                         int logicalX, int logicalY) {
    int fbX = -1;
    int fbY = -1;
    int rc = M11_Render_MapPointToFramebuffer(logicalX,
                                              logicalY,
                                              surfaceW,
                                              surfaceH,
                                              M11_FB_WIDTH,
                                              M11_FB_HEIGHT,
                                              M11_SCALE_FIT,
                                              0,
                                              M11_DISPLAY_ASPECT_CONTENT,
                                              &fbX,
                                              &fbY);
    CHECK(rc == 1);
    CHECK(fbX >= 163 && fbX <= 217);
    CHECK(fbY >= 86 && fbY <= 142);
}

static void expect_cancel_fb_in_box(int surfaceW, int surfaceH,
                                    int logicalX, int logicalY) {
    int fbX = -1;
    int fbY = -1;
    int rc = M11_Render_MapPointToFramebuffer(logicalX,
                                              logicalY,
                                              surfaceW,
                                              surfaceH,
                                              M11_FB_WIDTH,
                                              M11_FB_HEIGHT,
                                              M11_SCALE_FIT,
                                              0,
                                              M11_DISPLAY_ASPECT_CONTENT,
                                              &fbX,
                                              &fbY);
    CHECK(rc == 1);
    CHECK(fbX >= 104 && fbX <= 217);
    CHECK(fbY >= 146 && fbY <= 156);
}

/*
 * Off-by-one neighbor regression: a click one source pixel left of
 * panel.resurrect (vp source x=103) MUST NOT dispatch to
 * panel.resurrect / panel.reincarnate / panel.cancel on the viewport
 * path.  A regression that widens the panel hit-test by one source
 * pixel would route the deadzone click into the panel button.
 *
 * Pre-computed physical pixels per surface (source vp x=103, y=81):
 *
 *   1280 x 720    |  (592..597, 429..434)  first (592, 429)
 *   1920 x 1080   |  (889..896, 644..651)  first (889, 644)
 *   2560 x 1440   | (1185..1194, 858..868) first (1185, 858)
 *   1512 x 982    |  (696..701, 579..585)  first (696, 579)
 *
 * We sample the first physical pixel of the off-by-one band on each
 * surface.  The inverse normalize for these pixels lands on source
 * (103, 81) - one source pixel left of panel.resurrect box
 * (104..158, 53..109).  A correct HiDPI scaled mouse-rect dispatch
 * either routes them to a different panel-table zone (e.g. an
 * inventory subtable zone at source x in [70..103]) or returns 0;
 * it MUST NOT dispatch to panel.resurrect / panel.reincarnate /
 * panel.cancel.
 */
static void expect_neighbor_left_of_resurrect_misses_panel(int surfaceW,
                                                           int surfaceH,
                                                           int physicalX,
                                                           int physicalY) {
    TouchClickDispatchPc34Compat dispatch;
    if (dispatch_scaled_viewport(physicalX, physicalY,
                                 surfaceW, surfaceH, &dispatch) == 1) {
        CHECK(strcmp(dispatch.groupName, "panel.resurrect") != 0);
        CHECK(strcmp(dispatch.groupName, "panel.reincarnate") != 0);
        CHECK(strcmp(dispatch.groupName, "panel.cancel") != 0);
    }
}

int main(void) {
    printf("probe=firestaff_dm1_v1_hall_champion_portrait_15_highdpi_mouse_rect_pc34_compat\n");
    printf("sourceEvidence=%s\n", TOUCHCLICK_Compat_GetSourceEvidence());
    printf("sourcePanelBoxes=%s\n",
           "COMMAND.C:228-233/508-511 C160/C161/C162 -> M664/M665/M666; "
           "DEFS.H:3979-3982 I34E M664/M665/M666 -> layout-696 zones 570/571/573; "
           "m11_game_view.c:8755-8771 m11_point_in_rect(104,86,55,57) / "
           "(163,86,55,57) / (104,146,114,11) before any generic dispatch; "
           "COMMAND.C:1641-1644 source-orders primary mouse input before "
           "secondary mouse input; "
           "DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall = "
           "{96, 127, 35, 63}; "
           "DUNVIEW.C:3913-3928 C026 portrait blit at the D1C portrait rect "
           "(96, 35, 32, 29); "
           "DUNVIEW.C:3916-3919 C026 atlas 8 cols x 3 rows of 32x29 portraits; "
           "DEFS.H:821-826 M027/M028 portrait-grid macro math; "
           "ordinal 15 -> (15&7=7)*32=224 srcX, (15>>3=1)*29=29 srcY; "
           "TOUCHCLICK_Compat_NormalizeLetterboxedPointPc34Compat HiDPI inverse; "
           "M11_Render_MapPointToFramebuffer SDL render framebuffer map");

    /*
     * Pre-computed physical pixels per surface:
     *
     *   surface       | vp res | vp rei | vp can
     *   --------------|--------|--------|---------
     *   1280 x 720    |  (741, 429) | (1053, 429) |  (899, 625)
     *   1920 x 1080   | (1111, 644) | (1580, 644) | (1349, 938)
     *   2560 x 1440   | (1481, 858) | (2106, 858) | (1799,1250)
     *   1512 x 982    |  (885, 579) | (1283, 579) | (1087, 829)
     *
     *   surface       | sc res | sc rei | sc can
     *   --------------|--------|--------|---------
     *   1280 x 720    |  (536, 411) |  (748, 411) |  (644, 544)
     *   1920 x 1080   |  (804, 616) | (1122, 616) |  (966, 816)
     *   2560 x 1440   | (1072, 821) | (1496, 821) | (1288,1088)
     *   1512 x 982    |  (619, 557) |  (898, 557) |  (761, 732)
     */

    /* 1280x720 (16:9 letterbox; vp drawW=1185 drawH=720 drawX=47 drawY=0,
     * screen drawW=1152 drawH=720 drawX=64 drawY=0). */
    expect_resurrect_viewport(1280, 720, /*phys*/ 741, /*phys*/ 429);
    expect_reincarnate_viewport(1280, 720, 1053, 429);
    expect_cancel_viewport(1280, 720, 899, 625);
    expect_screen_path_not_to_panel(1280, 720, 536, 411);
    expect_resurrect_fb_in_box(1280, 720, /*logical*/ 536, /*logical*/ 411);

    /* 1920x1080 (1.78:1, no letterbox bars on either path). */
    expect_resurrect_viewport(1920, 1080, 1111, 644);
    expect_reincarnate_viewport(1920, 1080, 1580, 644);
    expect_cancel_viewport(1920, 1080, 1349, 938);
    expect_screen_path_not_to_panel(1920, 1080, 804, 616);
    expect_resurrect_fb_in_box(1920, 1080, 804, 616);

    /* 2560x1440 (4K HiDPI letterbox; vp drawW=1856 drawH=1125
     * drawX=176 drawY=157, screen drawW=2304 drawH=1440 drawX=128
     * drawY=0). */
    expect_resurrect_viewport(2560, 1440, 1481, 858);
    expect_reincarnate_viewport(2560, 1440, 2106, 858);
    expect_cancel_viewport(2560, 1440, 1799, 1250);
    expect_screen_path_not_to_panel(2560, 1440, 1072, 821);
    expect_resurrect_fb_in_box(2560, 1440, 1072, 821);

    /* 1512x982 MacBook Pro Retina (16" pre-2024). The failure case
     * in check_macbook_retina_drawable_rect_regression in
     * test_m11_display_aspect_present_rect.c. */
    expect_resurrect_viewport(1512, 982, 885, 579);
    expect_reincarnate_viewport(1512, 982, 1283, 579);
    expect_cancel_viewport(1512, 982, 1087, 829);
    expect_screen_path_not_to_panel(1512, 982, 619, 557);
    expect_resurrect_fb_in_box(1512, 982, 619, 557);

    /* Reincarnate and cancel FB-in-box spot checks on two distinct
     * surfaces so the screen-letterbox math is exercised for all
     * three panel buttons (not just resurrect). */
    expect_reincarnate_fb_in_box(1280, 720, 748, 411);
    expect_cancel_fb_in_box(1280, 720, 644, 544);
    expect_reincarnate_fb_in_box(1512, 982, 898, 557);
    expect_cancel_fb_in_box(1512, 982, 761, 732);

    /*
     * Off-by-one regression: a click one source pixel left of
     * panel.resurrect (vp source x=103, y=81) on a 1280x720 surface
     * must not dispatch to panel.resurrect / panel.reincarnate /
     * panel.cancel via the viewport dispatch path. We accept any
     * other dispatch (e.g. an inventory subtable zone at source x
     * in [70..103]) or no dispatch at all (return 0). The original
     * fail-mode is "wrong panel button for clicks outside the
     * panel box", which on a HiDPI Retina surface would route
     * every deadzone pixel into the resurrect button.
     */
    expect_neighbor_left_of_resurrect_misses_panel(1280, 720,
                                                   /*phys*/ 592,
                                                   /*phys*/ 429);
    expect_neighbor_left_of_resurrect_misses_panel(1920, 1080, 889, 644);
    expect_neighbor_left_of_resurrect_misses_panel(2560, 1440, 1185, 858);
    expect_neighbor_left_of_resurrect_misses_panel(1512, 982, 696, 579);

    /*
     * Source lock: the source viewport rect must remain
     * (0, 33, 224, 136) per COORD.C. The HiDPI scaling must not
     * bleed into the source view.
     */
    {
        int vx = -1, vy = -1, vw = -1, vh = -1;
        CHECK(TOUCHCLICK_Compat_GetSourceViewportRect(&vx, &vy, &vw, &vh) == 1);
        CHECK(vx == 0);
        CHECK(vy == 33);
        CHECK(vw == 224);
        CHECK(vh == 136);
    }

    /*
     * Source-lock reminder: ordinal 15 atlas address is (224, 29).
     * The HiDPI scaling must not perturb the C026 atlas math; this
     * is a documentation lock rather than a runtime assertion (the
     * atlas address is computed at C026 asset load, not at click
     * time), but we keep it as a printed breadcrumb so a regression
     * in any future refactor of the touch-click zone table is
     * visible next to the panel-button assertions.
     */
    {
        unsigned int ord15Col = 15u & 7u;
        unsigned int ord15Row = 15u >> 3;
        unsigned int ord15SrcX = ord15Col * 32u;
        unsigned int ord15SrcY = ord15Row * 29u;
        printf("ordinal15_atlas=(%u,%u)\n", ord15SrcX, ord15SrcY);
        CHECK(ord15Col == 7u);
        CHECK(ord15Row == 1u);
        CHECK(ord15SrcX == 224u);
        CHECK(ord15SrcY == 29u);
    }

    printf("result=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_pass, g_failures);

    return g_failures == 0 ? 0 : 1;
}
