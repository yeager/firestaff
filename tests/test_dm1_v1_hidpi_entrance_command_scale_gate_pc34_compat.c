/*
 * test_dm1_v1_hidpi_entrance_command_scale_gate_pc34_compat.c
 *
 * Data-free DM1 V1 high-DPI input regression: MacBook-style logical
 * window (1512x982) and Retina drawable (3024x1964) surfaces must map
 * source-locked 320x200 entrance button coordinates back to the intended
 * ReDMCSB entrance command/zone.
 *
 * Source lock:
 *   - ENTRANCE.C:739-747 installs the entrance mouse input table.
 *   - ENTRANCE.C:850-883 waits for a fresh entrance command.
 *   - COMMAND.C:340-353 defines the G0445 entrance mouse table.
 *   - COMMAND.C:1379-1449 F0358 scans source-order mouse rows and masks.
 *   - COMMAND.C:1641-1660 F0359 routes the primary click command queue.
 *   - COORD.C:1903-1920 expands inclusive source zones.
 *   - COORD.C:2490-2495 F0638_GetZone fetches layout-696 zone records.
 */

#include "entrance_mouse_routes_pc34_compat.h"
#include "dm1_v1_mouse_routes_pc34_compat.h"
#include "main_loop_m11.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

/* IMG3 globals are required when this focused gate links the full M11/M10
 * runtime libraries through main_loop_m11.c. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures = 0;
static int g_passes = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++g_failures; \
    } else { \
        ++g_passes; \
    } \
} while (0)

static int presented_center_for_source_axis(int sourceCoord,
                                            int sourceExtent,
                                            int presentedExtent) {
    int lo;
    int hi;
    if (sourceCoord < 0) sourceCoord = 0;
    if (sourceCoord >= sourceExtent) sourceCoord = sourceExtent - 1;
    lo = (sourceCoord * presentedExtent + sourceExtent - 1) / sourceExtent;
    hi = (((sourceCoord + 1) * presentedExtent) - 1) / sourceExtent;
    return (lo + hi) / 2;
}

static int map_source_point_to_window(int sourceX,
                                      int sourceY,
                                      int windowW,
                                      int windowH,
                                      int* outWindowX,
                                      int* outWindowY) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    if (!outWindowX || !outWindowY) return 0;
    if (M11_Render_ComputePresentationRect(windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &rectX,
                                           &rectY,
                                           &rectW,
                                           &rectH) != M11_RENDER_OK) {
        return 0;
    }
    *outWindowX = rectX + presented_center_for_source_axis(sourceX, M11_FB_WIDTH, rectW);
    *outWindowY = rectY + presented_center_for_source_axis(sourceY, M11_FB_HEIGHT, rectH);
    return 1;
}

static void expect_route_on_surface(const EntranceMouseRouteCompat* route,
                                    int windowW,
                                    int windowH,
                                    const char* surfaceName) {
    int sourceX;
    int sourceY;
    int windowX = -1;
    int windowY = -1;
    int fbX = -1;
    int fbY = -1;
    int command = -1;
    EntranceMouseRouteCompat hit;

    sourceX = route->x + route->w / 2;
    sourceY = route->y + route->h / 2;

    CHECK(map_source_point_to_window(sourceX,
                                     sourceY,
                                     windowW,
                                     windowH,
                                     &windowX,
                                     &windowY) == 1);
    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX >= route->x && fbX < route->x + route->w);
    CHECK(fbY >= route->y && fbY < route->y + route->h);
    CHECK(ENTRANCE_Compat_HitTestMouseRoute(fbX, fbY, route->buttonMask, &hit) == 1);
    CHECK(hit.zoneIndex == route->zoneIndex);
    CHECK(hit.commandId == route->commandId);

    command = M11_Entrance_DispatchSourceLockedPointerCommand(fbX, fbY, route->buttonMask);
    CHECK(command == (int)route->commandId);

    printf("surface=%s route=%s window=%d,%d fb=%d,%d zone=%u command=%u\n",
           surfaceName, route->name, windowX, windowY, fbX, fbY,
           hit.zoneIndex, hit.commandId);
}

static void expect_route_after_sdl3_pixel_resize(const EntranceMouseRouteCompat* route) {
    int windowW = -1;
    int windowH = -1;
    int renderW = -1;
    int renderH = -1;

    /* SDL3 reports mouse clicks in logical window coordinates while the
     * WINDOW_PIXEL_SIZE_CHANGED event carries the Retina drawable size.
     * ReDMCSB COMMAND.C:1379-1449 / 1641-1660 must still receive 320x200
     * source coordinates for the entrance command route. */
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
    expect_route_on_surface(route,
                            windowW,
                            windowH,
                            "sdl3_pixel_resize_keeps_logical_mouse_1512x982");
}

static void expect_right_edge_misses(const EntranceMouseRouteCompat* route,
                                     int windowW,
                                     int windowH,
                                     const char* surfaceName) {
    int windowX = -1;
    int windowY = -1;
    int fbX = -1;
    int fbY = -1;
    int command = -1;

    CHECK(map_source_point_to_window(route->x + route->w,
                                     route->y + route->h / 2,
                                     windowW,
                                     windowH,
                                     &windowX,
                                     &windowY) == 1);
    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    command = M11_Entrance_DispatchSourceLockedPointerCommand(fbX, fbY, route->buttonMask);
    CHECK(command == M11_ENTRANCE_RUNTIME_COMMAND_NONE);
    printf("surface=%s outside_right_of=%s window=%d,%d fb=%d,%d command=%d\n",
           surfaceName, route->name, windowX, windowY, fbX, fbY, command);
}

/* Original preserves the source pixels, but a user may select a larger host
 * presentation target.  This closes the whole production mapping chain used
 * by m11_map_window_pointer_to_game_source:
 *
 * window point -> M11_Render_MapPointToFramebuffer (host target) ->
 * M11_MapPresentedGamePointToSourceForPresentation (320x200) ->
 * ReDMCSB G0445 entrance command.
 *
 * Testing only either half missed the regression where V1's 640x400 or
 * 1920x1200 host pixels were passed directly to COMMAND.C. */
static void expect_original_host_target_route(
    const EntranceMouseRouteCompat* route,
    int targetW,
    int targetH,
    int windowW,
    int windowH,
    const char* surfaceName) {
    int sourceX;
    int sourceY;
    int presentedX;
    int presentedY;
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int windowX;
    int windowY;
    int mappedPresentedX = -1;
    int mappedPresentedY = -1;
    int mappedSourceX;
    int mappedSourceY;
    int command;

    sourceX = route->x + route->w / 2;
    sourceY = route->y + route->h / 2;
    presentedX = sourceX;
    presentedY = sourceY;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V1_ORIGINAL, targetW, targetH,
              &presentedX, &presentedY) == 1);
    CHECK(M11_Render_ComputePresentationRect(windowW, windowH,
                                             targetW, targetH,
                                             M11_SCALE_FIT, 0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX, &rectY,
                                             &rectW, &rectH) == M11_RENDER_OK);
    windowX = rectX + presented_center_for_source_axis(presentedX,
                                                        targetW, rectW);
    windowY = rectY + presented_center_for_source_axis(presentedY,
                                                        targetH, rectH);
    CHECK(M11_Render_MapPointToFramebuffer(windowX, windowY,
                                           windowW, windowH,
                                           targetW, targetH,
                                           M11_SCALE_FIT, 0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &mappedPresentedX,
                                           &mappedPresentedY) == 1);
    mappedSourceX = mappedPresentedX;
    mappedSourceY = mappedPresentedY;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V1_ORIGINAL, targetW, targetH,
              &mappedSourceX, &mappedSourceY) == 1);
    CHECK(mappedSourceX >= route->x &&
          mappedSourceX < route->x + route->w);
    CHECK(mappedSourceY >= route->y &&
          mappedSourceY < route->y + route->h);
    command = M11_Entrance_DispatchSourceLockedPointerCommand(
        mappedSourceX, mappedSourceY, route->buttonMask);
    CHECK(command == (int)route->commandId);

    /* A point beyond the presented surface's right edge must be rejected by
     * the host mapper rather than clamped into an original entrance button. */
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW, windowY,
                                           windowW, windowH,
                                           targetW, targetH,
                                           M11_SCALE_FIT, 0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &mappedPresentedX,
                                           &mappedPresentedY) == 0);
    printf("surface=%s target=%dx%d route=%s window=%d,%d presented=%d,%d source=%d,%d command=%d\n",
           surfaceName, targetW, targetH, route->name, windowX, windowY,
           mappedPresentedX, mappedPresentedY, mappedSourceX, mappedSourceY,
           command);
}

static int find_hoc_status_bar_source_point(int* outX, int* outY) {
    int x;
    int y;
    if (!outX || !outY) return 0;
    /* C187 is the leftmost source-owned champion bar. It is visible and
     * interactive after a HoC champion is admitted, so it covers the same
     * gameplay pointer route as inventory/HUD use rather than entrance-only
     * routing. Discover it from the ReDMCSB table instead of duplicating a
     * host rectangle. */
    for (y = 0; y < M11_FB_HEIGHT; ++y) {
        for (x = 0; x < M11_FB_WIDTH; ++x) {
            int space = DM1_V1_MOUSE_SPACE_NONE_PC34;
            int zone = 0;
            int command = DM1_V1_MouseRoutes_CommandForScreenPointPc34Compat(
                DM1_V1_MOUSE_LIST_INTERFACE_PC34, x, y,
                DM1_V1_MOUSE_MASK_LEFT_PC34, &space, &zone);
            if (command == 7 && zone == 187 &&
                space == DM1_V1_MOUSE_SPACE_SCREEN_PC34) {
                *outX = x;
                *outY = y;
                return 1;
            }
        }
    }
    return 0;
}

static void expect_original_host_target_hoc_route(int targetW,
                                                  int targetH,
                                                  int windowW,
                                                  int windowH) {
    int sourceX = -1;
    int sourceY = -1;
    int presentedX;
    int presentedY;
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int mappedX = -1;
    int mappedY = -1;
    int space = DM1_V1_MOUSE_SPACE_NONE_PC34;
    int zone = 0;
    int command;

    CHECK(find_hoc_status_bar_source_point(&sourceX, &sourceY) == 1);
    presentedX = sourceX;
    presentedY = sourceY;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V1_ORIGINAL, targetW, targetH,
              &presentedX, &presentedY) == 1);
    CHECK(M11_Render_ComputePresentationRect(windowW, windowH,
                                             targetW, targetH,
                                             M11_SCALE_FIT, 0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX, &rectY,
                                             &rectW, &rectH) == M11_RENDER_OK);
    CHECK(M11_Render_MapPointToFramebuffer(
              rectX + presented_center_for_source_axis(presentedX,
                                                        targetW, rectW),
              rectY + presented_center_for_source_axis(presentedY,
                                                        targetH, rectH),
              windowW, windowH, targetW, targetH, M11_SCALE_FIT, 0,
              M11_DISPLAY_ASPECT_CONTENT, &mappedX, &mappedY) == 1);
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V1_ORIGINAL, targetW, targetH,
              &mappedX, &mappedY) == 1);
    command = DM1_V1_MouseRoutes_CommandForScreenPointPc34Compat(
        DM1_V1_MOUSE_LIST_INTERFACE_PC34, mappedX, mappedY,
        DM1_V1_MOUSE_MASK_LEFT_PC34, &space, &zone);
    CHECK(command == 7);
    CHECK(zone == 187);
    CHECK(space == DM1_V1_MOUSE_SPACE_SCREEN_PC34);
    printf("hoc_original_host_target=%dx%d source=%d,%d mapped=%d,%d command=%d zone=%d\n",
           targetW, targetH, sourceX, sourceY, mappedX, mappedY,
           command, zone);
}

static void expect_macbook_drawable_not_tiny_view(void) {
    int logicalX = -1;
    int logicalY = -1;
    int logicalW = -1;
    int logicalH = -1;
    int drawableX = -1;
    int drawableY = -1;
    int drawableW = -1;
    int drawableH = -1;

    CHECK(M11_Render_ComputePresentationRect(1512,
                                             982,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &logicalX,
                                             &logicalY,
                                             &logicalW,
                                             &logicalH) == M11_RENDER_OK);
    CHECK(M11_Render_ComputePresentationRect(3024,
                                             1964,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &drawableX,
                                             &drawableY,
                                             &drawableW,
                                             &drawableH) == M11_RENDER_OK);
    CHECK(logicalX == 0);
    CHECK(logicalY == 18);
    CHECK(logicalW == 1512);
    CHECK(logicalH == 945);
    CHECK(drawableX == 0);
    CHECK(drawableY == 37);
    CHECK(drawableW == 3024);
    CHECK(drawableH == 1890);
    CHECK(drawableW == logicalW * 2);
    CHECK(drawableH == logicalH * 2);
    CHECK(drawableW > 2900);
    CHECK(drawableH > 1800);
}

static void expect_retina_integer_rectangle_matches_logical_input(void) {
    int logicalX = -1;
    int logicalY = -1;
    int logicalW = -1;
    int logicalH = -1;
    int drawableX = -1;
    int drawableY = -1;
    int drawableW = -1;
    int drawableH = -1;

    /* Original selects integer pixels. On a 2x surface its factor must be
     * selected from 1512x982 logical mouse points, not 3024x1964 pixels. */
    CHECK(M11_Render_ComputePresentationRect(1512, 982,
                                             M11_FB_WIDTH, M11_FB_HEIGHT,
                                             M11_SCALE_FIT, 1,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &logicalX, &logicalY,
                                             &logicalW, &logicalH) == M11_RENDER_OK);
    CHECK(logicalX == 116);
    CHECK(logicalY == 91);
    CHECK(logicalW == 1280);
    CHECK(logicalH == 800);
    CHECK(M11_Render_ComputeDrawablePresentationRect(1512, 982,
                                                      3024, 1964,
                                                      M11_FB_WIDTH, M11_FB_HEIGHT,
                                                      M11_SCALE_FIT, 1,
                                                      M11_DISPLAY_ASPECT_CONTENT,
                                                      &drawableX, &drawableY,
                                                      &drawableW, &drawableH) == M11_RENDER_OK);
    CHECK(drawableX == logicalX * 2);
    CHECK(drawableY == logicalY * 2);
    CHECK(drawableW == logicalW * 2);
    CHECK(drawableH == logicalH * 2);
}

int main(void) {
    const char* evidence = ENTRANCE_Compat_GetMouseRouteEvidence();
    unsigned int i;
    unsigned int count;

    printf("probe=dm1_v1_hidpi_entrance_command_scale_gate_pc34_compat\n");
    printf("routeEvidence=%s\n", evidence);

    CHECK(strstr(evidence, "ENTRANCE.C:739-747") != NULL);
    CHECK(strstr(evidence, "ENTRANCE.C:850-883") != NULL);
    CHECK(strstr(evidence, "COMMAND.C:340-353") != NULL);
    CHECK(strstr(evidence, "COMMAND.C:1379-1449") != NULL);
    CHECK(strstr(evidence, "COMMAND.C:1641-1660") != NULL);
    CHECK(strstr(evidence, "COORD.C:1903-1920") != NULL);
    CHECK(strstr(evidence, "COORD.C:2490-2495") != NULL);

    expect_macbook_drawable_not_tiny_view();
    expect_retina_integer_rectangle_matches_logical_input();
    expect_original_host_target_hoc_route(640, 400, 1512, 982);
    expect_original_host_target_hoc_route(1920, 1200, 1512, 982);

    count = ENTRANCE_Compat_GetMouseRouteCount();
    CHECK(count == 5u);
    for (i = 1u; i <= count; ++i) {
        EntranceMouseRouteCompat route;
        CHECK(ENTRANCE_Compat_GetMouseRoute(i, &route) == 1);
        expect_route_on_surface(&route, 1512, 982, "macbook_logical_1512x982");
        expect_route_on_surface(&route, 3024, 1964, "macbook_retina_drawable_3024x1964");
        expect_route_after_sdl3_pixel_resize(&route);
        expect_right_edge_misses(&route, 1512, 982, "macbook_logical_1512x982");
        expect_right_edge_misses(&route, 3024, 1964, "macbook_retina_drawable_3024x1964");
        expect_original_host_target_route(&route, 640, 400,
                                          1512, 982,
                                          "original_host_640x400_macbook");
        expect_original_host_target_route(&route, 1920, 1200,
                                          1512, 982,
                                          "original_host_1920x1200_macbook");
    }

    printf("result=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
