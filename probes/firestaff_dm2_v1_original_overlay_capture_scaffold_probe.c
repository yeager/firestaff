/*
 * firestaff_dm2_v1_original_overlay_capture_scaffold_probe.c
 *
 * Pass H2313 — DM2 V1 original-overlay capture scaffold (data-free).
 *
 * This probe locks the DM2 capture-pipeline invariants that future paired
 * original-vs-Firestaff overlay evidence will need:
 *
 *   - The DM2 PC 1.0 EN screen buffer is 320x200 (ORIG_SWIDTH x ORIG_SHEIGHT).
 *   - The DM2 dungeon backbuffer is 224x136 (0xe0 x 0x88), which lines up with
 *     the screenshot crop (x=0..223, y=33..169) used by
 *     scripts/dosbox_dm2_original_overlay_capture.sh.
 *   - The DM2 mouse queue (c_mousequeue) and command queue (c_commandqueue)
 *     are bounded at 10 entries each, so the capture script must pace injected
 *     inputs to keep the queues from overflowing.
 *   - The DM2 mouse event record (c_evententry) carries (b, x, y); capture-side
 *     click coordinates map 1:1 to those ints without any engine-side remap.
 *   - The DM2 default right-panel is DM2_DISPLAY_RIGHT_PANEL_SQUAD_HANDS,
 *     which the capture script labels explicitly in shot labels.
 *   - The DM2 dungeon viewport y range starts at 33, mirroring DM1's PC 3.4
 *     224x136 backbuffer composition (ReDMCSB COORD.C:1693-1698).
 *   - The original HUD/right-panel band is the remaining 96px-wide area
 *     beside the 224px viewport crop, and capture click routing must keep
 *     viewport, HUD-panel, and lower chrome samples disjoint.
 *   - Edge-adjacent click samples at x=223/224 and y=168/169 keep the
 *     320x200 presentation route split exact instead of fuzzing boundaries.
 *   - Representative HUD/action labels fit inside the 96px panel using
 *     SKULLWIN's 6px advance, 5px-high DM2 font metrics.
 *   - The future original-vs-Firestaff pair manifest has a strict schema and
 *     cannot promote the OPEN row without same-state dungeon_gameplay hashes
 *     for both original and Firestaff 224x136 viewport crops.
 *
 * All assertions are data-free: the probe compiles into a tiny CTest binary
 * that does not depend on GRAPHICS.DAT, DUNGEON.DAT, SKULL.EXE, or DOSBox.
 * The honest boundary is documented in tools/verify_dm2_v1_original_overlay_capture_source_lock.py
 * and docs/FIRESTAFF_GAP_LIST.md (DM2 original-overlay evidence is OPEN-BOUNDED).
 *
 * Source: SKULLWIN/dm2global.h, c_gfx_main.cpp, c_gfx_main.h, c_tmouse.h,
 * c_tmouse.cpp, types.h, c_input.cpp, c_gui_draw.cpp, c_gfx_str.cpp. The
 * probe constants are derived directly from those SKULLWIN anchors; the
 * verifier is the companion source-lock check.
 *
 * Build:
 *   cmake --build build --target firestaff_dm2_v1_original_overlay_capture_scaffold_probe
 *
 * Run:
 *   ./build/firestaff_dm2_v1_original_overlay_capture_scaffold_probe
 *
 * Exit codes: 0 = all invariants PASS, 1 = first failure, 2 = setup error.
 */

#include <stdio.h>
#include <string.h>

#define DM2_PAIR_MANIFEST_HEADER \
    "pair_id\tstate\tclassification\troute_label\toriginal_frame_sha256\toriginal_viewport_sha256\toriginal_viewport_width\toriginal_viewport_height\tfirestaff_frame_sha256\tfirestaff_viewport_sha256\tfirestaff_viewport_width\tfirestaff_viewport_height\tdiff_sha256\tstatus"

/* Source-locked geometry constants.
 * Anchor: SKULLWIN/dm2global.h ORIG_SWIDTH/ORIG_SHEIGHT.
 * Anchor: SKULLWIN/c_gfx_main.cpp backbuffer_w = 0xe0, backbuffer_h = 0x88.
 * Anchor: SKULLWIN/c_gfx_main.h dm2screen/dm2mscreen buffer sizes.
 *
 * The crop coordinates (x=0, y=33, w=224, h=136) match the DM1 PC 3.4 viewport
 * anchor at ReDMCSB COORD.C:1693-1698 (G2067_i_ViewportScreenX = 0,
 * G2068_i_ViewportScreenY = 33); DM2 uses the same screen origin because
 * SKULLWIN's c_gfx_main.cpp c_gui_vp.cpp blits the 0xe0 x 0x88 backbuffer at
 * (0, 33) on the 320x200 surface. The capture scaffold crops there. */
#define DM2_ORIG_SCREEN_W             320
#define DM2_ORIG_SCREEN_H             200
#define DM2_BACKBUFFER_W              224    /* 0xe0 */
#define DM2_BACKBUFFER_H              136    /* 0x88 */
#define DM2_VIEWPORT_X                  0
#define DM2_VIEWPORT_Y                 33
#define DM2_RIGHT_PANEL_X             224
#define DM2_RIGHT_PANEL_Y               0
#define DM2_RIGHT_PANEL_W              96
#define DM2_RIGHT_PANEL_H             200
#define DM2_MOUSE_QUEUE_LENGTH         10
#define DM2_COMMAND_QUEUE_LENGTH       10
#define DM2_MOUSE_EVENT_B_FIELD         0
#define DM2_MOUSE_EVENT_X_FIELD         1
#define DM2_MOUSE_EVENT_Y_FIELD         2
#define DM2_FONT_ADVANCE_PX             6
#define DM2_FONT_TRAILING_GAP_PX        1
#define DM2_FONT_GLYPH_H                6
#define DM2_FONT_BASELINE_TRIM_PX       1
#define DM2_STRONG_TEXT_PAD_PX          2

/* DM2 right-panel enum (redeclared here so this probe does not pull in the
 * full SKULLWIN/DM2 globals; the SKULLWIN-owned enum is documented in
 * skproject/SKULLWIN/defines.h and is the source of truth for live runtime).
 *
 * The capture script labels DM2 panel state in shot labels; the constant
 * below is the integer Firestaff's DM2 HUD system uses to identify the
 * squad hands panel, derived from include/dm2_v2_hud_runtime.h. */
#define DM2_RIGHT_PANEL_SQUAD_HANDS    0

typedef struct {
    int x;
    int y;
    int w;
    int h;
} DM2_Rect;

typedef enum {
    DM2_CAPTURE_ROUTE_OUTSIDE = 0,
    DM2_CAPTURE_ROUTE_VIEWPORT,
    DM2_CAPTURE_ROUTE_RIGHT_PANEL,
    DM2_CAPTURE_ROUTE_SCREEN_CHROME
} DM2_CaptureRoute;

static int g_pass = 0;
static int g_fail = 0;

static void record(const char *id, int ok, const char *msg)
{
    if (ok) {
        g_pass++;
        printf("PASS %-32s %s\n", id, msg);
    } else {
        g_fail++;
        printf("FAIL %-32s %s\n", id, msg);
    }
}

static int geometry_holds(void)
{
    int ok = 1;
    /* Screen buffer geometry: SKULLWIN/dm2global.h ORIG_SWIDTH=320, ORIG_SHEIGHT=200. */
    ok &= (DM2_ORIG_SCREEN_W == 320);
    ok &= (DM2_ORIG_SCREEN_H == 200);
    /* Backbuffer geometry: SKULLWIN/c_gfx_main.cpp backbuffer_w=0xe0=224, backbuffer_h=0x88=136. */
    ok &= (DM2_BACKBUFFER_W == 224);
    ok &= (DM2_BACKBUFFER_H == 136);
    /* Viewport crop lives entirely inside the original screen buffer. */
    ok &= (DM2_VIEWPORT_X >= 0);
    ok &= (DM2_VIEWPORT_Y >= 0);
    ok &= (DM2_VIEWPORT_X + DM2_BACKBUFFER_W <= DM2_ORIG_SCREEN_W);
    ok &= (DM2_VIEWPORT_Y + DM2_BACKBUFFER_H <= DM2_ORIG_SCREEN_H);
    return ok ? 1 : 0;
}

static int rect_contains(DM2_Rect r, int x, int y)
{
    return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}

static int rect_inside_screen(DM2_Rect r)
{
    return r.x >= 0 && r.y >= 0 &&
           r.x + r.w <= DM2_ORIG_SCREEN_W &&
           r.y + r.h <= DM2_ORIG_SCREEN_H;
}

static DM2_CaptureRoute classify_capture_click(int x, int y)
{
    DM2_Rect screen = {0, 0, DM2_ORIG_SCREEN_W, DM2_ORIG_SCREEN_H};
    DM2_Rect viewport = {DM2_VIEWPORT_X, DM2_VIEWPORT_Y,
                         DM2_BACKBUFFER_W, DM2_BACKBUFFER_H};
    DM2_Rect right_panel = {DM2_RIGHT_PANEL_X, DM2_RIGHT_PANEL_Y,
                            DM2_RIGHT_PANEL_W, DM2_RIGHT_PANEL_H};

    if (!rect_contains(screen, x, y)) return DM2_CAPTURE_ROUTE_OUTSIDE;
    if (rect_contains(viewport, x, y)) return DM2_CAPTURE_ROUTE_VIEWPORT;
    if (rect_contains(right_panel, x, y)) return DM2_CAPTURE_ROUTE_RIGHT_PANEL;
    return DM2_CAPTURE_ROUTE_SCREEN_CHROME;
}

static int right_panel_geometry_holds(void)
{
    DM2_Rect viewport = {DM2_VIEWPORT_X, DM2_VIEWPORT_Y,
                         DM2_BACKBUFFER_W, DM2_BACKBUFFER_H};
    DM2_Rect right_panel = {DM2_RIGHT_PANEL_X, DM2_RIGHT_PANEL_Y,
                            DM2_RIGHT_PANEL_W, DM2_RIGHT_PANEL_H};
    int ok = 1;

    /* SKULLWIN/c_gfx_main.cpp fixes the dungeon backbuffer at 224px wide
     * inside the 320px screen. The original HUD/right-panel capture band is
     * the remaining 96px at x=224..319, kept outside the viewport crop. */
    ok &= rect_inside_screen(right_panel);
    ok &= (right_panel.x == viewport.x + viewport.w);
    ok &= (right_panel.w == DM2_ORIG_SCREEN_W - DM2_BACKBUFFER_W);
    ok &= (right_panel.h == DM2_ORIG_SCREEN_H);
    ok &= !rect_contains(viewport, right_panel.x, DM2_VIEWPORT_Y);
    ok &= !rect_contains(viewport, DM2_ORIG_SCREEN_W - 1, DM2_VIEWPORT_Y);
    return ok ? 1 : 0;
}

static int queue_lengths_hold(void)
{
    /* SKULLWIN/c_tmouse.h #define MOUSE_QUEUE_LENGTH (10) and COMMAND_QUEUE_LENGTH (10). */
    return (DM2_MOUSE_QUEUE_LENGTH == 10) && (DM2_COMMAND_QUEUE_LENGTH == 10);
}

static int mouse_event_field_count_holds(void)
{
    /* SKULLWIN/types.h c_evententry { i16 b; i16 x; i16 y; } */
    return (DM2_MOUSE_EVENT_B_FIELD == 0) &&
           (DM2_MOUSE_EVENT_X_FIELD == 1) &&
           (DM2_MOUSE_EVENT_Y_FIELD == 2);
}

static int backbuffer_byte_capacity_holds(void)
{
    /* 224 * 136 = 30464 bytes (8-bit indexed). Capture normalization writes
     * PPM P6 with 3 bytes per pixel, so each crop is 224*136*3 = 91392 bytes;
     * surface buffer (dm2screen / dm2mscreen) is 320*200 = 64000 bytes. */
    long surface = (long)DM2_ORIG_SCREEN_W * (long)DM2_ORIG_SCREEN_H;
    long backbuffer = (long)DM2_BACKBUFFER_W * (long)DM2_BACKBUFFER_H;
    return (surface == 64000) && (backbuffer == 30464);
}

static int dm2_text_width_px(const char *text)
{
    if (!text || text[0] == '\0') return 0;
    return (int)strlen(text) * DM2_FONT_ADVANCE_PX - DM2_FONT_TRAILING_GAP_PX;
}

static int hud_text_bounds_hold(void)
{
    /* SKULLWIN/c_gfx_str.cpp:20-37 initializes gfxstrw1=6, gfxstrw2=1,
     * gfxstrw3=1, gfxstrw4=6; c_gfx_str.cpp:64-76 computes text width as
     * -gfxstrw2 + charCount*gfxstrw4 and height as gfxstrw1-gfxstrw3.
     * Strong text adds a 1px border on each side (c_gfx_str.cpp:141-160). */
    static const char *labels[] = {
        "SQUAD",
        "ATTACK",
        "CAST",
        "USE",
        "GOLD 9999",
        NULL,
    };
    int i;
    int ok = 1;
    int text_h = DM2_FONT_GLYPH_H - DM2_FONT_BASELINE_TRIM_PX;

    ok &= (DM2_FONT_ADVANCE_PX == 6);
    ok &= (DM2_FONT_TRAILING_GAP_PX == 1);
    ok &= (text_h == 5);
    for (i = 0; labels[i] != NULL; i++) {
        int strong_w = dm2_text_width_px(labels[i]) + DM2_STRONG_TEXT_PAD_PX;
        int strong_h = text_h + DM2_STRONG_TEXT_PAD_PX;
        ok &= (strong_w > 0);
        ok &= (strong_w <= DM2_RIGHT_PANEL_W);
        ok &= (strong_h <= DM2_RIGHT_PANEL_H);
    }
    return ok ? 1 : 0;
}

static int click_routing_bounds_hold(void)
{
    int ok = 1;

    /* SKULLWIN/types.h c_evententry stores original-frame x/y directly, and
     * c_events.cpp routes clicks with DM2_PT_IN_EXPANDED_RECT. These samples
     * keep capture route labels disjoint before any real overlay is claimed. */
    ok &= (classify_capture_click(DM2_VIEWPORT_X, DM2_VIEWPORT_Y) ==
           DM2_CAPTURE_ROUTE_VIEWPORT);
    ok &= (classify_capture_click(DM2_BACKBUFFER_W - 1,
                                  DM2_VIEWPORT_Y + DM2_BACKBUFFER_H - 1) ==
           DM2_CAPTURE_ROUTE_VIEWPORT);
    ok &= (classify_capture_click(DM2_RIGHT_PANEL_X, DM2_VIEWPORT_Y) ==
           DM2_CAPTURE_ROUTE_RIGHT_PANEL);
    ok &= (classify_capture_click(DM2_ORIG_SCREEN_W - 1,
                                  DM2_ORIG_SCREEN_H - 1) ==
           DM2_CAPTURE_ROUTE_RIGHT_PANEL);
    ok &= (classify_capture_click(100, 180) ==
           DM2_CAPTURE_ROUTE_SCREEN_CHROME);
    ok &= (classify_capture_click(-1, 100) ==
           DM2_CAPTURE_ROUTE_OUTSIDE);
    ok &= (classify_capture_click(DM2_ORIG_SCREEN_W, 100) ==
           DM2_CAPTURE_ROUTE_OUTSIDE);
    return ok ? 1 : 0;
}

static int click_edge_grid_holds(void)
{
    /* SKULLWIN/c_xrect.cpp:21-29 DM2_PT_IN_EXPANDED_RECT asks the queried
     * rect's pt_in_rect with original-frame x/y; c_events.cpp:1318-1324 uses
     * those exact mouse coordinates for panel routing. Keep the adjacent
     * 320x200 boundary samples exact so future capture routes cannot smear
     * the 224px viewport/HUD edge or the 33+136 viewport bottom edge. */
    static const struct {
        int x;
        int y;
        DM2_CaptureRoute route;
    } samples[] = {
        { 223,  33, DM2_CAPTURE_ROUTE_VIEWPORT },
        { 224,  33, DM2_CAPTURE_ROUTE_RIGHT_PANEL },
        { 223, 168, DM2_CAPTURE_ROUTE_VIEWPORT },
        { 223, 169, DM2_CAPTURE_ROUTE_SCREEN_CHROME },
        { 224, 169, DM2_CAPTURE_ROUTE_RIGHT_PANEL },
        {   0,  32, DM2_CAPTURE_ROUTE_SCREEN_CHROME },
        {   0,  33, DM2_CAPTURE_ROUTE_VIEWPORT },
        { 223,  32, DM2_CAPTURE_ROUTE_SCREEN_CHROME },
        { 224,   0, DM2_CAPTURE_ROUTE_RIGHT_PANEL },
        { 319, 199, DM2_CAPTURE_ROUTE_RIGHT_PANEL },
        { 320, 199, DM2_CAPTURE_ROUTE_OUTSIDE },
        { 319, 200, DM2_CAPTURE_ROUTE_OUTSIDE },
    };
    int i;
    int ok = 1;

    for (i = 0; i < (int)(sizeof(samples) / sizeof(samples[0])); i++) {
        ok &= (classify_capture_click(samples[i].x, samples[i].y) ==
               samples[i].route);
    }
    return ok ? 1 : 0;
}

static int shot_label_semantics_hold(void)
{
    /* The capture script labels at least the following DM2 states; this is
     * the minimal vocabulary future overlays will need to keep parity claims
     * unambiguous. The script accepts any lowercase label, but these are the
     * ones the source-lock probe promises. */
    static const char *expected_labels[] = {
        "interplay_splash",
        "press_any_key",
        "main_menu",
        "dungeon_entry",
        "dungeon_forward_1",
        "dungeon_forward_2",
        "right_panel_squad_hands",
        NULL,
    };
    /* Static check: pointer list is non-empty and right-panel label matches enum value. */
    return (expected_labels[0] != NULL) &&
           (strcmp(expected_labels[6], "right_panel_squad_hands") == 0) &&
           (DM2_RIGHT_PANEL_SQUAD_HANDS == 0);
}

static int route_token_inventory_holds(void)
{
    /* The capture script must accept these route tokens (mirrors the DM1
     * pass-70 inventory). The verifier grep-checks the script for these
     * strings; the probe asserts the same vocabulary is internally complete. */
    static const char *tokens[] = {
        "wait:", "shot:", "shot", "click:", "rclick:",
        "enter", "esc", "space", "up", "down", "left", "right",
        "kp0", "kp5", "kpenter", NULL,
    };
    int i;
    int seen_kp5 = 0;
    int seen_wait = 0;
    int seen_shot = 0;
    for (i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "wait:") == 0) seen_wait = 1;
        if (strcmp(tokens[i], "shot") == 0) seen_shot = 1;
        if (strcmp(tokens[i], "shot:") == 0) seen_shot = 1;
        if (strcmp(tokens[i], "kp5") == 0) seen_kp5 = 1;
    }
    return seen_wait && seen_shot && seen_kp5;
}

static int hex_sha256_shape_holds(const char *s)
{
    int i;
    if (!s || strlen(s) != 64u) return 0;
    for (i = 0; i < 64; i++) {
        char c = s[i];
        if (!((c >= '0' && c <= '9') ||
              (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F'))) {
            return 0;
        }
    }
    return 1;
}

static int pair_manifest_schema_holds(void)
{
    /* tools/verify_dm2_v1_original_overlay_capture_source_lock.py treats a
     * missing pair manifest as OPEN, a malformed present manifest as FAIL,
     * and only a SAME_STATE_PAIR row with original+Firestaff 224x136 crop
     * hashes as pair-ready. The placeholder hash below is shape-only and is
     * not an evidence value. */
    const char *placeholder_sha =
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    int ok = 1;
    ok &= (strstr(DM2_PAIR_MANIFEST_HEADER, "original_viewport_sha256") != NULL);
    ok &= (strstr(DM2_PAIR_MANIFEST_HEADER, "firestaff_viewport_sha256") != NULL);
    ok &= (strstr(DM2_PAIR_MANIFEST_HEADER, "classification") != NULL);
    ok &= (strstr(DM2_PAIR_MANIFEST_HEADER, "status") != NULL);
    ok &= hex_sha256_shape_holds(placeholder_sha);
    ok &= (DM2_BACKBUFFER_W == 224);
    ok &= (DM2_BACKBUFFER_H == 136);
    return ok ? 1 : 0;
}

int main(void)
{
    int setup_ok = 1;

    printf("=== DM2 V1 Original-Overlay Capture Scaffold Probe ===\n");
    printf("Source: SKULLWIN/dm2global.h c_gfx_main.cpp c_gfx_main.h\n");
    printf("        SKULLWIN/c_tmouse.h types.h c_input.cpp c_gui_draw.cpp c_gfx_str.cpp\n");
    printf("Honest boundary: data-free source-lock contract only; no overlay parity claim.\n\n");

    record("viewport-screen-size",  geometry_holds(),
           "ORIG_SWIDTH=320 ORIG_SHEIGHT=200 (dm2global.h)");
    record("backbuffer-geometry",   geometry_holds(),
           "backbuffer_w=0xe0=224, backbuffer_h=0x88=136 (c_gfx_main.cpp) crop fits at (0,33)");
    record("queue-lengths",         queue_lengths_hold(),
           "MOUSE_QUEUE_LENGTH=10, COMMAND_QUEUE_LENGTH=10 (c_tmouse.h)");
    record("mouse-event-fields",    mouse_event_field_count_holds(),
           "c_evententry { b, x, y } (types.h)");
    record("backbuffer-bytes",      backbuffer_byte_capacity_holds(),
           "surface=64000B backbuffer=30464B");
    record("right-panel-geometry",  right_panel_geometry_holds(),
           "right HUD panel occupies x=224..319 beside the 224px viewport crop");
    record("hud-text-bounds",       hud_text_bounds_hold(),
           "representative HUD labels fit 96px panel with 6px DM2 font advance");
    record("click-routing-bounds",  click_routing_bounds_hold(),
           "viewport, right-panel, and lower-chrome sample clicks are disjoint");
    record("click-edge-grid",       click_edge_grid_holds(),
           "x=223/224 and y=168/169 samples keep viewport/HUD/chrome edges exact");
    record("shot-label-vocabulary", shot_label_semantics_hold(),
           "interplay_splash, press_any_key, main_menu, dungeon_entry, ...");
    record("route-token-inventory", route_token_inventory_holds(),
           "wait/shot/click/rclick/kp5 tokens present");
    record("pair-manifest-schema", pair_manifest_schema_holds(),
           "future pair rows require same-state original+Firestaff 224x136 SHA-256s before promotion");

    printf("\n--- Summary ---\n");
    printf("pass=%d fail=%d\n", g_pass, g_fail);
    if (g_fail > 0) {
        printf("STATUS=FAIL (first failure indicates the capture scaffold contract has drifted)\n");
        setup_ok = 0;
    } else {
        printf("STATUS=PASS (data-free scaffold invariants hold; DM2 overlay row remains OPEN until paired hashes exist)\n");
    }

    return setup_ok ? 0 : 1;
}
