/*
 * firestaff_dm1_v22_finished_art_material_gate_probe.c
 *
 * DM1 V2.2 finished-art / material gate headless CI probe.
 *
 * Headless probe for the DM1 V2.2 finished-art / material screenshot
 * pixel gate. No game data, no SDL rendering required. Builds synthetic
 * modern_asset_manifest.json fixtures under /tmp/scratch/ and verifies
 * every gate state the gate exposes:
 *
 *   Scenario 1: unset manifest path                -> NO_MANIFEST gate
 *   Scenario 2: set path but file missing          -> NO_MANIFEST gate
 *   Scenario 3: empty manifest                     -> SYNTHETIC_PLACEHOLDER
 *   Scenario 4: placeholder-only manifest          -> SYNTHETIC_PLACEHOLDER
 *   Scenario 5: PARTIAL (some REAL, some PLACEHOLDER) -> PARTIAL
 *   Scenario 6: FINISHED_REAL (all 7 slots REAL)   -> FINISHED_REAL
 *   Scenario 7: PARTIAL via missing source_file    -> PARTIAL
 *                                                       (declared slots
 *                                                        PARTIAL-only)
 *   Scenario 8: bad PNG provenance                 -> PARTIAL
 *               (text file named .png, or IHDR dimensions mismatched)
 *   Scenario 9: garbage manifest                   -> NO_MANIFEST
 *                                                       (parser rejects)
 *   Scenario 10: per-slot invariant suite
 *               - 7 slots tracked
 *               - slot names match hero_01 ids from the sibling
 *                 SKIP-only real-asset test
 *               - source_evidence cites DUNVIEW.C / DUNGEON.C / PANEL.C
 *                 / PNG IHDR / FIRESTAFF_GAP_LIST / Honest boundary
 *
 * Source:
 *   - ReDMCSB DUNVIEW.C:6697-6816 (DM1 viewport composition order)
 *   - ReDMCSB DUNGEON.C:2238-2246 (square-type decode)
 *   - include/dm1_v22_finished_art_material_gate_pc34.h (module under test)
 *   - sibling probes/firestaff_dm2_v2_hud_widget_assets_probe.c
 */

#include "dm1_v22_finished_art_material_gate_pc34.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ── Test harness ───────────────────────────────────────────────── */

static int s_pass = 0;
static int s_fail = 0;

static void check(const char* name, int cond) {
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

static void put_be32(unsigned char* p, unsigned v) {
    p[0] = (unsigned char)((v >> 24) & 0xffU);
    p[1] = (unsigned char)((v >> 16) & 0xffU);
    p[2] = (unsigned char)((v >> 8) & 0xffU);
    p[3] = (unsigned char)(v & 0xffU);
}

static void put_le32(unsigned char* p, unsigned v) {
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8) & 0xffU);
    p[2] = (unsigned char)((v >> 16) & 0xffU);
    p[3] = (unsigned char)((v >> 24) & 0xffU);
}

static int write_png_header_file(const char* path,
                                 unsigned width,
                                 unsigned height) {
    static const unsigned char sig[8] = {
        0x89u, 0x50u, 0x4eu, 0x47u, 0x0du, 0x0au, 0x1au, 0x0au
    };
    unsigned char hdr[24];
    FILE* fp;
    memcpy(hdr, sig, sizeof(sig));
    hdr[8] = 0x00u; hdr[9] = 0x00u; hdr[10] = 0x00u; hdr[11] = 0x0du;
    memcpy(hdr + 12, "IHDR", 4);
    put_be32(hdr + 16, width);
    put_be32(hdr + 20, height);
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_bmp_header_file(const char* path,
                                 unsigned width,
                                 unsigned height) {
    unsigned char hdr[26];
    FILE* fp;
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 'B';
    hdr[1] = 'M';
    put_le32(hdr + 2, 26U);
    put_le32(hdr + 10, 26U);
    put_le32(hdr + 14, 12U);
    put_le32(hdr + 18, width);
    put_le32(hdr + 22, height);
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_all_real_manifest_with_receipt(const char* path,
                                                const char* receipt_generator,
                                                const char* receipt_source,
                                                const char* receipt_hash,
                                                const char* receipt_material_gate) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    fprintf(fp,
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-probe\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"wall_d3_carved_hero_01.png\",\"width\":64,\"height\":64}"
        "],"
        "\"floor_shapes\":["
        "{\"id\":\"floor_plain_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"floor_plain_hero_01.png\",\"width\":64,\"height\":64},"
        "{\"id\":\"floor_pit_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"floor_pit_hero_01.png\",\"width\":64,\"height\":64}"
        "],"
        "\"creature_shapes\":["
        "{\"id\":\"creature_demon_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"creature_demon_hero_01.png\",\"width\":48,\"height\":48}"
        "],"
        "\"champion_portraits\":["
        "{\"id\":\"champion_warrior_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"champion_warrior_hero_01.png\","
        "\"width\":48,\"height\":48}"
        "],"
        "\"door_shapes\":["
        "{\"id\":\"door_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"door_hero_01.png\",\"width\":32,\"height\":48}"
        "],"
        "\"field_shapes\":["
        "{\"id\":\"field_teleporter_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"field_teleporter_hero_01.png\","
        "\"width\":64,\"height\":64}"
        "]");
    if (receipt_generator) {
        fprintf(fp,
            ",\"runtime_screenshot_receipts\":["
            "{\"id\":\"dm1_v22_real_screenshot_material_receipt_01\","
            "\"generator\":\"%s\","
            "\"source_file\":\"%s\","
            "\"width\":320,\"height\":200,"
            "\"frame_hash\":\"%s\","
            "\"material_gate\":\"%s\"}"
            "]",
            receipt_generator,
            receipt_source ? receipt_source : "",
            receipt_hash ? receipt_hash : "",
            receipt_material_gate ? receipt_material_gate : "");
    }
    fprintf(fp, "}");
    return fclose(fp) == 0;
}

/* Helper: build expected manifest path. */
static void expected_manifest_path(char* out, size_t outSize,
                                    const char* dataDir) {
    char a[1024], b[1024];
    const char* slash = strrchr(dataDir, '/');
    if (!slash) {
        snprintf(out, outSize,
                 "%s/assets/dm1/modern/modern_asset_manifest.json", dataDir);
        return;
    }
    size_t la = (size_t)(slash - dataDir);
    if (la >= sizeof(a)) la = sizeof(a) - 1U;
    memcpy(a, dataDir, la); a[la] = '\0';
    slash = strrchr(a, '/');
    if (!slash) {
        snprintf(out, outSize,
                 "%s/assets/dm1/modern/modern_asset_manifest.json", dataDir);
        return;
    }
    size_t lb = (size_t)(slash - a);
    if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
    memcpy(b, a, lb); b[lb] = '\0';
    snprintf(out, outSize,
             "%s/assets/dm1/modern/modern_asset_manifest.json", b);
}

/* ── Probe body ────────────────────────────────────────────────── */

int main(void) {
    printf("=== DM1 V2.2 finished-art material gate probe ===\n");

    /* ── Scenario 1: unset path ───────────────────────────────────── */
    printf("\n[ Scenario 1: unset manifest path ]\n");
    dm1_v22_famg_set_manifest_path(NULL);
    check("unset path -> NO_MANIFEST gate",
          dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_NO_MANIFEST);
    check("unset path -> installed=0",
          dm1_v22_famg_get_installed() == 0);
    check("unset path -> wall_d3_carved=MISSING",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_WALL_D3_CARVED) ==
              DM1_V22_FAMG_CLASS_MISSING);
    check("unset path -> floor_plain=MISSING",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_FLOOR_PLAIN) ==
              DM1_V22_FAMG_CLASS_MISSING);
    check("unset path -> is_finished_real=0",
          dm1_v22_famg_is_finished_real() == 0);
    check("unset path -> is_synthetic_or_partial=0 (NOT_PROBED)",
          dm1_v22_famg_is_synthetic_or_partial() == 0);

    /* ── Scenario 2: set path but no manifest file ───────────────── */
    printf("\n[ Scenario 2: set path, file missing ]\n");
    /* Wipe any leftover scratch so this scenario really starts with
     * no manifest file. Earlier scenarios may have left state, and
     * previous test runs may have left the asset tree. */
    system("rm -rf /tmp/scratch/dm1-famg-probe");
    system("rm -rf /tmp/scratch/assets /tmp/scratch/dm1-famg-data");
    system("mkdir -p /tmp/scratch/dm1-famg-data/data/dm1");
    dm1_v22_famg_set_manifest_path(
        "/tmp/scratch/dm1-famg-data/data/dm1");
    char mpath[1024];
    expected_manifest_path(mpath, sizeof(mpath),
                            "/tmp/scratch/dm1-famg-data/data/dm1");
    const char* got = dm1_v22_famg_get_manifest_path();
    check("manifest path matches expected (assets/dm1/modern/...)",
          got && strcmp(got, mpath) == 0);
    check("missing file -> NO_MANIFEST gate",
          dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_NO_MANIFEST);
    check("validate_manifest on missing file -> -1",
          dm1_v22_famg_validate_manifest(NULL) == -1);

    /* ── Scenario 3: empty manifest -> SYNTHETIC_PLACEHOLDER ─────── */
    printf("\n[ Scenario 3: empty manifest ]\n");
    char resolved_mdir[1024];
    {
        const char* dd = "/tmp/scratch/dm1-famg-data/data/dm1";
        char a[1024], b[1024];
        const char* slash = strrchr(dd, '/');
        size_t la = (size_t)(slash - dd);
        if (la >= sizeof(a)) la = sizeof(a) - 1U;
        memcpy(a, dd, la); a[la] = '\0';
        slash = strrchr(a, '/');
        size_t lb = (size_t)(slash - a);
        if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
        memcpy(b, a, lb); b[lb] = '\0';
        snprintf(resolved_mdir, sizeof(resolved_mdir),
                 "%s/assets/dm1/modern", b);
    }
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", resolved_mdir);
    system(mkdir_cmd);
    char manifest_file[1024];
    snprintf(manifest_file, sizeof(manifest_file),
             "%s/modern_asset_manifest.json", resolved_mdir);
    check("wrote empty manifest", write_file(manifest_file, "{}"));
    check("empty manifest -> SYNTHETIC_PLACEHOLDER gate",
          dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER);
    check("empty manifest -> first slot MISSING",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_WALL_D3_CARVED) ==
              DM1_V22_FAMG_CLASS_MISSING);

    /* ── Scenario 4: placeholder manifest -> SYNTHETIC_PLACEHOLDER ─ */
    printf("\n[ Scenario 4: placeholder-only manifest ]\n");
    /* The DM1 V2.2 modern-asset runtime treats placeholder slots as
     * procedural fallbacks. This is the honest CI default baseline. */
    const char* placeholder_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-probe\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":64}"
        "],"
        "\"floor_shapes\":["
        "{\"id\":\"floor_plain_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":64},"
        "{\"id\":\"floor_pit_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":64}"
        "],"
        "\"creature_shapes\":["
        "{\"id\":\"creature_demon_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":48}"
        "],"
        "\"champion_portraits\":["
        "{\"id\":\"champion_warrior_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":48}"
        "],"
        "\"door_shapes\":["
        "{\"id\":\"door_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":32,\"height\":48}"
        "],"
        "\"field_shapes\":["
        "{\"id\":\"field_teleporter_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":64}"
        "]}";
    check("wrote placeholder manifest",
          write_file(manifest_file, placeholder_content));
    check("placeholder -> SYNTHETIC_PLACEHOLDER gate",
          dm1_v22_famg_gate() ==
              DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER);
    check("placeholder -> wall_d3_carved=PLACEHOLDER",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_WALL_D3_CARVED) ==
              DM1_V22_FAMG_CLASS_PLACEHOLDER);
    check("placeholder -> floor_plain=PLACEHOLDER",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_FLOOR_PLAIN) ==
              DM1_V22_FAMG_CLASS_PLACEHOLDER);
    check("placeholder -> creature_demon=PLACEHOLDER",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_CREATURE_DEMON) ==
              DM1_V22_FAMG_CLASS_PLACEHOLDER);
    check("placeholder -> champion_warrior=PLACEHOLDER",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_CHAMPION_WARRIOR) ==
              DM1_V22_FAMG_CLASS_PLACEHOLDER);
    check("placeholder -> door_front=PLACEHOLDER",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_DOOR_FRONT) ==
              DM1_V22_FAMG_CLASS_PLACEHOLDER);
    check("placeholder -> teleporter_field=PLACEHOLDER",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_TELEPORTER_FIELD) ==
              DM1_V22_FAMG_CLASS_PLACEHOLDER);
    check("placeholder -> uses_placeholder=1 (wall)",
          dm1_v22_famg_uses_placeholder(
              DM1_V22_FAMG_WALL_D3_CARVED) == 1);
    check("placeholder -> installed=0",
          dm1_v22_famg_get_installed() == 0);
    check("placeholder -> is_synthetic_or_partial=1",
          dm1_v22_famg_is_synthetic_or_partial() == 1);

    /* ── Scenario 5: PARTIAL gate (some REAL, some PLACEHOLDER) ───── */
    printf("\n[ Scenario 5: PARTIAL gate ]\n");
    /* Create the real asset file so wall_d3_carved source_file resolves. */
    char wall_dir[1024];
    snprintf(wall_dir, sizeof(wall_dir), "%s/wall_shapes", resolved_mdir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", wall_dir);
    system(mkdir_cmd);
    char real_file[1024];
    snprintf(real_file, sizeof(real_file),
             "%s/wall_d3_carved_hero_01.png", wall_dir);
    write_png_header_file(real_file, 64U, 64U);

    const char* partial_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-probe\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"wall_d3_carved_hero_01.png\","
        "\"width\":64,\"height\":64}"
        "],"
        "\"floor_shapes\":["
        "{\"id\":\"floor_plain_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":64},"
        "{\"id\":\"floor_pit_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":64}"
        "],"
        "\"creature_shapes\":["
        "{\"id\":\"creature_demon_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":48}"
        "],"
        "\"champion_portraits\":["
        "{\"id\":\"champion_warrior_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":48,\"height\":48}"
        "],"
        "\"door_shapes\":["
        "{\"id\":\"door_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":32,\"height\":48}"
        "],"
        "\"field_shapes\":["
        "{\"id\":\"field_teleporter_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder.png\",\"width\":64,\"height\":64}"
        "]}";
    check("wrote partial manifest",
          write_file(manifest_file, partial_content));
    check("partial -> PARTIAL gate",
          dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_PARTIAL);
    check("partial -> wall_d3_carved=REAL",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_WALL_D3_CARVED) ==
              DM1_V22_FAMG_CLASS_REAL);
    check("partial -> floor_plain=PLACEHOLDER",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_FLOOR_PLAIN) ==
              DM1_V22_FAMG_CLASS_PLACEHOLDER);
    check("partial -> uses_placeholder(REAL)=0",
          dm1_v22_famg_uses_placeholder(
              DM1_V22_FAMG_WALL_D3_CARVED) == 0);
    check("partial -> uses_placeholder(PLACEHOLDER)=1",
          dm1_v22_famg_uses_placeholder(
              DM1_V22_FAMG_FLOOR_PLAIN) == 1);
    check("partial -> installed=1",
          dm1_v22_famg_get_installed() == 1);
    check("partial -> is_finished_real=0",
          dm1_v22_famg_is_finished_real() == 0);
    check("partial -> is_synthetic_or_partial=1",
          dm1_v22_famg_is_synthetic_or_partial() == 1);
    int total = 0;
    int real = dm1_v22_famg_real_count(&total);
    check("partial -> real_count=1", real == 1);
    check("partial -> total=DM1_V22_FAMG_MATERIAL_COUNT",
          total == (int)DM1_V22_FAMG_MATERIAL_COUNT);

    /* ── Scenario 6: FINISHED_REAL gate (all 7 slots REAL) ───────── */
    printf("\n[ Scenario 6: FINISHED_REAL gate ]\n");
    /* Create every category directory so source_file resolution can
     * land on disk. */
    snprintf(mkdir_cmd, sizeof(mkdir_cmd),
             "mkdir -p '%s/wall_shapes' '%s/floor_shapes' "
             "'%s/creature_shapes' '%s/champion_portraits' '%s/door_shapes' "
             "'%s/field_shapes'",
             resolved_mdir, resolved_mdir, resolved_mdir,
             resolved_mdir, resolved_mdir, resolved_mdir);
    system(mkdir_cmd);

    /* Create all 7 source files on disk under their category dirs. */
    const char* files[DM1_V22_FAMG_MATERIAL_COUNT] = {
        "wall_d3_carved_hero_01.png",
        "floor_plain_hero_01.png",
        "floor_pit_hero_01.png",
        "creature_demon_hero_01.png",
        "champion_warrior_hero_01.png",
        "door_hero_01.png",
        "field_teleporter_hero_01.png"
    };
    for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        char fpath[1024];
        snprintf(fpath, sizeof(fpath), "%s/%s/%s",
                 resolved_mdir,
                 dm1_v22_famg_slot_category((DM1_V22_FamgSlot)i),
                 files[i]);
        write_png_header_file(fpath,
                              (i == DM1_V22_FAMG_DOOR_FRONT) ? 32U :
                              (i == DM1_V22_FAMG_CREATURE_DEMON ||
                               i == DM1_V22_FAMG_CHAMPION_WARRIOR) ? 48U : 64U,
                              (i == DM1_V22_FAMG_DOOR_FRONT) ? 48U :
                              (i == DM1_V22_FAMG_CREATURE_DEMON ||
                               i == DM1_V22_FAMG_CHAMPION_WARRIOR) ? 48U : 64U);
    }

    const char* finished_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-probe\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"wall_d3_carved_hero_01.png\","
        "\"width\":64,\"height\":64}"
        "],"
        "\"floor_shapes\":["
        "{\"id\":\"floor_plain_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"floor_plain_hero_01.png\",\"width\":64,\"height\":64},"
        "{\"id\":\"floor_pit_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"floor_pit_hero_01.png\",\"width\":64,\"height\":64}"
        "],"
        "\"creature_shapes\":["
        "{\"id\":\"creature_demon_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"creature_demon_hero_01.png\",\"width\":48,\"height\":48}"
        "],"
        "\"champion_portraits\":["
        "{\"id\":\"champion_warrior_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"champion_warrior_hero_01.png\","
        "\"width\":48,\"height\":48}"
        "],"
        "\"door_shapes\":["
        "{\"id\":\"door_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"door_hero_01.png\",\"width\":32,\"height\":48}"
        "],"
        "\"field_shapes\":["
        "{\"id\":\"field_teleporter_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"field_teleporter_hero_01.png\","
        "\"width\":64,\"height\":64}"
        "]}";
    check("wrote finished-real manifest",
          write_file(manifest_file, finished_content));
    check("finished-real -> FINISHED_REAL gate",
          dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_FINISHED_REAL);
    check("finished-real -> installed=1",
          dm1_v22_famg_get_installed() == 1);
    check("finished-real -> is_finished_real=1",
          dm1_v22_famg_is_finished_real() == 1);
    check("finished-real -> is_synthetic_or_partial=0",
          dm1_v22_famg_is_synthetic_or_partial() == 0);
    total = 0;
    real = dm1_v22_famg_real_count(&total);
    check("finished-real -> real_count=7", real == 7);
    check("finished-real -> total=7", total == 7);
    for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        check("finished-real -> all 7 slots REAL",
              dm1_v22_famg_classify_slot(
                  (DM1_V22_FamgSlot)i) ==
                  DM1_V22_FAMG_CLASS_REAL);
        check("finished-real -> all 7 slots uses_placeholder=0",
              dm1_v22_famg_uses_placeholder(
                  (DM1_V22_FamgSlot)i) == 0);
    }

    /* ── Scenario 6b: runtime screenshot receipt classification ───── */
    printf("\n[ Scenario 6b: runtime screenshot/material receipt gate ]\n");
    check("finished materials without receipt -> NO_RECEIPT",
          dm1_v22_famg_receipt_gate() ==
              DM1_V22_FAMG_RECEIPT_NO_RECEIPT);
    check("no receipt -> has_finished_real_receipt=0",
          dm1_v22_famg_has_finished_real_receipt() == 0);

    char receipt_dir[1024];
    snprintf(receipt_dir, sizeof(receipt_dir), "%s/receipts", resolved_mdir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", receipt_dir);
    system(mkdir_cmd);
    char receipt_file[1024];
    snprintf(receipt_file, sizeof(receipt_file),
             "%s/synthetic_frame.bmp", receipt_dir);
    write_file(receipt_file, "synthetic-runtime-bmp");
    check("wrote synthetic receipt manifest",
          write_all_real_manifest_with_receipt(
              manifest_file,
              "synthetic_test",
              "synthetic_frame.bmp",
              "sha256:synthetic",
              "FINISHED_REAL"));
    check("synthetic receipt -> SYNTHETIC_PLACEHOLDER",
          dm1_v22_famg_receipt_gate() ==
              DM1_V22_FAMG_RECEIPT_SYNTHETIC_PLACEHOLDER);
    check("synthetic receipt predicate true",
          dm1_v22_famg_has_synthetic_receipt() == 1);
    check("synthetic receipt still not final proof",
          dm1_v22_famg_has_finished_real_receipt() == 0);

    DM1_V22_FamgReceiptInfo receipt_info;
    check("receipt info available",
          dm1_v22_famg_get_receipt_info(&receipt_info) == 1);
    check("receipt info id stable",
          strcmp(receipt_info.id, dm1_v22_famg_receipt_manifest_id()) == 0);
    check("receipt info file exists",
          receipt_info.file_exists == 1);
    check("receipt info hash retained",
          strcmp(receipt_info.frame_hash, "sha256:synthetic") == 0);
    check("synthetic receipt text has no valid BMP header",
          receipt_info.bmp_header_valid == 0);

    check("wrote reviewed missing-file receipt manifest",
          write_all_real_manifest_with_receipt(
              manifest_file,
              "operator_reviewed",
              "missing_reviewed_frame.bmp",
              "sha256:reviewed",
              "FINISHED_REAL"));
    check("reviewed receipt missing file -> PARTIAL",
          dm1_v22_famg_receipt_gate() ==
              DM1_V22_FAMG_RECEIPT_PARTIAL);

    snprintf(receipt_file, sizeof(receipt_file),
             "%s/reviewed_frame.bmp", receipt_dir);
    write_file(receipt_file, "reviewed-runtime-bmp");
    check("wrote reviewed text receipt manifest",
          write_all_real_manifest_with_receipt(
              manifest_file,
              "operator_reviewed",
              "reviewed_frame.bmp",
              "sha256:reviewed",
              "FINISHED_REAL"));
    check("reviewed text receipt -> PARTIAL",
          dm1_v22_famg_receipt_gate() ==
              DM1_V22_FAMG_RECEIPT_PARTIAL);
    check("receipt info available for reviewed text",
          dm1_v22_famg_get_receipt_info(&receipt_info) == 1);
    check("reviewed text receipt invalid BMP header",
          receipt_info.bmp_header_valid == 0);

    write_bmp_header_file(receipt_file, 319U, 200U);
    check("reviewed mismatched BMP receipt -> PARTIAL",
          dm1_v22_famg_receipt_gate() ==
              DM1_V22_FAMG_RECEIPT_PARTIAL);
    check("receipt info available for mismatched BMP",
          dm1_v22_famg_get_receipt_info(&receipt_info) == 1);
    check("mismatched BMP dimensions retained",
          receipt_info.bmp_width == 319 && receipt_info.bmp_height == 200);

    write_bmp_header_file(receipt_file, 320U, 200U);
    check("reviewed matching BMP receipt ready",
          dm1_v22_famg_get_receipt_info(&receipt_info) == 1 &&
          receipt_info.bmp_header_valid == 1);
    check("wrote reviewed complete receipt manifest",
          write_all_real_manifest_with_receipt(
              manifest_file,
              "operator_reviewed",
              "reviewed_frame.bmp",
              "sha256:reviewed",
              "FINISHED_REAL"));
    check("reviewed receipt + finished materials -> FINISHED_REAL",
          dm1_v22_famg_receipt_gate() ==
              DM1_V22_FAMG_RECEIPT_FINISHED_REAL);
    check("finished receipt predicate true",
          dm1_v22_famg_has_finished_real_receipt() == 1);
    check("receipt gate FINISHED_REAL name",
          strcmp(dm1_v22_famg_receipt_gate_name(
              DM1_V22_FAMG_RECEIPT_FINISHED_REAL), "FINISHED_REAL") == 0);

    /* ── Scenario 7: PARTIAL via missing source_file ─────────────── */
    printf("\n[ Scenario 7: PARTIAL via missing source_file ]\n");
    /* Remove one of the real files. wall_d3_carved is in wall_shapes/.
     * The module still classifies it as PARTIAL (real metadata but the
     * file is gone), which routes through the SYNTHETIC_PLACEHOLDER gate. */
    unlink(real_file);
    check("manifest with missing source_file -> PARTIAL gate",
          dm1_v22_famg_gate() ==
              DM1_V22_FAMG_GATE_PARTIAL);
    check("missing source_file -> wall_d3_carved=PARTIAL",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_WALL_D3_CARVED) ==
              DM1_V22_FAMG_CLASS_PARTIAL);
    /* Recreate for subsequent scenarios */
    write_png_header_file(real_file, 64U, 64U);

    /* ── Scenario 8: bad PNG provenance -> PARTIAL ──────────────── */
    printf("\n[ Scenario 8: bad PNG provenance ]\n");
    write_file(real_file, "fake-png-bytes");
    check("text file with .png suffix -> wall_d3_carved=PARTIAL",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_WALL_D3_CARVED) ==
              DM1_V22_FAMG_CLASS_PARTIAL);
    DM1_V22_FamgSlotInfo info;
    check("text file with .png suffix -> png_header_valid=0",
          dm1_v22_famg_get_slot_info(
              DM1_V22_FAMG_WALL_D3_CARVED, &info) == 1 &&
          info.file_exists == 1 &&
          info.png_header_valid == 0);
    write_png_header_file(real_file, 32U, 64U);
    check("PNG IHDR dimension mismatch -> wall_d3_carved=PARTIAL",
          dm1_v22_famg_classify_slot(
              DM1_V22_FAMG_WALL_D3_CARVED) ==
              DM1_V22_FAMG_CLASS_PARTIAL);
    check("PNG IHDR dimension mismatch reports actual width",
          dm1_v22_famg_get_slot_info(
              DM1_V22_FAMG_WALL_D3_CARVED, &info) == 1 &&
          info.png_width == 32 &&
          info.png_height == 64 &&
          info.png_header_valid == 0);
    write_png_header_file(real_file, 64U, 64U);

    /* ── Scenario 9: garbage manifest -> NO_MANIFEST ────────────── */
    printf("\n[ Scenario 9: garbage manifest ]\n");
    write_file(manifest_file, "this is not json { [");
    check("garbage manifest -> NO_MANIFEST gate",
          dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_NO_MANIFEST);

    /* ── Scenario 10: per-slot invariants + names + evidence ─────── */
    printf("\n[ Scenario 10: invariants + names + evidence ]\n");
    check("DM1_V22_FAMG_MATERIAL_COUNT=7",
          DM1_V22_FAMG_MATERIAL_COUNT == 7);
    check("slot[0] = wall_d3_carved_hero_01",
          strcmp(dm1_v22_famg_slot_name(
              DM1_V22_FAMG_WALL_D3_CARVED),
              "wall_d3_carved_hero_01") == 0);
    check("slot[1] = floor_plain_hero_01",
          strcmp(dm1_v22_famg_slot_name(
              DM1_V22_FAMG_FLOOR_PLAIN),
              "floor_plain_hero_01") == 0);
    check("slot[2] = floor_pit_hero_01",
          strcmp(dm1_v22_famg_slot_name(
              DM1_V22_FAMG_FLOOR_PIT),
              "floor_pit_hero_01") == 0);
    check("slot[3] = creature_demon_hero_01",
          strcmp(dm1_v22_famg_slot_name(
              DM1_V22_FAMG_CREATURE_DEMON),
              "creature_demon_hero_01") == 0);
    check("slot[4] = champion_warrior_hero_01",
          strcmp(dm1_v22_famg_slot_name(
              DM1_V22_FAMG_CHAMPION_WARRIOR),
              "champion_warrior_hero_01") == 0);
    check("slot[5] = door_hero_01",
          strcmp(dm1_v22_famg_slot_name(
              DM1_V22_FAMG_DOOR_FRONT),
              "door_hero_01") == 0);
    check("slot[6] = field_teleporter_hero_01",
          strcmp(dm1_v22_famg_slot_name(
              DM1_V22_FAMG_TELEPORTER_FIELD),
              "field_teleporter_hero_01") == 0);
    check("out-of-range slot name -> UNKNOWN",
          strcmp(dm1_v22_famg_slot_name(
              (DM1_V22_FamgSlot)9999), "UNKNOWN") == 0);
    /* Verify the sibling SKIP-only gate's hero_01 ids are the same. */
    check("slot name format matches SKIP-only sibling hero_01 id pattern",
          strstr(dm1_v22_famg_slot_name(
              DM1_V22_FAMG_WALL_D3_CARVED), "_hero_01") != NULL);

    /* Class + gate names */
    check("class REAL name", strcmp(dm1_v22_famg_class_name(
        DM1_V22_FAMG_CLASS_REAL), "REAL") == 0);
    check("class PLACEHOLDER name", strcmp(dm1_v22_famg_class_name(
        DM1_V22_FAMG_CLASS_PLACEHOLDER), "PLACEHOLDER") == 0);
    check("class PARTIAL name", strcmp(dm1_v22_famg_class_name(
        DM1_V22_FAMG_CLASS_PARTIAL), "PARTIAL") == 0);
    check("class MISSING name", strcmp(dm1_v22_famg_class_name(
        DM1_V22_FAMG_CLASS_MISSING), "MISSING") == 0);
    check("gate NO_MANIFEST name", strcmp(dm1_v22_famg_gate_name(
        DM1_V22_FAMG_GATE_NO_MANIFEST), "NO_MANIFEST") == 0);
    check("gate SYNTHETIC_PLACEHOLDER name", strcmp(dm1_v22_famg_gate_name(
        DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER),
        "SYNTHETIC_PLACEHOLDER") == 0);
    check("gate PARTIAL name", strcmp(dm1_v22_famg_gate_name(
        DM1_V22_FAMG_GATE_PARTIAL), "PARTIAL") == 0);
    check("gate FINISHED_REAL name", strcmp(dm1_v22_famg_gate_name(
        DM1_V22_FAMG_GATE_FINISHED_REAL), "FINISHED_REAL") == 0);

    /* Source-evidence citation invariants */
    const char* ev = dm1_v22_famg_source_evidence();
    check("source_evidence cites DUNVIEW.C",
          ev != NULL && strstr(ev, "DUNVIEW.C") != NULL);
    check("source_evidence cites DUNGEON.C",
          ev != NULL && strstr(ev, "DUNGEON.C") != NULL);
    check("source_evidence cites PANEL.C",
          ev != NULL && strstr(ev, "PANEL.C") != NULL);
    check("source_evidence cites m11_v22_inplace_draw",
          ev != NULL && strstr(ev, "m11_v22_inplace_draw") != NULL);
    check("source_evidence cites modern_asset_manifest.json",
          ev != NULL && strstr(ev, "modern_asset_manifest.json") != NULL);
    check("source_evidence cites PNG IHDR provenance",
          ev != NULL && strstr(ev, "PNG IHDR") != NULL);
    check("source_evidence cites FIRESTAFF_GAP_LIST",
          ev != NULL && strstr(ev, "FIRESTAFF_GAP_LIST") != NULL);
    check("source_evidence cites Honest boundary",
          ev != NULL && strstr(ev, "Honest boundary") != NULL);
    check("source_evidence cites receipt id",
          ev != NULL &&
          strstr(ev, "dm1_v22_real_screenshot_material_receipt_01") != NULL);
    check("source_evidence cites receipt FINISHED_REAL boundary",
          ev != NULL && strstr(ev, "Receipt FINISHED_REAL") != NULL);

    /* ── Clean up ──────────────────────────────────────────────────── */
    system("rm -rf /tmp/scratch/dm1-famg-probe");
    system("rm -rf /tmp/scratch/dm1-famg-data");

    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
