/*
 * csb_v1_csbgraphics_dat_real_scan.h
 *
 * Hash-based discovery + cache handoff for a real CSBWin
 * "CSBgraphics.dat" custom graphics override file in the user's
 * ~/.firestaff/data tree.
 *
 * This module sits beside (not under) the format-contract
 * classifier in include/csb_v1_csbgraphics_dat_classify.h. The
 * split is intentional:
 *
 *   - csb_v1_csbgraphics_dat_classify owns the bytes → index
 *     shape contract: count + parallel compressed/decompressed
 *     tables + payload offset.
 *   - csb_v1_csbgraphics_dat_real_scan owns the discovery
 *     + ownership contract: where the file lives, which MD5s
 *     we accept, how virtual container paths get materialized,
 *     and how the file buffer + parsed index get cached.
 *
 * The known MD5 list is intentionally empty by default. CSBWin
 * "CSBgraphics.dat" is a CSBGraffer / CSBWin Viewport Compiler
 * product, not an original CSB asset. There is no canonical
 * reference hash; we only know the hashes of files we have
 * personally staged. Future sessions can append rows as users
 * stage real CSBWin-produced CSBgraphics.dat files under
 * ~/.firestaff/data/csbwin-custom/<label>/.
 *
 * Scope:
 *   - Hash-only discovery. No filename-based discovery.
 *   - Read-only. Does not bind to M11/M12, does not override a
 *     CSB runtime graphic, does not draw a viewport.
 *   - Skip-safe by design. When no known CSBgraphics.dat is
 *     present the cache stays cleared and the probe exits 0
 *     with a SKIP message — matches the existing HCSB.HTC
 *     real-scan probe pattern.
 *
 * Source references:
 *   - CSBWin/Graphics.cpp:1838 OpenCSBgraphicsFile (file path
 *     + signature split).
 *   - CSBWin/data.cpp:1936 Signature (MD5 file digest).
 *   - ReDMCSB F0200_DMMISC_ReadCompressedGraphic
 *   - dmweb Data Files page (graphics.dat layout).
 *
 * See also:
 *   - include/csb_v1_csbgraphics_dat_classify.h
 *   - include/csb_hint_oracle_htc_real_scan.h (the template
 *     this module mirrors)
 *   - docs/FIRESTAFF_GAP_LIST.md row C3 / A3
 */

#ifndef FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_REAL_SCAN_H
#define FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_REAL_SCAN_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_csbgraphics_dat_classify.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Known CSBWin CSBgraphics.dat MD5 hashes, hex, NULL-terminated.
 *
 * The list is intentionally empty in the public build — see the
 * rationale in the module header. Operators with a real
 * CSBWin-produced CSBgraphics.dat can extend the list in their
 * own fork or via a runtime config hook without touching this
 * header. Empty list = no discovery happens, the probe SKIPs.
 */
typedef struct {
    const char *label;        /* human-readable provenance label */
    const char *md5;          /* 32-char hex MD5 */
    size_t      size_bytes;   /* expected size in bytes */
} CSB_V1_CSBGraphicsDatRealKnownHash;

const CSB_V1_CSBGraphicsDatRealKnownHash *
csb_v1_csbgraphics_dat_real_known_hashes(size_t *out_count);

typedef enum {
    CSB_V1_CSBGRAPHICS_DAT_REAL_OK = 0,
    CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_ARGUMENT = -1,
    CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NO_DATA_DIR = -2,
    CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND = -3,
    CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_READ = -4,
    CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_PARSE = -5
} CSB_V1_CSBGraphicsDatRealResult;

#define CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP 512
#define CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP  33
#define CSB_V1_CSBGRAPHICS_DAT_REAL_LABEL_CAP 256

typedef struct {
    uint8_t *file_buffer;
    size_t   file_size;

    char resolved_path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
    char original_path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];

    char matched_md5[CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP];
    char matched_label[CSB_V1_CSBGRAPHICS_DAT_REAL_LABEL_CAP];

    int loaded;

    CSB_V1_CSBGraphicsIndex index;
} CSB_V1_CSBGraphicsDatRealCache;

void csb_v1_csbgraphics_dat_real_cache_init(
    CSB_V1_CSBGraphicsDatRealCache *cache);

void csb_v1_csbgraphics_dat_real_cache_free(
    CSB_V1_CSBGraphicsDatRealCache *cache);

/* Scan `data_dir` recursively for a CSBgraphics.dat matching one
 * of the known MD5 hashes, optionally materializing virtual
 * container paths into the supplied `cache_dir`. On success,
 * allocates and owns the file buffer and populates the parsed
 * index.
 *
 * Returns CSB_V1_CSBGRAPHICS_DAT_REAL_OK on a successful load,
 * CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND when no known
 * CSBgraphics.dat is present (callers may treat this as a SKIP),
 * or another negative error code.
 */
int csb_v1_csbgraphics_dat_real_scan_and_load(
    const char *data_dir,
    const char *cache_dir,
    int max_depth,
    CSB_V1_CSBGraphicsDatRealCache *cache);

/* Lookup helpers. */
int csb_v1_csbgraphics_dat_real_index(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    CSB_V1_CSBGraphicsIndex *out_index);

const char *csb_v1_csbgraphics_dat_real_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_REAL_SCAN_H */
