/*
 * DM1 V1 Hall of Champions -- ordinal 4 RESTING original-capture scaffold.
 *
 * This data-free probe locks the geometry and route vocabulary needed by a
 * future paired original-DOS capture for the LEIF (ordinal 4) RESTING overlay
 * row. It does not load game data, run DOSBox, or claim pixel parity.
 *
 * Source anchors:
 *   ReDMCSB COORD.C:1693-1698       PC 3.4 viewport origin (0,33).
 *   ReDMCSB DUNVIEW.C:525           D1C portrait box {96,127,35,63}.
 *   ReDMCSB DUNVIEW.C:3913-3928     C026 portrait blit, 32x29 atlas cells.
 *   ReDMCSB DEFS.H:821-826          M027/M028 portrait atlas math.
 *   ReDMCSB COMMAND.C:414           C145 rest icon zone on the viewport.
 *   ReDMCSB COMMAND.C:453-455       resting mouse routes wake with C146.
 *   ReDMCSB COMMAND.C:2336-2363     C145 rest -> G0300, C146 wake dispatch.
 *   ReDMCSB CHAMPION.C:1382-1401    F0314 wake clears G0300 and redraws.
 *   Firestaff m11_game_view.c:28414-28420 draws the current RESTING overlay
 *                                      at framebuffer (100,70,120,30).
 */

#include <stdio.h>
#include <string.h>

enum {
    DM1_SCREEN_W = 320,
    DM1_SCREEN_H = 200,
    DM1_VIEWPORT_X = 0,
    DM1_VIEWPORT_Y = 33,
    DM1_VIEWPORT_W = 224,
    DM1_VIEWPORT_H = 136,

    DM1_D1C_ZONE_X_VP = 80,
    DM1_D1C_ZONE_Y_VP = 29,
    DM1_D1C_ZONE_W = 64,
    DM1_D1C_ZONE_H = 43,

    DM1_PORTRAIT_X_VP = 96,
    DM1_PORTRAIT_Y_VP = 35,
    DM1_PORTRAIT_W = 32,
    DM1_PORTRAIT_H = 29,
    DM1_PORTRAIT_COLS = 8,
    DM1_PORTRAIT_ROWS = 3,
    DM1_PORTRAIT_TOTAL = 24,

    DM1_ORDINAL_LEIF = 4,
    DM1_ORDINAL4_SRC_X = 128,
    DM1_ORDINAL4_SRC_Y = 0,

    DM1_REST_ICON_X0_VP = 188,
    DM1_REST_ICON_X1_VP = 204,
    DM1_REST_ICON_Y0_VP = 36,
    DM1_REST_ICON_Y1_VP = 44,
    DM1_REST_ICON_CENTER_X_FB = DM1_VIEWPORT_X + 196,
    DM1_REST_ICON_CENTER_Y_FB = DM1_VIEWPORT_Y + 40,

    DM1_RESTING_OVERLAY_X_FB = 100,
    DM1_RESTING_OVERLAY_Y_FB = 70,
    DM1_RESTING_OVERLAY_W = 120,
    DM1_RESTING_OVERLAY_H = 30,
    DM1_RESTING_TEXT_X_FB = 112,
    DM1_RESTING_TEXT_Y_FB = 78,
    DM1_RESTING_WAKE_TEXT_Y_FB = 88,

    DM1_CAPTURE_DELAY_ENTER_MS = 9000,
    DM1_CAPTURE_DELAY_SETTLE_MS = 1500,
    DM1_CAPTURE_DELAY_REST_MS = 1200,
    DM1_CAPTURE_DELAY_WAKE_MS = 1200
};

typedef struct RectI {
    int x;
    int y;
    int w;
    int h;
} RectI;

typedef enum CaptureRoute {
    CAPTURE_ROUTE_OUTSIDE = 0,
    CAPTURE_ROUTE_VIEWPORT,
    CAPTURE_ROUTE_RIGHT_CHROME,
    CAPTURE_ROUTE_LOWER_CHROME
} CaptureRoute;

static int g_pass = 0;
static int g_fail = 0;

static void record(const char *id, int ok, const char *message)
{
    if (ok) {
        ++g_pass;
        printf("PASS %-36s %s\n", id, message);
    } else {
        ++g_fail;
        printf("FAIL %-36s %s\n", id, message);
    }
}

static int rect_contains(RectI r, int x, int y)
{
    return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}

static int rect_inside(RectI inner, RectI outer)
{
    return inner.x >= outer.x && inner.y >= outer.y &&
           inner.x + inner.w <= outer.x + outer.w &&
           inner.y + inner.h <= outer.y + outer.h;
}

static RectI rect_intersection(RectI a, RectI b)
{
    int x0 = a.x > b.x ? a.x : b.x;
    int y0 = a.y > b.y ? a.y : b.y;
    int x1 = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    int y1 = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    RectI out;
    if (x1 <= x0 || y1 <= y0) {
        out.x = out.y = out.w = out.h = 0;
        return out;
    }
    out.x = x0;
    out.y = y0;
    out.w = x1 - x0;
    out.h = y1 - y0;
    return out;
}

static CaptureRoute classify_capture_point(int x, int y)
{
    RectI screen = {0, 0, DM1_SCREEN_W, DM1_SCREEN_H};
    RectI viewport = {DM1_VIEWPORT_X, DM1_VIEWPORT_Y,
                      DM1_VIEWPORT_W, DM1_VIEWPORT_H};
    if (!rect_contains(screen, x, y)) return CAPTURE_ROUTE_OUTSIDE;
    if (rect_contains(viewport, x, y)) return CAPTURE_ROUTE_VIEWPORT;
    if (x >= DM1_VIEWPORT_W) return CAPTURE_ROUTE_RIGHT_CHROME;
    return CAPTURE_ROUTE_LOWER_CHROME;
}

static int geometry_contract_holds(void)
{
    RectI screen = {0, 0, DM1_SCREEN_W, DM1_SCREEN_H};
    RectI viewport = {DM1_VIEWPORT_X, DM1_VIEWPORT_Y,
                      DM1_VIEWPORT_W, DM1_VIEWPORT_H};
    RectI d1c = {DM1_D1C_ZONE_X_VP, DM1_D1C_ZONE_Y_VP,
                 DM1_D1C_ZONE_W, DM1_D1C_ZONE_H};
    RectI portrait_vp = {DM1_PORTRAIT_X_VP, DM1_PORTRAIT_Y_VP,
                         DM1_PORTRAIT_W, DM1_PORTRAIT_H};
    return DM1_SCREEN_W == 320 &&
           DM1_SCREEN_H == 200 &&
           DM1_VIEWPORT_X == 0 &&
           DM1_VIEWPORT_Y == 33 &&
           DM1_VIEWPORT_W == 224 &&
           DM1_VIEWPORT_H == 136 &&
           rect_inside(viewport, screen) &&
           rect_inside(portrait_vp, d1c);
}

static int ordinal4_atlas_contract_holds(void)
{
    int src_x = (DM1_ORDINAL_LEIF & 7) * DM1_PORTRAIT_W;
    int src_y = (DM1_ORDINAL_LEIF >> 3) * DM1_PORTRAIT_H;
    return DM1_PORTRAIT_COLS == 8 &&
           DM1_PORTRAIT_ROWS == 3 &&
           DM1_PORTRAIT_TOTAL == DM1_PORTRAIT_COLS * DM1_PORTRAIT_ROWS &&
           DM1_ORDINAL_LEIF == 4 &&
           src_x == DM1_ORDINAL4_SRC_X &&
           src_y == DM1_ORDINAL4_SRC_Y;
}

static int resting_overlay_contract_holds(void)
{
    RectI screen = {0, 0, DM1_SCREEN_W, DM1_SCREEN_H};
    RectI viewport = {DM1_VIEWPORT_X, DM1_VIEWPORT_Y,
                      DM1_VIEWPORT_W, DM1_VIEWPORT_H};
    RectI portrait_fb = {DM1_VIEWPORT_X + DM1_PORTRAIT_X_VP,
                         DM1_VIEWPORT_Y + DM1_PORTRAIT_Y_VP,
                         DM1_PORTRAIT_W, DM1_PORTRAIT_H};
    RectI overlay = {DM1_RESTING_OVERLAY_X_FB, DM1_RESTING_OVERLAY_Y_FB,
                     DM1_RESTING_OVERLAY_W, DM1_RESTING_OVERLAY_H};
    RectI overlap = rect_intersection(portrait_fb, overlay);

    return rect_inside(overlay, screen) &&
           rect_inside(overlay, viewport) &&
           DM1_RESTING_TEXT_X_FB >= overlay.x &&
           DM1_RESTING_TEXT_Y_FB >= overlay.y &&
           DM1_RESTING_WAKE_TEXT_Y_FB < overlay.y + overlay.h &&
           overlap.x == 100 &&
           overlap.y == 70 &&
           overlap.w == 28 &&
           overlap.h == 27 &&
           overlap.w * overlap.h == 756;
}

static int rest_wake_input_contract_holds(void)
{
    RectI viewport = {DM1_VIEWPORT_X, DM1_VIEWPORT_Y,
                      DM1_VIEWPORT_W, DM1_VIEWPORT_H};
    RectI rest_icon_fb = {
        DM1_VIEWPORT_X + DM1_REST_ICON_X0_VP,
        DM1_VIEWPORT_Y + DM1_REST_ICON_Y0_VP,
        DM1_REST_ICON_X1_VP - DM1_REST_ICON_X0_VP + 1,
        DM1_REST_ICON_Y1_VP - DM1_REST_ICON_Y0_VP + 1
    };
    return rect_inside(rest_icon_fb, viewport) &&
           DM1_REST_ICON_CENTER_X_FB == 196 &&
           DM1_REST_ICON_CENTER_Y_FB == 73 &&
           classify_capture_point(DM1_REST_ICON_CENTER_X_FB,
                                  DM1_REST_ICON_CENTER_Y_FB) ==
               CAPTURE_ROUTE_VIEWPORT &&
           classify_capture_point(112, 78) == CAPTURE_ROUTE_VIEWPORT &&
           classify_capture_point(10, 40) == CAPTURE_ROUTE_VIEWPORT &&
           classify_capture_point(319, 199) == CAPTURE_ROUTE_RIGHT_CHROME;
}

static int shot_label_contract_holds(void)
{
    static const char *labels[] = {
        "hoc_ordinal4_pre_rest",
        "hoc_ordinal4_resting_overlay",
        "hoc_ordinal4_wake_repaint",
        "hoc_ordinal4_post_wake_candidate_return",
        NULL
    };
    int i;
    for (i = 0; labels[i]; ++i) {
        if (strstr(labels[i], "ordinal4") == NULL) return 0;
        if (strstr(labels[i], "hoc_") != labels[i]) return 0;
    }
    return i == 4;
}

static int pacing_contract_holds(void)
{
    return DM1_CAPTURE_DELAY_ENTER_MS >= 9000 &&
           DM1_CAPTURE_DELAY_SETTLE_MS >= 1000 &&
           DM1_CAPTURE_DELAY_REST_MS >= 1000 &&
           DM1_CAPTURE_DELAY_WAKE_MS >= 1000;
}

int main(void)
{
    printf("=== DM1 V1 HoC ordinal 4 RESTING original-capture scaffold ===\n");
    printf("source=ReDMCSB COMMAND.C/CHAMPION.C/DUNVIEW.C/COORD.C + Firestaff M11 RESTING overlay\n");
    printf("route=LEIF ordinal4 pre_rest -> C145 rest -> RESTING overlay -> C146 wake -> wake_repaint\n");
    printf("artifact_root=verification-screens/dm1-v1-hoc-ordinal4-resting-original/ (future)\n\n");

    record("geometry_contract", geometry_contract_holds(),
           "320x200 screen, 224x136 viewport, D1C portrait rect anchored");
    record("ordinal4_atlas_contract", ordinal4_atlas_contract_holds(),
           "LEIF ordinal 4 maps to C026 source rect (128,0,32,29)");
    record("resting_overlay_contract", resting_overlay_contract_holds(),
           "Firestaff RESTING overlay is bounded and overlaps D1C cutout predictably");
    record("rest_wake_input_contract", rest_wake_input_contract_holds(),
           "C145 rest click and C146 wake sample points are route-classified");
    record("shot_label_contract", shot_label_contract_holds(),
           "future artifact labels are stable and ordinal-specific");
    record("pacing_contract", pacing_contract_holds(),
           "capture waits leave room for DOSBox input/render settling");

    printf("\nresult: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail) {
        return 1;
    }
    printf("PASS dm1 v1 HoC ordinal 4 RESTING original-capture scaffold (no pixel-parity claim)\n");
    return 0;
}
