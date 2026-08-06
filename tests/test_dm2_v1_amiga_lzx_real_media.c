/* Real-media receipt for DM2's original six-disk Amiga installer.
 *
 * The test streams each nested ADF to RAM, reads dm2_arcsplitN with the OFS
 * reader, and indexes the resulting LZX archive.  It never extracts a game
 * file to disk.  The fixture is optional because copyrighted media remains
 * user supplied. */

#include "dm2_v1_amiga_lzx.h"
#include "dm2_v1_amiga_cd_dat.h"
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

static char *shell_quote(const char *text) {
    size_t i, length, used = 0u;
    char *quoted;
    if (!text) return NULL;
    length = strlen(text);
    quoted = (char *)malloc(length * 5u + 3u);
    if (!quoted) return NULL;
    quoted[used++] = '\'';
    for (i = 0u; i < length; ++i) {
        if (text[i] == '\'') {
            memcpy(quoted + used, "'\\''", 4u);
            used += 4u;
        } else {
            quoted[used++] = text[i];
        }
    }
    quoted[used++] = '\'';
    quoted[used] = '\0';
    return quoted;
}

static uint8_t *read_pipe(const char *command, size_t *out_size) {
    FILE *pipe;
    uint8_t *bytes = NULL;
    size_t capacity = 0u, used = 0u;
    int status;
    if (!command || !out_size) return NULL;
    pipe = popen(command, "r");
    if (!pipe) return NULL;
    for (;;) {
        size_t got;
        if (used == capacity) {
            size_t next = capacity ? capacity * 2u : 65536u;
            uint8_t *grown;
            if (next > 2u * 1024u * 1024u) { free(bytes); pclose(pipe); return NULL; }
            grown = (uint8_t *)realloc(bytes, next);
            if (!grown) { free(bytes); pclose(pipe); return NULL; }
            bytes = grown;
            capacity = next;
        }
        got = fread(bytes + used, 1u, capacity - used, pipe);
        used += got;
        if (got == 0u) break;
    }
    status = pclose(pipe);
    if (status != 0 || used == 0u) { free(bytes); return NULL; }
    *out_size = used;
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
    char *quoted_archive;
    char command[1400];
    char wanted_name[32];
    uint8_t *adf;
    size_t adf_size;
    PartCapture capture;
    int found;
    if (!archive || !out_part || disk == 0u || disk > DM2_V1_AMIGA_LZX_PART_COUNT) return 0;
    quoted_archive = shell_quote(archive);
    if (!quoted_archive) return 0;
    snprintf(command, sizeof(command),
             "unzip -p %s 'Dungeon Master II - The Legend Of Skullkeep "
             "(1994)(Interplay)(AGA)(M3)(Disk %u of 6)\\[HD\\].zip' "
             "| bsdtar -xOf - '*.adf'",
             quoted_archive, disk);
    free(quoted_archive);
    adf = read_pipe(command, &adf_size);
    if (!adf) return 0;
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
    DM2_V1_AmigaLzxArchive archive;
    printf("DM2 Amiga original LZX media tests:\n");
    assert(dm2_v1_amiga_lzx_join_parts(NULL, NULL, NULL) == 0);
    assert(dm2_v1_amiga_lzx_parse(NULL, NULL, 0u) == 0);
    assert(dm2_v1_amiga_lzx_find(NULL, "GRAPHICS.DAT") == NULL);
    assert(dm2_v1_amiga_lzx_has_install_payload(&archive) == 0);
    test_original_installer_media();
    printf("All Amiga LZX media tests passed.\n");
    return 0;
}
