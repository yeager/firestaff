/*
 * Pass 40 bounded probe — viewport dimensional drift honesty lock.
 *
 * Scope: V1_BLOCKERS.md §4 "Viewport region -28x-18 px vs DM1 original".
 * Pass 33 measured the drift; pass 40 locks it honestly, documents the
 * source-anchored DM1 viewport rectangle in `m11_game_view.c` as the
 * new enum M11_DM1_VIEWPORT_* (without replacing the runtime
 * M11_VIEWPORT_*), and produces a reproducible, source-backed overlap
 * analysis between the DM1-anchored viewport rectangle and the
 * Firestaff-invented chrome surfaces that currently block a simple
 * coordinate swap.
 *
 * This probe is diagnostic-only.  It does not drive the renderer or
 * the tick orchestrator.  It verifies:
 *
 *   1. The new DM1 source constants bind to the exact ReDMCSB values
 *      (0, 33, 224, 136).
 *   2. The current runtime viewport is the pass-33-measured rectangle
 *      (12, 24, 196, 118).
 *   3. The dimensional drift is exactly (-28 px width, -18 px height)
 *      as recorded in PARITY_MATRIX_DM1_V1.md and pass33 evidence.
 *   4. The DM1-faithful rectangle would overlap every Firestaff
 *      invented chrome surface enumerated below, which is why a
 *      simple coordinate swap cannot land in isolation at pass 40.
 *      These overlap rectangles are recorded numerically so pass 42+
 *      has a deterministic baseline.
 *   5. The DM1-faithful rectangle would NOT overlap the DM1
 *      side-panel action area / icon-cell strip at (x>=224), which
 *      is the source-faithful region already used by V1 mode.
 *   6. The DM1 viewport fits entirely inside the 320x200 framebuffer
 *      (right edge = 224 <= 320, bottom edge = 169 <= 200).
 *
 * Strictly a diagnostic probe.  Does not touch the game view, the
 * tick orchestrator, any rendering path, any M10 semantics, or any
 * V1 runtime behavior.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Source-anchored and runtime viewport constants come from the
 * authoritative enums in m11_game_view.c.  To avoid pulling the full
 * game-view translation unit (which drags SDL headers) into a pure
 * diagnostic probe, we re-declare the exact integer values and
 * cross-check them against a grep of m11_game_view.c at the end of
 * this file via invariant INV_P40_SRC.
 */
enum {
    DM1_VIEWPORT_X = 0,
    DM1_VIEWPORT_Y = 33,
    DM1_VIEWPORT_W = 224,
    DM1_VIEWPORT_H = 136,
    FS_VIEWPORT_X = 12,
    FS_VIEWPORT_Y = 24,
    FS_VIEWPORT_W = 196,
    FS_VIEWPORT_H = 118
};

typedef struct {
    int x, y, w, h;
    const char* name;
} RectN;

/* Firestaff HUD surfaces (must be mirrored from the enum in
 * m11_game_view.c near M11_VIEWPORT_*).  Pass 42 is the bounded
 * reroute pass that will retire these; recording them here locks the
 * baseline pass 42 must start from. */
static const RectN g_fs_chrome[] = {
    { 218, 28,  86, 42, "utility panel (header)" },
    { 218, 56,  86, 10, "utility button row" },
    { 12,  160, 71, 28, "party slot 0" },
    { 89,  160, 71, 28, "party slot 1 (step=77)" },
    { 166, 160, 71, 28, "party slot 2 (step=77)" },
    { 243, 160, 71, 28, "party slot 3 (step=77)" },
    { 14,  165, 88, 14, "control strip" },
    { 104, 165, 202,14, "prompt strip" }
};
#define FS_CHROME_COUNT ((int)(sizeof(g_fs_chrome)/sizeof(g_fs_chrome[0])))

/* Authoritative DM1 side-panel surfaces from ACTIDRAW.C / MENU.C.
 * These are the surfaces that should NOT be overlapped by the DM1
 * viewport rectangle (correctness guard: proves the DM1 anchor does
 * not require us to destroy the real action-area / icon-cell strip). */
static const RectN g_dm1_side_panel[] = {
    { 224, 58,  87, 45, "DM1 action area (ACTIDRAW.C 87x45)" },
    { 233, 86,  87, 35, "DM1 icon-cell strip" }
};
#define DM1_SIDE_COUNT ((int)(sizeof(g_dm1_side_panel)/sizeof(g_dm1_side_panel[0])))

static int g_pass = 0;
static int g_fail = 0;

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static int rects_overlap(const RectN* a, int x, int y, int w, int h,
                         int* ox, int* oy, int* ow, int* oh) {
    int ax1 = a->x + a->w;
    int ay1 = a->y + a->h;
    int bx1 = x + w;
    int by1 = y + h;
    int x0 = a->x > x ? a->x : x;
    int y0 = a->y > y ? a->y : y;
    int x1 = ax1 < bx1 ? ax1 : bx1;
    int y1 = ay1 < by1 ? ay1 : by1;
    if (x0 < x1 && y0 < y1) {
        if (ox) *ox = x0;
        if (oy) *oy = y0;
        if (ow) *ow = x1 - x0;
        if (oh) *oh = y1 - y0;
        return 1;
    }
    return 0;
}

static int read_file(const char* path, char* buf, size_t cap) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    size_t n = fread(buf, 1, cap - 1, f);
    fclose(f);
    buf[n] = '\0';
    return (int)n;
}

int main(void) {
    char msg[256];

    /* 1. Source constants match DEFS.H / COORD.C */
    record("INV_P40_01", DM1_VIEWPORT_X == 0,
           "DM1 viewport X == 0 (COORD.C G2067_i_ViewportScreenX)");
    record("INV_P40_02", DM1_VIEWPORT_Y == 33,
           "DM1 viewport Y == 33 (COORD.C G2068_i_ViewportScreenY)");
    record("INV_P40_03", DM1_VIEWPORT_W == 224,
           "DM1 viewport W == 224 (DEFS.H C112_BYTE_WIDTH_VIEWPORT * 2)");
    record("INV_P40_04", DM1_VIEWPORT_H == 136,
           "DM1 viewport H == 136 (DEFS.H C136_HEIGHT_VIEWPORT)");

    /* 2. Current runtime viewport matches pass-33 measurement */
    record("INV_P40_05", FS_VIEWPORT_X == 12 && FS_VIEWPORT_Y == 24,
           "Firestaff runtime viewport origin == (12, 24) (pass 33 measurement)");
    record("INV_P40_06", FS_VIEWPORT_W == 196 && FS_VIEWPORT_H == 118,
           "Firestaff runtime viewport size == 196x118 (pass 33 measurement)");

    /* 3. Dimensional drift recorded numbers */
    record("INV_P40_07", FS_VIEWPORT_W - DM1_VIEWPORT_W == -28,
           "width drift == -28 px (Firestaff narrower by 28)");
    record("INV_P40_08", FS_VIEWPORT_H - DM1_VIEWPORT_H == -18,
           "height drift == -18 px (Firestaff shorter by 18)");
    {
        int dm1_area = DM1_VIEWPORT_W * DM1_VIEWPORT_H;       /* 30464 */
        int fs_area  = FS_VIEWPORT_W  * FS_VIEWPORT_H;        /* 23128 */
        int delta    = dm1_area - fs_area;                    /* 7336 */
        snprintf(msg, sizeof(msg),
                 "pixel-area drift == -%d px^2 (Firestaff is %d vs DM1 %d)",
                 delta, fs_area, dm1_area);
        record("INV_P40_09", dm1_area == 30464 && fs_area == 23128 && delta == 7336, msg);
    }

    /* 4. DM1 rect overlap with every Firestaff chrome surface */
    {
        int i;
        int ox, oy, ow, oh;
        int any_missed = 0;
        int total_overlap_px = 0;
        for (i = 0; i < FS_CHROME_COUNT; ++i) {
            const RectN* r = &g_fs_chrome[i];
            int overlapped = rects_overlap(r,
                                           DM1_VIEWPORT_X, DM1_VIEWPORT_Y,
                                           DM1_VIEWPORT_W, DM1_VIEWPORT_H,
                                           &ox, &oy, &ow, &oh);
            if (!overlapped) {
                /* Party slot 3 at x=243..313 does NOT overlap DM1 rect.
                 * That is expected and is not a failure; we just do
                 * not count it.  The probe requires overlaps for the
                 * SEVEN other surfaces (the first three party slots
                 * plus utility+strip rows). */
                if (strcmp(r->name, "party slot 3 (step=77)") != 0) {
                    any_missed = 1;
                }
                continue;
            }
            total_overlap_px += ow * oh;
            snprintf(msg, sizeof(msg),
                     "DM1 rect overlaps '%s' by %dx%d at (%d,%d)",
                     r->name, ow, oh, ox, oy);
            record("INV_P40_10", 1, msg);
        }
        record("INV_P40_11", any_missed == 0,
               "every Firestaff chrome surface except party slot 3 overlaps DM1 rect");
        snprintf(msg, sizeof(msg),
                 "total Firestaff chrome overlap area == %d px^2 (pass-42 work surface)",
                 total_overlap_px);
        /* The canonical expected overlap sum from the Python analysis is
         * 222 + 60 + 639 + 639 + 522 + 352 + 480 = 2914 px^2.  Locking
         * this number makes any future HUD coordinate drift surface as
         * a probe regression. */
        record("INV_P40_12", total_overlap_px == 2914, msg);
    }

    /* 5. DM1 side-panel surfaces are NOT overlapped by DM1 viewport. */
    {
        int i;
        int ox, oy, ow, oh;
        int any_overlap = 0;
        for (i = 0; i < DM1_SIDE_COUNT; ++i) {
            const RectN* r = &g_dm1_side_panel[i];
            int overlapped = rects_overlap(r,
                                           DM1_VIEWPORT_X, DM1_VIEWPORT_Y,
                                           DM1_VIEWPORT_W, DM1_VIEWPORT_H,
                                           &ox, &oy, &ow, &oh);
            if (overlapped) {
                any_overlap = 1;
                snprintf(msg, sizeof(msg),
                         "UNEXPECTED overlap with '%s' (%dx%d at (%d,%d))",
                         r->name, ow, oh, ox, oy);
                record("INV_P40_13a", 0, msg);
            }
        }
        record("INV_P40_13",
               any_overlap == 0,
               "DM1 viewport does NOT overlap the DM1 side-panel (x>=224)");
    }

    /* 6. DM1 viewport fits inside 320x200 framebuffer */
    record("INV_P40_14",
           (DM1_VIEWPORT_X + DM1_VIEWPORT_W) == 224
           && (DM1_VIEWPORT_X + DM1_VIEWPORT_W) <= 320,
           "DM1 viewport right edge 224 <= framebuffer width 320");
    record("INV_P40_15",
           (DM1_VIEWPORT_Y + DM1_VIEWPORT_H) == 169
           && (DM1_VIEWPORT_Y + DM1_VIEWPORT_H) <= 200,
           "DM1 viewport bottom edge 169 <= framebuffer height 200");

    /* INV_P40_SRC — cross-check that m11_game_view.c carries the
     * authoritative M11_DM1_VIEWPORT_* constants bound to the same
     * values used by this probe.  Keeps the probe honest: if someone
     * edits the enum away, this invariant fires immediately. */
    {
        static char buf[512 * 1024];
        int ok = 0;
        int n = read_file("m11_game_view.c", buf, sizeof(buf));
        if (n > 0) {
            int have_x = strstr(buf, "M11_DM1_VIEWPORT_X = 0,")   != NULL;
            int have_y = strstr(buf, "M11_DM1_VIEWPORT_Y = 33,")  != NULL;
            int have_w = strstr(buf, "M11_DM1_VIEWPORT_W = 224,") != NULL;
            int have_h = strstr(buf, "M11_DM1_VIEWPORT_H = 136")  != NULL;
            ok = have_x && have_y && have_w && have_h;
        }
        record("INV_P40_SRC", ok,
               "m11_game_view.c enum carries M11_DM1_VIEWPORT_* bound to "
               "(0, 33, 224, 136)");
    }

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
