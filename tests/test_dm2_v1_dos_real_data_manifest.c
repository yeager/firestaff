#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_dos_startup_media.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    /* Sanity lookups. */
    const dm2_v1_dos_file_fp_t *e = dm2_v1_dos_file_fp_lookup_pc34("skull.exe");
    assert(e != NULL);
    assert(e->size_bytes == 522637u);
    assert(dm2_v1_dos_file_fp_lookup_pc34("does-not-exist") == NULL);
    assert(dm2_v1_dos_file_fp_lookup_pc34(NULL) == NULL);

    /* Match/mismatch. */
    assert(dm2_v1_dos_file_fp_matches_pc34("skull.exe",
        e->size_bytes, e->sha256) == 1);
    assert(dm2_v1_dos_file_fp_matches_pc34("skull.exe",
        e->size_bytes + 1u, e->sha256) == 0);
    uint8_t bogus[32] = {0};
    assert(dm2_v1_dos_file_fp_matches_pc34("skull.exe",
        e->size_bytes, bogus) == 0);
    assert(dm2_v1_dos_file_fp_matches_pc34("skull.exe",
        e->size_bytes, NULL) == 0);

    /* Uniqueness: no duplicate names or digests. */
    for (int i = 0; i < DM2_V1_DOS_FILE_COUNT; ++i) {
        for (int j = i + 1; j < DM2_V1_DOS_FILE_COUNT; ++j) {
            assert(strcmp(dm2_v1_dos_files[i].name,
                          dm2_v1_dos_files[j].name) != 0);
            assert(memcmp(dm2_v1_dos_files[i].sha256,
                          dm2_v1_dos_files[j].sha256, 32) != 0);
        }
    }

    /* Real-data size check when a root is provided. */
    const char *root = getenv("FIRESTAFF_DM2_DOS_ROOT");
    if (!root) { puts("SKIP: no DM2 DOS root"); goto done; }
    int ok = 0;
    for (int i = 0; i < DM2_V1_DOS_FILE_COUNT; ++i) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", root,
                 dm2_v1_dos_files[i].name);
        FILE *fp = fopen(path, "rb");
        if (!fp) { printf("MISS: %s\n", dm2_v1_dos_files[i].name); continue; }
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        fclose(fp);
        if ((size_t)sz == dm2_v1_dos_files[i].size_bytes) ++ok;
        else printf("SIZE-MISMATCH: %s (expected %zu, actual %ld)\n",
            dm2_v1_dos_files[i].name, dm2_v1_dos_files[i].size_bytes, sz);
    }
    printf("PASS: %d/%d DM2 DOS files match manifest size\n",
        ok, DM2_V1_DOS_FILE_COUNT);
    assert(ok == DM2_V1_DOS_FILE_COUNT);
    {
        DM2_V1_DosStartupMediaReceipt startup;
        assert(dm2_v1_dos_startup_media_probe(root, &startup) == 1);
        assert(startup.valid && startup.complete &&
               startup.batch_dispatches_ibmiop && startup.ibmiop_verified &&
               startup.splash_verified && startup.ftl_verified &&
               startup.intro_verified && startup.end_verified &&
               startup.intrplay_pcx_verified &&
               startup.intro_has_interplay_mve && startup.end_has_interplay_mve &&
               startup.intro_mve_header_offset > 0u &&
               startup.end_mve_header_offset > 0u && startup.receipt_hash != 0u);
        puts("PASS: DM2 DOS IBMIOP/MVE startup route matches retail media");
    }

done:
    puts("All dm2_v1_dos_real_data_manifest tests passed.");
    return 0;
}
