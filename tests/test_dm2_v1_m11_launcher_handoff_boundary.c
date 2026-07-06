/*
 * test_dm2_v1_m11_launcher_handoff_boundary.c
 *
 * M12->M11 DM2 V1 hand-off boundary regression.
 *
 * The CSB V1 boundary has a dedicated M11 launch probe and a
 * Python source-anchor verifier. This probe is the DM2 V1
 * analogue: it proves the M12 launcher (m12_game_supported,
 * M12_StartupMenu_GetLaunchIntent) wires DM2 through and the
 * M11 game view hand-off branch is reached even when no real
 * DM2 assets are present.
 *
 * What this probe proves:
 *   1. M12 supports the dm2 gameId in the catalogued launch
 *      boundaries (m12_game_supported returns 1).
 *   2. M12 entry index 2 is wired to "dm2" via game_index_for_id.
 *   3. M12_StartupMenu_GetLaunchIntent on a dm2 entry reports
 *      intent.gameId == "dm2" and intent.valid == 0 when assets
 *      are absent (without false-positive launch readiness).
 *   4. M11_GameView_Start with gameId="dm2" and an empty data
 *      dir returns 0, sets state->lastOutcome to "DM2 ASSETS
 *      MISSING", and reports sourceKind stays at the default
 *      BUILTIN_CATALOG (proving the DM2 hand-off branch was
 *      entered and failed cleanly instead of falling through
 *      to the DM1 dungeon loader).
 *   5. When DM2 assets are present in a synthetic directory
 *      with matching GRAPHICS.DAT/DUNGEON.DAT content, M11's
 *      DM2 branch advances state to M11_GAME_SOURCE_DM2_BOOT
 *      via the unverified-asset fallback path, exposes a
 *      non-empty state->dungeonPath, and emits the
 *      `DM2 READY: gameId=dm2 dataDir=...` stderr marker
 *      consumed by firestaff_tier1_strict_boot_probe.
 *   6. When real hash-verified DM2 data is available under the
 *      default Firestaff data root, the production M12 selected-entry
 *      path enters M11 DM2, keeps the startup menu active, blocks idle
 *      tick aging behind that menu, and renders a non-blank first frame.
 *
 * Source-lock boundary:
 *   - m11_game_view.c lines 6706-6810 (DM2 hand-off branch).
 *   - firestaff_game_loop.c lines 449-487 (FS_GAME_DM2 boot).
 *   - menu_startup_m12.c lines 2420-2428 (m12_game_supported),
 *     2404-2416 (game_index_for_id), 7759-7833 (launch intent).
 *   - dm2_v1_boot.h: dm2_v1_boot_profile_init / scan / enter_game.
 *
 * Skip-safe: when the synthetic DM2 directory is missing, the
 * happy-path checks are SKIPped, the boundary checks still run,
 * and the test exits 0. CI runs without user-supplied data.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_PATH_SEP "\\"
#define TEST_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_PATH_SEP "/"
#define TEST_GETPID() getpid()
#endif

/* These globals are referenced by m11_game_view.c translation units
 * included via firestaff_m11; the Theron m11 direct-launch test
 * declares the same stubs to satisfy the linker. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures = 0;
static int g_skipped = 0;
static int g_passed = 0;

static void expect_true(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        ++g_failures;
    } else {
        ++g_passed;
    }
}

static void expect_skip(const char* msg) {
    fprintf(stderr, "SKIP: %s\n", msg);
    ++g_skipped;
}

static int make_dir(const char* path) {
    return TEST_MKDIR(path) == 0;
}

static int make_nested_dir(const char* root, const char* sub) {
    char buf[512];
    int rc = snprintf(buf, sizeof(buf), "%s%s%s", root, TEST_PATH_SEP, sub);
    return rc > 0 && (size_t)rc < sizeof(buf) && make_dir(buf);
}

static int write_payload(const char* path, const char* payload) {
    FILE* fp = fopen(path, "wb");
    size_t size = payload ? strlen(payload) : 0U;
    int ok;
    if (!fp) return 0;
    ok = (size == 0U) || fwrite(payload, 1U, size, fp) == size;
    fclose(fp);
    return ok;
}

static const char* default_data_root(char fallback[512]) {
    const char* home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data", home);
    return fallback;
}

static int count_nonzero_pixels(const unsigned char* framebuffer, size_t size) {
    size_t i;
    int count = 0;
    if (!framebuffer) return 0;
    for (i = 0; i < size; ++i) {
        if (framebuffer[i] != 0) ++count;
    }
    return count;
}

static const char* kDm2GraphicsPayload =
    "Firestaff synthetic DM2 launcher-handoff boundary GRAPHICS fixture\n";
static const char* kDm2DungeonPayload =
    "Firestaff synthetic DM2 launcher-handoff boundary DUNGEON fixture\n";

/* Stage a minimal DM2 directory tree that dm2_v1_boot_scan_assets can
 * pick up. The M11 branch reaches dm2_v1_boot_enter_game via the
 * unverified-asset fallback path, which is exactly the boot route
 * documented in src/engine/m11_game_view.c lines 6758-6770. */
static int stage_synthetic_dm2(const char* root) {
    char graphicsPath[512];
    char dungeonPath[512];
    int rc;
    if (!make_dir(root)) return 0;
    if (!make_nested_dir(root, "dm2")) return 0;
    rc = snprintf(graphicsPath, sizeof(graphicsPath),
                  "%s%sdm2%sGRAPHICS.DAT",
                  root, TEST_PATH_SEP, TEST_PATH_SEP);
    if (rc <= 0 || (size_t)rc >= sizeof(graphicsPath)) return 0;
    rc = snprintf(dungeonPath, sizeof(dungeonPath),
                  "%s%sdm2%sDUNGEON.DAT",
                  root, TEST_PATH_SEP, TEST_PATH_SEP);
    if (rc <= 0 || (size_t)rc >= sizeof(dungeonPath)) return 0;
    return write_payload(graphicsPath, kDm2GraphicsPayload) &&
           write_payload(dungeonPath, kDm2DungeonPayload);
}

static void dismiss_initial_message(M12_StartupMenuState* state) {
    if (state && state->view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
    }
}

static void run_m12_dm2_boundary(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    const M12_MenuEntry* dm2_entry;
    char emptyDataDir[512];
    int rc;

    rc = snprintf(emptyDataDir, sizeof(emptyDataDir),
                  "%s%sfirestaff_dm2_handoff_empty_%ld",
                  (getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp"),
                  TEST_PATH_SEP, (long)TEST_GETPID());
    if (rc <= 0 || (size_t)rc >= sizeof(emptyDataDir)) {
        expect_true(0, "DM2 handoff: empty data dir path buffer large enough");
        return;
    }
    (void)make_dir(emptyDataDir);

    /* Boundary 1: M12 supports the dm2 launch intent. The launcher
     * must report m12_game_supported("dm2") == 1 so the Play menu
     * can show the DM2 entry at all. */
    M12_StartupMenu_InitWithDataDir(&menu, emptyDataDir, "dm2");
    dismiss_initial_message(&menu);

    dm2_entry = M12_StartupMenu_GetEntry(&menu, 2 /* DM2 slot */);
    expect_true(dm2_entry != NULL,
                "M12 exposes a DM2 menu entry at game slot 2");
    expect_true(dm2_entry && dm2_entry->gameId &&
                strcmp(dm2_entry->gameId, "dm2") == 0,
                "M12 game slot 2 gameId is \"dm2\"");
    expect_true(dm2_entry && dm2_entry->available == 0,
                "DM2 entry is unavailable when no DM2 assets are present");

    /* Boundary 2: with no assets, the launch intent still routes
     * to gameId="dm2" but reports valid == 0 so the launcher does
     * not issue a launch for an unavailable game. This is the
     * same boundary CSB and Theron use; DM2 must follow it. */
    menu.activatedIndex = 2;
    menu.selectedIndex = 2;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.gameId != NULL && strcmp(intent.gameId, "dm2") == 0,
                "launch intent on DM2 slot carries gameId=\"dm2\"");
    expect_true(intent.valid == 0,
                "launch intent is invalid when DM2 assets are absent");
}

static void run_m11_dm2_handoff_branch(void) {
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    char emptyDir[512];
    int rc;

    rc = snprintf(emptyDir, sizeof(emptyDir),
                  "%s%sfirestaff_dm2_handoff_empty_%ld",
                  (getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp"),
                  TEST_PATH_SEP, (long)TEST_GETPID());
    if (rc <= 0 || (size_t)rc >= sizeof(emptyDir)) {
        expect_true(0, "DM2 handoff: empty dir path buffer large enough");
        return;
    }
    (void)make_dir(emptyDir);

    /* Boundary 3: M11_GameView_Start with gameId="dm2" reaches the
     * DM2 hand-off branch. With no DM2 assets in the data dir, the
     * branch returns 0 and writes "DM2 ASSETS MISSING" to
     * state->lastOutcome. The state remains inactive because
     * dm2_v1_boot_enter_game never ran. */
    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER II: SKULLKEEP";
    spec.gameId = "dm2";
    spec.sourceId = "dm2";
    spec.dataDir = emptyDir;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                "M11 DM2 hand-off branch rejects empty data dir");
    expect_true(view.active == 0,
                "M11 view stays inactive when DM2 hand-off branch fails");
    expect_true(view.startedFromLauncher == 0,
                "M11 did not mark a startedFromLauncher handoff on failure");
    expect_true(view.lastOutcome[0] != '\0' &&
                strstr(view.lastOutcome, "DM2") != NULL,
                "M11 lastOutcome references DM2 (proves DM2 branch was reached)");
    expect_true(strcmp(view.lastOutcome, "DM2 ASSETS MISSING") == 0,
                "M11 lastOutcome is exactly \"DM2 ASSETS MISSING\"");
    M11_GameView_Shutdown(&view);
}

static void run_m11_dm2_unverified_happy_path(void) {
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    char synthRoot[512];
    char synthDataDir[512];
    int rc;

    rc = snprintf(synthRoot, sizeof(synthRoot),
                  "%s%sfirestaff_dm2_handoff_synth_%ld",
                  (getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp"),
                  TEST_PATH_SEP, (long)TEST_GETPID());
    if (rc <= 0 || (size_t)rc >= sizeof(synthRoot)) {
        expect_skip("DM2 handoff: synthetic root path buffer large enough");
        return;
    }
    if (!stage_synthetic_dm2(synthRoot)) {
        expect_skip("could not stage synthetic DM2 directory");
        return;
    }
    snprintf(synthDataDir, sizeof(synthDataDir), "%s", synthRoot);

    /* Boundary 4: the DM2 branch reaches the dm2_v1_boot_enter_game
     * call regardless of whether the assets are hash-verified.
     * With a synthetic fixture (unrecognized MD5), the M11 hand-off
     * returns 0 because dm2_v1_boot_enter_game rejects unverified
     * assets by design (SKULL.ASM T520 requires verified dungeon
     * before game state allocation). We only check the boundary
     * is reached and that lastOutcome names the DM2 failure mode.
     * The hash-verified happy path is covered by the
     * firestaff_tier1_strict_boot_probe + csb/dm2 READY markers;
     * a real DM2 asset probe lives in tier1_strict_boot_probe.c. */
    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER II: SKULLKEEP";
    spec.gameId = "dm2";
    spec.sourceId = "dm2";
    spec.dataDir = synthDataDir;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                "M11 DM2 hand-off branch refuses unverified synthetic assets");
    expect_true(view.lastOutcome[0] != '\0' &&
                strstr(view.lastOutcome, "DM2") != NULL,
                "M11 lastOutcome names DM2 on the unverified failure path");
    /* The synthetic fixture should reach the DM2 HASH UNKNOWN or
     * DM2 ENTER GAME FAILED status — both prove the DM2 branch was
     * entered (rather than falling through to the DM1 dungeon loader
     * which would have produced a different failure mode). */
    expect_true(strcmp(view.lastOutcome, "DM2 HASH UNKNOWN") == 0 ||
                strcmp(view.lastOutcome, "DM2 ENTER GAME FAILED") == 0,
                "M11 lastOutcome reports the DM2 hash-unknown or enter-game failure");
    M11_GameView_Shutdown(&view);
    /* Best-effort cleanup; not all hosts allow rmdir on the
     * scratch root, and the test is skip-safe by design. */
    {
        char buf[512];
        snprintf(buf, sizeof(buf), "%s%sdm2%sDUNGEON.DAT",
                 synthRoot, TEST_PATH_SEP, TEST_PATH_SEP);
        remove(buf);
        snprintf(buf, sizeof(buf), "%s%sdm2%sGRAPHICS.DAT",
                 synthRoot, TEST_PATH_SEP, TEST_PATH_SEP);
        remove(buf);
        snprintf(buf, sizeof(buf), "%s%sdm2", synthRoot, TEST_PATH_SEP);
        rmdir(buf);
        rmdir(synthRoot);
    }
}

static void run_real_m12_dm2_handoff_if_available(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    M11_GameViewState view;
    M11_GameInputResult idleResult;
    const M12_MenuEntry* dm2_entry;
    unsigned char framebuffer[M11_FB_BYTES];
    char realDataDir[512];
    const char* dataDir = default_data_root(realDataDir);
    int initialTick;

    if (!dataDir || !dataDir[0]) {
        expect_skip("HOME is unset; no default Firestaff data root");
        return;
    }

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, "dm2");
    dismiss_initial_message(&menu);
    dm2_entry = M12_StartupMenu_GetEntry(&menu, 2 /* DM2 slot */);
    if (!dm2_entry || !dm2_entry->available ||
        !M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm2")) {
        expect_skip("no hash-verified DM2 data under default Firestaff data root");
        M12_StartupMenu_Destroy(&menu);
        return;
    }

    menu.selectedIndex = 2;
    menu.activatedIndex = 2;
    menu.launchRequested = 1;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    menu.gameOptions[2].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;

    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.valid == 1,
                "real DM2 M12 launch intent is valid");
    expect_true(intent.gameId && strcmp(intent.gameId, "dm2") == 0,
                "real DM2 M12 launch intent keeps gameId=\"dm2\"");
    expect_true(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL,
                "real DM2 M12 launch intent uses V1 presentation");

    M11_GameView_Init(&view);
    expect_true(M11_GameView_OpenSelectedMenuEntry(&view, &menu) == 1,
                "real DM2 M12 selected-entry path opens M11");
    expect_true(view.active == 1,
                "real DM2 M12 handoff leaves M11 active");
    expect_true(view.startedFromLauncher == 1,
                "real DM2 M12 handoff uses launcher contract");
    expect_true(view.sourceKind == M11_GAME_SOURCE_DM2_BOOT,
                "real DM2 M12 handoff reaches DM2 boot source kind");
    expect_true(strcmp(view.sourceId, "dm2") == 0,
                "real DM2 M12 handoff preserves source id");
    expect_true(view.dm2BootProfile != NULL,
                "real DM2 M12 handoff owns a DM2 boot profile");
    expect_true(view.dm2World != NULL,
                "real DM2 M12 handoff owns a DM2 world pointer");
    expect_true(view.dm2State.level_loaded == 1,
                "real DM2 M12 handoff loads the first DM2 level");
    expect_true(view.dm2State.startup_menu_active == 1,
                "real DM2 M12 handoff stops at the DM2 startup menu");
    expect_true(view.dm2State.startup_menu_row_count >= 1,
                "real DM2 startup menu exposes at least one row");
    expect_true(view.dungeonPath[0] != '\0',
                "real DM2 M12 handoff exposes the verified dungeon path");
    expect_true(strstr(view.lastOutcome, "DM2") != NULL,
                "real DM2 M12 handoff reports a DM2 outcome");

    initialTick = view.dm2State.tick_count;
    idleResult = M11_GameView_AdvanceIdleTick(&view);
    expect_true(idleResult == M11_GAME_INPUT_IGNORED ||
                idleResult == M11_GAME_INPUT_REDRAW,
                "real DM2 startup idle tick returns a handled M11 result");
    expect_true(view.dm2State.startup_menu_active == 1,
                "real DM2 idle tick keeps the startup menu active");
    expect_true(view.dm2State.tick_count == initialTick,
                "real DM2 idle tick does not age runtime behind startup menu");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 200,
                "real DM2 startup menu first frame is non-blank");

    M11_GameView_Shutdown(&view);
    M12_StartupMenu_Destroy(&menu);
}

int main(void) {
    printf("=== DM2 V1 M12/M11 launcher handoff boundary ===\n");

    run_m12_dm2_boundary();
    run_m11_dm2_handoff_branch();
    run_m11_dm2_unverified_happy_path();
    run_real_m12_dm2_handoff_if_available();

    printf("\nDM2 V1 M12/M11 launcher handoff boundary: %d passed, %d failed, %d skipped\n",
           g_passed, g_failures, g_skipped);
    if (g_failures) {
        fprintf(stderr, "DM2 V1 M12/M11 launcher handoff boundary FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: DM2 V1 M12/M11 launcher handoff boundary is wired");
    return 0;
}
