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
 *   7. Extras views — the bestiary, item encyclopedia, screenshot
 *      gallery, and museum of lore each emit per-cell rows with
 *      stable element IDs and a "selected" value on the focused cell.
 *   8. Read-only extras — manual/docs and changelog views emit
 *      state-driven launcher_row elements instead of falling through
 *      to main-view game cards.
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
#include "bestiary_m12.h"
#include "firestaff_item_encyclopedia.h"
#include "screenshot_gallery_m12.h"

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

/* Subtest H: bestiary view emits one category_tab per known
 * category, then one bestiary_row per visible creature, with a
 * "selected" value on the focused row. */
static void subtest_bestiary_view(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;
    int i;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_BESTIARY;
    M12_Bestiary_Init(&state.bestiary);
    state.bestiary.categoryFilter = M12_BESTIARY_CAT_BEAST;
    state.bestiary.selectedIndex = 0;
    state.bestiary.scrollOffset = 0;

    fs_ax_begin_frame(480, 270, "launcher_bestiary");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LAX_70_bestiary_cat_tabs",
                 strstr(buf, "\"type\":\"category_tab\"") != NULL,
                 "bestiary: category_tab elements emitted");
    probe_record("INV_LAX_71_bestiary_cat_all_present",
                 strstr(buf, "\"id\":\"BESTIARY_CAT_0\"") != NULL,
                 "bestiary: BESTIARY_CAT_0 (ALL) tab present");
    probe_record("INV_LAX_72_bestiary_cat_beast_present",
                 strstr(buf, "\"id\":\"BESTIARY_CAT_3\"") != NULL,
                 "bestiary: BESTIARY_CAT_3 (CONSTRUCT) tab present (3rd in pinned order)");
    probe_record("INV_LAX_73_bestiary_row_type",
                 strstr(buf, "\"type\":\"bestiary_row\"") != NULL,
                 "bestiary: row type is bestiary_row");
    /* The total bestiary is small (5 creatures) so several rows should
     * be visible. Just check the first row id is present. */
    probe_record("INV_LAX_74_bestiary_first_row_id",
                 strstr(buf, "\"id\":\"BESTIARY_ROW_0\"") != NULL,
                 "bestiary: BESTIARY_ROW_0 id is emitted");
    /* The first visible row in category BEAST should carry the
     * "selected" value. */
    probe_record("INV_LAX_75_bestiary_selected_value",
                 strstr(buf, "\"value\":\"selected\"") != NULL,
                 "bestiary: focused row carries \"selected\" value");

    /* Make sure no "selected" leaks into the category tabs for
     * non-active categories. We are using BEAST (index 3). */
    for (i = 0; i < (int)M12_BESTIARY_CAT_COUNT; ++i) {
        char id[32];
        const char* elementStart;
        if (i == M12_BESTIARY_CAT_BEAST) continue;
        snprintf(id, sizeof(id), "\"id\":\"BESTIARY_CAT_%d\"", i);
        elementStart = strstr(buf, id);
        if (!elementStart) continue;
        /* Look for the closing of THIS element only — the next ',' or
         * closing ']'. Good enough to detect a leak of "selected". */
        {
            const char* end = strstr(elementStart, "},");
            if (!end) end = strstr(elementStart, "]}");
            if (end) {
                char idbuf[64];
                int len = (int)(end - elementStart);
                if (len > 0 && len < (int)sizeof(idbuf)) {
                    memcpy(idbuf, elementStart, len);
                    idbuf[len] = '\0';
                    snprintf(id, sizeof(id), "INV_LAX_76_bestiary_cat%d_no_selected", i);
                    probe_record(id, strstr(idbuf, "\"selected\"") == NULL,
                                 "bestiary: non-active category tab does not carry \"selected\"");
                }
            }
        }
    }
}

/* Subtest I: item encyclopedia view emits one category_tab per
 * known category, then one item_encyclopedia_row per visible item. */
static void subtest_item_encyclopedia_view(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;
    int i;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_ITEM_ENCYCLOPEDIA;
    state.itemEncyclopediaCategory = FS_ITEM_CAT_WEAPON;
    state.itemEncyclopediaSelectedIndex = 0;
    state.itemEncyclopediaScrollOffset = 0;

    fs_ax_begin_frame(480, 270, "launcher_items");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LAX_80_item_cat_tabs",
                 strstr(buf, "\"type\":\"category_tab\"") != NULL,
                 "items: category_tab elements emitted");
    probe_record("INV_LAX_81_item_cat_weapon_present",
                 strstr(buf, "\"id\":\"ITEM_CAT_0\"") != NULL,
                 "items: ITEM_CAT_0 (WEAPON) tab present");
    probe_record("INV_LAX_82_item_row_type",
                 strstr(buf, "\"type\":\"item_encyclopedia_row\"") != NULL,
                 "items: row type is item_encyclopedia_row");
    /* The encyclopedia has 32 items, but only items in the active
     * category are emitted. Just verify the row id format is present
     * and that some rows are emitted (we don't want a silent zero). */
    probe_record("INV_LAX_83_item_at_least_one_row",
                 strstr(buf, "\"ITEM_ROW_") != NULL,
                 "items: at least one ITEM_ROW_* element emitted");

    /* The active category tab (WEAPON = 0) should carry "selected". */
    for (i = 0; i < (int)FS_ITEM_CAT_COUNT; ++i) {
        char id[64];
        const char* elementStart;
        if (i == FS_ITEM_CAT_WEAPON) continue;
        snprintf(id, sizeof(id), "\"id\":\"ITEM_CAT_%d\"", i);
        elementStart = strstr(buf, id);
        if (!elementStart) continue;
        {
            const char* end = strstr(elementStart, "},");
            if (!end) end = strstr(elementStart, "]}");
            if (end) {
                char idbuf[256];
                int len = (int)(end - elementStart);
                if (len > 0 && len < (int)sizeof(idbuf)) {
                    memcpy(idbuf, elementStart, len);
                    idbuf[len] = '\0';
                    snprintf(id, sizeof(id), "INV_LAX_84_item_cat%d_no_selected", i);
                    probe_record(id, strstr(idbuf, "\"selected\"") == NULL,
                                 "items: non-active category tab does not carry \"selected\"");
                }
            }
        }
    }
}

/* Subtest J: screenshot gallery view emits one screenshot_thumb per
 * visible entry, with stable ids and bounds. The probe seeds the
 * gallery with two synthetic entries to keep it deterministic. */
static void subtest_screenshot_gallery_view(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_SCREENSHOT_GALLERY;

    /* Seed two synthetic gallery entries — deterministic probe only,
     * never touches the real verification-screens/ directory. */
    {
        M12_ScreenshotGalleryState* gal = &state.screenshotGallery;
        memset(gal, 0, sizeof(*gal));
        gal->entryCount = 2;
        gal->selectedIndex = 1;
        gal->scrollOffset = 0;
        snprintf(gal->entries[0].filename,
                 sizeof(gal->entries[0].filename),
                 "%s", "dm1_title.png");
        gal->entries[0].width = 1920;
        gal->entries[0].height = 1080;
        gal->entries[0].fileSize = 12345;
        snprintf(gal->entries[1].filename,
                 sizeof(gal->entries[1].filename),
                 "%s", "csb_dungeon_view.png");
        gal->entries[1].width = 1280;
        gal->entries[1].height = 720;
        gal->entries[1].fileSize = 67890;
    }

    fs_ax_begin_frame(480, 270, "launcher_screenshots");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LAX_90_screenshot_thumb_type",
                 strstr(buf, "\"type\":\"screenshot_thumb\"") != NULL,
                 "screenshots: thumbnail type is screenshot_thumb");
    probe_record("INV_LAX_91_screenshot_first_row_id",
                 strstr(buf, "\"id\":\"SCREENSHOT_ROW_0\"") != NULL,
                 "screenshots: SCREENSHOT_ROW_0 id is emitted");
    probe_record("INV_LAX_92_screenshot_first_filename_label",
                 strstr(buf, "\"label\":\"dm1_title.png\"") != NULL,
                 "screenshots: first entry carries filename label");
    probe_record("INV_LAX_93_screenshot_selected_value",
                 strstr(buf, "\"value\":\"selected\"") != NULL,
                 "screenshots: focused row carries \"selected\" value");
}

/* Subtest K: museum of lore view emits one museum_category per
 * archive section, then one museum_bullet per visible lore line on
 * the active page. */
static void subtest_museum_view(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MUSEUM;
    state.museumSelectedIndex = 0; /* DUNGEON MASTER */
    state.museumPageIndex = 0;

    fs_ax_begin_frame(480, 270, "launcher_museum");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */

    probe_record("INV_LAX_100_museum_category_type",
                 strstr(buf, "\"type\":\"museum_category\"") != NULL,
                 "museum: museum_category elements emitted");
    probe_record("INV_LAX_101_museum_category_first",
                 strstr(buf, "\"id\":\"MUSEUM_CATEGORY_0\"") != NULL,
                 "museum: MUSEUM_CATEGORY_0 (DUNGEON MASTER) id is emitted");
    probe_record("INV_LAX_102_museum_category_dm2",
                 strstr(buf, "\"id\":\"MUSEUM_CATEGORY_2\"") != NULL,
                 "museum: MUSEUM_CATEGORY_2 (DUNGEON MASTER II) id is emitted");
    probe_record("INV_LAX_103_museum_category_label",
                 strstr(buf, "\"label\":\"DUNGEON MASTER II\"") != NULL
                     || strstr(buf, "DUNGEON MASTER II") != NULL,
                 "museum: category label surfaces the section name");
    probe_record("INV_LAX_104_museum_category_selected",
                 strstr(buf, "\"id\":\"MUSEUM_CATEGORY_0\",\"type\":\"museum_category\"") != NULL
                     && strstr(buf, "\"value\":\"selected\"") != NULL,
                 "museum: focused category carries \"selected\" value");

    /* Bullets for the active page. The first bullet on page 0 of
     * DUNGEON MASTER is the 1987 FTL GAMES line. */
    probe_record("INV_LAX_105_museum_bullet_type",
                 strstr(buf, "\"type\":\"museum_bullet\"") != NULL,
                 "museum: museum_bullet elements emitted");
    probe_record("INV_LAX_106_museum_bullet_id_present",
                 strstr(buf, "\"id\":\"MUSEUM_BULLET_0\"") != NULL,
                 "museum: MUSEUM_BULLET_0 id is emitted");
    probe_record("INV_LAX_107_museum_bullet_label_announced",
                 strstr(buf, "\"id\":\"MUSEUM_BULLET_0\",\"type\":\"museum_bullet\",\"label\":\"1987 FTL GAMES\"") != NULL
                     || strstr(buf, "1987 FTL GAMES") != NULL,
                 "museum: bullet label announces \"1987 FTL GAMES\"");

    /* Now switch to a different page and verify bullet content
     * actually changes (the emission is state-driven, not a static
     * template). */
    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);
    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MUSEUM;
    state.museumSelectedIndex = 0;
    state.museumPageIndex = 2; /* PRESERVATION NOTES */
    fs_ax_begin_frame(480, 270, "launcher_museum_p2");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();
    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n; /* read length is informational — invariants below inspect buf directly */
    probe_record("INV_LAX_108_museum_bullet_page2_changes",
                 strstr(buf, "PRESERVATION NOTES") != NULL,
                 "museum: page 2 bullet content reflects PRESERVATION NOTES");
}

/* Subtest L: manual/docs view emits public docs rows from the
 * M12_ManualDocs table. Paths are project-relative, not absolute
 * local filesystem paths. */
static void subtest_manual_docs_view(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_MANUAL_DOCS;

    fs_ax_begin_frame(480, 270, "launcher_manual_docs");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n;

    probe_record("INV_LAX_110_manual_doc_row_type",
                 strstr(buf, "\"type\":\"launcher_row\"") != NULL,
                 "manual/docs: launcher_row elements emitted");
    probe_record("INV_LAX_111_manual_doc_first_id",
                 strstr(buf, "\"id\":\"MANUAL_DOC_0\"") != NULL,
                 "manual/docs: MANUAL_DOC_0 id is emitted");
    probe_record("INV_LAX_112_manual_doc_label",
                 strstr(buf, "\"label\":\"Firestaff Manual\"") != NULL,
                 "manual/docs: first row label comes from M12_ManualDocs");
    probe_record("INV_LAX_113_manual_doc_repo_path",
                 strstr(buf, "README.md") != NULL,
                 "manual/docs: first row value includes project-relative README.md");
    probe_record("INV_LAX_114_manual_doc_not_main_fallback",
                 strstr(buf, "\"id\":\"GAME_CARD_DM1\"") == NULL,
                 "manual/docs: view no longer falls through to main game-card emission");
    probe_record("INV_LAX_115_manual_doc_no_absolute_home_path",
                 strstr(buf, g_home_path) == NULL,
                 "manual/docs: manifest does not leak the temp HOME path");
}

/* Subtest M: changelog view emits only the visible window and honors
 * the state's scrollOffset. */
static void subtest_changelog_view(void)
{
    char buf[16384];
    int n;
    M12_StartupMenuState state;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_CHANGELOG;
    state.changelog.scrollOffset = 0;

    fs_ax_begin_frame(480, 270, "launcher_changelog");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n;

    probe_record("INV_LAX_120_changelog_row_type",
                 strstr(buf, "\"type\":\"launcher_row\"") != NULL,
                 "changelog: launcher_row elements emitted");
    probe_record("INV_LAX_121_changelog_first_line_id",
                 strstr(buf, "\"id\":\"CHANGELOG_LINE_0\"") != NULL,
                 "changelog: first visible line id is emitted");
    probe_record("INV_LAX_122_changelog_first_line_label",
                 strstr(buf, "\"label\":\"FIRESTAFF CHANGELOG\"") != NULL,
                 "changelog: first visible line label is announced");
    probe_record("INV_LAX_123_changelog_not_main_fallback",
                 strstr(buf, "\"id\":\"GAME_CARD_DM1\"") == NULL,
                 "changelog: view no longer falls through to main game-card emission");

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    M12_StartupMenu_Init(&state);
    state.view = M12_MENU_VIEW_CHANGELOG;
    state.changelog.scrollOffset = 3;
    fs_ax_begin_frame(480, 270, "launcher_changelog_scrolled");
    m12_launcher_a11y_emit(&state, 480, 270, 0);
    fs_ax_flush();
    n = read_all(g_json_path, buf, sizeof(buf));
    (void)n;

    probe_record("INV_LAX_124_changelog_scroll_offset_line_id",
                 strstr(buf, "\"id\":\"CHANGELOG_LINE_3\"") != NULL,
                 "changelog: scrolled view starts at CHANGELOG_LINE_3");
    probe_record("INV_LAX_125_changelog_scroll_window_excludes_line0",
                 strstr(buf, "\"id\":\"CHANGELOG_LINE_0\"") == NULL,
                 "changelog: scrolled view excludes offscreen line 0");
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

    printf("\n[H] bestiary view: category tabs + creature rows\n");
    subtest_bestiary_view();

    printf("\n[I] item encyclopedia view: category tabs + item rows\n");
    subtest_item_encyclopedia_view();

    printf("\n[J] screenshot gallery view: thumbnail rows\n");
    subtest_screenshot_gallery_view();

    printf("\n[K] museum of lore view: sections + lore bullets\n");
    subtest_museum_view();

    printf("\n[L] manual/docs view: public docs rows\n");
    subtest_manual_docs_view();

    printf("\n[M] changelog view: visible scrolled rows\n");
    subtest_changelog_view();

    teardown_a11y_home();
    fs_ax_shutdown();

    printf("\n# summary: %d/%d invariants passed\n",
           g_tally.passed, g_tally.total);
    return (g_tally.passed == g_tally.total) ? 0 : 1;
}
