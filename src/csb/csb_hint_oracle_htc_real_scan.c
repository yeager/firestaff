/*
 * csb_hint_oracle_htc_real_scan.c
 *
 * Real Utility Disk HCSB.HTC scanner + cached handoff for the CSB
 * Hint Oracle text/layout format.
 *
 * See include/csb_hint_oracle_htc_real_scan.h for scope and source
 * references.
 *
 * The implementation is deliberately small and stays close to the
 * documented contract:
 *
 *   - known MD5 list is the only source of "what we accept"
 *   - asset_find_by_md5_list() does the recursive, virtual-aware
 *     discovery (it already handles ZIP/ISO entries)
 *   - asset_extract_virtual_path() materializes a virtual path
 *     into a real file when the caller provides a cache_dir
 *   - the parser in csb_hint_oracle_htc.c owns the format
 *     contract and remains the single source of truth for what
 *     the file's tables actually mean
 *
 * Nothing in this file reaches into the CSB runtime, the launcher
 * UI, or the Hint Oracle rendering path. That remains tracked
 * separately under docs/FIRESTAFF_GAP_LIST.md row C1/A1.
 */

#include "csb_hint_oracle_htc_real_scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"

/* ── Known MD5 list ──────────────────────────────────────────────── */

static const CSB_HintOracleHTC_RealKnownHash g_known_hashes[] = {
    {
        "csb-atari-st-2x/2009-02-22-PP-hard-disk",
        "8ce69b54cf255a15e98e909bb45b9742",
        66172u
    },
    {
        "csb-amiga-3.x-en-R2-util-disk",
        "334fc18cb98d1280a4c55a16566d5ef9",
        68912u
    },
    {
        "csb-amiga-3.x-en-R3-util-disk",
        "c06862298f193b1fe479eaeff6acd57e",
        69963u
    },
    {
        "csb-extras/legacy-amiga-dms/Meynaf-FR-v3.3-hard-disk",
        "803ede61136ccfc2bff8e266d8dc3935",
        75424u
    },
    {
        "csb-amiga-3.x-ge-R1-R2-util-disk",
        "5a7ab2c8387215c7b2abe772e2ddc689",
        75504u
    }
};

const CSB_HintOracleHTC_RealKnownHash *
csb_hint_oracle_htc_real_known_hashes(size_t *out_count)
{
    if (out_count) {
        *out_count = sizeof(g_known_hashes) / sizeof(g_known_hashes[0]);
    }
    return g_known_hashes;
}

/* ── Cache lifecycle ─────────────────────────────────────────────── */

void csb_hint_oracle_htc_real_cache_init(CSB_HintOracleHTC_RealCache *cache)
{
    if (!cache) {
        return;
    }
    memset(cache, 0, sizeof(*cache));
}

void csb_hint_oracle_htc_real_cache_free(CSB_HintOracleHTC_RealCache *cache)
{
    if (!cache) {
        return;
    }
    if (cache->file_buffer) {
        free(cache->file_buffer);
    }
    memset(cache, 0, sizeof(*cache));
}

/* ── Result-name table ───────────────────────────────────────────── */

const char *csb_hint_oracle_htc_real_result_name(int result)
{
    switch (result) {
    case CSB_HINT_ORACLE_HTC_REAL_OK: return "OK";
    case CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT: return "argument";
    case CSB_HINT_ORACLE_HTC_REAL_ERR_NO_DATA_DIR: return "no-data-dir";
    case CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND: return "not-found";
    case CSB_HINT_ORACLE_HTC_REAL_ERR_READ: return "read";
    case CSB_HINT_ORACLE_HTC_REAL_ERR_PARSE: return "parse";
    case CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED: return "not-loaded";
    case CSB_HINT_ORACLE_HTC_REAL_ERR_OUTPUT_TOO_SMALL: return "output-too-small";
    default: return "unknown";
    }
}

/* ── Helpers ─────────────────────────────────────────────────────── */

static int copy_string(char *dst, size_t dst_size, const char *src)
{
    size_t len;
    if (!dst || dst_size == 0u) {
        return 0;
    }
    if (!src) {
        dst[0] = '\0';
        return 0;
    }
    len = strlen(src);
    if (len + 1u > dst_size) {
        dst[0] = '\0';
        return 0;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
    return 1;
}

static int is_virtual_path(const char *path)
{
    return path && strstr(path, "::") != NULL;
}

/* Read the entire file at `path` into a freshly-allocated buffer.
 * Returns 1 on success and writes the size through `out_size`. */
static int read_whole_file(const char *path,
                           uint8_t **out_buffer,
                           size_t *out_size)
{
    FILE *fp;
    long sz;
    uint8_t *buf;
    size_t got;

    if (!path || !out_buffer || !out_size) {
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    sz = ftell(fp);
    if (sz < 0) {
        fclose(fp);
        return 0;
    }
    rewind(fp);
    buf = (uint8_t *)malloc((size_t)sz > 0u ? (size_t)sz : 1u);
    if (!buf) {
        fclose(fp);
        return 0;
    }
    got = fread(buf, 1u, (size_t)sz, fp);
    fclose(fp);
    if (got != (size_t)sz) {
        free(buf);
        return 0;
    }
    *out_buffer = buf;
    *out_size = (size_t)sz;
    return 1;
}

/* Materialize a virtual container path into a concrete file under
 * `cache_dir` (or the FSP default user-data asset-cache/csbbin
 * subtree when no explicit cache_dir is given). Returns 1 on success
 * and writes the materialized path through `out_path`. */
static int materialize_virtual(const char *virtual_path,
                               const char *cache_dir,
                               char *out_path,
                               size_t out_path_size)
{
    char default_root[ASSET_PATH_MAX];
    char hcsb_cache[ASSET_PATH_MAX];
    char out_file[ASSET_PATH_MAX];
    const char *use_dir;
    size_t use_dir_len;

    if (!virtual_path || !out_path || out_path_size == 0u) {
        return 0;
    }
    if (cache_dir && cache_dir[0] != '\0') {
        use_dir = cache_dir;
    } else if (FSP_GetUserDataDir(default_root, sizeof(default_root)) &&
               FSP_JoinPath(hcsb_cache, sizeof(hcsb_cache),
                            default_root, "asset-cache/csbbin")) {
        if (!FSP_CreateDirectoryRecursive(hcsb_cache)) {
            return 0;
        }
        use_dir = hcsb_cache;
    } else {
        return 0;
    }
    use_dir_len = strlen(use_dir);
    if (use_dir_len == 0u ||
        use_dir_len + 16u >= sizeof(out_file)) {
        return 0;
    }
    /* Append a deterministic suffix derived from the virtual path so
     * repeated scans don't collide. The buffer uses the last 16 hex
     * chars of a simple FNV-1a-style fold so we don't pull in a new
     * hash dependency just for the cache filename. */
    {
        static const uint32_t FNV_OFFSET = 0x811c9dc5u;
        static const uint32_t FNV_PRIME = 0x01000193u;
        uint32_t hash = FNV_OFFSET;
        const char *p;
        for (p = virtual_path; *p; ++p) {
            hash ^= (uint8_t)(*p);
            hash *= FNV_PRIME;
        }
        snprintf(out_file, sizeof(out_file), "%s/%s-%08x.bin",
                 use_dir, "HCSB", hash);
    }

    if (asset_extract_virtual_path(virtual_path, out_file)) {
        return copy_string(out_path, out_path_size, out_file);
    }
    return 0;
}

/* ── Public scan + load ──────────────────────────────────────────── */

int csb_hint_oracle_htc_real_scan_and_load(
    const char *data_dir,
    const char *cache_dir,
    int max_depth,
    CSB_HintOracleHTC_RealCache *cache)
{
    char resolved_data_dir[ASSET_PATH_MAX];
    char resolved_cache_dir[ASSET_PATH_MAX];
    char match_path[ASSET_PATH_MAX];
    const char *search_root;
    const char *materialize_root;
    const char *read_path;
    char materialized[ASSET_PATH_MAX];
    uint8_t *buf = NULL;
    size_t buf_size = 0u;
    int match_index = -1;
    int parse_rc;
    size_t known_count;
    const CSB_HintOracleHTC_RealKnownHash *known;
    const char *md5_list[8];
    size_t i;

    if (!cache) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT;
    }

    /* Build the md5 list (NULL-terminated) for the scanner. */
    known = csb_hint_oracle_htc_real_known_hashes(&known_count);
    if (known_count == 0u ||
        known_count + 1u > sizeof(md5_list) / sizeof(md5_list[0])) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT;
    }
    for (i = 0u; i < known_count; ++i) {
        md5_list[i] = known[i].md5;
    }
    md5_list[known_count] = NULL;

    /* Resolve data_dir. */
    if (data_dir && data_dir[0] != '\0') {
        if (!copy_string(resolved_data_dir, sizeof(resolved_data_dir),
                         data_dir)) {
            return CSB_HINT_ORACLE_HTC_REAL_ERR_NO_DATA_DIR;
        }
        search_root = resolved_data_dir;
    } else if (FSP_ResolveDataDir(resolved_data_dir,
                                  sizeof(resolved_data_dir), NULL)) {
        search_root = resolved_data_dir;
    } else {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_NO_DATA_DIR;
    }

    /* Resolve cache_dir (only used to materialize virtual paths). */
    if (cache_dir && cache_dir[0] != '\0') {
        if (!copy_string(resolved_cache_dir, sizeof(resolved_cache_dir),
                         cache_dir)) {
            return CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT;
        }
        materialize_root = resolved_cache_dir;
    } else {
        materialize_root = NULL;
    }

    /* Hash-based recursive discovery. */
    if (!asset_find_by_md5_list(search_root, md5_list,
                                match_path, sizeof(match_path),
                                &match_index, max_depth)) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND;
    }

    if (match_index < 0 || (size_t)match_index >= known_count) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND;
    }

    if (is_virtual_path(match_path)) {
        if (!materialize_virtual(match_path, materialize_root,
                                 materialized, sizeof(materialized))) {
            /* We still keep the discovery metadata, but we cannot
             * read it without a cache dir. Return READ so callers
             * can distinguish from a true "not found". */
            copy_string(cache->original_path,
                        sizeof(cache->original_path), match_path);
            copy_string(cache->matched_md5,
                        sizeof(cache->matched_md5),
                        known[match_index].md5);
            copy_string(cache->matched_label,
                        sizeof(cache->matched_label),
                        known[match_index].label);
            return CSB_HINT_ORACLE_HTC_REAL_ERR_READ;
        }
        copy_string(cache->original_path,
                    sizeof(cache->original_path), match_path);
        copy_string(cache->resolved_path,
                    sizeof(cache->resolved_path), materialized);
    } else {
        copy_string(cache->original_path,
                    sizeof(cache->original_path), match_path);
        copy_string(cache->resolved_path,
                    sizeof(cache->resolved_path), match_path);
    }
    copy_string(cache->matched_md5, sizeof(cache->matched_md5),
                known[match_index].md5);
    copy_string(cache->matched_label, sizeof(cache->matched_label),
                known[match_index].label);

    read_path = cache->resolved_path;
    if (!read_whole_file(read_path, &buf, &buf_size)) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_READ;
    }

    parse_rc = csb_hint_oracle_htc_parse(buf, buf_size, &cache->htc);
    if (parse_rc != CSB_HINT_ORACLE_HTC_OK) {
        free(buf);
        return CSB_HINT_ORACLE_HTC_REAL_ERR_PARSE;
    }

    cache->file_buffer = buf;
    cache->file_size = buf_size;
    cache->loaded = 1;
    return CSB_HINT_ORACLE_HTC_REAL_OK;
}

/* ── Lookup helpers ──────────────────────────────────────────────── */

int csb_hint_oracle_htc_real_find_hints_for_location(
    const CSB_HintOracleHTC_RealCache *cache,
    uint8_t level,
    uint8_t x,
    uint8_t y,
    uint16_t *out_indices,
    size_t out_capacity,
    size_t *out_count)
{
    int rc;
    if (!cache || !cache->loaded || !out_count) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED;
    }
    rc = csb_hint_oracle_htc_find_hints_for_location(&cache->htc,
                                                     level, x, y,
                                                     out_indices,
                                                     out_capacity,
                                                     out_count);
    if (rc == CSB_HINT_ORACLE_HTC_OK) {
        return CSB_HINT_ORACLE_HTC_REAL_OK;
    }
    if (rc == CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_OUTPUT_TOO_SMALL;
    }
    return CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED;
}

int csb_hint_oracle_htc_real_get_hint_name(
    const CSB_HintOracleHTC_RealCache *cache,
    size_t hint_index,
    char *buf,
    size_t buf_size)
{
    CSB_HintOracleHTC_Hint hint;
    int rc;
    if (!cache || !cache->loaded || !buf || buf_size == 0u) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT;
    }
    rc = csb_hint_oracle_htc_get_hint(&cache->htc, hint_index, &hint);
    if (rc != CSB_HINT_ORACLE_HTC_OK) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED;
    }
    if (!copy_string(buf, buf_size, hint.name)) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_OUTPUT_TOO_SMALL;
    }
    return CSB_HINT_ORACLE_HTC_REAL_OK;
}

int csb_hint_oracle_htc_real_decompress_first_page(
    const CSB_HintOracleHTC_RealCache *cache,
    size_t hint_index,
    uint8_t *out_buf,
    size_t out_capacity,
    size_t *out_size)
{
    CSB_HintOracleHTC_Hint hint;
    const uint8_t *comp = NULL;
    size_t comp_size = 0u;
    int rc;

    if (!cache || !cache->loaded || !out_buf || !out_size ||
        out_capacity == 0u) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT;
    }
    rc = csb_hint_oracle_htc_get_hint(&cache->htc, hint_index, &hint);
    if (rc != CSB_HINT_ORACLE_HTC_OK) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED;
    }
    rc = csb_hint_oracle_htc_get_hint_content_slice(&cache->htc,
                                                    hint_index,
                                                    &comp, &comp_size);
    if (rc != CSB_HINT_ORACLE_HTC_OK || !comp || comp_size == 0u) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_READ;
    }
    rc = csb_hint_oracle_htc_lzw_decompress(comp, comp_size,
                                           out_buf, out_capacity,
                                           out_size);
    if (rc != CSB_HINT_ORACLE_HTC_OK) {
        return CSB_HINT_ORACLE_HTC_REAL_ERR_PARSE;
    }
    return CSB_HINT_ORACLE_HTC_REAL_OK;
}
