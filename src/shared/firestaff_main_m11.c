/*
 * firestaff_main_m11.c — Phase A proof-of-life main binary.
 *
 * Opens a black 640x400 window via main_loop_m11, holds it for the
 * requested number of milliseconds (default 500), then exits cleanly.
 * Accepts "--duration <ms>" to override.
 *
 * This is the skeleton of the real game entry point; later phases will
 * replace the body with the full semi-fixed-timestep loop.
 */

#include "main_loop_m11.h"

#include "firestaff_version.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 global state required by the GRAPHICS.DAT image decompressor */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static void usage(const char* prog) {
    fprintf(stderr,
            "Usage: %s [options]\n"
            "  --duration <ms>    Run for specified milliseconds (-1 = run until exit, 0 = single frame)\n"
            "  --width <px>        Window width (default: 640)\n"
            "  --height <px>       Window height (default: 400)\n"
            "  --scale-mode <n>    Graphics mode: 1=V1, 2=V2.1, 3=V2.2\n"
            "  --script <cmds>     Comma-separated input script: up,down,left,right,enter,esc\n"
            "  --data-dir <path>   Asset directory (default: FIRESTAFF_DATA env var)\n"
            "  --fullscreen        Run in fullscreen mode\n"
            "  --no-vsync          Disable vertical sync\n"
            "  --fps               Show FPS counter\n"
            "  --game <id>         Start game directly: dm1, csb, dm2, nexus, theron\n"
            "  --menu              Show startup menu even when --game is set\n"
            "  --version           Show version and exit\n"
            "  --help, -h          Show this help\n",
            prog);
}

static int is_game_id(const char* value) {
    return value &&
           (strcmp(value, "dm1") == 0 ||
            strcmp(value, "csb") == 0 ||
            strcmp(value, "dm2") == 0 ||
            strcmp(value, "nexus") == 0 ||
            strcmp(value, "theron") == 0);
}

int main(int argc, char** argv) {
    M11_PhaseA_Options opts;
    M11_PhaseA_SetDefaultOptions(&opts);

    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];
        if (strcmp(a, "--help") == 0 || strcmp(a, "-h") == 0) {
            usage(argv[0]);
            return 0;
        }
        if (strcmp(a, "--duration") == 0 && i + 1 < argc) {
            opts.durationMs = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--width") == 0 && i + 1 < argc) {
            opts.windowWidth = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--height") == 0 && i + 1 < argc) {
            opts.windowHeight = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--script") == 0 && i + 1 < argc) {
            opts.script = argv[++i];
            continue;
        }
        if (strcmp(a, "--data-dir") == 0 && i + 1 < argc) {
            opts.dataDir = argv[++i];
            continue;
        }
        if (strcmp(a, "--game") == 0 && i + 1 < argc) {
            opts.gameId = argv[++i];
            opts.directLaunch = 1;
            continue;
        }
        if (strcmp(a, "--menu") == 0) {
            opts.directLaunch = 0;
            continue;
        }
        if (strcmp(a, "--version") == 0) {
            fprintf(stderr, "Firestaff " FIRESTAFF_VERSION_STRING "\n");
            return 0;
        }
        if (strcmp(a, "--scale-mode") == 0 && i + 1 < argc) {
            opts.scaleMode = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--fullscreen") == 0) {
            /* not yet wired */
            continue;
        }
        if (strcmp(a, "--no-vsync") == 0) {
            /* not yet wired */
            continue;
        }
        if (strcmp(a, "--fps") == 0) {
            /* not yet wired */
            continue;
        }
        if (is_game_id(a)) {
            opts.gameId = a;
            opts.directLaunch = 1;
            continue;
        }
        fprintf(stderr, "firestaff: unknown argument '%s'\n", a);
        usage(argv[0]);
        return 2;
    }

    int rc = M11_PhaseA_Run(&opts);
    if (rc != 0) {
        fprintf(stderr, "firestaff: phase-a run failed (rc=%d)\n", rc);
        return 1;
    }
    return 0;
}
