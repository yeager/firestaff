#include "dm1_v1_dos_pc34_real_data_manifest.h"
#include "firestaff_zip_extract.h"

/* The manifest checks are assertions, including calls that produce the
 * source identity inputs.  Keep them enabled under CI's Release build. */
#ifdef NDEBUG
#undef NDEBUG
#endif
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

static void test_real_archive(void) {
    /* The supplied retail ZIP is the media owner.  Read each selected member
     * directly to RAM, rather than requiring an extracted game directory. */
    const char *archive = getenv("FIRESTAFF_DM1_DOS_PC34_ARCHIVE");
    if (!archive || !archive[0]) {
        puts("SKIP: no DM1 DOS PC 3.4 archive");
        return;
    }
    int ok = 0;
    for (int i = 0; i < DM1_V1_DOS_PC34_FILE_COUNT; ++i) {
        uint8_t *bytes = NULL;
        size_t size = 0u;
        /* Manifest paths include DATA/.  The ZIP reader's by-name API is
         * deliberately basename-only, so retain the complete source path
         * through its case-insensitive suffix matcher. */
        if (firestaff_zip_extract_by_suffix(archive, dm1_v1_dos_pc34_files[i].name,
                                            &bytes, &size) != 0 || !bytes) {
            printf("MISS: %s\n", dm1_v1_dos_pc34_files[i].name);
            free(bytes);
            continue;
        }
        if (size == dm1_v1_dos_pc34_files[i].size_bytes) {
            ++ok;
        } else {
            printf("SIZE-MISMATCH: %s (expected %zu, actual %zu)\n",
                dm1_v1_dos_pc34_files[i].name,
                dm1_v1_dos_pc34_files[i].size_bytes, size);
        }
        free(bytes);
    }
    printf("PASS: %d/%d DM1 DOS PC 3.4 ZIP members match manifest size in RAM\n",
        ok, DM1_V1_DOS_PC34_FILE_COUNT);
    assert(ok == DM1_V1_DOS_PC34_FILE_COUNT);
}

int main(void) {
    test_lookup();
    test_matches();
    test_table_uniqueness();
    test_real_archive();
    puts("All dm1_v1_dos_pc34_real_data_manifest tests passed.");
    return 0;
}
