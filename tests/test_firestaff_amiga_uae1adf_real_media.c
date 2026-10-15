#include "asset_status_m12.h"
#include "firestaff_7z_extract.h"
#include "firestaff_amiga_adf.h"
#include "dm1_v1_amiga_title_f0437.h"
#include "dm1_v1_amiga_entrance_f0441.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int file_count;
    unsigned int verified_mask;
    uint8_t *payloads[4];
    size_t sizes[4];
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
        if (files->payloads[i]) return -1;
        files->payloads[i] = malloc(size);
        if (!files->payloads[i]) return -1;
        memcpy(files->payloads[i],bytes,size);
        files->sizes[i] = size;
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
    if (result == 12 && files.file_count == 12U && files.verified_mask == 0x0fU) {
        DM1_V1_AmigaStartupMedia media = {
            files.payloads[0],files.sizes[0], files.payloads[1],files.sizes[1],
            files.payloads[2],files.sizes[2], files.payloads[3],files.sizes[3]};
        DM1_V1_AmigaTitleF0437Receipt title;
        DM1_V1_AmigaEntranceF0441Receipt entrance;
        int admitted = dm1_v1_amiga_startup_media_is_a22g(&media) &&
            dm1_v1_amiga_title_f0437_receipt_for_startup(&media,&title) && title.valid &&
            dm1_v1_amiga_entrance_f0441_receipt_for_startup(&media,&entrance) && entrance.valid;
        int strict_rejects = !dm1_v1_amiga_title_f0437_receipt(
            media.graphics_dat,media.graphics_dat_bytes,
            media.swsh_executable,media.swsh_executable_bytes,&title);
        uint8_t original = files.payloads[2][0];
        files.payloads[2][0] ^= 1;
        int changed_rejects = !dm1_v1_amiga_startup_media_is_a22g(&media) &&
            !dm1_v1_amiga_title_f0437_receipt_for_startup(&media,&title) && !title.valid &&
            !dm1_v1_amiga_entrance_f0441_receipt_for_startup(&media,&entrance) && !entrance.valid;
        files.payloads[2][0] = original;
        media.dungeon_dat = NULL;
        int missing_rejects = !dm1_v1_amiga_startup_media_is_a22g(&media) &&
            !dm1_v1_amiga_title_f0437_receipt_for_startup(&media,&title) && !title.valid;
        result = admitted && strict_rejects && changed_rejects && missing_rejects ? 0 : 1;
    } else result = 1;
    for (unsigned int i = 0; i < 4; ++i) free(files.payloads[i]);
    if (result) {
        fputs("FAIL: authentic A22G startup admission or rejection boundary\n",stderr);
        return 1;
    }
    puts("PASS: original same-volume A22G title/entrance admitted with four authenticated files");
    puts("PASS: legacy pair, modified executable and missing dungeon remain rejected");
    return 0;
}
