#include "dm1_v1_dos_pc34_real_data_manifest.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_lookup(void) {
    const dm1_v1_dos_pc34_file_fp_t *e;
    e = dm1_v1_dos_pc34_file_fp_lookup_pc34("DM.EXE");
    assert(e != NULL);
    assert(e->size_bytes == 11471u);
    e = dm1_v1_dos_pc34_file_fp_lookup_pc34("DATA/GRAPHICS.DAT");
    assert(e != NULL);
    assert(e->size_bytes == 363417u);
    e = dm1_v1_dos_pc34_file_fp_lookup_pc34("DATA/DUNGEON.DAT");
    assert(e != NULL);
    assert(e->size_bytes == 33357u);
    assert(dm1_v1_dos_pc34_file_fp_lookup_pc34("does-not-exist") == NULL);
    assert(dm1_v1_dos_pc34_file_fp_lookup_pc34(NULL) == NULL);
}

static void test_matches(void) {
    const dm1_v1_dos_pc34_file_fp_t *e =
        dm1_v1_dos_pc34_file_fp_lookup_pc34("DM.EXE");
    assert(e != NULL);
    assert(dm1_v1_dos_pc34_file_fp_matches_pc34(
        "DM.EXE", e->size_bytes, e->sha256) == 1);
    /* Wrong size rejected. */
    assert(dm1_v1_dos_pc34_file_fp_matches_pc34(
        "DM.EXE", e->size_bytes + 1u, e->sha256) == 0);
    /* Wrong digest rejected. */
    uint8_t bogus[32] = {0};
    assert(dm1_v1_dos_pc34_file_fp_matches_pc34(
        "DM.EXE", e->size_bytes, bogus) == 0);
    /* NULL digest rejected. */
    assert(dm1_v1_dos_pc34_file_fp_matches_pc34(
        "DM.EXE", e->size_bytes, NULL) == 0);
    /* Unknown filename rejected. */
    assert(dm1_v1_dos_pc34_file_fp_matches_pc34(
        "OTHER", e->size_bytes, e->sha256) == 0);
}

static void test_table_uniqueness(void) {
    /* No duplicate filenames and no duplicate SHA-256 digests. */
    for (int i = 0; i < DM1_V1_DOS_PC34_FILE_COUNT; ++i) {
        for (int j = i + 1; j < DM1_V1_DOS_PC34_FILE_COUNT; ++j) {
            assert(strcmp(dm1_v1_dos_pc34_files[i].name,
                          dm1_v1_dos_pc34_files[j].name) != 0);
            assert(memcmp(dm1_v1_dos_pc34_files[i].sha256,
                          dm1_v1_dos_pc34_files[j].sha256, 32) != 0);
        }
    }
}

static void test_real_disc(void) {
    /* Opt-in: sizes are checked against shipping disc when
     * FIRESTAFF_DM1_DOS_PC34_ROOT is provided. Uses only stat/read
     * plus size comparison; SHA-256 verification is done by the
     * caller who already has a hash implementation. */
    const char *root = getenv("FIRESTAFF_DM1_DOS_PC34_ROOT");
    if (!root) { puts("SKIP: no DM1 DOS PC 3.4 root"); return; }
    int ok = 0;
    for (int i = 0; i < DM1_V1_DOS_PC34_FILE_COUNT; ++i) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", root,
                 dm1_v1_dos_pc34_files[i].name);
        FILE *fp = fopen(path, "rb");
        if (!fp) { printf("MISS: %s\n", dm1_v1_dos_pc34_files[i].name); continue; }
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        fclose(fp);
        if ((size_t)sz == dm1_v1_dos_pc34_files[i].size_bytes) {
            ++ok;
        } else {
            printf("SIZE-MISMATCH: %s (expected %zu, actual %ld)\n",
                dm1_v1_dos_pc34_files[i].name,
                dm1_v1_dos_pc34_files[i].size_bytes, sz);
        }
    }
    printf("PASS: %d/%d DM1 DOS PC 3.4 files match manifest size\n",
        ok, DM1_V1_DOS_PC34_FILE_COUNT);
    assert(ok == DM1_V1_DOS_PC34_FILE_COUNT);
}

int main(void) {
    test_lookup();
    test_matches();
    test_table_uniqueness();
    test_real_disc();
    puts("All dm1_v1_dos_pc34_real_data_manifest tests passed.");
    return 0;
}
