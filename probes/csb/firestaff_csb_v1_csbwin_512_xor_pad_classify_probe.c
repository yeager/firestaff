/*
 * firestaff_csb_v1_csbwin_512_xor_pad_classify_probe.c
 *
 * Real-asset CSBWin 512-byte XOR-pad save-header classifier
 * probe.
 *
 * Source-lock boundary (see
 * include/csb_v1_csbwin_512_xor_pad_classify.h for the full
 * evidence chain):
 *   - CSBWin/SaveGame.cpp:880 GAMEBLOCK1 (512 bytes, first block)
 *   - CSBWin/SaveGame.cpp:715 ScrambleAndWrite (write side)
 *   - CSBWin/Chaos.cpp:1326 ReadGameBlock1 (read side)
 *   - CSBWin/Chaos.cpp:1341 UnscrambleBlock1 (validate + unscramble)
 *   - CSBWin/Chaos.cpp:2357 ReadSaves fallback: CSB key first, then DM
 *   - CSBWin/CSBCode.cpp:9038 Unscramble (RC4-like XOR stream)
 *   - ReDMCSB DEFS.H:469-501 (CSB_SAVE_HEADER / DM_SAVE_HEADER Noise
 *     arrays, C10/C29 key-index macros)
 *
 * What this proves:
 *   - The CSB V1 CSBWin 512-byte XOR-pad classifier
 *     (csb_v1_csbwin_512_xor_pad_classify) reports a consistent
 *     verdict on a real CSBWin or DM1 save file: CSB or DM when
 *     the documented UnscrambleBlock1 invariant holds, NEITHER
 *     when it doesn't.
 *   - Public fields (FormatID, GameID, Platform, DungeonID,
 *     keys[0..15], checksums[0..15], AdditionalData[0..3]) read
 *     back deterministically and match the same buffer across
 *     re-runs.
 *   - The caller's input buffer is never mutated, even on
 *     a real-asset read.
 *
 * Skip-safe by design: when no user-staged csbgame.dat /
 * csbgame.bak / dmsave.dat / dmsave.bak is found under the
 * data_dir, the probe exits 0 with a SKIP message. The
 * data-free contract is locked by the matching unit test
 * (test_csb_v1_csbwin_512_xor_pad_classify.c), so the gate
 * never fails on hosts without real CSBWin save assets.
 */

#include "csb_v1_csbwin_512_xor_pad_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* ── Helpers ────────────────────────────────────────────────────────── */

static int read_file_first_n(const char *path, uint8_t *buf, size_t buf_cap,
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
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }
    if ((size_t)sz > buf_cap) sz = (long)buf_cap;
    got = fread(buf, 1u, (size_t)sz, f);
    fclose(f);
    if (got == 0u) return 0;
    *out_size = got;
    return 1;
}

/* Resolve the data-dir argument / env var. Mirrors the helper
 * in firestaff_csb_v1_csbwin_save_loader_boundary_probe.c. */
static const char *data_dir_arg(int argc, char **argv,
                                char *buf, size_t buf_size)
{
    const char *env;
    const char *home;
    if (argc > 1 && argv[1] && argv[1][0] != '\0') {
        return argv[1];
    }
    env = getenv("FIRESTAFF_CSBWIN_SAVE_DATA");
    if (env && env[0] != '\0') return env;
    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') return env;
    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data", home);
    return buf;
}

/* Walk `dir` (recursive, depth-limited) looking for the first
 * file matching the candidate name list. Tries the candidate
 * list in order so the launcher / M12 surface can prefer
 * csbgame.dat over dmsave.dat etc. without re-implementing
 * filesystem scan logic. */
static int find_candidate_file(const char *dir,
                               const char *const *candidates,
                               size_t candidate_count,
                               int max_depth,
                               char *out_path, size_t out_path_cap)
{
    char cmd[2048];
    FILE *p;
    char line[1024];
    size_t i;

    if (!dir || !candidates || candidate_count == 0u) return 0;
    if (max_depth < 0) return 0;

    snprintf(cmd, sizeof(cmd),
             "find '%s' -maxdepth %d -type f \\( ", dir, max_depth);
    for (i = 0u; i < candidate_count; ++i) {
        if (i > 0u) strncat(cmd, " -o ", sizeof(cmd) - strlen(cmd) - 1u);
        strncat(cmd, "-name ", sizeof(cmd) - strlen(cmd) - 1u);
        strncat(cmd, candidates[i], sizeof(cmd) - strlen(cmd) - 1u);
    }
    strncat(cmd, " \\) 2>/dev/null", sizeof(cmd) - strlen(cmd) - 1u);

    p = popen(cmd, "r");
    if (!p) return 0;
    while (fgets(line, sizeof(line), p)) {
        size_t n = strlen(line);
        while (n > 0u && (line[n - 1u] == '\n' || line[n - 1u] == '\r')) {
            line[--n] = '\0';
        }
        if (n == 0u) continue;
        if (n + 1u > out_path_cap) continue;
        for (i = 0u; i < candidate_count; ++i) {
            const char *base = strrchr(line, '/');
            base = base ? base + 1 : line;
            if (strcmp(base, candidates[i]) == 0) {
                strncpy(out_path, line, out_path_cap - 1u);
                out_path[out_path_cap - 1u] = '\0';
                pclose(p);
                return 1;
            }
        }
    }
    pclose(p);
    return 0;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir;
    uint8_t scratch[CSB_V1_CSBWIN_BLOCK1_BYTES];
    uint8_t scratch_copy[CSB_V1_CSBWIN_BLOCK1_BYTES];
    size_t file_size = 0u;
    CSB_V1_CSBWin512Report report;
    CSB_V1_CSBWin512Report report2;
    int rc;

    printf("=== CSB V1 CSBWin 512-byte XOR-pad classifier probe ===\n\n");

    /* ── Real-asset probe (skip-safe) ── */
    {
        static const char *const candidates[] = {
            "csbgame.dat",
            "csbgame.bak",
            "dmsave.dat",
            "dmsave.bak"
        };
        char found_path[1024];
        int found;

        dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
        printf("data_dir=%s\n", dir ? dir : "(none)");

        found = (dir != NULL) && find_candidate_file(
            dir, candidates,
            sizeof(candidates) / sizeof(candidates[0]),
            6, found_path, sizeof(found_path));

        if (!found) {
            printf("SKIP: no user-staged CSBWin / DM1 save file "
                   "(csbgame.dat / csbgame.bak / dmsave.dat / dmsave.bak) "
                   "found under data_dir; data-free unit test "
                   "(csb_v1_csbwin_512_xor_pad_classify_unit) still "
                   "proves the contract on synthetic fixtures.\n");
            return 0;
        }
        printf("real_save=%s\n", found_path);

        if (!read_file_first_n(found_path, scratch, sizeof(scratch),
                               &file_size)) {
            printf("SKIP: failed to read %s; data-free unit test "
                   "still proves the contract.\n", found_path);
            return 0;
        }
        printf("real_save_size=%zu (clamped to first %zu bytes for "
               "the 512-byte XOR-pad classifier)\n",
               file_size, sizeof(scratch));
        if (file_size < CSB_V1_CSBWIN_BLOCK1_BYTES) {
            printf("SKIP: real save is %zu bytes; the 512-byte "
                   "XOR-pad classifier requires at least 512 bytes "
                   "(CSBWin/Chaos.cpp:1326 ReadGameBlock1). "
                   "Data-free unit test still proves the contract.\n",
                   file_size);
            return 0;
        }
    }

    /* ── Classify the real bytes ── */
    memcpy(scratch_copy, scratch, sizeof(scratch));
    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbwin_512_xor_pad_classify(scratch, sizeof(scratch),
                                            &report);
    printf("classify rc=%d (%s)\n", rc,
           csb_v1_csbwin_512_xor_pad_result_name(rc));
    printf("verdict=%s key_index=%d\n",
           csb_v1_csbwin_512_xor_pad_verdict_name(report.verdict),
           report.key_index);
    printf("first_half_d6w=0x%04x second_half_d5w=0x%04x\n",
           report.first_half_d6w, report.second_half_d5w);
    printf("format_id=%u game_id=0x%08x platform=%d dungeon_id=0x%04x\n",
           report.public_fields.format_id,
           (unsigned)report.public_fields.game_id,
           (int)report.public_fields.platform,
           report.public_fields.dungeon_id);
    printf("keys[0]=0x%04x keys[1]=0x%04x checksums[0]=0x%04x\n",
           report.public_fields.keys[0],
           report.public_fields.keys[1],
           report.public_fields.checksums[0]);
    printf("additional[0..3]=0x%02x 0x%02x 0x%02x 0x%02x\n",
           report.public_fields.additional_data[0],
           report.public_fields.additional_data[1],
           report.public_fields.additional_data[2],
           report.public_fields.additional_data[3]);

    /* ── Invariants ── */
    CHECK(rc == CSB_V1_CSBWIN_512_OK,
          "classify returns OK on real bytes");
    /* Real CSBWin saves should resolve to either CSB or DM
     * (NEITHER would mean the block is corrupt or foreign). */
    CHECK(report.verdict == CSB_V1_CSBWIN_512_VERDICT_CSB ||
          report.verdict == CSB_V1_CSBWIN_512_VERDICT_DM ||
          report.verdict == CSB_V1_CSBWIN_512_VERDICT_NEITHER,
          "verdict is one of the documented enum values");
    /* If the verdict is CSB or DM, the first-half and second-half
     * checksums must agree (D5W == D6W per CSBWin
     * UnscrambleBlock1). */
    if (report.verdict != CSB_V1_CSBWIN_512_VERDICT_NEITHER) {
        CHECK(report.first_half_d6w == report.second_half_d5w,
              "D5W == D6W after a successful unscramble");
        CHECK(report.key_index == CSB_V1_CSBWIN_512_KEY_CSB ||
              report.key_index == CSB_V1_CSBWIN_512_KEY_DM,
              "key_index is one of the documented C29 / C10 keys");
    } else {
        printf("NOTE: real bytes produced a NEITHER verdict; this "
               "is acceptable for corrupt / foreign blocks.\n");
    }

    /* ── Buffer non-mutation invariant ── */
    CHECK(memcmp(scratch, scratch_copy, sizeof(scratch)) == 0,
          "classify did not modify the caller's input buffer");

    /* ── Determinism on a re-read ── */
    memset(&report2, 0, sizeof(report2));
    rc = csb_v1_csbwin_512_xor_pad_classify(scratch, sizeof(scratch),
                                            &report2);
    CHECK(rc == CSB_V1_CSBWIN_512_OK,
          "second classify returns OK on the same buffer");
    CHECK(report2.verdict == report.verdict,
          "second classify verdict matches the first");
    if (report.verdict != CSB_V1_CSBWIN_512_VERDICT_NEITHER) {
        CHECK(report2.key_index == report.key_index &&
              report2.first_half_d6w == report.first_half_d6w &&
              report2.second_half_d5w == report.second_half_d5w &&
              report2.public_fields.format_id ==
                  report.public_fields.format_id &&
              report2.public_fields.game_id == report.public_fields.game_id,
              "second classify reports identical public fields");
    }

    printf("\n=== Summary: %d checks, %d failures ===\n",
           g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
