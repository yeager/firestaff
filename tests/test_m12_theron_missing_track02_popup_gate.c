/*
 * test_m12_theron_missing_track02_popup_gate.c
 *
 * M12 launcher regression gate proving the Theron missing Track 02
 * launch path is correctly blocked with an actionable, user-facing
 * popup while the unrelated DM1 / CSB / DM2 / Nexus games remain
 * unaffected.
 *
 * The launcher used to fall through to the generic
 * `m12_show_missing_game_data_popup()` for every game, which produced
 * a popup body of "MISSING: Track 02 data image (JP, primary)" —
 * misleading for US/ISO media, leaking internal spec wording and giving the
 * user no hint about which file extension to drop or which folder to
 * drop it into.
 *
 * This gate exercises:
 *
 *   1. The hash-table contract: Theron still exposes 4 known hashes
 *      (JP/US BIN + JP/US ISO) and exactly one required-file role
 *      (Track 02 JP/US BIN/ISO marker).
 *   2. Asset-availability negative test: with no real Theron data,
 *      `M12_AssetStatus_GameAvailable("theron")` returns 0 but
 *      `M12_AssetStatus_GameHasCompleteHashSet("theron")` still
 *      returns 1 — proving the launcher has the catalog it needs to
 *      produce the user-facing Track 02 hint.
 *   3. Game-card click on the unavailable Theron card surfaces the
 *      Track 02 specific popup (not the generic popup), with line1
 *      prefixed "THERON", line2 starting with the per-role required
 *      file list "MISSING: ...", and an actionable hint that names
 *      the .BIN/.ISO extension. launchRequested stays 0 and
 *      `M12_StartupMenu_GetLaunchIntent().valid` stays 0.
 *   4. Options-screen Launch row produces the same popup and intent
 *      verdict.
 *   5. Quick Resume armed for Theron produces the same popup.
 *   6. Dismissal cleanly returns to MAIN with cleared message lines.
 *   7. Unrelated games (DM1, CSB, DM2, Nexus) remain unaffected when
 *      only the Theron Track 02 is missing — they still go through
 *      the generic missing-game-data popup with their own per-game
 *      prefix and required-file list, never the Track 02 hint.
 *   8. Render smoke: the popup view paints non-blank startup pixels.
 *   9. Hash-set stability: the gate compiles, the four known hashes
 *      remain indexed, and the missing-required hash test still
 *      reports exactly one required file with `required && !matched`.
 *
 * Source-lock: dmweb Theron's Quest reference + greatstone
 * d_track02.html (CD-ROM data-track position), ReDMCSB ENTRANCE.C
 * F0806 (media-load boundary used by CSB / DM1 / DM2 launcher
 * gating), and the existing `M12_AssetStatus_GameHasCompleteHashSet`
 * + `M12_AssetStatus_GameKnownHashCount` contracts.
 *
 * Runs under SDL_VIDEODRIVER=dummy; no real game data is needed.
 */

#include "asset_status_m12.h"
#include "menu_startup_m12.h"
#include "menu_startup_render_modern_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#endif

static int g_failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++g_failures; \
    } \
} while (0)

#define CHECK_STR_CONTAINS(haystack, needle) do { \
    const char* _h = (haystack); \
    const char* _n = (needle); \
    if (!_h || !_n || strstr(_h, _n) == NULL) { \
        fprintf(stderr, \
                "FAIL %s:%d: expected \"%s\" to contain \"%s\" (got: %s)\n", \
                __FILE__, __LINE__, _h ? _h : "(null)", _n, _h ? _h : "(null)"); \
        ++g_failures; \
    } \
} while (0)

#define CHECK_STR_STARTS_WITH(haystack, prefix) do { \
    const char* _h = (haystack); \
    const char* _p = (prefix); \
    if (!_h || !_p) { \
        fprintf(stderr, "FAIL %s:%d: NULL haystack/prefix in startswith\n", __FILE__, __LINE__); \
        ++g_failures; \
        break; \
    } \
    if (strncmp(_h, _p, strlen(_p)) != 0) { \
        fprintf(stderr, \
                "FAIL %s:%d: expected \"%s\" to start with \"%s\"\n", \
                __FILE__, __LINE__, _h, _p); \
        ++g_failures; \
    } \
} while (0)

/* Empty-data-dir helper: creates a unique tmp directory that is
 * guaranteed empty so M12_AssetStatus_Scan() reports no game as
 * available. The launcher treats the empty dir as "no data found". */
static int isolate_empty_data_dir(char out[M12_ASSET_DATA_DIR_CAPACITY]) {
#if defined(_WIN32)
    char tmp[256];
    snprintf(tmp, sizeof(tmp), ".\\firestaff_theron_track02_empty_%lu",
             (unsigned long)rand());
    if (TEST_MKDIR(tmp) != 0) {
        return 0;
    }
    snprintf(out, M12_ASSET_DATA_DIR_CAPACITY, "%s", tmp);
    return 1;
#else
    char tmpTemplate[] = "/tmp/firestaff_theron_track02_empty_XXXXXX";
    char* made = mkdtemp(tmpTemplate);
    if (!made) {
        return 0;
    }
    snprintf(out, M12_ASSET_DATA_DIR_CAPACITY, "%s", made);
    return 1;
#endif
}

static int popup_lines_cleared(const M12_StartupMenuState* state) {
    if (!state) return 0;
    if (state->messageLine1 && state->messageLine1[0] != '\0') return 0;
    if (state->messageLine2 && state->messageLine2[0] != '\0') return 0;
    if (state->messageLine3 && state->messageLine3[0] != '\0') return 0;
    if (state->messageIsMissingGameData != 0) return 0;
    if (state->messageGameId[0] != '\0') return 0;
    return 1;
}

static int launcher_in_clean_main(const M12_StartupMenuState* state) {
    if (!state) return 0;
    return state->view == M12_MENU_VIEW_MAIN &&
           state->launchRequested == 0 &&
           state->quickResumeLaunchRequested == 0 &&
           state->dataDirPickerActive == 0 &&
           popup_lines_cleared(state);
}

static int render_smoke_nonblank(const M12_StartupMenuState* state, const char* label) {
    const int w = M12_ModernMenu_NativeWidth();
    const int h = M12_ModernMenu_NativeHeight();
    const size_t bytes = (size_t)w * (size_t)h * 4u;
    unsigned char* rgba = (unsigned char*)malloc(bytes);
    int distinct;
    if (!rgba) {
        fprintf(stderr, "FAIL: %s render buffer allocation\n", label);
        return 0;
    }
    memset(rgba, 0, bytes);
    M12_ModernMenu_Render(state, rgba, w, h);
    distinct = M12_ModernMenu_CountDistinctColors(rgba, w, h, 128);
    free(rgba);
    if (distinct < 3) {
        fprintf(stderr, "FAIL: %s render should produce nonblank popup pixels\n", label);
        return 0;
    }
    return 1;
}

/* Seed Theron with synthetic Track 02 metadata so the launcher enters
 * GAME_OPTIONS for the click path. The card itself stays unavailable
 * (because theronAvailable=0) so clicking it routes to the popup. */
static void seed_theron_unavailable_with_metadata(M12_StartupMenuState* state,
                                                  int synthAvailableFlag) {
    int gi = 4;  /* theron is the 5th catalog entry */
    M12_AssetVersionStatus* jp;
    M12_AssetRequiredFileStatus* req;
    (void)synthAvailableFlag;

    state->entries[gi].title = "THERON'S QUEST";
    state->entries[gi].gameId = "theron";
    state->entries[gi].kind = M12_MENU_ENTRY_GAME;
    state->entries[gi].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[gi].available = 0;
    state->assetStatus.theronAvailable = 0;

    state->assetStatus.requiredFileCounts[gi] = 1U;
    req = &state->assetStatus.requiredFiles[gi][0];
    memset(req, 0, sizeof(*req));
    req->gameId = "theron";
    req->roleId = "track02";
    req->label = "Track 02 data image (JP/US BIN/ISO)";
    req->required = 1;
    req->matched = 0;

    jp = &state->assetStatus.versions[gi][0];
    memset(jp, 0, sizeof(*jp));
    jp->gameId = "theron";
    jp->versionId = "pce-jp";
    jp->label = "PC Engine JP (Track 02)";
    jp->shortLabel = "PCE JP";
    jp->matched = 0;

    state->gameOptions[gi].versionIndex = 0;
    state->gameOptions[gi].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
}

/* Seed one of DM1/CSB/DM2/Nexus as "available" while Theron stays
 * missing so we can prove the unrelated games are not affected by the
 * Theron popup gate. */
static void seed_dm1_available_only(M12_StartupMenuState* state) {
    int gi = 0;  /* dm1 is the 1st catalog entry */
    M12_AssetVersionStatus* v;
    M12_AssetRequiredFileStatus* g;
    M12_AssetRequiredFileStatus* d;

    state->entries[gi].title = "DUNGEON MASTER";
    state->entries[gi].gameId = "dm1";
    state->entries[gi].kind = M12_MENU_ENTRY_GAME;
    state->entries[gi].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[gi].available = 1;
    state->assetStatus.dm1Available = 1;

    state->assetStatus.requiredFileCounts[gi] = 2U;
    g = &state->assetStatus.requiredFiles[gi][0];
    memset(g, 0, sizeof(*g));
    g->gameId = "dm1";
    g->roleId = "graphics";
    g->label = "GRAPHICS.DAT";
    g->required = 1;
    g->matched = 1;

    d = &state->assetStatus.requiredFiles[gi][1];
    memset(d, 0, sizeof(*d));
    d->gameId = "dm1";
    d->roleId = "dungeon";
    d->label = "DUNGEON.DAT";
    d->required = 1;
    d->matched = 1;

    v = &state->assetStatus.versions[gi][0];
    memset(v, 0, sizeof(*v));
    v->gameId = "dm1";
    v->versionId = "pc34-en";
    v->label = "PC 3.4 English";
    v->shortLabel = "PC 3.4 EN";
    v->matched = 1;

    state->gameOptions[gi].versionIndex = 0;
    state->gameOptions[gi].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;

    state->assetStatus.originalFileCandidateFound = 1;
}

static void check_hash_set_contract(void) {
    /* Hash set: 4 known hashes, 1 required role. These are the static
     * numbers the popup gate relies on to produce a user-facing
     * Track 02 hint without consulting docs. */
    CHECK(M12_AssetStatus_GameHasCompleteHashSet("theron") == 1);
    CHECK(M12_AssetStatus_GameKnownHashCount("theron") == 4U);
    CHECK(M12_AssetStatus_GameRequiredFileCount("theron") == 1U);
    CHECK(M12_AssetStatus_GameVerifiedFileCount("theron") == 1U);
    CHECK(M12_AssetStatus_FindVersionIndex("theron", "pce-jp") == 0);
    CHECK(M12_AssetStatus_FindVersionIndex("theron", "pce-en") == 1);
    CHECK(M12_AssetStatus_FindVersionIndex("theron", "pce-jp-rev1-iso") == 2);
    CHECK(M12_AssetStatus_FindVersionIndex("theron", "pce-en-iso") == 3);
}

static void check_scan_no_data_marks_theron_unavailable(char* dataDir) {
    M12_AssetStatus status;
    memset(&status, 0, sizeof(status));
    M12_AssetStatus_Scan(&status, dataDir);
    CHECK(M12_AssetStatus_GameHasCompleteHashSet("theron") == 1);
    CHECK(M12_AssetStatus_GameAvailable(&status, "theron") == 0);
    /* The required Track 02 role is reported as required + unmatched
     * so the popup can list it. */
    CHECK(M12_AssetStatus_GetRequiredFileCount(&status, "theron") == 1U);
    {
        const M12_AssetRequiredFileStatus* req =
            M12_AssetStatus_GetRequiredFile(&status, "theron", 0U);
        CHECK(req != NULL);
        CHECK(req->required == 1);
        CHECK(req->matched == 0);
        CHECK(req->label && strcmp(req->label, "Track 02 data image (JP/US BIN/ISO)") == 0);
    }
}

static void check_card_click_popup_surfaces_track02_hint(char* dataDir) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const char* line1;
    const char* line2;
    int theronIndex = 4;

    M12_StartupMenu_InitWithDataDir(&state, dataDir, NULL);
    /* The empty-data init surfaces the "NO GAME DATA FOUND" popup.
     * Dismiss it so we land on a clean MAIN view. */
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    seed_theron_unavailable_with_metadata(&state, 0);
    state.selectedIndex = theronIndex;

    /* Selection event on the unavailable Theron card. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);

    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.messageIsMissingGameData == 1);
    CHECK(strcmp(state.messageGameId, "theron") == 0);

    line1 = state.messageLine1 ? state.messageLine1 : "";
    line2 = state.messageLine2 ? state.messageLine2 : "";
    CHECK_STR_STARTS_WITH(line1, "THERON");
    CHECK_STR_CONTAINS(line1, "GAME DATA NOT FOUND");
    /* The popup body must mention the .BIN/.ISO extension so the
     * user knows which file format to drop, JP/US so a valid US
     * Track 02 does not look like the wrong media, and the data-dir
     * hint so they know where to put it. */
    CHECK_STR_CONTAINS(line2, ".BIN");
    CHECK_STR_CONTAINS(line2, ".ISO");
    CHECK_STR_CONTAINS(line2, "JP/US");
    CHECK_STR_CONTAINS(line2, "DATA DIR");
    /* Footer still anchors the data directory so the user can find
     * it. */
    CHECK(state.messageLine3 && strstr(state.messageLine3, "DATA DIR") != NULL);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);

    CHECK(render_smoke_nonblank(&state, "theron card click Track 02 popup"));

    /* Dismissal returns to MAIN with cleared message lines and no
     * leaked launch request. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(launcher_in_clean_main(&state));
}

static void check_options_launch_popup_surfaces_track02_hint(char* dataDir) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const char* line2;
    int theronIndex = 4;

    M12_StartupMenu_InitWithDataDir(&state, dataDir, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    seed_theron_unavailable_with_metadata(&state, 0);

    /* Mark the card as available only for routing purposes so the
     * launcher reaches the GAME_OPTIONS Launch row. We then flip
     * theronAvailable back to 0 so the Launch row triggers the
     * missing-data path (this mirrors the real "card shows ready
     * because the catalog has hashes, but the data root has no real
     * Track 02" scenario). */
    state.entries[theronIndex].available = 1;
    state.activatedIndex = theronIndex;
    state.view = M12_MENU_VIEW_GAME_OPTIONS;
    state.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;  /* +1 = Launch row */
    state.assetStatus.theronAvailable = 0;

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);

    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.messageIsMissingGameData == 1);
    CHECK(strcmp(state.messageGameId, "theron") == 0);
    line2 = state.messageLine2 ? state.messageLine2 : "";
    CHECK_STR_CONTAINS(line2, ".BIN");
    CHECK_STR_CONTAINS(line2, ".ISO");
    CHECK_STR_CONTAINS(line2, "JP/US");

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    /* Dismissal from GAME_OPTIONS returns to GAME_OPTIONS, not MAIN.
     * Verify the launcher is back on the options screen with cleared
     * message lines and no leaked launch request. */
    CHECK(state.view == M12_MENU_VIEW_GAME_OPTIONS);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(popup_lines_cleared(&state));
}

static void check_quick_resume_popup_surfaces_track02_hint(char* dataDir) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const char* line1;
    const char* line2;

    M12_StartupMenu_InitWithDataDir(&state, dataDir, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    seed_theron_unavailable_with_metadata(&state, 0);

    /* Arm quick resume for Theron while no real Track 02 is on disk. */
    state.quickResumeAvailable = 1;
    snprintf(state.quickResumeGameId, sizeof(state.quickResumeGameId), "theron");
    state.selectedIndex = -1;

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.messageIsMissingGameData == 1);
    CHECK(strcmp(state.messageGameId, "theron") == 0);

    line1 = state.messageLine1 ? state.messageLine1 : "";
    line2 = state.messageLine2 ? state.messageLine2 : "";
    CHECK_STR_STARTS_WITH(line1, "THERON");
    CHECK_STR_CONTAINS(line2, ".BIN");
    CHECK_STR_CONTAINS(line2, ".ISO");
    CHECK_STR_CONTAINS(line2, "JP/US");

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(launcher_in_clean_main(&state));
}

static void check_unrelated_games_unaffected_by_theron_gate(char* dataDir) {
    M12_StartupMenuState state;
    const char* line1;
    const char* line2;

    M12_StartupMenu_InitWithDataDir(&state, dataDir, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    seed_dm1_available_only(&state);
    /* Keep all five catalog entries at their default "no required
     * files matched" state so DM1's required files remain unmatched. */
    {
        size_t gi;
        for (gi = 0U; gi < (size_t)M12_CONFIG_GAME_COUNT; ++gi) {
            state.assetStatus.requiredFileCounts[gi] = 0U;
            state.assetStatus.versions[gi][0].gameId = NULL;
            state.assetStatus.versions[gi][0].versionId = NULL;
            state.assetStatus.versions[gi][0].label = NULL;
            state.assetStatus.versions[gi][0].shortLabel = NULL;
            state.assetStatus.versions[gi][0].matched = 0;
        }
        /* Reinstall DM1's seeded required files + version metadata. */
        seed_dm1_available_only(&state);
        /* Force DM1 to be unavailable so its card routes through the
         * generic missing-game-data popup (not the Track 02 one). */
        state.entries[0].available = 0;
        state.assetStatus.dm1Available = 0;
    }

    state.selectedIndex = 0;  /* DM1 card */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageIsMissingGameData == 1);
    CHECK(strcmp(state.messageGameId, "dm1") == 0);

    line1 = state.messageLine1 ? state.messageLine1 : "";
    line2 = state.messageLine2 ? state.messageLine2 : "";
    CHECK_STR_STARTS_WITH(line1, "DM1");
    CHECK_STR_CONTAINS(line1, "GAME DATA NOT FOUND");
    /* The DM1 popup must NOT carry the Theron Track 02 hint. */
    if (line2 && strstr(line2, ".BIN") != NULL && strstr(line2, "TRACK 02") != NULL) {
        fprintf(stderr,
                "FAIL: DM1 popup leaked the Theron Track 02 hint: \"%s\"\n",
                line2);
        ++g_failures;
    }
    /* Dismissal returns to MAIN. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(launcher_in_clean_main(&state));
}

int main(void) {
    char dataDir[M12_ASSET_DATA_DIR_CAPACITY];
    int rc;

#if defined(_WIN32)
    rc = _putenv_s("SDL_VIDEODRIVER", "dummy") == 0;
#else
    rc = setenv("SDL_VIDEODRIVER", "dummy", 1) == 0;
#endif
    if (!rc) {
        fprintf(stderr, "FAIL: could not set SDL_VIDEODRIVER=dummy\n");
        return 1;
    }

    if (!isolate_empty_data_dir(dataDir)) {
        fprintf(stderr, "FAIL: could not create empty data dir\n");
        return 1;
    }

    check_hash_set_contract();
    check_scan_no_data_marks_theron_unavailable(dataDir);
    check_card_click_popup_surfaces_track02_hint(dataDir);
    check_options_launch_popup_surfaces_track02_hint(dataDir);
    check_quick_resume_popup_surfaces_track02_hint(dataDir);
    check_unrelated_games_unaffected_by_theron_gate(dataDir);

    if (g_failures) {
        fprintf(stderr, "%d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: M12 Theron missing Track 02 popup gate blocks launch with actionable hint");
    puts("sourceEvidence=dmweb Theron reference + greatstone d_track02.html + ReDMCSB ENTRANCE.C F0806");
    return 0;
}
