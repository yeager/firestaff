#include "dm1_v1_amiga_graphics_dat.h"
#include "dm1_v1_amiga_title_f0437.h"
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

static int selected_adf_member(const char *name,
                               const uint8_t *bytes,
                               size_t byte_count,
                               void *user_data) {
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
    DM1_V1_AmigaTitleF0437Receipt receipt;
    DM1_V1_AmigaTitleF0437ZoomStep step;
    FILE *archive_file;
    unsigned int index;
    int failures = 0;

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
                                            &media) < 0) {
        fprintf(stderr, "FAIL: could not read authentic nested ZIP -> ADF\n");
        ++failures;
        goto done;
    }
    if (!media.graphics || !media.swsh) {
        fprintf(stderr, "FAIL: selected ADF lacks GRAPHICS.DAT or swoosh\n");
        ++failures;
        goto done;
    }
    if (!dm1_v1_amiga_title_f0437_receipt(
            media.graphics, media.graphics_bytes, media.swsh,
            media.swsh_bytes, &receipt)) {
        fprintf(stderr, "FAIL: authentic A20 F0437 receipt rejected\n");
        ++failures;
        goto done;
    }
    mutated = (uint8_t *)malloc(media.graphics_bytes);
    if (!mutated) {
        fprintf(stderr, "FAIL: allocate negative-test buffer\n");
        ++failures;
        goto done;
    }
    memcpy(mutated, media.graphics, media.graphics_bytes);
    mutated[media.graphics_bytes - 1u] ^= 1u;
    if (dm1_v1_amiga_title_f0437_receipt(
            mutated, media.graphics_bytes, media.swsh, media.swsh_bytes,
            &receipt)) {
        fprintf(stderr, "FAIL: accepted modified GRAPHICS.DAT identity\n");
        ++failures;
        goto done;
    }
    free(mutated);
    mutated = (uint8_t *)malloc(media.swsh_bytes);
    if (!mutated) {
        fprintf(stderr, "FAIL: allocate SWSH negative-test buffer\n");
        ++failures;
        goto done;
    }
    memcpy(mutated, media.swsh, media.swsh_bytes);
    mutated[media.swsh_bytes - 1u] ^= 1u;
    if (dm1_v1_amiga_title_f0437_receipt(
            media.graphics, media.graphics_bytes, mutated,
            media.swsh_bytes, &receipt)) {
        fprintf(stderr, "FAIL: accepted modified A20 executable identity\n");
        ++failures;
        goto done;
    }
    if (!dm1_v1_amiga_title_f0437_receipt(
            media.graphics, media.graphics_bytes, media.swsh,
            media.swsh_bytes, &receipt)) {
        fprintf(stderr, "FAIL: authentic A20 receipt failed after rejects\n");
        ++failures;
        goto done;
    }
    if (strcmp(receipt.graphics_md5,
               "6a2f135b53c2220f0251fa103e2a6e7e") != 0 ||
        strcmp(receipt.executable_md5,
               "a0ffbcc7ae8cecac03128ddb32887ef4") != 0 ||
        receipt.c001_width != 320u || receipt.c001_height != 200u ||
        receipt.source_zoom_steps != 18u ||
        receipt.source_delay_ticks != 25u ||
        receipt.source_beam_wait_line != 152u ||
        receipt.presents_nonzero_pixels == 0u ||
        receipt.dungeon_nonzero_pixels == 0u ||
        receipt.master_nonzero_pixels == 0u) {
        fprintf(stderr, "FAIL: source identity or C001 receipt mismatch\n");
        ++failures;
        goto done;
    }
    if (receipt.initial_palette[0] != 0x0004u ||
        receipt.presents_palette[15] != 0x0fffu ||
        receipt.zoom_palette[3] != 0x0a82u ||
        receipt.final_palette[10] != 0x0000u ||
        receipt.final_palette[12] != 0x0f00u) {
        fprintf(stderr, "FAIL: A20 RGB4 title palette mismatch\n");
        ++failures;
        goto done;
    }
    for (index = 0u; index < DM1_V1_AMIGA_TITLE_ZOOM_STEP_COUNT; ++index) {
        if (!dm1_v1_amiga_title_f0437_zoom_step(&receipt, index, &step) ||
            step.index != index ||
            step.destination_width != 48u + index * 16u ||
            step.destination_height != 12u + index * 4u ||
            step.wait_for_beam_line != 152u) {
            fprintf(stderr, "FAIL: F0437 zoom step %u mismatch\n", index);
            ++failures;
            break;
        }
    }
    if (failures == 0 &&
        (!dm1_v1_amiga_title_f0437_zoom_step(&receipt, 0u, &step) ||
         step.destination_x != 136u || step.destination_y != 74u ||
         !dm1_v1_amiga_title_f0437_zoom_step(&receipt, 17u, &step) ||
         step.destination_x != 0u || step.destination_y != 40u)) {
        fprintf(stderr, "FAIL: F0437 zoom endpoint geometry mismatch\n");
        ++failures;
    }
    if (failures == 0 &&
        dm1_v1_amiga_title_f0437_zoom_step(&receipt, 18u, &step)) {
        fprintf(stderr, "FAIL: F0437 accepted an out-of-range zoom step\n");
        ++failures;
    }

done:
    free(mutated);
    free(inner_zip);
    free(adf);
    free(media.graphics);
    free(media.swsh);
    if (failures) return 1;
    puts("PASS: authentic DM1 Amiga A20 C001 F0437 receipt and 18 zoom steps");
    return 0;
}
