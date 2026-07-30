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
#include "asset_find_by_hash.h"
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
            "  --scale-mode <n>    Window scaling: 0=1x, 1=2x, 2=3x, 3=4x, 4=fit, 5=stretch\n"
            "  --presentation-mode <v1|v20|v21|v22> Select game presentation without changing saved settings\n"
            "  --script <cmds>     Comma-separated input script: up,down,left,right,enter,action,esc\n"
            "  --data-dir <path>   Asset directory (default: FIRESTAFF_DATA env var)\n"
            "  --save <path>       Resume a validated save for --game\n"
            "  --scan-data         Recursively scan asset directory by hash and exit\n"
            "  --scan-game-data    Alias for --scan-data\n"
            "  --boot-probe        With --game, verify selected-entry boot handoff and exit\n"
            "  --boot-probe-frames <n> Advance n M11 idle frames before probe receipt\n"
            "                       In boot-probe mode, --script input is applied after those frames\n"
            "                       Boot scripts also accept waitN / wait:N frame tokens\n"
            "  --boot-probe-expect-phase <name> Fail unless the boot receipt phase matches\n"
            "  --boot-probe-expect-runtime Fail unless startup is inactive and a level is loaded\n"
            "  --boot-probe-expect-party x,y,dir Fail unless the boot receipt party matches\n"
            "  --boot-probe-expect-champions <n> Fail unless the boot receipt champion count matches\n"
            "  --boot-probe-expect-level-loaded <0|1> Fail unless level-loaded flag matches\n"
            "  --boot-probe-expect-asset-md5 <md5> Fail unless the boot receipt asset hash matches\n"
            "  --boot-probe-expect-map <n> Fail unless the boot receipt map/level index matches\n"
            "  --boot-probe-expect-runtime-tick-min <n> Fail unless runtime tick is at least n\n"
            "  --boot-probe-expect-runtime-tick-max <n> Fail unless runtime tick is at most n\n"
            "  --boot-probe-expect-startup-active <0|1> Fail unless startup-active flag matches\n"
            "  --boot-probe-expect-startup-frame-min <n> Fail unless startup frame is at least n\n"
            "  --boot-probe-expect-startup-frame-max <n> Fail unless startup frame is at most n\n"
            "  --boot-probe-expect-startup-animation <id> Fail unless startup/title animation matches\n"
            "  --boot-probe-expect-startup-animation-active <0|1> Fail unless animation-active flag matches\n"
            "  --boot-probe-expect-title-frame-min <n> Fail unless title frame is at least n\n"
            "  --boot-probe-expect-title-frame-max <n> Fail unless title frame is at most n\n"
            "  --boot-probe-expect-title-frame-boundary <n> Fail unless title frame max matches n\n"
            "  --boot-probe-expect-title-ready <0|1> Fail unless title-ready flag matches\n"
            "  --boot-probe-expect-dm1-hoc-full-graphics Fail unless DM1 HoC full graphics receipt is ready\n"
            "  --boot-probe-expect-dm1-hoc-release-app-capture Fail unless DM1 HoC release/app capture is ready\n"
            "  --fullscreen        Run in fullscreen mode\n"
            "  --no-vsync          Disable vertical sync\n"
            "  --fps               Show FPS counter\n"
            "  --game <id>         Start game directly: dm1, csb, dm2, nexus, theron\n"
            "  --retroachievements Enable RetroAchievements runtime bridge\n"
            "  --ra-user <name>    RetroAchievements username\n"
            "  --ra-token <token>  RetroAchievements API token\n"
            "  --ra-hardcore <0|1> Toggle hardcore mode for RA config\n"
            "  --menu              Show startup menu even when --game is set\n"
            "  --version           Show version and exit\n"
            "  --help, -h          Show this help\n",
            prog);
}

static int parse_party_triplet(const char* text,
                               int* outX,
                               int* outY,
                               int* outDir) {
    int x;
    int y;
    int dir;
    char tail;
    if (!text || sscanf(text, "%d,%d,%d%c", &x, &y, &dir, &tail) != 3) {
        return 0;
    }
    if (outX) *outX = x;
    if (outY) *outY = y;
    if (outDir) *outDir = dir;
    return 1;
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
    asset_scan_clear_missing_extractor_diagnostics();
    if (dataDir && dataDir[0] != '\0') {
        M12_AssetStatusScanOptions scanOptions;
        memset(&scanOptions, 0, sizeof(scanOptions));
        scanOptions.honorRequestedDataDir = 1;
        (void)M12_AssetStatus_ScanWithOptions(&status, dataDir, &scanOptions);
    } else {
        M12_AssetStatus_Scan(&status, dataDir);
    }
    printf("Firestaff game-data scan\n");
    printf("Data dir: %s\n\n", M12_AssetStatus_GetDataDir(&status));
    print_scan_game(&status, "dm1", "Dungeon Master");
    print_scan_game(&status, "csb", "Chaos Strikes Back");
    print_scan_game(&status, "dm2", "Dungeon Master II");
    print_scan_game(&status, "nexus", "DM Nexus");
    print_scan_game(&status, "theron", "Theron's Quest");
    printf("\nNon-essential intro/title files are optional and do not block launch.\n");
    {
        int missing = asset_scan_missing_extractor_count();
        if (missing > 0) {
            int i;
            printf("\nExternal archives skipped (no extractor installed):\n");
            for (i = 0; i < missing; ++i) {
                printf("  %s\n    install one of: %s\n",
                       asset_scan_missing_extractor_path(i),
                       asset_scan_missing_extractor_tools(i));
            }
        }
    }
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

static int parse_presentation_mode(const char* value, int* out_mode) {
    if (!value || !out_mode) return 0;
    if (strcmp(value, "v1") == 0 || strcmp(value, "0") == 0) {
        *out_mode = M12_PRESENTATION_V1_ORIGINAL;
    } else if (strcmp(value, "v20") == 0 || strcmp(value, "1") == 0) {
        *out_mode = M12_PRESENTATION_V20_FILTERED;
    } else if (strcmp(value, "v21") == 0 || strcmp(value, "2") == 0) {
        *out_mode = M12_PRESENTATION_V21_UPSCALED;
    } else if (strcmp(value, "v22") == 0 || strcmp(value, "3") == 0) {
        *out_mode = M12_PRESENTATION_V22_MODERN;
    } else {
        return 0;
    }
    return 1;
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
        if (strcmp(a, "--boot-probe-expect-runtime") == 0) {
            opts.bootProbeExpectRuntime = 1;
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-party") == 0 && i + 1 < argc) {
            if (!parse_party_triplet(argv[++i],
                                     &opts.bootProbeExpectPartyX,
                                     &opts.bootProbeExpectPartyY,
                                     &opts.bootProbeExpectPartyDir)) {
                fprintf(stderr,
                        "firestaff: --boot-probe-expect-party requires x,y,dir\n");
                return 2;
            }
            opts.bootProbeExpectParty = 1;
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-champions") == 0 && i + 1 < argc) {
            opts.bootProbeExpectChampionCount = atoi(argv[++i]);
            opts.bootProbeExpectChampions = 1;
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-level-loaded") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectLevelLoaded = atoi(argv[++i]) ? 1 : 0;
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-asset-md5") == 0 && i + 1 < argc) {
            opts.bootProbeExpectAssetMd5 = argv[++i];
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-map") == 0 && i + 1 < argc) {
            opts.bootProbeExpectMapIndex = atoi(argv[++i]);
            opts.bootProbeExpectMap = 1;
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-runtime-tick-min") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectRuntimeTickMin = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-runtime-tick-max") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectRuntimeTickMax = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-startup-active") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectStartupActive = atoi(argv[++i]) ? 1 : 0;
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-startup-frame-min") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectStartupFrameMin = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-startup-frame-max") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectStartupFrameMax = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-startup-animation") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectStartupAnimation = argv[++i];
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-startup-animation-active") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectStartupAnimationActive =
                atoi(argv[++i]) ? 1 : 0;
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-title-frame-min") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectTitleFrameMin = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-title-frame-max") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectTitleFrameMax = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-title-frame-boundary") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectTitleFrameBoundary = atoi(argv[++i]);
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-title-ready") == 0 &&
            i + 1 < argc) {
            opts.bootProbeExpectTitleReady = atoi(argv[++i]) ? 1 : 0;
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-dm1-hoc-full-graphics") == 0) {
            opts.bootProbeExpectDm1HoCFullGraphics = 1;
            continue;
        }
        if (strcmp(a, "--boot-probe-expect-dm1-hoc-release-app-capture") == 0) {
            opts.bootProbeExpectDm1HoCReleaseAppCapture = 1;
            continue;
        }
        if (strcmp(a, "--game") == 0 && i + 1 < argc) {
            opts.gameId = argv[++i];
            opts.directLaunch = 1;
            continue;
        }
        if (strcmp(a, "--save") == 0 && i + 1 < argc) {
            opts.savePath = argv[++i];
            continue;
        }
        if (strcmp(a, "--retroachievements") == 0) {
            opts.retroAchievementsEnabled = 1;
            continue;
        }
        if (strcmp(a, "--ra-user") == 0 && i + 1 < argc) {
            opts.retroAchievementsUser = argv[++i];
            continue;
        }
        if (strcmp(a, "--ra-token") == 0 && i + 1 < argc) {
            opts.retroAchievementsToken = argv[++i];
            continue;
        }
        if (strcmp(a, "--ra-hardcore") == 0 && i + 1 < argc) {
            opts.retroAchievementsHardcore = atoi(argv[++i]) ? 1 : 0;
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
            opts.scaleModeOverride = 1;
            continue;
        }
        if (strcmp(a, "--presentation-mode") == 0 && i + 1 < argc) {
            if (!parse_presentation_mode(argv[++i],
                                         &opts.presentationModeOverride)) {
                fprintf(stderr,
                        "firestaff: --presentation-mode must be v1, v20, v21, or v22\n");
                return 2;
            }
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
            opts.showFpsOverlay = 1;
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
    if (opts.savePath && !opts.gameId) {
        fprintf(stderr, "firestaff: --save requires --game <id>\n");
        return 2;
    }

    int rc = M11_PhaseA_Run(&opts);
    if (rc != 0) {
        fprintf(stderr, "firestaff: phase-a run failed (rc=%d)\n", rc);
        return 1;
    }
    return 0;
}
