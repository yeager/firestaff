/*
 * test_dm2_v2_hud_widget_assets.c — DM2 V2 HUD Widget Asset Manifest gate
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
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - include/dm2_v22_modern_assets_pc34.h (sibling V2.2 manifest pattern)
 *   - include/dm2_v2_hud_widget_assets.h (module under test)
 */

#include "dm2_v2_hud_widget_assets.h"

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
 * data dir. Mirrors dm2_v2_hud_widget_assets_set_manifest_path. */
static void build_expected_manifest_path(char* out, size_t outSize,
                                          const char* dataDir) {
    /* dataDir = <root>/data/dm2 → manifest = <root>/assets/dm2/hud/hud_widget_manifest.json
     * Walk up two parents. */
    char a[1024], b[1024];
    const char* slash;
    slash = strrchr(dataDir, '/');
    if (!slash) { snprintf(out, outSize, "%s/assets/dm2/hud/hud_widget_manifest.json", dataDir); return; }
    size_t la = (size_t)(slash - dataDir);
    if (la >= sizeof(a)) la = sizeof(a) - 1U;
    memcpy(a, dataDir, la); a[la] = '\0';
    slash = strrchr(a, '/');
    if (!slash) { snprintf(out, outSize, "%s/assets/dm2/hud/hud_widget_manifest.json", dataDir); return; }
    size_t lb = (size_t)(slash - a);
    if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
    memcpy(b, a, lb); b[lb] = '\0';
    snprintf(out, outSize, "%s/assets/dm2/hud/hud_widget_manifest.json", b);
}

/* Helper: clean a scratch dir before each scenario. We also remove
 * the asset tree the module looks at (~/.firestaff/assets/dm2/hud/…)
 * so that running the test suite twice in a row produces identical
 * results — the all-REAL and other tests write files there that
 * would otherwise leak across runs. */
static void clean_scratch(void) {
    system("rm -rf /tmp/scratch/dm2_hwa_test");
    system("rm -rf /tmp/scratch/assets /tmp/scratch/firestaff-data");
}

/* ── Tests ──────────────────────────────────────────────────────── */

static void test_unset_path_is_safe(void) {
    dm2_v2_hud_widget_assets_set_manifest_path(NULL);
    dm2_v2_hud_widget_assets_set_manifest_path("");
    const char* p = dm2_v2_hud_widget_assets_get_manifest_path();
    CHECK(p && p[0] == '\0', "unset path returns empty string");

    int total = 0;
    int real = dm2_v2_hud_widget_assets_real_count(&total);
    CHECK(real == 0, "real_count=0 with no manifest");
    CHECK(total == 0, "total=0 with no manifest");

    DM2_V2_HudWidgetGate gate = dm2_v2_hud_widget_assets_gate();
    CHECK(gate == DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST,
          "gate=NO_MANIFEST when unset");

    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        DM2_V2_HudWidgetClass cls =
            dm2_v2_hud_widget_assets_classify_slot((DM2_V2_HudWidgetSlot)i);
        CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_MISSING,
              "all slots MISSING when manifest unset");
        CHECK(dm2_v2_hud_widget_assets_uses_placeholder(
                (DM2_V2_HudWidgetSlot)i) == 1,
              "uses_placeholder=1 when manifest unset");
    }
}

static void test_set_path_resolves_correctly(void) {
    dm2_v2_hud_widget_assets_set_manifest_path("/tmp/scratch/firestaff-data/dm2");
    char expected[1024];
    build_expected_manifest_path(expected, sizeof(expected),
                                  "/tmp/scratch/firestaff-data/dm2");
    const char* actual = dm2_v2_hud_widget_assets_get_manifest_path();
    CHECK(actual && strcmp(actual, expected) == 0,
          "manifest path resolves correctly under assets/dm2/hud/");
}

static void test_missing_manifest_file_yields_no_manifest(void) {
    clean_scratch();
    dm2_v2_hud_widget_assets_set_manifest_path("/tmp/scratch/firestaff-data/dm2");
    /* Don't actually create the manifest path */
    DM2_V2_HudWidgetGate gate = dm2_v2_hud_widget_assets_gate();
    CHECK(gate == DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST,
          "missing manifest file → NO_MANIFEST gate");
    int v = dm2_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == -1, "validate_manifest(NULL) with no file → -1");

    /* Direct validate of nonexistent path */
    v = dm2_v2_hud_widget_assets_validate_manifest("/nope/does-not-exist.json");
    CHECK(v == -1, "validate_manifest on missing file → -1");
}

static void test_empty_manifest_yields_placeholder_gate(void) {
    clean_scratch();
    /* Build the directory structure the module looks for. */
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
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

    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    int v = dm2_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == 0, "empty manifest validates as partial");

    DM2_V2_HudWidgetGate gate = dm2_v2_hud_widget_assets_gate();
    CHECK(gate == DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER,
          "empty manifest → PLACEHOLDER gate");

    /* First slot should classify as MISSING (declared but not present) */
    DM2_V2_HudWidgetClass cls = dm2_v2_hud_widget_assets_classify_slot(
        DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW);
    CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_MISSING,
          "first slot MISSING in empty manifest");
}

static void test_placeholder_slot_classifies_as_placeholder(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    char scratch_dir[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(scratch_dir, sizeof(scratch_dir),
             "%s/assets/dm2/hud", "/tmp/scratch/firestaff-data/..");
    /* Easier: just mkdir based on dataDir */
    char pdir[1024];
    snprintf(pdir, sizeof(pdir), "%s/../../assets/dm2/hud", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_inventory_quick_view.png\","
        "\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_action_prompt.png\","
        "\"width\":48,\"height\":16}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote placeholder manifest");

    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    int v = dm2_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == 1, "valid placeholder manifest validates as complete");

    DM2_V2_HudWidgetClass cls = dm2_v2_hud_widget_assets_classify_slot(
        DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW);
    CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER,
          "inventory_quick_view → PLACEHOLDER");
    cls = dm2_v2_hud_widget_assets_classify_slot(
        DM2_V2_HUD_WIDGET_ACTION_PROMPT);
    CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER,
          "action_prompt → PLACEHOLDER");

    /* Other slots absent from manifest → MISSING */
    cls = dm2_v2_hud_widget_assets_classify_slot(
        DM2_V2_HUD_WIDGET_COMPASS_ROSE);
    CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_MISSING,
          "compass_rose (not declared) → MISSING");

    DM2_V2_HudWidgetGate gate = dm2_v2_hud_widget_assets_gate();
    CHECK(gate == DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER,
          "gate=PLACEHOLDER when slots declare 'placeholder' generator");

    CHECK(dm2_v2_hud_widget_assets_get_installed() == 0,
          "installed=0 when gate=PLACEHOLDER");
}

static void test_real_slot_classifies_as_real(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    /* Create category directories for both hud_widgets (primary) and
     * hud_chrome (supporting slots — compass_rose, depth_indicator,
     * gold_counter, champion_bar_frame, action_strip_frame). */
    char widgets_dir[1024];
    char chrome_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/dm2/hud/hud_widgets", dataDir);
    snprintf(chrome_dir, sizeof(chrome_dir),
             "%s/../../assets/dm2/hud/hud_chrome", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s' '%s'",
             widgets_dir, chrome_dir);
    system(mkdir_cmd);

    /* Create the actual file on disk so source_file resolves. */
    char real_file[1024];
    snprintf(real_file, sizeof(real_file), "%s/inventory_quick_view.png", widgets_dir);
    CHECK(write_file(real_file, "fake-png-bytes-for-test"), "wrote real asset file");

    /* All seven slots fully REAL */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"inventory_quick_view.png\",\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"action_prompt.png\",\"width\":48,\"height\":16},"
        "{\"id\":\"compass_rose\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"compass_rose.png\",\"width\":32,\"height\":32},"
        "{\"id\":\"depth_indicator\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"depth_indicator.png\",\"width\":40,\"height\":16},"
        "{\"id\":\"gold_counter\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"gold_counter.png\",\"width\":56,\"height\":16},"
        "{\"id\":\"champion_bar_frame\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"champion_bar_frame.png\",\"width\":64,\"height\":8},"
        "{\"id\":\"action_strip_frame\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"action_strip_frame.png\",\"width\":28,\"height\":28}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote all-real manifest");

    /* Create each slot's source file in its declared category dir.
     * Slot 0/1 live in hud_widgets/, slots 2..6 in hud_chrome/. */
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        DM2_V2_HudWidgetSlotInfo info;
        memset(&info, 0, sizeof(info));
        /* Use the public slot name + lookup table category without a
         * manifest to drive path resolution. Simpler: derive category
         * from the slot index — first two are hud_widgets, rest are
         * hud_chrome, matching k_slot_table. */
        const char* cat = (i < 2) ? "hud_widgets" : "hud_chrome";
        char fpath[1024];
        snprintf(fpath, sizeof(fpath),
                 "%s/../../assets/dm2/hud/%s/%s.png",
                 dataDir, cat,
                 dm2_v2_hud_widget_assets_slot_name((DM2_V2_HudWidgetSlot)i));
        if (access(fpath, F_OK) != 0) {
            write_file(fpath, "fake-png-bytes-for-test");
        }
    }

    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    DM2_V2_HudWidgetGate gate = dm2_v2_hud_widget_assets_gate();
    CHECK(gate == DM2_V2_HUD_WIDGET_GATE_COMPLETE,
          "all slots REAL → COMPLETE gate");

    CHECK(dm2_v2_hud_widget_assets_get_installed() == 1,
          "installed=1 when gate=COMPLETE");

    int total = 0;
    int real = dm2_v2_hud_widget_assets_real_count(&total);
    CHECK(real == (int)DM2_V2_HUD_WIDGET_COUNT,
          "real_count=DM2_V2_HUD_WIDGET_COUNT");
    CHECK(total == (int)DM2_V2_HUD_WIDGET_COUNT,
          "total=DM2_V2_HUD_WIDGET_COUNT");

    /* Each slot should classify as REAL */
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        DM2_V2_HudWidgetClass cls =
            dm2_v2_hud_widget_assets_classify_slot((DM2_V2_HudWidgetSlot)i);
        CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_REAL,
              "all slots REAL with disk-resolvable source_file");
        CHECK(dm2_v2_hud_widget_assets_uses_placeholder(
                (DM2_V2_HudWidgetSlot)i) == 0,
              "uses_placeholder=0 for REAL slot");
    }
}

static void test_partial_when_some_real(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    char pdir[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(pdir, sizeof(pdir), "%s/../../assets/dm2/hud/hud_widgets", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    /* Only inventory_quick_view is REAL; action_prompt placeholder;
     * remaining absent (MISSING). */
    char real_file[1024];
    snprintf(real_file, sizeof(real_file), "%s/inventory_quick_view.png", pdir);
    CHECK(write_file(real_file, "fake-png-bytes-for-test"), "wrote real asset file");

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"inventory_quick_view.png\",\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":16}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote mixed manifest");

    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    DM2_V2_HudWidgetGate gate = dm2_v2_hud_widget_assets_gate();
    CHECK(gate == DM2_V2_HUD_WIDGET_GATE_PARTIAL,
          "mixed manifest → PARTIAL gate");

    CHECK(dm2_v2_hud_widget_assets_get_installed() == 1,
          "installed=1 when gate=PARTIAL");

    /* First slot REAL */
    DM2_V2_HudWidgetClass cls = dm2_v2_hud_widget_assets_classify_slot(
        DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW);
    CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_REAL, "inventory_quick_view=REAL");
    /* Second slot PLACEHOLDER */
    cls = dm2_v2_hud_widget_assets_classify_slot(
        DM2_V2_HUD_WIDGET_ACTION_PROMPT);
    CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER, "action_prompt=PLACEHOLDER");

    int total = 0;
    int real = dm2_v2_hud_widget_assets_real_count(&total);
    CHECK(real == 1, "real_count=1 with one REAL slot");
    CHECK(total == 2, "total=2 with two declared slots");
}

static void test_partial_when_real_but_missing_source_file(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    char pdir[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(pdir, sizeof(pdir), "%s/../../assets/dm2/hud/hud_widgets", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    /* Manifest declares pbr_hero but the source_file does not exist */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"missing_on_disk.png\",\"width\":64,\"height\":32}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote manifest with missing source");

    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    DM2_V2_HudWidgetClass cls = dm2_v2_hud_widget_assets_classify_slot(
        DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW);
    CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_PARTIAL,
          "real metadata but missing file → PARTIAL");

    int total = 0;
    int real = dm2_v2_hud_widget_assets_real_count(&total);
    CHECK(real == 0, "real_count=0 (no disk-resolved file)");
    CHECK(total == 1, "total=1 (one declared slot)");

    /* Gate: declared slots but no REAL → PLACEHOLDER (procedural
     * fallback covers PARTIAL too) */
    DM2_V2_HudWidgetGate gate = dm2_v2_hud_widget_assets_gate();
    CHECK(gate == DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER,
          "PARTIAL-only slots → PLACEHOLDER gate");
}

static void test_partial_when_fields_missing(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    char pdir[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(pdir, sizeof(pdir), "%s/../../assets/dm2/hud", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    /* Entry missing width/height → PARTIAL (real metadata incomplete) */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"x.png\"}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote manifest with incomplete entry");

    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    DM2_V2_HudWidgetClass cls = dm2_v2_hud_widget_assets_classify_slot(
        DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW);
    CHECK(cls == DM2_V2_HUD_WIDGET_CLASS_PARTIAL,
          "missing width/height → PARTIAL");

    int v = dm2_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == 0, "incomplete manifest validates as partial (not complete)");
}

static void test_get_slot_info_populates_fields(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    char pdir[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(pdir, sizeof(pdir), "%s/../../assets/dm2/hud/hud_widgets", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    char real_file[1024];
    snprintf(real_file, sizeof(real_file), "%s/inventory_quick_view.png", pdir);
    write_file(real_file, "fake-png-bytes-for-test");

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"inventory_quick_view.png\",\"width\":64,\"height\":32}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote manifest for slot info");

    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    DM2_V2_HudWidgetSlotInfo info;
    int ok = dm2_v2_hud_widget_assets_get_slot_info(
        DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW, &info);
    CHECK(ok == 1, "get_slot_info returns 1 for present slot");
    CHECK(strcmp(info.id, "inventory_quick_view") == 0,
          "slot info id correct");
    CHECK(strcmp(info.category, "hud_widgets") == 0,
          "slot info category correct");
    CHECK(strcmp(info.generator, "pbr_hero") == 0,
          "slot info generator correct");
    CHECK(strcmp(info.source_file, "inventory_quick_view.png") == 0,
          "slot info source_file correct");
    CHECK(info.width == 64, "slot info width=64");
    CHECK(info.height == 32, "slot info height=32");
    CHECK(info.file_exists == 1, "slot info file_exists=1");
    CHECK(info.classification == DM2_V2_HUD_WIDGET_CLASS_REAL,
          "slot info classification=REAL");
    CHECK(info.resolved_path[0] != '\0', "resolved_path populated");

    /* NULL out is safe */
    dm2_v2_hud_widget_assets_get_slot_info(
        DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW, NULL);
    CHECK(1, "get_slot_info(NULL) safe");

    /* Out-of-range slot returns 0 */
    DM2_V2_HudWidgetSlotInfo info2;
    ok = dm2_v2_hud_widget_assets_get_slot_info(
        (DM2_V2_HudWidgetSlot)9999, &info2);
    CHECK(ok == 0, "get_slot_info out-of-range returns 0");

    /* Absent slot returns 0 */
    ok = dm2_v2_hud_widget_assets_get_slot_info(
        DM2_V2_HUD_WIDGET_COMPASS_ROSE, &info2);
    CHECK(ok == 0, "get_slot_info absent slot returns 0");
}

static void test_slot_count_is_seven(void) {
    CHECK(DM2_V2_HUD_WIDGET_COUNT == 7,
          "DM2_V2_HUD_WIDGET_COUNT=7 (2 Phase 3 primary + 5 chrome)");
    /* Phase 3 primary slots are explicitly listed in gap-list */
    CHECK(DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW == 0, "primary[0] = inventory_quick_view");
    CHECK(DM2_V2_HUD_WIDGET_ACTION_PROMPT == 1,        "primary[1] = action_prompt");
}

static void test_names_are_stable(void) {
    /* Slot names match k_slot_table */
    CHECK(strcmp(dm2_v2_hud_widget_assets_slot_name(
            DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW),
            "inventory_quick_view") == 0,
          "inventory_quick_view name stable");
    CHECK(strcmp(dm2_v2_hud_widget_assets_slot_name(
            DM2_V2_HUD_WIDGET_ACTION_PROMPT),
            "action_prompt") == 0,
          "action_prompt name stable");
    CHECK(strcmp(dm2_v2_hud_widget_assets_slot_name(
            DM2_V2_HUD_WIDGET_COMPASS_ROSE),
            "compass_rose") == 0,
          "compass_rose name stable");
    CHECK(strcmp(dm2_v2_hud_widget_assets_slot_name(
            DM2_V2_HUD_WIDGET_CHAMPION_BAR_FRAME),
            "champion_bar_frame") == 0,
          "champion_bar_frame name stable");
    CHECK(strcmp(dm2_v2_hud_widget_assets_slot_name(
            DM2_V2_HUD_WIDGET_ACTION_STRIP_FRAME),
            "action_strip_frame") == 0,
          "action_strip_frame name stable");
    /* Out-of-range name */
    CHECK(strcmp(dm2_v2_hud_widget_assets_slot_name(
            (DM2_V2_HudWidgetSlot)9999),
            "UNKNOWN") == 0,
          "out-of-range slot name → UNKNOWN");

    /* Class names */
    CHECK(strcmp(dm2_v2_hud_widget_assets_class_name(
            DM2_V2_HUD_WIDGET_CLASS_REAL), "REAL") == 0,
          "class REAL name");
    CHECK(strcmp(dm2_v2_hud_widget_assets_class_name(
            DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER), "PLACEHOLDER") == 0,
          "class PLACEHOLDER name");
    CHECK(strcmp(dm2_v2_hud_widget_assets_class_name(
            DM2_V2_HUD_WIDGET_CLASS_PARTIAL), "PARTIAL") == 0,
          "class PARTIAL name");
    CHECK(strcmp(dm2_v2_hud_widget_assets_class_name(
            DM2_V2_HUD_WIDGET_CLASS_MISSING), "MISSING") == 0,
          "class MISSING name");

    /* Gate names */
    CHECK(strcmp(dm2_v2_hud_widget_assets_gate_name(
            DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST), "NO_MANIFEST") == 0,
          "gate NO_MANIFEST name");
    CHECK(strcmp(dm2_v2_hud_widget_assets_gate_name(
            DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER), "PLACEHOLDER") == 0,
          "gate PLACEHOLDER name");
    CHECK(strcmp(dm2_v2_hud_widget_assets_gate_name(
            DM2_V2_HUD_WIDGET_GATE_PARTIAL), "PARTIAL") == 0,
          "gate PARTIAL name");
    CHECK(strcmp(dm2_v2_hud_widget_assets_gate_name(
            DM2_V2_HUD_WIDGET_GATE_COMPLETE), "COMPLETE") == 0,
          "gate COMPLETE name");
}

static void test_source_evidence_citations(void) {
    const char* ev = dm2_v2_hud_widget_assets_source_evidence();
    CHECK(ev != NULL && strlen(ev) > 100, "source_evidence non-empty");
    CHECK(strstr(ev, "SKULL.ASM T560") != NULL, "mentions SKULL.ASM T560");
    CHECK(strstr(ev, "SKULLWIN") != NULL, "mentions SKULLWIN");
    CHECK(strstr(ev, "ReDMCSB PANEL.C") != NULL, "mentions ReDMCSB PANEL.C");
    CHECK(strstr(ev, "DUNGEON.C") != NULL, "mentions DUNGEON.C");
    CHECK(strstr(ev, "FIRESTAFF_GAP_LIST") != NULL, "mentions gap list");
    CHECK(strstr(ev, "V1") != NULL, "mentions V1 invariant");
}

static void test_installed_flag_round_trip(void) {
    dm2_v2_hud_widget_assets_set_installed(1);
    CHECK(dm2_v2_hud_widget_assets_get_installed() == 1,
          "installed=1 after set_installed(1)");
    dm2_v2_hud_widget_assets_set_installed(0);
    CHECK(dm2_v2_hud_widget_assets_get_installed() == 0,
          "installed=0 after set_installed(0)");
    /* Truthy clamp */
    dm2_v2_hud_widget_assets_set_installed(99);
    CHECK(dm2_v2_hud_widget_assets_get_installed() == 1,
          "installed clamped to 1 for non-zero input");
}

static void test_validate_manifest_three_branches(void) {
    /* Already covered: missing → -1, partial → 0, complete → 1.
     * Quick sanity check. */
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    char pdir[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(pdir, sizeof(pdir), "%s/../../assets/dm2/hud", dataDir);
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    /* Complete manifest */
    const char* good =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"placeholder\","
        "\"source_file\":\"x.png\",\"width\":64,\"height\":32}"
        "]}";
    CHECK(write_file(manifest_path, good), "wrote complete test manifest");
    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    int v = dm2_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == 1, "complete manifest → 1");

    /* Partial manifest */
    const char* partial =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-test\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"source_file\":\"x.png\"}"
        "]}";
    write_file(manifest_path, partial);
    v = dm2_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == 0, "partial manifest → 0");

    /* Garbage manifest */
    write_file(manifest_path, "this is not json { [");
    v = dm2_v2_hud_widget_assets_validate_manifest(NULL);
    CHECK(v == -1 || v == 0, "garbage manifest → -1 or 0 (parser rejects)");
}

/* ── Main ───────────────────────────────────────────────────────── */

int main(void) {
    printf("=== DM2 V2 HUD Widget Asset Manifest unit test ===\n");
    test_unset_path_is_safe();
    test_set_path_resolves_correctly();
    test_missing_manifest_file_yields_no_manifest();
    test_empty_manifest_yields_placeholder_gate();
    test_placeholder_slot_classifies_as_placeholder();
    test_real_slot_classifies_as_real();
    test_partial_when_some_real();
    test_partial_when_real_but_missing_source_file();
    test_partial_when_fields_missing();
    test_get_slot_info_populates_fields();
    test_slot_count_is_seven();
    test_names_are_stable();
    test_source_evidence_citations();
    test_installed_flag_round_trip();
    test_validate_manifest_three_branches();
    clean_scratch();
    printf("\n=== Results: %d passed, %d failed ===\n",
           s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
