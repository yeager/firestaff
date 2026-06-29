/*
 * test_firestaff_accessibility_manifest.c
 *   Narrow regression for the Firestaff accessibility JSON manifest writer
 *   (src/engine/firestaff_accessibility.c).
 *
 * Locks in behavior that external automation (Peekaboo, screen readers) depends
 * on. Test isolation: HOME is redirected to a per-run temp dir so the manifest
 * never touches the real user ~/.firestaff. Each subtest enables/disables the
 * writer through the public fs_ax_* API and inspects the resulting file.
 *
 * Subtests:
 *   1. Disabled state writes nothing (no JSON, no .tmp).
 *   2. Empty manifest field presence — version, app, gameState, framebuffer,
 *      and a well-formed empty elements list (no "[,", no leading or trailing
 *      comma, no malformed bracket).
 *   3. Atomic write — after flush only the final file exists; the .tmp residue
 *      from the atomic rename is gone.
 *   4. NULL game_state argument renders as an empty string in JSON, not as
 *      the JSON literal null.
 *   5. Element field round-trip — id/label/type/bounds/enabled/value render
 *      correctly; NULL value renders as JSON null; non-NULL value renders as
 *      a JSON string; embedded " \n \t \ are JSON-escaped (not raw).
 *   6. Every public FS_AX_ElementType renders its stable JSON type string.
 *   7. The public FS_AX_MAX_ELEMENTS budget is honored: exactly the budgeted
 *      elements flush, overflow additions are dropped, and JSON stays valid.
 */

#include "firestaff_accessibility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define FS_AX_MKDIR(p)    _mkdir(p)
#define FS_AX_RMDIR(p)    _rmdir(p)
#define FS_AX_STAT_FN     _stat
#define FS_AX_STAT_TYPE   struct _stat
#define FS_AX_REMOVE(p)   remove(p)
static int fs_ax_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
static int fs_ax_unsetenv(const char* name) {
    return _putenv_s(name, "") == 0;
}
#else
#include <unistd.h>
#define FS_AX_MKDIR(p)    mkdir((p), 0700)
#define FS_AX_RMDIR(p)    rmdir(p)
#define FS_AX_STAT_FN     stat
#define FS_AX_STAT_TYPE   struct stat
#define FS_AX_REMOVE(p)   remove(p)
static int fs_ax_setenv(const char* name, const char* value) {
    return setenv(name, value, 1) == 0;
}
static int fs_ax_unsetenv(const char* name) {
    return unsetenv(name) == 0;
}
#endif

static int g_failures = 0;
static int g_passes = 0;
static char g_home_path[640];
static char g_json_path[1024];
static char g_tmp_path[1024];
static char g_subdir_path[1024];

static void fs_ax_check(int cond, const char* msg) {
    if (cond) {
        ++g_passes;
    } else {
        ++g_failures;
        fprintf(stderr, "  FAIL: %s\n", msg);
    }
}

static int fs_ax_file_exists(const char* path) {
    FS_AX_STAT_TYPE st;
    return FS_AX_STAT_FN(path, &st) == 0;
}

static int fs_ax_read_all(const char* path, char* out, size_t outSize) {
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

static int fs_ax_count_substr(const char* haystack, const char* needle) {
    int count = 0;
    const char* p = haystack;
    size_t needleLen;
    if (!haystack || !needle || !*needle) return 0;
    needleLen = strlen(needle);
    while ((p = strstr(p, needle)) != NULL) {
        ++count;
        p += needleLen;
    }
    return count;
}

static void fs_ax_recompute_paths(void) {
    snprintf(g_subdir_path, sizeof(g_subdir_path),
             "%s/.firestaff", g_home_path);
    snprintf(g_json_path, sizeof(g_json_path),
             "%s/accessibility.json", g_subdir_path);
    snprintf(g_tmp_path, sizeof(g_tmp_path),
             "%s.tmp", g_json_path);
}

static int fs_ax_setup_temp_home(void) {
#if defined(_WIN32)
    /*
     * Firestaff's accessibility writer currently keys its output path off
     * getenv("HOME") with no USERPROFILE fallback, so on bare Windows where
     * HOME is unset the writer targets a path it cannot create. Redirect
     * HOME to a freshly-created temp subdir so the writer has somewhere
     * valid to drop accessibility.json.
     */
    char tmpl[64];
    unsigned int seed = (unsigned int)(_getpid() ^ 0x9E3779B9U);
    int attempt;
    g_home_path[0] = '\0';
    for (attempt = 0; attempt < 32; ++attempt) {
        seed = seed * 1103515245U + 12345U;
        snprintf(tmpl, sizeof(tmpl), "./firestaff_ax_test_%x",
                 seed & 0xFFFFFFU);
        snprintf(g_home_path, sizeof(g_home_path), "%s", tmpl);
        if (FS_AX_MKDIR(g_home_path) == 0) {
            break;
        }
        g_home_path[0] = '\0';
    }
    if (g_home_path[0] == '\0') return 0;
#else
    char tmpl[] = "/tmp/firestaff-ax-XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) return 0;
    snprintf(g_home_path, sizeof(g_home_path), "%s", made);
#endif
    fs_ax_recompute_paths();
    return fs_ax_setenv("HOME", g_home_path) ? 1 : 0;
}

static void fs_ax_teardown_temp_home(void) {
    FS_AX_REMOVE(g_json_path);
    FS_AX_REMOVE(g_tmp_path);
    FS_AX_REMOVE(g_subdir_path);
    if (g_home_path[0] != '\0') {
        FS_AX_RMDIR(g_home_path);
    }
    (void)fs_ax_unsetenv("HOME");
}

/* ── Subtest 1: disabled state writes nothing ───────────────────── */

static void subtest_disabled_writes_nothing(void) {
    fs_ax_shutdown();
    FS_AX_REMOVE(g_json_path);
    FS_AX_REMOVE(g_tmp_path);

    /* Don't enable — flush should be a no-op. */
    fs_ax_begin_frame(320, 200, "main_menu");
    fs_ax_flush();

    fs_ax_check(!fs_ax_file_exists(g_json_path),
                "disabled: no accessibility.json is written");
    fs_ax_check(!fs_ax_file_exists(g_tmp_path),
                "disabled: no .tmp residue is left behind");
    fs_ax_check(fs_ax_is_enabled() == 0,
                "disabled: fs_ax_is_enabled() reports 0 before set_enabled(1)");
}

/* ── Subtest 2: empty manifest field presence ───────────────────── */

static void subtest_empty_manifest_field_presence(void) {
    char buf[8192];
    int n;

    fs_ax_shutdown();
    FS_AX_REMOVE(g_json_path);
    FS_AX_REMOVE(g_tmp_path);

    fs_ax_set_enabled(1);
    fs_ax_check(fs_ax_is_enabled() == 1,
                "empty: fs_ax_set_enabled(1) flips enabled state");

    fs_ax_begin_frame(320, 200, "main_menu");
    fs_ax_flush();

    fs_ax_check(fs_ax_file_exists(g_json_path),
                "empty: final JSON file is written");

    n = fs_ax_read_all(g_json_path, buf, sizeof(buf));
    fs_ax_check(n > 0, "empty: file is non-empty");
    if (n <= 0) return;

    fs_ax_check(strstr(buf, "\"version\":1") != NULL,
                "empty: version field present (Peekaboo schema v1)");
    fs_ax_check(strstr(buf, "\"app\":\"firestaff\"") != NULL,
                "empty: app field present and pinned to \"firestaff\"");
    fs_ax_check(strstr(buf, "\"gameState\":\"main_menu\"") != NULL,
                "empty: gameState field present");
    fs_ax_check(strstr(buf, "\"framebuffer\":{\"width\":320,\"height\":200}") != NULL,
                "empty: framebuffer dims present (320x200)");
    fs_ax_check(strstr(buf, "\"elements\":[]") != NULL,
                "empty: elements list is well-formed empty array");
    fs_ax_check(strstr(buf, "[,") == NULL,
                "empty: no leading-comma artifact in elements");
    fs_ax_check(strstr(buf, ",]") == NULL,
                "empty: no trailing-comma artifact in elements");
    fs_ax_check(strstr(buf, ",,") == NULL,
                "empty: no double-comma artifact anywhere in manifest");
}

/* ── Subtest 3: atomic write leaves no .tmp residue ─────────────── */

static void subtest_atomic_write_no_tmp_residue(void) {
    fs_ax_shutdown();
    FS_AX_REMOVE(g_json_path);
    FS_AX_REMOVE(g_tmp_path);

    fs_ax_set_enabled(1);
    fs_ax_begin_frame(320, 200, "menu");
    fs_ax_flush();

    fs_ax_check(fs_ax_file_exists(g_json_path),
                "atomic: final file exists after flush");
    fs_ax_check(!fs_ax_file_exists(g_tmp_path),
                "atomic: tmp file is gone after flush (rename succeeded)");

    /* A second flush must still leave no .tmp residue. */
    fs_ax_begin_frame(640, 400, "menu");
    fs_ax_flush();
    fs_ax_check(fs_ax_file_exists(g_json_path),
                "atomic: second flush still leaves a final file");
    fs_ax_check(!fs_ax_file_exists(g_tmp_path),
                "atomic: second flush still leaves no .tmp residue");
}

/* ── Subtest 4: NULL game_state renders as empty string ─────────── */

static void subtest_null_game_state_is_empty_string(void) {
    char buf[8192];
    int n;

    fs_ax_shutdown();
    FS_AX_REMOVE(g_json_path);
    FS_AX_REMOVE(g_tmp_path);

    fs_ax_set_enabled(1);
    fs_ax_begin_frame(320, 200, NULL);
    fs_ax_flush();

    n = fs_ax_read_all(g_json_path, buf, sizeof(buf));
    fs_ax_check(n > 0, "null-state: file is written");
    if (n <= 0) return;
    fs_ax_check(strstr(buf, "\"gameState\":\"\"") != NULL,
                "null-state: empty string is rendered for gameState");
    fs_ax_check(strstr(buf, "\"gameState\":null") == NULL,
                "null-state: JSON null literal must not appear for gameState");
}

/* ── Subtest 5: element field round-trip + escape behavior ──────── */

static void subtest_element_round_trip_and_escape(void) {
    char buf[8192];
    int n;
    FS_AX_Element with_null_value;
    FS_AX_Element with_str_value;
    FS_AX_Element with_escape_chars;

    fs_ax_shutdown();
    FS_AX_REMOVE(g_json_path);
    FS_AX_REMOVE(g_tmp_path);

    fs_ax_set_enabled(1);
    fs_ax_begin_frame(1920, 1080, "in_game");

    /* element 1: NULL value -> JSON null */
    memset(&with_null_value, 0, sizeof(with_null_value));
    with_null_value.id = "B_FORWARD";
    with_null_value.label = "Forward";
    with_null_value.type = FS_AX_MOVEMENT;
    with_null_value.x = 100; with_null_value.y = 200;
    with_null_value.w = 50;  with_null_value.h = 50;
    with_null_value.enabled = 1;
    with_null_value.value = NULL;
    fs_ax_add_element(&with_null_value);

    /* element 2: non-NULL value -> quoted JSON string */
    memset(&with_str_value, 0, sizeof(with_str_value));
    with_str_value.id = "B_HAND_L";
    with_str_value.label = "Left Hand";
    with_str_value.type = FS_AX_SLOT;
    with_str_value.x = 10;  with_str_value.y = 10;
    with_str_value.w = 60;  with_str_value.h = 60;
    with_str_value.enabled = 1;
    with_str_value.value = "Sword +1";
    fs_ax_add_element(&with_str_value);

    /* element 3: special characters must be JSON-escaped in label and value */
    memset(&with_escape_chars, 0, sizeof(with_escape_chars));
    with_escape_chars.id = "T_DIALOG";
    with_escape_chars.label = "He said \"hi\"\nand left";
    with_escape_chars.type = FS_AX_TEXT;
    with_escape_chars.x = 0;  with_escape_chars.y = 0;
    with_escape_chars.w = 1;  with_escape_chars.h = 1;
    with_escape_chars.enabled = 0;
    with_escape_chars.value = "tab\there\\back";
    fs_ax_add_element(&with_escape_chars);

    fs_ax_flush();

    n = fs_ax_read_all(g_json_path, buf, sizeof(buf));
    fs_ax_check(n > 0, "elements: file is written");
    if (n <= 0) return;

    fs_ax_check(strstr(buf, "\"gameState\":\"in_game\"") != NULL,
                "elements: gameState is present");
    fs_ax_check(strstr(buf,
                       "\"framebuffer\":{\"width\":1920,\"height\":1080}") != NULL,
                "elements: framebuffer dims present (1920x1080)");
    fs_ax_check(strstr(buf, "\"id\":\"B_FORWARD\"") != NULL,
                "elements: id 1 is present");
    fs_ax_check(strstr(buf, "\"id\":\"B_HAND_L\"") != NULL,
                "elements: id 2 is present");
    fs_ax_check(strstr(buf, "\"id\":\"T_DIALOG\"") != NULL,
                "elements: id 3 is present");
    fs_ax_check(strstr(buf, "\"type\":\"movement\"") != NULL,
                "elements: type=FS_AX_MOVEMENT renders as \"movement\"");
    fs_ax_check(strstr(buf, "\"type\":\"slot\"") != NULL,
                "elements: type=FS_AX_SLOT renders as \"slot\"");
    fs_ax_check(strstr(buf, "\"type\":\"text\"") != NULL,
                "elements: type=FS_AX_TEXT renders as \"text\"");
    fs_ax_check(strstr(buf, "\"enabled\":true") != NULL,
                "elements: enabled=1 renders as JSON true");
    fs_ax_check(strstr(buf, "\"enabled\":false") != NULL,
                "elements: enabled=0 renders as JSON false");
    fs_ax_check(strstr(buf, "\"value\":null}") != NULL,
                "elements: NULL value renders as JSON null");
    fs_ax_check(strstr(buf, "\"value\":\"Sword +1\"") != NULL,
                "elements: non-NULL value renders as a JSON string");

    /* Escape behavior: " \n \t \ must be JSON-escaped inside string content. */
    fs_ax_check(strstr(buf, "\\\"hi\\\"") != NULL,
                "elements: embedded double-quote is JSON-escaped (\\\"hi\\\")");
    fs_ax_check(strstr(buf, "\\n") != NULL,
                "elements: embedded newline is JSON-escaped (\\n)");
    fs_ax_check(strstr(buf, "\\t") != NULL,
                "elements: embedded tab is JSON-escaped (\\t)");
    fs_ax_check(strstr(buf, "\\\\") != NULL,
                "elements: embedded backslash is JSON-escaped (\\\\)");
    fs_ax_check(strstr(buf, "\\\\\\\"") == NULL,
                "elements: no double-escaped \\\" in output");
}

/* ── Subtest 6: all element type strings are stable ─────────────── */

static void subtest_all_element_types_render_stable_strings(void) {
    static const struct {
        FS_AX_ElementType type;
        const char* id;
        const char* jsonType;
    } cases[] = {
        { FS_AX_BUTTON, "TYPE_BUTTON", "button" },
        { FS_AX_REGION, "TYPE_REGION", "region" },
        { FS_AX_TEXT, "TYPE_TEXT", "text" },
        { FS_AX_SLOT, "TYPE_SLOT", "slot" },
        { FS_AX_PORTRAIT, "TYPE_PORTRAIT", "portrait" },
        { FS_AX_MOVEMENT, "TYPE_MOVEMENT", "movement" },
        { FS_AX_DIALOG_CHOICE, "TYPE_DIALOG_CHOICE", "dialog_choice" },
        { FS_AX_CHAMPION_MIRROR, "TYPE_CHAMPION_MIRROR", "champion_mirror" },
        { FS_AX_LAUNCHER_CARD, "TYPE_LAUNCHER_CARD", "launcher_card" },
        { FS_AX_LAUNCHER_TAB, "TYPE_LAUNCHER_TAB", "launcher_tab" },
        { FS_AX_LAUNCHER_ROW, "TYPE_LAUNCHER_ROW", "launcher_row" },
        { FS_AX_POPUP, "TYPE_POPUP", "popup" },
        { FS_AX_POPUP_OK, "TYPE_POPUP_OK", "popup_ok" },
        { FS_AX_CATEGORY_TAB, "TYPE_CATEGORY_TAB", "category_tab" },
        { FS_AX_BESTIARY_ROW, "TYPE_BESTIARY_ROW", "bestiary_row" },
        { FS_AX_ITEM_ENCYCLOPEDIA_ROW, "TYPE_ITEM_ROW", "item_encyclopedia_row" },
        { FS_AX_SCREENSHOT_THUMB, "TYPE_SCREENSHOT_THUMB", "screenshot_thumb" },
        { FS_AX_MUSEUM_CATEGORY, "TYPE_MUSEUM_CATEGORY", "museum_category" },
        { FS_AX_MUSEUM_BULLET, "TYPE_MUSEUM_BULLET", "museum_bullet" }
    };
    char buf[32768];
    int n;
    int i;

    fs_ax_shutdown();
    FS_AX_REMOVE(g_json_path);
    FS_AX_REMOVE(g_tmp_path);

    fs_ax_set_enabled(1);
    fs_ax_begin_frame(320, 200, "type_catalog");

    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        FS_AX_Element element;
        memset(&element, 0, sizeof(element));
        element.id = cases[i].id;
        element.label = cases[i].id;
        element.type = cases[i].type;
        element.x = i;
        element.y = i + 1;
        element.w = 2;
        element.h = 3;
        element.enabled = 1;
        fs_ax_add_element(&element);
    }

    fs_ax_flush();

    n = fs_ax_read_all(g_json_path, buf, sizeof(buf));
    fs_ax_check(n > 0, "types: file is written");
    if (n <= 0) return;

    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        char idNeedle[96];
        char typeNeedle[96];
        snprintf(idNeedle, sizeof(idNeedle), "\"id\":\"%s\"", cases[i].id);
        snprintf(typeNeedle, sizeof(typeNeedle), "\"type\":\"%s\"",
                 cases[i].jsonType);
        fs_ax_check(strstr(buf, idNeedle) != NULL,
                    "types: element id is present");
        fs_ax_check(strstr(buf, typeNeedle) != NULL,
                    "types: stable type string is present");
    }
    fs_ax_check(strstr(buf, "\"type\":\"unknown\"") == NULL,
                "types: no public element type renders as unknown");
}

/* ── Subtest 7: public element budget clamps overflow ───────────── */

static void subtest_element_budget_clamps_without_malformed_json(void) {
    char buf[65536];
    char ids[FS_AX_MAX_ELEMENTS + 2][32];
    FS_AX_Element elements[FS_AX_MAX_ELEMENTS + 2];
    int n;
    int i;

    fs_ax_shutdown();
    FS_AX_REMOVE(g_json_path);
    FS_AX_REMOVE(g_tmp_path);

    fs_ax_set_enabled(1);
    fs_ax_begin_frame(320, 200, "budget");

    for (i = 0; i < FS_AX_MAX_ELEMENTS + 2; ++i) {
        snprintf(ids[i], sizeof(ids[i]), "BUDGET_%03d", i);
        memset(&elements[i], 0, sizeof(elements[i]));
        elements[i].id = ids[i];
        elements[i].label = "Budget Element";
        elements[i].type = FS_AX_BUTTON;
        elements[i].x = i;
        elements[i].y = i;
        elements[i].w = 1;
        elements[i].h = 1;
        elements[i].enabled = 1;
        fs_ax_add_element(&elements[i]);
    }

    fs_ax_flush();

    n = fs_ax_read_all(g_json_path, buf, sizeof(buf));
    fs_ax_check(n > 0, "budget: file is written");
    if (n <= 0) return;

    fs_ax_check(FS_AX_MAX_ELEMENTS == 128,
                "budget: public FS_AX_MAX_ELEMENTS remains 128");
    fs_ax_check(strstr(buf, "\"id\":\"BUDGET_000\"") != NULL,
                "budget: first element is present");
    fs_ax_check(strstr(buf, "\"id\":\"BUDGET_127\"") != NULL,
                "budget: last in-budget element is present");
    fs_ax_check(strstr(buf, "\"id\":\"BUDGET_128\"") == NULL &&
                strstr(buf, "\"id\":\"BUDGET_129\"") == NULL,
                "budget: overflow elements are dropped");
    fs_ax_check(fs_ax_count_substr(buf, "\"id\":\"BUDGET_") == FS_AX_MAX_ELEMENTS,
                "budget: exactly FS_AX_MAX_ELEMENTS budget IDs emitted");
    fs_ax_check(strstr(buf, "[,") == NULL,
                "budget: no leading-comma artifact in elements");
    fs_ax_check(strstr(buf, ",]") == NULL,
                "budget: no trailing-comma artifact in elements");
    fs_ax_check(strstr(buf, ",,") == NULL,
                "budget: no double-comma artifact anywhere in manifest");
    fs_ax_check(!fs_ax_file_exists(g_tmp_path),
                "budget: overflow flush leaves no .tmp residue");
}

/* ── main ───────────────────────────────────────────────────────── */

int main(void) {
    printf("  [1] disabled state writes nothing...\n");
    subtest_disabled_writes_nothing();

    if (!fs_ax_setup_temp_home()) {
        fprintf(stderr,
                "could not redirect HOME to a temp dir; "
                "subtests 2-7 skipped\n");
        printf("\n  firestaff_accessibility_manifest: %d passed, %d failed "
               "(isolation failed)\n",
               g_passes, g_failures);
        return g_failures == 0 ? 0 : 1;
    }

    printf("  [2] empty manifest field presence + no malformed elements...\n");
    subtest_empty_manifest_field_presence();

    printf("  [3] atomic write leaves no .tmp residue...\n");
    subtest_atomic_write_no_tmp_residue();

    printf("  [4] NULL game_state renders as empty string...\n");
    subtest_null_game_state_is_empty_string();

    printf("  [5] element field round-trip + escape behavior...\n");
    subtest_element_round_trip_and_escape();

    printf("  [6] all element type strings are stable...\n");
    subtest_all_element_types_render_stable_strings();

    printf("  [7] element budget clamps overflow without malformed JSON...\n");
    subtest_element_budget_clamps_without_malformed_json();

    fs_ax_teardown_temp_home();

    printf("\n  firestaff_accessibility_manifest: %d passed, %d failed\n",
           g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
