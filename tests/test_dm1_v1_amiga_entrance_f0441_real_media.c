#include "dm1_v1_amiga_entrance_f0441.h"
#include "firestaff_amiga_adf.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *graphics;
    size_t graphics_bytes;
    uint8_t *swsh;
    size_t swsh_bytes;
} SelectedMedia;

static int selected_adf_member(const char *name, const uint8_t *bytes,
                               size_t byte_count, void *user_data) {
    SelectedMedia *media = (SelectedMedia *)user_data;
    uint8_t **destination = NULL;
    size_t *destination_bytes = NULL;
    if (!name || !bytes || !media) return -1;
    if (strcmp(name, "graphics.dat") == 0) {
        destination = &media->graphics;
        destination_bytes = &media->graphics_bytes;
    } else if (strcmp(name, "swoosh") == 0) {
        destination = &media->swsh;
        destination_bytes = &media->swsh_bytes;
    }
    if (!destination) return 0;
    *destination = (uint8_t *)malloc(byte_count);
    if (!*destination) return -1;
    memcpy(*destination, bytes, byte_count);
    *destination_bytes = byte_count;
    return 0;
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    uint8_t *inner_zip = NULL;
    uint8_t *adf = NULL;
    uint8_t *mutated = NULL;
    size_t inner_zip_bytes = 0u;
    size_t adf_bytes = 0u;
    SelectedMedia media;
    DM1_V1_AmigaEntranceF0441Receipt receipt;
    FILE *archive_file;
    int result = 1;

    if (!archive || !archive[0]) {
        archive = getenv("HOME");
        if (archive && archive[0]) {
            static char default_archive[1024];
            (void)snprintf(default_archive, sizeof(default_archive),
                           "%s/.firestaff/data/dm1/"
                           "Dungeon-Master_Amiga_EN_Version-20.zip",
                           archive);
            archive = default_archive;
        }
    }
    archive_file = archive && archive[0] ? fopen(archive, "rb") : NULL;
    if (!archive_file) {
        puts("SKIP: authentic DM1 Amiga v2.0 archive is not staged");
        return 77;
    }
    fclose(archive_file);
    memset(&media, 0, sizeof(media));
    memset(&receipt, 0, sizeof(receipt));
    if (firestaff_zip_extract_by_suffix(
            archive, "Dungeon Master v2.0 (1988)(FTL).zip",
            &inner_zip, &inner_zip_bytes) != 0 ||
        firestaff_zip_extract_memory_by_suffix(
            inner_zip, inner_zip_bytes,
            "Dungeon Master v2.0 (1988)(FTL).adf", &adf,
            &adf_bytes) != 0 ||
        firestaff_amiga_adf_visit_ofs_files(adf, adf_bytes,
                                            selected_adf_member,
                                            &media) < 0 ||
        !media.graphics || !media.swsh) {
        fprintf(stderr, "FAIL: could not read authentic selected A20 ADF\n");
        goto cleanup;
    }
    if (!dm1_v1_amiga_entrance_f0441_receipt(
            media.graphics, media.graphics_bytes, media.swsh,
            media.swsh_bytes, &receipt)) {
        fprintf(stderr, "FAIL: authentic A20 F0441 receipt rejected\n");
        goto cleanup;
    }
    if (strcmp(receipt.graphics_md5,
               "6a2f135b53c2220f0251fa103e2a6e7e") != 0 ||
        strcmp(receipt.executable_md5,
               "a0ffbcc7ae8cecac03128ddb32887ef4") != 0 ||
        receipt.screen_width != 320u || receipt.screen_height != 200u ||
        receipt.left_door_width != 128u ||
        receipt.left_door_height != 161u ||
        receipt.right_door_width != 128u ||
        receipt.right_door_height != 161u ||
        receipt.screen_nonzero_pixels == 0u ||
        receipt.left_door_nonzero_pixels == 0u ||
        receipt.right_door_nonzero_pixels == 0u ||
        receipt.door_frame_count != 8u || receipt.opening_steps != 31u ||
        receipt.switch_delay_ticks != 20u || !receipt.mouse_input_only ||
        receipt.entrance_palette_rgb4[5] != 0x0c0u ||
        receipt.entrance_palette_rgb4[15] != 0xfffu) {
        fprintf(stderr, "FAIL: A20 entrance receipt facts mismatch\n");
        goto cleanup;
    }
    mutated = (uint8_t *)malloc(media.graphics_bytes);
    if (!mutated) {
        fprintf(stderr, "FAIL: allocate mutation buffer\n");
        goto cleanup;
    }
    memcpy(mutated, media.graphics, media.graphics_bytes);
    mutated[media.graphics_bytes - 1u] ^= 1u;
    if (dm1_v1_amiga_entrance_f0441_receipt(
            mutated, media.graphics_bytes, media.swsh, media.swsh_bytes,
            &receipt)) {
        fprintf(stderr, "FAIL: accepted modified A20 GRAPHICS.DAT\n");
        goto cleanup;
    }
    puts("ok: A20 F0441 receipt authenticates selected C002/C003/C004 media and input/palette facts");
    result = 0;

cleanup:
    free(mutated);
    free(media.graphics);
    free(media.swsh);
    free(adf);
    free(inner_zip);
    return result;
}
