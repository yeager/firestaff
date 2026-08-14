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
#include "menu_startup_m12.h"
#include "asset_find_by_hash.h"
#include "firestaff_game_data_fingerprint.h"
#include "firestaff_version.h"
#include "fs_portable_compat.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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
            "  --nexus-mednafen <path>  Start the authentic Nexus Saturn disc in this Mednafen executable (requires --game nexus)\n"
            "  --nexus-disc <path>      Nexus CUE sheet; defaults to Dungeon Master Nexus (English).cue below --data-dir\n"
            "  --nexus-bios <path>      Japanese Saturn BIOS passed to Mednafen as -ss.bios_jp\n"
            "  --theron-authenticated-fallback  Run Theron from verified Track 02 records when the original CD runtime handoff is unavailable (non-parity)\n"
            "  --save <path>       Resume a validated save for --game\n"
            "  --csb-hint-oracle   Open Atari R1 CSB Utility Disk Hint Oracle (requires --data-dir; optional --save MINI.DAT)\n"
            "  --csb-utility-disk  Open the verified FM Towns CSB C06 Utility Disk\n"
            "  --dm2-english-companion <path>  Hash-verified PC-English GRAPHICS.DAT for DM2 FM Towns\n"
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
            "  --verbose, -v       Show detailed information during operations\n"
            "  --fullscreen        Run in fullscreen mode\n"
            "  --no-vsync          Disable vertical sync\n"
            "  --fps               Show FPS counter\n"
            "  --game <id>         Start game directly: dm1, csb, dm2, nexus, theron\n"
            "  --platform <name>   Select source platform: auto, pc, amiga, atari-st, fm-towns, mac, pce, saturn\n"
            "  --fm-towns          Select the verified FM Towns edition (dm1, csb, or dm2)\n"
            "  --csb-fmtowns-ja    Select CSB's verified Japanese FM Towns edition\n"
            "  --amiga             Select the verified Amiga edition (including DM2 Amiga English)\n"
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

static void print_csb_verified_source_media(const M12_AssetStatus* status) {
    const char* dataRoot;
    const char* hashes[FIRESTAFF_FINGERPRINT_COUNT + 1U];
    char hashText[FIRESTAFF_FINGERPRINT_COUNT][33];
    char paths[FIRESTAFF_FINGERPRINT_COUNT][ASSET_PATH_MAX];
    int matched[FIRESTAFF_FINGERPRINT_COUNT];
    size_t count = 0U;
    size_t i;
    int heading_printed = 0;

    if (!status) return;
    dataRoot = M12_AssetStatus_GetDataDir(status);
    if (!dataRoot || dataRoot[0] == '\0') return;

    /* The two required files are reported above.  All remaining CSB entries
     * in the fingerprint registry are optional source media.  Search for
     * their authenticated bytes in one recursive pass so a sidecar nested in
     * 7z -> ADF (for example Amiga SWSH.FTL) is visible to --scan-data just
     * like GRAPHICS.DAT and DUNGEON.DAT.  ReDMCSB SWSHSND.C F0908-F0910,
     * HINTLOAD.C:15-18, ANIM.C:67-72 and SWITCH.C:473 establish the media
     * roles; none of them changes the base launch gate. */
    for (i = 0U; i < FIRESTAFF_FINGERPRINT_COUNT; ++i) {
        const FirestaffGameDataFingerprint* entry = &firestaff_fingerprint_table[i];
        size_t byteIndex;
        if (entry->game != FIRESTAFF_GAME_CSB ||
            entry->file_type == FIRESTAFF_FILE_GRAPHICS_DAT ||
            entry->file_type == FIRESTAFF_FILE_DUNGEON_DAT) {
            continue;
        }
        for (byteIndex = 0U; byteIndex < sizeof(entry->md5); ++byteIndex) {
            static const char hex[] = "0123456789abcdef";
            hashText[count][byteIndex * 2U] = hex[(entry->md5[byteIndex] >> 4) & 15U];
            hashText[count][byteIndex * 2U + 1U] = hex[entry->md5[byteIndex] & 15U];
        }
        hashText[count][32] = '\0';
        hashes[count] = hashText[count];
        ++count;
    }
    hashes[count] = NULL;
    if (count == 0U ||
        asset_find_all_by_md5_list(dataRoot, hashes, paths, matched,
                                   (int)count, 32) < 0) {
        return;
    }
    for (i = 0U; i < count; ++i) {
        FirestaffGameDataClassifyResult classified;
        if (!matched[i]) continue;
        classified = firestaff_game_data_classify_hex(hashes[i]);
        if (!classified.valid || !classified.entry || classified.entry->game != FIRESTAFF_GAME_CSB) continue;
        if (!heading_printed) {
            printf("  Verified CSB source media (does not block start):\n");
            heading_printed = 1;
        }
        printf("    %-26s FOUND  %s\n", classified.entry->description,
               paths[i]);
    }
}

/* Required GRAPHICS.DAT/DUNGEON.DAT hashes establish launchability, but a
 * shared data root can contain several authentic CSB editions.  Report each
 * matched catalogue entry so the Victor FM Towns CD is visible even when a
 * different valid edition supplies the selected runtime cache. */
static void print_csb_verified_editions(const M12_AssetStatus* status) {
    size_t count;
    size_t i;
    int heading_printed = 0;
    if (!status) return;
    count = M12_AssetStatus_GetVersionCount("csb");
    for (i = 0U; i < count; ++i) {
        const M12_AssetVersionStatus* version =
            M12_AssetStatus_GetVersion(status, "csb", i);
        if (!version || !version->matched || !version->versionId ||
            !version->label || version->matchedPath[0] == '\0') {
            continue;
        }
        if (!heading_printed) {
            printf("  Verified CSB editions:\n");
            heading_printed = 1;
        }
        printf("    %-26s FOUND  %s\n", version->label,
               version->matchedPath);
    }
}

static void print_scan_game(const M12_AssetStatus* status,
                            const char* gameId,
                            const char* title,
                            int verbose) {
    size_t count;
    size_t i;
    int ready;
    if (!status || !gameId || !title) {
        return;
    }
    ready = M12_AssetStatus_GameAvailable(status, gameId);
    count = M12_AssetStatus_GetRequiredFileCount(status, gameId);
    printf("%-22s %s\n", title, ready ? "READY" : "MISSING");
    printf("  Launch requirements:\n");
    for (i = 0U; i < count; ++i) {
        const M12_AssetRequiredFileStatus* file =
            M12_AssetStatus_GetRequiredFile(status, gameId, i);
        if (!file) {
            continue;
        }
        printf("  %-28s %s", file->label, file->matched ? "FOUND" : "MISSING");
        if (file->matched) {
            /* The runtime cache is an implementation detail. `--scan-data`
             * must report the hash-verified user-supplied container/member,
             * otherwise a scan of an archive misleadingly appears to have
             * found loose files under Application Support. */
            printf("  %s", file->sourcePath[0] != '\0'
                              ? file->sourcePath : file->matchedPath);
            if (verbose && file->matchedHash[0] != '\0') {
                printf("\n    md5: %s", file->matchedHash);
            }
        }
        printf("\n");
    }
    if (strcmp(gameId, "csb") == 0) {
        const char* blockReason = M12_AssetStatus_GetCSBLaunchBlockReason(status);
        if (!ready && blockReason[0] != '\0') {
            printf("  Launch blocked: %s\n", blockReason);
        }
        print_csb_verified_editions(status);
        print_csb_verified_source_media(status);
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
        /* A non-Theron CUE/BIN can be present beside otherwise valid game
         * data (for example the FM Towns CSB disc).  The broad filesystem
         * classifier intentionally records its layout for later Theron
         * discovery, but --scan-data must not present that unrelated disc as
         * a failed Theron's Quest Track 02. */
        if (media && media->layout != FIRESTAFF_THERON_MEDIA_LAYOUT_UNKNOWN &&
            media->launch_candidate) {
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

static int verbose_scan_progress(const M12_AssetScanProgress* progress,
                                 void* userData) {
    (void)userData;
    if (!progress) return 1;
    if (progress->currentGameId[0] != '\0') {
        printf("  [%s] %s",
               M12_StartupMenu_GameDisplayTitleForLocale(
                   0, progress->currentGameId),
               progress->currentTask);
        if (progress->currentPath[0] != '\0') {
            printf(": %s", progress->currentPath);
        }
        printf("\n");
    }
    return 1;
}

static int run_data_scan(const char* dataDir, int verbose) {
    M12_AssetStatus status;
    asset_scan_clear_missing_extractor_diagnostics();
    if (verbose) {
        M12_AssetStatusScanOptions scanOptions;
        memset(&scanOptions, 0, sizeof(scanOptions));
        scanOptions.progressFn = verbose_scan_progress;
        if (dataDir && dataDir[0] != '\0') {
            scanOptions.honorRequestedDataDir = 1;
        }
        printf("Firestaff game-data scan (verbose)\n");
        printf("Scanning...\n");
        fflush(stdout);
        (void)M12_AssetStatus_ScanWithOptions(&status,
                                              dataDir ? dataDir : "", &scanOptions);
    } else if (dataDir && dataDir[0] != '\0') {
        M12_AssetStatusScanOptions scanOptions;
        memset(&scanOptions, 0, sizeof(scanOptions));
        scanOptions.honorRequestedDataDir = 1;
        /* Archive-backed original media can take a while to inspect.  Emit
         * and flush the same immediate acknowledgement as verbose mode so
         * --scan-data never looks hung before its final availability report. */
        printf("Firestaff game-data scan\nScanning...\n");
        fflush(stdout);
        (void)M12_AssetStatus_ScanWithOptions(&status, dataDir, &scanOptions);
    } else {
        printf("Firestaff game-data scan\nScanning...\n");
        fflush(stdout);
        M12_AssetStatus_Scan(&status, dataDir);
    }
    printf("Data dir: %s\n", M12_AssetStatus_GetDataDir(&status));
    if (verbose) {
        const char* dirs[] = {"dm1", "csb", "dm2", "nexus", "theron"};
        const char* names[] = {
            "Dungeon Master", "Chaos Strikes Back",
            "Dungeon Master II: The Legend of Skullkeep",
            "Dungeon Master Nexus", "Theron's Quest"};
        size_t gi;
        printf("Version: " FIRESTAFF_VERSION_STRING "\n");
        printf("\n");
        for (gi = 0; gi < 5; ++gi) {
            print_scan_game(&status, dirs[gi], names[gi], verbose);
        }
    } else {
        printf("\n");
        print_scan_game(&status, "dm1", "Dungeon Master", 0);
        print_scan_game(&status, "csb", "Chaos Strikes Back", 0);
        print_scan_game(&status, "dm2",
                        "Dungeon Master II: The Legend of Skullkeep", 0);
        print_scan_game(&status, "nexus", "Dungeon Master Nexus", 0);
        print_scan_game(&status, "theron", "Theron's Quest", 0);
    }
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

/* Accept the conventional --option=value spelling as well as the space form.
 * The two typographic dash aliases make a pasted command from macOS Notes,
 * Messages or rich-text documentation recoverable instead of inexplicably
 * reporting an unknown argument.  They are deliberately limited to --game:
 * game selection is the command most commonly copied verbatim from the
 * launcher documentation, and accepting arbitrary Unicode option prefixes
 * would make misspelled switches harder to diagnose. */
static int is_game_option_name(const char* value) {
    return value &&
           (strcmp(value, "--game") == 0 ||
            strcmp(value, "\xE2\x80\x94game") == 0 || /* em dash */
            strcmp(value, "\xE2\x80\x93game") == 0);  /* en dash */
}

static const char* game_option_inline_value(const char* value) {
    static const char* const prefixes[] = {
        "--game=",
        "\xE2\x80\x94game=", /* em dash */
        "\xE2\x80\x93game="  /* en dash */
    };
    size_t i;
    if (!value) return NULL;
    for (i = 0U; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
        size_t length = strlen(prefixes[i]);
        if (strncmp(value, prefixes[i], length) == 0 && value[length] != '\0') {
            return value + length;
        }
    }
    return NULL;
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

static int parse_architecture(const char* value, int* out_architecture) {
    if (!value || !out_architecture) return 0;
    if (strcmp(value, "auto") == 0) *out_architecture = M12_ARCH_AUTO;
    else if (strcmp(value, "pc") == 0) *out_architecture = M12_ARCH_PC;
    else if (strcmp(value, "amiga") == 0) *out_architecture = M12_ARCH_AMIGA;
    else if (strcmp(value, "atari-st") == 0 || strcmp(value, "st") == 0)
        *out_architecture = M12_ARCH_ATARI_ST;
    else if (strcmp(value, "fm-towns") == 0 || strcmp(value, "fmtowns") == 0 ||
             strcmp(value, "fm_towns") == 0)
        *out_architecture = M12_ARCH_FM_TOWNS;
    else if (strcmp(value, "mac") == 0)
        *out_architecture = M12_ARCH_MAC;
    else if (strcmp(value, "pce") == 0 || strcmp(value, "pc-engine") == 0)
        *out_architecture = M12_ARCH_PCE;
    else if (strcmp(value, "saturn") == 0) *out_architecture = M12_ARCH_SATURN;
    else return 0;
    return 1;
}

static int nexus_path_is_readable(const char* path) {
#if defined(_WIN32)
    return path && path[0] != '\0' && _access(path, 4) == 0;
#else
    return path && path[0] != '\0' && access(path, R_OK) == 0;
#endif
}

static int nexus_path_is_executable(const char* path) {
#if defined(_WIN32)
    return path && path[0] != '\0' && _access(path, 0) == 0;
#else
    return path && path[0] != '\0' && access(path, X_OK) == 0;
#endif
}

static int nexus_is_cue_path(const char* path) {
    size_t length;
    if (!path) return 0;
    length = strlen(path);
    return length >= 4U && strcmp(path + length - 4U, ".cue") == 0;
}

static int nexus_copy_path(char* out, size_t out_size, const char* path) {
    int written;
    if (!out || out_size == 0U || !path) return 0;
    written = snprintf(out, out_size, "%s", path);
    return written >= 0 && (size_t)written < out_size;
}

static int nexus_find_default_disc(const char* data_dir,
                                   char* out,
                                   size_t out_size) {
    static const char cue_name[] = "Dungeon Master Nexus (English).cue";
    const char* root = data_dir;
    int written;

    if (!root || root[0] == '\0') root = getenv("FIRESTAFF_DATA");
    if (!root || root[0] == '\0') return 0;
    if (nexus_is_cue_path(root) && nexus_path_is_readable(root)) {
        return nexus_copy_path(out, out_size, root);
    }
    written = snprintf(out, out_size, "%s/%s", root, cue_name);
    if (written >= 0 && (size_t)written < out_size &&
        nexus_path_is_readable(out)) {
        return 1;
    }
    written = snprintf(out, out_size, "%s/nexus/%s", root, cue_name);
    return written >= 0 && (size_t)written < out_size &&
           nexus_path_is_readable(out);
}

/* Nexus is kept fail-closed in the native runtime until a source-owned Saturn
 * title/display consumer is captured.  This explicit route starts the user's
 * unmodified retail CUE in Mednafen instead; it neither decodes nor claims a
 * Firestaff-native title/menu.  No shell is involved, so paths with spaces
 * (including the retail disc name) remain exact arguments. */
static int launch_nexus_mednafen(const char* mednafen,
                                 const char* bios,
                                 const char* requested_disc,
                                 const char* data_dir) {
    char default_disc[PATH_MAX];
    const char* disc = requested_disc;
    char* child_argv[5];
    int child_argc = 0;

    if (!nexus_path_is_executable(mednafen)) {
        fprintf(stderr, "firestaff: --nexus-mednafen must name an executable file\n");
        return 2;
    }
    if (!disc || disc[0] == '\0') {
        if (!nexus_find_default_disc(data_dir, default_disc, sizeof(default_disc))) {
            fprintf(stderr,
                    "firestaff: Nexus CUE not found; pass --nexus-disc or use --data-dir containing nexus/Dungeon Master Nexus (English).cue\n");
            return 2;
        }
        disc = default_disc;
    }
    if (!nexus_is_cue_path(disc) || !nexus_path_is_readable(disc)) {
        fprintf(stderr, "firestaff: --nexus-disc must name a readable .cue file\n");
        return 2;
    }
    if (bios && bios[0] != '\0' && !nexus_path_is_readable(bios)) {
        fprintf(stderr, "firestaff: --nexus-bios must name a readable BIOS file\n");
        return 2;
    }

    child_argv[child_argc++] = (char*)mednafen;
    if (bios && bios[0] != '\0') {
        child_argv[child_argc++] = "-ss.bios_jp";
        child_argv[child_argc++] = (char*)bios;
    }
    child_argv[child_argc++] = (char*)disc;
    child_argv[child_argc] = NULL;

    printf("FIRESTAFF NEXUS EXTERNAL LAUNCH: emulator=%s disc=%s bios=%s\n",
           mednafen, disc, (bios && bios[0] != '\0') ? bios : "configured-by-mednafen");
    fflush(stdout);

#if defined(_WIN32)
    (void)child_argv;
    fprintf(stderr,
            "firestaff: --nexus-mednafen is currently implemented for macOS and Linux only\n");
    return 2;
#else
    {
        pid_t child = fork();
        int status;
        if (child < 0) {
            perror("firestaff: could not start Mednafen");
            return 1;
        }
        if (child == 0) {
            execv(mednafen, child_argv);
            perror("firestaff: could not execute Mednafen");
            _exit(127);
        }
        if (waitpid(child, &status, 0) < 0) {
            perror("firestaff: could not wait for Mednafen");
            return 1;
        }
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        fprintf(stderr, "firestaff: Mednafen ended unexpectedly\n");
        return 1;
    }
#endif
}

int main(int argc, char** argv) {
    M11_PhaseA_Options opts;
    int scanData = 0;
    int verbose = 0;
    int theronAuthenticatedFallback = 0;
    const char* nexusMednafen = NULL;
    const char* nexusDisc = NULL;
    const char* nexusBios = NULL;
    M11_PhaseA_SetDefaultOptions(&opts);

    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];
        const char* inlineGameId;
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
            /* An explicit size describes a windowed presentation.  The
             * launcher may still be maximized by the saved default, but a
             * requested small/large Mac window must not be silently lost. */
            opts.windowModeOverride = M11_WINDOW_MODE_WINDOWED;
            continue;
        }
        if (strcmp(a, "--height") == 0 && i + 1 < argc) {
            opts.windowHeight = atoi(argv[++i]);
            opts.windowModeOverride = M11_WINDOW_MODE_WINDOWED;
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
        if (strcmp(a, "--nexus-mednafen") == 0 && i + 1 < argc) {
            nexusMednafen = argv[++i];
            continue;
        }
        if (strcmp(a, "--nexus-disc") == 0 && i + 1 < argc) {
            nexusDisc = argv[++i];
            continue;
        }
        if (strcmp(a, "--nexus-bios") == 0 && i + 1 < argc) {
            nexusBios = argv[++i];
            continue;
        }
        if (strcmp(a, "--csb-hint-oracle") == 0) {
            opts.csbHintOracle = 1;
            opts.directLaunch = 1;
            continue;
        }
        if (strcmp(a, "--csb-utility-disk") == 0) {
            opts.csbFmtownsUtilityDisk = 1;
            opts.gameId = "csb";
            opts.architectureOverride = M12_ARCH_FM_TOWNS;
            opts.directLaunch = 1;
            continue;
        }
        if (strcmp(a, "--theron-authenticated-fallback") == 0) {
            theronAuthenticatedFallback = 1;
            continue;
        }
        if (strcmp(a, "--scan-data") == 0 ||
            strcmp(a, "--scan-game-data") == 0) {
            scanData = 1;
            continue;
        }
        if (strcmp(a, "--verbose") == 0 || strcmp(a, "-v") == 0) {
            verbose = 1;
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
        if (is_game_option_name(a) && i + 1 < argc) {
            opts.gameId = argv[++i];
            opts.directLaunch = 1;
            continue;
        }
        inlineGameId = game_option_inline_value(a);
        if (inlineGameId) {
            opts.gameId = inlineGameId;
            opts.directLaunch = 1;
            continue;
        }
        if (strcmp(a, "--platform") == 0 && i + 1 < argc) {
            if (!parse_architecture(argv[++i], &opts.architectureOverride)) {
                fprintf(stderr,
                        "firestaff: --platform must be auto, pc, amiga, atari-st, fm-towns, mac, pce, or saturn\n");
                return 2;
            }
            continue;
        }
        if (strcmp(a, "--fm-towns") == 0) {
            opts.architectureOverride = M12_ARCH_FM_TOWNS;
            continue;
        }
        if (strcmp(a, "--csb-fmtowns-ja") == 0) {
            opts.csbFmtownsJapanese = 1;
            opts.architectureOverride = M12_ARCH_FM_TOWNS;
            continue;
        }
        if (strcmp(a, "--amiga") == 0) {
            opts.architectureOverride = M12_ARCH_AMIGA;
            continue;
        }
        if (strcmp(a, "--save") == 0 && i + 1 < argc) {
            opts.savePath = argv[++i];
            continue;
        }
        if (strcmp(a, "--dm2-english-companion") == 0 && i + 1 < argc) {
            opts.dm2EnglishCompanionPath = argv[++i];
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
            opts.windowModeOverride = M11_WINDOW_MODE_FULLSCREEN;
            continue;
        }
        if (strcmp(a, "--windowed") == 0) {
            opts.windowModeOverride = M11_WINDOW_MODE_WINDOWED;
            continue;
        }
        if (strcmp(a, "--no-vsync") == 0) {
            opts.vsyncOverride = 0;
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

    opts.verbose = verbose;

    if (theronAuthenticatedFallback) {
#if defined(_WIN32)
        _putenv_s("FIRESTAFF_THERON_ALLOW_AUTHENTICATED_FALLBACK", "1");
#else
        setenv("FIRESTAFF_THERON_ALLOW_AUTHENTICATED_FALLBACK", "1", 1);
#endif
    }

    if (scanData) {
        return run_data_scan(opts.dataDir, verbose);
    }

    if (nexusMednafen || nexusDisc || nexusBios) {
        if (!nexusMednafen) {
            fprintf(stderr,
                    "firestaff: --nexus-disc and --nexus-bios require --nexus-mednafen\n");
            return 2;
        }
        if (!opts.gameId || strcmp(opts.gameId, "nexus") != 0) {
            fprintf(stderr,
                    "firestaff: --nexus-mednafen requires --game nexus\n");
            return 2;
        }
        if (opts.bootProbe) {
            fprintf(stderr,
                    "firestaff: --nexus-mednafen cannot be combined with --boot-probe\n");
            return 2;
        }
        return launch_nexus_mednafen(nexusMednafen, nexusBios, nexusDisc,
                                     opts.dataDir);
    }

    if (opts.bootProbe && !opts.gameId) {
        fprintf(stderr, "firestaff: --boot-probe requires --game <id>\n");
        return 2;
    }
    if (opts.csbHintOracle && (!opts.dataDir || !opts.dataDir[0])) {
        fprintf(stderr,
                "firestaff: --csb-hint-oracle requires --data-dir\n");
        return 2;
    }
    if (opts.csbFmtownsUtilityDisk &&
        (!opts.gameId || strcmp(opts.gameId, "csb") != 0)) {
        fprintf(stderr, "firestaff: --csb-utility-disk requires CSB FM Towns media\n");
        return 2;
    }
    if (opts.savePath && !opts.csbHintOracle && !opts.gameId) {
        fprintf(stderr, "firestaff: --save requires --game <id>\n");
        return 2;
    }
    if (opts.dm2EnglishCompanionPath &&
        (!opts.gameId || strcmp(opts.gameId, "dm2") != 0)) {
        fprintf(stderr,
                "firestaff: --dm2-english-companion requires --game dm2\n");
        return 2;
    }

    int rc = M11_PhaseA_Run(&opts);
    if (rc != 0) {
        fprintf(stderr, "firestaff: phase-a run failed (rc=%d)\n", rc);
        return 1;
    }
    return 0;
}
