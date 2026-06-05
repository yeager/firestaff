/*
 * firestaff_dm1_v1_entrance_button_click_runtime_probe.c
 *
 * DM1 V1 v2.7.4 release regression probe: Entrance door buttons not clickable.
 *
 * The v2.7.4 release migration to M12_CONFIG_SCALE_FIT /
 * M12_CONFIG_DISPLAY_ASPECT_CONTENT + integerScaling=0 changed the
 * entrance click hit-test path.  The mouse event → window-coordinate →
 * framebuffer-coordinate → command dispatch chain must keep hitting
 * the entrance buttons (enter_dungeon, enter_bonus_dungeon, resume,
 * quit, draw_credits) on the v2.7.4 layout.  This probe:
 *
 *   1. Initializes the M11 renderer at the v2.7.4 default 960x540
 *      window with FIT + content-native aspect, exactly like the
 *      release binary after the layout migration.
 *   2. Reads the actual presentation rect from the renderer so the
 *      click coordinates match the real destRect used for blits.
 *   3. Computes the expected window-space hit point for each
 *      ReDMCSB G0445 entrance button.
 *   4. Drives the same dispatch chain the entrance wait loop uses:
 *        M11_Render_MapWindowToFramebuffer → M11_Entrance_Dispatch
 *      and asserts each click returns the correct source-locked
 *      command id (200/201/202/203/216).
 *
 * ReDMCSB: ENTRANCE.C:739-747 installs entrance input;
 * ENTRANCE.C:850-883 waits for a fresh entrance command; the click
 * route is at G0445 with C407..C411 zones per COMMAND.C:340-353 and
 * DEFS.H:375-384.
 *
 * Exits 0 when every button round-trip succeeds, non-zero otherwise.
 */

#include "render_sdl_m11.h"
#include "entrance_mouse_routes_pc34_compat.h"
#include "main_loop_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
# define m11_setenv(k, v) _putenv_s((k), (v))
#else
# include <stdlib.h>
# define m11_setenv(k, v) setenv((k), (v), 0)
#endif

typedef struct {
    int total;
    int passed;
} InvTally;

static void record(InvTally* t, const char* id, int ok, const char* msg) {
    t->total += 1;
    if (ok) {
        t->passed += 1;
        printf("PASS %s %s\n", id, msg ? msg : "");
    } else {
        printf("FAIL %s %s\n", id, msg ? msg : "");
    }
}

/* IMG3 globals are required by firestaff_m10 when this focused gate links
 * the full runtime libraries through main_loop_m11.c. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    m11_setenv("SDL_VIDEODRIVER", "dummy");

    InvTally t = {0, 0};
    unsigned int i;
    unsigned int count;
    int rc;
    int rectX = 0;
    int rectY = 0;
    int rectW = 0;
    int rectH = 0;

    printf("# firestaff_dm1_v1_entrance_button_click_runtime_probe\n");
    printf("# SDL major version linked: %d\n",
           M11_Render_GetSdlMajorVersion());
    printf("# route_evidence=%s\n", ENTRANCE_Compat_GetMouseRouteEvidence());

    /* v2.7.4 default: 960x540 window, FIT scale, content-native aspect,
     * no integer scaling.  Matches M11_PhaseA_SetDefaultOptions() after
     * the layout migration runs once. */
    rc = M11_Render_Init(960, 540, M11_SCALE_FIT);
    if (rc != M11_RENDER_OK) {
        fprintf(stderr,
                "FATAL: M11_Render_Init failed rc=%d\n", rc);
        printf("summary: 0/0 invariants passed (init failed)\n");
        return 2;
    }
    M11_Render_SetScaleMode(M11_SCALE_FIT);
    M11_Render_SetDisplayAspectMode(M11_DISPLAY_ASPECT_CONTENT);
    M11_Render_SetIntegerScaling(0);

    /* Read the actual presentation rect used for SDL_RenderTexture so
     * the test click coords match the real destRect. */
    if (M11_Render_GetPresentRect(&rectX, &rectY, &rectW, &rectH) != M11_RENDER_OK) {
        fprintf(stderr, "FATAL: M11_Render_GetPresentRect failed\n");
        M11_Render_Shutdown();
        return 2;
    }
    {
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "presentRect=(%d,%d,%d,%d) windowW=%d windowH=%d",
                 rectX, rectY, rectW, rectH,
                 M11_Render_GetWindowWidth(),
                 M11_Render_GetWindowHeight());
        record(&t, "present_rect_valid",
               rectW > 0 && rectH > 0,
               buf);
    }

    /* Every registered entrance button must round-trip the click chain. */
    count = ENTRANCE_Compat_GetMouseRouteCount();
    for (i = 1U; i <= count; ++i) {
        EntranceMouseRouteCompat route;
        int fbCenterX;
        int fbCenterY;
        int windowX;
        int windowY;
        int mappedFbX = 0;
        int mappedFbY = 0;
        int dispatched = 0;
        char id[80];
        char msg[256];

        if (!ENTRANCE_Compat_GetMouseRoute(i, &route)) {
            record(&t, "route_present", 0, "ENTRANCE_Compat_GetMouseRoute returned 0");
            continue;
        }

        /* ReDMCSB ENTRANCE.C:739-747 hit boxes are inclusive.  Use the
         * geometric center of the framebuffer box for the click so the
         * window->framebuffer mapping has the easiest possible time. */
        fbCenterX = route.x + route.w / 2;
        fbCenterY = route.y + route.h / 2;
        /* Convert the framebuffer center to window coordinates using
         * the same rect the renderer blits into.  This is the inverse
         * of M11_Render_MapWindowToFramebuffer, computed inline so
         * the probe does not need a new public symbol. */
        windowX = rectX + (fbCenterX * rectW) / 320;
        windowY = rectY + (fbCenterY * rectH) / 200;

        /* The computed window coord must be inside the rect (sanity check). */
        snprintf(id, sizeof(id), "route_%u_window_in_rect", i);
        snprintf(msg, sizeof(msg),
                 "%s window=(%d,%d) inside rect=(%d,%d,%d,%d)",
                 route.name, windowX, windowY,
                 rectX, rectY, rectW, rectH);
        record(&t, id,
               windowX >= rectX && windowX < rectX + rectW &&
               windowY >= rectY && windowY < rectY + rectH,
               msg);

        /* Now feed the window coord back through the dispatch chain that
         * the real entrance wait loop uses.  M11_Entrance_DispatchSourceLockedPointerCommand
         * is the same routine m11_wait_for_redmcsb_entrance_command()
         * calls for SDL_EVENT_MOUSE_BUTTON_DOWN, so this exercises the
         * full mouse-button -> window-coord -> framebuffer-coord ->
         * command dispatch path. */
        if (!M11_Render_MapWindowToFramebuffer(windowX, windowY,
                                               &mappedFbX, &mappedFbY)) {
            snprintf(id, sizeof(id), "route_%u_map_window_back", i);
            snprintf(msg, sizeof(msg),
                     "MapWindowToFramebuffer(window=%d,%d) returned 0",
                     windowX, windowY);
            record(&t, id, 0, msg);
            continue;
        }
        snprintf(id, sizeof(id), "route_%u_map_window_back", i);
        snprintf(msg, sizeof(msg),
                 "%s window=(%d,%d) -> framebuffer=(%d,%d) center=(%d,%d)",
                 route.name, windowX, windowY, mappedFbX, mappedFbY,
                 fbCenterX, fbCenterY);
        /* Integer scaling is not always an exact inverse at non-320x200
         * window sizes.  The source-locked contract is that the mapped
         * framebuffer coordinate remains inside the ReDMCSB G0445 zone;
         * the following dispatch assertion proves the command table then
         * receives the expected C200/C201/M566/C203/M567 command. */
        record(&t, id,
               mappedFbX >= route.x && mappedFbX < route.x + route.w &&
               mappedFbY >= route.y && mappedFbY < route.y + route.h,
               msg);

        dispatched = M11_Entrance_DispatchSourceLockedPointerCommand(
            mappedFbX, mappedFbY, route.buttonMask);

        snprintf(id, sizeof(id), "route_%u_dispatch", i);
        snprintf(msg, sizeof(msg),
                 "%s -> command=%d expected=%u",
                 route.name, dispatched, route.commandId);
        record(&t, id, (int)dispatched == (int)route.commandId, msg);
    }

    /* Off-button clicks must not dispatch any command (negative guard). */
    {
        int missCommand = -1;
        missCommand = M11_Entrance_DispatchSourceLockedPointerCommand(
            0, 0, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT);
        record(&t, "outside_click_no_command",
               missCommand == 0,
               "framebuffer (0,0) is outside every entrance button");
    }

    M11_Render_Shutdown();

    printf("# summary: %d/%d invariants passed\n", t.passed, t.total);
    return (t.passed == t.total) ? 0 : 1;
}
