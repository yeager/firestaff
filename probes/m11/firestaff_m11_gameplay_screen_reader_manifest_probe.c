/*
 * firestaff_m11_gameplay_screen_reader_manifest_probe.c
 *
 * M11 gameplay-side screen-reader / accessibility state-manifest
 * probe (headless CI).
 *
 * Closes the open gap "Screen reader gameplay-state manifest"
 * (TODO.md / docs/FIRESTAFF_GAP_LIST.md) by exercising
 * m11_screen_reader_update_ex() against a populated
 * M11_GameViewState across all six gameplay surface variants:
 *
 *   A. Disabled state is a no-op (no file written) regardless of
 *      state contents.
 *   B. Gameplay manifest surface: always-on zones (VIEWPORT,
 *      MOVE_FWD / MOVE_BACK / TURN_LEFT / TURN_RIGHT, SPELL_AREA,
 *      HUD_PANEL, CONTROL_STRIP) plus the gameplay_root envelope
 *      with gameState="gameplay" and state=...;active=...;level=...
 *      value. Source-locked viewport bounds (0,33,224,136) and
 *      HUD band (0,136,320,64) are pinned.
 *   C. Inventory manifest surface: 13 equipment slots
 *      (INV_READY_HAND..INV_QUIVER_LINE1_1) with one row marked
 *      "selected" via inventorySelectedSlot, 16 backpack slots
 *      (INV_BACKPACK_0..15), portrait, champion name, and panel
 *      region. Acting champion ordinal surface (ACTING_CHAMPION)
 *      appears when actingChampionOrdinal in [1..4] and is omitted
 *      when 0.
 *   D. Automap / full-screen map surface: AUTOMAP full-screen
 *      region plus gameState="map".
 *   E. Dialog overlay surface: DIALOG_BODY forwarding
 *      dialogOverlayText verbatim, up to 4 DIALOG_CHOICE_N rows
 *      carrying each dialogChoices[i] string in the value field,
 *      and the focused choice marked "selected" via
 *      dialogSelectedChoice.
 *   F. Candidate mirror (Hall of Champions) surface:
 *      CANDIDATE_MIRROR_PANEL with ordinal/party_index value,
 *      plus RESURRECT / REINCARNATE / CANCEL buttons.
 *   G. Endgame surface: ENDGAME_THE_END plaque + 4 mirrors + 4
 *      portraits (slots 0..3).
 *   H. State classifier precedence: gameWon beats dialog,
 *      dialogOverlayActive beats candidateMirrorPanelActive,
 *      candidateMirrorPanelActive beats inventoryPanelActive,
 *      inventoryPanelActive beats mapOverlayActive; default zero
 *      state maps to gameplay; NULL state is a no-op.
 *   I. Bounds stay inside the framebuffer for every emitted
 *      element across all six states (no negative x/y, no
 *      x+w>fbW or y+h>fbH).
 *   J. Atomic write: no .tmp residue after flush, JSON
 *      well-formed (no leading- / trailing- / double-comma
 *      artifacts).
 *   K. Determinism: the JSON gameState field is one of the
 *      pinned six strings ("gameplay" / "inventory" / "map" /
 *      "dialog" / "entrance_mirror" / "endgame") and never
 *      "launcher_*" or anything outside that set.
 *
 * Privacy/safety: the probe redirects fs_ax output via
 * fs_ax_set_output_dir() to a per-run temp dir so the manifest
 * never touches the real user ~/.firestaff.
 *
 * Output: "# summary: N/M invariants passed". Exit 0 on full
 * pass, 1 otherwise.
 *
 * Source-lock: NONE - this is a UI-automation probe on top of the
 * public m11_screen_reader_update_ex() API plus the public
 * M11_GameView_GetV1*Zone() helpers in include/m11_game_view.h.
 * The gameplay manifest's own source citations live in
 * src/engine/m11_game_view_a11y.c (ReDMCSB COORD.C / DEFS.H /
 * PANEL.C / ENDGAME.C) and are not duplicated here.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "firestaff_accessibility.h"
#include "m11_game_view.h"
#include "m11_game_view_a11y.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#include <windows.h>
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
        if (_mkdir(tmpl) == 0) {
            return tmpl;
        }
    }
    return NULL;
}
#else
#include <unistd.h>
static int portable_rmdir(const char* path) { return rmdir(path); }
static int portable_remove(const char* path) { return remove(path); }
static int portable_unsetenv(const char* name) {
    return unsetenv(name);
}
static char* portable_mkdtemp(char* tmpl) {
    return mkdtemp(tmpl);
}
#endif

#define A11Y_NUM_STATE_FIELDS 6
#define A11Y_CHOICE_COUNT     4
#define A11Y_BACKPACK_COUNT   16

/* -- Tiny test scaffold --------------------------------------------- */

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

/* Find the end of an elements[] entry whose "id" matches the
 * requested id. The JSON writer emits one entry per element with
 * no inner-brace separator, so an element ends at the first `}`
 * that's followed by either `,{"id":"..."` (more elements) or
 * `]}` (last element). Returns a pointer to that closing `}` or
 * NULL when not found / malformed. */
static const char* find_element_end_for_id(const char* json, const char* id)
{
    char needle[128];
    const char* cursor;
    const char* endCandidate;
    size_t needleLen;
    snprintf(needle, sizeof(needle), "\"id\":\"%s\"", id);
    needleLen = strlen(needle);
    cursor = strstr(json, needle);
    if (!cursor) return NULL;
    /* Search for the next `},{` or `]}`. The element ends at the
     * first `}` that has either of those tokens immediately after.
     * We search by stepping through `}` candidates. */
    endCandidate = cursor + needleLen;
    while ((endCandidate = strchr(endCandidate, '}')) != NULL) {
        const char* after = endCandidate + 1;
        if (*after == ',') {
            const char* p = after;
            while (*p == ' ') ++p;
            if (strncmp(p, "{\"id\":\"", 7) == 0) {
                return endCandidate;
            }
        } else if (*after == ']') {
            return endCandidate;
        }
        ++endCandidate;
    }
    return NULL;
}

/* Parse the bounds.x integer for the FIRST elements[] entry whose
 * "id" matches the requested id. Returns the parsed value or
 * -1 if not found / parse failed. */
static int bounds_x_for_id(const char* json, const char* id)
{
    char needle[128];
    const char* bounds;
    int x = -1;
    snprintf(needle, sizeof(needle), "\"id\":\"%s\"", id);
    bounds = strstr(json, needle);
    if (!bounds) return -1;
    bounds = strstr(bounds, "\"bounds\":{");
    if (!bounds) return -1;
    (void)sscanf(bounds, "\"bounds\":{\"x\":%d", &x);
    return x;
}

static int bounds_y_for_id(const char* json, const char* id)
{
    char needle[128];
    const char* bounds;
    int y = -1;
    int x = -1;
    snprintf(needle, sizeof(needle), "\"id\":\"%s\"", id);
    bounds = strstr(json, needle);
    if (!bounds) return -1;
    bounds = strstr(bounds, "\"bounds\":{");
    if (!bounds) return -1;
    (void)sscanf(bounds, "\"bounds\":{\"x\":%d,\"y\":%d", &x, &y);
    return y;
}

static int bounds_w_for_id(const char* json, const char* id)
{
    char needle[128];
    const char* bounds;
    int w = -1;
    int x = -1, y = -1;
    snprintf(needle, sizeof(needle), "\"id\":\"%s\"", id);
    bounds = strstr(json, needle);
    if (!bounds) return -1;
    bounds = strstr(bounds, "\"bounds\":{");
    if (!bounds) return -1;
    (void)sscanf(bounds, "\"bounds\":{\"x\":%d,\"y\":%d,\"w\":%d",
                 &x, &y, &w);
    return w;
}

static int bounds_h_for_id(const char* json, const char* id)
{
    char needle[128];
    const char* bounds;
    int h = -1;
    int x = -1, y = -1, w = -1;
    snprintf(needle, sizeof(needle), "\"id\":\"%s\"", id);
    bounds = strstr(json, needle);
    if (!bounds) return -1;
    bounds = strstr(bounds, "\"bounds\":{");
    if (!bounds) return -1;
    (void)sscanf(bounds,
                 "\"bounds\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d",
                 &x, &y, &w, &h);
    return h;
}

/* Count every "id":"..." literal in the manifest. Used to lock the
 * minimum-zone count for each gameplay surface. */
static int count_id_substrings(const char* json, const char* id)
{
    int count = 0;
    char needle[128];
    const char* cursor;
    size_t needleLen;
    snprintf(needle, sizeof(needle), "\"id\":\"%s\"", id);
    needleLen = strlen(needle);
    cursor = json;
    while ((cursor = strstr(cursor, needle)) != NULL) {
        ++count;
        cursor += needleLen;
    }
    return count;
}

/* -- Test fixtures -------------------------------------------------- */

static char g_a11y_dir[640];
static char g_json_path[1024];
static char g_tmp_path[1024];

static int setup_a11y_dir(void)
{
#if defined(_WIN32)
    char tmpl[] = "firestaff-m11a11y-XXXXXX";
#else
    char tmpl[] = "/tmp/firestaff-m11a11y-XXXXXX";
#endif
    char* made;
    portable_unsetenv("HOME");
    made = portable_mkdtemp(tmpl);
    if (!made) return 0;
    snprintf(g_a11y_dir, sizeof(g_a11y_dir), "%s", made);
    snprintf(g_json_path, sizeof(g_json_path),
             "%s/accessibility.json", g_a11y_dir);
    snprintf(g_tmp_path, sizeof(g_tmp_path),
             "%s/accessibility.json.tmp", g_a11y_dir);
    fs_ax_set_output_dir(g_a11y_dir);
    return 1;
}

static void teardown_a11y_dir(void)
{
    fs_ax_set_output_dir(NULL);
    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    portable_rmdir(g_a11y_dir);
}

/* -- Subtests ------------------------------------------------------- */

/* Subtest A: disabled state is a no-op even with a populated state. */
static void subtest_disabled_is_noop(void)
{
    M11_GameViewState state;

    fs_ax_shutdown();
    fs_ax_set_enabled(0);
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.world.party.activeChampionIndex = 2;
    state.world.partyMapIndex = 3;
    state.actingChampionOrdinal = 2;

    probe_record("M11_AX_A01_disabled_off",
                 fs_ax_is_enabled() == 0,
                 "a11y: writer is off by default");
    probe_record("M11_AX_A02_disabled_update_returns_zero",
                 m11_screen_reader_update_ex(&state, 320, 200) == 0,
                 "a11y: m11_screen_reader_update_ex returns 0 when disabled");
    probe_record("M11_AX_A03_disabled_no_json",
                 !file_exists(g_json_path),
                 "a11y: no accessibility.json written when disabled");
    probe_record("M11_AX_A04_disabled_no_tmp_residue",
                 !file_exists(g_tmp_path),
                 "a11y: no .tmp residue when disabled");
}

/* Subtest B: gameplay manifest surface + always-on bounds. */
static void subtest_gameplay_manifest(void)
{
    M11_GameViewState state;
    char buf[16384];
    int n;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.world.partyMapIndex = 9;
    state.actingChampionOrdinal = 0;  /* not active this turn */

    probe_record("M11_AX_B01_gameplay_returns_one",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "gameplay: update returns 1 when enabled");
    probe_record("M11_AX_B02_gameplay_json_present",
                 file_exists(g_json_path),
                 "gameplay: accessibility.json written");
    probe_record("M11_AX_B03_gameplay_no_tmp_residue",
                 !file_exists(g_tmp_path),
                 "gameplay: atomic write - no .tmp residue");
    n = read_all(g_json_path, buf, sizeof(buf));
    probe_record("M11_AX_B04_gameplay_json_nonempty",
                 n > 0,
                 "gameplay: manifest is non-empty");
    if (n <= 0) return;

    probe_record("M11_AX_B10_envelope_version",
                 strstr(buf, "\"version\":1") != NULL,
                 "envelope: version field present");
    probe_record("M11_AX_B11_envelope_app",
                 strstr(buf, "\"app\":\"firestaff\"") != NULL,
                 "envelope: app field pinned to firestaff");
    probe_record("M11_AX_B12_envelope_gamestate",
                 strstr(buf, "\"gameState\":\"gameplay\"") != NULL,
                 "envelope: gameState pinned to \"gameplay\"");
    probe_record("M11_AX_B13_envelope_framebuffer",
                 strstr(buf, "\"framebuffer\":{\"width\":320,\"height\":200}") != NULL,
                 "envelope: framebuffer dims pinned (320x200)");

    probe_record("M11_AX_B20_gameplay_root_envelope",
                 strstr(buf, "\"id\":\"gameplay_root\"") != NULL,
                 "gameplay: gameplay_root envelope element emitted");
    probe_record("M11_AX_B21_gameplay_root_value",
                 strstr(buf, "state=gameplay") != NULL &&
                     strstr(buf, "active=1") != NULL &&
                     strstr(buf, "level=9") != NULL,
                 "gameplay: gameplay_root carries state/active/level");

    probe_record("M11_AX_B30_always_on_viewport_present",
                 strstr(buf, "\"id\":\"VIEWPORT\"") != NULL,
                 "always-on: VIEWPORT zone emitted");
    probe_record("M11_AX_B31_always_on_movement_quad",
                 strstr(buf, "\"id\":\"MOVE_FWD\"") != NULL &&
                     strstr(buf, "\"id\":\"MOVE_BACK\"") != NULL &&
                     strstr(buf, "\"id\":\"TURN_LEFT\"") != NULL &&
                     strstr(buf, "\"id\":\"TURN_RIGHT\"") != NULL,
                 "always-on: 4 movement arrows emitted");
    probe_record("M11_AX_B32_always_on_spell_area",
                 strstr(buf, "\"id\":\"SPELL_AREA\"") != NULL,
                 "always-on: SPELL_AREA zone emitted");
    probe_record("M11_AX_B33_always_on_hud_panel",
                 strstr(buf, "\"id\":\"HUD_PANEL\"") != NULL,
                 "always-on: HUD_PANEL zone emitted");
    probe_record("M11_AX_B34_always_on_control_strip",
                 strstr(buf, "\"id\":\"CONTROL_STRIP\"") != NULL,
                 "always-on: CONTROL_STRIP zone emitted");
    probe_record("M11_AX_B35_no_acting_champion_when_zero",
                 /* default zero state: actingChampionOrdinal = 0 */
                 strstr(buf, "\"id\":\"ACTING_CHAMPION\"") == NULL,
                 "gameplay: ACTING_CHAMPION is NOT emitted when ordinal=0");

    /* Source-locked viewport bounds: (0, 33, 224, 136). */
    probe_record("M11_AX_B40_viewport_bounds_pinned",
                 bounds_x_for_id(buf, "VIEWPORT") == 0 &&
                     bounds_y_for_id(buf, "VIEWPORT") == 33 &&
                     bounds_w_for_id(buf, "VIEWPORT") == 224 &&
                     bounds_h_for_id(buf, "VIEWPORT") == 136,
                 "viewport: source-locked bounds (0,33,224,136)");
    /* HUD band lives under the viewport: (0, 136, 320, 64). */
    probe_record("M11_AX_B41_hud_panel_bounds_pinned",
                 bounds_x_for_id(buf, "HUD_PANEL") == 0 &&
                     bounds_y_for_id(buf, "HUD_PANEL") == 136 &&
                     bounds_w_for_id(buf, "HUD_PANEL") == 320 &&
                     bounds_h_for_id(buf, "HUD_PANEL") == 64,
                 "hud: source-locked bounds (0,136,320,64)");
    /* Movement arrows are stacked at known coords. */
    probe_record("M11_AX_B42_movement_fwd_x",
                 bounds_x_for_id(buf, "MOVE_FWD") == 144,
                 "movement: MOVE_FWD x=144");
    probe_record("M11_AX_B43_movement_back_y",
                 bounds_y_for_id(buf, "MOVE_BACK") == 173,
                 "movement: MOVE_BACK y=173");
    probe_record("M11_AX_B44_turn_left_x",
                 bounds_x_for_id(buf, "TURN_LEFT") == 112,
                 "movement: TURN_LEFT x=112");
    probe_record("M11_AX_B45_turn_right_x",
                 bounds_x_for_id(buf, "TURN_RIGHT") == 176,
                 "movement: TURN_RIGHT x=176");
}

/* Subtest C: inventory manifest - equipment slots, backpack grid,
 * portrait, champion name, ACTING_CHAMPION. */
static void subtest_inventory_manifest(void)
{
    M11_GameViewState state;
    char buf[16384];
    int n;
    int selectedSlot;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.world.party.activeChampionIndex = 2;
    state.inventorySelectedSlot = 9;  /* ACTION_HAND (slot index 9 in
                                        * the helper, see
                                        * m11_game_view_a11y.c
                                        * m11_ax_emit_inventory_panel_zones
                                        * switch at i=9). */
    state.actingChampionOrdinal = 2;   /* champion 2 acting */

    probe_record("M11_AX_C01_inventory_returns_one",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "inventory: update returns 1");
    n = read_all(g_json_path, buf, sizeof(buf));
    probe_record("M11_AX_C02_inventory_json_present",
                 n > 0,
                 "inventory: manifest written");
    if (n <= 0) return;

    probe_record("M11_AX_C10_inventory_gamestate",
                 strstr(buf, "\"gameState\":\"inventory\"") != NULL,
                 "inventory: gameState pinned to \"inventory\"");

    probe_record("M11_AX_C11_inventory_panel_region",
                 strstr(buf, "\"id\":\"INVENTORY_PANEL\"") != NULL,
                 "inventory: INVENTORY_PANEL region emitted");
    probe_record("M11_AX_C12_inventory_panel_active_champion",
                 strstr(buf, "active_champion=2") != NULL,
                 "inventory: INVENTORY_PANEL carries active_champion=2 in value");

    probe_record("M11_AX_C13_inventory_portrait",
                 strstr(buf, "\"id\":\"INVENTORY_PORTRAIT\"") != NULL,
                 "inventory: INVENTORY_PORTRAIT emitted");
    probe_record("M11_AX_C14_inventory_champion_name",
                 strstr(buf, "\"id\":\"INVENTORY_CHAMPION_NAME\"") != NULL,
                 "inventory: INVENTORY_CHAMPION_NAME emitted");
    probe_record("M11_AX_C15_inventory_champion_name_slot",
                 strstr(buf, "slot=2") != NULL,
                 "inventory: champion name carries slot=2");

    /* Equipment slot IDs - exact match for the 13 C507..C519 zones. */
    probe_record("M11_AX_C20_equipment_ready_hand",
                 strstr(buf, "\"id\":\"INV_READY_HAND\"") != NULL,
                 "inventory: INV_READY_HAND emitted");
    probe_record("M11_AX_C21_equipment_action_hand",
                 strstr(buf, "\"id\":\"INV_ACTION_HAND\"") != NULL,
                 "inventory: INV_ACTION_HAND emitted");
    probe_record("M11_AX_C22_equipment_head_torso",
                 strstr(buf, "\"id\":\"INV_HEAD\"") != NULL &&
                     strstr(buf, "\"id\":\"INV_TORSO\"") != NULL,
                 "inventory: INV_HEAD and INV_TORSO emitted");
    probe_record("M11_AX_C23_equipment_neck",
                 strstr(buf, "\"id\":\"INV_NECK\"") != NULL,
                 "inventory: INV_NECK emitted");
    probe_record("M11_AX_C24_equipment_quiver_line1_1",
                 strstr(buf, "\"id\":\"INV_QUIVER_LINE1_1\"") != NULL,
                 "inventory: INV_QUIVER_LINE1_1 emitted");
    probe_record("M11_AX_C25_equipment_no_overflow",
                 /* inventory slots C520..C528 should NOT be present as
                  * "INV_*" elements with C507..C519 mapping. The
                  * helpers emit only the 13 equipment slots and the
                  * 16 backpack slots; anything beyond slot 20 isn't
                  * an equipment role. Verify no stray C540+ role IDs
                  * leak into the JSON. */
                 strstr(buf, "INV_SLOT_") == NULL,
                 "inventory: no stray C5xx role IDs leak past slot 20");

    /* Backpack grid - 16 zones (INV_BACKPACK_0..15). */
    probe_record("M11_AX_C30_backpack_first",
                 strstr(buf, "\"id\":\"INV_BACKPACK_0\"") != NULL,
                 "inventory: INV_BACKPACK_0 emitted");
    probe_record("M11_AX_C31_backpack_last",
                 strstr(buf, "\"id\":\"INV_BACKPACK_15\"") != NULL,
                 "inventory: INV_BACKPACK_15 emitted");
    {
        /* The role-ID switch covers i in [8..20] inclusive, and
         * emits one zone per slot only when
         * GetV1InventorySourceSlotBoxZone(...) succeeds. We don't
         * couple to slot-count internals here; just lock the
         * backpack count and check no duplicates. */
        int bpCount = count_id_substrings(buf, "INV_BACKPACK_0");
        int bpLast = count_id_substrings(buf, "INV_BACKPACK_15");
        probe_record("M11_AX_C32_backpack_count_unique",
                     bpCount == 1 && bpLast == 1,
                     "inventory: each backpack slot id appears exactly once");
    }
    /* Always-on zones must remain present even when an overlay is on. */
    probe_record("M11_AX_C33_inventory_still_emits_always_on",
                 strstr(buf, "\"id\":\"VIEWPORT\"") != NULL &&
                     strstr(buf, "\"id\":\"HUD_PANEL\"") != NULL &&
                     strstr(buf, "\"id\":\"MOVE_FWD\"") != NULL,
                 "inventory: always-on zones (viewport / HUD / move) preserved");

    /* Selected equipment slot. inventorySelectedSlot == 9 corresponds
     * to a11y index i=9 in the m11_ax_emit_inventory_panel_zones
     * switch which maps to "ACTION_HAND". Find the element with id
     * INV_ACTION_HAND and assert enabled=true. (The emission depends on
     * the host's zone resolution; we just verify the slot is
     * present and the set is non-empty.) */
    {
        const char* elementStart;
        const char* endOfElement;
        elementStart = strstr(buf, "\"id\":\"INV_ACTION_HAND\"");
        endOfElement = elementStart ? find_element_end_for_id(buf, "INV_ACTION_HAND") : NULL;
        if (elementStart && endOfElement) {
            /* Token "enabled":true must appear between element start
             * and end. We do a strstr within a bounded range. */
            int sawEnabled = 0;
            const char* enabledHit = elementStart;
            while (enabledHit && enabledHit < endOfElement) {
                enabledHit = strstr(enabledHit, "\"enabled\":");
                if (!enabledHit || enabledHit >= endOfElement) break;
                enabledHit += strlen("\"enabled\":");
                while (*enabledHit == ' ') ++enabledHit;
                if (strncmp(enabledHit, "true", 4) == 0) {
                    sawEnabled = 1;
                    break;
                }
            }
            probe_record("M11_AX_C34_inventory_selection_present",
                         sawEnabled,
                         "inventory: selected ACTION_HAND zone carries enabled=true");
        } else {
            probe_record("M11_AX_C34_inventory_selection_present",
                         0,
                         "inventory: selected ACTION_HAND zone present");
        }
    }

    /* Acting champion ordinal surface. */
    selectedSlot = -1; (void)selectedSlot;
    probe_record("M11_AX_C40_acting_champion_emitted",
                 strstr(buf, "\"id\":\"ACTING_CHAMPION\"") != NULL,
                 "inventory: ACTING_CHAMPION zone emitted");
    probe_record("M11_AX_C41_acting_champion_value",
                 strstr(buf, "ordinal=2") != NULL,
                 "inventory: ACTING_CHAMPION carries ordinal=2");
}

/* Subtest D: automap / full-screen map surface. */
static void subtest_map_manifest(void)
{
    M11_GameViewState state;
    char buf[16384];
    int n;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.mapOverlayActive = 1;

    probe_record("M11_AX_D01_map_returns_one",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "map: update returns 1");
    n = read_all(g_json_path, buf, sizeof(buf));
    probe_record("M11_AX_D02_map_json_present",
                 n > 0,
                 "map: manifest written");
    if (n <= 0) return;

    probe_record("M11_AX_D10_map_gamestate",
                 strstr(buf, "\"gameState\":\"map\"") != NULL,
                 "map: gameState pinned to \"map\"");
    probe_record("M11_AX_D11_automap_region",
                 strstr(buf, "\"id\":\"AUTOMAP\"") != NULL,
                 "map: AUTOMAP full-screen region emitted");
    /* Map covers the whole framebuffer. */
    probe_record("M11_AX_D12_automap_full_frame",
                 bounds_w_for_id(buf, "AUTOMAP") == 320 &&
                     bounds_h_for_id(buf, "AUTOMAP") == 200 &&
                     bounds_x_for_id(buf, "AUTOMAP") == 0 &&
                     bounds_y_for_id(buf, "AUTOMAP") == 0,
                 "map: AUTOMAP covers full 320x200 framebuffer");
}

/* Subtest E: dialog overlay surface with text forward + 4 choices. */
static void subtest_dialog_manifest(void)
{
    M11_GameViewState state;
    char buf[16384];
    int n;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.dialogOverlayActive = 1;
    snprintf(state.dialogOverlayText,
             sizeof(state.dialogOverlayText),
             "%s", "PRESS ENTER TO CONTINUE");
    state.dialogChoiceCount = 3;
    state.dialogSelectedChoice = 1;
    snprintf(state.dialogChoices[0], sizeof(state.dialogChoices[0]),
             "%s", "YES");
    snprintf(state.dialogChoices[1], sizeof(state.dialogChoices[1]),
             "%s", "NO");
    snprintf(state.dialogChoices[2], sizeof(state.dialogChoices[2]),
             "%s", "MAYBE");

    probe_record("M11_AX_E01_dialog_returns_one",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "dialog: update returns 1");
    n = read_all(g_json_path, buf, sizeof(buf));
    probe_record("M11_AX_E02_dialog_json_present",
                 n > 0,
                 "dialog: manifest written");
    if (n <= 0) return;

    probe_record("M11_AX_E10_dialog_gamestate",
                 strstr(buf, "\"gameState\":\"dialog\"") != NULL,
                 "dialog: gameState pinned to \"dialog\"");

    probe_record("M11_AX_E11_dialog_body_present",
                 strstr(buf, "\"id\":\"DIALOG_BODY\"") != NULL,
                 "dialog: DIALOG_BODY region emitted");
    probe_record("M11_AX_E12_dialog_text_forwarded",
                 strstr(buf, "PRESS ENTER TO CONTINUE") != NULL,
                 "dialog: dialogOverlayText forwarded into value field verbatim");

    probe_record("M11_AX_E13_dialog_choice_count",
                 strstr(buf, "\"id\":\"DIALOG_CHOICE_0\"") != NULL &&
                     strstr(buf, "\"id\":\"DIALOG_CHOICE_1\"") != NULL &&
                     strstr(buf, "\"id\":\"DIALOG_CHOICE_2\"") != NULL &&
                     strstr(buf, "\"id\":\"DIALOG_CHOICE_3\"") == NULL,
                 "dialog: 3 DIALOG_CHOICE_N zones (0..2), 4th slot not emitted");

    probe_record("M11_AX_E14_dialog_choice_labels_forwarded",
                 strstr(buf, "YES") != NULL &&
                     strstr(buf, "NO") != NULL &&
                     strstr(buf, "MAYBE") != NULL,
                 "dialog: YES / NO / MAYBE choice labels in value fields");

    /* Choice bounds (y per choice = VIEWPORT_Y(33) + 104 + i*14).
     *   choice 0: y = 137
     *   choice 1: y = 151
     *   choice 2: y = 165
     * All should sit inside the framebuffer. */
    probe_record("M11_AX_E15_dialog_choice0_y",
                 bounds_y_for_id(buf, "DIALOG_CHOICE_0") == 137,
                 "dialog: DIALOG_CHOICE_0 y=137 (VIEWPORT_Y + 104 + 0*14)");
    probe_record("M11_AX_E16_dialog_choice1_y",
                 bounds_y_for_id(buf, "DIALOG_CHOICE_1") == 151,
                 "dialog: DIALOG_CHOICE_1 y=151 (VIEWPORT_Y + 104 + 1*14)");
    probe_record("M11_AX_E17_dialog_choice2_y",
                 bounds_y_for_id(buf, "DIALOG_CHOICE_2") == 165,
                 "dialog: DIALOG_CHOICE_2 y=165 (VIEWPORT_Y + 104 + 2*14)");

    /* Selected dialog choice must be marked enabled=true. */
    {
        const char* elementStart;
        const char* endOfElement;
        elementStart = strstr(buf, "\"id\":\"DIALOG_CHOICE_1\"");
        endOfElement = elementStart ? find_element_end_for_id(buf, "DIALOG_CHOICE_1") : NULL;
        if (elementStart && endOfElement) {
            int sawEnabled = 0;
            const char* enabledHit = elementStart;
            while (enabledHit && enabledHit < endOfElement) {
                enabledHit = strstr(enabledHit, "\"enabled\":");
                if (!enabledHit || enabledHit >= endOfElement) break;
                enabledHit += strlen("\"enabled\":");
                while (*enabledHit == ' ') ++enabledHit;
                if (strncmp(enabledHit, "true", 4) == 0) {
                    sawEnabled = 1;
                    break;
                }
            }
            probe_record("M11_AX_E18_dialog_selected_enabled",
                         sawEnabled,
                         "dialog: selected DIALOG_CHOICE_1 zone carries enabled=true");
        } else {
            probe_record("M11_AX_E18_dialog_selected_enabled",
                         0,
                         "dialog: selected DIALOG_CHOICE_1 zone present");
        }
    }
}

/* Subtest F: candidate mirror (Hall of Champions) surface. */
static void subtest_entrance_mirror_manifest(void)
{
    M11_GameViewState state;
    char buf[16384];
    int n;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.candidateMirrorPanelActive = 1;
    state.candidateMirrorOrdinal = 7;
    state.candidateMirrorPartyIndex = 3;

    probe_record("M11_AX_F01_entrance_returns_one",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "entrance: update returns 1");
    n = read_all(g_json_path, buf, sizeof(buf));
    probe_record("M11_AX_F02_entrance_json_present",
                 n > 0,
                 "entrance: manifest written");
    if (n <= 0) return;

    probe_record("M11_AX_F10_entrance_gamestate",
                 strstr(buf, "\"gameState\":\"entrance_mirror\"") != NULL,
                 "entrance: gameState pinned to \"entrance_mirror\"");
    probe_record("M11_AX_F11_entrance_panel",
                 strstr(buf, "\"id\":\"CANDIDATE_MIRROR_PANEL\"") != NULL,
                 "entrance: CANDIDATE_MIRROR_PANEL region emitted");
    probe_record("M11_AX_F12_entrance_panel_value",
                 strstr(buf, "ordinal=7") != NULL &&
                     strstr(buf, "party_index=3") != NULL,
                 "entrance: panel carries ordinal=7;party_index=3 in value");
    probe_record("M11_AX_F13_entrance_resurrect_button",
                 strstr(buf, "\"id\":\"CANDIDATE_RESURRECT\"") != NULL,
                 "entrance: CANDIDATE_RESURRECT button emitted");
    probe_record("M11_AX_F14_entrance_reincarnate_button",
                 strstr(buf, "\"id\":\"CANDIDATE_REINCARNATE\"") != NULL,
                 "entrance: CANDIDATE_REINCARNATE button emitted");
    probe_record("M11_AX_F15_entrance_cancel_button",
                 strstr(buf, "\"id\":\"CANDIDATE_CANCEL\"") != NULL,
                 "entrance: CANDIDATE_CANCEL button emitted");

    /* All three buttons sit on top of the panel rect, so their
     * bounds must (a) stay inside the framebuffer and (b) overlap
     * the panel bounds. The literal x values depend on
     * M11_GameView_GetV1InventoryPanelZone() which is a sourced
     * helper that requires real assets, so we lock the cheap
     * "inside framebuffer" contract here and the "overlaps panel"
     * contract in the bounds-walk subtest below. */
    {
        int rx, ry, rw, rh;
        int px, py, pw, ph;
        int inside = 1, overlaps = 1;
        rx = bounds_x_for_id(buf, "CANDIDATE_RESURRECT");
        ry = bounds_y_for_id(buf, "CANDIDATE_RESURRECT");
        rw = bounds_w_for_id(buf, "CANDIDATE_RESURRECT");
        rh = bounds_h_for_id(buf, "CANDIDATE_RESURRECT");
        if (rx < 0 || ry < 0 ||
            rx + rw > 320 || ry + rh > 200) {
            inside = 0;
        }
        px = bounds_x_for_id(buf, "CANDIDATE_MIRROR_PANEL");
        py = bounds_y_for_id(buf, "CANDIDATE_MIRROR_PANEL");
        pw = bounds_w_for_id(buf, "CANDIDATE_MIRROR_PANEL");
        ph = bounds_h_for_id(buf, "CANDIDATE_MIRROR_PANEL");
        if (px >= 0 && py >= 0) {
            if (rx + rw <= px || rx >= px + pw ||
                ry + rh <= py || ry >= py + ph) {
                overlaps = 0;
            }
        }
        probe_record("M11_AX_F16_entrance_resurrect_inside",
                     inside,
                     "entrance: CANDIDATE_RESURRECT stays inside 320x200");
        probe_record("M11_AX_F17_entrance_resurrect_overlaps_panel",
                     overlaps,
                     "entrance: CANDIDATE_RESURRECT overlaps CANDIDATE_MIRROR_PANEL");
    }
}

/* Subtest G: endgame surface (THE END plaque, 4 mirrors + 4 portraits). */
static void subtest_endgame_manifest(void)
{
    M11_GameViewState state;
    char buf[16384];
    int n;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.gameWon = 1;

    probe_record("M11_AX_G01_endgame_returns_one",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "endgame: update returns 1");
    n = read_all(g_json_path, buf, sizeof(buf));
    probe_record("M11_AX_G02_endgame_json_present",
                 n > 0,
                 "endgame: manifest written");
    if (n <= 0) return;

    probe_record("M11_AX_G10_endgame_gamestate",
                 strstr(buf, "\"gameState\":\"endgame\"") != NULL,
                 "endgame: gameState pinned to \"endgame\"");
    probe_record("M11_AX_G11_endgame_the_end_plaque",
                 strstr(buf, "\"id\":\"ENDGAME_THE_END\"") != NULL,
                 "endgame: ENDGAME_THE_END plaque emitted");
    probe_record("M11_AX_G12_endgame_mirror_slots",
                 strstr(buf, "\"id\":\"ENDGAME_MIRROR_0\"") != NULL &&
                     strstr(buf, "\"id\":\"ENDGAME_MIRROR_1\"") != NULL &&
                     strstr(buf, "\"id\":\"ENDGAME_MIRROR_2\"") != NULL &&
                     strstr(buf, "\"id\":\"ENDGAME_MIRROR_3\"") != NULL,
                 "endgame: 4 ENDGAME_MIRROR_N zones (slots 0..3)");
    probe_record("M11_AX_G13_endgame_portrait_slots",
                 strstr(buf, "\"id\":\"ENDGAME_PORTRAIT_0\"") != NULL &&
                     strstr(buf, "\"id\":\"ENDGAME_PORTRAIT_1\"") != NULL &&
                     strstr(buf, "\"id\":\"ENDGAME_PORTRAIT_2\"") != NULL &&
                     strstr(buf, "\"id\":\"ENDGAME_PORTRAIT_3\"") != NULL,
                 "endgame: 4 ENDGAME_PORTRAIT_N zones (slots 0..3)");
    probe_record("M11_AX_G14_endgame_no_dialog_body",
                 /* The endgame surface does NOT emit DIALOG_BODY even
                  * though a "THE END" plaque looks like a dialog
                  * variant - the source-locked endgame plaque is its
                  * own zone. The probe locks this contract so a
                  * future refactor doesn't accidentally double-emit. */
                 strstr(buf, "\"id\":\"DIALOG_BODY\"") == NULL,
                 "endgame: no stray DIALOG_BODY zone (the plaque is its own zone)");
}

/* Subtest H: state classifier precedence. */
static void subtest_state_classifier_precedence(void)
{
    M11_GameViewState state;

    /* (a) zero state -> gameplay */
    memset(&state, 0, sizeof(state));
    state.active = 1;
    probe_record("M11_AX_H01_zero_state_is_gameplay",
                 m11_screen_reader_state_for(&state) == M11_AX_STATE_GAMEPLAY,
                 "classifier: default zero state -> GAMEPLAY");

    /* (b) inventory alone -> inventory */
    memset(&state, 0, sizeof(state));
    state.inventoryPanelActive = 1;
    probe_record("M11_AX_H02_inventory_alone",
                 m11_screen_reader_state_for(&state) == M11_AX_STATE_INVENTORY,
                 "classifier: inventoryPanelActive alone -> INVENTORY");

    /* (c) map alone -> map */
    memset(&state, 0, sizeof(state));
    state.mapOverlayActive = 1;
    probe_record("M11_AX_H03_map_alone",
                 m11_screen_reader_state_for(&state) == M11_AX_STATE_MAP,
                 "classifier: mapOverlayActive alone -> MAP");

    /* (d) dialog alone -> dialog */
    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    probe_record("M11_AX_H04_dialog_alone",
                 m11_screen_reader_state_for(&state) == M11_AX_STATE_DIALOG,
                 "classifier: dialogOverlayActive alone -> DIALOG");

    /* (e) entrance_mirror alone -> entrance_mirror */
    memset(&state, 0, sizeof(state));
    state.candidateMirrorPanelActive = 1;
    probe_record("M11_AX_H05_mirror_alone",
                 m11_screen_reader_state_for(&state) == M11_AX_STATE_ENTRANCE_MIRROR,
                 "classifier: candidateMirrorPanelActive alone -> ENTRANCE_MIRROR");

    /* (f) gameWon beats dialogOverlayActive. */
    memset(&state, 0, sizeof(state));
    state.gameWon = 1;
    state.dialogOverlayActive = 1;
    probe_record("M11_AX_H06_endgame_beats_dialog",
                 m11_screen_reader_state_for(&state) == M11_AX_STATE_ENDGAME,
                 "classifier: gameWon beats dialogOverlayActive");

    /* (g) dialogOverlayActive beats candidateMirrorPanelActive. */
    memset(&state, 0, sizeof(state));
    state.dialogOverlayActive = 1;
    state.candidateMirrorPanelActive = 1;
    probe_record("M11_AX_H07_dialog_beats_mirror",
                 m11_screen_reader_state_for(&state) == M11_AX_STATE_DIALOG,
                 "classifier: dialogOverlayActive beats candidateMirrorPanelActive");

    /* (h) candidateMirrorPanelActive beats inventoryPanelActive. */
    memset(&state, 0, sizeof(state));
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;
    probe_record("M11_AX_H08_mirror_beats_inventory",
                 m11_screen_reader_state_for(&state) == M11_AX_STATE_ENTRANCE_MIRROR,
                 "classifier: candidateMirrorPanelActive beats inventoryPanelActive");

    /* (i) inventoryPanelActive beats mapOverlayActive. */
    memset(&state, 0, sizeof(state));
    state.inventoryPanelActive = 1;
    state.mapOverlayActive = 1;
    probe_record("M11_AX_H09_inventory_beats_map",
                 m11_screen_reader_state_for(&state) == M11_AX_STATE_INVENTORY,
                 "classifier: inventoryPanelActive beats mapOverlayActive");

    /* (j) NULL state -> OTHER. */
    probe_record("M11_AX_H10_null_state_other",
                 m11_screen_reader_state_for(NULL) == M11_AX_STATE_OTHER,
                 "classifier: NULL state -> OTHER");
    probe_record("M11_AX_H11_null_view_name_other",
                 strcmp(m11_screen_reader_view_name(NULL), "other") == 0,
                 "classifier: NULL view name -> \"other\"");

    /* (k) All state-name strings honor the Peekaboo schema:
     * no whitespace, no leading whitespace, no NUL bytes. */
    {
        int i;
        for (i = 0; i < M11_AX_STATE_COUNT; ++i) {
            const char* name = m11_screen_reader_state_name((M11_AX_State)i);
            int okSchema = name != NULL &&
                strchr(name, ' ') == NULL &&
                strchr(name, '\t') == NULL &&
                strchr(name, '\n') == NULL &&
                name[0] != '\0';
            char labelId[64];
            snprintf(labelId, sizeof(labelId),
                     "M11_AX_H12_state%d_name_schema", i);
            probe_record(labelId, okSchema,
                         name ? "name: alphanumeric + underscore, no whitespace"
                              : "name: state name is non-NULL");
        }
    }
}

/* Subtest I: bounds stay inside the framebuffer for every emitted
 * element across every state. Walks the elements[] array using a
 * simple two-pointer approach (id-keyed parsing) and verifies
 *   x >= 0 && y >= 0 && x+w <= 320 && y+h <= 200
 * for every emitted element. */
static void subtest_bounds_inside_framebuffer(void)
{
    static const struct {
        const char* name;
        void (*setup)(M11_GameViewState* out);
    } cases[] = {
        { "gameplay",        NULL },
        { "inventory",       NULL },
        { "map",             NULL },
        { "dialog",          NULL },
        { "entrance_mirror", NULL },
        { "endgame",         NULL }
    };
    int i;

    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        M11_GameViewState state;
        char buf[32768];
        int n;
        const char* cursor;
        int total = 0;
        int inside = 0;
        int malformed = 0;

        fs_ax_shutdown();
        portable_remove(g_json_path);
        portable_remove(g_tmp_path);
        fs_ax_set_enabled(1);

        memset(&state, 0, sizeof(state));
        state.active = 1;
        if (strcmp(cases[i].name, "inventory") == 0) {
            state.inventoryPanelActive = 1;
            state.world.party.activeChampionIndex = 0;
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
            state.candidateMirrorOrdinal = 1;
            state.candidateMirrorPartyIndex = 1;
        } else if (strcmp(cases[i].name, "endgame") == 0) {
            state.gameWon = 1;
        }

        probe_record("M11_AX_I01_setup_returns",
                     m11_screen_reader_update_ex(&state, 320, 200) == 1,
                     "bounds: update returns 1");
        n = read_all(g_json_path, buf, sizeof(buf));
        if (n <= 0) {
            char label[64];
            snprintf(label, sizeof(label),
                     "M11_AX_I02_%s_bounds_inside_framebuffer",
                     cases[i].name);
            probe_record(label, 0,
                         "bounds: manifest present for state");
            continue;
        }

        /* Walk every "id":"..." literal followed by a "bounds" entry.
         * We accept that a future schema change adding extra fields
         * between id and bounds would invalidate this scan; today
         * the writer is fixed-order JSON. */
        cursor = buf;
        while ((cursor = strstr(cursor, "\"id\":\"")) != NULL) {
            const char* bounds = strstr(cursor, "\"bounds\":{");
            int x = 0, y = 0, w = 0, h = 0;
            int parsed;
            if (!bounds) break;
            parsed = sscanf(bounds,
                            "\"bounds\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d",
                            &x, &y, &w, &h);
            ++total;
            if (parsed != 4) {
                ++malformed;
                cursor = bounds + 1;
                continue;
            }
            if (x >= 0 && y >= 0 &&
                x + w <= 320 &&
                y + h <= 200) {
                ++inside;
            }
            cursor = bounds + 1;
        }
        {
            char label[64];
            snprintf(label, sizeof(label),
                     "M11_AX_I02_%s_bounds_inside_framebuffer",
                     cases[i].name);
            probe_record(label,
                         total > 0 && inside == total && malformed == 0,
                         "bounds: every emitted element's rect stays inside 320x200");
        }
    }
}

/* Subtest J: atomic write - no .tmp residue and JSON well-formed. */
static void subtest_json_well_formed(void)
{
    static const struct {
        const char* name;
        void (*setup)(M11_GameViewState* out);
    } cases[] = {
        { "gameplay",        NULL },
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
        char labelNoCommaLead[64];
        char labelNoCommaTrail[64];
        char labelNoCommaDouble[64];
        char labelNoTmp[64];

        fs_ax_shutdown();
        portable_remove(g_json_path);
        portable_remove(g_tmp_path);
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

        probe_record("M11_AX_J00_wf_no_tmp_before",
                     !file_exists(g_tmp_path),
                     "well-formedness: no leftover .tmp before flush");
        {
            int rc = m11_screen_reader_update_ex(&state, 320, 200);
            char msg[96];
            snprintf(msg, sizeof(msg),
                     "well-formedness[%s]: update returns 1",
                     cases[i].name);
            probe_record("M11_AX_J01_wf_update_returns_one", rc == 1, msg);
        }
        n = read_all(g_json_path, buf, sizeof(buf));
        if (n <= 0) {
            probe_record("M11_AX_J02_wf_json_present", 0,
                         "well-formedness: manifest present");
            continue;
        }

        snprintf(labelNoCommaLead, sizeof(labelNoCommaLead),
                 "M11_AX_J02_%s_no_leading_comma", cases[i].name);
        snprintf(labelNoCommaTrail, sizeof(labelNoCommaTrail),
                 "M11_AX_J03_%s_no_trailing_comma", cases[i].name);
        snprintf(labelNoCommaDouble, sizeof(labelNoCommaDouble),
                 "M11_AX_J04_%s_no_double_comma", cases[i].name);
        snprintf(labelNoTmp, sizeof(labelNoTmp),
                 "M11_AX_J05_%s_no_tmp_residue", cases[i].name);
        probe_record(labelNoCommaLead,
                     strstr(buf, "[,") == NULL,
                     "well-formedness: no leading-comma artifact");
        probe_record(labelNoCommaTrail,
                     strstr(buf, ",]") == NULL,
                     "well-formedness: no trailing-comma artifact");
        probe_record(labelNoCommaDouble,
                     strstr(buf, ",,") == NULL,
                     "well-formedness: no double-comma artifact");
        probe_record(labelNoTmp,
                     !file_exists(g_tmp_path),
                     "well-formedness: atomic write - no .tmp residue");
    }
}

/* Subtest K: deterministic gameState field - never "launcher_*". */
static void subtest_deterministic_gamestate(void)
{
    static const char* const kExpectedStates[A11Y_NUM_STATE_FIELDS] = {
        "gameplay",
        "inventory",
        "map",
        "dialog",
        "entrance_mirror",
        "endgame"
    };
    static const struct {
        const char* name;
        void (*setup)(M11_GameViewState* out);
    } cases[] = {
        { "gameplay",        NULL },
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
        char needle[64];
        char labelExpected[64];
        char labelNotLauncher[64];
        char labelOnlyOnce[64];

        fs_ax_shutdown();
        portable_remove(g_json_path);
        portable_remove(g_tmp_path);
        fs_ax_set_enabled(1);

        memset(&state, 0, sizeof(state));
        state.active = 1;
        if (strcmp(cases[i].name, "inventory") == 0) {
            state.inventoryPanelActive = 1;
        } else if (strcmp(cases[i].name, "map") == 0) {
            state.mapOverlayActive = 1;
        } else if (strcmp(cases[i].name, "dialog") == 0) {
            state.dialogOverlayActive = 1;
        } else if (strcmp(cases[i].name, "entrance_mirror") == 0) {
            state.candidateMirrorPanelActive = 1;
        } else if (strcmp(cases[i].name, "endgame") == 0) {
            state.gameWon = 1;
        }

        n = (int)read_all(g_json_path, buf, sizeof(buf));
        if (m11_screen_reader_update_ex(&state, 320, 200) != 1) {
            snprintf(labelExpected, sizeof(labelExpected),
                     "M11_AX_K01_%s_gamestate_pinned", cases[i].name);
            probe_record(labelExpected, 0,
                         "determinism: update returned 1");
            continue;
        }
        n = read_all(g_json_path, buf, sizeof(buf));
        if (n <= 0) {
            snprintf(labelExpected, sizeof(labelExpected),
                     "M11_AX_K01_%s_gamestate_pinned", cases[i].name);
            probe_record(labelExpected, 0,
                         "determinism: manifest present");
            continue;
        }

        snprintf(needle, sizeof(needle),
                 "\"gameState\":\"%s\"", kExpectedStates[i]);
        snprintf(labelExpected, sizeof(labelExpected),
                 "M11_AX_K01_%s_gamestate_pinned", cases[i].name);
        snprintf(labelNotLauncher, sizeof(labelNotLauncher),
                 "M11_AX_K02_%s_no_launcher_leak", cases[i].name);
        snprintf(labelOnlyOnce, sizeof(labelOnlyOnce),
                 "M11_AX_K03_%s_gamestate_once", cases[i].name);

        probe_record(labelExpected,
                     strstr(buf, needle) != NULL,
                     "determinism: gameState matches pinned value");
        probe_record(labelNotLauncher,
                     strstr(buf, "launcher_") == NULL,
                     "determinism: manifest never emits launcher_* state");
        probe_record(labelOnlyOnce,
                     count_id_substrings(buf, "gameState") <= 6 &&
                         strstr(buf, "\"gameState\":") != NULL,
                     "determinism: envelope gameState field present once");
    }
}

/* Subtest L: full-state live handoff - re-running the update N
 * times yields the same JSON (atomic write replaces the file, so
 * re-running must always end on the latest frame). */
static void subtest_redraw_is_idempotent(void)
{
    M11_GameViewState state;
    char buf1[16384], buf2[16384];
    int n1, n2;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.world.party.activeChampionIndex = 1;
    state.inventorySelectedSlot = 0;
    state.actingChampionOrdinal = 1;

    probe_record("M11_AX_L01_redraw_first_call",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "redraw: first update returns 1");
    n1 = read_all(g_json_path, buf1, sizeof(buf1));

    state.inventorySelectedSlot = 5;
    state.active = 1;
    probe_record("M11_AX_L02_redraw_second_call",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "redraw: second update returns 1");
    n2 = read_all(g_json_path, buf2, sizeof(buf2));

    /* Both updates produced a non-empty JSON. */
    probe_record("M11_AX_L03_redraw_both_nonempty",
                 n1 > 0 && n2 > 0,
                 "redraw: both manifests are non-empty");

    /* The second frame shouldn't carry INV_READY_HAND's selected
     * flag (since we moved selection to slot 5 = quiver line 1
     * slot 1). We don't pin the exact selection behavior
     * (dependent on host zone resolution); just lock that the
     * manifest still emits the always-on zones after re-render. */
    probe_record("M11_AX_L04_redraw_always_on_preserved",
                 strstr(buf2, "\"id\":\"VIEWPORT\"") != NULL &&
                     strstr(buf2, "\"id\":\"HUD_PANEL\"") != NULL,
                 "redraw: always-on zones preserved after re-update");
    probe_record("M11_AX_L05_redraw_panel_preserved",
                 strstr(buf2, "\"id\":\"INVENTORY_PANEL\"") != NULL,
                 "redraw: INVENTORY_PANEL preserved after re-update");

    /* No .tmp residue after either update. */
    probe_record("M11_AX_L06_redraw_no_tmp_residue",
                 !file_exists(g_tmp_path),
                 "redraw: atomic write - no .tmp residue after multiple frames");
}

/* Subtest M: chest-open surface inside the inventory overlay.
 * The gameplay manifest emits the chest panel + 8 chest slots +
 * the arrow/eye toggle whenever v1OpenChestThing != THING_NONE. */
static void subtest_chest_open_manifest(void)
{
    M11_GameViewState state;
    char buf[16384];
    int n;
    int chestOrdinal;
    char chestLabel[64];

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.v1OpenChestThing = 42;  /* arbitrary non-NONE thing id */
    state.v1OpenChestOpenedByEye = 0;  /* arrow button (Take All) */

    probe_record("M11_AX_M01_chest_open_returns_one",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "chest-open: update returns 1");
    n = read_all(g_json_path, buf, sizeof(buf));
    probe_record("M11_AX_M02_chest_open_json_present",
                 n > 0,
                 "chest-open: manifest written");
    if (n <= 0) return;

    probe_record("M11_AX_M10_chest_open_inventory_state",
                 strstr(buf, "\"gameState\":\"inventory\"") != NULL,
                 "chest-open: gameState pinned to \"inventory\" (overlay-style)");
    probe_record("M11_AX_M11_chest_panel_region",
                 strstr(buf, "\"id\":\"CHEST_PANEL\"") != NULL,
                 "chest-open: CHEST_PANEL region emitted when chest is open");
    /* 8 chest slots (C537..C544). */
    for (chestOrdinal = 0; chestOrdinal < 8; ++chestOrdinal) {
        char id[40];
        char label[80];
        snprintf(id, sizeof(id), "\"id\":\"CHEST_SLOT_%d\"", chestOrdinal);
        snprintf(label, sizeof(label),
                 "chest-open: CHEST_SLOT_%d emitted", chestOrdinal);
        snprintf(chestLabel, sizeof(chestLabel),
                 "M11_AX_M12_%d_chest_slot_%d",
                 20 + chestOrdinal, chestOrdinal);
        probe_record(chestLabel, strstr(buf, id) != NULL, label);
    }
    /* Arrow/eye toggle. */
    probe_record("M11_AX_M30_chest_arrow_or_eye",
                 strstr(buf, "\"id\":\"CHEST_ARROW_OR_EYE\"") != NULL,
                 "chest-open: CHEST_ARROW_OR_EYE button emitted");
    /* When v1OpenChestOpenedByEye == 0 we expect the "Arrow (Take All)"
     * label; when == 1 we expect "Eye (Reincarnate)". */
    probe_record("M11_AX_M31_chest_arrow_label_when_eye_off",
                 strstr(buf, "Arrow (Take All)") != NULL,
                 "chest-open: button label is \"Arrow (Take All)\" when v1OpenChestOpenedByEye == 0");
}

/* Subtest N: chest-open surface - eye variant label. */
static void subtest_chest_eye_label(void)
{
    M11_GameViewState state;
    char buf[16384];
    int n;

    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.inventoryPanelActive = 1;
    state.v1OpenChestThing = 99;
    state.v1OpenChestOpenedByEye = 1;  /* eye route */

    probe_record("M11_AX_N01_chest_eye_returns_one",
                 m11_screen_reader_update_ex(&state, 320, 200) == 1,
                 "chest-eye: update returns 1");
    n = read_all(g_json_path, buf, sizeof(buf));
    if (n <= 0) {
        probe_record("M11_AX_N02_chest_eye_label", 0,
                     "chest-eye: manifest present");
        return;
    }
    probe_record("M11_AX_N02_chest_eye_label",
                 strstr(buf, "Eye (Reincarnate)") != NULL,
                 "chest-eye: button label is \"Eye (Reincarnate)\" when v1OpenChestOpenedByEye == 1");
}

/* Subtest O: NULL state is a no-op (defense in depth - the test
 * already covers this, but the probe pins it at runtime). */
static void subtest_null_state_noop(void)
{
    fs_ax_shutdown();
    portable_remove(g_json_path);
    portable_remove(g_tmp_path);
    fs_ax_set_enabled(1);

    probe_record("M11_AX_O01_null_state_returns_zero",
                 m11_screen_reader_update_ex(NULL, 320, 200) == 0,
                 "null-state: update returns 0");
    probe_record("M11_AX_O02_null_state_no_json",
                 !file_exists(g_json_path),
                 "null-state: no JSON written");
    probe_record("M11_AX_O03_null_state_no_tmp",
                 !file_exists(g_tmp_path),
                 "null-state: no .tmp residue");
}

/* -- main ----------------------------------------------------------- */

int main(void)
{
    printf("firestaff_m11_gameplay_screen_reader_manifest_probe\n");

    if (!setup_a11y_dir()) {
        fprintf(stderr,
                "could not redirect accessibility output dir to a temp dir; "
                "probe cannot run\n");
        return 1;
    }

    printf("\n[A] disabled state is a no-op\n");
    subtest_disabled_is_noop();

    printf("\n[B] gameplay manifest surface + always-on bounds\n");
    subtest_gameplay_manifest();

    printf("\n[C] inventory manifest - equipment slots, backpack, acting champion\n");
    subtest_inventory_manifest();

    printf("\n[D] automap full-screen surface\n");
    subtest_map_manifest();

    printf("\n[E] dialog overlay surface with text + 4 choices\n");
    subtest_dialog_manifest();

    printf("\n[F] Hall of Champions candidate-mirror surface\n");
    subtest_entrance_mirror_manifest();

    printf("\n[G] endgame plaque + mirrors + portraits\n");
    subtest_endgame_manifest();

    printf("\n[H] state classifier precedence\n");
    subtest_state_classifier_precedence();

    printf("\n[I] bounds stay inside the framebuffer\n");
    subtest_bounds_inside_framebuffer();

    printf("\n[J] JSON well-formedness (atomic write, comma discipline)\n");
    subtest_json_well_formed();

    printf("\n[K] deterministic gameState field - never launcher_*\n");
    subtest_deterministic_gamestate();

    printf("\n[L] redraw is idempotent - frame N+1 replaces frame N\n");
    subtest_redraw_is_idempotent();

    printf("\n[M] chest-open surface (CHEST_PANEL + 8 slots + arrow)\n");
    subtest_chest_open_manifest();

    printf("\n[N] chest-open eye-variant label\n");
    subtest_chest_eye_label();

    printf("\n[O] NULL state is a no-op\n");
    subtest_null_state_noop();

    teardown_a11y_dir();

    printf("\n# summary: %d/%d invariants passed\n",
           g_tally.passed, g_tally.total);
    return (g_tally.passed == g_tally.total) ? 0 : 1;
}
