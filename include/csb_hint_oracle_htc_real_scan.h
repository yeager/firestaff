/*
 * csb_hint_oracle_htc_real_scan.h
 *
 * Real Utility Disk HCSB.HTC scanner + cached handoff for the CSB
 * Hint Oracle text/layout format.
 *
 * This module is a thin, data-side bridge that sits on top of the
 * format-contract parser in include/csb_hint_oracle_htc.h:
 *
 *   data_dir
 *     └── <game-data>/csb, csb-atari-st-2x/, csb-extras/...
 *         (recursive, any depth, including ZIP/ISO virtual paths)
 *           └── HCSB.HTC  ← matched by MD5
 *
 *   Steps:
 *     1. Hash-based discovery via asset_find_by_md5_list().
 *        The md5List is a small, source-cited set of known
 *        Utility-Disk / hard-disk HCSB.HTC blobs. Each entry is
 *        a 32-char hex MD5 of a CSB Utility Disk / Meynaf PP
 *        variant. The list is intentionally narrow: only hashes
 *        we can cite to a concrete local source path.
 *     2. If the matched path is a virtual container path
 *        ("archive.zip::HCSB.HTC" or "disc.iso::HCSB.HTC"),
 *        materialize it into a local cache directory using
 *        asset_extract_virtual_path() so the parser can read
 *        ordinary file I/O.
 *     3. Parse with csb_hint_oracle_htc_parse() and cache the
 *        parsed view + the resolved path + matched MD5 + the
 *        owned buffer (so callers don't need to manage file
 *        memory).
 *
 * Scope:
 *   - Hash-only discovery. No filename-based discovery.
 *   - Read-only. Does not draw the Hint Oracle UI or bind to
 *     CSB runtime launch flow.
 *   - Skip-safe by design. When the data root is empty or no
 *     known HCSB.HTC is present, the cache stays cleared and
 *     lookups return no matches; tests can SKIP cleanly.
 *
 * Source references:
 *   - ReDMCSB HINTLOAD.C:11-18 names HCSB.HTC as the canonical
 *     Hint Oracle content file for CSB Utility Disk releases.
 *   - ReDMCSB HINTHTC.C:177-358 validates the format 2 / dungeon
 *     id 13 file the parser expects.
 *   - dmweb Hint Oracle Files page describes the same big-endian
 *     HCSB.HTC table the parser already understands.
 *   - Local HCSB.HTC provenance: see
 *     ~/.firestaff/data/csb-atari-st-2x/HCSB.HTC
 *       (Atari ST 2.x PP 2009-02-22 hard-disk variant,
 *        md5 8ce69b54cf255a15e98e909bb45b9742, 66172 bytes).
 *     ~/.firestaff/data/csb-extras/legacy-amiga-dms/.../HCSBF.HTC
 *       (Amiga 3.3 FR Meynaf hard-disk utility variant,
 *        md5 803ede61136ccfc2bff8e266d8dc3935, 75424 bytes; the
 *        HCSBF marker keeps format 2 / dungeon 13 in the same
 *        big-endian shape, only the compressed-payload stream
 *        length differs — see ReDMCSB HINTLZW.C:122-212).
 */

#ifndef FIRESTAFF_CSB_HINT_ORACLE_HTC_REAL_SCAN_H
#define FIRESTAFF_CSB_HINT_ORACLE_HTC_REAL_SCAN_H

#include <stddef.h>
#include <stdint.h>

#include "csb_hint_oracle_htc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Known HCSB.HTC MD5 hashes, hex, NULL-terminated.
 *
 * The list is intentionally narrow and source-cited:
 *   [0] Atari ST 2.x hard-disk PP variant (2009-02-22), 66172 bytes.
 *       Verified locally at
 *       ~/.firestaff/data/csb-atari-st-2x/HCSB.HTC and at
 *       ~/.firestaff/data/csb-extras/legacy-atari-st/HardDisk/
 *         2009-02-22 PP/HCSB.HTC
 *       (same content). Both have sha256
 *       1b2fbff81a11928afd153f46c117355cce1f9a93f482d14d58e35a115d9cde38
 *   [1] Amiga 3.3 FR Meynaf hard-disk utility variant, 75424 bytes.
 *       Verified locally at
 *       ~/.firestaff/data/csb-extras/legacy-amiga-dms/HardDisk/
 *         Chaos Strikes Back for Amiga v3.3 (French) Hacked by
 *         Meynaf/FTL_CSB_Utility/HCSBF.HTC
 *
 * Additional documented CSB Utility Disk releases (English R1/R2/R3,
 * French R1, German R1/R2) are not yet in the list because we have
 * not verified a local copy of the HCSB.HTC contents under their
 * canonical hashes. The list is meant to grow as more variants are
 * classified; see docs/FIRESTAFF_GAP_LIST.md row C2.
 */
typedef struct {
    const char *label;        /* human-readable provenance label */
    const char *md5;          /* 32-char hex MD5 */
    size_t      size_bytes;   /* expected size in bytes */
} CSB_HintOracleHTC_RealKnownHash;

const CSB_HintOracleHTC_RealKnownHash *
csb_hint_oracle_htc_real_known_hashes(size_t *out_count);

typedef enum {
    CSB_HINT_ORACLE_HTC_REAL_OK = 0,
    CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_HTC_REAL_ERR_NO_DATA_DIR = -2,
    CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND = -3,
    CSB_HINT_ORACLE_HTC_REAL_ERR_READ = -4,
    CSB_HINT_ORACLE_HTC_REAL_ERR_PARSE = -5,
    CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED = -6,
    CSB_HINT_ORACLE_HTC_REAL_ERR_OUTPUT_TOO_SMALL = -7
} CSB_HintOracleHTC_RealResult;

/* Maximum length (incl. trailing NUL) of the resolved-path
 * buffer and the MD5 buffer. */
#define CSB_HINT_ORACLE_HTC_REAL_PATH_CAP 512
#define CSB_HINT_ORACLE_HTC_REAL_MD5_CAP 33

typedef struct {
    /* Owned file buffer; valid between _load() and _free(). */
    uint8_t *file_buffer;
    size_t   file_size;

    /* Resolved path used for the load. May be:
     *   - ordinary path on disk (e.g.
     *     "/Users/.../csb-atari-st-2x/HCSB.HTC")
     *   - virtual container path materialized through
     *     asset_extract_virtual_path() (e.g.
     *     "archive.zip::HCSB.HTC" — the parser reads the
     *     materialized file but the cache keeps the original
 *     virtual descriptor for diagnostics).
     */
    char resolved_path[CSB_HINT_ORACLE_HTC_REAL_PATH_CAP];
    char original_path[CSB_HINT_ORACLE_HTC_REAL_PATH_CAP];

    /* The MD5 we matched, hex string. */
    char matched_md5[CSB_HINT_ORACLE_HTC_REAL_MD5_CAP];
    char matched_label[CSB_HINT_ORACLE_HTC_REAL_PATH_CAP];

    /* Set to 1 once a successful parse has populated `htc`. */
    int loaded;

    /* Parsed view into file_buffer. Pointers inside this struct
 * point into file_buffer — they remain valid only while
 * file_buffer is owned. */
    CSB_HintOracleHTC htc;
} CSB_HintOracleHTC_RealCache;

/* Initialize an empty cache (no allocation, no file I/O). */
void csb_hint_oracle_htc_real_cache_init(CSB_HintOracleHTC_RealCache *cache);

/* Scan `data_dir` recursively for an HCSB.HTC matching one of the
 * known MD5 hashes, optionally materializing virtual container
 * paths into the supplied `cache_dir`. On success, allocates and
 * owns the file buffer and populates the parsed view.
 *
 * `data_dir` may be NULL or empty to fall back to the FSP
 * user-data default; `cache_dir` may be NULL to skip explicit
 * materialization (in which case virtual container paths are
 * reported but not extracted — only ordinary on-disk matches
 * load successfully).
 *
 * Returns CSB_HINT_ORACLE_HTC_REAL_OK on a successful load,
 * CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND when no known HCSB.HTC
 * is present (callers may treat this as a SKIP), or another
 * negative error code.
 */
int csb_hint_oracle_htc_real_scan_and_load(
    const char *data_dir,
    const char *cache_dir,
    int max_depth,
    CSB_HintOracleHTC_RealCache *cache);

/* Free the owned file buffer and reset the cache in place. */
void csb_hint_oracle_htc_real_cache_free(CSB_HintOracleHTC_RealCache *cache);

/* Look up hints at (level, x, y) using the loaded cache.
 * Returns CSB_HINT_ORACLE_HTC_REAL_OK with `*out_count` set, or
 * CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED when the cache is empty,
 * CSB_HINT_ORACLE_HTC_REAL_ERR_OUTPUT_TOO_SMALL when `out_capacity`
 * is too small (the count is still written back). */
int csb_hint_oracle_htc_real_find_hints_for_location(
    const CSB_HintOracleHTC_RealCache *cache,
    uint8_t level,
    uint8_t x,
    uint8_t y,
    uint16_t *out_indices,
    size_t out_capacity,
    size_t *out_count);

/* Copy the hint name for `hint_index` into `buf` (NUL-terminated).
 * Returns CSB_HINT_ORACLE_HTC_REAL_OK, or
 * CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT, _NOT_LOADED, or _OUTPUT_TOO_SMALL. */
int csb_hint_oracle_htc_real_get_hint_name(
    const CSB_HintOracleHTC_RealCache *cache,
    size_t hint_index,
    char *buf,
    size_t buf_size);

/* Decompress the first page of hint `hint_index` into `out_buf`.
 * `*out_size` is set to the decompressed byte count.
 * Returns CSB_HINT_ORACLE_HTC_REAL_OK or one of the standard
 * error codes. */
int csb_hint_oracle_htc_real_decompress_first_page(
    const CSB_HintOracleHTC_RealCache *cache,
    size_t hint_index,
    uint8_t *out_buf,
    size_t out_capacity,
    size_t *out_size);

const char *csb_hint_oracle_htc_real_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_HTC_REAL_SCAN_H */
