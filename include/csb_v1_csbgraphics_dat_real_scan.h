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
 *     we accept, how user-staged hash manifests are consumed,
 *     how virtual container paths are read in memory, and how the
 *     file buffer + parsed index get cached.
 *
 * The built-in known MD5 list is intentionally empty by default.
 * CSBWin "CSBgraphics.dat" is a CSBGraffer / CSBWin Viewport
 * Compiler product, not an original CSB asset. There is no
 * canonical reference hash. User-staged files can be registered
 * without rebuilding Firestaff by adding a manifest named
 * `csbgraphics.hashes` under the scanned data root:
 *
 *   <md5-hex> <size-bytes> <label>
 *
 * Lines starting with `#` are ignored. Discovery still remains
 * hash-only: the manifest registers acceptable MD5/size pairs;
 * filenames and directory layout are not trusted.
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

/* Built-in CSBWin CSBgraphics.dat MD5 hashes, hex, NULL-terminated.
 *
 * The list is intentionally empty in the public build — see the
 * rationale in the module header. Operators with a real
 * CSBWin-produced CSBgraphics.dat should prefer the data-root
 * `csbgraphics.hashes` manifest over recompiling this list.
 * Empty built-in list plus absent manifest = no discovery happens,
 * the probe SKIPs.
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

#define CSB_V1_CSBGRAPHICS_DAT_PALETTE_BYTES 768

/* A candidate is a strictly sized indexed-archive payload, not a guessed
 * palette interpretation. It is reported only after the declared 768 bytes
 * decode exactly and an FNV identity is available. */
typedef struct {
    int valid;
    char source_path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
    char source_md5[CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP];
    CSB_V1_CSBGraphicsEntrySpan entry_span;
    uint32_t decoded_fnv1a;
} CSB_V1_CSBGraphicsDatPaletteCandidate;

typedef struct {
    int valid;
    char source_path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
    char source_md5[CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP];
    size_t candidate_count;
    CSB_V1_CSBGraphicsDatPaletteCandidate *candidates;
} CSB_V1_CSBGraphicsDatPaletteCandidateReport;

typedef struct {
    const char *source_path;
    const char *source_md5;
    uint32_t entry_index;
    uint32_t decoded_fnv1a;
} CSB_V1_CSBGraphicsDatPaletteAdmissionSpec;

typedef struct {
    int valid;
    char source_path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
    char source_md5[CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP];
    CSB_V1_CSBGraphicsEntrySpan entry_span;
    uint32_t decoded_fnv1a;
    uint8_t decoded_bytes[CSB_V1_CSBGRAPHICS_DAT_PALETTE_BYTES];
} CSB_V1_CSBGraphicsDatPaletteSourceReceipt;

void csb_v1_csbgraphics_dat_real_cache_init(
    CSB_V1_CSBGraphicsDatRealCache *cache);

void csb_v1_csbgraphics_dat_real_cache_free(
    CSB_V1_CSBGraphicsDatRealCache *cache);

/* Scan `data_dir` recursively for a CSBgraphics.dat matching one
 * of the known MD5 hashes. `cache_dir` is retained for source compatibility
 * but is ignored: virtual container paths are read in bounded RAM. On success,
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

/* Walk only exact 768-byte declared entries in a loaded, hash-admitted cache.
 * Each returned candidate has passed a bounded decode and carries cache path,
 * MD5, entry span and decoded FNV-1a. The caller releases report storage. */
int csb_v1_csbgraphics_dat_real_scan_palette_candidates(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    CSB_V1_CSBGraphicsDatPaletteCandidateReport *out_report);
void csb_v1_csbgraphics_dat_real_palette_candidate_report_free(
    CSB_V1_CSBGraphicsDatPaletteCandidateReport *report);

/* Produce a palette receipt only when a caller-declared path, MD5, entry and
 * FNV identity exactly match a previously discovered candidate and a second
 * bounded decode produces those bytes again. */
int csb_v1_csbgraphics_dat_real_admit_palette_candidate(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsDatPaletteCandidate *candidate,
    const CSB_V1_CSBGraphicsDatPaletteAdmissionSpec *spec,
    CSB_V1_CSBGraphicsDatPaletteSourceReceipt *out_receipt);

const char *csb_v1_csbgraphics_dat_real_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_REAL_SCAN_H */
