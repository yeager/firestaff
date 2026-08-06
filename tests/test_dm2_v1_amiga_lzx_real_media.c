/* Real-media receipt for DM2's original six-disk Amiga installer.
 *
 * The test streams each nested ADF to RAM, reads dm2_arcsplitN with the OFS
 * reader, and indexes the resulting LZX archive.  It never extracts a game
 * file to disk.  The fixture is optional because copyrighted media remains
 * user supplied. */

#include "dm2_v1_amiga_lzx.h"
#include "dm2_v1_amiga_cd_dat.h"
#include "firestaff_zip_extract.h"
#include "firestaff_amiga_adf.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *wanted_name;
    uint8_t *bytes;
    size_t size;
} PartCapture;

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *file;
    long length;
    uint8_t *bytes;
    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) <= 0L ||
        fseek(file, 0L, SEEK_SET) != 0) { fclose(file); return NULL; }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1U, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)length;
    return bytes;
}

static int capture_part(const char *name, const uint8_t *bytes,
                        size_t size, void *user_data) {
    PartCapture *capture = (PartCapture *)user_data;
    if (!capture || strcmp(name, capture->wanted_name) != 0) return 0;
    capture->bytes = (uint8_t *)malloc(size);
    if (!capture->bytes) return -1;
    memcpy(capture->bytes, bytes, size);
    capture->size = size;
    return 1;
}

static int load_original_part(const char *archive, unsigned int disk,
                              DM2_V1_AmigaLzxPart *out_part) {
    char inner_suffix[128];
    char wanted_name[32];
    uint8_t *outer;
    uint8_t *inner = NULL;
    uint8_t *adf = NULL;
    size_t outer_size;
    size_t inner_size;
    size_t adf_size;
    PartCapture capture;
    int found;
    if (!archive || !out_part || disk == 0u || disk > DM2_V1_AMIGA_LZX_PART_COUNT) return 0;
    outer = read_file(archive, &outer_size);
    if (!outer) return 0;
    snprintf(inner_suffix, sizeof(inner_suffix),
             "(1994)(Interplay)(AGA)(M3)(Disk %u of 6)[HD].zip", disk);
    if (firestaff_zip_extract_memory_by_suffix(outer, outer_size, inner_suffix,
                                               &inner, &inner_size) != 0 ||
        firestaff_zip_extract_memory_by_suffix(inner, inner_size, ".adf",
                                               &adf, &adf_size) != 0) {
        free(outer);
        free(inner);
        return 0;
    }
    free(outer);
    free(inner);
    snprintf(wanted_name, sizeof(wanted_name), "dm2_arcsplit%u", disk);
    memset(&capture, 0, sizeof(capture));
    capture.wanted_name = wanted_name;
    found = firestaff_amiga_adf_visit_ofs_files(adf, adf_size, capture_part, &capture);
    free(adf);
    if (found < 0 || !capture.bytes || capture.size == 0u) {
        free(capture.bytes);
        return 0;
    }
    out_part->bytes = capture.bytes;
    out_part->size = capture.size;
    return 1;
}

static const char *amiga_archive_path(void) {
    const char *configured = getenv("FIRESTAFF_DM2_AMIGA_ARCHIVE");
    const char *home = getenv("HOME");
    static char default_path[1024];
    if (configured && configured[0] != '\0') return configured;
    if (!home) return NULL;
    snprintf(default_path, sizeof(default_path),
             "%s/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Amiga_EN.zip", home);
    return default_path;
}

static void test_original_installer_media(void) {
    const char *archive_path = amiga_archive_path();
    DM2_V1_AmigaLzxPart parts[DM2_V1_AMIGA_LZX_PART_COUNT] = {{0}};
    DM2_V1_AmigaLzxArchive archive;
    DM2_V1_AmigaCdDat cd_map;
    const DM2_V1_AmigaLzxEntry *graphics;
    const DM2_V1_AmigaLzxEntry *dungeon;
    const DM2_V1_AmigaLzxEntry *cd_dat;
    uint8_t *joined = NULL;
    uint8_t *decoded = NULL;
    size_t decoded_size = 0u;
    size_t joined_size = 0u;
    unsigned int i;
    FILE *file;
    if (!archive_path || !(file = fopen(archive_path, "rb"))) {
        printf("  SKIP: original Amiga archive not available\n");
        return;
    }
    fclose(file);
    for (i = 0u; i < DM2_V1_AMIGA_LZX_PART_COUNT; ++i) {
        printf("  loading original disk %u\n", i + 1u);
        fflush(stdout);
        assert(load_original_part(archive_path, i + 1u, &parts[i]) == 1);
    }
    assert(dm2_v1_amiga_lzx_join_parts(parts, &joined, &joined_size) == 1);
    assert(joined_size == 4310121u);
    assert(dm2_v1_amiga_lzx_parse(&archive, joined, joined_size) == 1);
    assert(archive.valid == 1);
    assert(archive.entry_count == 35u);
    assert(dm2_v1_amiga_lzx_has_install_payload(&archive) == 1);
    graphics = dm2_v1_amiga_lzx_find(&archive, "GRAPHICS.DAT");
    dungeon = dm2_v1_amiga_lzx_find(&archive, "DUNGEON.DAT");
    cd_dat = dm2_v1_amiga_lzx_find(&archive, "CD.DAT");
    assert(graphics && graphics->uncompressed_size == 3493879u &&
           graphics->compressed_size == 2755876u && graphics->method == 2u);
    assert(dungeon && dungeon->uncompressed_size == 39411u &&
           dungeon->compressed_size == 0u && dungeon->method == 2u);
    assert(cd_dat && cd_dat->uncompressed_size == 176u && cd_dat->method == 2u);
    assert(dm2_v1_amiga_lzx_extract_entry(&archive, joined, dungeon,
                                          &decoded, &decoded_size) == 1);
    assert(decoded_size == dungeon->uncompressed_size);
    dm2_v1_amiga_lzx_free(decoded);
    decoded = NULL;
    assert(dm2_v1_amiga_lzx_extract_entry(&archive, joined, graphics,
                                          &decoded, &decoded_size) == 1);
    assert(decoded_size == graphics->uncompressed_size);
    dm2_v1_amiga_lzx_free(decoded);
    decoded = NULL;
    assert(dm2_v1_amiga_lzx_extract_entry(&archive, joined, cd_dat,
                                          &decoded, &decoded_size) == 1);
    assert(decoded_size == 176u);
    assert(dm2_v1_amiga_cd_dat_parse(&cd_map, decoded, decoded_size) == 1);
    assert(dm2_v1_amiga_cd_dat_mod_for_map(&cd_map, 0) == 3);
    dm2_v1_amiga_lzx_free(decoded);
    printf("  PASS: six original parts index %u LZX entries in RAM\n", archive.entry_count);
    printf("  PASS: original GRAPHICS.DAT, DUNGEON.DAT and CD.DAT decode in RAM\n");
    dm2_v1_amiga_lzx_free(joined);
    for (i = 0u; i < DM2_V1_AMIGA_LZX_PART_COUNT; ++i) free((void *)parts[i].bytes);
}

int main(void) {
    DM2_V1_AmigaLzxArchive archive = {0};
    printf("DM2 Amiga original LZX media tests:\n");
    assert(dm2_v1_amiga_lzx_join_parts(NULL, NULL, NULL) == 0);
    assert(dm2_v1_amiga_lzx_parse(NULL, NULL, 0u) == 0);
    assert(dm2_v1_amiga_lzx_find(NULL, "GRAPHICS.DAT") == NULL);
    assert(dm2_v1_amiga_lzx_has_install_payload(&archive) == 0);
    test_original_installer_media();
    printf("All Amiga LZX media tests passed.\n");
    return 0;
}
