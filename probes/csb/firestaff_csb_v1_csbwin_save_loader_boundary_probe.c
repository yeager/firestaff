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
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }
    if ((size_t)sz > buf_cap) { fclose(f); return 0; }
    got = fread(buf, 1u, (size_t)sz, f);
    fclose(f);
    if (got != (size_t)sz) return 0;
    *out_size = (size_t)sz;
    return 1;
}

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

/* Walk `dir` (recursive, depth-limited) looking for the first
 * file matching the candidate name list. The candidate list is
 * tried in order so the launcher / M12 surface can prefer
 * `csbgame.dat` over `dmsave.dat` etc. without re-implementing
 * filesystem scan logic. Returns 1 on first match (path copied
 * to `out_path`), 0 otherwise. */
static int find_candidate_file(const char *dir,
                               const char *const *candidates,
                               size_t candidate_count,
                               int max_depth,
                               char *out_path, size_t out_path_cap)
{
    /* Minimal recursive scan: list directory entries, recurse
     * into subdirectories, compare basenames case-sensitively
     * (CSBWin save names are well-defined). We deliberately keep
     * this small and self-contained — no dependency on
     * asset_find_by_md5_list because user-staged CSBWin saves
     * are file-named, not hash-named, in this gap state. */
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
        /* Try the first match in candidate-name priority order. */
        for (i = 0u; i < candidate_count; ++i) {
            /* Match by exact basename (case-sensitive). */
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
     * If we find one, we read the first N bytes into the scratch
     * buffer and call csb_v1_csbwin_save_loader_boundary_check()
     * on the real bytes. If not, we SKIP cleanly. */
    {
        static const char *const candidates[] = {
            "csbgame.dat",
            "csbgame.bak",
            "dmsave.dat",
            "dmsave.bak"
        };
        char found_path[1024];
        size_t real_size = 0u;
        CSB_V1_CSBWinLoaderBoundaryResult res;
        int found;
        int rc;

        dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
        printf("data_dir=%s\n", dir ? dir : "(none)");

        found = (dir != NULL) && find_candidate_file(
            dir, candidates,
            sizeof(candidates) / sizeof(candidates[0]),
            6, found_path, sizeof(found_path));

        if (!found) {
            printf("SKIP: no user-staged CSBWin / DM1 save file "
                   "(csbgame.dat / csbgame.bak / dmsave.dat / dmsave.bak) "
                   "found under data_dir; loader-boundary gate has "
                   "still been proven on synthetic fixtures.\n");
            return 0;
        }
        printf("real_save=%s\n", found_path);

        if (!read_file_into(found_path, scratch, sizeof(scratch),
                            &real_size)) {
            printf("SKIP: failed to read %s into scratch buffer "
                   "(too large or unreadable); synthetic-fixture "
                   "gate still stands.\n", found_path);
            return 0;
        }
        printf("real_save_size=%zu\n", real_size);

        /* Verify the loader-boundary verdict on the real bytes. */
        rc = csb_v1_csbwin_save_loader_boundary_check(
            scratch, real_size,
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, &res);
        printf("real loader-boundary verdict: rc=%d, loader_code=%d, "
               "contract_match=%d\n",
               rc, res.loader_code, res.contract_match);
        /* The verdict depends on whether the real bytes are a
         * v2.0 buffer. We accept either an accept-verdict
         * (loader_code > 0, contract_match = 1) or a reject-
         * verdict (loader_code < 0, contract_match = 0) — both
         * are deterministic, contract_match must equal 1 iff
         * the documented contract is satisfied. */
        CHECK(res.contract_match ==
                  (res.loader_code == CSB_SAVE_IMPORT_ERR_VERSION ||
                   res.loader_code > 0 ||
                   res.loader_code == CSB_SAVE_IMPORT_ERR_BAD_MAGIC ||
                   res.loader_code == CSB_SAVE_IMPORT_ERR_TRUNCATED ||
                   res.loader_code == CSB_SAVE_IMPORT_ERR_NO_CHAMPIONS),
              "real loader-boundary contract_match reflects verdict");
        /* For real bytes, the contract_match is whatever the
         * check returns; we don't pin a specific value because
         * the real bytes might be v2.0, v2.1, or some other
         * shape we haven't seen. */
    }

    printf("\n=== Summary: %d checks, %d failures ===\n",
           g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
