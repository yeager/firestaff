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

#include "asset_status_m12.h"
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
            "  --scan-data         Recursively scan asset directory by hash and exit\n"
            "  --scan-game-data    Alias for --scan-data\n"
            "  --boot-probe        With --game, verify selected-entry boot handoff and exit\n"
            "  --boot-probe-frames <n> Advance n M11 idle frames before probe receipt\n"
            "                       In boot-probe mode, --script input is applied after those frames\n"
            "                       Boot scripts also accept waitN / wait:N frame tokens\n"
            "  --boot-probe-expect-phase <name> Fail unless the boot receipt phase matches\n"
            "  --fullscreen        Run in fullscreen mode\n"
            "  --no-vsync          Disable vertical sync\n"
            "  --fps               Show FPS counter\n"
            "  --game <id>         Start game directly: dm1, csb, dm2, nexus, theron\n"
            "  --menu              Show startup menu even when --game is set\n"
            "  --version           Show version and exit\n"
            "  --help, -h          Show this help\n",
            prog);
}

static void print_scan_game(const M12_AssetStatus* status,
                            const char* gameId,
                            const char* title) {
    size_t count;
    size_t i;
    int ready;
    if (!status || !gameId || !title) {
        return;
    }
    ready = M12_AssetStatus_GameAvailable(status, gameId);
    count = M12_AssetStatus_GetRequiredFileCount(status, gameId);
    printf("%-22s %s\n", title, ready ? "READY" : "MISSING");
    for (i = 0U; i < count; ++i) {
        const M12_AssetRequiredFileStatus* file =
            M12_AssetStatus_GetRequiredFile(status, gameId, i);
        if (!file) {
            continue;
        }
        printf("  %-28s %s", file->label, file->matched ? "FOUND" : "MISSING");
        if (file->matched) {
            printf("  %s", file->matchedPath);
        }
        printf("\n");
    }
    if (strcmp(gameId, "nexus") == 0) {
        const M12_NexusBpkTrailerMetadata* bpk =
            M12_AssetStatus_GetNexusBpkTrailerMetadata(status);
        if (bpk && bpk->found && bpk->parsed && bpk->trailerFound) {
            printf("  %-28s FOUND  entries=%u prs3=%u trailer[%u]=0x%08x,0x%08x  %s\n",
                   "MENU.BPK trailer",
                   bpk->entryCount,
                   bpk->prs3PayloadCount,
                   bpk->trailerIndex,
                   bpk->trailerFirstOffset,
                   bpk->trailerSecondOffset,
                   bpk->matchedPath);
        }
    }
    if (strcmp(gameId, "theron") == 0) {
        const FirestaffTheronMediaStatus* media =
            M12_AssetStatus_GetTheronMediaStatus(status);
        if (media && media->layout != FIRESTAFF_THERON_MEDIA_LAYOUT_UNKNOWN) {
            printf("  %-28s %s", "Media layout",
                   FirestaffTheronMedia_LayoutLabel(media->layout));
            printf("  %s",
                   media->launch_candidate ? "TRACK02-CANDIDATE" : "NO-DATA-TRACK");
            if (media->has_iso9660_pvd) {
                printf("  ISO9660");
            }
            if (media->candidate_path[0] != '\0') {
                printf("  %s", media->candidate_path);
            }
            printf("\n");
        }
    }
}

static int run_data_scan(const char* dataDir) {
    M12_AssetStatus status;
    M12_AssetStatus_Scan(&status, dataDir);
    printf("Firestaff game-data scan\n");
    printf("Data dir: %s\n\n", M12_AssetStatus_GetDataDir(&status));
    print_scan_game(&status, "dm1", "Dungeon Master");
    print_scan_game(&status, "csb", "Chaos Strikes Back");
    print_scan_game(&status, "dm2", "Dungeon Master II");
    print_scan_game(&status, "nexus", "DM Nexus");
    print_scan_game(&status, "theron", "Theron's Quest");
    printf("\nNon-essential intro/title files are optional and do not block launch.\n");
    return 0;
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
    int scanData = 0;
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
        if (strcmp(a, "--scan-data") == 0 ||
            strcmp(a, "--scan-game-data") == 0) {
            scanData = 1;
            continue;
        }
        if (strcmp(a, "--boot-probe") == 0) {
            opts.bootProbe = 1;
            opts.directLaunch = 1;
            continue;
        }
        if (strcmp(a, "--boot-probe-frames") == 0 && i + 1 < argc) {
            opts.bootProbeFrames = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-phase") == 0 && i + 1 < argc) {
            opts.bootProbeExpectPhase = argv[++i];
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

    if (scanData) {
        return run_data_scan(opts.dataDir);
    }

    if (opts.bootProbe && !opts.gameId) {
        fprintf(stderr, "firestaff: --boot-probe requires --game <id>\n");
        return 2;
    }

    int rc = M11_PhaseA_Run(&opts);
    if (rc != 0) {
        fprintf(stderr, "firestaff: phase-a run failed (rc=%d)\n", rc);
        return 1;
    }
    return 0;
}
