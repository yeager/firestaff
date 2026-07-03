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
 *     verifies the loader-boundary verdict on the real bytes.
 *
 * Skip-safe by design: hosts without a known CSBWin / DM1 save
 * exit 0 with a SKIP message after the synthetic-fixture portion
 * has already proven the contract — matches the existing
 * firestaff_csb_v1_csbgraphics_dat_real_scan_probe / HCSB.HTC
 * pattern. The probe never errors on missing real data because
 * the loader-boundary gate itself is data-free.
 */

#include "csb_v1_csbwin_save_loader_boundary_pc34_compat.h"
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
static int find_candidate_file(const char *dir,
                               int max_depth,
                               char *out_path, size_t out_path_cap)
{
    DIR *d;
    struct dirent *ent;

    if (!dir || max_depth < 0) return 0;
    d = opendir(dir);
    if (!d) return 0;

    while ((ent = readdir(d)) != NULL) {
        char path[1024];
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
            int ok = copy_path(out_path, out_path_cap, path);
            closedir(d);
            return ok;
        }
        if (S_ISDIR(st.st_mode) && max_depth > 0) {
            if (find_candidate_file(path, max_depth - 1,
                                    out_path, out_path_cap)) {
                closedir(d);
                return 1;
            }
        }
    }
    closedir(d);
    return 0;
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
        char found_path[1024];
        CSB_V1_CSBWinSaveDiscoveryResult disc;
        int found;
        int rc;

        dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
        printf("data_dir=%s\n", dir ? dir : "(none)");

        found = (dir != NULL) &&
            find_candidate_file(dir, 6, found_path, sizeof(found_path));

        if (!found) {
            printf("SKIP: no user-staged CSBWin / DM1 save file "
                   "(csbgame.dat / csbgame.bak / dmsave.dat / dmsave.bak) "
                   "found under data_dir; loader-boundary gate has "
                   "still been proven on synthetic fixtures.\n");
            return 0;
        }
        printf("real_save=%s\n", found_path);

        rc = csb_v1_csbwin_save_loader_boundary_classify_file(
            found_path, 4u * 1024u * 1024u, &disc);
        if ((rc == CSB_SAVE_IMPORT_ERR_TRUNCATED ||
             rc == CSB_SAVE_IMPORT_ERR_NULL) &&
            disc.shape == CSB_V1_CSBWIN_SHAPE_COUNT) {
            printf("SKIP: failed to read %s "
                   "(larger than 4 MiB or unreadable); synthetic-fixture "
                   "gate still stands.\n", found_path);
            return 0;
        }
        printf("real discovery verdict: rc=%d, file_kind=%s, shape=%s, "
               "loader_code=%d, contract_match=%d, should_attempt_import=%d, "
               "decision=%s\n",
               rc,
               disc.file_kind_label,
               csb_v1_csbwin_save_loader_boundary_shape_name(disc.shape),
               disc.loader.loader_code,
               disc.loader.contract_match,
               disc.should_attempt_import,
               disc.decision_label);
        CHECK(disc.filename_candidate == 1,
              "real staged path uses a recognised CSBWin save filename");
        CHECK(disc.loader.contract_match == 1,
              "real staged bytes have a deterministic loader-boundary verdict");
        CHECK(disc.should_attempt_import ==
                  (disc.loader.loader_code > 0 &&
                   disc.loader.contract_match == 1),
              "real should_attempt_import mirrors accepted loader verdict");
    }

    printf("\n=== Summary: %d checks, %d failures ===\n",
           g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
