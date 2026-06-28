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

int main(void) {
    const char* evidence = ENTRANCE_Compat_GetMouseRouteEvidence();
    unsigned int i;
    unsigned int count;
    EntranceMouseRouteCompat firstRoute;

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

    count = ENTRANCE_Compat_GetMouseRouteCount();
    CHECK(count == 5u);
    for (i = 1u; i <= count; ++i) {
        EntranceMouseRouteCompat route;
        CHECK(ENTRANCE_Compat_GetMouseRoute(i, &route) == 1);
        expect_route_on_surface(&route, 1512, 982, "macbook_logical_1512x982");
        expect_route_on_surface(&route, 3024, 1964, "macbook_retina_drawable_3024x1964");
    }

    CHECK(ENTRANCE_Compat_GetMouseRoute(1u, &firstRoute) == 1);
    expect_right_edge_misses(&firstRoute, 1512, 982, "macbook_logical_1512x982");
    expect_right_edge_misses(&firstRoute, 3024, 1964, "macbook_retina_drawable_3024x1964");

    printf("result=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
