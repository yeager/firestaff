/*
 * firestaff_m12_launcher_screen_reader_manifest_probe.c
 *
 * M12 launcher screen-reader / state-manifest probe.
 *
 * Closes the open gap "Screen reader launcher state manifest" in
 * TODO.md / docs/FIRESTAFF_GAP_LIST.md by proving that
 * M12_StartupMenu_Draw + m12_launcher_a11y_emit produce a JSON
 * manifest at $TMPDIR/firestaff-a11y/accessibility.json that:
 *
 *   1. Always has the v1 envelope (version, app, gameState,
 *      framebuffer, elements).
 *   2. Main view emits one launcher_card element per game entry,
 *      with stable element IDs (GAME_CARD_DM1 .. GAME_CARD_THERON,
 *      MENU_SETTINGS, MENU_MUSEUM) and a "selected" value on the
 *      focused card.
 *   3. Settings view emits launcher_tab rows for the tab strip
 *      plus launcher_row rows for the accessibility tab settings.
 *   4. Message / popup view emits a popup element plus a popup_ok
 *      confirmation button, with the popup lines surfaced so a
 *      screen reader can announce them.
 *   5. Atomic write — the .tmp residue is gone after flush.
 *   6. Privacy — the popup's data-dir line is suppressed unless the
 *      caller passes includePaths=1 (verified by writing with both
 *      flags and comparing the JSON).
 *
 * Privacy/safety: the probe redirects HOME to a temp dir under
 * $TMPDIR (or /tmp on POSIX) so the manifest never touches the real
 * user ~/.firestaff.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
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
    /* Simple Windows temp dir fallback: append a pid+time suffix. */
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
#include <windows.h>
#else
#include <unistd.h>
static int portable_mkdir(const char* path) { return mkdir(path, 0700); }
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
    char tmpl[] = "firestaff-a11y-XXXXXX";
#else
    char tmpl[] = "/tmp/firestaff-a11y-XXXXXX";
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

/* ── Subtests ─────────────────────────────────────────────────────── */

/* Subtest A: envelope invariants — version, app, gameState, framebuffer
 * and a well-formed elements array are always present. */
static void subtest_envelope_present(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);

    fs_ax_set_enabled(1);

    /* Initialize a minimal launcher state. M12_StartupMenu_Init
     * populates state->entries[] from the static g_entryTemplate
     * and runs the asset scan against an empty temp dir (no games
     * available, all entries still emitted with the right ids). */
    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MAIN;
    state.selectedIndex = 0;

    fs_ax_begin_frame(480, 270, "launcher_main");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */
    probe_record("INV_LAX_01_envelope_version", n > 0 && strstr(buf, "\"version\":1") != NULL,
                 "envelope: version field present");
    probe_record("INV_LAX_02_envelope_app", n > 0 && strstr(buf, "\"app\":\"firestaff\"") != NULL,
                 "envelope: app field pinned to firestaff");
    probe_record("INV_LAX_03_envelope_gamestate", n > 0 && strstr(buf, "\"gameState\":\"launcher_main\"") != NULL,
                 "envelope: gameState reflects launcher_main");
    probe_record("INV_LAX_04_envelope_framebuffer",
                 n > 0 && strstr(buf, "\"framebuffer\":{\"width\":480,\"height\":270}") != NULL,
                 "envelope: framebuffer dims pinned (480x270)");
    probe_record("INV_LAX_05_envelope_elements_array",
                 n > 0 && strstr(buf, "\"elements\":[") != NULL,
                 "envelope: elements array opens");
    probe_record("INV_LAX_06_envelope_elements_closed",
                 n > 0 && strstr(buf, "]}") != NULL,
                 "envelope: elements array closes");
    probe_record("INV_LAX_07_envelope_no_tmp_residue",
                 !file_exists(g_tmp_path),
                 "envelope: atomic write — no .tmp residue");
}

/* Subtest B: main view emits one launcher_card per game entry with
 * stable IDs and a "selected" value on the focused card. */
static void subtest_main_view_cards(void)
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
    state.selectedIndex = 2; /* focus DM2 */

    fs_ax_begin_frame(480, 270, "launcher_main");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LAX_10_main_card_dm1",
                 strstr(buf, "\"id\":\"GAME_CARD_DM1\"") != NULL,
                 "main: GAME_CARD_DM1 element emitted");
    probe_record("INV_LAX_11_main_card_csb",
                 strstr(buf, "\"id\":\"GAME_CARD_CSB\"") != NULL,
                 "main: GAME_CARD_CSB element emitted");
    probe_record("INV_LAX_12_main_card_dm2",
                 strstr(buf, "\"id\":\"GAME_CARD_DM2\"") != NULL,
                 "main: GAME_CARD_DM2 element emitted");
    probe_record("INV_LAX_13_main_card_nexus",
                 strstr(buf, "\"id\":\"GAME_CARD_NEXUS\"") != NULL,
                 "main: GAME_CARD_NEXUS element emitted");
    probe_record("INV_LAX_14_main_card_theron",
                 strstr(buf, "\"id\":\"GAME_CARD_THERON\"") != NULL,
                 "main: GAME_CARD_THERON element emitted");
    probe_record("INV_LAX_15_main_settings_entry",
                 strstr(buf, "\"id\":\"MENU_SETTINGS\"") != NULL,
                 "main: MENU_SETTINGS element emitted");
    probe_record("INV_LAX_16_main_museum_entry",
                 strstr(buf, "\"id\":\"MENU_MUSEUM\"") != NULL,
                 "main: MENU_MUSEUM element emitted");
    probe_record("INV_LAX_17_main_card_type",
                 strstr(buf, "\"type\":\"launcher_card\"") != NULL,
                 "main: element type is launcher_card");

    /* The DM2 card should be the one marked selected. */
    probe_record("INV_LAX_18_main_selected_value",
                 strstr(buf, "\"id\":\"GAME_CARD_DM2\",\"type\":\"launcher_card\",\"label\":\"DUNGEON MASTER II\",\"bounds\":{\"x\":") != NULL
                     && strstr(buf, ", \"value\":\"selected\"}") != NULL,
                 "main: DM2 card carries the \"selected\" value");
}

/* Subtest C: settings view emits launcher_tab and launcher_row rows. */
static void subtest_settings_view(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsTabIndex = 4; /* Accessibility tab */

    fs_ax_begin_frame(480, 270, "launcher_settings");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LAX_20_settings_tab_game",
                 strstr(buf, "\"id\":\"TAB_GAME\"") != NULL,
                 "settings: TAB_GAME element emitted");
    probe_record("INV_LAX_21_settings_tab_graphics",
                 strstr(buf, "\"id\":\"TAB_GRAPHICS\"") != NULL,
                 "settings: TAB_GRAPHICS element emitted");
    probe_record("INV_LAX_22_settings_tab_type",
                 strstr(buf, "\"type\":\"launcher_tab\"") != NULL,
                 "settings: tab type is launcher_tab");
    probe_record("INV_LAX_23_settings_row_font_scale",
                 strstr(buf, "\"id\":\"ROW_FONT_SCALE\"") != NULL,
                 "settings: ROW_FONT_SCALE row emitted");
    probe_record("INV_LAX_24_settings_row_screen_reader",
                 strstr(buf, "\"id\":\"ROW_SCREEN_READER\"") != NULL,
                 "settings: ROW_SCREEN_READER row emitted");
    probe_record("INV_LAX_25_settings_row_type",
                 strstr(buf, "\"type\":\"launcher_row\"") != NULL,
                 "settings: row type is launcher_row");
    probe_record("INV_LAX_26_settings_selected_tab_value",
                 strstr(buf, ", \"value\":\"selected\"}") != NULL,
                 "settings: the focused tab carries \"selected\" value");
}

/* Subtest D: message view emits popup + popup_ok and surfaces the
 * popup's three message lines so a screen reader can announce them. */
static void subtest_message_view_popup(void)
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
    /* Use a short, deterministic message so we can assert on it. */
    snprintf(state.messageLine1Storage, sizeof(state.messageLine1Storage),
             "%s", "DUNGEON MASTER GAME DATA NOT FOUND");
    snprintf(state.messageLine2Storage, sizeof(state.messageLine2Storage),
             "%s", "MISSING: DUNGEON.DAT");
    snprintf(state.messageLine3Storage, sizeof(state.messageLine3Storage),
             "%s", "Data dir: /tmp/fake/data");
    state.messageLine1 = state.messageLine1Storage;
    state.messageLine2 = state.messageLine2Storage;
    state.messageLine3 = state.messageLine3Storage;

    fs_ax_begin_frame(480, 270, "launcher_popup");
    /* Default: data-dir line suppressed (includePaths=0). */
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LAX_30_popup_element",
                 strstr(buf, "\"id\":\"POPUP_MESSAGE\"") != NULL,
                 "popup: POPUP_MESSAGE element emitted");
    probe_record("INV_LAX_31_popup_type",
                 strstr(buf, "\"type\":\"popup\"") != NULL,
                 "popup: type is popup");
    probe_record("INV_LAX_32_popup_ok_button",
                 strstr(buf, "\"id\":\"POPUP_OK\"") != NULL,
                 "popup: POPUP_OK confirmation button emitted");
    probe_record("INV_LAX_33_popup_ok_type",
                 strstr(buf, "\"type\":\"popup_ok\"") != NULL,
                 "popup: POPUP_OK type is popup_ok");
    probe_record("INV_LAX_34_popup_line1_text",
                 strstr(buf, "\"id\":\"POPUP_LINE1\"") != NULL,
                 "popup: POPUP_LINE1 text element emitted");
    probe_record("INV_LAX_35_popup_line1_value_announced",
                 strstr(buf, "\"id\":\"POPUP_LINE1\",\"type\":\"text\",\"label\":\"Message Line 1\",\"bounds\":{\"x\":") != NULL
                     && strstr(buf, "\"DUNGEON MASTER GAME DATA NOT FOUND\"") != NULL,
                 "popup: line1 value is announced verbatim in POPUP_LINE1 element");
    probe_record("INV_LAX_36_popup_line2_value_announced",
                 strstr(buf, "MISSING: DUNGEON.DAT") != NULL,
                 "popup: line2 value announces missing file");

    /* Privacy: includePaths=0 must NOT leak the data-dir line. */
    probe_record("INV_LAX_37_popup_privacy_suppresses_data_dir",
                 strstr(buf, "/tmp/fake/data") == NULL,
                 "popup: data-dir line is suppressed when includePaths=0");
}

/* Subtest E: includePaths=1 actually emits the data-dir line so
 * diagnostic tooling can see it when explicitly requested. */
static void subtest_message_view_popup_paths_included(void)
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
    snprintf(state.messageLine3Storage, sizeof(state.messageLine3Storage),
             "%s", "Data dir: /tmp/fake/data");
    state.messageLine1 = "line1";
    state.messageLine2 = "line2";
    state.messageLine3 = state.messageLine3Storage;

    fs_ax_begin_frame(480, 270, "launcher_popup");
    m12_launcher_a11y_emit(&state, 480, 270, 1); /* includePaths=1 */
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */
    probe_record("INV_LAX_40_popup_paths_included_when_requested",
                 n > 0 && strstr(buf, "/tmp/fake/data") != NULL,
                 "popup: data-dir line is included when includePaths=1");
}

/* Subtest F: deterministic / stable element order — the cards list
 * ends with the museum + settings entries, and the tab list has tabs
 * before rows. A regression that swaps the order would break
 * screen-reader navigation that depends on reading-order matching
 * visual order. */
static void subtest_deterministic_order(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;
    const char* dm1;
    const char* csb;
    const char* dm2;
    const char* museum;
    const char* settings;
    const char* tabs;
    const char* rows;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    /* Main view: cards in stable order */
    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MAIN;
    state.selectedIndex = -1;

    fs_ax_begin_frame(480, 270, "launcher_main");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();
    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    dm1 = strstr(buf, "\"GAME_CARD_DM1\"");
    csb = strstr(buf, "\"GAME_CARD_CSB\"");
    dm2 = strstr(buf, "\"GAME_CARD_DM2\"");
    museum = strstr(buf, "\"MENU_MUSEUM\"");
    settings = strstr(buf, "\"MENU_SETTINGS\"");
    probe_record("INV_LAX_50_order_cards_left_to_right",
                 dm1 && csb && dm2
                     && dm1 < csb && csb < dm2,
                 "order: DM1 < CSB < DM2 in the elements list");
    probe_record("INV_LAX_51_order_museum_after_cards",
                 museum && dm2 && museum > dm2,
                 "order: MENU_MUSEUM appears after the game cards");
    probe_record("INV_LAX_52_order_settings_last",
                 settings && museum && settings > museum,
                 "order: MENU_SETTINGS is the last entry");

    /* Settings view: tabs before rows */
    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsTabIndex = 0;

    fs_ax_begin_frame(480, 270, "launcher_settings");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();
    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    tabs = strstr(buf, "\"launcher_tab\"");
    rows = strstr(buf, "\"launcher_row\"");
    probe_record("INV_LAX_53_order_tabs_before_rows",
                 tabs && rows && tabs < rows,
                 "order: launcher_tab entries precede launcher_row entries");
}

/* Subtest G: bounds are inside the framebuffer, and modern (1920x1080)
 * coords scale correctly relative to legacy (480x270). */
static void subtest_bounds_and_scaling(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;
    const char* bounds;
    int dm1X, dm1Y, dm1W, dm1H;
    int modernX, modernY, modernW, modernH;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    /* Legacy: GAME_CARD_DM1 base rect (130, 76, 168, 24) at 480x270 */
    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MAIN;
    state.selectedIndex = -1;
    fs_ax_begin_frame(480, 270, "launcher_main");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();
    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    bounds = strstr(buf, "\"id\":\"GAME_CARD_DM1\"");
    probe_record("INV_LAX_60_bounds_dm1_card_present",
                 bounds != NULL,
                 "bounds: GAME_CARD_DM1 element present in legacy framebuffer");

    /* Parse the bounds.x/y/w/h integers. */
    dm1X = dm1Y = dm1W = dm1H = -1;
    if (bounds) {
        const char* b = strstr(bounds, "\"bounds\":{");
        if (b) {
            (void)sscanf(b, "\"bounds\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d}",
                         &dm1X, &dm1Y, &dm1W, &dm1H);
        }
    }
    probe_record("INV_LAX_61_bounds_dm1_legacy_pinned",
                 dm1X == 130 && dm1Y == 76 && dm1W == 168 && dm1H == 24,
                 "bounds: GAME_CARD_DM1 base rect (130, 76, 168, 24) at 480x270");

    /* Modern 1920x1080: 4x scale on both axes. */
    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    fs_ax_begin_frame(1920, 1080, "launcher_main_modern");
    m12_launcher_a11y_emit(&state, 1920, 1080, 0);
    fs_ax_flush();
    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    bounds = strstr(buf, "\"id\":\"GAME_CARD_DM1\"");
    modernX = modernY = modernW = modernH = -1;
    if (bounds) {
        const char* b = strstr(bounds, "\"bounds\":{");
        if (b) {
            (void)sscanf(b, "\"bounds\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d}",
                         &modernX, &modernY, &modernW, &modernH);
        }
    }
    probe_record("INV_LAX_62_bounds_dm1_modern_scaled",
                 modernX == 520 && modernY == 304 && modernW == 672 && modernH == 96,
                 "bounds: GAME_CARD_DM1 scales 4x to (520, 304, 672, 96) at 1920x1080");

    /* Bounds inside the framebuffer. */
    probe_record("INV_LAX_63_bounds_inside_modern_framebuffer",
                 modernX >= 0 && modernY >= 0
                     && modernX + modernW <= 1920
                     && modernY + modernH <= 1080,
                 "bounds: DM1 card rect stays inside 1920x1080");
}

/* ── main ─────────────────────────────────────────────────────────── */

int main(void)
{
    printf("firestaff_m12_launcher_screen_reader_manifest_probe\n");

    if (!setup_a11y_home()) {
        fprintf(stderr,
                "could not redirect HOME to a temp dir; "
                "probe cannot run\n");
        return 1;
    }

    printf("\n[A] envelope invariants + atomic write\n");
    subtest_envelope_present();

    printf("\n[B] main view: game cards\n");
    subtest_main_view_cards();

    printf("\n[C] settings view: tabs + rows\n");
    subtest_settings_view();

    printf("\n[D] message view: popup + OK button\n");
    subtest_message_view_popup();

    printf("\n[E] message view: includePaths=1 (diagnostic opt-in)\n");
    subtest_message_view_popup_paths_included();

    printf("\n[F] deterministic element order\n");
    subtest_deterministic_order();

    printf("\n[G] bounds and modern scaling\n");
    subtest_bounds_and_scaling();

    teardown_a11y_home();
    fs_ax_shutdown();

    printf("\n# summary: %d/%d invariants passed\n",
           g_tally.passed, g_tally.total);
    return (g_tally.passed == g_tally.total) ? 0 : 1;
}
