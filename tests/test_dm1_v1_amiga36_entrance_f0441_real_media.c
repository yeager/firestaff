#include "dm1_v1_amiga_entrance_f0441.h"
#include "firestaff_amiga_adf.h"
#include "firestaff_zip_extract.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *graphics;
    size_t graphics_bytes;
} SelectedMedia;

static int selected_adf_member(const char *name, const uint8_t *bytes,
                               size_t byte_count, void *user_data) {
    SelectedMedia *media = (SelectedMedia *)user_data;
    static const char graphics_name[] = "graphics.dat";
    size_t index;
    if (!name || !bytes || !media) return -1;
    for (index = 0u; index < sizeof(graphics_name); ++index) {
        if ((unsigned char)tolower((unsigned char)name[index]) !=
            (unsigned char)graphics_name[index]) return 0;
    }
    media->graphics = (uint8_t *)malloc(byte_count);
    if (!media->graphics) return -1;
    memcpy(media->graphics, bytes, byte_count);
    media->graphics_bytes = byte_count;
    return 0;
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA36_ARCHIVE");
    uint8_t *adf = NULL;
    uint8_t *mutated = NULL;
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
                           "Dungeon-Master_Amiga_36.zip", archive);
            archive = default_archive;
        }
    }
    archive_file = archive && archive[0] ? fopen(archive, "rb") : NULL;
    if (!archive_file) {
        puts("SKIP: authentic DM1 Amiga 3.6 archive is not staged");
        return 77;
    }
    fclose(archive_file);
    memset(&media, 0, sizeof(media));
    memset(&receipt, 0, sizeof(receipt));
    if (firestaff_zip_extract_by_suffix(
            archive, "Dungeon Master Amiga 3.6.adf", &adf,
            &adf_bytes) != 0 ||
        firestaff_amiga_adf_visit_ofs_files(adf, adf_bytes,
                                            selected_adf_member,
                                            &media) < 0 ||
        !media.graphics) {
        fprintf(stderr, "FAIL: could not read authentic A36 GRAPHICS.DAT\n");
        goto cleanup;
    }
    if (!dm1_v1_amiga36_entrance_f0441_receipt(
            media.graphics, media.graphics_bytes, &receipt)) {
        fprintf(stderr, "FAIL: authentic A36 F0441 receipt rejected\n");
        goto cleanup;
    }
    if (strcmp(receipt.graphics_md5,
               "7f9458e4a3972d06e649a6fa85a7f34b") != 0 ||
        receipt.executable_md5[0] != '\0' ||
        receipt.screen_width != 320u || receipt.screen_height != 200u ||
        receipt.left_door_width != 105u ||
        receipt.left_door_height != 161u ||
        receipt.right_door_width != 128u ||
        receipt.right_door_height != 161u ||
        receipt.screen_nonzero_pixels == 0u ||
        receipt.left_door_nonzero_pixels == 0u ||
        receipt.right_door_nonzero_pixels == 0u ||
        receipt.door_frame_count != 8u || receipt.opening_steps != 31u ||
        receipt.switch_delay_ticks != 20u || receipt.mouse_input_only ||
        !receipt.has_credits_graphic ||
        !receipt.has_entrance_buttons_graphic ||
        receipt.entrance_palette_rgb4[5] != 0x0c0u ||
        receipt.entrance_palette_rgb4[15] != 0xfffu ||
        !receipt.source_evidence ||
        strstr(receipt.source_evidence, "STARTUP2.C") == NULL ||
        strstr(receipt.source_evidence, "COMMAND.C") == NULL) {
        fprintf(stderr, "FAIL: A36 entrance receipt facts mismatch\n");
        goto cleanup;
    }
    mutated = (uint8_t *)malloc(media.graphics_bytes);
    if (!mutated) {
        fprintf(stderr, "FAIL: allocate mutation buffer\n");
        goto cleanup;
    }
    memcpy(mutated, media.graphics, media.graphics_bytes);
    mutated[media.graphics_bytes - 1u] ^= 1u;
    if (dm1_v1_amiga36_entrance_f0441_receipt(
            mutated, media.graphics_bytes, &receipt)) {
        fprintf(stderr, "FAIL: accepted modified A36 GRAPHICS.DAT\n");
        goto cleanup;
    }
    puts("ok: A36 F0441 receipt authenticates original C002/C003/C004/C005/C011 media and keyboard input");
    result = 0;

cleanup:
    free(mutated);
    free(media.graphics);
    free(adf);
    return result;
}
