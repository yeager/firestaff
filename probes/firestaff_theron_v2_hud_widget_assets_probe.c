/*
 * firestaff_theron_v2_hud_widget_assets_probe.c — Theron V2 HUD Widget Asset Gate
 *
 * Headless CI probe for the Theron V2 HUD widget asset manifest gate.
 * No game data, no SDL rendering required.
 *
 * Builds synthetic manifests in a temp scratch directory and asserts:
 *   1. NO_MANIFEST gate when path unset
 *   2. NO_MANIFEST gate when manifest file missing
 *   3. PLACEHOLDER gate when manifest valid but no slot declared
 *   4. PLACEHOLDER gate when manifest declares placeholder generator
 *   5. PARTIAL gate when some slots real, some placeholder
 *   6. COMPLETE gate when all slots real
 *   7. PARTIAL gate when manifest declares real generator but
 *      source_file does not resolve on disk
 *   8. Per-slot classification for all 7 Theron-specific slots
 *      (compass_rose, quest_items, dungeon_progress, relic_counter,
 *      rune_indicator, champion_bars, action_strip)
 *   9. uses_placeholder() returns 0 only for REAL slots
 *  10. installed flag mirrors gate (PARTIAL/COMPLETE → 1, else 0)
 *  11. Source evidence citations are present and stable
 *  12. Slot names are stable (gap-list primary slots are present)
 *  13. real_count() returns correct counts per scenario
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
 *   - docs/FIRESTAFF_GAP_LIST.md Theron V2 Phase 3 row
 */

#include "theron_v2_hud_widget_assets_pc34.h"

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
                 "%s/assets/theron/hud/hud_widget_manifest.json", dataDir);
        return;
    }
    size_t la = (size_t)(slash - dataDir);
    if (la >= sizeof(a)) la = sizeof(a) - 1U;
    memcpy(a, dataDir, la); a[la] = '\0';
    slash = strrchr(a, '/');
    if (!slash) {
        snprintf(out, outSize,
                 "%s/assets/theron/hud/hud_widget_manifest.json", dataDir);
        return;
    }
    size_t lb = (size_t)(slash - a);
    if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
    memcpy(b, a, lb); b[lb] = '\0';
    snprintf(out, outSize,
             "%s/assets/theron/hud/hud_widget_manifest.json", b);
}

int main(void) {
    printf("=== Theron V2 HUD Widget Asset Gate probe ===\n");

    /* ── Scenario 1: unset path ───────────────────────────────────── */
    printf("\n[ Scenario 1: unset manifest path ]\n");
    theron_v2_hud_widget_assets_set_manifest_path(NULL);
    check("unset path → NO_MANIFEST gate",
          theron_v2_hud_widget_assets_gate() ==
              THERON_V2_HUD_WIDGET_GATE_NO_MANIFEST);
    check("unset path → installed=0",
          theron_v2_hud_widget_assets_get_installed() == 0);
    check("unset path → compass_rose=MISSING",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_COMPASS_ROSE) ==
              THERON_V2_HUD_WIDGET_CLASS_MISSING);
    check("unset path → quest_items=MISSING",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_QUEST_ITEMS) ==
              THERON_V2_HUD_WIDGET_CLASS_MISSING);
    check("unset path → rune_indicator=MISSING",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_RUNE_INDICATOR) ==
              THERON_V2_HUD_WIDGET_CLASS_MISSING);
    check("unset path → champion_bars=MISSING",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_CHAMPION_BARS) ==
              THERON_V2_HUD_WIDGET_CLASS_MISSING);

    /* ── Scenario 2: set path but no manifest file ───────────────── */
    printf("\n[ Scenario 2: set path, file missing ]\n");
    /* Wipe any leftover scratch so this scenario really starts with
     * no manifest file. The earlier Scenario 1 may have left g_state,
     * and previous test runs may have left the asset tree. */
    system("rm -rf /tmp/scratch/theron_hwa_probe");
    system("rm -rf /tmp/scratch/assets /tmp/scratch/firestaff-data");
    system("mkdir -p /tmp/scratch/firestaff-data/theron");
    theron_v2_hud_widget_assets_set_manifest_path(
        "/tmp/scratch/firestaff-data/theron");
    char mpath[1024];
    expected_manifest_path(mpath, sizeof(mpath),
                            "/tmp/scratch/firestaff-data/theron");
    /* Verify path resolved to the expected location */
    const char* got = theron_v2_hud_widget_assets_get_manifest_path();
    check("manifest path matches expected (assets/theron/hud/...)",
          got && strcmp(got, mpath) == 0);
    check("missing file → NO_MANIFEST gate",
          theron_v2_hud_widget_assets_gate() ==
              THERON_V2_HUD_WIDGET_GATE_NO_MANIFEST);
    check("validate_manifest on missing file → -1",
          theron_v2_hud_widget_assets_validate_manifest(NULL) == -1);

    /* ── Scenario 3: empty manifest → PLACEHOLDER gate ────────────── */
    printf("\n[ Scenario 3: empty manifest ]\n");
    /* Walk up two parents from dataDir to land on the resolved
     * manifest's parent directory. */
    char resolved_mdir[1024];
    {
        const char* dd = "/tmp/scratch/firestaff-data/theron";
        char a[1024], b[1024];
        const char* slash = strrchr(dd, '/');
        size_t la = (size_t)(slash - dd);
        if (la >= sizeof(a)) la = sizeof(a) - 1U;
        memcpy(a, dd, la); a[la] = '\0';
        slash = strrchr(a, '/');
        size_t lb = (size_t)(slash - a);
        if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
        memcpy(b, a, lb); b[lb] = '\0';
        snprintf(resolved_mdir, sizeof(resolved_mdir), "%s/assets/theron/hud", b);
    }
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", resolved_mdir);
    system(mkdir_cmd);
    char manifest_file[1024];
    snprintf(manifest_file, sizeof(manifest_file), "%s/hud_widget_manifest.json",
             resolved_mdir);
    check("wrote empty manifest", write_file(manifest_file, "{}"));
    check("empty manifest → PLACEHOLDER gate",
          theron_v2_hud_widget_assets_gate() ==
              THERON_V2_HUD_WIDGET_GATE_PLACEHOLDER);
    check("empty manifest → first slot MISSING",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_COMPASS_ROSE) ==
              THERON_V2_HUD_WIDGET_CLASS_MISSING);

    /* ── Scenario 4: placeholder manifest → PLACEHOLDER gate ──────── */
    printf("\n[ Scenario 4: placeholder manifest ]\n");
    const char* placeholder_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"theron-hwa-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"compass_rose\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":32,\"height\":32},"
        "{\"id\":\"quest_items\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":16}"
        "]}";
    check("wrote placeholder manifest",
          write_file(manifest_file, placeholder_content));
    check("placeholder → PLACEHOLDER gate",
          theron_v2_hud_widget_assets_gate() ==
              THERON_V2_HUD_WIDGET_GATE_PLACEHOLDER);
    check("placeholder → compass_rose=PLACEHOLDER",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_COMPASS_ROSE) ==
              THERON_V2_HUD_WIDGET_CLASS_PLACEHOLDER);
    check("placeholder → quest_items=PLACEHOLDER",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_QUEST_ITEMS) ==
              THERON_V2_HUD_WIDGET_CLASS_PLACEHOLDER);
    check("placeholder → relic_counter=MISSING (not declared)",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_RELIC_COUNTER) ==
              THERON_V2_HUD_WIDGET_CLASS_MISSING);
    check("placeholder → uses_placeholder=1",
          theron_v2_hud_widget_assets_uses_placeholder(
              THERON_V2_HUD_WIDGET_COMPASS_ROSE) == 1);
    check("placeholder → installed=0",
          theron_v2_hud_widget_assets_get_installed() == 0);

    /* ── Scenario 5: PARTIAL gate (some real, some placeholder) ───── */
    printf("\n[ Scenario 5: PARTIAL gate ]\n");
    /* Create the real asset file so source_file resolves */
    char widget_dir[1024];
    snprintf(widget_dir, sizeof(widget_dir), "%s/hud_widgets", resolved_mdir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", widget_dir);
    system(mkdir_cmd);
    char real_file[1024];
    snprintf(real_file, sizeof(real_file),
             "%s/compass_rose.png", widget_dir);
    write_file(real_file, "fake-png-bytes");
    const char* partial_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"theron-hwa-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"compass_rose\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"compass_rose.png\",\"width\":32,\"height\":32},"
        "{\"id\":\"quest_items\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":16}"
        "]}";
    check("wrote partial manifest",
          write_file(manifest_file, partial_content));
    check("partial → PARTIAL gate",
          theron_v2_hud_widget_assets_gate() ==
              THERON_V2_HUD_WIDGET_GATE_PARTIAL);
    check("partial → compass_rose=REAL",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_COMPASS_ROSE) ==
              THERON_V2_HUD_WIDGET_CLASS_REAL);
    check("partial → quest_items=PLACEHOLDER",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_QUEST_ITEMS) ==
              THERON_V2_HUD_WIDGET_CLASS_PLACEHOLDER);
    check("partial → uses_placeholder(REAL)=0",
          theron_v2_hud_widget_assets_uses_placeholder(
              THERON_V2_HUD_WIDGET_COMPASS_ROSE) == 0);
    check("partial → uses_placeholder(PLACEHOLDER)=1",
          theron_v2_hud_widget_assets_uses_placeholder(
              THERON_V2_HUD_WIDGET_QUEST_ITEMS) == 1);
    check("partial → installed=1",
          theron_v2_hud_widget_assets_get_installed() == 1);
    int total = 0;
    int real = theron_v2_hud_widget_assets_real_count(&total);
    check("partial → real_count=1", real == 1);
    check("partial → total=2", total == 2);

    /* ── Scenario 6: COMPLETE gate (all 7 slots REAL) ─────────────── */
    printf("\n[ Scenario 6: COMPLETE gate ]\n");
    /* Create both category directories. Theron slots 0..4 are in
     * hud_widgets/, slots 5..6 in hud_chrome/ — matching the
     * k_slot_table categorisation that drives source_file resolution. */
    char chrome_dir[1024];
    snprintf(chrome_dir, sizeof(chrome_dir), "%s/hud_chrome", resolved_mdir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", chrome_dir);
    system(mkdir_cmd);
    /* Create all 7 source files on disk — slots 0..4 in hud_widgets/,
     * slots 5..6 in hud_chrome/. */
    for (size_t i = 0; i < THERON_V2_HUD_WIDGET_COUNT; ++i) {
        const char* cat = (i <= 4) ? widget_dir : chrome_dir;
        char fpath[1024];
        snprintf(fpath, sizeof(fpath), "%s/%s.png", cat,
                 theron_v2_hud_widget_assets_slot_name(
                     (Theron_V2_HudWidgetSlot)i));
        write_file(fpath, "fake-png-bytes");
    }
    const char* complete_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"theron-hwa-probe\","
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
    check("wrote complete manifest",
          write_file(manifest_file, complete_content));
    check("complete → COMPLETE gate",
          theron_v2_hud_widget_assets_gate() ==
              THERON_V2_HUD_WIDGET_GATE_COMPLETE);
    check("complete → installed=1",
          theron_v2_hud_widget_assets_get_installed() == 1);
    total = 0;
    real = theron_v2_hud_widget_assets_real_count(&total);
    check("complete → real_count=7", real == 7);
    check("complete → total=7", total == 7);
    for (size_t i = 0; i < THERON_V2_HUD_WIDGET_COUNT; ++i) {
        check("complete → all 7 slots REAL",
              theron_v2_hud_widget_assets_classify_slot(
                  (Theron_V2_HudWidgetSlot)i) ==
                  THERON_V2_HUD_WIDGET_CLASS_REAL);
        check("complete → all 7 slots uses_placeholder=0",
              theron_v2_hud_widget_assets_uses_placeholder(
                  (Theron_V2_HudWidgetSlot)i) == 0);
    }

    /* ── Scenario 7: PARTIAL via missing source_file ──────────────── */
    printf("\n[ Scenario 7: PARTIAL via missing source_file ]\n");
    /* Remove one of the real files. Champion_bars is in hud_chrome/. */
    {
        char fpath[1024];
        snprintf(fpath, sizeof(fpath), "%s/champion_bars.png", chrome_dir);
        unlink(fpath);
    }
    /* Re-read manifest: still all 7 declared as pbr_hero, but
     * champion_bars file is missing → PARTIAL (or PLACEHOLDER if no
     * other REAL slots remain; here compass_rose is still REAL so the
     * gate is PARTIAL). */
    check("manifest with missing source_file → PARTIAL gate",
          theron_v2_hud_widget_assets_gate() ==
              THERON_V2_HUD_WIDGET_GATE_PARTIAL);
    check("missing source_file → champion_bars=PARTIAL",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_CHAMPION_BARS) ==
              THERON_V2_HUD_WIDGET_CLASS_PARTIAL);
    /* Recreate for the rest of the test */
    {
        char fpath[1024];
        snprintf(fpath, sizeof(fpath), "%s/champion_bars.png", chrome_dir);
        write_file(fpath, "fake-png-bytes");
    }

    /* ── Scenario 8: PARTIAL via malformed entry ──────────────────── */
    printf("\n[ Scenario 8: PARTIAL via malformed entry ]\n");
    /* Replace the manifest with one where the rune_indicator entry is
     * missing required fields. */
    const char* malformed_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"theron-hwa-probe\","
        "\"hud_widgets\":["
        "{\"id\":\"compass_rose\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"compass_rose.png\",\"width\":32,\"height\":32},"
        "{\"id\":\"rune_indicator\"}"  /* missing required fields */
        "]}";
    check("wrote malformed manifest",
          write_file(manifest_file, malformed_content));
    check("malformed → PARTIAL gate (compass_rose still REAL)",
          theron_v2_hud_widget_assets_gate() ==
              THERON_V2_HUD_WIDGET_GATE_PARTIAL);
    check("malformed → rune_indicator=PARTIAL",
          theron_v2_hud_widget_assets_classify_slot(
              THERON_V2_HUD_WIDGET_RUNE_INDICATOR) ==
              THERON_V2_HUD_WIDGET_CLASS_PARTIAL);

    /* ── Scenario 9: names + evidence + count invariants ──────────── */
    printf("\n[ Names + evidence + count ]\n");
    check("THERON_V2_HUD_WIDGET_COUNT=7",
          THERON_V2_HUD_WIDGET_COUNT == 7);
    check("Phase 3 primary slot 0 = compass_rose",
          strcmp(theron_v2_hud_widget_assets_slot_name(
              THERON_V2_HUD_WIDGET_COMPASS_ROSE),
              "compass_rose") == 0);
    check("Phase 3 primary slot 4 = rune_indicator",
          strcmp(theron_v2_hud_widget_assets_slot_name(
              THERON_V2_HUD_WIDGET_RUNE_INDICATOR),
              "rune_indicator") == 0);
    check("chrome supporting slot 5 = champion_bars",
          strcmp(theron_v2_hud_widget_assets_slot_name(
              THERON_V2_HUD_WIDGET_CHAMPION_BARS),
              "champion_bars") == 0);
    check("chrome supporting slot 6 = action_strip",
          strcmp(theron_v2_hud_widget_assets_slot_name(
              THERON_V2_HUD_WIDGET_ACTION_STRIP),
              "action_strip") == 0);

    const char* ev = theron_v2_hud_widget_assets_source_evidence();
    check("source_evidence cites THQUEST.ASM T600",
          ev != NULL && strstr(ev, "THQUEST.ASM T600") != NULL);
    check("source_evidence cites THQUEST.ASM T900 (rune magic)",
          ev != NULL && strstr(ev, "THQUEST.ASM T900") != NULL);
    check("source_evidence cites HuC6260 (PC Engine VDC)",
          ev != NULL && strstr(ev, "HuC6260") != NULL);
    check("source_evidence cites ReDMCSB PANEL.C",
          ev != NULL && strstr(ev, "ReDMCSB PANEL.C") != NULL);
    check("source_evidence cites FIRESTAFF_GAP_LIST",
          ev != NULL && strstr(ev, "FIRESTAFF_GAP_LIST") != NULL);
    check("source_evidence mentions dmweb",
          ev != NULL && strstr(ev, "dmweb") != NULL);
    check("source_evidence mentions sibling DM2 widget assets",
          ev != NULL && strstr(ev, "dm2_v2_hud_widget_assets") != NULL);
    check("source_evidence mentions sibling Theron V2.2 modern assets",
          ev != NULL && strstr(ev, "theron_v22_modern_assets") != NULL);

    /* ── Clean up ──────────────────────────────────────────────────── */
    system("rm -rf /tmp/scratch/theron_hwa_probe");
    system("rm -rf /tmp/scratch/firestaff-data");
    theron_v2_hud_widget_assets_set_manifest_path(NULL);

    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
