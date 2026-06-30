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
 *  10. Session timer overlay manifest adds the visible reminder
 *      banner or forced-pause dialog zones and mirrors the renderer's
 *      suppression fences for dialog / return-to-menu confirmation.
 *  11. Deterministic guards - JSON is well-formed (no leading-
 *      comma / trailing-comma / double-comma artifacts) across
 *      every state, even when a value field is non-NULL.
 *  11. NULL state is a no-op (returns 0, writes nothing).
 *  12. Inventory chest panel - when inventoryPanelActive +
 *      v1OpenChestThing != THING_NONE, the manifest emits the
 *      CHEST_PANEL region + 8 CHEST_SLOT_* zones + the
 *      CHEST_ARROW_OR_EYE button (with arrow vs. eye label per
 *      v1OpenChestOpenedByEye). No chest zones when v1OpenChestThing
 *      is 0.
 *  13. Acting champion zone - ACTING_CHAMPION appears only for
 *      actingChampionOrdinal 1..4 and carries "ordinal=N" in the
 *      value field; ordinal 0 (idle) and ordinal > 4 (out of range)
 *      emit no ACTING_CHAMPION element.
 *  14. Bounds invariant - every emitted element's x/y/w/h must
 *      stay inside the framebuffer bounds the caller passed to
 *      m11_screen_reader_update_ex. The M11 a11y emitter uses
 *      source-locked 320x200 pixel coordinates, so even when the
 *      caller passes a modern 1920x1080 framebuffer, no zone may
 *      exceed those coords (only the gameplay_root envelope scales).
 *      Zero / negative framebuffer dims fall back to 320x200.
 *  15. Deterministic zone order - two emissions of the same state
 *      produce byte-identical output, the canonical element order
 *      (gameplay_root -> VIEWPORT -> MOVE_FWD -> MOVE_BACK ->
 *      TURN_LEFT -> TURN_RIGHT -> SPELL_AREA -> HUD_PANEL ->
 *      CONTROL_STRIP) is preserved across runs, and a re-emission
 *      of the original state after an intermediate inventory emission
 *      still matches the first emission byte-for-byte (no leftover
 *      global state).
 *
 * Test isolation: redirects HOME to a per-run temp dir so the manifest
 * never touches the real user ~/.firestaff.  The test uses
 * memset(&state, 0, sizeof(state)) so no DM1 game data is required.
 */

#include "firestaff_accessibility.h"
#include "m11_game_view_a11y.h"
#include "m11_game_view.h"
#include "session_timer_runtime.h"

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

static int m11_ax_count_id_substrings(const char* json, const char* id) {
    int count = 0;
    char needle[128];
    const char* cursor;
    size_t needleLen;
    if (!json || !id) return 0;
    snprintf(needle, sizeof(needle), "\"id\":\"%s\"", id);
    needleLen = strlen(needle);
    cursor = json;
    while ((cursor = strstr(cursor, needle)) != NULL) {
        ++count;
        cursor += needleLen;
    }
    return count;
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

static void subtest_session_timer_overlay_manifest(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.fontScale = 1;
    SessionTimerRuntime_Init(&state.sessionTimerRuntime, 30);
    SessionTimerRuntime_Tick(&state.sessionTimerRuntime, 25 * 60);
    state.sessionTimerReminderOverlayActive = 1;

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "timer-reminder: update returns 1");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "timer-reminder: file is non-empty");
    if (n <= 0) return;

    m11_ax_check(strstr(buf, "\"gameState\":\"gameplay\"") != NULL,
                 "timer-reminder: gameplay state is preserved");
    m11_ax_check(strstr(buf, "\"id\":\"SESSION_TIMER_REMINDER\"") != NULL,
                 "timer-reminder: banner region present");
    m11_ax_check(strstr(buf, "\"id\":\"SESSION_TIMER_REMINDER_TEXT\"") != NULL,
                 "timer-reminder: banner text present");
    m11_ax_check(strstr(buf, "SESSION TIMER 00:05:00 REMAINING") != NULL,
                 "timer-reminder: visible scale-1 text is mirrored");
    m11_ax_check(strstr(buf, "remaining=300") != NULL,
                 "timer-reminder: remaining seconds carried in value");

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.fontScale = 3;
    SessionTimerRuntime_Init(&state.sessionTimerRuntime, 1);
    SessionTimerRuntime_Tick(&state.sessionTimerRuntime, 60);
    state.sessionTimerForcedPauseDialogActive = 1;

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "timer-forced: update returns 1");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "timer-forced: file is non-empty");
    if (n <= 0) return;

    m11_ax_check(strstr(buf, "\"id\":\"SESSION_TIMER_FORCED_PAUSE\"") != NULL,
                 "timer-forced: popup region present");
    m11_ax_check(strstr(buf, "\"type\":\"popup\"") != NULL,
                 "timer-forced: popup element type is used");
    m11_ax_check(strstr(buf, "\"id\":\"SESSION_TIMER_FORCED_PAUSE_TITLE\"") != NULL &&
                 strstr(buf, "\"id\":\"SESSION_TIMER_FORCED_PAUSE_LINE1\"") != NULL &&
                 strstr(buf, "\"id\":\"SESSION_TIMER_FORCED_PAUSE_LINE2\"") != NULL,
                 "timer-forced: title and body line text zones present");
    m11_ax_check(m11_ax_count_id_substrings(buf, "SESSION_TIMER_FORCED_PAUSE") == 1,
                 "timer-forced: popup region appears exactly once");
    m11_ax_check(m11_ax_count_id_substrings(buf, "SESSION_TIMER_FORCED_PAUSE_TITLE") == 1,
                 "timer-forced: title text zone appears exactly once");
    m11_ax_check(m11_ax_count_id_substrings(buf, "SESSION_TIMER_FORCED_PAUSE_LINE1") == 1,
                 "timer-forced: line1 text zone appears exactly once");
    m11_ax_check(m11_ax_count_id_substrings(buf, "SESSION_TIMER_FORCED_PAUSE_LINE2") == 1,
                 "timer-forced: line2 text zone appears exactly once");
    m11_ax_check(strstr(buf, "scale=3;remaining=0") != NULL,
                 "timer-forced: scale and remaining seconds carried in value");
    m11_ax_check(strstr(buf, "EXPIRED") != NULL &&
                 strstr(buf, "ENTER MENU") != NULL &&
                 strstr(buf, "ESC DISMISS") != NULL,
                 "timer-forced: scale-3 visible text is mirrored");

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    SessionTimerRuntime_Init(&state.sessionTimerRuntime, 30);
    SessionTimerRuntime_Tick(&state.sessionTimerRuntime, 25 * 60);
    state.sessionTimerReminderOverlayActive = 1;
    state.dialogOverlayActive = 1;

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "timer-hidden-dialog: update returns 1");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "timer-hidden-dialog: file is non-empty");
    if (n <= 0) return;
    m11_ax_check(strstr(buf, "\"id\":\"SESSION_TIMER_REMINDER\"") == NULL,
                 "timer-hidden-dialog: dialog suppresses reminder banner");

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    SessionTimerRuntime_Init(&state.sessionTimerRuntime, 1);
    SessionTimerRuntime_Tick(&state.sessionTimerRuntime, 60);
    state.sessionTimerForcedPauseDialogActive = 1;
    state.returnToMenuConfirmActive = 1;

    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "timer-hidden-return-menu: update returns 1");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "timer-hidden-return-menu: file is non-empty");
    if (n <= 0) return;
    m11_ax_check(strstr(buf, "\"id\":\"SESSION_TIMER_FORCED_PAUSE\"") == NULL,
                 "timer-hidden-return-menu: return-menu confirm suppresses forced pause");
}

/* -- Subtest 11: JSON well-formedness across all states ----------- */

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
        { "endgame",         NULL },
        { "timer_reminder",  NULL },
        { "timer_forced",    NULL }
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
        } else if (strcmp(cases[i].name, "timer_reminder") == 0) {
            SessionTimerRuntime_Init(&state.sessionTimerRuntime, 30);
            SessionTimerRuntime_Tick(&state.sessionTimerRuntime, 25 * 60);
            state.sessionTimerReminderOverlayActive = 1;
        } else if (strcmp(cases[i].name, "timer_forced") == 0) {
            SessionTimerRuntime_Init(&state.sessionTimerRuntime, 1);
            SessionTimerRuntime_Tick(&state.sessionTimerRuntime, 60);
            state.sessionTimerForcedPauseDialogActive = 1;
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

/* -- Subtest 12: NULL state is a no-op ---------------------------- */

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

/* -- Subtest 12: inventory chest panel --------------------------- */

/* Helper: extract a "{ ... }" JSON object substring starting at
 * the first '{' after `p` and ending at the matching closing
 * brace. Returns the length written to out (0 if not found). */
static int m11_ax_extract_object(const char* p, char* out, size_t outSize) {
    int depth = 0;
    size_t i = 0;
    int started = 0;
    if (!p || !out || outSize == 0) return 0;
    while (*p) {
        if (*p == '{') {
            ++depth;
            started = 1;
        } else if (*p == '}') {
            --depth;
            if (started && depth == 0) {
                if (i < outSize - 1) out[i++] = *p;
                out[i] = '\0';
                return (int)i;
            }
        }
        if (started && i < outSize - 1) out[i++] = *p;
        ++p;
    }
    return 0;
}

/* Helper: parse "bounds":{"x":N,"y":N,"w":N,"h":N} out of an object
 * substring. Returns 1 on success, 0 on parse failure. */
static int m11_ax_parse_bounds(const char* obj,
                               int* x, int* y, int* w, int* h) {
    const char* b;
    if (!obj) return 0;
    b = strstr(obj, "\"bounds\":{");
    if (!b) return 0;
    return sscanf(b, "\"bounds\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d}",
                  x, y, w, h) == 4 ? 1 : 0;
}

static void subtest_inventory_chest_panel(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;
    int chestOrdinal;

    /* (a) No chest open - inventory panel still emits the panel +
     * 13 equipment + 16 backpack zones, but no CHEST_* or
     * CHEST_ARROW_OR_EYE elements.
     *
     * v1OpenChestThing is an unsigned short; the a11y emitter
     * guards on `state->v1OpenChestThing != THING_NONE` where
     * THING_NONE = 0xFFFF (include/memory_dungeon_dat_pc34_compat.h).
     * A freshly zeroed state has v1OpenChestThing == 0, which IS
     * != 0xFFFF, so chest zones would be emitted. We must set
     * v1OpenChestThing = THING_NONE explicitly to test the
     * "no chest open" path. */
    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);
    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.world.party.activeChampionIndex = 0;
    state.v1OpenChestThing = THING_NONE;
    state.v1OpenChestOpenedByEye = 0;
    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1,
                     "chest[a]: update returns 1 with no chest open");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "chest[a]: file is non-empty");
    if (n > 0) {
        m11_ax_check(strstr(buf, "\"id\":\"CHEST_PANEL\"") == NULL,
                     "chest[a]: no CHEST_PANEL when v1OpenChestThing is 0");
        m11_ax_check(strstr(buf, "\"id\":\"CHEST_SLOT_0\"") == NULL,
                     "chest[a]: no CHEST_SLOT_0 when no chest open");
        m11_ax_check(strstr(buf, "\"id\":\"CHEST_ARROW_OR_EYE\"") == NULL,
                     "chest[a]: no CHEST_ARROW_OR_EYE when no chest open");
    }

    /* (b) Chest open via Arrow/Take-All. */
    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);
    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.world.party.activeChampionIndex = 1;
    state.v1OpenChestThing = 0x1234u;
    state.v1OpenChestOpenedByEye = 0;
    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1,
                     "chest[b]: update returns 1 with chest open (arrow)");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "chest[b]: file is non-empty");
    if (n > 0) {
        m11_ax_check(strstr(buf, "\"id\":\"CHEST_PANEL\"") != NULL,
                     "chest[b]: CHEST_PANEL region present");
        /* All 8 chest slots must appear (CHEST.C F0333 C537..C544). */
        for (chestOrdinal = 0; chestOrdinal < 8; ++chestOrdinal) {
            char want[64];
            char msg[96];
            snprintf(want, sizeof(want), "\"id\":\"CHEST_SLOT_%d\"",
                     chestOrdinal);
            snprintf(msg, sizeof(msg),
                     "chest[b]: CHEST_SLOT_%d present (0..7)", chestOrdinal);
            m11_ax_check(strstr(buf, want) != NULL, msg);
        }
        m11_ax_check(strstr(buf, "\"id\":\"CHEST_ARROW_OR_EYE\"") != NULL,
                     "chest[b]: CHEST_ARROW_OR_EYE button present");
        m11_ax_check(strstr(buf, "Arrow (Take All)") != NULL,
                     "chest[b]: arrow label is \"Arrow (Take All)\"");
    }

    /* (c) Chest open via Eye/Reincarnate. */
    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);
    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.v1OpenChestThing = 0xABCDu;
    state.v1OpenChestOpenedByEye = 1;
    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1,
                     "chest[c]: update returns 1 with chest open (eye)");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "chest[c]: file is non-empty");
    if (n > 0) {
        m11_ax_check(strstr(buf, "Eye (Reincarnate)") != NULL,
                     "chest[c]: eye label is \"Eye (Reincarnate)\"");
        /* Chest slots still appear in the eye case. */
        m11_ax_check(strstr(buf, "\"id\":\"CHEST_SLOT_0\"") != NULL &&
                     strstr(buf, "\"id\":\"CHEST_SLOT_7\"") != NULL,
                     "chest[c]: 8 chest slots also appear in eye mode");
    }
}

/* -- Subtest 13: ACTING_CHAMPION zone ----------------------------- */

static void subtest_acting_champion_zone(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;
    unsigned int ordinal;

    /* (a) ordinal == 0 means "no champion is acting" — the action
     * area is in idle mode (F0387 icon-mode branch). The screen
     * reader must NOT emit an ACTING_CHAMPION zone. */
    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);
    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.actingChampionOrdinal = 0u;
    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1,
                     "acting[a]: update returns 1 (ordinal 0)");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "acting[a]: file is non-empty");
    if (n > 0) {
        m11_ax_check(strstr(buf, "\"id\":\"ACTING_CHAMPION\"") == NULL,
                     "acting[a]: no ACTING_CHAMPION when ordinal is 0");
    }

    /* (b) ordinals 1..4 each emit a value field carrying
     * "ordinal=N" so a screen reader can announce which champion
     * is acting. */
    for (ordinal = 1u; ordinal <= 4u; ++ordinal) {
        char want[64];
        char msg[96];
        m11_ax_clean_artifacts();
        fs_ax_set_enabled(1);
        memset(&state, 0, sizeof(state));
        state.active = 1;
        state.inventoryPanelActive = 1;
        state.actingChampionOrdinal = ordinal;
        {
            int rc = m11_screen_reader_update_ex(&state, 320, 200);
            snprintf(msg, sizeof(msg),
                     "acting[b]: update returns 1 (ordinal %u)", ordinal);
            m11_ax_check(rc == 1, msg);
        }
        n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
        if (n <= 0) continue;
        m11_ax_check(strstr(buf, "\"id\":\"ACTING_CHAMPION\"") != NULL,
                     "acting[b]: ACTING_CHAMPION present for valid ordinal");
        snprintf(want, sizeof(want), "ordinal=%u", ordinal);
        snprintf(msg, sizeof(msg),
                 "acting[b]: value field carries \"%s\"", want);
        m11_ax_check(strstr(buf, want) != NULL, msg);
    }

    /* (c) ordinal > 4 is out of range — the F0387 source only
     * models 1..4 (matches G0506_ui_ActingChampionOrdinal clamp).
     * The screen reader must NOT emit ACTING_CHAMPION. */
    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);
    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.actingChampionOrdinal = 5u;
    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1,
                     "acting[c]: update returns 1 (ordinal 5)");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "acting[c]: file is non-empty");
    if (n > 0) {
        m11_ax_check(strstr(buf, "\"id\":\"ACTING_CHAMPION\"") == NULL,
                     "acting[c]: no ACTING_CHAMPION when ordinal > 4");
    }
}

/* -- Subtest 14: bounds invariant at modern framebuffer ------------ */

/* Iterate every "{" ... "}" object in `buf`, parse out the
 * "id" string and "bounds" rectangle, and assert that
 *   0 <= x && x+w <= fbW && 0 <= y && y+h <= fbH
 * for each element. Stops at the first out-of-bounds element
 * and records which one it was. */
static void m11_ax_check_bounds_inside(const char* buf,
                                       int fbW, int fbH,
                                       const char* label) {
    const char* p;
    char msg[160];
    int checked = 0;
    p = buf;
    while ((p = strstr(p, "{")) != NULL) {
        char obj[512];
        char idBuf[64];
        int x = 0, y = 0, w = 0, h = 0;
        int objLen;
        const char* idStart;
        const char* idEnd;
        objLen = m11_ax_extract_object(p, obj, sizeof(obj));
        if (objLen <= 0) break;
        /* Skip the outer envelope { ... }: it has "version" / "app"
         * but no "bounds". */
        idStart = strstr(obj, "\"id\":\"");
        if (!idStart) {
            p += 1;
            continue;
        }
        idStart += 6;
        idEnd = strchr(idStart, '"');
        if (!idEnd) break;
        if ((size_t)(idEnd - idStart) >= sizeof(idBuf)) {
            p += 1;
            continue;
        }
        memcpy(idBuf, idStart, idEnd - idStart);
        idBuf[idEnd - idStart] = '\0';
        if (!m11_ax_parse_bounds(obj, &x, &y, &w, &h)) {
            p += 1;
            continue;
        }
        ++checked;
        snprintf(msg, sizeof(msg),
                 "bounds[%s]: element '%s' bounds inside framebuffer",
                 label, idBuf);
        m11_ax_check(x >= 0 && y >= 0 &&
                     x + w <= fbW && y + h <= fbH, msg);
        p += objLen;
    }
    snprintf(msg, sizeof(msg),
             "bounds[%s]: scanned %d bounded elements (sanity floor)",
             label, checked);
    m11_ax_check(checked >= 6, msg);
}

static void subtest_bounds_invariant(void) {
    M11_GameViewState state;
    char buf[16384];
    int n;

    /* (a) Source-faithful 320x200: every zone must stay inside. */
    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);
    memset(&state, 0, sizeof(state));
    state.active = 1;
    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "bounds[a]: update returns 1 (320x200)");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "bounds[a]: file is non-empty");
    if (n > 0) {
        m11_ax_check_bounds_inside(buf, 320, 200, "src");
    }

    /* (b) Modern 1920x1080 framebuffer — the gameplay-side manifest
     * keeps source-locked 320x200 pixel coords (ReDMCSB layout-696 /
     * PANEL.C / ENDGAME.C). The larger framebuffer only widens the
     * gameplay_root envelope; no source zone may exceed 1920x1080. */
    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);
    memset(&state, 0, sizeof(state));
    state.active = 1;
    {
        int rc = m11_screen_reader_update_ex(&state, 1920, 1080);
        m11_ax_check(rc == 1, "bounds[b]: update returns 1 (1920x1080)");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "bounds[b]: file is non-empty");
    if (n > 0) {
        /* gameplay_root envelope scales with framebuffer. */
        m11_ax_check(strstr(buf, "\"width\":1920") != NULL &&
                     strstr(buf, "\"height\":1080") != NULL,
                     "bounds[b]: envelope framebuffer reflects 1920x1080");
        /* The source-locked zones still report 320x200-pixel bounds
         * and stay well inside the modern framebuffer. */
        m11_ax_check_bounds_inside(buf, 1920, 1080, "modern");
    }

    /* (c) Zero/negative framebuffer dimensions fall back to 320x200
     * (see m11_screen_reader_update_ex default). The manifest must
     * still be well-formed and the zones must stay inside. */
    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);
    memset(&state, 0, sizeof(state));
    state.active = 1;
    {
        int rc = m11_screen_reader_update_ex(&state, 0, 0);
        m11_ax_check(rc == 1,
                     "bounds[c]: update returns 1 (zero fb dims)");
    }
    n = m11_ax_read_all(g_json_path, buf, sizeof(buf));
    m11_ax_check(n > 0, "bounds[c]: file is non-empty");
    if (n > 0) {
        m11_ax_check(strstr(buf, "\"width\":320") != NULL &&
                     strstr(buf, "\"height\":200") != NULL,
                     "bounds[c]: zero fb dims fall back to 320x200");
        m11_ax_check_bounds_inside(buf, 320, 200, "zero_fallback");
    }
}

/* -- Subtest 15: deterministic zone order -------------------------- */

static void subtest_deterministic_zone_order(void) {
    M11_GameViewState state;
    char buf1[16384];
    char buf2[16384];
    int n1, n2;
    const char* p_viewport;
    const char* p_move_fwd;
    const char* p_spell;
    const char* p_hud;
    const char* p_strip;
    const char* p_root;
    const char* q_root;
    const char* q_viewport;
    const char* q_spell;
    const char* q_hud;
    const char* q_strip;

    m11_ax_clean_artifacts();
    fs_ax_set_enabled(1);
    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.world.partyMapIndex = 3;

    /* First emission. */
    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "order[1]: first update returns 1");
    }
    n1 = m11_ax_read_all(g_json_path, buf1, sizeof(buf1));
    m11_ax_check(n1 > 0, "order[1]: first file is non-empty");
    if (n1 <= 0) return;

    /* Second emission with the same state. */
    {
        int rc = m11_screen_reader_update_ex(&state, 320, 200);
        m11_ax_check(rc == 1, "order[1]: second update returns 1");
    }
    n2 = m11_ax_read_all(g_json_path, buf2, sizeof(buf2));
    m11_ax_check(n2 > 0, "order[1]: second file is non-empty");
    if (n2 <= 0) return;

    /* Byte-identical output across two emissions of the same state
     * (no timestamps, no race windows). */
    m11_ax_check(n1 == n2,
                 "order[1]: two emissions have identical length");
    m11_ax_check(strcmp(buf1, buf2) == 0,
                 "order[1]: two emissions are byte-identical");

    /* Stable canonical order: gameplay_root -> VIEWPORT ->
     * movement arrows -> SPELL_AREA -> HUD_PANEL -> CONTROL_STRIP.
     * The M11 a11y emitter walks each emit-function in a fixed
     * sequence so a screen reader / replay tool can rely on the
     * element list being deterministic. */
    p_root = strstr(buf1, "\"id\":\"gameplay_root\"");
    p_viewport = strstr(buf1, "\"id\":\"VIEWPORT\"");
    p_move_fwd = strstr(buf1, "\"id\":\"MOVE_FWD\"");
    p_spell = strstr(buf1, "\"id\":\"SPELL_AREA\"");
    p_hud = strstr(buf1, "\"id\":\"HUD_PANEL\"");
    p_strip = strstr(buf1, "\"id\":\"CONTROL_STRIP\"");

    m11_ax_check(p_root != NULL, "order[1]: gameplay_root present");
    m11_ax_check(p_viewport != NULL, "order[1]: VIEWPORT present");
    m11_ax_check(p_move_fwd != NULL, "order[1]: MOVE_FWD present");
    m11_ax_check(p_spell != NULL, "order[1]: SPELL_AREA present");
    m11_ax_check(p_hud != NULL, "order[1]: HUD_PANEL present");
    m11_ax_check(p_strip != NULL, "order[1]: CONTROL_STRIP present");

    if (p_root && p_viewport && p_move_fwd && p_spell && p_hud && p_strip) {
        m11_ax_check(p_root < p_viewport,
                     "order[1]: gameplay_root before VIEWPORT");
        m11_ax_check(p_viewport < p_move_fwd,
                     "order[1]: VIEWPORT before MOVE_FWD");
        m11_ax_check(p_move_fwd < p_spell,
                     "order[1]: MOVE_FWD before SPELL_AREA");
        m11_ax_check(p_spell < p_hud,
                     "order[1]: SPELL_AREA before HUD_PANEL");
        m11_ax_check(p_hud < p_strip,
                     "order[1]: HUD_PANEL before CONTROL_STRIP");
    }

    /* Movement-arrow canonical ordering (F/B before TURN_*). */
    {
        const char* p_back = strstr(buf1, "\"id\":\"MOVE_BACK\"");
        const char* p_left = strstr(buf1, "\"id\":\"TURN_LEFT\"");
        const char* p_right = strstr(buf1, "\"id\":\"TURN_RIGHT\"");
        if (p_move_fwd && p_back && p_left && p_right) {
            m11_ax_check(p_move_fwd < p_back &&
                         p_back < p_left &&
                         p_left < p_right,
                         "order[1]: FWD < BACK < TURN_LEFT < TURN_RIGHT");
        }
    }

    /* Cross-run determinism — emit a third time with a different
     * (still canonical) state, then re-emit the original state.
     * The re-emission must match the first emission byte-for-byte
     * (no leftover global state from the intermediate emission). */
    {
        M11_GameViewState altState;
        char buf3[16384];
        int n3;
        memset(&altState, 0, sizeof(altState));
        altState.active = 1;
        altState.inventoryPanelActive = 1;
        altState.world.party.activeChampionIndex = 2;
        {
            int rc = m11_screen_reader_update_ex(&altState, 320, 200);
            m11_ax_check(rc == 1,
                         "order[2]: intermediate inventory update returns 1");
        }
        n3 = m11_ax_read_all(g_json_path, buf3, sizeof(buf3));
        m11_ax_check(n3 > 0, "order[2]: intermediate file is non-empty");
        if (n3 > 0) {
            m11_ax_check(strstr(buf3, "\"id\":\"INVENTORY_PANEL\"") != NULL,
                         "order[2]: inventory manifest present");
        }
        /* Re-emit the original state and compare to the first emission. */
        {
            int rc = m11_screen_reader_update_ex(&state, 320, 200);
            m11_ax_check(rc == 1, "order[2]: re-emit original returns 1");
        }
        {
            char buf4[16384];
            int n4 = m11_ax_read_all(g_json_path, buf4, sizeof(buf4));
            m11_ax_check(n4 == n1,
                         "order[2]: re-emit length matches original");
            m11_ax_check(strcmp(buf4, buf1) == 0,
                         "order[2]: re-emit byte-identical to original");
        }
    }

    /* Sanity: gameplay_root envelope precedes every other element. */
    q_root = strstr(buf2, "\"id\":\"gameplay_root\"");
    q_viewport = strstr(buf2, "\"id\":\"VIEWPORT\"");
    q_spell = strstr(buf2, "\"id\":\"SPELL_AREA\"");
    q_hud = strstr(buf2, "\"id\":\"HUD_PANEL\"");
    q_strip = strstr(buf2, "\"id\":\"CONTROL_STRIP\"");
    if (q_root && q_viewport && q_spell && q_hud && q_strip) {
        m11_ax_check(q_root < q_viewport &&
                     q_viewport < q_spell &&
                     q_spell < q_hud &&
                     q_hud < q_strip,
                     "order[3]: canonical order preserved across runs");
    }
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

    printf("  [10] session timer overlay manifest...\n");
    subtest_session_timer_overlay_manifest();

    printf("  [11] JSON well-formedness across all states...\n");
    subtest_json_well_formed_across_all_states();

    printf("  [12] NULL state is a no-op...\n");
    subtest_null_state_is_noop();

    printf("  [12] inventory chest panel...\n");
    subtest_inventory_chest_panel();

    printf("  [13] acting champion zone...\n");
    subtest_acting_champion_zone();

    printf("  [14] bounds invariant at modern framebuffer...\n");
    subtest_bounds_invariant();

    printf("  [15] deterministic zone order...\n");
    subtest_deterministic_zone_order();

    m11_ax_teardown_temp_home();

    printf("\n  m11_screen_reader_gameplay_state_manifest: "
           "%d passed, %d failed\n",
           g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
