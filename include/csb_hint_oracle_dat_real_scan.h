/* Hash-only, read-only discovery for original CSB Utility Disk HCSB.DAT. */
#ifndef FIRESTAFF_CSB_HINT_ORACLE_DAT_REAL_SCAN_H
#define FIRESTAFF_CSB_HINT_ORACLE_DAT_REAL_SCAN_H

#include "csb_hint_oracle_dat.h"

#define CSB_HINT_ORACLE_DAT_REAL_PATH_CAP 512

typedef enum {
    CSB_HINT_ORACLE_DAT_REAL_OK = 0,
    CSB_HINT_ORACLE_DAT_REAL_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_DAT_REAL_ERR_NOT_FOUND = -2,
    CSB_HINT_ORACLE_DAT_REAL_ERR_READ = -3,
    CSB_HINT_ORACLE_DAT_REAL_ERR_PARSE = -4
} CSB_HintOracleDAT_RealResult;

typedef struct {
    const char *label;
    const char *md5;
} CSB_HintOracleDAT_RealKnownHash;

typedef struct {
    uint8_t *file_buffer;
    size_t file_size;
    char original_path[CSB_HINT_ORACLE_DAT_REAL_PATH_CAP];
    char matched_md5[33];
    int loaded;
    CSB_HintOracleDAT archive;
} CSB_HintOracleDAT_RealCache;

const CSB_HintOracleDAT_RealKnownHash *csb_hint_oracle_dat_real_known_hashes(size_t *count);
void csb_hint_oracle_dat_real_cache_init(CSB_HintOracleDAT_RealCache *cache);
void csb_hint_oracle_dat_real_cache_free(CSB_HintOracleDAT_RealCache *cache);
int csb_hint_oracle_dat_real_scan_and_load(const char *data_dir, int max_depth,
                                           const char *expected_md5,
                                           CSB_HintOracleDAT_RealCache *cache);
const char *csb_hint_oracle_dat_real_result_name(int result);
#endif
