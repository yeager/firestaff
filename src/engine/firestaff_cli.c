
#include "firestaff_cli.h"
#include "firestaff_l10n.h"
#include "firestaff_save.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void fs_cli_print_version(void) {
    printf("Firestaff v1.4.0 — Dungeon Master Collection\n");
}

void fs_cli_print_help(const char *prog) {
    printf("Usage: %s [OPTIONS] [GAME]\n", prog ? prog : "firestaff");
    printf("\n");
    printf("Games:\n");
    printf("  dm1              Dungeon Master (default)\n");
    printf("  csb              Chaos Strikes Back\n");
    printf("  dm2              Dungeon Master II: Skullkeep\n");
    printf("  nexus            Dungeon Master Nexus\n");
    printf("  theron           Theron's Quest (PC Engine/TurboGrafx-16)\n");
    printf("\n");
    printf("Version:\n");
    printf("  --v1             Original faithful (320x200)\n");
    printf("  --v2, --v21      Upscaled (EPX 2x)\n");
    printf("  --v22            Enhanced (smooth movement, particles, etc)\n");
    printf("\n");
    printf("Display:\n");
    printf("  --fullscreen     Start in fullscreen\n");
    printf("  --windowed       Start in window (default)\n");
    printf("  --scale N        Scale factor: 1, 2, 4 (default: 2)\n");
    printf("  --width W        Window width\n");
    printf("  --height H       Window height\n");
    printf("  --vsync          Enable VSync (default)\n");
    printf("  --no-vsync       Disable VSync\n");
    printf("\n");
    printf("Language:\n");
    printf("  --lang CODE      Set language. Codes:\n");
    printf("    en sv de fr es it pt nl pl cs ru ja ko zh da no fi hu tr\n");
    printf("  --lang auto      Detect from system (default)\n");
    printf("\n");
    printf("Data:\n");
    printf("  --data DIR       Path to game data directory\n");
    printf("  --saves DIR      Path to save directory\n");
    printf("  --config FILE    Path to settings INI file\n");
    printf("\n");
    printf("Save/Load:\n");
    printf("  --load N         Load save slot N (0-9)\n");
    printf("  --list-saves     List available save files and exit\n");
    printf("\n");
    printf("Other:\n");
    printf("  --headless       Run without window (for testing)\n");
    printf("  --menu           Force startup menu\n");
    printf("  --no-menu        Skip startup menu (default when game specified)\n");
    printf("  --help, -h       Show this help\n");
    printf("  --validate       Check game data files\n");
    printf("  --version, -V    Show version\n");
    printf("\n");
    printf("Developer: Daniel Nylander\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s dm1 --v21 --fullscreen --lang sv\n", prog);
    printf("  %s csb --v1 --scale 4\n", prog);
    printf("  %s csb --v1 --scale 4\n", prog);
    printf("  %s dm2 --v22 --load 3\n", prog);
    printf("  %s --list-saves\n", prog);
}

static int match_game(const char *arg) {
    if (!arg) return -1;
    if (strcmp(arg, "dm1") == 0 || strcmp(arg, "dungeon-master") == 0) return FS_GAME_DM1;
    if (strcmp(arg, "csb") == 0 || strcmp(arg, "chaos") == 0) return FS_GAME_CSB;
    if (strcmp(arg, "dm2") == 0 || strcmp(arg, "skullkeep") == 0) return FS_GAME_DM2;
    if (strcmp(arg, "nexus") == 0) return FS_GAME_NEXUS;
    if (strcmp(arg, "theron") == 0 || strcmp(arg, "therons-quest") == 0) return 5; /* FS_GAME_THERON - placeholder */
    return -1;
}

static int match_lang(const char *arg) {
    if (!arg) return -1;
    if (strcmp(arg, "en") == 0) return 0;
    if (strcmp(arg, "sv") == 0) return 1;
    if (strcmp(arg, "de") == 0) return 2;
    if (strcmp(arg, "fr") == 0) return 3;
    if (strcmp(arg, "es") == 0) return 4;
    if (strcmp(arg, "it") == 0) return 5;
    if (strcmp(arg, "pt") == 0) return 6;
    if (strcmp(arg, "nl") == 0) return 7;
    if (strcmp(arg, "pl") == 0) return 8;
    if (strcmp(arg, "cs") == 0) return 9;
    if (strcmp(arg, "ru") == 0) return 10;
    if (strcmp(arg, "ja") == 0) return 11;
    if (strcmp(arg, "ko") == 0) return 12;
    if (strcmp(arg, "zh") == 0) return 13;
    if (strcmp(arg, "da") == 0) return 14;
    if (strcmp(arg, "no") == 0) return 15;
    if (strcmp(arg, "fi") == 0) return 16;
    if (strcmp(arg, "hu") == 0) return 17;
    if (strcmp(arg, "tr") == 0) return 18;
    if (strcmp(arg, "auto") == 0) return -1;
    return -1;
}

int fs_cli_parse(FS_CLIOptions *opts, int argc, char **argv) {
    int i;
    if (!opts) return -1;
    memset(opts, 0, sizeof(*opts));

    /* Defaults */
    opts->game = FS_GAME_DM1;
    opts->version = FS_VERSION_V1;
    opts->scale = 1;
    opts->vsync = 1;
    opts->lang = -1; /* auto-detect */
    opts->load_slot = -1;
    opts->skip_menu = -1; /* auto: skip if game specified on CLI */

    for (i = 1; i < argc; i++) {
        const char *arg = argv[i];

        /* Game selection (positional) */
        int g = match_game(arg);
        if (g >= 0) { opts->game = (FS_GameId)g; continue; }

        /* Version */
        if (strcmp(arg, "--v1") == 0) { opts->version = FS_VERSION_V1; opts->scale = 1; continue; }
        if (strcmp(arg, "--v2") == 0 || strcmp(arg, "--v21") == 0)
            { opts->version = FS_VERSION_V1;
    opts->scale = 1; opts->scale = 2; continue; }
        if (strcmp(arg, "--v22") == 0) { opts->version = FS_VERSION_V22; opts->scale = 4; continue; }

        /* Display */
        if (strcmp(arg, "--fullscreen") == 0) { opts->fullscreen = 1; continue; }
        if (strcmp(arg, "--windowed") == 0) { opts->fullscreen = 0; continue; }
        if (strcmp(arg, "--vsync") == 0) { opts->vsync = 1; continue; }
        if (strcmp(arg, "--no-vsync") == 0) { opts->vsync = 0; continue; }
        if (strcmp(arg, "--headless") == 0) { opts->headless = 1; continue; }

        if (strcmp(arg, "--scale") == 0 && i + 1 < argc)
            { opts->scale = atoi(argv[++i]); continue; }
        if (strcmp(arg, "--width") == 0 && i + 1 < argc)
            { opts->width = atoi(argv[++i]); continue; }
        if (strcmp(arg, "--height") == 0 && i + 1 < argc)
            { opts->height = atoi(argv[++i]); continue; }

        /* Language */
        if (strcmp(arg, "--lang") == 0 && i + 1 < argc)
            { opts->lang = match_lang(argv[++i]); continue; }

        /* Data paths */
        if (strcmp(arg, "--data") == 0 && i + 1 < argc)
            { opts->data_dir = argv[++i]; continue; }
        if (strcmp(arg, "--saves") == 0 && i + 1 < argc)
            { opts->save_dir = argv[++i]; continue; }
        if (strcmp(arg, "--config") == 0 && i + 1 < argc)
            { opts->config_path = argv[++i]; continue; }

        /* Save/Load */
        if (strcmp(arg, "--load") == 0 && i + 1 < argc)
            { opts->load_slot = atoi(argv[++i]); continue; }
        if (strcmp(arg, "--list-saves") == 0) { opts->list_saves = 1; continue; }

        /* Help/Version */
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)
            { opts->show_help = 1; continue; }
        if (strcmp(arg, "--validate") == 0) { opts->show_help = 2; continue; }
        if (strcmp(arg, "--version") == 0 || strcmp(arg, "-V") == 0)
            { opts->show_version = 1; continue; }

        if (strcmp(arg, "--menu") == 0) { opts->skip_menu = 0; continue; }
        if (strcmp(arg, "--no-menu") == 0) { opts->skip_menu = 1; continue; }

        fprintf(stderr, "Unknown option: %s\n", arg);
        return -1;
    }

    /* Auto-detect skip_menu: if a game was explicitly specified on the
     * command line, skip the menu and start directly.  The menu is only
     * shown when no game is given or --menu is explicit. */
    if (opts->skip_menu < 0) {
        /* Check if any positional arg matched a game name */
        int game_explicit = 0;
        for (i = 1; i < argc; i++) {
            if (match_game(argv[i]) >= 0) { game_explicit = 1; break; }
        }
        opts->skip_menu = game_explicit;
    }

    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Detailed startup error formatting
 * ══════════════════════════════════════════════════════════════════════ */

static void fs_cli_print_startup_error(const FS_StartupError *err) {
    if (!err || err->code == 0) return;
    fprintf(stderr, "\n╔══════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║  FIRESTAFF STARTUP ERROR                         ║\n");
    fprintf(stderr, "╚══════════════════════════════════════════════════╝\n\n");
    fprintf(stderr, "  Error:      %s\n", err->message);
    if (err->detail[0])     fprintf(stderr, "  Detail:     %s\n", err->detail);
    if (err->suggestion[0]) fprintf(stderr, "  Suggestion: %s\n", err->suggestion);
    fprintf(stderr, "  Error code: %d\n\n", err->code);
}

int fs_cli_run(const FS_CLIOptions *opts) {
    FS_GameConfig config;
    FS_GameState state;
    const char *game_names[] = {"Dungeon Master", "Chaos Strikes Back",
                                "Dungeon Master II", "DM Nexus"};
    const char *ver_names[] = {"V1 Original", "V2.1 Upscaled", "V2.2 Enhanced"};

    if (!opts) return 1;

    if (opts->show_version) { fs_cli_print_version(); return 0; }
    if (opts->show_help && opts->data_dir) {
        /* --validate mode */
    }
    if (opts->show_help) { fs_cli_print_help("firestaff"); return 0; }

    /* Set language */
    if (opts->lang >= 0) {
        fs_l10n_set_language((FS_Language)opts->lang);
    } else {
        fs_l10n_init_from_system();
    }

    /* List saves */
    if (opts->list_saves) {
        printf("Save slots for %s:\n", game_names[opts->game]);
        for (int i = 0; i < 10; i++) {
            if (fs_save_exists(opts->game, i)) {
                char path[256];
                fs_save_slot_path(opts->game, i, path, sizeof(path));
                printf("  Slot %d: %s\n", i, path);
            }
        }
        return 0;
    }

    if (!opts->skip_menu) {
        printf("Firestaff — %s %s\n", game_names[opts->game], ver_names[opts->version]);
        printf("Language: %s\n", fs_l10n_language_name(fs_l10n_get_language()));
        printf("Scale: %dx\n", opts->scale);
        if (opts->fullscreen) printf("Fullscreen\n");
        if (opts->load_slot >= 0) printf("Loading save slot %d\n", opts->load_slot);
        printf("\n");
    }

    /* Build game config */
    memset(&config, 0, sizeof(config));
    config.game = opts->game;
    config.version = opts->version;
    config.window_width = opts->width > 0 ? opts->width : 320 * opts->scale;
    config.window_height = opts->height > 0 ? opts->height : 200 * opts->scale;
    config.fullscreen = opts->fullscreen;
    config.vsync = opts->vsync;
    config.data_dir = opts->data_dir;
    config.save_dir = opts->save_dir;

    /* Init + run */
    config.skip_menu = opts->skip_menu;

    if (fs_game_init(&state, &config) < 0) {
        fs_cli_print_startup_error(&state.last_error);
        return 1;
    }
    if (fs_game_load_assets(&state) < 0) {
        fs_cli_print_startup_error(&state.last_error);
        return 1;
    }
    if (opts->load_slot >= 0) {
        if (fs_load_game(&state, opts->load_slot) < 0) {
            fs_cli_print_startup_error(&state.last_error);
            fprintf(stderr, "Failed to load save slot %d — starting new game\n", opts->load_slot);
        }
    }

    fs_game_run(&state);
    fs_game_shutdown(&state);
    return 0;
}
