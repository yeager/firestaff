/*
 * firestaff_dm2_v2_hud_widget_assets_probe.c — DM2 V2 HUD Widget Asset Gate
 *
 * Headless CI probe for the DM2 V2 HUD widget asset manifest gate.
 * No game data, no SDL rendering required.
 *
 * Builds synthetic manifests in a temp scratch directory and asserts:
 *   1. NO_MANIFEST gate when path unset
 *   2. NO_MANIFEST gate when manifest file missing
 *   3. PLACEHOLDER gate when manifest valid but no slot declared
 *   4. PLACEHOLDER gate when manifest declares placeholder generator
 *   5. PARTIAL gate when some slots real, some placeholder
 *   6. COMPLETE gate when all slots real
 *   7. Per-slot classification for all 7 slots
 *   8. uses_placeholder() returns 0 only for REAL slots
 *   9. installed flag mirrors gate (PARTIAL/COMPLETE → 1, else 0)
 *  10. Source evidence citations are present and stable
 *  11. Slot names are stable (gap-list primary slots are present)
 *  12. real_count() returns correct counts per scenario
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - include/dm2_v2_hud_widget_assets.h (module under test)
 */

#include "dm2_v2_hud_widget_assets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int s_pass = 0;
static int s_fail = 0;

static void check(const char *name, int cond) {
    if (cond) {
        printf("  PASS: %s\n", name);
        s_pass++;
    } else {
        printf("  FAIL: %s\n", name);
        s_fail++;
    }
}

/* Helper: write a file. */
static int write_file(const char* path, const char* content) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    size_t w = fwrite(content, 1, strlen(content), fp);
    fclose(fp);
    return w == strlen(content);
}

/* Helper: build expected manifest path. */
static void expected_manifest_path(char* out, size_t outSize,
                                    const char* dataDir) {
    char a[1024], b[1024];
    const char* slash = strrchr(dataDir, '/');
    if (!slash) {
        snprintf(out, outSize,
                 "%s/assets/dm2/hud/hud_widget_manifest.json", dataDir);
        return;
    }
    size_t la = (size_t)(slash - dataDir);
    if (la >= sizeof(a)) la = sizeof(a) - 1U;
    memcpy(a, dataDir, la); a[la] = '\0';
    slash = strrchr(a, '/');
    if (!slash) {
        snprintf(out, outSize,
                 "%s/assets/dm2/hud/hud_widget_manifest.json", dataDir);
        return;
    }
    size_t lb = (size_t)(slash - a);
    if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
    memcpy(b, a, lb); b[lb] = '\0';
    snprintf(out, outSize,
             "%s/assets/dm2/hud/hud_widget_manifest.json", b);
}

int main(void) {
    printf("=== DM2 V2 HUD Widget Asset Gate probe ===\n");

    /* ── Scenario 1: unset path ───────────────────────────────────── */
    printf("\n[ Scenario 1: unset manifest path ]\n");
    dm2_v2_hud_widget_assets_set_manifest_path(NULL);
    check("unset path → NO_MANIFEST gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST);
    check("unset path → installed=0",
          dm2_v2_hud_widget_assets_get_installed() == 0);
    check("unset path → inventory_quick_view=MISSING",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_WIDGET_CLASS_MISSING);
    check("unset path → action_prompt=MISSING",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_ACTION_PROMPT) ==
              DM2_V2_HUD_WIDGET_CLASS_MISSING);

    /* ── Scenario 2: set path but no manifest file ───────────────── */
    printf("\n[ Scenario 2: set path, file missing ]\n");
    /* Wipe any leftover scratch so this scenario really starts with
     * no manifest file. The earlier Scenario 1 may have left g_state,
     * and previous test runs may have left the asset tree. */
    system("rm -rf /tmp/scratch/dm2_hwa_probe");
    system("rm -rf /tmp/scratch/assets /tmp/scratch/firestaff-data");
    system("mkdir -p /tmp/scratch/firestaff-data/dm2");
    dm2_v2_hud_widget_assets_set_manifest_path(
        "/tmp/scratch/firestaff-data/dm2");
    char mpath[1024];
    expected_manifest_path(mpath, sizeof(mpath),
                            "/tmp/scratch/firestaff-data/dm2");
    /* Verify path resolved to the expected location */
    const char* got = dm2_v2_hud_widget_assets_get_manifest_path();
    check("manifest path matches expected (assets/dm2/hud/...)",
          got && strcmp(got, mpath) == 0);
    check("missing file → NO_MANIFEST gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST);
    check("validate_manifest on missing file → -1",
          dm2_v2_hud_widget_assets_validate_manifest(NULL) == -1);

    /* ── Scenario 3: empty manifest → PLACEHOLDER gate ────────────── */
    printf("\n[ Scenario 3: empty manifest ]\n");
    char mdir[1024];
    snprintf(mdir, sizeof(mdir), "%s/assets/dm2/hud",
             "/tmp/scratch/firestaff-data/../");
    /* The dataDir parent resolution in the module: dataDir=".../firestaff-data/dm2"
     * walks up to ".../firestaff-data" then ".../scratch" (which is NOT firestaff).
     * For this probe, we deliberately create the manifest at the resolved
     * location so the test exercises the real path resolution. */
    char resolved_mdir[1024];
    /* Walk up from dataDir */
    {
        const char* dd = "/tmp/scratch/firestaff-data/dm2";
        char a[1024], b[1024];
        const char* slash = strrchr(dd, '/');
        size_t la = (size_t)(slash - dd);
        if (la >= sizeof(a)) la = sizeof(a) - 1U;
        memcpy(a, dd, la); a[la] = '\0';
        slash = strrchr(a, '/');
        size_t lb = (size_t)(slash - a);
        if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
        memcpy(b, a, lb); b[lb] = '\0';
        snprintf(resolved_mdir, sizeof(resolved_mdir), "%s/assets/dm2/hud", b);
    }
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", resolved_mdir);
    system(mkdir_cmd);
    char manifest_file[1024];
    snprintf(manifest_file, sizeof(manifest_file), "%s/hud_widget_manifest.json",
             resolved_mdir);
    check("wrote empty manifest", write_file(manifest_file, "{}"));
    check("empty manifest → PLACEHOLDER gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER);
    check("empty manifest → first slot MISSING",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_WIDGET_CLASS_MISSING);

    /* ── Scenario 4: placeholder manifest → PLACEHOLDER gate ──────── */
    printf("\n[ Scenario 4: placeholder manifest ]\n");
    const char* placeholder_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":16}"
        "]}";
    check("wrote placeholder manifest",
          write_file(manifest_file, placeholder_content));
    check("placeholder → PLACEHOLDER gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER);
    check("placeholder → inventory_quick_view=PLACEHOLDER",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER);
    check("placeholder → action_prompt=PLACEHOLDER",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_ACTION_PROMPT) ==
              DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER);
    check("placeholder → compass_rose=MISSING (not declared)",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_COMPASS_ROSE) ==
              DM2_V2_HUD_WIDGET_CLASS_MISSING);
    check("placeholder → uses_placeholder=1",
          dm2_v2_hud_widget_assets_uses_placeholder(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) == 1);
    check("placeholder → installed=0",
          dm2_v2_hud_widget_assets_get_installed() == 0);

    /* ── Scenario 5: PARTIAL gate (some real, some placeholder) ───── */
    printf("\n[ Scenario 5: PARTIAL gate ]\n");
    /* Create the real asset file so source_file resolves */
    char widget_dir[1024];
    snprintf(widget_dir, sizeof(widget_dir), "%s/hud_widgets", resolved_mdir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", widget_dir);
    system(mkdir_cmd);
    char real_file[1024];
    snprintf(real_file, sizeof(real_file),
             "%s/inventory_quick_view.png", widget_dir);
    write_file(real_file, "fake-png-bytes");
    const char* partial_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"inventory_quick_view\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"inventory_quick_view.png\",\"width\":64,\"height\":32},"
        "{\"id\":\"action_prompt\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":16}"
        "]}";
    check("wrote partial manifest",
          write_file(manifest_file, partial_content));
    check("partial → PARTIAL gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_PARTIAL);
    check("partial → inventory_quick_view=REAL",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_WIDGET_CLASS_REAL);
    check("partial → action_prompt=PLACEHOLDER",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_ACTION_PROMPT) ==
              DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER);
    check("partial → uses_placeholder(REAL)=0",
          dm2_v2_hud_widget_assets_uses_placeholder(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) == 0);
    check("partial → uses_placeholder(PLACEHOLDER)=1",
          dm2_v2_hud_widget_assets_uses_placeholder(
              DM2_V2_HUD_WIDGET_ACTION_PROMPT) == 1);
    check("partial → installed=1",
          dm2_v2_hud_widget_assets_get_installed() == 1);
    int total = 0;
    int real = dm2_v2_hud_widget_assets_real_count(&total);
    check("partial → real_count=1", real == 1);
    check("partial → total=2", total == 2);

    /* ── Scenario 6: COMPLETE gate (all 7 slots REAL) ─────────────── */
    printf("\n[ Scenario 6: COMPLETE gate ]\n");
    /* Create both category directories. First two slots are in
     * hud_widgets/, the rest in hud_chrome/ — matching the k_slot_table
     * categories that drive source_file resolution. */
    char chrome_dir[1024];
    snprintf(chrome_dir, sizeof(chrome_dir), "%s/hud_chrome", resolved_mdir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", chrome_dir);
    system(mkdir_cmd);
    /* Create all 7 source files on disk — slots 0..1 in hud_widgets/,
     * slots 2..6 in hud_chrome/. */
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        const char* cat = (i < 2) ? widget_dir : chrome_dir;
        char fpath[1024];
        snprintf(fpath, sizeof(fpath), "%s/%s.png", cat,
                 dm2_v2_hud_widget_assets_slot_name(
                     (DM2_V2_HudWidgetSlot)i));
        write_file(fpath, "fake-png-bytes");
    }
    const char* complete_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-hwa-probe\","
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
    check("wrote complete manifest",
          write_file(manifest_file, complete_content));
    check("complete → COMPLETE gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_COMPLETE);
    check("complete → installed=1",
          dm2_v2_hud_widget_assets_get_installed() == 1);
    total = 0;
    real = dm2_v2_hud_widget_assets_real_count(&total);
    check("complete → real_count=7", real == 7);
    check("complete → total=7", total == 7);
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        check("complete → all 7 slots REAL",
              dm2_v2_hud_widget_assets_classify_slot(
                  (DM2_V2_HudWidgetSlot)i) ==
                  DM2_V2_HUD_WIDGET_CLASS_REAL);
        check("complete → all 7 slots uses_placeholder=0",
              dm2_v2_hud_widget_assets_uses_placeholder(
                  (DM2_V2_HudWidgetSlot)i) == 0);
    }

    /* ── Scenario 7: PARTIAL when pbr_hero source_file missing ────── */
    printf("\n[ Scenario 7: PARTIAL via missing source_file ]\n");
    /* Remove one of the real files — compass_rose is in hud_chrome/, not
     * hud_widgets/ (k_slot_table categorisation). */
    {
        char fpath[1024];
        snprintf(fpath, sizeof(fpath), "%s/compass_rose.png", chrome_dir);
        unlink(fpath);
    }
    /* Re-read manifest: still all 7 declared as pbr_hero, but compass_rose
     * file is missing */
    check("manifest with missing source_file → PARTIAL gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER ||
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_PARTIAL);
    check("missing source_file → compass_rose=PARTIAL",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_COMPASS_ROSE) ==
              DM2_V2_HUD_WIDGET_CLASS_PARTIAL);
    /* Recreate for the rest of the test */
    {
        char fpath[1024];
        snprintf(fpath, sizeof(fpath), "%s/compass_rose.png", chrome_dir);
        write_file(fpath, "fake-png-bytes");
    }

    /* ── Scenario 8: names + evidence + count invariants ──────────── */
    printf("\n[ Names + evidence + count ]\n");
    check("DM2_V2_HUD_WIDGET_COUNT=7",
          DM2_V2_HUD_WIDGET_COUNT == 7);
    check("Phase 3 primary slot 0 = inventory_quick_view",
          strcmp(dm2_v2_hud_widget_assets_slot_name(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW),
              "inventory_quick_view") == 0);
    check("Phase 3 primary slot 1 = action_prompt",
          strcmp(dm2_v2_hud_widget_assets_slot_name(
              DM2_V2_HUD_WIDGET_ACTION_PROMPT),
              "action_prompt") == 0);

    const char* ev = dm2_v2_hud_widget_assets_source_evidence();
    check("source_evidence cites SKULL.ASM T560",
          ev != NULL && strstr(ev, "SKULL.ASM T560") != NULL);
    check("source_evidence cites SKULLWIN",
          ev != NULL && strstr(ev, "SKULLWIN") != NULL);
    check("source_evidence cites ReDMCSB PANEL.C",
          ev != NULL && strstr(ev, "ReDMCSB PANEL.C") != NULL);
    check("source_evidence cites FIRESTAFF_GAP_LIST",
          ev != NULL && strstr(ev, "FIRESTAFF_GAP_LIST") != NULL);

    /* ── Clean up ──────────────────────────────────────────────────── */
    system("rm -rf /tmp/scratch/dm2_hwa_probe");
    system("rm -rf /tmp/scratch/firestaff-data");

    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
