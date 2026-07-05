/*
 * test_m11_nexus_startup_gate.c -- M11 Nexus startup ownership gate.
 *
 * Scope: startup handoff only. Empty or partial Nexus data must not leave
 * M11 in an active half-started state; a real extracted/ISO data directory,
 * when staged locally, must reach M11_GAME_SOURCE_NEXUS_DGN.
 *
 * Source: src/nexus/nexus_v1_launcher.c owns init/load-level sequencing;
 * src/engine/m11_game_view.c M11_GameView_StartNexus owns the M11 handoff.
 */

#include "m11_game_view.h"
#include "nexus_v1_launcher.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_RMDIR(path) _rmdir(path)
#define TEST_PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_RMDIR(path) rmdir(path)
#define TEST_PATH_SEP "/"
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures;

static void expect_true(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int make_temp_root(char root[512]) {
#ifdef _WIN32
    snprintf(root, 512, ".\\firestaff_nexus_m11_startup_%lu",
             (unsigned long)rand());
    return TEST_MKDIR(root) == 0;
#else
    char tmpl[] = "/tmp/firestaff_nexus_m11_startup_XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) {
        return 0;
    }
    snprintf(root, 512, "%s", made);
    return 1;
#endif
}

static int write_file(const char* path, const char* bytes) {
    FILE* f = fopen(path, "wb");
    size_t n = bytes ? strlen(bytes) : 0u;
    int ok;
    if (!f) {
        return 0;
    }
    ok = n == 0u || fwrite(bytes, 1u, n, f) == n;
    fclose(f);
    return ok;
}

static void fill_nexus_spec(M11_GameLaunchSpec* spec, const char* data_dir) {
    memset(spec, 0, sizeof(*spec));
    spec->title = "DUNGEON MASTER NEXUS";
    spec->gameId = "nexus";
    spec->sourceId = "nexus";
    spec->dataDir = data_dir;
    spec->rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec->presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
}

static void expect_failed_start_is_inactive(const char* data_dir,
                                            const char* expected_status) {
    M11_GameViewState view;
    M11_GameLaunchSpec spec;

    fill_nexus_spec(&spec, data_dir);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                "Nexus M11 startup rejects invalid data");
    expect_true(view.active == 0,
                "failed Nexus startup leaves M11 inactive");
    expect_true(view.startedFromLauncher == 0,
                "failed Nexus startup does not claim launcher start");
    expect_true(view.sourceKind == M11_GAME_SOURCE_BUILTIN_CATALOG,
                "failed Nexus startup does not retain Nexus sourceKind");
    expect_true(view.nexusEngine == NULL,
                "failed Nexus startup does not expose a Nexus engine");
    expect_true(strstr(view.lastOutcome, expected_status) != NULL,
                "failed Nexus startup reports expected blocker");
    M11_GameView_Shutdown(&view);
    nexus_v1_launcher_shutdown();
}

static const char* nexus_data_dir(char fallback[512]) {
    const char* data_dir = getenv("FIRESTAFF_NEXUS_V1_DATA_DIR");
    const char* home;
    if (data_dir && data_dir[0]) {
        return data_dir;
    }
    home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data/nexus", home);
    return fallback;
}

int main(void) {
    char empty_root[512];
    char partial_root[512];
    char partial_dm_bin[512];
    char real_fallback[512];
    const char* real_dir;

    if (make_temp_root(empty_root)) {
        expect_failed_start_is_inactive(empty_root, "NEXUS DATA ERROR");
        (void)TEST_RMDIR(empty_root);
    } else {
        expect_true(0, "created empty Nexus temp root");
    }

    if (make_temp_root(partial_root)) {
        snprintf(partial_dm_bin, sizeof(partial_dm_bin), "%s%sDM.BIN",
                 partial_root, TEST_PATH_SEP);
        expect_true(write_file(partial_dm_bin, "not-a-real-nexus-dm-bin"),
                    "seeded partial Nexus DM.BIN");
        expect_failed_start_is_inactive(partial_root, "NEXUS LEVEL ERROR");
        remove(partial_dm_bin);
        (void)TEST_RMDIR(partial_root);
    } else {
        expect_true(0, "created partial Nexus temp root");
    }

    real_dir = nexus_data_dir(real_fallback);
    if (real_dir && real_dir[0]) {
        M11_GameViewState view;
        M11_GameLaunchSpec spec;
        fill_nexus_spec(&spec, real_dir);
        M11_GameView_Init(&view);
        if (M11_GameView_Start(&view, &spec)) {
            expect_true(view.active == 1,
                        "real Nexus startup leaves M11 active");
            expect_true(view.sourceKind == M11_GAME_SOURCE_NEXUS_DGN,
                        "real Nexus startup claims Nexus sourceKind");
            expect_true(view.nexusEngine != NULL,
                        "real Nexus startup exposes engine");
            expect_true(view.nexusState.level_loaded == 1,
                        "real Nexus startup loads level zero");
            expect_true(strstr(view.dungeonPath, "LEV00.DGN") != NULL,
                        "real Nexus startup exposes level path");
        } else {
            printf("skip: no launchable Nexus V1 data at %s\n", real_dir);
        }
        M11_GameView_Shutdown(&view);
        nexus_v1_launcher_shutdown();
    }

    if (g_failures) {
        fprintf(stderr, "M11 Nexus startup gate FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: M11 Nexus startup gate");
    return 0;
}
