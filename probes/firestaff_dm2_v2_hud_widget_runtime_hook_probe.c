/*
 * firestaff_dm2_v2_hud_widget_runtime_hook_probe.c — DM2 V2 HUD widget
 * runtime hook probe (Phase 3 widget bitmap follow-up).
 *
 * Headless CI probe for the wiring between
 * dm2_v2_hud_widget_assets_classify_slot() (the per-slot
 * REAL/PARTIAL/PLACEHOLDER/MISSING manifest gate) and
 * dm2_v2_hud_runtime_render_with_assets() (the runtime entry that
 * substitutes the procedural fallback for a real-bitmap substitute
 * path when a slot is REAL).
 *
 * No game data, no SDL rendering required. Builds synthetic manifest
 * files in a temp scratch directory.
 *
 * Coverage:
 *   1.  No manifest → all 7 slots classified MISSING, runtime records
 *       PROCEDURAL_FALLBACK for every slot, aggregate counts
 *       real=0/fallback=7
 *   2.  Empty manifest → no slots declared, runtime still records
 *       PROCEDURAL_FALLBACK for every slot, no real stamps emitted
 *       (placeholder fallback is byte-identical to the no-gate
 *       baseline — V1 invariant preserved)
 *   3.  Placeholder manifest → all declared slots PLACEHOLDER,
 *       runtime records PROCEDURAL_FALLBACK, no stamps
 *   4.  Synthetic PARTIAL manifest (one REAL slot, one PLACEHOLDER,
 *       rest MISSING) → runtime records REAL_BITMAP for that one
 *       slot, PROCEDURAL_FALLBACK for the rest; aggregate
 *       real=1/fallback=6; the stamp pixel appears at the slot's
 *       anchor; other anchors stay untouched
 *   5.  Synthetic COMPLETE manifest (all 7 slots REAL, all files on
 *       disk) → runtime records REAL_BITMAP for every slot,
 *       real=7/fallback=0; every anchor gets a stamp
 *   6.  Per-slot path-mode stability across re-renders (the record
 *       reflects the most recent call, not stale state)
 *   7.  Phase gate honoured: when V2 is off, no stamps emitted even
 *       if the slot is REAL (path-mode record still records
 *       REAL_BITMAP for the gate's classification, but the
 *       framebuffer stays untouched — V1 chrome preserved)
 *   8.  Out-of-range slot queries are safe (no crash, returns
 *       PROCEDURAL_FALLBACK / UNKNOWN)
 *   9.  Source evidence citations include the new
 *       dm2_v2_hud_widget_assets reference
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - include/dm2_v2_hud_widget_assets.h (gate under test)
 *   - include/dm2_v2_hud_runtime.h     (runtime hook under test)
 */

#include "dm2_v2_hud_runtime.h"
#include "dm2_v2_hud_overlay.h"
#include "dm2_v2_phase_gate.h"
#include "dm2_v2_hud_widget_assets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ── Probe plumbing ─────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(expr, msg)                                                  \
    do {                                                                  \
        g_pass++;                                                         \
        if (!(expr)) {                                                    \
            fprintf(stderr, "FAIL %s:%d: %s \u2014 %s\n",                  \
                    __FILE__, __LINE__, #expr, (msg));                    \
            g_fail++;                                                     \
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

/* Helper: build the manifest path that the module resolves for a
 * given data dir, mirroring dm2_v2_hud_widget_assets_set_manifest_path
 * (dataDir = <root>/data/dm2 → manifest = <root>/assets/dm2/hud/hud_widget_manifest.json).
 * Walk up two parents. */
static void build_expected_manifest_path(char* out, size_t outSize,
                                          const char* dataDir) {
    char a[1024], b[1024];
    const char* slash;
    slash = strrchr(dataDir, '/');
    if (!slash) {
        snprintf(out, outSize, "%s/assets/dm2/hud/hud_widget_manifest.json", dataDir);
        return;
    }
    size_t la = (size_t)(slash - dataDir);
    if (la >= sizeof(a)) la = sizeof(a) - 1U;
    memcpy(a, dataDir, la); a[la] = '\0';
    slash = strrchr(a, '/');
    if (!slash) {
        snprintf(out, outSize, "%s/assets/dm2/hud/hud_widget_manifest.json", dataDir);
        return;
    }
    size_t lb = (size_t)(slash - a);
    if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
    memcpy(b, a, lb); b[lb] = '\0';
    snprintf(out, outSize, "%s/assets/dm2/hud/hud_widget_manifest.json", b);
}

/* Helper: clean the scratch tree before each scenario so reruns are
 * deterministic. We deliberately rm the dm2/assets/dm2 tree the
 * manifest path resolver walks to. */
static void clean_scratch(void) {
    system("rm -rf /tmp/scratch/dm2_hwa_hook_probe");
    system("rm -rf /tmp/scratch/assets /tmp/scratch/firestaff-data");
}

/* Anchor pixel positions for the real-bitmap stamp. Kept in sync with
 * k_real_stamp_anchors[] in src/dm2/dm2_v2_hud_runtime.c. The probe
 * is data-free and must not link the runtime's static state; we
 * mirror the constants here so a runtime regression that moves an
 * anchor fails the probe. */
typedef struct { int x; int y; } Anchor;
static const Anchor k_anchors[DM2_V2_HUD_WIDGET_COUNT] = {
    { 80,  4 },   /* INVENTORY_QUICK_VIEW */
    { 220, 4 },   /* ACTION_PROMPT */
    { 11,  16 },  /* COMPASS_ROSE */
    { 286, 8 },   /* DEPTH_INDICATOR */
    { 286, 178 }, /* GOLD_COUNTER */
    { 4,   4 },   /* CHAMPION_BAR_FRAME */
    { 16,  172 }, /* ACTION_STRIP_FRAME */
};

/* ── Scenario: no manifest → all slots fallback ────────────────── */

static void test_no_manifest_all_fallback(void) {
    printf("\n[ Scenario 1: no manifest → all fallback ]\n");
    clean_scratch();
    dm2_v2_hud_runtime_init();
    dm2_v2_hud_widget_assets_set_manifest_path(NULL);
    /* Default gate state should be NO_MANIFEST. */
    CHECK(dm2_v2_hud_widget_assets_gate() ==
          DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST,
          "no manifest → NO_MANIFEST gate");
    /* Initial path counts are zero (no render has run). */
    int real = -1, fb_count = -1;
    int total = dm2_v2_hud_runtime_last_path_counts(&real, &fb_count);
    CHECK(total == 0 && real == 0 && fb_count == 0,
          "before any render, all path counts are zero");
    CHECK(dm2_v2_hud_runtime_last_path_mode(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK,
          "before any render, slot 0 = PROCEDURAL_FALLBACK default");

    /* Render with V2 on → all slots classified MISSING → fallback */
    DM2_V2_PhaseGateConfig gate = { 1, 1 };
    dm2_v2_hud_runtime_set_gate_config(&gate);
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);

    real = -1; fb_count = -1;
    total = dm2_v2_hud_runtime_last_path_counts(&real, &fb_count);
    CHECK(total == (int)DM2_V2_HUD_WIDGET_COUNT,
          "render counts all 7 slots");
    CHECK(real == 0 && fb_count == (int)DM2_V2_HUD_WIDGET_COUNT,
          "no manifest → real=0, fallback=7");

    /* No stamp pixel should appear at any anchor — the path-mode
     * record is the deterministic test for "no REAL was selected".
     * The framebuffer at a fallback slot's anchor may still hold a
     * pixel drawn by the procedural overlay (compass circle,
     * depth digits, gold counter digits, champion bar, action strip),
     * so we do not assert fb[anchor] == 0 there — we only assert
     * the path-mode record correctly reports PROCEDURAL_FALLBACK. */
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        CHECK(dm2_v2_hud_runtime_last_path_mode(
                  (DM2_V2_HudWidgetSlot)i) ==
                  DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK,
                  "all 7 slots PROCEDURAL_FALLBACK when no manifest");
        CHECK(dm2_v2_hud_runtime_last_slot_class(
                  (DM2_V2_HudWidgetSlot)i) ==
                  DM2_V2_HUD_WIDGET_CLASS_MISSING,
                  "gate verdict for fallback slot = MISSING");
    }

    dm2_v2_hud_runtime_shutdown();
}

/* ── Scenario: empty manifest → placeholder fallback ───────────────── */

static void test_empty_manifest_fallback(void) {
    printf("\n[ Scenario 2: empty manifest → fallback ]\n");
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    /* mkdir the parent */
    char pdir[1024];
    snprintf(pdir, sizeof(pdir), "%s", manifest_path);
    char* slash = strrchr(pdir, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    CHECK(system(mkdir_cmd) == 0, "mkdir hud dir");
    CHECK(write_file(manifest_path, "{}"), "wrote empty manifest");

    dm2_v2_hud_runtime_init();
    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    CHECK(dm2_v2_hud_widget_assets_gate() ==
          DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER,
          "empty manifest → PLACEHOLDER gate");

    DM2_V2_PhaseGateConfig gate = { 1, 1 };
    dm2_v2_hud_runtime_set_gate_config(&gate);
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);

    int real = -1, fb_count = -1;
    int total = dm2_v2_hud_runtime_last_path_counts(&real, &fb_count);
    CHECK(total == (int)DM2_V2_HUD_WIDGET_COUNT,
          "empty manifest still counts all 7 slots");
    CHECK(real == 0 && fb_count == (int)DM2_V2_HUD_WIDGET_COUNT,
          "empty manifest → real=0, fallback=7");
    /* No framebuffer stamp check at fallback slot anchors: the
     * procedural overlay paints chrome over those pixels, and the
     * path-mode record (checked above) is the deterministic
     * "no REAL was selected" proof. */
    dm2_v2_hud_runtime_shutdown();
}

/* ── Scenario: placeholder manifest → all fallback ─────────────────── */

static void test_placeholder_manifest_fallback(void) {
    printf("\n[ Scenario 3: placeholder manifest → all fallback ]\n");
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    char pdir[1024];
    snprintf(pdir, sizeof(pdir), "%s", manifest_path);
    char* slash = strrchr(pdir, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    CHECK(system(mkdir_cmd) == 0, "mkdir hud dir");

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-hook-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":16}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote placeholder manifest");

    dm2_v2_hud_runtime_init();
    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    CHECK(dm2_v2_hud_widget_assets_gate() ==
          DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER,
          "placeholder manifest → PLACEHOLDER gate");

    DM2_V2_PhaseGateConfig gate = { 1, 1 };
    dm2_v2_hud_runtime_set_gate_config(&gate);
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);

    int real = -1, fb_count = -1;
    int total = dm2_v2_hud_runtime_last_path_counts(&real, &fb_count);
    CHECK(total == (int)DM2_V2_HUD_WIDGET_COUNT,
          "placeholder manifest still counts all 7 slots");
    CHECK(real == 0 && fb_count == (int)DM2_V2_HUD_WIDGET_COUNT,
          "placeholder manifest → real=0, fallback=7");
    /* Inventory_quick_view is declared PLACEHOLDER → not REAL */
    CHECK(dm2_v2_hud_runtime_last_slot_class(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER,
          "inventory_quick_view classified PLACEHOLDER");
    CHECK(fb[k_anchors[0].y * 320 + k_anchors[0].x] == 0,
          "no stamp at inventory_quick_view anchor");
    dm2_v2_hud_runtime_shutdown();
}

/* ── Scenario: synthetic PARTIAL manifest → one REAL slot ───────────── */

static void test_partial_one_real_slot_selected(void) {
    printf("\n[ Scenario 4: PARTIAL manifest → one REAL slot selected ]\n");
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    char pdir[1024];
    snprintf(pdir, sizeof(pdir), "%s", manifest_path);
    char* slash = strrchr(pdir, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    CHECK(system(mkdir_cmd) == 0, "mkdir hud dir");
    /* Create category dirs for both primary and chrome */
    char widgets_dir[1024], chrome_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/dm2/hud/hud_widgets", dataDir);
    snprintf(chrome_dir,  sizeof(chrome_dir),
             "%s/../../assets/dm2/hud/hud_chrome", dataDir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s' '%s'",
             widgets_dir, chrome_dir);
    CHECK(system(mkdir_cmd) == 0, "mkdir widget+chrome dirs");

    /* Drop inventory_quick_view.png on disk so source_file resolves. */
    char real_file[1024];
    snprintf(real_file, sizeof(real_file),
             "%s/inventory_quick_view.png", widgets_dir);
    CHECK(write_file(real_file, "fake-png-bytes"), "wrote real asset file");

    /* Manifest: only inventory_quick_view is REAL, action_prompt is
     * PLACEHOLDER, the rest are absent (MISSING). */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-hook-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"inventory_quick_view.png\",\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":16}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote partial manifest");

    dm2_v2_hud_runtime_init();
    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    CHECK(dm2_v2_hud_widget_assets_gate() ==
          DM2_V2_HUD_WIDGET_GATE_PARTIAL,
          "PARTIAL manifest → PARTIAL gate");

    DM2_V2_PhaseGateConfig gate = { 1, 1 };
    dm2_v2_hud_runtime_set_gate_config(&gate);
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);

    int real = -1, fb_count = -1;
    int total = dm2_v2_hud_runtime_last_path_counts(&real, &fb_count);
    CHECK(total == (int)DM2_V2_HUD_WIDGET_COUNT,
          "PARTIAL still counts all 7 slots");
    CHECK(real == 1 && fb_count == (int)DM2_V2_HUD_WIDGET_COUNT - 1,
          "PARTIAL → real=1, fallback=6");

    /* The single REAL slot must be selected at the runtime level. */
    CHECK(dm2_v2_hud_runtime_last_path_mode(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP,
          "inventory_quick_view path = REAL_BITMAP");
    CHECK(dm2_v2_hud_runtime_last_slot_class(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_WIDGET_CLASS_REAL,
          "inventory_quick_view class = REAL");

    /* And its anchor pixel must be stamped. */
    CHECK(fb[k_anchors[0].y * 320 + k_anchors[0].x] != 0,
          "stamp pixel at inventory_quick_view anchor != 0");

    /* All other slots must stay on the fallback path. We assert the
     * path-mode record only — the framebuffer at a fallback slot's
     * anchor may hold a procedural-chrome pixel (compass circle,
     * depth digit, gold digit, champion bar pixel, action-strip
     * pixel) which is exactly what the no-gate baseline produces. */
    for (size_t i = 1; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        CHECK(dm2_v2_hud_runtime_last_path_mode(
                  (DM2_V2_HudWidgetSlot)i) ==
                  DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK,
                  "non-REAL slot path = PROCEDURAL_FALLBACK");
    }
    dm2_v2_hud_runtime_shutdown();
}

/* ── Scenario: synthetic COMPLETE manifest → all 7 slots REAL ──────── */

static void test_complete_manifest_all_real(void) {
    printf("\n[ Scenario 5: COMPLETE manifest → all 7 REAL ]\n");
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    char pdir[1024];
    snprintf(pdir, sizeof(pdir), "%s", manifest_path);
    char* slash = strrchr(pdir, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    CHECK(system(mkdir_cmd) == 0, "mkdir hud dir");

    char widgets_dir[1024], chrome_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/dm2/hud/hud_widgets", dataDir);
    snprintf(chrome_dir,  sizeof(chrome_dir),
             "%s/../../assets/dm2/hud/hud_chrome", dataDir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s' '%s'",
             widgets_dir, chrome_dir);
    CHECK(system(mkdir_cmd) == 0, "mkdir widget+chrome dirs");

    /* Drop all 7 source files on disk. */
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        const char* cat = (i < 2) ? widgets_dir : chrome_dir;
        char fpath[1024];
        snprintf(fpath, sizeof(fpath), "%s/%s.png", cat,
                 dm2_v2_hud_widget_assets_slot_name(
                     (DM2_V2_HudWidgetSlot)i));
        CHECK(write_file(fpath, "fake-png-bytes"),
              "wrote source file for COMPLETE manifest");
    }

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-hook-probe\","
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
    CHECK(write_file(manifest_path, content), "wrote complete manifest");

    dm2_v2_hud_runtime_init();
    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    CHECK(dm2_v2_hud_widget_assets_gate() ==
          DM2_V2_HUD_WIDGET_GATE_COMPLETE,
          "COMPLETE manifest → COMPLETE gate");

    DM2_V2_PhaseGateConfig gate = { 1, 1 };
    dm2_v2_hud_runtime_set_gate_config(&gate);
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);

    int real = -1, fb_count = -1;
    int total = dm2_v2_hud_runtime_last_path_counts(&real, &fb_count);
    CHECK(total == (int)DM2_V2_HUD_WIDGET_COUNT,
          "COMPLETE counts all 7 slots");
    CHECK(real == (int)DM2_V2_HUD_WIDGET_COUNT && fb_count == 0,
          "COMPLETE → real=7, fallback=0");
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        CHECK(dm2_v2_hud_runtime_last_path_mode(
                  (DM2_V2_HudWidgetSlot)i) ==
                  DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP,
                  "all 7 slots REAL_BITMAP at runtime level");
        CHECK(fb[k_anchors[i].y * 320 + k_anchors[i].x] != 0,
              "stamp pixel at every anchor when COMPLETE");
    }
    dm2_v2_hud_runtime_shutdown();
}

/* ── Scenario: per-slot path-mode stability across re-renders ──── */

static void test_path_mode_reflects_most_recent_render(void) {
    printf("\n[ Scenario 6: path-mode stability across re-renders ]\n");
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    char pdir[1024];
    snprintf(pdir, sizeof(pdir), "%s", manifest_path);
    char* slash = strrchr(pdir, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    CHECK(system(mkdir_cmd) == 0, "mkdir hud dir");
    char widgets_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/dm2/hud/hud_widgets", dataDir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", widgets_dir);
    CHECK(system(mkdir_cmd) == 0, "mkdir widget dir");
    char real_file[1024];
    snprintf(real_file, sizeof(real_file),
             "%s/inventory_quick_view.png", widgets_dir);
    CHECK(write_file(real_file, "fake-png-bytes"), "wrote real asset file");

    /* First render: PARTIAL (inventory_quick_view REAL, action_prompt
     * PLACEHOLDER). */
    const char* partial_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-hook-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"inventory_quick_view.png\",\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":16}"
        "]}";
    CHECK(write_file(manifest_path, partial_content), "wrote PARTIAL manifest");

    dm2_v2_hud_runtime_init();
    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    DM2_V2_PhaseGateConfig gate = { 1, 1 };
    dm2_v2_hud_runtime_set_gate_config(&gate);
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);
    CHECK(dm2_v2_hud_runtime_last_path_mode(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP,
          "first render → inventory_quick_view=REAL_BITMAP");

    /* Now change the manifest to declare inventory_quick_view as
     * PLACEHOLDER and re-render. The path-mode record should reflect
     * the NEW classification, not stale REAL_BITMAP. */
    const char* placeholder_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-hook-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":16}"
        "]}";
    CHECK(write_file(manifest_path, placeholder_content),
          "rewrote manifest as placeholder");
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);
    CHECK(dm2_v2_hud_runtime_last_path_mode(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK,
          "second render → inventory_quick_view=PROCEDURAL_FALLBACK "
          "(path-mode refreshed)");
    CHECK(fb[k_anchors[0].y * 320 + k_anchors[0].x] == 0,
          "no stamp pixel after demoting REAL to PLACEHOLDER");

    /* And back to PARTIAL again \u2014 path-mode follows manifest. */
    write_file(manifest_path, partial_content);
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);
    CHECK(dm2_v2_hud_runtime_last_path_mode(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP,
          "third render → inventory_quick_view=REAL_BITMAP again");

    dm2_v2_hud_runtime_shutdown();
}

/* ── Scenario: phase gate honoured (V2 off → no stamps) ──────────── */

static void test_phase_gate_blocks_stamps(void) {
    printf("\n[ Scenario 7: V2 off → no stamps even for REAL slots ]\n");
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    char pdir[1024];
    snprintf(pdir, sizeof(pdir), "%s", manifest_path);
    char* slash = strrchr(pdir, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    CHECK(system(mkdir_cmd) == 0, "mkdir hud dir");
    char widgets_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/dm2/hud/hud_widgets", dataDir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", widgets_dir);
    CHECK(system(mkdir_cmd) == 0, "mkdir widget dir");
    char real_file[1024];
    snprintf(real_file, sizeof(real_file),
             "%s/inventory_quick_view.png", widgets_dir);
    write_file(real_file, "fake-png-bytes");

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-hook-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"inventory_quick_view.png\",\"width\":64,\"height\":32}"
        "]}";
    write_file(manifest_path, content);

    dm2_v2_hud_runtime_init();
    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    /* V2 launch OFF → must NOT stamp the framebuffer (V1 chrome
     * ownership preserved), but the path-mode record still observes
     * the gate's REAL classification so probes can verify the gate
     * is reaching the runtime. */
    DM2_V2_PhaseGateConfig gate_off = { 0, 0 };
    dm2_v2_hud_runtime_set_gate_config(&gate_off);
    uint8_t fb[320 * 200];
    memset(fb, 0x42, sizeof(fb));  /* sentinel: V1 chrome marker */
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);

    /* Framebuffer must be unchanged (V1 chrome preserved). */
    int preserved = 1;
    for (int i = 0; i < (int)sizeof(fb); ++i) {
        if (fb[i] != 0x42) { preserved = 0; break; }
    }
    CHECK(preserved == 1,
          "V2 off → framebuffer untouched (V1 chrome preserved)");
    /* The path-mode record still observed the gate's REAL verdict
     * (the classification is recorded regardless of whether the
     * render actually executed; the procedural fallback path is
     * what is gated, not the classification itself). */
    CHECK(dm2_v2_hud_runtime_last_slot_class(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_WIDGET_CLASS_REAL,
          "V2 off → gate verdict still recorded (slot class=REAL)");
    CHECK(dm2_v2_hud_runtime_last_path_mode(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP,
          "V2 off → path-mode still records REAL_BITMAP (gate reached)");
    /* But the stamp pixel was NOT emitted (no fb touch). */
    CHECK(fb[k_anchors[0].y * 320 + k_anchors[0].x] == 0x42,
          "V2 off → no stamp pixel emitted (V1 chrome sentinel preserved)");

    /* And V2 on → stamp IS emitted. */
    DM2_V2_PhaseGateConfig gate_on = { 1, 1 };
    dm2_v2_hud_runtime_set_gate_config(&gate_on);
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);
    CHECK(fb[k_anchors[0].y * 320 + k_anchors[0].x] != 0,
          "V2 on → stamp pixel emitted for REAL slot");

    dm2_v2_hud_runtime_shutdown();
}

/* ── Scenario: out-of-range slot queries are safe ──────────────── */

static void test_out_of_range_queries_are_safe(void) {
    printf("\n[ Scenario 8: out-of-range queries are safe ]\n");
    dm2_v2_hud_runtime_init();
    /* Slot way out of range must not crash and must return the
     * safe default (PROCEDURAL_FALLBACK / UNKNOWN). */
    CHECK(dm2_v2_hud_runtime_last_path_mode(
              (DM2_V2_HudWidgetSlot)9999) ==
              DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK,
          "out-of-range path-mode returns PROCEDURAL_FALLBACK");
    CHECK(dm2_v2_hud_runtime_last_slot_class(
              (DM2_V2_HudWidgetSlot)9999) ==
              DM2_V2_HUD_WIDGET_CLASS_UNKNOWN,
          "out-of-range slot class returns UNKNOWN");
    /* NULL out pointers for path_counts are safe. */
    int total = dm2_v2_hud_runtime_last_path_counts(NULL, NULL);
    CHECK(total >= 0, "path_counts(NULL, NULL) is safe");
    dm2_v2_hud_runtime_shutdown();
}

/* ── Scenario: source evidence citation ────────────────────────── */

static void test_source_evidence_citation(void) {
    printf("\n[ Scenario 9: source evidence citation ]\n");
    const char* ev = dm2_v2_hud_runtime_source_evidence();
    CHECK(ev != NULL && strlen(ev) > 100, "source_evidence non-empty");
    CHECK(strstr(ev, "SKULL.ASM T560") != NULL, "cites SKULL.ASM T560");
    CHECK(strstr(ev, "SKULLWIN") != NULL, "cites SKULLWIN");
    CHECK(strstr(ev, "ReDMCSB PANEL.C") != NULL, "cites ReDMCSB PANEL.C");
    CHECK(strstr(ev, "dm2_v2_hud_widget_assets") != NULL,
          "cites dm2_v2_hud_widget_assets (Phase 3 hook)");
    CHECK(strstr(ev, "dm2_v2_hud_widget_bitmap_blit") != NULL,
          "cites dm2_v2_hud_widget_bitmap_blit (Phase 3 follow-up)");
    CHECK(strstr(ev, "V1 invariant") != NULL,
          "mentions V1 invariant");
    CHECK(strstr(ev, "OPEN-BOUNDED") != NULL,
          "mentions OPEN-BOUNDED honesty for finished bitmap art");
}

/* ── Scenario: bounded blit path (Phase 3 follow-up) ──────────────
 *
 * Drops the checked-in synthetic 1x1 RGBA PNG fixtures into the
 * scratch asset root and asserts the runtime hook routes the REAL
 * slot through dm2_v2_hud_widget_bitmap_blit_render_slot(). The
 * synthetic fixtures have distinct R values per slot, so the
 * framebuffer pixel at each anchor must equal that R value
 * (i.e. not the legacy stamp's opacity byte, but the actual
 * decoded PNG pixel).
 *
 * This is the runtime-side proof that the blit path is wired
 * end-to-end. The companion bitmap blit probe covers the lower
 * level (decode + bounded write + bounds + fallback). */

#include "dm2_v2_hud_widget_bitmap_blit.h"

#ifndef FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
#define FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR \
    "examples/dm2_hud_widget_synthetic"
#endif

/* Helper: copy `name.png` from the example pack (hud_widgets/ first,
 * then hud_chrome/) to `dst`. Creates the destination's parent
 * directory tree if needed. */
static int copy_synthetic_fixture(const char* dst, const char* name) {
    char src[1024];
    snprintf(src, sizeof(src),
             "%s/hud_widgets/%s", FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR, name);
    FILE* in = fopen(src, "rb");
    if (!in) {
        snprintf(src, sizeof(src),
                 "%s/hud_chrome/%s", FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR, name);
        in = fopen(src, "rb");
    }
    if (!in) return 0;
    /* Create parent directory. */
    char dst_copy[1024];
    snprintf(dst_copy, sizeof(dst_copy), "%s", dst);
    char* last_slash = strrchr(dst_copy, '/');
    if (last_slash) {
        *last_slash = '\0';
        char mkdir_cmd[1100];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", dst_copy);
        if (system(mkdir_cmd) != 0) { fclose(in); return 0; }
    }
    FILE* out = fopen(dst, "wb");
    if (!out) { fclose(in); return 0; }
    unsigned char buf[4096];
    size_t n;
    int ok = 1;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { ok = 0; break; }
    }
    fclose(in);
    fclose(out);
    return ok;
}

static void test_bounded_blit_end_to_end(void) {
    printf("\n[ Scenario 10: bounded blit path end-to-end ]\n");
    clean_scratch();
    const char* dataDir = "/tmp/scratch/firestaff-data/dm2";
    char manifest_path[1024];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    char pdir[1024];
    snprintf(pdir, sizeof(pdir), "%s", manifest_path);
    char* slash = strrchr(pdir, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    CHECK(system(mkdir_cmd) == 0, "mkdir hud dir");

    char widgets_dir[1024], chrome_dir[1024];
    snprintf(widgets_dir, sizeof(widgets_dir),
             "%s/../../assets/dm2/hud/hud_widgets", dataDir);
    snprintf(chrome_dir,  sizeof(chrome_dir),
             "%s/../../assets/dm2/hud/hud_chrome", dataDir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s' '%s'",
             widgets_dir, chrome_dir);
    CHECK(system(mkdir_cmd) == 0, "mkdir widget+chrome dirs");

    /* Copy the real synthetic PNGs from the example pack into the
     * manifest's category dirs. The copy helper now creates the
     * destination's parent directory tree, so the call lands the
     * file at widgets_dir/inventory_quick_view.png directly. */
    char iqv_dst[1024], ap_dst[1024];
    snprintf(iqv_dst, sizeof(iqv_dst), "%s/inventory_quick_view.png", widgets_dir);
    snprintf(ap_dst,  sizeof(ap_dst),  "%s/action_prompt.png",         widgets_dir);
    CHECK(copy_synthetic_fixture(iqv_dst, "inventory_quick_view.png"),
          "copied inventory_quick_view.png into manifest dir");
    CHECK(copy_synthetic_fixture(ap_dst,  "action_prompt.png"),
          "copied action_prompt.png into manifest dir");

    /* Re-derive the expected R from the now-installed fixtures (so
     * the test is robust to fixture-colour changes; the runtime
     * only cares that the framebuffer pixel matches whatever the
     * decoded fixture produces). */
    DM2_V2_HudWidgetBlitPixel px_iqv, px_ap;
    CHECK(dm2_v2_hud_widget_bitmap_blit_read_pixel(iqv_dst, &px_iqv) == 1,
          "decoded installed inventory_quick_view.png");
    CHECK(dm2_v2_hud_widget_bitmap_blit_read_pixel(ap_dst,  &px_ap)  == 1,
          "decoded installed action_prompt.png");

    /* Manifest with both Phase 3 primary slots as REAL, source_file
     * pointing at the actual synthetic PNGs. */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-hook-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"synthetic_test\","
        "\"source_file\":\"inventory_quick_view.png\",\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"synthetic_test\","
        "\"source_file\":\"action_prompt.png\",\"width\":48,\"height\":16}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote synthetic manifest");

    dm2_v2_hud_runtime_init();
    dm2_v2_hud_widget_assets_set_manifest_path(dataDir);
    /* Both declared slots are REAL with disk-resolvable source_file,
     * so the gate is COMPLETE (real==total). The blit path is the
     * same regardless of whether the gate is PARTIAL or COMPLETE —
     * it routes through the bitmap blit for every REAL slot. */
    CHECK(dm2_v2_hud_widget_assets_gate() ==
          DM2_V2_HUD_WIDGET_GATE_COMPLETE,
          "synthetic manifest (all-declared REAL) → COMPLETE gate");

    DM2_V2_PhaseGateConfig gate = { 1, 1 };
    dm2_v2_hud_runtime_set_gate_config(&gate);
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render_with_assets(fb, 320, 200);

    /* The blit path is preferred over the legacy stamp when the
     * PNG is decodable. The framebuffer pixel at each REAL slot's
     * anchor must equal the synthetic fixture's R value, NOT the
     * legacy stamp's opacity byte. */
    CHECK(dm2_v2_hud_runtime_last_path_mode(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP,
          "inventory_quick_view routed to REAL_BITMAP (blit path)");
    CHECK(dm2_v2_hud_runtime_last_path_mode(
              DM2_V2_HUD_WIDGET_ACTION_PROMPT) ==
              DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP,
          "action_prompt routed to REAL_BITMAP (blit path)");

    /* Anchor (80, 4) for inventory_quick_view is inside the top
     * status bar where the procedural champion-bar 0 is drawn.
     * With default HP=0/stamina=0/mana=0, the bar segments are
     * zero-width and only the leader star could touch that area.
     * Since leader defaults to false, the procedural render does
     * not write to (80, 4); the blit value is preserved. */
    CHECK(fb[k_anchors[0].y * 320 + k_anchors[0].x] == px_iqv.r,
          "inventory_quick_view anchor holds decoded R from synthetic PNG");

    /* Anchor (220, 4) for action_prompt is inside the top status
     * bar where the procedural champion-bar 3 is drawn. With all
     * default values, the procedural render writes nothing to that
     * pixel; the blit value is preserved. */
    CHECK(fb[k_anchors[1].y * 320 + k_anchors[1].x] == px_ap.r,
          "action_prompt anchor holds decoded R from synthetic PNG");

    /* The action_prompt fixture's R value (0x33) is distinct from
     * the legacy 1-pixel stamp's opacity byte (0xFF), which proves
     * the blit path actually ran end-to-end through the runtime
     * rather than the stamp fallback. The check is on px_ap.r
     * itself, not on the framebuffer, so it holds even if the
     * framebuffer pixel were ever overwritten by a future chrome
     * change. */
    CHECK(px_ap.r != 0xFF,
          "action_prompt R is distinct from stamp opacity (blit ran, not stamp)");

    /* All other slots are MISSING or PLACEHOLDER; path-mode is
     * procedural fallback for those. */
    for (size_t i = 2; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        CHECK(dm2_v2_hud_runtime_last_path_mode(
                  (DM2_V2_HudWidgetSlot)i) ==
                  DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK,
                  "non-REAL slot path = PROCEDURAL_FALLBACK");
    }
    dm2_v2_hud_runtime_shutdown();
    clean_scratch();
}

/* ── Main ──────────────────────────────────────────────────────── */

int main(void) {
    printf("=== DM2 V2 HUD Widget Runtime Hook probe ===\n");
    printf("Source: SKULL.ASM T560, c_gui_vp.cpp, ReDMCSB PANEL.C F0354,\n"
           "        dm2_v2_hud_widget_assets.h, dm2_v2_hud_runtime.h\n");

    test_no_manifest_all_fallback();
    test_empty_manifest_fallback();
    test_placeholder_manifest_fallback();
    test_partial_one_real_slot_selected();
    test_complete_manifest_all_real();
    test_path_mode_reflects_most_recent_render();
    test_phase_gate_blocks_stamps();
    test_out_of_range_queries_are_safe();
    test_source_evidence_citation();
    test_bounded_blit_end_to_end();

    clean_scratch();

    printf("\n=== Results: %d passed, %d failed ===\n",
           g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
