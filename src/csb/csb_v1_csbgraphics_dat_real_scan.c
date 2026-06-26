/*
 * csb_v1_csbgraphics_dat_real_scan.c
 *
 * Real-asset scanner + cached handoff for a CSBWin "CSBgraphics.dat"
 * custom graphics override file. See
 * include/csb_v1_csbgraphics_dat_real_scan.h for scope and source
 * references.
 *
 * The implementation is deliberately small and stays close to the
 * existing HCSB.HTC real-scan template (csb_hint_oracle_htc_real_scan.c):
 *
 *   - known MD5 list is the only source of "what we accept"
 *   - asset_find_by_md5_list() does the recursive, virtual-aware
 *     discovery (it already handles ZIP/ISO entries)
 *   - asset_extract_virtual_path() materializes a virtual path
 *     into a real file when the caller provides a cache_dir
 *   - the classifier in csb_v1_csbgraphics_dat_classify.c owns
 *     the format contract and remains the single source of truth
 *     for what the file's tables actually mean
 *
 * Nothing in this file reaches into the CSB runtime, the launcher
 * UI, or the viewport drawing path. That remains tracked
 * separately under docs/FIRESTAFF_GAP_LIST.md row C3 / A3.
 */

#include "csb_v1_csbgraphics_dat_real_scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"

/* ── Known MD5 list ──────────────────────────────────────────────── */

/* Intentionally empty by default. CSBWin "CSBgraphics.dat" is a
 * CSBGraffer / CSBWin Viewport Compiler product, not an original
 * CSB asset; there is no canonical reference hash to gate
 * discovery on until a user stages a real file. Operators with
 * a real CSBgraphics.dat can extend this list locally.
 *
 * Format mirrors csb_hint_oracle_htc_real_known_hashes() so the
 * hash-list API stays uniform across real-asset modules.
 */
static const CSB_V1_CSBGraphicsDatRealKnownHash g_known_hashes[] = {
    /* Intentionally empty in the public build.
     *
     * CSBgraphics.dat is a CSBGraffer / CSBWin Viewport Compiler
     * product, not an original CSB asset; there is no canonical
     * reference hash to gate discovery on until a user stages a
     * real file under ~/.firestaff/data/csbwin-custom/<label>/.
     * Operators with a real CSBWin-produced CSBgraphics.dat can
     * extend this list locally. Format:
     *   { "<label>", "<md5-hex>", <size_bytes> },
     *
     * The trailing sentinel stays so the table compiles under
     * strict -std=c99 -Werror without relying on the empty
     * initializer extension. known_hashes() reports the count
     * excluding the sentinel so the scanner still sees an empty
     * list and SKIPs cleanly.
     */
    { "", "", 0u }, /* sentinel — never matched */
};

const CSB_V1_CSBGraphicsDatRealKnownHash *
csb_v1_csbgraphics_dat_real_known_hashes(size_t *out_count)
{
    /* The table always carries a trailing sentinel entry whose
     * label/md5 are empty. Strip it from the reported count so
     * the scanner sees an empty list and SKIPs cleanly. */
    size_t total = sizeof(g_known_hashes) / sizeof(g_known_hashes[0]);
    size_t real_count = 0u;
    size_t i;
    for (i = 0u; i < total; ++i) {
        if (g_known_hashes[i].label && g_known_hashes[i].label[0] != '\0' &&
            g_known_hashes[i].md5 && g_known_hashes[i].md5[0] != '\0') {
            ++real_count;
        }
    }
    if (out_count) {
        *out_count = real_count;
    }
    return g_known_hashes;
}

/* ── Result-name table ───────────────────────────────────────────── */

const char *csb_v1_csbgraphics_dat_real_result_name(int result)
{
    switch (result) {
    case CSB_V1_CSBGRAPHICS_DAT_REAL_OK: return "OK";
    case CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_ARGUMENT: return "argument";
    case CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NO_DATA_DIR: return "no-data-dir";
    case CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND: return "not-found";
    case CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_READ: return "read";
    case CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_PARSE: return "parse";
    default: return "unknown";
    }
}

/* ── Cache lifecycle ─────────────────────────────────────────────── */

void csb_v1_csbgraphics_dat_real_cache_init(
    CSB_V1_CSBGraphicsDatRealCache *cache)
{
    if (!cache) {
        return;
    }
    memset(cache, 0, sizeof(*cache));
}

void csb_v1_csbgraphics_dat_real_cache_free(
    CSB_V1_CSBGraphicsDatRealCache *cache)
{
    if (!cache) {
        return;
    }
    if (cache->file_buffer) {
        free(cache->file_buffer);
    }
    memset(cache, 0, sizeof(*cache));
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
 * `cache_dir` (or the FSP default user-data asset-cache/csbwingraphics
 * subtree when no explicit cache_dir is given). Returns 1 on
 * success and writes the materialized path through `out_path`.
 */
static int materialize_virtual(const char *virtual_path,
                               const char *cache_dir,
                               char *out_path,
                               size_t out_path_size)
{
    char default_root[ASSET_PATH_MAX];
    char csb_cache[ASSET_PATH_MAX];
    char out_file[ASSET_PATH_MAX];
    const char *use_dir;
    size_t use_dir_len;

    if (!virtual_path || !out_path || out_path_size == 0u) {
        return 0;
    }
    if (cache_dir && cache_dir[0] != '\0') {
        use_dir = cache_dir;
    } else if (FSP_GetUserDataDir(default_root, sizeof(default_root)) &&
               FSP_JoinPath(csb_cache, sizeof(csb_cache),
                            default_root,
                            "asset-cache/csbwingraphics")) {
        if (!FSP_CreateDirectoryRecursive(csb_cache)) {
            return 0;
        }
        use_dir = csb_cache;
    } else {
        return 0;
    }
    use_dir_len = strlen(use_dir);
    if (use_dir_len == 0u ||
        use_dir_len + 32u >= sizeof(out_file)) {
        return 0;
    }
    /* Append a deterministic suffix derived from the virtual path so
     * repeated scans don't collide. Same FNV-1a fold used by the
     * HCSB.HTC scanner.
     */
    {
        static const uint32_t FNV_OFFSET = 0x811c9dc5u;
        static const uint32_t FNV_PRIME = 0x01000193u;
        uint32_t hash = FNV_OFFSET;
        const char *p;
        for (p = virtual_path; *p; ++p) {
            hash ^= (uint8_t)(*p);
            hash *= FNV_PRIME;
        }
        snprintf(out_file, sizeof(out_file), "%s/%s-%08x.dat",
                 use_dir, "CSBGRAPH", hash);
    }

    if (asset_extract_virtual_path(virtual_path, out_file)) {
        return copy_string(out_path, out_path_size, out_file);
    }
    return 0;
}

/* ── Public scan + load ──────────────────────────────────────────── */

int csb_v1_csbgraphics_dat_real_scan_and_load(
    const char *data_dir,
    const char *cache_dir,
    int max_depth,
    CSB_V1_CSBGraphicsDatRealCache *cache)
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
    const CSB_V1_CSBGraphicsDatRealKnownHash *known;
    const char *md5_list[8];
    size_t i;

    if (!cache) {
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_ARGUMENT;
    }

    /* Build the md5 list (NULL-terminated) for the scanner. */
    known = csb_v1_csbgraphics_dat_real_known_hashes(&known_count);
    if (known_count == 0u) {
        /* Empty list = no canonical hash known yet. Treat as
         * "not found" so probes SKIP cleanly rather than fail. */
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND;
    }
    if (known_count + 1u > sizeof(md5_list) / sizeof(md5_list[0])) {
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_ARGUMENT;
    }
    for (i = 0u; i < known_count; ++i) {
        md5_list[i] = known[i].md5;
    }
    md5_list[known_count] = NULL;

    /* Resolve data_dir. */
    if (data_dir && data_dir[0] != '\0') {
        if (!copy_string(resolved_data_dir, sizeof(resolved_data_dir),
                         data_dir)) {
            return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NO_DATA_DIR;
        }
        search_root = resolved_data_dir;
    } else if (FSP_ResolveDataDir(resolved_data_dir,
                                  sizeof(resolved_data_dir), NULL)) {
        search_root = resolved_data_dir;
    } else {
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NO_DATA_DIR;
    }

    /* Resolve cache_dir (only used to materialize virtual paths). */
    if (cache_dir && cache_dir[0] != '\0') {
        if (!copy_string(resolved_cache_dir, sizeof(resolved_cache_dir),
                         cache_dir)) {
            return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_ARGUMENT;
        }
        materialize_root = resolved_cache_dir;
    } else {
        materialize_root = NULL;
    }

    /* Hash-based recursive discovery. */
    if (!asset_find_by_md5_list(search_root, md5_list,
                                match_path, sizeof(match_path),
                                &match_index, max_depth)) {
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND;
    }

    if (match_index < 0 || (size_t)match_index >= known_count) {
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND;
    }

    if (is_virtual_path(match_path)) {
        if (!materialize_virtual(match_path, materialize_root,
                                 materialized, sizeof(materialized))) {
            copy_string(cache->original_path,
                        sizeof(cache->original_path), match_path);
            copy_string(cache->matched_md5,
                        sizeof(cache->matched_md5),
                        known[match_index].md5);
            copy_string(cache->matched_label,
                        sizeof(cache->matched_label),
                        known[match_index].label);
            return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_READ;
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
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_READ;
    }

    parse_rc = csb_v1_csbgraphics_dat_classify(buf, buf_size, &cache->index);
    if (parse_rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK) {
        free(buf);
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_PARSE;
    }

    cache->file_buffer = buf;
    cache->file_size = buf_size;
    cache->loaded = 1;
    return CSB_V1_CSBGRAPHICS_DAT_REAL_OK;
}

int csb_v1_csbgraphics_dat_real_index(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    CSB_V1_CSBGraphicsIndex *out_index)
{
    if (!cache || !out_index) {
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_ARGUMENT;
    }
    if (!cache->loaded) {
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_READ;
    }
    *out_index = cache->index;
    return CSB_V1_CSBGRAPHICS_DAT_REAL_OK;
}
