#include "asset_status_m12.h"
#include "firestaff_7z_extract.h"
#include "firestaff_amiga_adf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int file_count;
    unsigned int verified_mask;
} A22GFiles;

static int verify_file(const char *name, const uint8_t *bytes, size_t size,
                       void *user_data) {
    static const char *const file_names[] = {
        "graphics.dat", "Dungeon.dat", "dm", "swoosh"
    };
    static const size_t file_sizes[] = {399581U, 25380U, 177352U, 78568U};
    static const char *const file_md5[] = {
        "0679e39da9dcc2e855cb33c6c64ddcb5",
        "59e332cf47852d5c7c88a9538f0adcea",
        "e8fc5da52d69aa61b8b5ac7b2e294ebd",
        "1038138978975415571a878bb08f54be"
    };
    A22GFiles *files = (A22GFiles *)user_data;
    char md5[33];
    unsigned int i;
    if (!name || !bytes || !files || size == 0U) return -1;
    ++files->file_count;
    for (i = 0U; i < sizeof(file_names) / sizeof(file_names[0]); ++i) {
        if (strcmp(name, file_names[i]) != 0) continue;
        if (size != file_sizes[i] ||
            !m12_bytes_md5_hex(bytes, size, md5) ||
            strcmp(md5, file_md5[i]) != 0) return -1;
        files->verified_mask |= 1U << i;
        break;
    }
    return 0;
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA_SOFTWARE_ARCHIVE");
    const char *member =
        "Floppy Disks ADF/Dungeon Master for Amiga v2.2 (German) Original (Not working).adf";
    uint8_t *image = NULL;
    size_t image_size = 0U;
    A22GFiles files;
    int result;
    if (!archive || !archive[0]) {
        puts("SKIP: authentic DM1 Amiga software archive is not configured");
        return 77;
    }
    if (!firestaff_7z_read_member(archive, member, &image, &image_size)) {
        puts("SKIP: authentic DM1 Amiga v2.2 German disk is unavailable");
        return 77;
    }
    if (image_size != 910058U) {
        free(image);
        puts("FAIL: unexpected authentic UAE-1ADF disk size");
        return 1;
    }
    memset(&files, 0, sizeof(files));
    result = firestaff_amiga_adf_visit_ofs_files(
        image, image_size, verify_file, &files);
    free(image);
    if (result != 12 || files.file_count != 12U || files.verified_mask != 0x0fU) {
        fprintf(stderr,
                "FAIL: UAE-1ADF OFS files result=%d visited=%u verified=%x\n",
                result, files.file_count, files.verified_mask);
        return 1;
    }
    puts("PASS: authentic A22G UAE-1ADF yields original OFS files and identities");
    puts("NOTE: its SWSH is the French A20F asset; this test does not admit A22G");
    return 0;
}
