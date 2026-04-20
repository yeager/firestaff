/*
 * Firestaff headless CLI driver.
 *
 * Usage:
 *   firestaff_headless --dungeon PATH [--stream PATH] [--seed N] [--ticks N]
 *
 * If --stream is given, tick inputs come from the stream file (text,
 * one line per tick: TICK CMD ARG1 ARG2 in hex).
 * If --ticks is given (and no stream), runs N CMD_NONE ticks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"

static void usage(void) {
    fprintf(stderr,
        "Usage: firestaff_headless --dungeon PATH "
        "[--stream PATH] [--seed N] [--ticks N]\n");
}

int main(int argc, char** argv) {
    const char* dungeonPath = NULL;
    const char* streamPath  = NULL;
    uint32_t seed = 1u;
    int ticksArg = 0;
    int i;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--dungeon") && i + 1 < argc) {
            dungeonPath = argv[++i];
        } else if (!strcmp(argv[i], "--stream") && i + 1 < argc) {
            streamPath = argv[++i];
        } else if (!strcmp(argv[i], "--seed") && i + 1 < argc) {
            seed = (uint32_t)strtoul(argv[++i], NULL, 0);
        } else if (!strcmp(argv[i], "--ticks") && i + 1 < argc) {
            ticksArg = (int)strtol(argv[++i], NULL, 0);
        } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage();
            return 0;
        } else {
            fprintf(stderr, "Unknown arg: %s\n", argv[i]);
            usage();
            return 2;
        }
    }

    if (!dungeonPath) {
        usage();
        return 2;
    }

    {
        struct GameWorld_Compat world;
        struct TickInput_Compat* inputs = NULL;
        int inputCount = 0;
        uint32_t finalHash = 0;
        int ticksRun = 0;

        memset(&world, 0, sizeof(world));
        if (F0882_WORLD_InitFromDungeonDat_Compat(dungeonPath, seed, &world) != 1) {
            fprintf(stderr, "Failed to load dungeon: %s\n", dungeonPath);
            return 3;
        }

        if (streamPath) {
            if (F0894_DRIVER_LoadTickStream_Compat(streamPath, &inputs, &inputCount) != 1) {
                fprintf(stderr, "Failed to load stream: %s\n", streamPath);
                F0883_WORLD_Free_Compat(&world);
                return 4;
            }
        } else {
            int n = ticksArg > 0 ? ticksArg : 10;
            int k;
            inputs = (struct TickInput_Compat*)calloc((size_t)n, sizeof(*inputs));
            if (!inputs) {
                F0883_WORLD_Free_Compat(&world);
                return 5;
            }
            for (k = 0; k < n; k++) { inputs[k].tick = (uint32_t)k; }
            inputCount = n;
        }

        ticksRun = F0895_DRIVER_RunStream_Compat(&world, inputs, inputCount, NULL, &finalHash);
        F0896_DRIVER_WriteSummary_Compat(&world, finalHash, ticksRun, stdout);

        free(inputs);
        F0883_WORLD_Free_Compat(&world);
        return 0;
    }
}
