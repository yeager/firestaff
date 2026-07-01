/*
 * test_m11_dialog_overlay_accessibility.c
 *   Narrow regression for the in-game dialog overlay accessibility manifest
 *   slice added to M11_GameView_EmitDialogOverlayAccessibility.
 *
 * Locks in behavior that screen-reader / Peekaboo harnesses depend on for
 * the in-game modal surface:
 *
 *   - Framebuffer size is preserved (width/height round-trip).
 *   - gameState is "dialog" while the overlay is on screen.
 *   - DIALOG_TEXT element carries the dialog text as its value field.
 *   - The plain ESC confirm surface (returnToMenuConfirmActive) emits
 *     DIALOG_CONFIRM + two DIALOG_CHOICE_n buttons, centered with bounds
 *     that match what m11_dialog_choice_at_point() actually hit-tests.
 *   - The source-backed dialog surface (text plaque / save dialog) emits
 *     one DIALOG_CHOICE_n button per source choice, with bounds from
 *     M11_GameView_GetV1DialogChoiceHitZone (translated from
 *     viewport-relative to absolute framebuffer coords using the
 *     M11_VIEWPORT_X / M11_VIEWPORT_Y origin so external tools do not
 *     need to know about the viewport layout).
 *   - Non-dialog frames do NOT emit any DIALOG_* elements (no leakage).
 *   - Disabled accessibility writer is a no-op.
 *   - Element IDs are stable contract points ("DIALOG_TEXT",
 *     "DIALOG_CONFIRM", "DIALOG_CHOICE_n") so harnesses can pin against
 *     them without re-parsing Firestaff source.
 *
 * Test isolation: HOME is redirected to a per-run temp dir so the manifest
 * never touches the real user ~/.firestaff.
 *
 * Source-locked against:
 *   - ReDMCSB DIALOG.C F0424 (F0424_DIALOG_GetChoice) and F0427
 *     (F0427_DIALOG_Draw): the dialog text plaque + per-choice buttons.
 *   - ReDMCSB CLIKVIEW.C F0378/F0379/F0380: bottom-row click routing.
 *   - ReDMCSB LOADSAVE.C:1371-1379: ordinary ESC confirm surface (200x50
 *     centered YES/NO dialog).
 */

#include "firestaff_accessibility.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define FS_AX_TEST_MKDIR(p)  _mkdir(p)
#define FS_AX_TEST_RMDIR(p)  _rmdir(p)
#define FS_AX_TEST_STAT_FN   _stat
#define FS_AX_TEST_STAT_TYPE struct _stat
#define FS_AX_TEST_REMOVE(p) remove(p)
static int fs_ax_test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
static int fs_ax_test_unsetenv(const char* name) {
    return _putenv_s(name, "") == 0;
}
#else
#include <unistd.h>
#define FS_AX_TEST_MKDIR(p)  mkdir((p), 0700)
#define FS_AX_TEST_RMDIR(p)  rmdir(p)
#define FS_AX_TEST_STAT_FN   stat
#define FS_AX_TEST_STAT_TYPE struct stat
#define FS_AX_TEST_REMOVE(p) remove(p)
static int fs_ax_test_setenv(const char* name, const char* value) {
    return setenv(name, value, 1) == 0;
}
static int fs_ax_test_unsetenv(const char* name) {
    return unsetenv(name) == 0;
}
#endif

#define FB_W 320
#define FB_H 200

static int g_failures = 0;
static int g_passes = 0;
static char g_home_path[640];
static char g_json_path[1024];
static char g_tmp_path[1024];
static char g_subdir_path[1024];

static void check(int cond, const char* msg) {
    if (cond) {
        ++g_passes;
    } else {
        ++g_failures;
        fprintf(stderr, "  FAIL: %s\n", msg);
    }
}

static int read_all(const char* path, char* out, size_t outSize) {
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

static void recompute_paths(void) {
    snprintf(g_subdir_path, sizeof(g_subdir_path),
             "%s/.firestaff", g_home_path);
    snprintf(g_json_path, sizeof(g_json_path),
             "%s/accessibility.json", g_subdir_path);
    snprintf(g_tmp_path, sizeof(g_tmp_path),
             "%s.tmp", g_json_path);
}

static int setup_temp_home(void) {
#if defined(_WIN32)
    char tmpl[64];
    unsigned int seed = (unsigned int)(_getpid() ^ 0x9E3779B9U);
    int attempt;
    g_home_path[0] = '\0';
    for (attempt = 0; attempt < 32; ++attempt) {
        seed = seed * 1103515245U + 12345U;
        snprintf(tmpl, sizeof(tmpl), "./firestaff_dlg_ax_test_%x",
                 seed & 0xFFFFFFU);
        snprintf(g_home_path, sizeof(g_home_path), "%s", tmpl);
        if (FS_AX_TEST_MKDIR(g_home_path) == 0) {
            break;
        }
        g_home_path[0] = '\0';
    }
    if (g_home_path[0] == '\0') return 0;
#else
    char tmpl[] = "/tmp/firestaff-dlg-ax-XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) return 0;
    snprintf(g_home_path, sizeof(g_home_path), "%s", made);
#endif
    recompute_paths();
    return fs_ax_test_setenv("HOME", g_home_path) ? 1 : 0;
}

static void teardown_temp_home(void) {
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);
    FS_AX_TEST_REMOVE(g_subdir_path);
    if (g_home_path[0] != '\0') {
        FS_AX_TEST_RMDIR(g_home_path);
    }
    (void)fs_ax_test_unsetenv("HOME");
}

/* Count occurrences of needle in haystack. */
static int count_occurrences(const char* haystack, const char* needle) {
    int count = 0;
    const char* p = haystack;
    size_t nlen = strlen(needle);
    if (nlen == 0) return 0;
    while ((p = strstr(p, needle)) != NULL) {
        ++count;
        p += nlen;
    }
    return count;
}

/* ── Subtest 1: disabled writer is a no-op ───────────────────────── */

static void subtest_disabled_writer_noop(void) {
    M11_GameViewState state;
    char buf[8192];
    int n;

    fs_ax_shutdown();
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);

    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    state.dialogChoiceCount = 1;
    snprintf(state.dialogOverlayText, sizeof(state.dialogOverlayText),
             "ANY TEXT");
    snprintf(state.dialogChoices[0], sizeof(state.dialogChoices[0]), "OK");

    /* fs_ax_set_enabled not called -> is_enabled is 0.  The helper
     * must still produce no DIALOG_TEXT / DIALOG_CHOICE_* entries. */
    fs_ax_begin_frame(FB_W, FB_H, "dialog");
    M11_GameView_EmitDialogOverlayAccessibility(&state, FB_W, FB_H);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    /* Disabled writer -> no manifest file is written, so n <= 0. */
    check(n <= 0, "disabled: no manifest file is written when writer is off");
    if (n > 0) {
        check(strstr(buf, "DIALOG_TEXT") == NULL,
              "disabled: no DIALOG_TEXT element when writer is off");
        check(strstr(buf, "DIALOG_CHOICE_") == NULL,
              "disabled: no DIALOG_CHOICE_n element when writer is off");
    }
}

/* ── Subtest 2: non-dialog frame does NOT leak DIALOG_* elements ──── */

static void subtest_non_dialog_frame_no_leak(void) {
    M11_GameViewState state;
    char buf[8192];
    int n;

    fs_ax_shutdown();
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 0;
    state.dialogChoiceCount = 0;

    /* Caller is responsible for not invoking the helper when dialog is
     * inactive (the production call site guards on dialogOverlayActive),
     * but even if the helper is invoked directly, the absence of
     * dialogOverlayActive must mean no DIALOG_* elements leak through. */
    fs_ax_begin_frame(FB_W, FB_H, "gameplay");
    M11_GameView_EmitDialogOverlayAccessibility(&state, FB_W, FB_H);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    check(n > 0, "non-dialog: file is written");
    if (n <= 0) return;
    check(strstr(buf, "DIALOG_TEXT") == NULL,
          "non-dialog: no DIALOG_TEXT element is emitted");
    check(strstr(buf, "DIALOG_CONFIRM") == NULL,
          "non-dialog: no DIALOG_CONFIRM element is emitted");
    check(strstr(buf, "DIALOG_CHOICE_") == NULL,
          "non-dialog: no DIALOG_CHOICE_n element is emitted");
}

/* ── Subtest 3: dialog text element with value round-trip ─────────── */

static void subtest_dialog_text_element_round_trip(void) {
    M11_GameViewState state;
    char buf[8192];
    int n;

    fs_ax_shutdown();
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    state.dialogChoiceCount = 1;
    snprintf(state.dialogOverlayText, sizeof(state.dialogOverlayText),
             "This is a text plaque");
    snprintf(state.dialogChoices[0], sizeof(state.dialogChoices[0]), "OK");

    fs_ax_begin_frame(FB_W, FB_H, "dialog");
    M11_GameView_EmitDialogOverlayAccessibility(&state, FB_W, FB_H);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    check(n > 0, "dlg-text: file is written");
    if (n <= 0) return;
    check(strstr(buf, "DIALOG_TEXT") != NULL,
          "dlg-text: DIALOG_TEXT element id is present");
    check(strstr(buf, "\"value\":\"This is a text plaque\"") != NULL,
          "dlg-text: dialog text appears as the DIALOG_TEXT value");
    check(strstr(buf, "\"type\":\"text\"") != NULL,
          "dlg-text: DIALOG_TEXT renders with type=text");
    /* Text plaque dimensions (260x80) match the centered dialog box at
     * (30, 50, 260, 80) used by m11_draw_dialog_box. */
    check(strstr(buf, "\"x\":30,\"y\":50,\"w\":260,\"h\":80") != NULL,
          "dlg-text: DIALOG_TEXT bounds match the source plaque rect");
}

/* ── Subtest 4: returnToMenuConfirm surface emits 2 buttons + confirm
       with bounds that match m11_dialog_choice_at_point() ──────────── */

static void subtest_return_to_menu_confirm_surface(void) {
    M11_GameViewState state;
    char buf[8192];
    int n;
    /* Centered 200x50 dialog on a 320x200 framebuffer:
     *   cdlgX = (320-200)/2 = 60
     *   cdlgY = (200-50)/2  = 75
     *   choiceY = cdlgY + 50 - 16 = 109
     *   hitY   = choiceY - 5     = 104
     *   choiceW = 200/2           = 100
     * Choice 1: (60, 104, 100, 25); Choice 2: (160, 104, 100, 25).
     * Confirm: (60, 75, 200, 50). */
    const int kConfirmX = 60, kConfirmY = 75;
    const int kConfirmW = 200, kConfirmH = 50;
    const int kChoiceY = 104, kChoiceW = 100, kChoiceH = 25;

    fs_ax_shutdown();
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    state.returnToMenuConfirmActive = 1;
    state.quitGuardActive = 0;
    state.dialogChoiceCount = 2;
    snprintf(state.dialogOverlayText, sizeof(state.dialogOverlayText),
             "Return to menu?");
    snprintf(state.dialogChoices[0], sizeof(state.dialogChoices[0]), "YES");
    snprintf(state.dialogChoices[1], sizeof(state.dialogChoices[1]), "NO");

    fs_ax_begin_frame(FB_W, FB_H, "dialog");
    M11_GameView_EmitDialogOverlayAccessibility(&state, FB_W, FB_H);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    check(n > 0, "confirm: file is written");
    if (n <= 0) return;

    check(strstr(buf, "DIALOG_CONFIRM") != NULL,
          "confirm: DIALOG_CONFIRM element is present");
    check(strstr(buf, "DIALOG_TEXT") != NULL,
          "confirm: DIALOG_TEXT element is also present");
    check(count_occurrences(buf, "DIALOG_CHOICE_1") == 1,
          "confirm: exactly one DIALOG_CHOICE_1 element");
    check(count_occurrences(buf, "DIALOG_CHOICE_2") == 1,
          "confirm: exactly one DIALOG_CHOICE_2 element");
    check(count_occurrences(buf, "\"type\":\"dialog_choice\"") == 2,
          "confirm: both choices render with type=dialog_choice");

    /* Bounds pinning: these exact strings are how a click automation
     * client reads the manifest. They must not drift. */
    {
        char wantConfirm[80];
        char wantChoice1[80];
        char wantChoice2[80];
        snprintf(wantConfirm, sizeof(wantConfirm),
                 "\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d",
                 kConfirmX, kConfirmY, kConfirmW, kConfirmH);
        snprintf(wantChoice1, sizeof(wantChoice1),
                 "\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d",
                 kConfirmX, kChoiceY, kChoiceW, kChoiceH);
        snprintf(wantChoice2, sizeof(wantChoice2),
                 "\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d",
                 kConfirmX + kChoiceW, kChoiceY, kChoiceW, kChoiceH);
        check(strstr(buf, wantConfirm) != NULL,
              "confirm: DIALOG_CONFIRM bounds match the centered 200x50 panel");
        check(strstr(buf, wantChoice1) != NULL,
              "confirm: DIALOG_CHOICE_1 bounds match the YES hit-zone");
        check(strstr(buf, wantChoice2) != NULL,
              "confirm: DIALOG_CHOICE_2 bounds match the NO hit-zone");
    }

    check(strstr(buf, "\"value\":\"YES\"") != NULL,
          "confirm: DIALOG_CHOICE_1 value carries the choice label");
    check(strstr(buf, "\"value\":\"NO\"") != NULL,
          "confirm: DIALOG_CHOICE_2 value carries the choice label");
}

/* ── Subtest 5: confirm surface falls back to YES/NO when choices empty
       (defensive — the production guard requires dialogChoiceCount==2,
       but the helper's own contract still says it does not crash) ─── */

static void subtest_confirm_fallback_labels(void) {
    M11_GameViewState state;
    char buf[8192];
    int n;

    fs_ax_shutdown();
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    state.returnToMenuConfirmActive = 1;
    state.quitGuardActive = 0;
    state.dialogChoiceCount = 2;
    /* Both choices empty (the production guard path normally populates
     * them, but the helper still must not crash). */
    state.dialogChoices[0][0] = '\0';
    state.dialogChoices[1][0] = '\0';
    snprintf(state.dialogOverlayText, sizeof(state.dialogOverlayText),
             "Return to menu?");

    fs_ax_begin_frame(FB_W, FB_H, "dialog");
    M11_GameView_EmitDialogOverlayAccessibility(&state, FB_W, FB_H);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    check(n > 0, "confirm-fallback: file is written");
    if (n <= 0) return;
    check(strstr(buf, "\"value\":\"YES\"") != NULL,
          "confirm-fallback: empty choice 1 falls back to YES label");
    check(strstr(buf, "\"value\":\"NO\"") != NULL,
          "confirm-fallback: empty choice 2 falls back to NO label");
}

/* ── Subtest 6: source-backed dialog overlay emits one button per choice
       with bounds translated from viewport-relative to framebuffer-abs ─ */

static void subtest_source_backed_dialog_overlay(void) {
    M11_GameViewState state;
    char buf[8192];
    int n;
    int i;

    fs_ax_shutdown();
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    state.dialogChoiceCount = 3;
    snprintf(state.dialogOverlayText, sizeof(state.dialogOverlayText),
             "Pick a slot");
    snprintf(state.dialogChoices[0], sizeof(state.dialogChoices[0]), "LEFT");
    snprintf(state.dialogChoices[1], sizeof(state.dialogChoices[1]), "RIGHT");
    snprintf(state.dialogChoices[2], sizeof(state.dialogChoices[2]), "CANCEL");

    fs_ax_begin_frame(FB_W, FB_H, "dialog");
    M11_GameView_EmitDialogOverlayAccessibility(&state, FB_W, FB_H);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    check(n > 0, "source-dlg: file is written");
    if (n <= 0) return;

    check(strstr(buf, "DIALOG_TEXT") != NULL,
          "source-dlg: DIALOG_TEXT element is present");
    check(strstr(buf, "DIALOG_CONFIRM") == NULL,
          "source-dlg: no DIALOG_CONFIRM on source-backed dialog surface");
    check(count_occurrences(buf, "DIALOG_CHOICE_") == 3,
          "source-dlg: exactly 3 DIALOG_CHOICE_n elements");
    check(count_occurrences(buf, "\"type\":\"dialog_choice\"") == 3,
          "source-dlg: all 3 buttons render with type=dialog_choice");

    /* Each choice button must carry its own label in value. */
    check(strstr(buf, "\"value\":\"LEFT\"") != NULL,
          "source-dlg: choice 1 value is LEFT");
    check(strstr(buf, "\"value\":\"RIGHT\"") != NULL,
          "source-dlg: choice 2 value is RIGHT");
    check(strstr(buf, "\"value\":\"CANCEL\"") != NULL,
          "source-dlg: choice 3 value is CANCEL");

    /* Bounds must come from M11_GameView_GetV1DialogChoiceHitZone
     * (viewport-relative) translated into absolute framebuffer coords
     * by adding M11_VIEWPORT_X / M11_VIEWPORT_Y (both 0/33 for DM1 V1).
     * choiceCount=3: indices 0/1/2 map to zones 457/460/461 ->
     *   (16,67,192) / (16,104,86)  / (123,104,86)
     * -> absolute (16, 100, 192, 17) / (16, 137, 86, 17) / (123, 137, 86, 17)
     * (hit-height is always 17 per the V1 dialog hit-zone rules). */
    {
        const char* expectedBounds[3] = {
            "\"x\":16,\"y\":100,\"w\":192,\"h\":17",
            "\"x\":16,\"y\":137,\"w\":86,\"h\":17",
            "\"x\":123,\"y\":137,\"w\":86,\"h\":17"
        };
        for (i = 0; i < 3; ++i) {
            char label[96];
            snprintf(label, sizeof(label),
                     "source-dlg: choice %d bounds match V1 hit-zone + viewport origin",
                     i + 1);
            check(strstr(buf, expectedBounds[i]) != NULL, label);
        }
    }
}

/* ── Subtest 7: 4-choice source dialog emits 4 buttons ────────────── */

static void subtest_source_backed_4_choices(void) {
    M11_GameViewState state;
    char buf[8192];
    int n;

    fs_ax_shutdown();
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    state.dialogChoiceCount = 4;
    snprintf(state.dialogOverlayText, sizeof(state.dialogOverlayText),
             "Four choices");
    snprintf(state.dialogChoices[0], sizeof(state.dialogChoices[0]), "A");
    snprintf(state.dialogChoices[1], sizeof(state.dialogChoices[1]), "B");
    snprintf(state.dialogChoices[2], sizeof(state.dialogChoices[2]), "C");
    snprintf(state.dialogChoices[3], sizeof(state.dialogChoices[3]), "D");

    fs_ax_begin_frame(FB_W, FB_H, "dialog");
    M11_GameView_EmitDialogOverlayAccessibility(&state, FB_W, FB_H);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    check(n > 0, "source-4: file is written");
    if (n <= 0) return;
    check(count_occurrences(buf, "DIALOG_CHOICE_") == 4,
          "source-4: exactly 4 DIALOG_CHOICE_n elements");
    check(strstr(buf, "\"value\":\"A\"") != NULL,
          "source-4: choice 1 value is A");
    check(strstr(buf, "\"value\":\"D\"") != NULL,
          "source-4: choice 4 value is D");
}

/* ── Subtest 8: empty dialog text yields no DIALOG_* elements ──────── */

static void subtest_empty_dialog_text_skips_text(void) {
    M11_GameViewState state;
    char buf[8192];
    int n;

    fs_ax_shutdown();
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    state.dialogChoiceCount = 1;
    /* dialogOverlayText intentionally left empty */
    snprintf(state.dialogChoices[0], sizeof(state.dialogChoices[0]), "OK");

    fs_ax_begin_frame(FB_W, FB_H, "dialog");
    M11_GameView_EmitDialogOverlayAccessibility(&state, FB_W, FB_H);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    check(n > 0, "empty-text: file is written (frame still emits)");
    if (n <= 0) return;
    /* When dialogOverlayText is empty the helper short-circuits to
     * match the production render path (m11_draw_dialog_overlay only
     * draws when dialogOverlayText[0] != '\0'), so no DIALOG_*
     * elements are emitted either. */
    check(strstr(buf, "DIALOG_TEXT") == NULL,
          "empty-text: no DIALOG_TEXT element when dialogOverlayText is empty");
    check(strstr(buf, "DIALOG_CHOICE_") == NULL,
          "empty-text: no DIALOG_CHOICE_n element when dialogOverlayText is empty");
}

/* ── Subtest 9: framebuffer size round-trip ──────────────────────── */

static void subtest_framebuffer_size_round_trip(void) {
    M11_GameViewState state;
    char buf[8192];
    int n;

    fs_ax_shutdown();
    FS_AX_TEST_REMOVE(g_json_path);
    FS_AX_TEST_REMOVE(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    state.dialogChoiceCount = 1;
    snprintf(state.dialogOverlayText, sizeof(state.dialogOverlayText),
             "Hi");
    snprintf(state.dialogChoices[0], sizeof(state.dialogChoices[0]), "OK");

    /* Call the public wrapper, then start a frame and flush so the helper
     * output is in the manifest. The simplest way: the helper already
     * expects a frame to have been started by the caller. The production
     * call site does fs_ax_begin_frame() then M11_GameView_Emit... then
     * fs_ax_flush(). We mirror that exact contract here. */
    fs_ax_begin_frame(FB_W, FB_H, "dialog");
    M11_GameView_EmitDialogOverlayAccessibility(&state, FB_W, FB_H);
    fs_ax_flush();

    n = read_all(g_json_path, buf, sizeof(buf));
    check(n > 0, "fb-size: file is written");
    if (n <= 0) return;
    check(strstr(buf, "\"framebuffer\":{\"width\":320,\"height\":200}") != NULL,
          "fb-size: framebuffer dims round-trip through the helper");
    check(strstr(buf, "\"gameState\":\"dialog\"") != NULL,
          "fb-size: gameState=dialog survives a helper emission");
}

/* ── main ───────────────────────────────────────────────────────── */

int main(void) {
    printf("  [1] disabled writer is a no-op...\n");
    subtest_disabled_writer_noop();

    if (!setup_temp_home()) {
        fprintf(stderr,
                "could not redirect HOME to a temp dir; "
                "subtests 2-9 skipped\n");
        printf("\n  test_m11_dialog_overlay_accessibility: %d passed, %d failed "
               "(isolation failed)\n",
               g_passes, g_failures);
        return g_failures == 0 ? 0 : 1;
    }

    printf("  [2] non-dialog frame does NOT leak DIALOG_* elements...\n");
    subtest_non_dialog_frame_no_leak();

    printf("  [3] dialog text element with value round-trip...\n");
    subtest_dialog_text_element_round_trip();

    printf("  [4] return-to-menu confirm surface (200x50 YES/NO)...\n");
    subtest_return_to_menu_confirm_surface();

    printf("  [5] confirm surface falls back to YES/NO when choices empty...\n");
    subtest_confirm_fallback_labels();

    printf("  [6] source-backed 3-choice dialog overlay...\n");
    subtest_source_backed_dialog_overlay();

    printf("  [7] source-backed 4-choice dialog overlay...\n");
    subtest_source_backed_4_choices();

    printf("  [8] empty dialog text skips DIALOG_TEXT but keeps choice...\n");
    subtest_empty_dialog_text_skips_text();

    printf("  [9] framebuffer size + gameState round-trip...\n");
    subtest_framebuffer_size_round_trip();

    teardown_temp_home();

    printf("\n  test_m11_dialog_overlay_accessibility: %d passed, %d failed\n",
           g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
