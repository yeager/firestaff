#include "csb_hint_oracle_dat_real_scan.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    const char *root = argc > 1 ? argv[1] : getenv("FIRESTAFF_CSB_HCSB_DAT_DATA");
    CSB_HintOracleDAT_RealCache cache;
    int rc;
    if (!root || !root[0]) { puts("SKIP: set FIRESTAFF_CSB_HCSB_DAT_DATA."); return 0; }
    csb_hint_oracle_dat_real_cache_init(&cache);
    rc = csb_hint_oracle_dat_real_scan_and_load(root, 6, NULL, &cache);
    if (rc == CSB_HINT_ORACLE_DAT_REAL_ERR_NOT_FOUND) { puts("SKIP: no registered HCSB.DAT."); return 0; }
    if (rc != CSB_HINT_ORACLE_DAT_REAL_OK || !cache.loaded || cache.archive.segment_count == 0u) { fprintf(stderr, "FAIL: %s\n", csb_hint_oracle_dat_real_result_name(rc)); csb_hint_oracle_dat_real_cache_free(&cache); return 1; }
    printf("HCSB.DAT md5=%s segments=%u path=%s\n", cache.matched_md5, (unsigned)cache.archive.segment_count, cache.original_path);
    csb_hint_oracle_dat_real_cache_free(&cache);
    return 0;
}
