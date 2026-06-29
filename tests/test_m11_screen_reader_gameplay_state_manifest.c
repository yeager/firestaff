/*
 * test_m11_screen_reader_gameplay_state_manifest.c
 *
 *   Regression for the M11 gameplay-side screen-reader / accessibility
 *   manifest (include/m11_game_view_a11y.h +
 *   src/engine/m11_game_view_a11y.c).
 *
 * Mirrors the established pattern of
 * tests/test_firestaff_accessibility_manifest.c (which covers the
 * fs_ax_* writer) and tests/test_m12_screen_reader_launcher_state_manifest.c
 * (which covers the launcher-side M12 manifest).
 *
 * Subtests:
 *   1. Disabled state writes nothing - m11_screen_reader_update_ex must
 *      be a no-op when fs_ax_is_enabled() returns 0, even when called
 *      with a populated M11_GameViewState.
 *   2. State name mapping is stable - every M11_AX_State enum value
 *      maps to its canonical name; out-of-range collapses to "other";
 *      NULL state maps to "other".
 *   3. State classifier maps M11_GameViewState flags correctly -
 *      gameplay (no overlays), inventory (inventoryPanelActive),
 *      map (mapOverlayActive), dialog (dialogOverlayActive),
 *      entrance_mirror (candidateMirrorPanelActive), and endgame
 *      (gameWon).
 *   4. Gameplay state manifest emits the always-on viewport /
 *      movement / spell / HUD / control strip zones plus the
 *      gameplay_root envelope with gameState="gameplay".
 *   5. Inventory state manifest adds the champion portrait /
 *      champion name / 13 equipment slots / 16 backpack slots /
 *      panel region (plus the always-on zones from subtest 4).
 *   6. Map state manifest adds the AUTOMAP full-screen region
 *      and gameState="map".
 *   7. Dialog state manifest adds the dialog body + up to 4
 *      choice buttons and gameState="dialog".
 *   8. Candidate mirror state manifest adds the panel region +
 *      RESURRECT / REINCARNATE / CANCEL buttons with
 *      gameState="entrance_mirror".
 *   9. Endgame state manifest adds the THE END plaque + 4
 *      champion mirrors + 4 champion portraits with
 *      gameState="endgame".
 *  10. Deterministic guards - JSON is well-formed (no leading-
 *      comma / trailing-comma / double-comma artifacts) across
 *      every state, even when a value field is non-NULL.
 *  11. NULL state is a no-op (returns 0, writes nothing).
 *
 * Test isolation: redirects HOME to a per-run temp dir so the manifest
 * never touches the real user ~/.firestaff.  The test uses
 * memset(&state, 0, sizeof(state)) so no DM1 game data is required.
 */

#include "firestaff_accessibility.h"
#include "m11_game_view_a11y.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define M11_AX_MKDIR(p)    _mkdir(p)
#define M11_AX_RMDIR(p)    _rmdir(p)
#define M11_AX_STAT_FN     _stat
#define M11_AX_STAT_TYPE   struct _stat
#define M11_AX_REMOVE(p)   remove(p)
static int m11_ax_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
static int m11_ax_unsetenv(const char* name) {
    return _putenv_s(name, "") == 0;
}
#else
#include <unistd.h>
#define M11_AX_MKDIR(p)    mkdir((p), 0700)
#define M11_AX_RMDIR(p)    rmdir(p)
#define M11_AX_STAT_FN     stat
#define M11_AX_STAT_TYPE   struct stat
#define M11_AX_REMOVE(p)   remove(p)
static int m11_ax_setenv(const char* name, const char* value) {
    return setenv(name, value, 1) == 0;
}
static int m11_ax_unsetenv(const char* name) {
    return unsetenv(name) == 0;
}
#endif

static int g_failures = 0;
static int g_passes = 0;
static char g_home_path[640];
static char g_json_path[1024];
static char g_tmp_path[1024];
static char g_subdir_path[1024];

static void m11_ax_check(int cond, const char* msg) {
    if (cond) {
        ++g_passes;
    } else {
        ++g_failures;
        fprintf(stderr, "  FAIL: %s\n", msg);
    }
}

static int m11_ax_file_exists(const char* path) {
    M11_AX_STAT_TYPE st;
    return M11_AX_STAT_FN(path, &st) == 0;
}

static int m11_ax_read_all(const char* path, char* out, size_t outSize) {
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

static void m11_ax_recompute_paths(void) {
    snprintf(g_subdir_path, sizeof(g_subdir_path),
             "%s/.firestaff", g_home_path);
    snprintf(g_json_path, sizeof(g_json_path),
             "%s/accessibility.json", g_subdir_path);
    snprintf(g_tmp_path, sizeof(g_tmp_path),
             "%s.tmp", g_json_path);
}

static int m11_ax_setup_temp_home(void) {
#if defined(_WIN32)
    char tmpl[64];
    unsigned int seed = (unsigned int)(_getpid() ^ 0x9E3779B9U);
    int attempt;
    g_home_path[0] = '\0';
    for (attempt = 0; attempt < 32; ++attempt) {
        seed = seed * 1103515245U + 12345U;
        snprintf(tmpl, sizeof(tmpl), "./firestaff_m11sr_test_%x",
                 seed & 0xFFFFFFU);
        snprintf(g_home_path, sizeof(g_home_path), "%s", tmpl);
        if (M11_AX_MKDIR(g_home_path) == 0) {
            break;
        }
        g_home_path[0] = '\0';
    }
    if (g_home_path[0] == '\0') return 0;
#else
    char tmpl[] = "/tmp/firestaff-m11sr-XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) return 0;
    snprintf(g_home_path, sizeof(g_home_path), "%s", made);
#endif
    m11_ax_recompute_paths();
    return m11_ax_setenv("HOME", g_home_path) ? 1 : 0;
}

static void m11_ax_teardown_temp_home(void) {
    M11_AX_REMOVE(g_json_path);
    M11_AX_REMOVE(g_tmp_path);
    M11_AX_REMOVE(g_subdir_path);
    if (g_home_path[0] != '\0') {
        M11_AX_RMDIR(g_home_path);
    }
    (void)m11_ax_unsetenv("HOME");
}

static void m11_ax_clean_artifacts(void) {
    fs_ax_shutdown();
    M11_AX_REMOVE(g_json_path);
    M11_AX_REMOVE(g_tmp_path);
}

/* -- Subtest 1: disabled state writes nothing --------------------- */

static void subtest_disabled_state_is_noop(void) {
    M11_GameViewState state;

    m11_ax_clean_artifacts();
    memset(&state, 0, sizeof(state));
    state.active = 1;

    m11_ax_check(fs_ax_is_enabled() == 0,
                 "disabled: writer is off by default (no FS_ACCESSIBILITY)");

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 0,
                     "disabled: m11_screen_reader_update_ex returns 0 when off");
    }
    m11_ax_check(!m11_ax_file_exists(g_json_path),
                 "disabled: no accessibility.json written");
    m11_ax_check(!m11_ax_file_exists(g_tmp_path),
                 "disabled: no .tmp residue left behind");
}

/* -- Subtest 2: state name mapping is stable ---------------------- */

static void subtest_state_name_mapping_is_stable(void) {
    static const struct { M11_AX_State state; const char* expected; } cases[] = {
        {M11_AX_STATE_GAMEPLAY,        "gameplay"},
        {M11_AX_STATE_INVENTORY,       "inventory"},
        {M11_AX_STATE_MAP,             "map"},
        {M11_AX_STATE_DIALOG,          "dialog"},
        {M11_AX_STATE_ENTRANCE_MIRROR, "entrance_mirror"},
        {M11_AX_STATE_ENDGAME,         "endgame"},
        {M11_AX_STATE_OTHER,           "other"}
    };
    int i;
    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        const char* got = m11_screen_reader_state_name(cases[i].state);
        m11_ax_check(got != NULL, "mapping: state name is non-NULL");
        if (!got) continue;
        m11_ax_check(strcmp(got, cases[i].expected) == 0,
                     "mapping: state maps to canonical state name");
        /* Peekaboo schema constraint: no whitespace in state names. */
        m11_ax_check(strchr(got, ' ') == NULL &&
                     strchr(got, '\t') == NULL &&
                     strchr(got, '\n') == NULL,
                     "mapping: state name has no whitespace");
    }
    /* Out-of-range enum value collapses to "other". */
    m11_ax_check(strcmp(m11_screen_reader_state_name((M11_AX_State)9999),
                        "other") == 0,
                 "mapping: out-of-range state -> \"other\"");
}

/* -- Subtest 3: state classifier is correct ----------------------- */

static void subtest_state_classifier(void) {
    M11_GameViewState state;

    /* (a) default zero state -> gameplay. */
    memset(&state, 0, sizeof(state));
    state.active = 1;
    m11_ax_check(m11_screen_reader_state_for(&state) == M11_AX_STATE_GAMEPLAY,
                 "classifier: default state -> GAMEPLAY");

    /* (b) inventoryPanelActive -> inventory. */
    memset(&state, 0, sizeof(state));
    state.inventoryPanelActive = 1;
    m11_ax_check(m11_screen_reader_state_for(&state) == M11_AX_STATE_INVENTORY,
                 "classifier: inventoryPanelActive -> INVENTORY");

    /* (c) mapOverlayActive -> map. */
    memset(&state, 0, sizeof(state));
    state.mapOverlayActive = 1;
    m11_ax_check(m11_screen_reader_state_for(&state) == M11_AX_STATE_MAP,
                 "classifier: mapOverlayActive -> MAP");

    /* (d) dialogOverlayActive -> dialog. */
    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    m11_ax_check(m11_screen_reader_state_for(&state) == M11_AX_STATE_DIALOG,
                 "classifier: dialogOverlayActive -> DIALOG");

    /* (e) candidateMirrorPanelActive -> entrance_mirror. */
    memset(&state, 0, sizeof(state));
    state.candidateMirrorPanelActive = 1;
    m11_ax_check(m11_screen_reader_state_for(&state) == M11_AX_STATE_ENTRANCE_MIRROR,
                 "classifier: candidateMirrorPanelActive -> ENTRANCE_MIRROR");

    /* (f) gameWon -> endgame (beats dialogOverlayActive when both set). */
    memset(&state, 0, sizeof(state));
    state.gameWon = 1;
    state.dialogOverlayActive = 1;
    m11_ax_check(m11_screen_reader_state_for(&state) == M11_AX_STATE_ENDGAME,
                 "classifier: gameWon -> ENDGAME (beats dialog)");

    /* (g) NULL state -> OTHER. */
    m11_ax_check(m11_screen_reader_state_for(NULL) == M11_AX_STATE_OTHER,
                 "classifier: NULL state -> OTHER");
    m11_ax_check(strcmp(m11_screen_reader_view_name(NULL), "other") == 0,
                 "classifier: NULL state view name -> \"other\"");
}

/* -- Subtest 4: gameplay manifest shape --------------------------- */

static void subtest_gameplay_manifest_shape(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.world.partyMapIndex = 7;  /* surface in gameplay_root value */

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "gameplay: update returns 1 when enabled");
    }
    m11_ax_check(m11_ax_file_exists(g_json_path),
                 "gameplay: accessibility.json is written");
    m11_ax_check(!m11_ax_file_exists(g_tmp_path),
                 "gameplay: no .tmp residue after flush");

    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "gameplay: file is non-empty");
    if (n <= 0) return;

    m11_ax_check(strstr(buf, "\"version\":1") != NULL,
                 "gameplay: version=1 present");
    m11_ax_check(strstr(buf, "\"app\":\"firestaff\"") != NULL,
                 "gameplay: app=firestaff present");
    m11_ax_check(strstr(buf, "\"gameState\":\"gameplay\"") != NULL,
                 "gameplay: gameState pinned to \"gameplay\"");
    m11_ax_check(strstr(buf, "\"framebuffer\":{\"width\":320,\"height\":200}") != NULL,
                 "gameplay: framebuffer dims present (320x200)");

    /* gameplay_root envelope carries state + active + level. */
    m11_ax_check(strstr(buf, "\"id\":\"gameplay_root\"") != NULL,
                 "gameplay: gameplay_root envelope present");
    m11_ax_check(strstr(buf, "state=gameplay") != NULL &&
                 strstr(buf, "active=1") != NULL &&
                 strstr(buf, "level=7") != NULL,
                 "gameplay: gameplay_root carries state/active/level");

    /* Always-on zones. */
    m11_ax_check(strstr(buf, "\"id\":\"VIEWPORT\"") != NULL,
                 "gameplay: VIEWPORT zone present");
    m11_ax_check(strstr(buf, "\"id\":\"MOVE_FWD\"") != NULL &&
                 strstr(buf, "\"id\":\"MOVE_BACK\"") != NULL &&
                 strstr(buf, "\"id\":\"TURN_LEFT\"") != NULL &&
                 strstr(buf, "\"id\":\"TURN_RIGHT\"") != NULL,
                 "gameplay: all four movement zones present");
    m11_ax_check(strstr(buf, "\"id\":\"SPELL_AREA\"") != NULL,
                 "gameplay: SPELL_AREA zone present");
    m11_ax_check(strstr(buf, "\"id\":\"HUD_PANEL\"") != NULL,
                 "gameplay: HUD_PANEL zone present");
    m11_ax_check(strstr(buf, "\"id\":\"CONTROL_STRIP\"") != NULL,
                 "gameplay: CONTROL_STRIP zone present");
}

/* -- Subtest 5: inventory manifest adds panel zones --------------- */

static void subtest_inventory_manifest_adds_panel_zones(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.world.party.activeChampionIndex = 0;

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "inventory: update returns 1");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "inventory: file is non-empty");
    if (n <= 0) return;

    m11_ax_check(strstr(buf, "\"gameState\":\"inventory\"") != NULL,
                 "inventory: gameState pinned to \"inventory\"");
    m11_ax_check(strstr(buf, "\"id\":\"INVENTORY_PANEL\"") != NULL,
                 "inventory: INVENTORY_PANEL region present");
    m11_ax_check(strstr(buf, "\"id\":\"INVENTORY_PORTRAIT\"") != NULL,
                 "inventory: champion portrait zone present");
    m11_ax_check(strstr(buf, "\"id\":\"INVENTORY_CHAMPION_NAME\"") != NULL,
                 "inventory: champion name zone present");
    /* Equipment slot IDs (C507..C519 -> 13 slots). */
    m11_ax_check(strstr(buf, "\"id\":\"INV_READY_HAND\"") != NULL &&
                 strstr(buf, "\"id\":\"INV_ACTION_HAND\"") != NULL &&
                 strstr(buf, "\"id\":\"INV_HEAD\"") != NULL &&
                 strstr(buf, "\"id\":\"INV_TORSO\"") != NULL &&
                 strstr(buf, "\"id\":\"INV_LEGS\"") != NULL &&
                 strstr(buf, "\"id\":\"INV_FEET\"") != NULL &&
                 strstr(buf, "\"id\":\"INV_NECK\"") != NULL &&
                 strstr(buf, "\"id\":\"INV_QUIVER_LINE1_1\"") != NULL,
                 "inventory: at least 8 equipment slot IDs present");
    /* Backpack grid (16 slots, C520..C536). */
    m11_ax_check(strstr(buf, "\"id\":\"INV_BACKPACK_0\"") != NULL &&
                 strstr(buf, "\"id\":\"INV_BACKPACK_15\"") != NULL,
                 "inventory: backpack slots 0..15 present");
    /* The always-on zones must still be present. */
    m11_ax_check(strstr(buf, "\"id\":\"VIEWPORT\"") != NULL &&
                 strstr(buf, "\"id\":\"HUD_PANEL\"") != NULL,
                 "inventory: still emits the always-on zones");
}

/* -- Subtest 6: map manifest -------------------------------------- */

static void subtest_map_manifest(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.mapOverlayActive = 1;

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "map: update returns 1");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "map: file is non-empty");
    if (n <= 0) return;

    m11_ax_check(strstr(buf, "\"gameState\":\"map\"") != NULL,
                 "map: gameState pinned to \"map\"");
    m11_ax_check(strstr(buf, "\"id\":\"AUTOMAP\"") != NULL,
                 "map: AUTOMAP full-screen region present");
}

/* -- Subtest 7: dialog manifest ----------------------------------- */

static void subtest_dialog_manifest(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.dialogOverlayActive = 1;
    snprintf(state.dialogOverlayText,
             sizeof(state.dialogOverlayText),
             "%s", "PRESS ENTER TO CONTINUE");
    state.dialogChoiceCount = 2;
    state.dialogSelectedChoice = 0;
    snprintf(state.dialogChoices[0],
             sizeof(state.dialogChoices[0]), "%s", "YES");
    snprintf(state.dialogChoices[1],
             sizeof(state.dialogChoices[1]), "%s", "NO");

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "dialog: update returns 1");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "dialog: file is non-empty");
    if (n <= 0) return;

    m11_ax_check(strstr(buf, "\"gameState\":\"dialog\"") != NULL,
                 "dialog: gameState pinned to \"dialog\"");
    m11_ax_check(strstr(buf, "\"id\":\"DIALOG_BODY\"") != NULL,
                 "dialog: DIALOG_BODY region present");
    m11_ax_check(strstr(buf, "PRESS ENTER TO CONTINUE") != NULL,
                 "dialog: dialogOverlayText is forwarded into the value field");
    m11_ax_check(strstr(buf, "\"id\":\"DIALOG_CHOICE_0\"") != NULL &&
                 strstr(buf, "\"id\":\"DIALOG_CHOICE_1\"") != NULL,
                 "dialog: DIALOG_CHOICE_0 and DIALOG_CHOICE_1 zones present");
    m11_ax_check(strstr(buf, "YES") != NULL &&
                 strstr(buf, "NO") != NULL,
                 "dialog: YES / NO choice labels forwarded into value fields");
}

/* -- Subtest 8: candidate mirror (entrance) manifest -------------- */

static void subtest_entrance_mirror_manifest(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.candidateMirrorPanelActive = 1;
    state.candidateMirrorOrdinal = 9;
    state.candidateMirrorPartyIndex = 4;

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "entrance: update returns 1");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "entrance: file is non-empty");
    if (n <= 0) return;

    m11_ax_check(strstr(buf, "\"gameState\":\"entrance_mirror\"") != NULL,
                 "entrance: gameState pinned to \"entrance_mirror\"");
    m11_ax_check(strstr(buf, "\"id\":\"CANDIDATE_MIRROR_PANEL\"") != NULL,
                 "entrance: CANDIDATE_MIRROR_PANEL region present");
    m11_ax_check(strstr(buf, "ordinal=9") != NULL &&
                 strstr(buf, "party_index=4") != NULL,
                 "entrance: candidate ordinal + party_index carried in value");
    m11_ax_check(strstr(buf, "\"id\":\"CANDIDATE_RESURRECT\"") != NULL,
                 "entrance: CANDIDATE_RESURRECT button present");
    m11_ax_check(strstr(buf, "\"id\":\"CANDIDATE_REINCARNATE\"") != NULL,
                 "entrance: CANDIDATE_REINCARNATE button present");
    m11_ax_check(strstr(buf, "\"id\":\"CANDIDATE_CANCEL\"") != NULL,
                 "entrance: CANDIDATE_CANCEL button present");
}

/* -- Subtest 9: endgame manifest ---------------------------------- */

static void subtest_endgame_manifest(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.gameWon = 1;

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "endgame: update returns 1");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "endgame: file is non-empty");
    if (n <= 0) return;

    m11_ax_check(strstr(buf, "\"gameState\":\"endgame\"") != NULL,
                 "endgame: gameState pinned to \"endgame\"");
    m11_ax_check(strstr(buf, "\"id\":\"ENDGAME_THE_END\"") != NULL,
                 "endgame: ENDGAME_THE_END plaque present");
    m11_ax_check(strstr(buf, "\"id\":\"ENDGAME_MIRROR_0\"") != NULL &&
                 strstr(buf, "\"id\":\"ENDGAME_MIRROR_3\"") != NULL,
                 "endgame: 4 champion mirror zones present (slots 0..3)");
    m11_ax_check(strstr(buf, "\"id\":\"ENDGAME_PORTRAIT_0\"") != NULL &&
                 strstr(buf, "\"id\":\"ENDGAME_PORTRAIT_3\"") != NULL,
                 "endgame: 4 champion portrait zones present (slots 0..3)");
}

/* -- Subtest 10: JSON well-formedness across all states ----------- */

static void subtest_json_well_formed_across_all_states(void) {
    static const struct {
        const char* name;
        int (*setup)(M11_GameViewState* out);
    } cases[] = {
        /* gameplay setup: zero state. */
        { "gameplay",        NULL },
        /* inventory / map / dialog / entrance / endgame: see below. */
        { "inventory",       NULL },
        { "map",             NULL },
        { "dialog",          NULL },
        { "entrance_mirror", NULL },
        { "endgame",         NULL }
    };
    int i;
    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        M11_GameViewState state;
        char buf[16384];
        int n;
        m11_ax_clean_artifacts();
        fs_ax_set_enabled(1);

        memset(&state, 0, sizeof(state));
        state.active = 1;
        if (strcmp(cases[i].name, "inventory") == 0) {
            state.inventoryPanelActive = 1;
        } else if (strcmp(cases[i].name, "map") == 0) {
            state.mapOverlayActive = 1;
        } else if (strcmp(cases[i].name, "dialog") == 0) {
            state.dialogOverlayActive = 1;
            state.dialogChoiceCount = 1;
            snprintf(state.dialogChoices[0],
                     sizeof(state.dialogChoices[0]),
                     "%s", "OK");
        } else if (strcmp(cases[i].name, "entrance_mirror") == 0) {
            state.candidateMirrorPanelActive = 1;
        } else if (strcmp(cases[i].name, "endgame") == 0) {
            state.gameWon = 1;
        }

        {
            int rc = m11_screen_reader_update_ex(&state, 320, 200);
            char msg[96];
            snprintf(msg, sizeof(msg),
                     "wf[%s]: update returns 1", cases[i].name);
            m11_ax_check(rc == 1, msg);
        }

        n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
        {
            char msg[96];
            snprintf(msg, sizeof(msg),
                     "wf[%s]: file is non-empty", cases[i].name);
            m11_ax_check(n > 0, msg);
        }
        if (n <= 0) continue;

        {
            char msg[96];
            snprintf(msg, sizeof(msg),
                     "wf[%s]: no leading-comma artifact",
                     cases[i].name);
            m11_ax_check(strstr(buf, "[,") == NULL, msg);
            snprintf(msg, sizeof(msg),
                     "wf[%s]: no trailing-comma artifact",
                     cases[i].name);
            m11_ax_check(strstr(buf, ",]") == NULL, msg);
            snprintf(msg, sizeof(msg),
                     "wf[%s]: no double-comma artifact",
                     cases[i].name);
            m11_ax_check(strstr(buf, ",,") == NULL, msg);
        }
    }
}

/* -- Subtest 11: NULL state is a no-op ---------------------------- */

static void subtest_null_state_is_noop(void) {
    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    {
        int rc = m11_screen_reader_update_ex(NULL, 320, 200);
        m11_ax_check(rc == 0,
                     "null-state: m11_screen_reader_update_ex(NULL) returns 0");
    }
    m11_ax_check(!m11_ax_file_exists(g_json_path),
                 "null-state: no JSON written");
    m11_ax_check(!m11_ax_file_exists(g_tmp_path),
                 "null-state: no .tmp residue");
}

/* -- main --------------------------------------------------------- */

int main(void) {
    if (!m11_ax_setup_temp_home()) {
        fprintf(stderr,
                "could not redirect HOME to a temp dir; "
                "all subtests skipped\n");
        printf("\n  m11_screen_reader_gameplay_state_manifest: "
               "%d passed, %d failed (isolation failed)\n",
               g_passes, g_failures);
        return g_failures == 0 ? 0 : 1;
    }

    printf("  [1] disabled state writes nothing...\n");
    subtest_disabled_state_is_noop();

    printf("  [2] state name mapping is stable...\n");
    subtest_state_name_mapping_is_stable();

    printf("  [3] state classifier is correct...\n");
    subtest_state_classifier();

    printf("  [4] gameplay manifest shape...\n");
    subtest_gameplay_manifest_shape();

    printf("  [5] inventory manifest adds panel zones...\n");
    subtest_inventory_manifest_adds_panel_zones();

    printf("  [6] map manifest...\n");
    subtest_map_manifest();

    printf("  [7] dialog manifest...\n");
    subtest_dialog_manifest();

    printf("  [8] candidate mirror (entrance) manifest...\n");
    subtest_entrance_mirror_manifest();

    printf("  [9] endgame manifest...\n");
    subtest_endgame_manifest();

    printf("  [10] JSON well-formedness across all states...\n");
    subtest_json_well_formed_across_all_states();

    printf("  [11] NULL state is a no-op...\n");
    subtest_null_state_is_noop();

    m11_ax_teardown_temp_home();

    printf("\n  m11_screen_reader_gameplay_state_manifest: "
           "%d passed, %d failed\n",
           g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
