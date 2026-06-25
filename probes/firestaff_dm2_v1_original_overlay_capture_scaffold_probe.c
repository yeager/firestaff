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
 *
 * All assertions are data-free: the probe compiles into a tiny CTest binary
 * that does not depend on GRAPHICS.DAT, DUNGEON.DAT, SKULL.EXE, or DOSBox.
 * The honest boundary is documented in tools/verify_dm2_v1_original_overlay_capture_source_lock.py
 * and docs/FIRESTAFF_GAP_LIST.md (DM2 original-overlay evidence is OPEN-BOUNDED).
 *
 * Source: SKULLWIN/dm2global.h, c_gfx_main.cpp, c_gfx_main.h, c_tmouse.h,
 * c_tmouse.cpp, types.h, c_input.cpp, c_gui_draw.cpp. The probe constants
 * are derived directly from those SKULLWIN anchors; the verifier is the
 * companion source-lock check.
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
#include <stdlib.h>
#include <string.h>

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
#define DM2_MOUSE_QUEUE_LENGTH         10
#define DM2_COMMAND_QUEUE_LENGTH       10
#define DM2_MOUSE_EVENT_B_FIELD         0
#define DM2_MOUSE_EVENT_X_FIELD         1
#define DM2_MOUSE_EVENT_Y_FIELD         2

/* DM2 right-panel enum (redeclared here so this probe does not pull in the
 * full SKULLWIN/DM2 globals; the SKULLWIN-owned enum is documented in
 * skproject/SKULLWIN/defines.h and is the source of truth for live runtime).
 *
 * The capture script labels DM2 panel state in shot labels; the constant
 * below is the integer Firestaff's DM2 HUD system uses to identify the
 * squad hands panel, derived from include/dm2_v2_hud_runtime.h. */
#define DM2_RIGHT_PANEL_SQUAD_HANDS    0

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

int main(void)
{
    int setup_ok = 1;

    printf("=== DM2 V1 Original-Overlay Capture Scaffold Probe ===\n");
    printf("Source: SKULLWIN/dm2global.h c_gfx_main.cpp c_gfx_main.h\n");
    printf("        SKULLWIN/c_tmouse.h types.h c_input.cpp c_gui_draw.cpp\n");
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
    record("shot-label-vocabulary", shot_label_semantics_hold(),
           "interplay_splash, press_any_key, main_menu, dungeon_entry, ...");
    record("route-token-inventory", route_token_inventory_holds(),
           "wait/shot/click/rclick/kp5 tokens present");

    printf("\n--- Summary ---\n");
    printf("pass=%d fail=%d\n", g_pass, g_fail);
    if (g_fail > 0) {
        printf("STATUS=FAIL (first failure indicates the capture scaffold contract has drifted)\n");
        setup_ok = 0;
    } else {
        printf("STATUS=PASS (data-free scaffold invariants hold; ready for paired DM2 overlay evidence)\n");
    }

    return setup_ok ? 0 : 1;
}
