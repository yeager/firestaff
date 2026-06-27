/*
 * test_theron_v2_hud_widget_assets_pc34.c — Theron V2 HUD Widget Asset Manifest gate
 *
 * Data-free unit test. Builds synthetic manifest files in a temp
 * scratch directory and exercises every classification/gate branch:
 *
 *   1. No manifest path set → NO_MANIFEST gate, all slots MISSING
 *   2. Manifest path set but file missing → NO_MANIFEST gate
 *   3. Empty manifest → PLACEHOLDER gate
 *   4. Manifest with one valid placeholder slot → PLACEHOLDER gate
 *   5. Manifest with placeholder + missing source_file → PARTIAL
 *   6. Manifest with all slots fully REAL → COMPLETE gate
 *   7. Mixed manifest → PARTIAL gate
 *   8. Manifest with malformed entries (missing fields) → PARTIAL gate
 *   9. validate_manifest() returns -1/0/1 as documented
 *  10. Slot names, class names, gate names are stable and non-empty
 *  11. uses_placeholder() returns 1 for non-REAL, 0 for REAL
 *  12. get_slot_info() populates inline fields when slot is present
 *  13. real_count() returns slot counts correctly
 *  14. installed flag mirrors gate state
 *  15. NULL / out-of-range inputs are safe (no crash)
 *  16. Source-evidence citation contains the documented anchors
 *  17. Theron-specific slot names (compass_rose, quest_items,
 *      dungeon_progress, relic_counter, rune_indicator,
 *      champion_bars, action_strip) are stable
 *
 * Source:
 *   - THQUEST.ASM T520/T560/T600/T700/T800/T900
 *   - HuC6260/HuC6270 datasheet (PC Engine VDC + VCE)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 *   - dmweb Theron overview (7 dungeons + 7 relic goals + rune magic)
 *   - docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *   - src/theron/theron_v2_hud_overlay_pc34.c (procedural fallback)
 *   - include/theron_v22_modern_assets_pc34.h (sibling V2.2 manifest)
 *   - include/dm2_v2_hud_widget_assets.h (sibling Phase 3 gate pattern)
 */

#include "theron_v2_hud_widget_assets_pc34.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ── Test harness ───────────────────────────────────────────────── */

static int s_pass = 0;
static int s_fail = 0;

#define CHECK(expr, msg)                                                  \
    do {                                                                  \
        s_pass++;                                                         \
        if (!(expr)) {                                                    \
            fprintf(stderr, "FAIL %s:%d: %s — %s\n",                      \
                    __FILE__, __LINE__, #expr, (msg));                    \
            s_fail++;                                                     \
        }                                                                 \
    } while (0)

/* Helper: write a string to a file (returns 1 on success). */
static int write_file(const char* path, const char* content) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, fp);
    fclose(fp);
    return (written == len);
}

/* Helper: build the manifest path that the module resolves for a given
 * data dir. Mirrors theron_v2_hud_widget_assets_set_manifest_path. */
static void build_expected_manifest_path(char* out, size_t outSize,
                                         const char* dataDir) {
    /* dataDir = <root>/data/theron → manifest = <root>/assets/theron/hud/hud_widget_manifest.json
     * Walk up two parents. */
    char a[1024], b[1024];
    const char* slash;
    slash = strrchr(dataDir, '/');
    if (!slash) { snprintf(out, outSize, "%s/assets/theron/hud/hud_widget_manifest.json", dataDir); return; }
    size_t la = (size_t)(slash - dataDir);
    if (la >= sizeof(a)) la = sizeof(a) - 1U;
    memcpy(a, dataDir, la); a[la] = '\0';
    slash = strrchr(a, '/');
    if (!slash) { snprintf(out, outSize, "%s/assets/theron/hud/hud_widget_manifest.json", dataDir); return; }
    size_t lb = (size_t)(slash - a);
    if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
    memcpy(b, a, lb); b[lb] = '\0';
    snprintf(out, outSize, "%s/assets/theron/hud/hud_widget_manifest.json", b);
}

/* Helper: clean a scratch dir before each scenario. We also remove
 * the asset tree the module looks at (~/.firestaff/assets/theron/hud/…)
 * so that running the test suite twice in a row produces identical
 * results — the all-REAL and other tests write files there that
 * would otherwise leak across runs. */
static void clean_scratch(void) {
    system("rm -rf /tmp/scratch/theron_hwa_test");
    system("rm -rf /tmp/scratch/assets /tmp/scratch/firestaff-data");
}

/* ── Tests ──────────────────────────────────────────────────────── */

static void test_unset_path_is_safe(void) {
    theron_v2_hud_widget_assets_set_manifest_path(NULL);
    theron_v2_hud_widget_assets_set_manifest_path("");
    const char* p = theron_v2_hud_widget_assets_get_manifest_path();
    CHECK(p && p[0] == '\0', "unset path returns empty string");

    int total = 0;
    int real = theron_v2_hud_widget_assets_real_count(&total);
    CHECK(real == 0, "real_count=0 with no manifest");
    CHECK(total == 0, "total=0 with no manifest");

    Theron_V2_HudWidgetGate gate = theron_v2_hud_widget_assets_gate();
    CHECK(gate == THERON_V2_HUD_WIDGET_GATE_NO_MANIFEST,
          "gate=NO_MANIFEST when unset");

    for (size_t i = 0; i < THERON_V2_HUD_WIDGET_COUNT; ++i) {
        Theron_V2_HudWidgetClass cls =
            theron_v2_hud_widget_assets_classify_slot((Theron_V2_HudWidgetSlot)i);
        CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_MISSING,
              "all slots MISSING when manifest unset");
        CHECK(theron_v2_hud_widget_assets_uses_placeholder(
                (Theron_V2_HudWidgetSlot)i) == 1,
              "uses_placeholder=1 when manifest unset");
    }
}

static void test_set_path_resolves_correctly(void) {
    theron_v2_hud_widget_assets_set_manifest_path("/tmp/scratch/firestaff-data/theron");
    char expected[1024];
    build_expected_manifest_path(expected, sizeof(expected),
                                  "/tmp/scratch/firestaff-data/theron");
    const char* actual = theron_v2_hud_widget_assets_get_manifest_path();
    CHECK(actual && strcmp(actual, expected) == 0,
          "manifest path resolves correctly under assets/theron/hud/");
}

static void test_missing_manifest_file_yields_no_manifest(void) {
    clean_scratch();
    theron_v2_hud_widget_assets_set_manifest_path("/tmp/scratch/firestaff-data/theron");
    /* Don't actually create the manifest path */
    Theron_V2_HudWidgetGate gate = theron_v2_hud_widget_assets_gate();
    CHECK(gate == THERON_V2_HUD_WIDGET_GATE_NO_MANIFEST,
          "missing manifest file → NO_MANIFEST gate");
    int v = theron_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == -1, "validate_manifest(NULL) with no file → -1");

    /* Direct validate of nonexistent path */
    v = theron_v2_hud_widget_assets_validate_manifest("/nope/does-not-exist.json");
    CHECK(v == -1, "validate_manifest on missing file → -1");
}

static void test_empty_manifest_yields_placeholder_gate(void) {
    clean_scratch();
    /* Build the directory structure the module looks for. */
    const char* dataDir = "/tmp/scratch/firestaff-data/theron";
    char manifest[1024];
    build_expected_manifest_path(manifest, sizeof(manifest), dataDir);
    /* Strip filename to get the parent dir. */
    char* slash = strrchr(manifest, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", manifest);
    CHECK(system(mkdir_cmd) == 0, "mkdir hud dir");
    /* Write empty manifest */
    char manifest_path[1024];
    snprintf(manifest_path, sizeof(manifest_path),
             "%s/hud_widget_manifest.json", manifest);
    CHECK(write_file(manifest_path, "{}"), "wrote empty manifest");

    theron_v2_hud_widget_assets_set_manifest_path(dataDir);
    int v = theron_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == 0, "empty manifest validates as partial");

    Theron_V2_HudWidgetGate gate = theron_v2_hud_widget_assets_gate();
    CHECK(gate == THERON_V2_HUD_WIDGET_GATE_PLACEHOLDER,
          "empty manifest → PLACEHOLDER gate");

    /* First slot should classify as MISSING (declared but not present) */
    Theron_V2_HudWidgetClass cls = theron_v2_hud_widget_assets_classify_slot(
        THERON_V2_HUD_WIDGET_COMPASS_ROSE);
    CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_MISSING,
          "first slot MISSING in empty manifest");
}

static void test_placeholder_slot_classifies_as_placeholder(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/theron";
    char manifest_path[1024];
    char pdir[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(pdir, sizeof(pdir), "%s/../../assets/theron/hud", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"theron-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"compass_rose\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_compass_rose.png\","
        "\"width\":32,\"height\":32},"
        "{\"id\":\"quest_items\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_quest_items.png\","
        "\"width\":48,\"height\":16}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote placeholder manifest");

    theron_v2_hud_widget_assets_set_manifest_path(dataDir);
    int v = theron_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == 1, "valid placeholder manifest validates as complete");

    Theron_V2_HudWidgetClass cls = theron_v2_hud_widget_assets_classify_slot(
        THERON_V2_HUD_WIDGET_COMPASS_ROSE);
    CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_PLACEHOLDER,
          "compass_rose → PLACEHOLDER");
    cls = theron_v2_hud_widget_assets_classify_slot(
        THERON_V2_HUD_WIDGET_QUEST_ITEMS);
    CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_PLACEHOLDER,
          "quest_items → PLACEHOLDER");

    /* Other slots absent from manifest → MISSING */
    cls = theron_v2_hud_widget_assets_classify_slot(
        THERON_V2_HUD_WIDGET_RELIC_COUNTER);
    CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_MISSING,
          "relic_counter (not declared) → MISSING");

    Theron_V2_HudWidgetGate gate = theron_v2_hud_widget_assets_gate();
    CHECK(gate == THERON_V2_HUD_WIDGET_GATE_PLACEHOLDER,
          "gate=PLACEHOLDER when slots declare 'placeholder' generator");

    CHECK(theron_v2_hud_widget_assets_get_installed() == 0,
          "installed=0 when gate=PLACEHOLDER");
}

static void test_real_slot_classifies_as_real(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/theron";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);

    /* Create category directories for both hud_widgets (Phase 3 primary:
     * compass_rose, quest_items, dungeon_progress, relic_counter,
     * rune_indicator) and hud_chrome (champion_bars, action_strip). */
    char widgets_dir[1024];
    char chrome_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/theron/hud/hud_widgets", dataDir);
    snprintf(chrome_dir, sizeof(chrome_dir),
             "%s/../../assets/theron/hud/hud_chrome", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s' '%s'",
             widgets_dir, chrome_dir);
    system(mkdir_cmd);

    /* Create the actual file on disk so source_file resolves. */
    char real_file[1024];
    snprintf(real_file, sizeof(real_file), "%s/compass_rose.png", widgets_dir);
    CHECK(write_file(real_file, "fake-png-bytes-for-test"), "wrote real asset file");

    /* All seven slots fully REAL.  Categories match k_slot_table in
     * src/theron/theron_v2_hud_widget_assets_pc34.c:
     *   hud_widgets  (Phase 3 primary): compass_rose, quest_items,
     *                 dungeon_progress, relic_counter, rune_indicator
     *   hud_chrome   (chrome supporting): champion_bars, action_strip */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"theron-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"compass_rose\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"compass_rose.png\",\"width\":32,\"height\":32},"
        "{\"id\":\"quest_items\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"quest_items.png\",\"width\":48,\"height\":16},"
        "{\"id\":\"dungeon_progress\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"dungeon_progress.png\",\"width\":48,\"height\":16},"
        "{\"id\":\"relic_counter\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"relic_counter.png\",\"width\":48,\"height\":16},"
        "{\"id\":\"rune_indicator\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"rune_indicator.png\",\"width\":40,\"height\":16},"
        "{\"id\":\"champion_bars\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"champion_bars.png\",\"width\":256,\"height\":24},"
        "{\"id\":\"action_strip\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"action_strip.png\",\"width\":200,\"height\":28}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote all-real manifest");

    /* Create each slot's source file in its declared category dir.
     * Slots 0..4 live in hud_widgets/, slots 5..6 in hud_chrome/. */
    for (size_t i = 0; i < THERON_V2_HUD_WIDGET_COUNT; ++i) {
        const char* cat = (i <= 4) ? "hud_widgets" : "hud_chrome";
        char fpath[1024];
        snprintf(fpath, sizeof(fpath),
                 "%s/../../assets/theron/hud/%s/%s.png",
                 dataDir, cat,
                 theron_v2_hud_widget_assets_slot_name((Theron_V2_HudWidgetSlot)i));
        if (access(fpath, F_OK) != 0) {
            write_file(fpath, "fake-png-bytes-for-test");
        }
    }

    theron_v2_hud_widget_assets_set_manifest_path(dataDir);
    Theron_V2_HudWidgetGate gate = theron_v2_hud_widget_assets_gate();
    CHECK(gate == THERON_V2_HUD_WIDGET_GATE_COMPLETE,
          "all slots real → COMPLETE gate");

    int total = 0;
    int real = theron_v2_hud_widget_assets_real_count(&total);
    CHECK(total == (int)THERON_V2_HUD_WIDGET_COUNT,
          "real_count total == WIDGET_COUNT");
    CHECK(real == total, "real_count real == total (COMPLETE)");

    CHECK(theron_v2_hud_widget_assets_get_installed() == 1,
          "installed=1 when gate=COMPLETE");

    for (size_t i = 0; i < THERON_V2_HUD_WIDGET_COUNT; ++i) {
        Theron_V2_HudWidgetClass cls =
            theron_v2_hud_widget_assets_classify_slot((Theron_V2_HudWidgetSlot)i);
        CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_REAL,
              "all slots classify as REAL");
        CHECK(theron_v2_hud_widget_assets_uses_placeholder(
                (Theron_V2_HudWidgetSlot)i) == 0,
              "uses_placeholder=0 for REAL");
    }
}

static void test_mixed_manifest_yields_partial_gate(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/theron";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);

    char widgets_dir[1024];
    char chrome_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/theron/hud/hud_widgets", dataDir);
    snprintf(chrome_dir, sizeof(chrome_dir),
             "%s/../../assets/theron/hud/hud_chrome", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s' '%s'",
             widgets_dir, chrome_dir);
    system(mkdir_cmd);

    /* One REAL (compass_rose with file on disk), one PLACEHOLDER,
     * remaining declared but invalid → PARTIAL. */
    char real_file[1024];
    snprintf(real_file, sizeof(real_file), "%s/compass_rose.png", widgets_dir);
    write_file(real_file, "fake-png-bytes-for-test");

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"theron-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"compass_rose\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"compass_rose.png\",\"width\":32,\"height\":32},"
        "{\"id\":\"quest_items\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_quest_items.png\","
        "\"width\":48,\"height\":16},"
        "{\"id\":\"dungeon_progress\"}"  /* malformed: missing fields */
        "]}";
    CHECK(write_file(manifest_path, content), "wrote mixed manifest");

    theron_v2_hud_widget_assets_set_manifest_path(dataDir);
    Theron_V2_HudWidgetGate gate = theron_v2_hud_widget_assets_gate();
    CHECK(gate == THERON_V2_HUD_WIDGET_GATE_PARTIAL,
          "mixed manifest → PARTIAL gate");

    Theron_V2_HudWidgetClass cls = theron_v2_hud_widget_assets_classify_slot(
        THERON_V2_HUD_WIDGET_COMPASS_ROSE);
    CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_REAL,
          "compass_rose (file on disk) → REAL");

    cls = theron_v2_hud_widget_assets_classify_slot(
        THERON_V2_HUD_WIDGET_QUEST_ITEMS);
    CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_PLACEHOLDER,
          "quest_items (placeholder) → PLACEHOLDER");

    cls = theron_v2_hud_widget_assets_classify_slot(
        THERON_V2_HUD_WIDGET_DUNGEON_PROGRESS);
    CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_PARTIAL,
          "dungeon_progress (missing fields) → PARTIAL");

    CHECK(theron_v2_hud_widget_assets_get_installed() == 1,
          "installed=1 when gate=PARTIAL");
}

static void test_partial_source_file_missing(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/theron";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    char widgets_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/theron/hud/hud_widgets", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", widgets_dir);
    system(mkdir_cmd);

    /* All fields present, but source_file does NOT exist on disk →
     * PARTIAL classification. */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"theron-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"compass_rose\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"does_not_exist.png\",\"width\":32,\"height\":32}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote manifest");

    theron_v2_hud_widget_assets_set_manifest_path(dataDir);
    Theron_V2_HudWidgetClass cls = theron_v2_hud_widget_assets_classify_slot(
        THERON_V2_HUD_WIDGET_COMPASS_ROSE);
    CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_PARTIAL,
          "missing source_file → PARTIAL");
}

static void test_slot_info_populates_fields(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/theron";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    char widgets_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/theron/hud/hud_widgets", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", widgets_dir);
    system(mkdir_cmd);

    char real_file[1024];
    snprintf(real_file, sizeof(real_file), "%s/compass_rose.png", widgets_dir);
    write_file(real_file, "fake-png-bytes-for-test");

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"theron-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"compass_rose\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"compass_rose.png\",\"width\":32,\"height\":32}"
        "]}";
    write_file(manifest_path, content);

    theron_v2_hud_widget_assets_set_manifest_path(dataDir);

    Theron_V2_HudWidgetSlotInfo info;
    memset(&info, 0, sizeof(info));
    int ok = theron_v2_hud_widget_assets_get_slot_info(
        THERON_V2_HUD_WIDGET_COMPASS_ROSE, &info);
    CHECK(ok == 1, "get_slot_info returns 1 for declared slot");
    CHECK(info.slot == THERON_V2_HUD_WIDGET_COMPASS_ROSE, "info.slot correct");
    CHECK(strcmp(info.id, "compass_rose") == 0, "info.id == 'compass_rose'");
    CHECK(strcmp(info.category, "hud_widgets") == 0, "info.category == 'hud_widgets'");
    CHECK(strcmp(info.generator, "pbr_hero") == 0, "info.generator == 'pbr_hero'");
    CHECK(strcmp(info.source_file, "compass_rose.png") == 0, "info.source_file correct");
    CHECK(info.width == 32, "info.width == 32");
    CHECK(info.height == 32, "info.height == 32");
    CHECK(info.file_exists == 1, "info.file_exists == 1 (file on disk)");
    CHECK(info.classification == THERON_V2_HUD_WIDGET_CLASS_REAL,
          "info.classification == REAL");
}

static void test_names_are_stable(void) {
    /* Slot names — anchored to gap-list Phase 3 row. */
    CHECK(strcmp(theron_v2_hud_widget_assets_slot_name(
        THERON_V2_HUD_WIDGET_COMPASS_ROSE), "compass_rose") == 0,
          "slot name: compass_rose");
    CHECK(strcmp(theron_v2_hud_widget_assets_slot_name(
        THERON_V2_HUD_WIDGET_QUEST_ITEMS), "quest_items") == 0,
          "slot name: quest_items");
    CHECK(strcmp(theron_v2_hud_widget_assets_slot_name(
        THERON_V2_HUD_WIDGET_DUNGEON_PROGRESS), "dungeon_progress") == 0,
          "slot name: dungeon_progress");
    CHECK(strcmp(theron_v2_hud_widget_assets_slot_name(
        THERON_V2_HUD_WIDGET_RELIC_COUNTER), "relic_counter") == 0,
          "slot name: relic_counter");
    CHECK(strcmp(theron_v2_hud_widget_assets_slot_name(
        THERON_V2_HUD_WIDGET_RUNE_INDICATOR), "rune_indicator") == 0,
          "slot name: rune_indicator");
    CHECK(strcmp(theron_v2_hud_widget_assets_slot_name(
        THERON_V2_HUD_WIDGET_CHAMPION_BARS), "champion_bars") == 0,
          "slot name: champion_bars");
    CHECK(strcmp(theron_v2_hud_widget_assets_slot_name(
        THERON_V2_HUD_WIDGET_ACTION_STRIP), "action_strip") == 0,
          "slot name: action_strip");

    /* Class names. */
    CHECK(strcmp(theron_v2_hud_widget_assets_class_name(
        THERON_V2_HUD_WIDGET_CLASS_UNKNOWN), "UNKNOWN") == 0,
          "class name: UNKNOWN");
    CHECK(strcmp(theron_v2_hud_widget_assets_class_name(
        THERON_V2_HUD_WIDGET_CLASS_MISSING), "MISSING") == 0,
          "class name: MISSING");
    CHECK(strcmp(theron_v2_hud_widget_assets_class_name(
        THERON_V2_HUD_WIDGET_CLASS_PLACEHOLDER), "PLACEHOLDER") == 0,
          "class name: PLACEHOLDER");
    CHECK(strcmp(theron_v2_hud_widget_assets_class_name(
        THERON_V2_HUD_WIDGET_CLASS_PARTIAL), "PARTIAL") == 0,
          "class name: PARTIAL");
    CHECK(strcmp(theron_v2_hud_widget_assets_class_name(
        THERON_V2_HUD_WIDGET_CLASS_REAL), "REAL") == 0,
          "class name: REAL");

    /* Gate names. */
    CHECK(strcmp(theron_v2_hud_widget_assets_gate_name(
        THERON_V2_HUD_WIDGET_GATE_NOT_PROBED), "NOT_PROBED") == 0,
          "gate name: NOT_PROBED");
    CHECK(strcmp(theron_v2_hud_widget_assets_gate_name(
        THERON_V2_HUD_WIDGET_GATE_NO_MANIFEST), "NO_MANIFEST") == 0,
          "gate name: NO_MANIFEST");
    CHECK(strcmp(theron_v2_hud_widget_assets_gate_name(
        THERON_V2_HUD_WIDGET_GATE_PLACEHOLDER), "PLACEHOLDER") == 0,
          "gate name: PLACEHOLDER");
    CHECK(strcmp(theron_v2_hud_widget_assets_gate_name(
        THERON_V2_HUD_WIDGET_GATE_PARTIAL), "PARTIAL") == 0,
          "gate name: PARTIAL");
    CHECK(strcmp(theron_v2_hud_widget_assets_gate_name(
        THERON_V2_HUD_WIDGET_GATE_COMPLETE), "COMPLETE") == 0,
          "gate name: COMPLETE");
}

static void test_out_of_range_inputs_safe(void) {
    /* Out-of-range slot */
    Theron_V2_HudWidgetClass cls = theron_v2_hud_widget_assets_classify_slot(
        (Theron_V2_HudWidgetSlot)9999);
    CHECK(cls == THERON_V2_HUD_WIDGET_CLASS_UNKNOWN,
          "out-of-range slot → UNKNOWN");

    /* Out-of-range get_slot_info */
    Theron_V2_HudWidgetSlotInfo info;
    memset(&info, 0, sizeof(info));
    int ok = theron_v2_hud_widget_assets_get_slot_info(
        (Theron_V2_HudWidgetSlot)9999, &info);
    CHECK(ok == 0, "get_slot_info out-of-range → 0");

    /* NULL out parameter */
    ok = theron_v2_hud_widget_assets_get_slot_info(
        THERON_V2_HUD_WIDGET_COMPASS_ROSE, NULL);
    CHECK(ok == 0, "get_slot_info(NULL) → 0 (no crash)");

    /* uses_placeholder with out-of-range */
    int ph = theron_v2_hud_widget_assets_uses_placeholder(
        (Theron_V2_HudWidgetSlot)9999);
    CHECK(ph == 1, "uses_placeholder out-of-range → 1");

    /* Slot name out-of-range */
    const char* n = theron_v2_hud_widget_assets_slot_name(
        (Theron_V2_HudWidgetSlot)9999);
    CHECK(n && strcmp(n, "UNKNOWN") == 0, "slot name out-of-range → UNKNOWN");
}

static void test_source_evidence_citations(void) {
    const char* ev = theron_v2_hud_widget_assets_source_evidence();
    CHECK(ev && strlen(ev) > 10, "source_evidence not NULL/empty");
    CHECK(strstr(ev, "THQUEST.ASM T520") != NULL,
          "source_evidence: mentions THQUEST.ASM T520");
    CHECK(strstr(ev, "THQUEST.ASM T560") != NULL,
          "source_evidence: mentions THQUEST.ASM T560");
    CHECK(strstr(ev, "THQUEST.ASM T600") != NULL,
          "source_evidence: mentions THQUEST.ASM T600 (UI overlay zones)");
    CHECK(strstr(ev, "THQUEST.ASM T900") != NULL,
          "source_evidence: mentions THQUEST.ASM T900 (rune magic)");
    CHECK(strstr(ev, "HuC6260") != NULL && strstr(ev, "HuC6270") != NULL,
          "source_evidence: mentions HuC6260 + HuC6270 (PC Engine VDC + VCE)");
    CHECK(strstr(ev, "PANEL.C F0354") != NULL,
          "source_evidence: mentions ReDMCSB PANEL.C F0354");
    CHECK(strstr(ev, "DUNGEON.C F0260") != NULL,
          "source_evidence: mentions ReDMCSB DUNGEON.C F0260");
    CHECK(strstr(ev, "dmweb") != NULL,
          "source_evidence: mentions dmweb Theron overview");
    CHECK(strstr(ev, "manifestVersion") != NULL ||
          strstr(ev, "manifest") != NULL,
          "source_evidence: mentions manifest schema/path");
    CHECK(strstr(ev, "placeholder") != NULL,
          "source_evidence: documents 'placeholder' generator as fallback marker");
}

int main(void) {
    printf("=== Theron V2 HUD Widget Asset Manifest gate ===\n\n");

    test_unset_path_is_safe();
    test_set_path_resolves_correctly();
    test_missing_manifest_file_yields_no_manifest();
    test_empty_manifest_yields_placeholder_gate();
    test_placeholder_slot_classifies_as_placeholder();
    test_real_slot_classifies_as_real();
    test_mixed_manifest_yields_partial_gate();
    test_partial_source_file_missing();
    test_slot_info_populates_fields();
    test_names_are_stable();
    test_out_of_range_inputs_safe();
    test_source_evidence_citations();

    clean_scratch();
    theron_v2_hud_widget_assets_set_manifest_path(NULL);

    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
