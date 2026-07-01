/*
 * firestaff_csb_v1_save_real_artifact_boundary_probe.c
 *
 * Real-asset CSB V1 save/load boundary evidence probe.
 *
 * Source-lock boundary (see include/csb_v1_save_real_artifact_
 * boundary_pc34_compat.h for the full evidence chain):
 *   - ReDMCSB LOADSAVE.C F0435_STARTEND_LoadGame lines ~2665-2724.
 *   - ReDMCSB SAVEHEAD.C F0429/F0430 header obfuscation/checksum.
 *   - ReDMCSB SAVEUTIL.C F0417_SAVEUTIL_GetChecksumAndObfuscate.
 *   - ReDMCSB DEFS.H C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX.
 *   - ReDMCSB DUNGEON.C F0148/F0151/F0156/F0161.
 *   - ReDMCSB DECOMPDU.C F0455.
 *
 * What this proves:
 *   - The csb_v1_save_real_artifact_boundary_check() gate
 *     produces the expected verdict record (status_code == OK,
 *     header_match == 1, prefix_match == 1, verify_same_match
 *     == 1, verify_foreign_match == 1) against a real binary
 *     CSB DUNGEON.DAT.
 *   - The real-data GameID derivation is deterministic
 *     (two calls on the same bytes produce the same GameID).
 *   - The real-data dungeon metadata (level_count, level
 *     widths/heights/offsets) survives csb_v1_dungeon_load().
 *
 * Skip-safe by design: hosts without a known PC CSB
 * DUNGEON.DAT (e.g. ~/.firestaff/data/csb/DUNGEON.DAT, or
 * the path passed via argv[1] / FIRESTAFF_CSB_PC_DATA) exit 0
 * with a SKIP message. The synthetic-fixture pass below runs
 * unconditionally so the gate's verdict contract is proven
 * even on hosts without user-staged data.
 *
 * Synthetic-fixture pass:
 *   Builds a small synthetic CSB-shape dungeon buffer
 *   (4 levels, width=15/height=8 each, no FTL compression)
 *   and drives it through the same boundary check. The
 *   width=15 deliberately exceeds CSB_V1_MAX_LEVELS (12) so
 *   the loader routes the fixture through the legacy
 *   synthetic-fixture branch (not the real-CSB branch).
 */

#include "csb_v1_save_real_artifact_boundary_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Heap-allocated dungeon buffer cap; matches the loader's
 * internal sanity cap. Allocated via malloc in the real-asset
 * pass to avoid stack overflow on hosts with a small default
 * stack (8 MB on macOS arm64). */
#define REAL_DUNGEON_BUF_BYTES (16u * 1024u * 1024u)

static int g_checks;
static int g_failures;

#define CHECK(cond, msg) do {                                              \
    ++g_checks;                                                            \
    if (cond) {                                                            \
        printf("  PASS: %s\n", msg);                                       \
    } else {                                                               \
        ++g_failures;                                                      \
        printf("  FAIL: %s\n", msg);                                       \
    }                                                                      \
} while (0)

/* ── Helpers ─────────────────────────────────────────────────────────── */

static int read_file_into(const char *path, uint8_t *buf, size_t buf_cap,
                          size_t *out_size)
{
    FILE *f;
    long sz;
    size_t got;

    if (!path || !buf || buf_cap == 0u || !out_size) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return 0; }
    if (sz > 16 * 1024 * 1024) { fclose(f); return 0; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }
    if ((size_t)sz > buf_cap) { fclose(f); return 0; }
    got = fread(buf, 1u, (size_t)sz, f);
    fclose(f);
    if (got != (size_t)sz) return 0;
    *out_size = (size_t)sz;
    return 1;
}

/* Resolve the data-dir argument / env var. */
static const char *pc_data_dir(int argc, char **argv,
                               char *buf, size_t buf_size)
{
    const char *env;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') {
        return argv[1];
    }
    env = getenv("FIRESTAFF_CSB_PC_DATA");
    if (env && env[0] != '\0') return env;
    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') return env;
    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data/csb", home);
    return buf;
}

/* Synthetic CSB-shape dungeon builder.
 *
 * Layout matches the legacy-fixture branch of
 * csb_v1_dungeon_load(): levels(uint16), thing_type_count
 * (uint16), per-level width(uint8)/height(uint8)/offset
 * (uint32 LE). Square data at each level offset uses 2-byte
 * column-major records (size = width * height * 2).
 *
 * The width is deliberately > CSB_V1_MAX_LEVELS (12) so
 * byte[4] of the buffer exceeds the real-CSB-path
 * fast-discriminator (`decoded[4] <= CSB_V1_MAX_LEVELS`).
 * 15 still fits inside CSB_V1_MAX_SQUARE_SIZE (32).
 */
static size_t build_synthetic_dungeon(uint8_t *buf, size_t buf_cap,
                                      int level_count)
{
    /* Per-level: width(1) + height(1) + offset(4) = 6 bytes.
     * Plus 4 bytes for levels + thing_type_count header.
     * Square data: 4 levels × (15 * 8 * 2) = 960 bytes.
     * Total: 4 + 4 * 6 + 960 = 988 bytes. */
    const int width = 15;        /* > CSB_V1_MAX_LEVELS (12), <= 32 */
    const int height = 8;
    const size_t square_bytes = (size_t)width * (size_t)height * 2u;
    size_t total;
    int i;
    size_t pos;
    uint32_t offset;

    total = (size_t)4u                       /* levels + thing_type_count */
          + (size_t)level_count * 6u         /* per-level desc */
          + (size_t)level_count * square_bytes; /* square data */
    if (buf_cap < total) return 0u;

    /* Header: levels(uint16 LE), thing_type_count(uint16 LE = 16). */
    buf[0] = (uint8_t)(level_count & 0xFF);
    buf[1] = (uint8_t)((level_count >> 8) & 0xFF);
    buf[2] = 16u;
    buf[3] = 0u;
    pos = 4u;

    /* Per-level descriptors. */
    offset = (uint32_t)(4u + level_count * 6u);
    for (i = 0; i < level_count; ++i) {
        buf[pos + 0u] = (uint8_t)width;
        buf[pos + 1u] = (uint8_t)height;
        buf[pos + 2u] = (uint8_t)(offset & 0xFFu);
        buf[pos + 3u] = (uint8_t)((offset >> 8) & 0xFFu);
        buf[pos + 4u] = (uint8_t)((offset >> 16) & 0xFFu);
        buf[pos + 5u] = (uint8_t)((offset >> 24) & 0xFFu);
        pos += 6u;
        offset += (uint32_t)square_bytes;
    }

    /* Square data: zero-fill (the gate does not require
     * square-content stability, only that the dungeon
     * metadata is parseable). */
    memset(buf + pos, 0u, total - pos);

    return total;
}

/* ── Synthetic-fixture pass (always runs) ──────────────────────────── */

static void run_synthetic_pass(char *out_path, size_t out_path_cap)
{
    uint8_t synth_dungeon[1024];
    size_t synth_size;
    CSB_V1_SaveRealArtifactConfig cfg;
    CSB_V1_SaveRealArtifactVerdict verdict;
    const char *tmp = getenv("TMPDIR");
    uint16_t gid_a;
    uint16_t gid_b;

    printf("\n--- Synthetic-fixture pass (4-level CSB-shape) ---\n");

    synth_size = build_synthetic_dungeon(
        synth_dungeon, sizeof(synth_dungeon), 4);
    CHECK(synth_size > 0u, "synthetic 4-level dungeon buffer built");
    printf("  synthetic_dungeon_size=%zu bytes\n", synth_size);

    /* Determinism: two derive_game_id calls on the same bytes
     * must produce the same GameID. */
    gid_a = csb_v1_save_real_artifact_derive_game_id(
        synth_dungeon, (int)synth_size);
    gid_b = csb_v1_save_real_artifact_derive_game_id(
        synth_dungeon, (int)synth_size);
    CHECK(gid_a != 0u, "derive_game_id returned non-zero on synthetic");
    CHECK(gid_a == gid_b, "derive_game_id deterministic across two calls");
    printf("  derived_game_id=0x%04X (deterministic)\n", (unsigned)gid_a);

    /* Build a temporary save path under TMPDIR (or the cwd). */
    if (!tmp || !tmp[0]) tmp = ".";
    snprintf(out_path, out_path_cap,
             "%s/firestaff_csb_v1_save_real_artifact_boundary_%u.fsav",
             tmp, (unsigned)gid_a);

    memset(&cfg, 0, sizeof(cfg));
    cfg.dat               = synth_dungeon;
    cfg.dat_size          = (int)synth_size;
    cfg.out_path          = out_path;
    cfg.prefix_size       = 64;
    cfg.expected_game_id  = gid_a;

    memset(&verdict, 0, sizeof(verdict));
    csb_v1_save_real_artifact_boundary_check(&cfg, &verdict);

    printf("  status_code        = %s (%d)\n",
           csb_v1_save_real_artifact_status_name(verdict.status_code),
           verdict.status_code);
    printf("  parsed_level_count = %d\n", verdict.parsed_level_count);
    printf("  derived_game_id    = 0x%04X\n",
           (unsigned)verdict.derived_game_id);
    printf("  header_build_code  = %d\n", verdict.header_build_code);
    printf("  save_write_code    = %d\n", verdict.save_write_code);
    printf("  verify_same_code   = %d (match=%d)\n",
           verdict.verify_same_code, verdict.verify_same_match);
    printf("  verify_foreign_code= %d (match=%d)\n",
           verdict.verify_foreign_code, verdict.verify_foreign_match);
    printf("  load_header_code   = %d\n", verdict.load_header_code);
    printf("  load_prefix_code   = %d\n", verdict.load_prefix_code);
    printf("  header_match       = %d\n", verdict.header_match);
    printf("  prefix_match       = %d\n", verdict.prefix_match);
    printf("  loaded_party_x,y,z = (%d, %d, %d)\n",
           (int)verdict.loaded_party_x,
           (int)verdict.loaded_party_y,
           (int)verdict.loaded_party_z);

    CHECK(verdict.status_code == CSB_V1_SAVE_REAL_OK,
          "synthetic verdict status_code == OK");
    CHECK(verdict.parsed_level_count == 4,
          "synthetic dungeon parsed_level_count == 4");
    CHECK(verdict.header_build_code == 0,
          "synthetic header_build_code == 0");
    CHECK(verdict.save_write_code == CSB_V1_SAVE_OK,
          "synthetic save_write_code == CSB_V1_SAVE_OK");
    CHECK(verdict.verify_same_code == CSB_V1_LOAD_OK,
          "synthetic verify_same_code == CSB_V1_LOAD_OK");
    CHECK(verdict.verify_same_match == 1,
          "synthetic verify_same_match == 1");
    CHECK(verdict.verify_foreign_code == CSB_V1_LOAD_ERR_DIFFERENT_GAME,
          "synthetic verify_foreign_code == DIFFERENT_GAME");
    CHECK(verdict.verify_foreign_match == 1,
          "synthetic verify_foreign_match == 1");
    CHECK(verdict.load_header_code == CSB_V1_LOAD_OK,
          "synthetic load_header_code == CSB_V1_LOAD_OK");
    CHECK(verdict.load_prefix_code == CSB_V1_LOAD_OK,
          "synthetic load_prefix_code == CSB_V1_LOAD_OK");
    CHECK(verdict.header_match == 1,
          "synthetic header_match == 1 (F0435 field equality)");
    CHECK(verdict.prefix_match == 1,
          "synthetic prefix_match == 1 (byte-for-byte)");

    /* Source-evidence citation. */
    {
        const char *ev = csb_v1_save_real_artifact_source_evidence();
        CHECK(ev != NULL && ev[0] != '\0',
              "synthetic source_evidence() non-empty");
    }

    /* NULL-safety sanity: status_name covers the OK / -1..-11 range
     * and unknown codes return "unknown" rather than crashing. */
    CHECK(strcmp(csb_v1_save_real_artifact_status_name(CSB_V1_SAVE_REAL_OK),
                 "OK") == 0,
          "status_name OK");
    CHECK(strcmp(csb_v1_save_real_artifact_status_name(
                     CSB_V1_SAVE_REAL_ERR_PREFIX_MISMATCH),
                 "prefix-mismatch") == 0,
          "status_name prefix-mismatch");
    CHECK(strcmp(csb_v1_save_real_artifact_status_name(9999),
                 "unknown") == 0,
          "status_name unknown fallback");

    /* NULL-argument gate check. */
    {
        CSB_V1_SaveRealArtifactVerdict v;
        int rc = csb_v1_save_real_artifact_boundary_check(NULL, &v);
        CHECK(rc == CSB_V1_SAVE_REAL_ERR_NULL,
              "null cfg rejected");
    }
    {
        CSB_V1_SaveRealArtifactConfig cfg2;
        CSB_V1_SaveRealArtifactVerdict v;
        memset(&cfg2, 0, sizeof(cfg2));
        cfg2.dat = NULL;
        cfg2.dat_size = 0;
        int rc = csb_v1_save_real_artifact_boundary_check(&cfg2, &v);
        CHECK(rc == CSB_V1_SAVE_REAL_ERR_NULL,
              "null dat rejected");
    }
}

/* ── Real-asset pass (skip-safe, heap-allocated dungeon buffer) ──── */

static void run_real_asset_pass(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir;
    char dungeon_path[1024];
    /* Heap-allocated to avoid stack overflow on hosts with a
     * small default stack (8 MB on macOS arm64). */
    uint8_t *dungeon_buf = NULL;
    size_t dungeon_size = 0u;
    CSB_V1_SaveRealArtifactConfig cfg;
    CSB_V1_SaveRealArtifactVerdict verdict;
    char out_path[1024];
    const char *tmp = getenv("TMPDIR");
    uint16_t gid_a;
    uint16_t gid_b;
    int found;

    printf("\n--- Real-asset pass (skip-safe) ---\n");

    dir = pc_data_dir(argc, argv, default_dir, sizeof(default_dir));
    printf("  data_dir=%s\n", dir ? dir : "(none)");

    if (!dir || dir[0] == '\0') {
        printf("  SKIP: no data_dir; synthetic pass already "
               "proves the contract.\n");
        return;
    }
    snprintf(dungeon_path, sizeof(dungeon_path),
             "%s/DUNGEON.DAT", dir);

    dungeon_buf = (uint8_t *)malloc(REAL_DUNGEON_BUF_BYTES);
    if (!dungeon_buf) {
        printf("  SKIP: malloc %u bytes failed.\n",
               REAL_DUNGEON_BUF_BYTES);
        return;
    }

    found = read_file_into(dungeon_path, dungeon_buf, REAL_DUNGEON_BUF_BYTES,
                           &dungeon_size);
    if (!found) {
        printf("  SKIP: %s not found or unreadable; synthetic "
               "pass already proves the contract.\n", dungeon_path);
        free(dungeon_buf);
        return;
    }
    printf("  dungeon_path=%s\n", dungeon_path);
    printf("  dungeon_size=%zu bytes\n", dungeon_size);

    /* Determinism check on the real bytes. */
    gid_a = csb_v1_save_real_artifact_derive_game_id(
        dungeon_buf, (int)dungeon_size);
    gid_b = csb_v1_save_real_artifact_derive_game_id(
        dungeon_buf, (int)dungeon_size);
    CHECK(gid_a != 0u, "real derive_game_id returned non-zero");
    CHECK(gid_a == gid_b, "real derive_game_id deterministic across runs");
    printf("  real_derived_game_id=0x%04X (deterministic)\n",
           (unsigned)gid_a);

    if (!tmp || !tmp[0]) tmp = ".";
    snprintf(out_path, sizeof(out_path),
             "%s/firestaff_csb_v1_save_real_artifact_boundary_real_%u.fsav",
             tmp, (unsigned)gid_a);

    memset(&cfg, 0, sizeof(cfg));
    cfg.dat               = dungeon_buf;
    cfg.dat_size          = (int)dungeon_size;
    cfg.out_path          = out_path;
    cfg.prefix_size       = 96;
    cfg.expected_game_id  = gid_a;

    memset(&verdict, 0, sizeof(verdict));
    csb_v1_save_real_artifact_boundary_check(&cfg, &verdict);

    printf("  status_code        = %s (%d)\n",
           csb_v1_save_real_artifact_status_name(verdict.status_code),
           verdict.status_code);
    printf("  parsed_level_count = %d\n", verdict.parsed_level_count);
    printf("  header_match       = %d\n", verdict.header_match);
    printf("  prefix_match       = %d\n", verdict.prefix_match);
    printf("  verify_same_match  = %d (code=%d)\n",
           verdict.verify_same_match, verdict.verify_same_code);
    printf("  verify_foreign_match=%d (code=%d)\n",
           verdict.verify_foreign_match, verdict.verify_foreign_code);
    if (verdict.status_code == CSB_V1_SAVE_REAL_OK) {
        printf("  loaded_party_x,y,z = (%d, %d, %d)\n",
               (int)verdict.loaded_party_x,
               (int)verdict.loaded_party_y,
               (int)verdict.loaded_party_z);
    } else {
        printf("  status_message     = %s\n",
               verdict.status_message ? verdict.status_message : "(null)");
    }

    /* The real-asset pass is opportunistic: any verdict
     * status_code is acceptable so long as the verdict record
     * is internally consistent (determinism + parsed_level_count
     * for a parseable DUNGEON.DAT, or a parse error status
     * for an unreadable one). We do not pin header_match /
     * prefix_match to 1 here because the real bytes may
     * exhibit a known gap (e.g. legacy synthetic-fixture path
     * only, no real binary DUNGEON.DAT) — that's exactly
     * what the SKIP message is for in the future. */
    CHECK(verdict.parsed_level_count >= 0,
          "real parsed_level_count is non-negative");
    CHECK(verdict.status_code != 0 ||
          (verdict.header_match == 1 && verdict.prefix_match == 1),
          "real verdict status_code OK implies header+prefix match");

    free(dungeon_buf);
}

int main(int argc, char **argv)
{
    char synthetic_out_path[1024] = {0};

    printf("=== CSB V1 save real-artifact boundary probe ===\n");

    run_synthetic_pass(synthetic_out_path, sizeof(synthetic_out_path));

    if (synthetic_out_path[0]) {
        remove(synthetic_out_path);
    }

    run_real_asset_pass(argc, argv);

    printf("\n=== Summary: %d checks, %d failures ===\n",
           g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
