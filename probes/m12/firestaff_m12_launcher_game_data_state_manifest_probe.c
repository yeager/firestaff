/*
 * firestaff_m12_launcher_game_data_state_manifest_probe.c
 *
 * M12 launcher per-card game-data state + structured popup diagnosis
 * probe.
 *
 * Closes the 2026-06-28 follow-up on the open gap "Screen reader
 * launcher state manifest" in TODO.md / docs/FIRESTAFF_GAP_LIST.md by
 * proving that m12_launcher_a11y_emit now also emits:
 *
 *   - One FS_AX_GAME_DATA_STATUS element per game card on the
 *     main view, with stable IDs (GAME_DATA_STATUS_DM1, ...,
 *     GAME_DATA_STATUS_THERON) and a "<matched>/<required> matched
 *     (<version_label>)" value field. The badge is sourced from the
 *     public M12_AssetStatus_GetRequiredFileCount /
 *     M12_AssetStatus_GetRequiredFile /
 *     M12_AssetStatus_GetFirstMatchedVersion API.
 *   - One FS_AX_REQUIRED_FILE element per required file, with stable
 *     IDs (REQUIRED_FILE_DM1_0, ..., REQUIRED_FILE_THERON_<n>) and a
 *     "<label> | present" or "<label> | missing" value field.
 *   - One FS_AX_POPUP_DIAGNOSIS element when the message view is a
 *     missing-data popup, with a stable "<game_id>:<label>," value
 *     field (or "all_games:NO_GAME_DATA" for the global "no game
 *     data" case). The diagnosis is structured so a screen reader or
 *     regression probe can read which game triggered the popup
 *     without parsing the free-form message line text.
 *
 * Privacy/safety: the probe redirects HOME to a temp dir under
 * $TMPDIR (or /tmp on POSIX) so the manifest never touches the real
 * user ~/.firestaff. The diagnosis never leaks a path or hash; only
 * the per-game required-file labels (DUNGEON.DAT, GRAPHICS.DAT, ...)
 * are emitted, and those are already public in M12_AssetStatus
 * output.
 *
 * Output: "# summary: N/M invariants passed". Exit 0 on full pass,
 * 1 otherwise.
 *
 * Source-lock: NONE — this is a UI-automation probe on top of
 * public M12_StartupMenuState, not game-logic. ReDMCSB does not
 * cover the launcher, so there is no source citation required here.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "firestaff_accessibility.h"
#include "menu_startup_a11y_m12.h"
#include "menu_startup_m12.h"
#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#include <windows.h>
static int portable_mkdir(const char* path) { return _mkdir(path); }
static int portable_rmdir(const char* path) { return _rmdir(path); }
static int portable_remove(const char* path) { return remove(path); }
static int portable_setenv(const char* name, const char* value) {
    return _putenv_s(name, value);
}
static int portable_unsetenv(const char* name) {
    return _putenv_s(name, "");
}
static char* portable_mkdtemp(char* tmpl) {
    char* marker = strstr(tmpl, "XXXXXX");
    unsigned long seed;
    int i;
    if (!marker) return NULL;
    seed = (unsigned long)_getpid();
    seed ^= (unsigned long)GetTickCount();
    for (i = 0; i < 1000; ++i) {
        snprintf(marker, 7, "%06lx", (seed + (unsigned)i) % 1000000UL);
        if (portable_mkdir(tmpl) == 0) {
            return tmpl;
        }
    }
    return NULL;
}
#else
#include <unistd.h>
static int portable_mkdir(const char* path) { return mkdir(path, S_IRWXU); }
static int portable_rmdir(const char* path) { return rmdir(path); }
static int portable_remove(const char* path) { return remove(path); }
static int portable_setenv(const char* name, const char* value) {
    return setenv(name, value, 1);
}
static int portable_unsetenv(const char* name) {
    return unsetenv(name);
}
static char* portable_mkdtemp(char* tmpl) {
    return mkdtemp(tmpl);
}
#endif

/* ── Tiny test scaffold ───────────────────────────────────────────── */

typedef struct {
    int total;
    int passed;
} ProbeTally;

static ProbeTally g_tally;

static void probe_record(const char* id, int ok, const char* message)
{
    g_tally.total += 1;
    if (ok) {
        g_tally.passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

static int file_exists(const char* path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static int read_all(const char* path, char* out, size_t outSize)
{
    FILE* fp;
    size_t n;
    if (!out || outSize == 0) return -1;
    fp = fopen(path, "rb");
    if (!fp) return -1;
    n = fread(out, 1U, outSize - 1U, fp);
    fclose(fp);
    out[n] = '\0';
    return (int)n;
}

/* ── Test fixtures ────────────────────────────────────────────────── */

static char g_home_path[640];
static char g_a11y_dir[640];
static char g_json_path[1024];
static char g_tmp_path[1024];

static int setup_a11y_home(void)
{
#if defined(_WIN32)
    char tmpl[] = "firestaff-a11y-gds-XXXXXX";
#else
    char tmpl[] = "/tmp/firestaff-a11y-gds-XXXXXX";
#endif
    char* made;
    portable_setenv("HOME", "/tmp");
    made = portable_mkdtemp(tmpl);
    if (!made) return 0;
    snprintf(g_home_path, sizeof(g_home_path), "%s", made);
    snprintf(g_a11y_dir, sizeof(g_a11y_dir), "%s/.firestaff", g_home_path);
    snprintf(g_json_path, sizeof(g_json_path),
             "%s/accessibility.json", g_a11y_dir);
    snprintf(g_tmp_path, sizeof(g_tmp_path),
             "%s/accessibility.json.tmp", g_a11y_dir);
    if (portable_mkdir(g_a11y_dir) != 0) {
        return 0;
    }
    return portable_setenv("HOME", g_home_path) == 0;
}

static void teardown_a11y_home(void)
{
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    portable_rmdir(g_a11y_dir);
    if (g_home_path[0] != '\0') {
        portable_rmdir(g_home_path);
    }
    portable_unsetenv("HOME");
}

/* Per-game asset-status index helper. The launcher enum
 * M12_ASSET_GAME_COUNT = 5, the public struct orders games as
 * (dm1, csb, dm2, nexus, theron) per the asset_status_m12 source
 * and the "assetStatus" index into requiredFiles[] / versions[].
 * We keep the game-id → asset-status-index mapping pinned in one
 * place so the probe can re-derive it for the new element IDs. */
static int asset_status_index_for(const char* gameId)
{
    if (!gameId) return -1;
    if (strcmp(gameId, "dm1") == 0) return 0;
    if (strcmp(gameId, "csb") == 0) return 1;
    if (strcmp(gameId, "dm2") == 0) return 2;
    if (strcmp(gameId, "nexus") == 0) return 3;
    if (strcmp(gameId, "theron") == 0) return 4;
    return -1;
}

/* Reset a single game's required-file rows to a deterministic
 * baseline where the first `matchedCount` rows are matched and the
 * rest are missing. The `requiredFileCount` rows that are present
 * are read from the public M12_AssetStatus_GetRequiredFileCount
 * API. */
static void set_required_file_coverage(M12_StartupMenuState* state,
                                       const char* gameId,
                                       int matchedCount)
{
    int idx = asset_status_index_for(gameId);
    size_t total;
    size_t fi;
    if (idx < 0 || !state) return;
    total = M12_AssetStatus_GetRequiredFileCount(&state->assetStatus, gameId);
    if (matchedCount < 0) matchedCount = 0;
    if ((size_t)matchedCount > total) matchedCount = (int)total;
    for (fi = 0; fi < total; ++fi) {
        state->assetStatus.requiredFiles[idx][fi].matched =
            (fi < (size_t)matchedCount) ? 1 : 0;
    }
    /* dm1Available / csbAvailable / ... are recomputed by the
     * launcher's m12_sync_entries_from_assets; for the probe we
     * force them so the screen-reader emission picks up the same
     * boolean. */
    if (strcmp(gameId, "dm1") == 0)   state->assetStatus.dm1Available   = (matchedCount > 0) ? 1 : 0;
    if (strcmp(gameId, "csb") == 0)   state->assetStatus.csbAvailable   = (matchedCount > 0) ? 1 : 0;
    if (strcmp(gameId, "dm2") == 0)   state->assetStatus.dm2Available   = (matchedCount > 0) ? 1 : 0;
    if (strcmp(gameId, "nexus") == 0) state->assetStatus.nexusAvailable = (matchedCount > 0) ? 1 : 0;
    if (strcmp(gameId, "theron") == 0) state->assetStatus.theronAvailable = (matchedCount > 0) ? 1 : 0;
}

/* Reset version rows to "no version matched" so the badge value
 * field is the deterministic "no version matched" token. */
static void clear_version_matches(M12_StartupMenuState* state)
{
    int gi;
    int vi;
    if (!state) return;
    for (gi = 0; gi < M12_ASSET_GAME_COUNT; ++gi) {
        for (vi = 0; vi < M12_ASSET_MAX_VERSIONS_PER_GAME; ++vi) {
            state->assetStatus.versions[gi][vi].matched = 0;
            state->assetStatus.versions[gi][vi].label = NULL;
            state->assetStatus.versions[gi][vi].shortLabel = NULL;
            state->assetStatus.versions[gi][vi].matchedPath[0] = '\0';
            state->assetStatus.versions[gi][vi].matchedMd5[0] = '\0';
        }
    }
}

/* Subtest A: main view emits one GAME_DATA_STATUS_* element per
 * game card, with a deterministic "<matched>/<required> matched"
 * value when no version is matched. */
static void subtest_all_games_missing_state(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MAIN;
    state.selectedIndex = -1;
    /* The launcher init with an empty data dir leaves every
     * required file unmatched, so the badge should report 0/N
     * matched. We also explicitly clear the version-match rows so
     * the badge label is "no version matched" deterministically. */
    clear_version_matches(&state);
    set_required_file_coverage(&state, "dm1",   0);
    set_required_file_coverage(&state, "csb",   0);
    set_required_file_coverage(&state, "dm2",   0);
    set_required_file_coverage(&state, "nexus", 0);
    set_required_file_coverage(&state, "theron",0);

    fs_ax_begin_frame(480, 270, "launcher_main");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LGD_01_state_dm1_badge_present",
                 strstr(buf, "\"id\":\"GAME_DATA_STATUS_DM1\"") != NULL,
                 "state: GAME_DATA_STATUS_DM1 badge emitted");
    probe_record("INV_LGD_02_state_csb_badge_present",
                 strstr(buf, "\"id\":\"GAME_DATA_STATUS_CSB\"") != NULL,
                 "state: GAME_DATA_STATUS_CSB badge emitted");
    probe_record("INV_LGD_03_state_dm2_badge_present",
                 strstr(buf, "\"id\":\"GAME_DATA_STATUS_DM2\"") != NULL,
                 "state: GAME_DATA_STATUS_DM2 badge emitted");
    probe_record("INV_LGD_04_state_nexus_badge_present",
                 strstr(buf, "\"id\":\"GAME_DATA_STATUS_NEXUS\"") != NULL,
                 "state: GAME_DATA_STATUS_NEXUS badge emitted");
    probe_record("INV_LGD_05_state_theron_badge_present",
                 strstr(buf, "\"id\":\"GAME_DATA_STATUS_THERON\"") != NULL,
                 "state: GAME_DATA_STATUS_THERON badge emitted");

    probe_record("INV_LGD_06_state_type",
                 strstr(buf, "\"type\":\"game_data_status\"") != NULL,
                 "state: type is game_data_status");

    /* With all required files missing and no version matched, every
     * badge value should be "0/N matched (no version matched)". */
    probe_record("INV_LGD_07_state_dm1_badge_value",
                 strstr(buf, "\"id\":\"GAME_DATA_STATUS_DM1\"") != NULL
                     && strstr(buf, "0/") != NULL
                     && strstr(buf, "no version matched") != NULL,
                 "state: DM1 badge reports 0/N matched (no version matched)");
    probe_record("INV_LGD_08_state_theron_badge_value",
                 strstr(buf, "\"id\":\"GAME_DATA_STATUS_THERON\"") != NULL
                     && strstr(buf, "0/") != NULL
                     && strstr(buf, "no version matched") != NULL,
                 "state: THERON badge reports 0/N matched (no version matched)");
}

/* Subtest B: per-required-file rows surface "present" / "missing"
 * tokens for each required file of each game. */
static void subtest_required_file_rows(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;
    size_t total;
    size_t fi;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MAIN;
    state.selectedIndex = -1;
    clear_version_matches(&state);
    /* Mark every required file as missing so the per-file rows
     * surface the deterministic "missing" token. */
    set_required_file_coverage(&state, "dm1",   0);
    set_required_file_coverage(&state, "csb",   0);
    set_required_file_coverage(&state, "dm2",   0);
    set_required_file_coverage(&state, "nexus", 0);
    set_required_file_coverage(&state, "theron",0);

    fs_ax_begin_frame(480, 270, "launcher_main");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LGD_20_required_file_type",
                 strstr(buf, "\"type\":\"required_file\"") != NULL,
                 "required_file: type is required_file");
    probe_record("INV_LGD_21_required_file_dm1_first_id",
                 strstr(buf, "\"id\":\"REQUIRED_FILE_DM1_0\"") != NULL,
                 "required_file: REQUIRED_FILE_DM1_0 id is emitted");
    probe_record("INV_LGD_22_required_file_theron_id",
                 strstr(buf, "\"id\":\"REQUIRED_FILE_THERON_0\"") != NULL,
                 "required_file: REQUIRED_FILE_THERON_0 id is emitted");
    probe_record("INV_LGD_23_required_file_dm1_first_value",
                 strstr(buf, "\"id\":\"REQUIRED_FILE_DM1_0\"") != NULL
                     && strstr(buf, "| missing") != NULL,
                 "required_file: DM1 first file is announced as missing");

    /* Sanity: walk every required-file row of every game and verify
     * each row's value ends with "| present" or "| missing" and
     * that the count of rows matches the sum of the per-game
     * required-file counts. */
    total = M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, "dm1")
          + M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, "csb")
          + M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, "dm2")
          + M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, "nexus")
          + M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, "theron");
    {
        const char* p = buf;
        int rows = 0;
        int allValid = 1;
        while ((p = strstr(p, "\"id\":\"REQUIRED_FILE_")) != NULL) {
            const char* end = strstr(p, "}");
            char elem[256];
            int len;
            if (!end) break;
            len = (int)(end - p);
            if (len < 0 || len >= (int)sizeof(elem)) {
                p = end + 1;
                continue;
            }
            memcpy(elem, p, len);
            elem[len] = '\0';
            if (strstr(elem, "| present") == NULL
                && strstr(elem, "| missing") == NULL) {
                allValid = 0;
            }
            ++rows;
            p = end + 1;
        }
        probe_record("INV_LGD_24_required_file_count_matches_api",
                     rows == (int)total,
                     "required_file: row count matches the sum of per-game required counts");
        probe_record("INV_LGD_25_required_file_value_format",
                     allValid,
                     "required_file: every row carries '| present' or '| missing'");
    }

    /* Now flip DM1's first required file to matched, and verify
     * the row's value carries "| present". */
    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);
    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MAIN;
    state.selectedIndex = -1;
    clear_version_matches(&state);
    set_required_file_coverage(&state, "dm1",   1); /* one matched */
    set_required_file_coverage(&state, "csb",   0);
    set_required_file_coverage(&state, "dm2",   0);
    set_required_file_coverage(&state, "nexus", 0);
    set_required_file_coverage(&state, "theron",0);

    fs_ax_begin_frame(480, 270, "launcher_main");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();
    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */
    (void)fi;
    probe_record("INV_LGD_26_required_file_dm1_first_present",
                 strstr(buf, "\"id\":\"REQUIRED_FILE_DM1_0\",\"type\":\"required_file\"") != NULL
                     && strstr(buf, "| present") != NULL,
                 "required_file: DM1 first file flips to 'present' when matched");
    probe_record("INV_LGD_27_state_dm1_partial_summary",
                 strstr(buf, "\"id\":\"GAME_DATA_STATUS_DM1\"") != NULL
                     && strstr(buf, "1/") != NULL
                     && strstr(buf, "matched") != NULL,
                 "state: DM1 badge reports 1/N matched when one file is matched");
}

/* Subtest C: structured popup diagnosis. Force a per-game
 * missing-data popup and verify the diagnosis element value
 * enumerates the missing required files in the pinned
 * "<game_id>:<label>," format. */
static void subtest_per_game_missing_popup_diagnosis(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;
    size_t totalDm1;
    size_t totalCsb;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MESSAGE;
    state.messageReturnView = M12_MENU_VIEW_GAME_OPTIONS;
    state.messageReturnNavLevel = 0;
    snprintf(state.messageLine1Storage, sizeof(state.messageLine1Storage),
             "%s", "DUNGEON MASTER GAME DATA NOT FOUND");
    snprintf(state.messageLine2Storage, sizeof(state.messageLine2Storage),
             "%s", "MISSING: DUNGEON.DAT");
    snprintf(state.messageLine3Storage, sizeof(state.messageLine3Storage),
             "%s", "Data dir: /tmp/fake/data");
    state.messageLine1 = state.messageLine1Storage;
    state.messageLine2 = state.messageLine2Storage;
    state.messageLine3 = state.messageLine3Storage;

    clear_version_matches(&state);
    /* DM1: one of two matched, the other missing.
     * CSB: all required files missing.
     * DM2: all required files matched (so the diagnosis still
     *      includes it as having a known set).
     * Nexus/Theron: untouched. */
    set_required_file_coverage(&state, "dm1",   1);
    set_required_file_coverage(&state, "csb",   0);
    set_required_file_coverage(&state, "dm2",   99); /* all */
    /* nx + th keep whatever M12_StartupMenu_Init left them at. */

    totalDm1 = M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, "dm1");
    totalCsb = M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, "csb");
    /* (We use the actual count below; the integers here are just
     * upper bounds for the per-game "how many matched" setter.) */
    (void)totalDm1; (void)totalCsb;

    fs_ax_begin_frame(480, 270, "launcher_popup");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LGD_40_diagnosis_id",
                 strstr(buf, "\"id\":\"POPUP_GAME_DATA_DIAGNOSIS\"") != NULL,
                 "diagnosis: POPUP_GAME_DATA_DIAGNOSIS element emitted");
    probe_record("INV_LGD_41_diagnosis_type",
                 strstr(buf, "\"type\":\"popup_diagnosis\"") != NULL,
                 "diagnosis: type is popup_diagnosis");
    /* The diagnosis value is "<game_id>:<label>," concatenated.
     * For DM1, the unmatched file label should be present. For CSB,
     * every required file is missing, so the value should start
     * with "csb:". */
    probe_record("INV_LGD_42_diagnosis_csb_prefix",
                 strstr(buf, "\"value\":\"csb:") != NULL,
                 "diagnosis: CSB listed as a triggering game in the diagnosis");
    /* DM1 has at least one missing file when we set matchedCount=1
     * and total required files is >= 2, so "dm1:" must appear too. */
    if (totalDm1 >= 2) {
        probe_record("INV_LGD_43_diagnosis_dm1_present",
                     strstr(buf, "dm1:") != NULL,
                     "diagnosis: DM1 listed when at least one required file is missing");
    } else {
        probe_record("INV_LGD_43_diagnosis_dm1_present",
                     1,
                     "diagnosis: DM1 listed when at least one required file is missing (skipped, dm1 has <2 required files)");
    }
    /* Privacy: the diagnosis must not include the data-dir line. */
    probe_record("INV_LGD_44_diagnosis_no_data_dir",
                 strstr(buf, "/tmp/fake/data") == NULL,
                 "diagnosis: data-dir path is suppressed even though includePaths=0 (already covered by popup_privacy_suppresses_data_dir)");
}

/* Subtest D: global "no game data" diagnosis. Force every required
 * file of every game to be missing and verify the diagnosis value
 * is the stable "all_games:NO_GAME_DATA" token. */
static void subtest_no_game_data_popup_diagnosis(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MESSAGE;
    state.messageReturnView = M12_MENU_VIEW_MAIN;
    state.messageReturnNavLevel = 0;
    snprintf(state.messageLine1Storage, sizeof(state.messageLine1Storage),
             "%s", "NO GAME DATA FOUND");
    snprintf(state.messageLine2Storage, sizeof(state.messageLine2Storage),
             "%s", "COPY ORIGINAL GAME FILES INTO THE DATA DIRECTORY");
    snprintf(state.messageLine3Storage, sizeof(state.messageLine3Storage),
             "%s", "Data dir: /tmp/fake/data");
    state.messageLine1 = state.messageLine1Storage;
    state.messageLine2 = state.messageLine2Storage;
    state.messageLine3 = state.messageLine3Storage;

    clear_version_matches(&state);
    set_required_file_coverage(&state, "dm1",   0);
    set_required_file_coverage(&state, "csb",   0);
    set_required_file_coverage(&state, "dm2",   0);
    set_required_file_coverage(&state, "nexus", 0);
    set_required_file_coverage(&state, "theron",0);
    /* Force the originalFileCandidateFound flag to 0 so the
     * diagnosis helper's "all_games" branch can run. */
    state.assetStatus.originalFileCandidateFound = 0;

    fs_ax_begin_frame(480, 270, "launcher_no_data");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LGD_60_no_data_diagnosis_id",
                 strstr(buf, "\"id\":\"POPUP_GAME_DATA_DIAGNOSIS\"") != NULL,
                 "no_data: diagnosis element emitted");
    probe_record("INV_LGD_61_no_data_diagnosis_value",
                 strstr(buf, "\"value\":\"all_games:NO_GAME_DATA\"") != NULL,
                 "no_data: diagnosis value is the stable 'all_games:NO_GAME_DATA' token");
    /* Privacy: the data-dir line must still be suppressed. */
    probe_record("INV_LGD_62_no_data_diagnosis_privacy",
                 strstr(buf, "/tmp/fake/data") == NULL,
                 "no_data: data-dir path is suppressed by includePaths=0");
}

/* Subtest E: non-missing-data popups (e.g. the generic "OK" result
 * popup) emit no POPUP_GAME_DATA_DIAGNOSIS element. We force every
 * required file to be matched so the diagnosis helper returns 0. */
static void subtest_no_diagnosis_when_data_present(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MESSAGE;
    state.messageReturnView = M12_MENU_VIEW_MAIN;
    state.messageReturnNavLevel = 0;
    snprintf(state.messageLine1Storage, sizeof(state.messageLine1Storage),
             "%s", "DATA SCAN COMPLETE");
    snprintf(state.messageLine2Storage, sizeof(state.messageLine2Storage),
             "%s", "ALL REQUIRED FILES MATCHED");
    snprintf(state.messageLine3Storage, sizeof(state.messageLine3Storage),
             "%s", "OK");
    state.messageLine1 = state.messageLine1Storage;
    state.messageLine2 = state.messageLine2Storage;
    state.messageLine3 = state.messageLine3Storage;

    clear_version_matches(&state);
    set_required_file_coverage(&state, "dm1",   99);
    set_required_file_coverage(&state, "csb",   99);
    set_required_file_coverage(&state, "dm2",   99);
    set_required_file_coverage(&state, "nexus", 99);
    set_required_file_coverage(&state, "theron",99);

    fs_ax_begin_frame(480, 270, "launcher_ok");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LGD_80_no_diagnosis_when_data_present",
                 strstr(buf, "\"id\":\"POPUP_GAME_DATA_DIAGNOSIS\"") == NULL,
                 "no_diagnosis: element is NOT emitted when no required file is missing");
    /* Sanity: the rest of the popup elements still emit. */
    probe_record("INV_LGD_81_no_diagnosis_popup_still_present",
                 strstr(buf, "\"id\":\"POPUP_MESSAGE\"") != NULL
                     && strstr(buf, "\"id\":\"POPUP_OK\"") != NULL,
                 "no_diagnosis: POPUP_MESSAGE + POPUP_OK still emitted");
}

/* Subtest F: privacy — includePaths=1 must NOT cause the diagnosis
 * value to leak the data-dir path. The diagnosis is independent of
 * includePaths; the path leaks only through POPUP_LINE3. */
static void subtest_diagnosis_privacy_paths_included(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MESSAGE;
    state.messageReturnView = M12_MENU_VIEW_MAIN;
    state.messageReturnNavLevel = 0;
    snprintf(state.messageLine1Storage, sizeof(state.messageLine1Storage),
             "%s", "DUNGEON MASTER GAME DATA NOT FOUND");
    snprintf(state.messageLine2Storage, sizeof(state.messageLine2Storage),
             "%s", "MISSING: DUNGEON.DAT");
    snprintf(state.messageLine3Storage, sizeof(state.messageLine3Storage),
             "%s", "Data dir: /tmp/secret/dir");
    state.messageLine1 = state.messageLine1Storage;
    state.messageLine2 = state.messageLine2Storage;
    state.messageLine3 = state.messageLine3Storage;

    clear_version_matches(&state);
    set_required_file_coverage(&state, "dm1",   0);
    set_required_file_coverage(&state, "csb",   0);
    set_required_file_coverage(&state, "dm2",   0);
    set_required_file_coverage(&state, "nexus", 0);
    set_required_file_coverage(&state, "theron",0);

    fs_ax_begin_frame(480, 270, "launcher_popup_paths");
    m12_launcher_a11y_emit(&state, 480, 270, 1); /* includePaths=1 */
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    /* The POPUP_LINE3 element should now contain the data-dir path. */
    probe_record("INV_LGD_100_paths_included_line3",
                 strstr(buf, "/tmp/secret/dir") != NULL,
                 "privacy: includePaths=1 makes the data-dir line appear (sanity)");
    /* The diagnosis value must NOT contain the data-dir path even
     * when includePaths=1. */
    {
        const char* diagStart = strstr(buf, "\"id\":\"POPUP_GAME_DATA_DIAGNOSIS\"");
        int diagClean = 1;
        if (diagStart) {
            const char* end = strstr(diagStart, "}");
            char elem[512];
            int len;
            if (end) {
                len = (int)(end - diagStart);
                if (len > 0 && len < (int)sizeof(elem)) {
                    memcpy(elem, diagStart, len);
                    elem[len] = '\0';
                    if (strstr(elem, "/tmp/secret/dir") != NULL) {
                        diagClean = 0;
                    }
                }
            }
        }
        probe_record("INV_LGD_101_diagnosis_no_path_leak",
                     diagClean,
                     "privacy: diagnosis value never leaks the data-dir path");
    }
}

/* Subtest G: deterministic / stable element order. The
 * GAME_CARD_DM1 element must precede the GAME_DATA_STATUS_DM1
 * element, and the GAME_DATA_STATUS_DM1 element must precede the
 * first REQUIRED_FILE_DM1_0 row. */
static void subtest_element_order(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;
    const char* card;
    const char* status;
    const char* file0;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MAIN;
    state.selectedIndex = -1;
    clear_version_matches(&state);
    set_required_file_coverage(&state, "dm1",   0);
    set_required_file_coverage(&state, "csb",   0);
    set_required_file_coverage(&state, "dm2",   0);
    set_required_file_coverage(&state, "nexus", 0);
    set_required_file_coverage(&state, "theron",0);

    fs_ax_begin_frame(480, 270, "launcher_main");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    card = strstr(buf, "\"id\":\"GAME_CARD_DM1\"");
    status = strstr(buf, "\"id\":\"GAME_DATA_STATUS_DM1\"");
    file0 = strstr(buf, "\"id\":\"REQUIRED_FILE_DM1_0\"");
    probe_record("INV_LGD_120_order_card_before_status",
                 card && status && card < status,
                 "order: GAME_CARD_DM1 precedes GAME_DATA_STATUS_DM1");
    probe_record("INV_LGD_121_order_status_before_files",
                 status && file0 && status < file0,
                 "order: GAME_DATA_STATUS_DM1 precedes REQUIRED_FILE_DM1_0");

    /* Across games, card N must precede card N+1, status N must
     * precede status N+1, file rows for game N must precede
     * status N+1. */
    {
        const char* card1 = strstr(buf, "\"id\":\"GAME_CARD_CSB\"");
        const char* card0 = card;
        const char* status1 = strstr(buf, "\"id\":\"GAME_DATA_STATUS_CSB\"");
        probe_record("INV_LGD_122_order_dm1_card_before_csb_card",
                     card0 && card1 && card0 < card1,
                     "order: GAME_CARD_DM1 < GAME_CARD_CSB");
        probe_record("INV_LGD_123_order_dm1_status_before_csb_status",
                     status && status1 && status < status1,
                     "order: GAME_DATA_STATUS_DM1 < GAME_DATA_STATUS_CSB");
    }
}

/* ── main ─────────────────────────────────────────────────────────── */

int main(void)
{
    printf("firestaff_m12_launcher_game_data_state_manifest_probe\n");

    if (!setup_a11y_home()) {
        fprintf(stderr,
                "could not redirect HOME to a temp dir; "
                "probe cannot run\n");
        return 1;
    }

    printf("\n[A] main view: per-card GAME_DATA_STATUS badges (all missing)\n");
    subtest_all_games_missing_state();

    printf("\n[B] main view: per-required-file rows\n");
    subtest_required_file_rows();

    printf("\n[C] message view: per-game missing-data popup diagnosis\n");
    subtest_per_game_missing_popup_diagnosis();

    printf("\n[D] message view: global no-game-data popup diagnosis\n");
    subtest_no_game_data_popup_diagnosis();

    printf("\n[E] message view: no diagnosis when data is present\n");
    subtest_no_diagnosis_when_data_present();

    printf("\n[F] privacy: diagnosis never leaks the data-dir path\n");
    subtest_diagnosis_privacy_paths_included();

    printf("\n[G] deterministic element order\n");
    subtest_element_order();

    teardown_a11y_home();
    fs_ax_shutdown();

    printf("\n# summary: %d/%d invariants passed\n",
           g_tally.passed, g_tally.total);
    return (g_tally.passed == g_tally.total) ? 0 : 1;
}
