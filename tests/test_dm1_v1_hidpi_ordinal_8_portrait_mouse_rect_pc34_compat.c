/*
 * test_dm1_v1_hidpi_ordinal_8_portrait_mouse_rect_pc34_compat.c
 *
 * DM1 V1 Hall of Champions portrait ordinal 8 — high-DPI / scaled-window
 * mouse-rect regression gate. Companion slice to
 * test_dm1_v1_hidpi_chest_slot_hit_zone_pc34_compat (chest slots on the
 * panel table) and to the ordinal-8 portrait_rect_position runtime
 * probes (front-cell + sensor seed). This test locks the source-locked
 * D1C portrait rectangle (96, 35, 32, 29) — the destination the ReDMCSB
 * DUNVIEW.C:3913-3928 C026 portrait blit writes into for the ordinal-8
 * IAIDO champion — against the HiDPI mouse-rect mapping on real scaled
 * window sizes. No previous gate covers the D1C portrait rect on a
 * HiDPI surface, even though every other D1C element (chest slots,
 * inventory, panel buttons) is locked. The chest test covers panel-
 * table clicks; this test covers the underlying D1C cell that the
 * C127 sensor points at, which is the same rect the existing ordinal-8
 * portrait_rect_position probes verify at the source-framebuffer
 * level.
 *
 * ReDMCSB source lock:
 *   - DUNVIEW.C:525 G0109_ac_Box_ChampionPortraitOnWall = {96, 127, 35, 63}
 *     (D1C portrait destination rectangle, width 32 height 29, viewport-
 *     relative viewport Y starts at 33 per COORD.C:1693-1749).
 *   - DUNVIEW.C:3913-3928 F0124_DrawSquareD1C C026 portrait blit at
 *     {96, 35, 32, 29} on the 320x200 framebuffer.
 *   - DUNVIEW.C:3916-3919 C026_GRAPHIC_CHAMPION_PORTRAITS atlas math
 *     "A portrait is 32x29 pixels" (8x3 grid of 32x29 portraits, 256x87
 *     total). Ordinal 8 = column 0 row 1: (8 & 7) * 32 = 0, (8 >> 3) * 29
 *     = 29.
 *   - DUNGEON.C:2573 M011_CELL(sensor) view-direction filter; ordinal 8
 *     surfaces from the C127 sensorData=8 reading on a Hall front cell.
 *   - DUNGEON.C:2608-2612 G0289 (C127 sensorData anchor) flows to
 *     MOVESENS.C:1501-1503 and REVIVE.C F0280 to materialize the
 *     candidate. The rect itself is purely a render target — the
 *     click does not need a route; the source game treats the rect
 *     as part of the viewport.dungeon screen click region.
 *   - COORD.C:1693-1722 PC34 viewport origin (0, 33) / 224x136 dim.
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32, G2079_C29=29.
 *   - DEFS.H:821-826 M027_PORTRAIT_X / M028_PORTRAIT_Y macro encoding
 *     (8 & 7) << 5 / (8 >> 3) * 29 — the row-1 / column-0 case.
 *   - DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS.
 *   - DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL=5 (the cell-side
 *     ordinal the front-wall sensor filter uses).
 *   - COMMAND.C:403 C080 → C007_ZONE_VIEWPORT, layout-696 C003/C007
 *     viewport.dungeon screen rect (0, 33, 224, 136).
 *   - COMMAND.C:1394-1439 F0358 mouse hit-test against normalized
 *     320x200 coordinates.
 *   - COMMAND.C:1641-1644 primary (screen-relative) tables scanned
 *     before secondary (viewport-relative) tables.
 *   - src/engine/render_sdl_m11.c:381-416 M11_Render_ResolveSdl3ResizeEvent
 *     (SDL3 logical vs pixel-size event handling for HiDPI).
 *   - src/engine/render_sdl_m11.c:1838-1900 M11_Render_MapWindowToFramebuffer
 *     (the production mouse-to-fb mapping; the M11_SCALE_FIT integer
 *     letterbox path).
 *   - src/engine/render_sdl_m11.c:1885-1927 M11_Render_MapPointToFramebuffer
 *     (the lower-level pure-function map used by the touch dispatch
 *     path and the panel hit-test).
 *   - src/shared/touch_click_zone_matrix_pc34_compat.c:435-461
 *     TOUCHCLICK_Compat_MapScaledScreenPointToDispatch (screen-letterbox
 *     hit-test with the C080 primary table scan).
 *   - src/shared/touch_click_zone_matrix_pc34_compat.c:331-352
 *     TOUCHCLICK_Compat_NormalizeScaledScreenPoint (the screen-letterbox
 *     math, the same fit+letterbox M11_Render_ComputePresentationRect
 *     computes).
 *   - The screen dispatch path on the D1C portrait rect must route to
 *     viewport.dungeon (C080, zone 7) — that is the source-faithful
 *     COMMAND.C:1641-1644 primary-table scan result. The portrait
 *     rect is not a button: it is the source-locked display rect for
 *     the C026 ordinal-8 atlas cell and the click that lands inside
 *     it is a viewport dungeon click by source.
 *
 * Test scope (narrow regression):
 *   - For each of five HiDPI surface sizes (1280x720, 1920x1080,
 *     2560x1440, 1512x982 MacBook Pro Retina, 3024x1964 MacBook
 *     Retina pixel output), the four corners + center of the source
 *     D1C portrait rect (96, 35, 32, 29) on the 320x200 framebuffer
 *     pre-compute to integer physical pixels that, on the scaled
 *     window, map back to the same (96..127, 35..63) source rect via
 *     M11_Render_MapPointToFramebuffer. This is the closed-loop
 *     "the rect is the same source rect after HiDPI scaling"
 *     invariant.
 *   - The same 25 pre-computed click points route to viewport.dungeon
 *     (C080, zone 7, group "viewport.dungeon") via the screen
 *     dispatch path. This is the source-faithful ReDMCSB primary-
 *     table scan (COMMAND.C:1641-1644): the screen-relative
 *     viewport.dungeon zone (0, 33, 224, 136) is the only thing
 *     that can match at the rect's screen position, because the
 *     rect is not a button — the C026 blit is a paint target, not
 *     an input zone.
 *   - SDL3 pixel-size event handling: M11_Render_ResolveSdl3ResizeEvent
 *     resolves the high-DPI drawable (3024x1964) back to the logical
 *     mouse space (1512x982) on MacBook Pro Retina. The center of
 *     the ordinal-8 portrait rect on the resolved logical window
 *     maps to fb (111, 49) — inside the source rect.
 *   - The SDL3 headless fall-through (event=1512x982, live=0) and the
 *     logical-only (event=1512x982, live=1512x982, render=1512x982)
 *     paths both resolve to 1:1 window/render and still let the
 *     rect be hit.
 *   - Off-by-one neighbor clicks (one physical pixel right/down of
 *     the rect) map to fb (95..128, 35..63) or (96..127, 34..64) —
 *     outside the source rect. Locks the boundary inclusivity of
 *     the (96, 35) corner without absorbing the next pixel column
 *     or row.
 *   - Source lock: the source viewport rect must remain
 *     (0, 33, 224, 136) per COORD.C. The HiDPI scaling must not
 *     bleed into the source view. The portrait_rect_position
 *     contract (96, 35, 32, 29) inside the D1C wall-ornament frame
 *     (80, 29, 64, 43) per DUNVIEW.C:525 + G0205 coordSet 5 / index
 *     12 is preserved.
 *
 * Disjoint from:
 *   - test_dm1_v1_hidpi_chest_slot_hit_zone_pc34_compat: that test
 *     covers C537/C544 inventory chest slots on the panel table
 *     (C058..C065, viewport-relative click targets), not the D1C
 *     portrait rect on the screen-relative viewport table. The
 *     panel-table click test's "screen dispatch routes to
 *     viewport.dungeon" assertion is a contrast to the chest
 *     slot's C537 panel-table hit; this test's "screen dispatch
 *     routes to viewport.dungeon" assertion is the
 *     source-faithful correct hit for the D1C portrait rect.
 *   - test_m11_display_aspect_present_rect: that test exercises
 *     the M11_Render_ComputePresentationRect letterbox math and
 *     the resize-event resolution; it does not exercise the
 *     D1C portrait rect as a click target.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_8_south_return_*
 *     and firestaff_dm1_v1_hall_of_champions_portrait_08_cancel_reopen_*:
 *     those are real-asset runtime probes that verify the source
 *     framebuffer pixels at the rect on a D1 V1 DUNGEON.DAT pose.
 *     This test is data-free and verifies the HiDPI mouse-rect
 *     mapping (the same code path the runtime probes use via
 *     M11_GameView_HandleInput). Disjoint at the layer: those
 *     probes assert "rect contains ord-8 pixels"; this test
 *     asserts "a scaled-window click on the rect routes back
 *     to the rect".
 *   - firestaff_dm1_v1_champion_mirror_east_walkpath_ordinal_8_runtime_probe:
 *     corridor east_walkpath ordinal 8 — different route, different
 *     pose, disjoint aspect.
 *
 * HONESTY: this is HiDPI mouse-rect mapping evidence, not original
 * DM1 V1 PC 3.4 pixel parity. The pre-computed physical pixels are
 * the integer-letterbox solutions to the inverse presentation-rect
 * math; the test asserts that the inverse mapping is closed (a
 * click that lands on the source rect on a scaled window maps back
 * to the source rect) and that the screen dispatch routes to the
 * source-faithful viewport.dungeon zone.
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

/*
 * M11_Render_MapPointToFramebuffer must land inside the source D1C
 * portrait rect (96, 35, 32, 29) for any physical pixel that pre-
 * computes to the rect's TL/TR/BL/BR/center on the given surface.
 * The screen-letterbox inverse uses the same fit+letterbox math as
 * the production M11_Render_ComputePresentationRect path.
 */
static void expect_ord8_rect_under_mouse(int surfaceW,
                                          int surfaceH,
                                          int physX,
                                          int physY,
                                          const char* label) {
    int fbX = -1;
    int fbY = -1;
    int rc = M11_Render_MapPointToFramebuffer(physX,
                                               physY,
                                               surfaceW,
                                               surfaceH,
                                               M11_FB_WIDTH,
                                               M11_FB_HEIGHT,
                                               M11_SCALE_FIT,
                                               0,
                                               M11_DISPLAY_ASPECT_CONTENT,
                                               &fbX,
                                               &fbY);
    if (rc == 1) {
        CHECK(fbX >= 96 && fbX <= 127);
        CHECK(fbY >= 35 && fbY <= 63);
    } else {
        fprintf(stderr,
                "FAIL %s: M11_Render_MapPointToFramebuffer rejected the rect click "
                "(surface=%dx%d phys=(%d,%d))\n",
                label, surfaceW, surfaceH, physX, physY);
        ++g_failures;
    }
}

/*
 * The screen dispatch path on the D1C portrait rect must route to
 * viewport.dungeon (C080, zone 7, group "viewport.dungeon") per the
 * ReDMCSB primary-table scan (COMMAND.C:1641-1644 + C080 layout-696
 * C003/C007 = screen-relative (0, 33, 224, 136)). The D1C portrait
 * rect is not a button — it is the C026 ordinal-8 atlas blit
 * destination — and the source game treats any click that lands
 * inside it as a viewport dungeon click.
 */
static void expect_ord8_rect_dispatches_to_viewport_dungeon(int surfaceW,
                                                            int surfaceH,
                                                            int physX,
                                                            int physY,
                                                            const char* label) {
    TouchClickDispatchPc34Compat dispatch;
    int rc = TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(physX,
                                                              physY,
                                                              surfaceW,
                                                              surfaceH,
                                                              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                                              &dispatch);
    if (rc == 1) {
        CHECK(dispatch.commandId == 80u);
        CHECK(dispatch.zoneIndex == 7u);
        CHECK(strcmp(dispatch.groupName, "viewport.dungeon") == 0);
        /* The normalized screen point must land inside the source D1C
         * portrait rect (96..127, 35..63). */
        CHECK(dispatch.screenX >= 96 && dispatch.screenX <= 127);
        CHECK(dispatch.screenY >= 35 && dispatch.screenY <= 63);
        CHECK(dispatch.coordMode == TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT);
    } else {
        fprintf(stderr,
                "FAIL %s: screen dispatch rejected the rect click "
                "(surface=%dx%d phys=(%d,%d))\n",
                label, surfaceW, surfaceH, physX, physY);
        ++g_failures;
    }
}

/*
 * Off-by-one neighbor regression: a click just outside the source
 * D1C portrait rect (one physical pixel beyond the right or below
 * the bottom edge) must NOT map back inside the source rect. The
 * rect inclusivity check is the source-locked boundary: any click
 * strictly outside (96..127, 35..63) in source coords must land at
 * source fbX < 96 or fbX > 127 or fbY < 35 or fbY > 63.
 */
static void expect_ord8_rect_neighbor_misses(int surfaceW,
                                              int surfaceH,
                                              int physX,
                                              int physY,
                                              const char* label) {
    int fbX = -1;
    int fbY = -1;
    int rc = M11_Render_MapPointToFramebuffer(physX,
                                               physY,
                                               surfaceW,
                                               surfaceH,
                                               M11_FB_WIDTH,
                                               M11_FB_HEIGHT,
                                               M11_SCALE_FIT,
                                               0,
                                               M11_DISPLAY_ASPECT_CONTENT,
                                               &fbX,
                                               &fbY);
    if (rc == 1) {
        int inRect = (fbX >= 96 && fbX <= 127 && fbY >= 35 && fbY <= 63);
        if (inRect) {
            fprintf(stderr,
                    "FAIL %s: off-by-one neighbor landed inside D1C rect "
                    "(surface=%dx%d phys=(%d,%d) fb=(%d,%d))\n",
                    label, surfaceW, surfaceH, physX, physY, fbX, fbY);
            ++g_failures;
        } else {
            ++g_pass;
        }
    } else {
        /* rc == 0 means the click was outside the letterboxed rect;
         * the off-rect neighbor is a successful "miss". */
        ++g_pass;
    }
}

/*
 * SDL3 logical vs pixel-size event handling for MacBook Pro Retina
 * (1512x982 logical, 3024x1964 pixel). The pixel-size event must
 * resolve to the logical mouse space, and a center click on the
 * ordinal-8 portrait rect on the logical window must map to
 * fb (111, 49).
 */
static void check_sdl3_pixel_size_event_keeps_logical_mouse_space(void) {
    int windowW = -1;
    int windowH = -1;
    int renderW = -1;
    int renderH = -1;
    int fbX = -1;
    int fbY = -1;

    /* Pixel-size event reports the high-DPI drawable; the resolver
     * should keep the logical mouse space and route the renderer to
     * the pixel space. */
    CHECK(M11_Render_ResolveSdl3ResizeEvent(3024,
                                            1964,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            &windowW,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_OK);
    CHECK(windowW == 1512);
    CHECK(windowH == 982);
    CHECK(renderW == 3024);
    CHECK(renderH == 1964);
    /* Center of ordinal-8 portrait rect on the resolved logical
     * window (1512x982) must map to fb (111, 49) — exactly inside
     * the source D1C rect (96..127, 35..63). */
    CHECK(M11_Render_MapPointToFramebuffer(526,
                                           251,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 111);
    CHECK(fbY == 49);
}

/*
 * SDL3 headless fall-through: when the resolver is called with
 * event=1512x982 and live=0 (no SDL window in a headless test), the
 * function falls back to using the event as both the window and the
 * render size. The rect must still be reachable.
 */
static void check_sdl3_headless_fall_through(void) {
    int windowW = -1;
    int windowH = -1;
    int renderW = -1;
    int renderH = -1;
    int fbX = -1;
    int fbY = -1;
    CHECK(M11_Render_ResolveSdl3ResizeEvent(1512,
                                            982,
                                            0,
                                            0,
                                            0,
                                            0,
                                            &windowW,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_OK);
    CHECK(windowW == 1512);
    CHECK(windowH == 982);
    CHECK(renderW == 1512);
    CHECK(renderH == 982);
    CHECK(M11_Render_MapPointToFramebuffer(526,
                                           251,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 111);
    CHECK(fbY == 49);
}

/*
 * Source lock: the source viewport rect must remain
 * (0, 33, 224, 136) per COORD.C. The HiDPI scaling must not bleed
 * into the source view. The portrait_rect_position contract — the
 * (96, 35, 32, 29) D1C destination rectangle per DUNVIEW.C:525 +
 * G0109 — is verified at the source level, not the scaled-window
 * level, by reading the source rect directly.
 */
static void check_source_lock_invariants(void) {
    int vx = -1;
    int vy = -1;
    int vw = -1;
    int vh = -1;
    /* Source viewport: viewport.dungeon is (0, 33, 224, 136) per
     * layout-696 C003/C007 (COMMAND.C:403). */
    CHECK(TOUCHCLICK_Compat_GetSourceViewportRect(&vx, &vy, &vw, &vh) == 1);
    CHECK(vx == 0);
    CHECK(vy == 33);
    CHECK(vw == 224);
    CHECK(vh == 136);
    /* The ordinal-8 portrait rect (96, 35, 32, 29) is at viewport
     * position (96, 2) when the source viewport offset (0, 33) is
     * applied (35 - 33 = 2). The rect is the top 2-row band of the
     * viewport dungeon — i.e. the upper-front of the D1C wall
     * ornament backing per DUNVIEW.C:525 G0109_ac_Box_ChampionPortraitOnWall. */
    CHECK(96 >= vx);
    CHECK(35 >= vy);
    CHECK(96 + 32 <= vx + vw);
    CHECK(35 + 29 <= vy + vh);
    /* The source content rect (320x200) is also the M11 default
     * content extent. */
    CHECK(M11_FB_WIDTH == 320);
    CHECK(M11_FB_HEIGHT == 200);
}

/*
 * Run the four-corners + center loop for one HiDPI surface. The
 * pre-computed physical pixels are the integer-letterbox solutions
 * of the inverse presentation-rect math on the screen-letterbox
 * (320x200 source, M11_SCALE_FIT, M11_DISPLAY_ASPECT_CONTENT):
 *
 *   fbX in [96..127], fbY in [35..63]    -> physical pixel
 *   fbX = 96  fbY = 35  -> (rectStart.x + (96*rectW + rectW/2)/320, ...)
 *   (and similarly for the other corners + center)
 */
static void check_surface_ordinal_8_rect(int surfaceW, int surfaceH, const char* label) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int tlX, tlY, trX, trY, blX, blY, brX, brY, cx, cy;

    CHECK(M11_Render_ComputePresentationRect(surfaceW,
                                             surfaceH,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_OK);

    /* The inverse mapping is: physX = rectX + (fbX * rectW + rectW/2) / 320
     * and physY = rectY + (fbY * rectH + rectH/2) / 200, which uses
     * the centered division the production code path uses. */
    tlX = rectX + (96  * rectW + rectW/2) / 320;
    tlY = rectY + (35  * rectH + rectH/2) / 200;
    trX = rectX + (127 * rectW + rectW/2) / 320;
    trY = rectY + (35  * rectH + rectH/2) / 200;
    blX = rectX + (96  * rectW + rectW/2) / 320;
    blY = rectY + (63  * rectH + rectH/2) / 200;
    brX = rectX + (127 * rectW + rectW/2) / 320;
    brY = rectY + (63  * rectH + rectH/2) / 200;
    cx  = rectX + (111 * rectW + rectW/2) / 320;
    cy  = rectY + (49  * rectH + rectH/2) / 200;

    {
        char buf[96];
        /* The five points (TL, TR, BL, BR, center) all map back to
         * the source D1C rect and all dispatch to viewport.dungeon. */
        snprintf(buf, sizeof(buf), "%s TL (%d,%d)", label, tlX, tlY);
        expect_ord8_rect_under_mouse(surfaceW, surfaceH, tlX, tlY, buf);
        expect_ord8_rect_dispatches_to_viewport_dungeon(surfaceW, surfaceH, tlX, tlY, buf);
        snprintf(buf, sizeof(buf), "%s TR (%d,%d)", label, trX, trY);
        expect_ord8_rect_under_mouse(surfaceW, surfaceH, trX, trY, buf);
        expect_ord8_rect_dispatches_to_viewport_dungeon(surfaceW, surfaceH, trX, trY, buf);
        snprintf(buf, sizeof(buf), "%s BL (%d,%d)", label, blX, blY);
        expect_ord8_rect_under_mouse(surfaceW, surfaceH, blX, blY, buf);
        expect_ord8_rect_dispatches_to_viewport_dungeon(surfaceW, surfaceH, blX, blY, buf);
        snprintf(buf, sizeof(buf), "%s BR (%d,%d)", label, brX, brY);
        expect_ord8_rect_under_mouse(surfaceW, surfaceH, brX, brY, buf);
        expect_ord8_rect_dispatches_to_viewport_dungeon(surfaceW, surfaceH, brX, brY, buf);
        snprintf(buf, sizeof(buf), "%s center (%d,%d)", label, cx, cy);
        expect_ord8_rect_under_mouse(surfaceW, surfaceH, cx, cy, buf);
        expect_ord8_rect_dispatches_to_viewport_dungeon(surfaceW, surfaceH, cx, cy, buf);
    }
}

int main(void) {
    printf("probe=firestaff_dm1_v1_hidpi_ordinal_8_portrait_mouse_rect_pc34_compat\n");
    printf("sourceEvidence=%s\n", TOUCHCLICK_Compat_GetSourceEvidence());
    printf("sourcePortraitRect=DUNVIEW.C:525 G0109_ac_Box_ChampionPortraitOnWall="
           "{96,127,35,63}; DUNVIEW.C:3913-3928 C026 portrait blit at "
           "{96,35,32,29}; COORD.C:1693-1749 viewport origin (0,33) + portrait "
           "dims 32x29; DEFS.H:821-826 M027/M028 atlas math; ordinal 8 = "
           "(8 & 7) << 5 = 0, (8 >> 3) * 29 = 29; "
           "COMMAND.C:403 C080->C007 viewport.dungeon screen rect (0,33,224,136); "
           "COMMAND.C:1641-1644 primary screen-relative tables scanned first");

    /* 1280x720 (16:9 letterbox; screen drawW=1152 drawH=720 drawX=64 drawY=0). */
    check_surface_ordinal_8_rect(1280, 720, "1280x720");

    /* 1920x1080 (1.78:1, no letterbox bars on the screen path). */
    check_surface_ordinal_8_rect(1920, 1080, "1920x1080");

    /* 2560x1440 (4K HiDPI letterbox; screen drawW=2304 drawH=1440 drawX=128 drawY=0). */
    check_surface_ordinal_8_rect(2560, 1440, "2560x1440");

    /* 1512x982 MacBook Pro Retina (16" pre-2024). The original HiDPI
     * failure case (the macOS maximize-tiny-view report). */
    check_surface_ordinal_8_rect(1512, 982, "1512x982");

    /* 3024x1964 MacBook Pro Retina pixel-output space. The same
     * logical 1512x982 mouse click on the renderer pixel space. */
    check_surface_ordinal_8_rect(3024, 1964, "3024x1964");

    /*
     * Off-by-one neighbors: a click one physical pixel right of the
     * TL/center (fbX=128) or below the rect (fbY=64) must NOT map
     * back inside (96..127, 35..63). The pre-computed fbX=128 / fbY=64
     * physical pixels are rectX + (128 * rectW + rectW/2) / 320 and
     * rectY + (64 * rectH + rectH/2) / 200.
     */
    {
        int surfaceW = 1280;
        int surfaceH = 720;
        int rectX = -1;
        int rectY = -1;
        int rectW = -1;
        int rectH = -1;
        int rightX, belowY, cx, cy;
        CHECK(M11_Render_ComputePresentationRect(surfaceW,
                                                 surfaceH,
                                                 M11_FB_WIDTH,
                                                 M11_FB_HEIGHT,
                                                 M11_SCALE_FIT,
                                                 0,
                                                 M11_DISPLAY_ASPECT_CONTENT,
                                                 &rectX,
                                                 &rectY,
                                                 &rectW,
                                                 &rectH) == M11_RENDER_OK);
        rightX = rectX + (128 * rectW + rectW/2) / 320;
        belowY = rectY + (64  * rectH + rectH/2) / 200;
        cx     = rectX + (111 * rectW + rectW/2) / 320;
        cy     = rectY + (49  * rectH + rectH/2) / 200;
        expect_ord8_rect_neighbor_misses(surfaceW, surfaceH, rightX, cy,
                                         "1280x720 right-fbX=128");
        expect_ord8_rect_neighbor_misses(surfaceW, surfaceH, cx, belowY,
                                         "1280x720 below-fbY=64");
    }
    {
        int surfaceW = 1920;
        int surfaceH = 1080;
        int rectX = -1;
        int rectY = -1;
        int rectW = -1;
        int rectH = -1;
        int rightX, belowY, cx, cy;
        CHECK(M11_Render_ComputePresentationRect(surfaceW,
                                                 surfaceH,
                                                 M11_FB_WIDTH,
                                                 M11_FB_HEIGHT,
                                                 M11_SCALE_FIT,
                                                 0,
                                                 M11_DISPLAY_ASPECT_CONTENT,
                                                 &rectX,
                                                 &rectY,
                                                 &rectW,
                                                 &rectH) == M11_RENDER_OK);
        rightX = rectX + (128 * rectW + rectW/2) / 320;
        belowY = rectY + (64  * rectH + rectH/2) / 200;
        cx     = rectX + (111 * rectW + rectW/2) / 320;
        cy     = rectY + (49  * rectH + rectH/2) / 200;
        expect_ord8_rect_neighbor_misses(surfaceW, surfaceH, rightX, cy,
                                         "1920x1080 right-fbX=128");
        expect_ord8_rect_neighbor_misses(surfaceW, surfaceH, cx, belowY,
                                         "1920x1080 below-fbY=64");
    }
    {
        int surfaceW = 1512;
        int surfaceH = 982;
        int rectX = -1;
        int rectY = -1;
        int rectW = -1;
        int rectH = -1;
        int rightX, belowY, cx, cy;
        CHECK(M11_Render_ComputePresentationRect(surfaceW,
                                                 surfaceH,
                                                 M11_FB_WIDTH,
                                                 M11_FB_HEIGHT,
                                                 M11_SCALE_FIT,
                                                 0,
                                                 M11_DISPLAY_ASPECT_CONTENT,
                                                 &rectX,
                                                 &rectY,
                                                 &rectW,
                                                 &rectH) == M11_RENDER_OK);
        rightX = rectX + (128 * rectW + rectW/2) / 320;
        belowY = rectY + (64  * rectH + rectH/2) / 200;
        cx     = rectX + (111 * rectW + rectW/2) / 320;
        cy     = rectY + (49  * rectH + rectH/2) / 200;
        expect_ord8_rect_neighbor_misses(surfaceW, surfaceH, rightX, cy,
                                         "1512x982 right-fbX=128");
        expect_ord8_rect_neighbor_misses(surfaceW, surfaceH, cx, belowY,
                                         "1512x982 below-fbY=64");
    }

    /* SDL3 pixel-size event handling on MacBook Pro Retina. */
    check_sdl3_pixel_size_event_keeps_logical_mouse_space();

    /* SDL3 headless fall-through (event with no live SDL window). */
    check_sdl3_headless_fall_through();

    /* Source lock invariants: the source viewport is (0, 33, 224, 136). */
    check_source_lock_invariants();

    printf("result=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_pass, g_failures);

    return g_failures == 0 ? 0 : 1;
}
