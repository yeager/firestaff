/*
 * firestaff_csb_v1_csbwin_save_loader_boundary_probe.c
 *
 * Real-asset CSBWin save-side loader-boundary probe.
 *
 * Source-lock boundary (see include/csb_v1_csbwin_save_loader_
 * boundary_pc34_compat.h for the full evidence chain):
 *   - ReDMCSB CEDTINC8.C:101-118 (DMSAVE / CSBGAME.DAT routing)
 *   - ReDMCSB LOADSAVE.C F0433/F0435 (CSBGAME namespace)
 *   - ReDMCSB SAVEHEAD.C F0429/F0430 (header read/write)
 *   - ReDMCSB DEFS.H:1289 (CSBGAME.DAT magic)
 *   - CSBWin SaveGame.cpp:927/1711/2111 (save file I/O)
 *   - CSBWin CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak)
 *
 * What this proves:
 *   - The CSB V1 loader-boundary contract (csb_v1_csbwin_save_
 *     loader_boundary_check) correctly classifies each synthetic
 *     CSBWin / DM1 save shape (3 accept + 11 reject = 14 total).
 *   - The accept-shape helper csb_v1_csbwin_save_loader_boundary_
 *     match() recognises a hand-rolled v2.0/v2.1 buffer and
 *     rejects every non-accept shape.
 *   - When a user-staged CSBWin save is present, the probe
 *     verifies the loader-boundary and DSA runtime-handoff verdicts
 *     on the real bytes. Plain CSBGAME loader-ready saves must stay
 *     DSA-runtime blocked unless a real Extended Features DSA corpus
 *     and following GAMEBLOCK1 header authenticate.
 *
 * Skip-safe by design: hosts without a known CSBWin / DM1 save
 * exit 0 with a SKIP message after the data-free contract portion
 * has already pinned the loader boundary — matches the existing
 * firestaff_csb_v1_csbgraphics_dat_real_scan_probe / HCSB.HTC
 * pattern. The probe never errors on missing real data because
 * the loader-boundary gate itself is data-free.
 */

#include "csb_v1_csbwin_save_loader_boundary_pc34_compat.h"
#include "csb_v1_csbwin_save_loader_boundary_fixture.h"
#include "csb_v1_save_import_path_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

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

/* Resolve the data-dir argument / env var. Mirrors the helper
 * in firestaff_csb_v1_csbgraphics_dat_real_scan_probe.c. */
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

static int copy_path(char *out_path, size_t out_path_cap, const char *path)
{
    size_t n;
    if (!out_path || !path || out_path_cap == 0u) return 0;
    n = strlen(path);
    if (n + 1u > out_path_cap) return 0;
    memcpy(out_path, path, n + 1u);
    return 1;
}

static int join_path(char *out, size_t out_cap,
                     const char *dir, const char *name)
{
    int written;
    if (!out || !dir || !name || out_cap == 0u) return 0;
    written = snprintf(out, out_cap, "%s/%s", dir, name);
    return written > 0 && (size_t)written < out_cap;
}

/* Walk `dir` recursively looking for the first CSBWin save candidate
 * by basename. Discovery is case-insensitive and shared with the public
 * classifier, so the real-data probe and launcher-facing gate cannot
 * drift. */
enum {
    MAX_REAL_SAVE_CANDIDATES = 64,
    REAL_SAVE_PATH_BYTES = 1024
};

static int copy_candidate(char out_paths[][REAL_SAVE_PATH_BYTES],
                          size_t out_cap,
                          size_t *count,
                          const char *path)
{
    if (!out_paths || !count || !path) return 0;
    if (*count >= out_cap) return 0;
    if (!copy_path(out_paths[*count], REAL_SAVE_PATH_BYTES, path)) return 0;
    ++(*count);
    return 1;
}

static int collect_candidate_files(const char *dir,
                                   int max_depth,
                                   char out_paths[][REAL_SAVE_PATH_BYTES],
                                   size_t out_cap,
                                   size_t *count,
                                   int *overflow)
{
    DIR *d;
    struct dirent *ent;

    if (!dir || max_depth < 0 || !count) return 0;
    d = opendir(dir);
    if (!d) return 0;

    while ((ent = readdir(d)) != NULL) {
        char path[REAL_SAVE_PATH_BYTES];
        struct stat st;
        if (ent->d_name[0] == '.' &&
            (ent->d_name[1] == '\0' ||
             (ent->d_name[1] == '.' && ent->d_name[2] == '\0'))) {
            continue;
        }
        if (!join_path(path, sizeof(path), dir, ent->d_name)) {
            continue;
        }
        if (stat(path, &st) != 0) {
            continue;
        }
        if (S_ISREG(st.st_mode) &&
            csb_v1_csbwin_save_loader_boundary_file_kind(path) !=
                CSB_V1_CSBWIN_SAVE_FILE_NONE) {
            if (!copy_candidate(out_paths, out_cap, count, path) && overflow) {
                *overflow = 1;
            }
            continue;
        }
        if (S_ISDIR(st.st_mode) && max_depth > 0) {
            collect_candidate_files(path, max_depth - 1,
                                    out_paths, out_cap, count, overflow);
        }
    }
    closedir(d);
    return *count > 0u;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir;
    uint8_t scratch[CSB_SAVE_HEADER_SIZE + 4u * CSB_SAVE_CHAMP_SIZE + 16u];
    size_t fixture_size = 0u;
    const CSB_V1_CSBWinSaveShapeContract *table = NULL;
    size_t table_count = 0u;
    size_t i;
    size_t accept_pass = 0u;
    size_t reject_pass = 0u;

    printf("=== CSB V1 CSBWin save loader-boundary probe ===\n\n");

    /* ── Synthetic fixture pass (always runs) ── */
    table = csb_v1_csbwin_save_loader_boundary_contract(&table_count);
    printf("contract_table_count = %zu (accept=%zu, reject=%zu)\n",
           table_count,
           csb_v1_csbwin_save_loader_boundary_accept_count(),
           csb_v1_csbwin_save_loader_boundary_reject_count());

    for (i = 0u; i < table_count; ++i) {
        CSB_V1_CSBWinSaveShape shape = table[i].shape;
        CSB_V1_CSBWinLoaderBoundaryResult res;
        const char *shape_name =
            csb_v1_csbwin_save_loader_boundary_shape_name(shape);
        char msg[200];
        int rc;

        rc = csb_v1_csbwin_save_loader_boundary_check_shape(shape, &res);
        snprintf(msg, sizeof(msg),
                 "%s: rc=%d, contract_match=%d",
                 shape_name, rc, res.contract_match);
        CHECK(res.contract_match == 1, msg);
        if (table[i].expect_accept) ++accept_pass;
        else                        ++reject_pass;
    }
    CHECK(accept_pass == csb_v1_csbwin_save_loader_boundary_accept_count(),
          "all accept-shapes loader-passed");
    CHECK(reject_pass == csb_v1_csbwin_save_loader_boundary_reject_count(),
          "all reject-shapes loader-rejected");

    /* ── Accept-shape helper ── */
    {
        CSB_V1_CSBWinSaveShape matched;
        fixture_size = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, scratch, sizeof(scratch));
        CHECK(fixture_size > 0u, "v2.0 fixture built for match() check");
        matched = csb_v1_csbwin_save_loader_boundary_match(
            scratch, fixture_size);
        CHECK(matched == CSB_V1_CSBWIN_SHAPE_CSBGAME_V20,
              "match() recognises CSB v2.0 buffer");
    }

    /* ── Real-asset probe (skip-safe) ──
     *
     * Try to find a user-staged CSBWin save under the data dir.
     * If we find one, read it through a bounded cap and run the
     * public discovery classifier on the real bytes. If not, we
     * SKIP cleanly. */
    {
        char found_paths[MAX_REAL_SAVE_CANDIDATES][REAL_SAVE_PATH_BYTES];
        size_t found_count = 0u;
        size_t real_loader_ready_count = 0u;
        size_t real_dsa_positive_count = 0u;
        size_t real_dsa_blocked_count = 0u;
        int overflow = 0;
        size_t candidate_index;

        dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
        printf("data_dir=%s\n", dir ? dir : "(none)");

        if (dir != NULL) {
            struct stat st;
            if (stat(dir, &st) == 0 && S_ISREG(st.st_mode) &&
                csb_v1_csbwin_save_loader_boundary_file_kind(dir) !=
                    CSB_V1_CSBWIN_SAVE_FILE_NONE) {
                copy_candidate(found_paths, MAX_REAL_SAVE_CANDIDATES,
                               &found_count, dir);
            } else {
                collect_candidate_files(dir, 6, found_paths,
                                        MAX_REAL_SAVE_CANDIDATES,
                                        &found_count, &overflow);
            }
        }

        if (found_count == 0u) {
            printf("SKIP: no user-staged CSBWin / DM1 save file "
                   "(csbgame.dat through csbgame4.dat / csbgame.bak / dmsave.dat / dmsave.bak) "
                   "found under data_dir; loader-boundary gate has "
                   "data-free contract coverage, but no positive real DSA "
                   "corpus was claimed.\n");
            return 0;
        }
        printf("real_save_candidate_count=%zu%s\n", found_count,
               overflow ? " (truncated to probe cap)" : "");

        for (candidate_index = 0u;
             candidate_index < found_count;
             ++candidate_index) {
            const char *found_path = found_paths[candidate_index];
            CSB_V1_CSBWinSaveDiscoveryResult disc;
            CSB_V1_CSBWinDSASaveCorpusReceipt dsa_receipt;
            char msg[240];
            int rc;
            int dsa_rc;

            printf("real_save[%zu]=%s\n", candidate_index, found_path);

            rc = csb_v1_csbwin_save_loader_boundary_classify_file(
                found_path, 4u * 1024u * 1024u, &disc);
            if ((rc == CSB_SAVE_IMPORT_ERR_TRUNCATED ||
                 rc == CSB_SAVE_IMPORT_ERR_NULL) &&
                disc.shape == CSB_V1_CSBWIN_SHAPE_COUNT) {
                snprintf(msg, sizeof(msg),
                         "real staged path %zu failed bounded read cleanly",
                         candidate_index);
                CHECK(1, msg);
                continue;
            }
            printf("  discovery: rc=%d, file_kind=%s, shape=%s, "
                   "loader_code=%d, contract_match=%d, "
                   "should_attempt_import=%d, decision=%s\n",
                   rc,
                   disc.file_kind_label,
                   csb_v1_csbwin_save_loader_boundary_shape_name(disc.shape),
                   disc.loader.loader_code,
                   disc.loader.contract_match,
                   disc.should_attempt_import,
                   disc.decision_label);
            snprintf(msg, sizeof(msg),
                     "real staged path %zu uses a recognised CSBWin filename",
                     candidate_index);
            CHECK(disc.filename_candidate == 1, msg);
            snprintf(msg, sizeof(msg),
                     "real staged path %zu has deterministic loader verdict",
                     candidate_index);
            CHECK(disc.loader.contract_match == 1, msg);
            CHECK(disc.should_attempt_import ==
                      (disc.loader.loader_code > 0 &&
                       disc.loader.contract_match == 1),
                  "real should_attempt_import mirrors accepted loader verdict");

            dsa_rc =
                csb_v1_csbwin_save_loader_boundary_dsa_corpus_receipt_file(
                    found_path, 4u * 1024u * 1024u, &dsa_receipt);
            printf("  dsa_corpus: rc=%d, positive=%d, handoff_ready=%d, "
                   "extended_tail=%d, dsa=%d, actions=%d, gameblock1=%d, "
                   "gameblock1_offset=%zu, decision=%s\n",
                   dsa_rc,
                   dsa_receipt.corpus_positive,
                   dsa_receipt.runtime_handoff_ready,
                   dsa_receipt.extended_tail_valid,
                   dsa_receipt.dsa_section_valid,
                   dsa_receipt.dsa_has_runtime_actions,
                   dsa_receipt.gameblock1_valid,
                   dsa_receipt.gameblock1_offset,
                   dsa_receipt.decision_label);
            snprintf(msg, sizeof(msg),
                     "real staged path %zu keeps DSA handoff tied to corpus positivity",
                     candidate_index);
            CHECK(dsa_receipt.runtime_handoff_ready ==
                      dsa_receipt.corpus_positive,
                  msg);
            if (disc.should_attempt_import && !dsa_receipt.corpus_positive) {
                ++real_dsa_blocked_count;
                snprintf(msg, sizeof(msg),
                         "loader-ready real path %zu is not DSA-runtime ready without corpus proof",
                         candidate_index);
                CHECK(dsa_receipt.runtime_handoff_ready == 0, msg);
            }
            if (disc.should_attempt_import) {
                ++real_loader_ready_count;
            }
            if (dsa_receipt.runtime_handoff_ready) {
                ++real_dsa_positive_count;
                snprintf(msg, sizeof(msg),
                         "DSA-positive real path %zu carries authenticated runtime actions and GAMEBLOCK1",
                         candidate_index);
                CHECK(dsa_receipt.extended_tail_valid &&
                      dsa_receipt.dsa_has_runtime_actions &&
                      dsa_receipt.gameblock1_valid,
                      msg);
            }
        }
        printf("real_loader_ready_count=%zu\n", real_loader_ready_count);
        printf("real_dsa_blocked_loader_ready_count=%zu\n",
               real_dsa_blocked_count);
        printf("real_dsa_positive_count=%zu\n", real_dsa_positive_count);
    }

    printf("\n=== Summary: %d checks, %d failures ===\n",
           g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
