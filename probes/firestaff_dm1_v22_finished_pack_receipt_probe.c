/*
 * firestaff_dm1_v22_finished_pack_receipt_probe.c
 *
 * DM1 V2.2 finished-pack reviewer-receipt gate headless CI probe.
 *
 * Headless probe for the operator-reviewed FINISHED_REAL receipt
 * module. No game data, no SDL rendering required. Builds synthetic
 * finish_receipt.json + modern_asset_manifest.json fixtures under
 * /tmp/scratch/ and verifies the skip-safe receipt path the gap-list
 * row calls out:
 *
 *   Scenario 1: unset path             -> NOT_INSTALLED gate
 *   Scenario 2: no receipt file        -> NOT_INSTALLED gate
 *   Scenario 3: receipt missing hash   -> MALFORMED gate
 *   Scenario 4: receipt + placeholder manifest
 *                                      -> MATERIAL_NOT_REAL gate
 *   Scenario 5: receipt + FINISHED_REAL manifest + slot list
 *               incomplete            -> MATCH_PARTIAL gate
 *   Scenario 6: receipt + FINISHED_REAL manifest + all slots
 *                                      -> MATCH_FINISHED_REAL gate
 *               + is_promoted=1
 *   Scenario 7: stale review after manifest regression
 *                                      -> MATERIAL_NOT_REAL gate
 *   Scenario 8: receipt_hash_matches() helper behaviour
 *   Scenario 9: deterministic FNV-1a fixture check
 *  10-13. Source-evidence citation invariants
 *
 * Synthetic fallback baseline (the default CI scenario when no
 * reviewer has shipped a real receipt): the receipt path is unset
 * and the gate reports NOT_INSTALLED, is_promoted=0 — that is the
 * honest state until an operator drops finish_receipt.json.
 *
 * Companion to:
 *   - include/dm1_v22_finished_pack_receipt_pc34.h
 *   - src/dm1v2/dm1_v22_finished_pack_receipt_pc34.c
 *   - sibling probe firestaff_dm1_v22_finished_art_material_gate_probe.c
 */

#include "dm1_v22_finished_pack_receipt_pc34.h"
#include "dm1_v22_finished_art_material_gate_pc34.h"
#include "fs_portable_compat.h"

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

/* ── Shared paths & fixtures ────────────────────────────────────── */

static const char* k_data    = "/tmp/scratch/dm1_fpr_probe_data/data/dm1";
static const char* k_modern  = "/tmp/scratch/dm1_fpr_probe_data/assets/dm1/modern";
static const char* k_manifest= "/tmp/scratch/dm1_fpr_probe_data/assets/dm1/modern/modern_asset_manifest.json";
static const char* k_receipt = "/tmp/scratch/dm1_fpr_probe_data/assets/dm1/modern/finish_receipt.json";

static const char* k_finished_real_manifest =
    "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-fpr-probe\","
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

static const char* k_placeholder_manifest =
    "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-fpr-probe\","
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

static void lay_down_finish_real_files(void) {
    char cmd[FSP_PATH_MAX * 4];
    snprintf(cmd, sizeof(cmd),
             "mkdir -p '%s/wall_shapes' '%s/floor_shapes' "
             "'%s/creature_shapes' '%s/champion_portraits' '%s/door_shapes' "
             "'%s/field_shapes'",
             k_modern, k_modern, k_modern, k_modern, k_modern, k_modern);
    system(cmd);
    const char* files[DM1_V22_FAMG_MATERIAL_COUNT] = {
        "wall_d3_carved_hero_01.png",
        "floor_plain_hero_01.png",
        "floor_pit_hero_01.png",
        "creature_demon_hero_01.png",
        "champion_warrior_hero_01.png",
        "door_hero_01.png",
        "field_teleporter_hero_01.png"
    };
    const unsigned widths[DM1_V22_FAMG_MATERIAL_COUNT] = {
        64U, 64U, 64U, 48U, 48U, 32U, 64U
    };
    const unsigned heights[DM1_V22_FAMG_MATERIAL_COUNT] = {
        64U, 64U, 64U, 48U, 48U, 48U, 64U
    };
    for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        char fpath[FSP_PATH_MAX];
        snprintf(fpath, sizeof(fpath), "%s/%s/%s",
                 k_modern,
                 dm1_v22_famg_slot_category((DM1_V22_FamgSlot)i),
                 files[i]);
        write_png_header_file(fpath, widths[i], heights[i]);
    }
}

static void build_receipt(char* out, size_t outSize,
                          const char* manifest_hash_hex,
                          int reviewed_count) {
    const char* ids[DM1_V22_FAMG_MATERIAL_COUNT] = {
        "wall_d3_carved_hero_01",
        "floor_plain_hero_01",
        "floor_pit_hero_01",
        "creature_demon_hero_01",
        "champion_warrior_hero_01",
        "door_hero_01",
        "field_teleporter_hero_01"
    };
    size_t off = 0U;
    int n;
    off += (size_t)snprintf(out + off, outSize - off,
        "{\"receiptVersion\":\"1.0.0\","
        "\"manifestPath\":\"<synthetic>\","
        "\"manifestHashFnv1a\":\"%s\","
        "\"reviewer\":\"firestaff-probe\","
        "\"reviewedAtUtc\":\"2026-06-29T07:50:00Z\","
        "\"gateTarget\":\"FINISHED_REAL\","
        "\"reviewedSlots\":[",
        manifest_hash_hex);
    for (n = 0; n < reviewed_count && n < (int)DM1_V22_FAMG_MATERIAL_COUNT; ++n) {
        off += (size_t)snprintf(out + off, outSize - off,
            "%s\"%s\"", (n > 0 ? "," : ""), ids[n]);
    }
    snprintf(out + off, outSize - off, "]}");
}

static void reset_scratch(void) {
    system("rm -rf /tmp/scratch/dm1_fpr_probe_data");
    /* Both the data dir (set as receipt/manifest root) and the
     * modern-asset dir (where manifest + receipt land) must exist
     * before write_file() can land fixtures. Otherwise write_file
     * silently fails on the missing parent dir. */
    system("mkdir -p /tmp/scratch/dm1_fpr_probe_data/data/dm1");
    system("mkdir -p /tmp/scratch/dm1_fpr_probe_data/assets/dm1/modern");
}

/* ── Probe body ────────────────────────────────────────────────── */

int main(void) {
    printf("=== DM1 V2.2 finished-pack receipt gate probe ===\n");

    /* ── Scenario 1: unset path ─────────────────────────────────── */
    printf("\n[ Scenario 1: unset path - skip-safe default ]\n");
    dm1_v22_fpr_set_receipt_path(NULL);
    check("unset path -> NOT_INSTALLED gate",
          dm1_v22_fpr_state() == DM1_V22_FPR_NOT_INSTALLED);
    check("unset path -> is_promoted=0",
          dm1_v22_fpr_is_promoted() == 0);
    check("unset path -> receipt_present=0",
          dm1_v22_fpr_receipt_present() == 0);
    check("unset path -> receipt_hash_matches=-1",
          dm1_v22_fpr_receipt_hash_matches() == -1);

    /* ── Scenario 2: path set, no receipt file ──────────────────── */
    printf("\n[ Scenario 2: path set, no receipt file ]\n");
    reset_scratch();
    dm1_v22_fpr_set_receipt_path(k_data);
    dm1_v22_fpr_reset_state();
    check("manifest path == expected",
          strcmp(dm1_v22_fpr_get_manifest_path(), k_manifest) == 0);
    check("receipt path == expected",
          strcmp(dm1_v22_fpr_get_receipt_path(), k_receipt) == 0);
    check("no receipt file -> NOT_INSTALLED gate",
          dm1_v22_fpr_state() == DM1_V22_FPR_NOT_INSTALLED);
    check("no receipt file -> is_promoted=0",
          dm1_v22_fpr_is_promoted() == 0);

    /* ── Scenario 3: receipt missing manifestHashFnv1a ──────────── */
    printf("\n[ Scenario 3: receipt missing manifestHashFnv1a ]\n");
    write_file(k_receipt,
               "{\"receiptVersion\":\"1.0.0\","
               "\"reviewedSlots\":[\"wall_d3_carved_hero_01\"]}");
    dm1_v22_fpr_reset_state();
    check("receipt missing hash -> MALFORMED gate",
          dm1_v22_fpr_state() == DM1_V22_FPR_MALFORMED);
    check("receipt missing hash -> is_promoted=0",
          dm1_v22_fpr_is_promoted() == 0);

    /* ── Scenario 4: receipt + placeholder manifest ─────────────── */
    printf("\n[ Scenario 4: receipt + placeholder manifest ]\n");
    reset_scratch();
    write_file(k_manifest, k_placeholder_manifest);
    uint32_t h = dm1_v22_fpr_fnv1a_file(k_manifest);
    char hex[16];
    snprintf(hex, sizeof(hex), "%08x", (unsigned)h);
    char receipt[2048];
    build_receipt(receipt, sizeof(receipt), hex,
                  (int)DM1_V22_FAMG_MATERIAL_COUNT);
    write_file(k_receipt, receipt);
    dm1_v22_fpr_set_receipt_path(k_data);
    dm1_v22_fpr_reset_state();

    check("placeholder manifest -> receipt_hash_matches=1",
          dm1_v22_fpr_receipt_hash_matches() == 1);
    check("placeholder manifest -> material gate != FINISHED_REAL",
          dm1_v22_famg_is_finished_real() == 0);
    check("placeholder manifest + receipt -> MATERIAL_NOT_REAL gate",
          dm1_v22_fpr_state() == DM1_V22_FPR_MATERIAL_NOT_REAL);
    check("placeholder manifest + receipt -> is_promoted=0",
          dm1_v22_fpr_is_promoted() == 0);

    /* ── Scenario 5: receipt + FINISHED_REAL + slot list incomplete */
    printf("\n[ Scenario 5: receipt + FINISHED_REAL + slot list incomplete ]\n");
    reset_scratch();
    write_file(k_manifest, k_finished_real_manifest);
    lay_down_finish_real_files();
    h = dm1_v22_fpr_fnv1a_file(k_manifest);
    snprintf(hex, sizeof(hex), "%08x", (unsigned)h);
    build_receipt(receipt, sizeof(receipt), hex, 3);
    write_file(k_receipt, receipt);
    dm1_v22_fpr_set_receipt_path(k_data);
    dm1_v22_fpr_reset_state();

    check("FINISHED_REAL manifest -> material gate=FINISHED_REAL",
          dm1_v22_famg_is_finished_real() == 1);
    check("3-of-7 reviewed -> MATCH_PARTIAL gate",
          dm1_v22_fpr_state() == DM1_V22_FPR_MATCH_PARTIAL);
    check("3-of-7 reviewed -> is_promoted=0",
          dm1_v22_fpr_is_promoted() == 0);
    {
        int required = 0;
        int reviewed = dm1_v22_fpr_receipt_slot_count(&required);
        check("3-of-7 reviewed -> required=7", required == 7);
        check("3-of-7 reviewed -> reviewed=3", reviewed == 3);
    }

    /* ── Scenario 6: receipt + FINISHED_REAL + all slots reviewed ─ */
    printf("\n[ Scenario 6: receipt + FINISHED_REAL + all slots reviewed ]\n");
    build_receipt(receipt, sizeof(receipt), hex,
                  (int)DM1_V22_FAMG_MATERIAL_COUNT);
    write_file(k_receipt, receipt);
    dm1_v22_fpr_reset_state();

    check("full review + FINISHED_REAL -> MATCH_FINISHED_REAL gate",
          dm1_v22_fpr_state() == DM1_V22_FPR_MATCH_FINISHED_REAL);
    check("full review + FINISHED_REAL -> is_promoted=1",
          dm1_v22_fpr_is_promoted() == 1);
    {
        int required = 0;
        int reviewed = dm1_v22_fpr_receipt_slot_count(&required);
        check("full review -> required=7", required == 7);
        check("full review -> reviewed=7", reviewed == 7);
        check("full review -> stale_review_count=0",
              dm1_v22_fpr_receipt_stale_review_count() == 0);
    }

    /* ── Scenario 7: stale review after manifest regression ────── */
    printf("\n[ Scenario 7: stale review after manifest regression ]\n");
    {
        char door_path[FSP_PATH_MAX];
        snprintf(door_path, sizeof(door_path),
                 "%s/door_shapes/door_hero_01.png", k_modern);
        unlink(door_path);
    }
    dm1_v22_fpr_reset_state();

    check("removed door file -> material gate != FINISHED_REAL",
          dm1_v22_famg_is_finished_real() == 0);
    check("removed door file + receipt -> MATERIAL_NOT_REAL gate",
          dm1_v22_fpr_state() == DM1_V22_FPR_MATERIAL_NOT_REAL);
    check("removed door file -> is_promoted=0",
          dm1_v22_fpr_is_promoted() == 0);

    /* ── Scenario 8: receipt_hash_matches() returns correctly ──── */
    printf("\n[ Scenario 8: receipt_hash_matches helper ]\n");
    reset_scratch();
    dm1_v22_fpr_set_receipt_path(k_data);
    dm1_v22_fpr_reset_state();
    check("absent receipt -> hash_matches=-1",
          dm1_v22_fpr_receipt_hash_matches() == -1);
    write_file(k_receipt, "definitely not json");
    dm1_v22_fpr_reset_state();
    check("malformed receipt -> hash_matches=0",
          dm1_v22_fpr_receipt_hash_matches() == 0);
    /* Re-write the manifest so the helper can compare. */
    write_file(k_manifest, k_finished_real_manifest);
    build_receipt(receipt, sizeof(receipt), "deadbeef",
                  (int)DM1_V22_FAMG_MATERIAL_COUNT);
    write_file(k_receipt, receipt);
    dm1_v22_fpr_reset_state();
    check("wrong manifestHashFnv1a -> hash_matches=0",
          dm1_v22_fpr_receipt_hash_matches() == 0);

    /* ── Scenario 9: FNV-1a fixture invariant ──────────────────── */
    printf("\n[ Scenario 9: FNV-1a fixture invariant ]\n");
    check("FNV-1a(\"foobar\") == 0xbf9cf968",
          dm1_v22_fpr_fnv1a_buf("foobar", 6) == 0xbf9cf968u);
    check("FNV-1a(\"\") == 0x811c9dc5 (offset basis)",
          dm1_v22_fpr_fnv1a_buf("", 0U) == 2166136261u);

    /* ── Scenario 10: state name invariants ────────────────────── */
    printf("\n[ Scenario 10: state name invariants ]\n");
    check("name NOT_INSTALLED",
          strcmp(dm1_v22_fpr_state_name(DM1_V22_FPR_NOT_INSTALLED),
                 "NOT_INSTALLED") == 0);
    check("name MALFORMED",
          strcmp(dm1_v22_fpr_state_name(DM1_V22_FPR_MALFORMED),
                 "MALFORMED") == 0);
    check("name STALE",
          strcmp(dm1_v22_fpr_state_name(DM1_V22_FPR_STALE),
                 "STALE") == 0);
    check("name MATERIAL_NOT_REAL",
          strcmp(dm1_v22_fpr_state_name(DM1_V22_FPR_MATERIAL_NOT_REAL),
                 "MATERIAL_NOT_REAL") == 0);
    check("name MATCH_PARTIAL",
          strcmp(dm1_v22_fpr_state_name(DM1_V22_FPR_MATCH_PARTIAL),
                 "MATCH_PARTIAL") == 0);
    check("name MATCH_FINISHED_REAL",
          strcmp(dm1_v22_fpr_state_name(DM1_V22_FPR_MATCH_FINISHED_REAL),
                 "MATCH_FINISHED_REAL") == 0);
    check("out-of-range state name -> INVALID",
          strcmp(dm1_v22_fpr_state_name((DM1_V22_FprState)9999),
                 "INVALID") == 0);

    /* ── Scenario 11: source-evidence invariant ─────────────────── */
    printf("\n[ Scenario 11: source-evidence citations ]\n");
    {
        const char* ev = dm1_v22_fpr_source_evidence();
        check("evidence cites DUNVIEW.C",
              ev != NULL && strstr(ev, "DUNVIEW.C") != NULL);
        check("evidence cites DUNGEON.C",
              ev != NULL && strstr(ev, "DUNGEON.C") != NULL);
        check("evidence cites sibling material header",
              ev != NULL && strstr(ev,
                  "dm1_v22_finished_art_material_gate_pc34.h") != NULL);
        check("evidence names finish_receipt.json",
              ev != NULL && strstr(ev, "finish_receipt.json") != NULL);
        check("evidence names FNV-1a",
              ev != NULL && strstr(ev, "FNV-1a") != NULL);
        check("evidence names manifestHashFnv1a",
              ev != NULL && strstr(ev, "manifestHashFnv1a") != NULL);
        check("evidence names NOT_INSTALLED",
              ev != NULL && strstr(ev, "NOT_INSTALLED") != NULL);
        check("evidence names FINISHED_REAL",
              ev != NULL && strstr(ev, "FINISHED_REAL") != NULL);
        check("evidence calls out Honest boundary / PKI",
              ev != NULL && (strstr(ev, "Honest boundary") != NULL ||
                             strstr(ev, "PKI") != NULL));
    }

    /* ── Scenario 12: NULL safety ──────────────────────────────── */
    printf("\n[ Scenario 12: NULL safety ]\n");
    dm1_v22_fpr_set_receipt_path(NULL);
    check("FNV-1a NULL buf -> 0",
          dm1_v22_fpr_fnv1a_buf(NULL, 0U) == 0U);
    check("FNV-1a NULL path -> 0",
          dm1_v22_fpr_fnv1a_file(NULL) == 0U);

    /* ── Scenario 13: reset behaviour ──────────────────────────── */
    printf("\n[ Scenario 13: reset behaviour ]\n");
    reset_scratch();
    dm1_v22_fpr_set_receipt_path(k_data);
    dm1_v22_fpr_reset_state();
    check("reset with no receipt -> NOT_INSTALLED",
          dm1_v22_fpr_state() == DM1_V22_FPR_NOT_INSTALLED);
    /* Subsequent calls return the cached state without re-reading disk. */
    check("subsequent calls -> same state (cached)",
          dm1_v22_fpr_state() == DM1_V22_FPR_NOT_INSTALLED);
    /* Drop receipt on disk without resetting — state should be stale. */
    write_file(k_receipt,
               "{\"manifestHashFnv1a\":\"deadbeef\","
               "\"reviewedSlots\":["
               "\"wall_d3_carved_hero_01\",\"floor_plain_hero_01\","
               "\"floor_pit_hero_01\",\"creature_demon_hero_01\","
               "\"champion_warrior_hero_01\",\"door_hero_01\","
               "\"field_teleporter_hero_01\"]}");
    check("state cached -> still NOT_INSTALLED after file drop",
          dm1_v22_fpr_state() == DM1_V22_FPR_NOT_INSTALLED);
    dm1_v22_fpr_reset_state();
    /* The receipt has manifestHashFnv1a + reviewedSlots and parses;
     * with no manifest file on disk the FNV-1a comparison falls
     * through to STALE. (MALFORMED would only happen if the receipt
     * itself failed to parse.) */
    check("reset -> STALE (hash mismatch, manifest missing)",
          dm1_v22_fpr_state() == DM1_V22_FPR_STALE);

    /* ── Cleanup ────────────────────────────────────────────────── */
    system("rm -rf /tmp/scratch/dm1_fpr_probe_data");

    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
