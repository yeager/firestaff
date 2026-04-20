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
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char* prog) {
    fprintf(stderr,
            "Usage: %s [--duration <ms>] [--width <px>] [--height <px>] [--script <commands>]\n",
            prog);
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
