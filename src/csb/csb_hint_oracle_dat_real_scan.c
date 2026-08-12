#include "csb_hint_oracle_dat_real_scan.h"
#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const CSB_HintOracleDAT_RealKnownHash g_known[] = {
    {"CSB Utility Disk English R1/RX/ST 2.0", "708e113c869ab922633e885aa72a3c77"},
    {"CSB Utility Disk English R2/R3", "7496b3b8b9ff6e2368eac9a16be8230b"},
    {"CSB Utility Disk French", "bbf3ada2da9722577feea4fa213b32f1"},
    {"CSB Utility Disk German", "9e0da6c5a569859c6191201dcc6e6aae"}
};
const CSB_HintOracleDAT_RealKnownHash *csb_hint_oracle_dat_real_known_hashes(size_t *count) { if (count) *count = sizeof(g_known) / sizeof(g_known[0]); return g_known; }
void csb_hint_oracle_dat_real_cache_init(CSB_HintOracleDAT_RealCache *c) { if (c) memset(c, 0, sizeof(*c)); }
void csb_hint_oracle_dat_real_cache_free(CSB_HintOracleDAT_RealCache *c) { if (c) { free(c->file_buffer); memset(c, 0, sizeof(*c)); } }
const char *csb_hint_oracle_dat_real_result_name(int r) { switch (r) { case 0: return "OK"; case -1: return "argument"; case -2: return "not-found"; case -3: return "read"; case -4: return "parse"; default: return "unknown"; } }

static int csb_hint_oracle_dat_read_path(const char *path, uint8_t **out,
                                         size_t *out_size)
{
    char user_data[ASSET_PATH_MAX];
    char cache_dir[ASSET_PATH_MAX];
    char cache_path[ASSET_PATH_MAX];
    unsigned long hash = 5381UL;
    const unsigned char *cursor;
    if (!path || !out || !out_size) return 0;
    if (!strstr(path, "::")) return asset_read_path_alloc(path, out, out_size);
    for (cursor = (const unsigned char *)path; *cursor; ++cursor)
        hash = ((hash << 5) + hash) ^ *cursor;
    if (!FSP_GetUserDataDir(user_data, sizeof(user_data)) ||
        !FSP_JoinPath(cache_dir, sizeof(cache_dir), user_data,
                      "asset-cache/csbbin") ||
        !FSP_CreateDirectoryRecursive(cache_dir) ||
        snprintf(cache_path, sizeof(cache_path), "%s/HCSBDAT-%08lx.bin",
                 cache_dir, hash) >= (int)sizeof(cache_path) ||
        !asset_extract_virtual_path(path, cache_path)) return 0;
    return asset_read_path_alloc(cache_path, out, out_size);
}

int csb_hint_oracle_dat_real_scan_and_load(const char *root, int depth, const char *expected, CSB_HintOracleDAT_RealCache *c) {
    const char *list[5]; char path[ASSET_PATH_MAX]; int match = -1, rc; size_t i, n;
    if (!root || !root[0] || !c) return CSB_HINT_ORACLE_DAT_REAL_ERR_ARGUMENT;
    n = sizeof(g_known) / sizeof(g_known[0]);
    if (expected && expected[0]) { for (i = 0; i < n && strcmp(expected, g_known[i].md5); ++i) {} if (i == n) return CSB_HINT_ORACLE_DAT_REAL_ERR_ARGUMENT; list[0] = g_known[i].md5; list[1] = NULL; }
    else { for (i = 0; i < n; ++i) list[i] = g_known[i].md5; list[n] = NULL; }
    if (!asset_find_by_md5_list(root, list, path, sizeof(path), expected && expected[0] ? NULL : &match, depth)) return CSB_HINT_ORACLE_DAT_REAL_ERR_NOT_FOUND;
    if (expected && expected[0]) { for (i = 0; i < n && strcmp(expected, g_known[i].md5); ++i) {} match = (int)i; }
    if (match < 0 || (size_t)match >= n) return CSB_HINT_ORACLE_DAT_REAL_ERR_NOT_FOUND;
    if (!csb_hint_oracle_dat_read_path(path, &c->file_buffer, &c->file_size)) return CSB_HINT_ORACLE_DAT_REAL_ERR_READ;
    rc = csb_hint_oracle_dat_parse(c->file_buffer, c->file_size, &c->archive);
    if (rc) { free(c->file_buffer); c->file_buffer = NULL; return CSB_HINT_ORACLE_DAT_REAL_ERR_PARSE; }
    strncpy(c->original_path, path, sizeof(c->original_path) - 1u);
    strncpy(c->matched_md5, g_known[match].md5, 32u);
    c->loaded = 1;
    return CSB_HINT_ORACLE_DAT_REAL_OK;
}
