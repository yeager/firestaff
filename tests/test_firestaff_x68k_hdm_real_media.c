#include "firestaff_7z_extract.h"
#include "firestaff_x68k_hdm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    uint32_t size;
    uint16_t first_cluster;
    unsigned seen;
} ExpectedRootFile;

static ExpectedRootFile g_expected[] = {
    {"HUMAN.SYS", 44660u, 2u, 0u},
    {"SYS", 0u, 46u, 0u},
    {"SWOOSH", 9784u, 63u, 0u},
    {"DM.X", 8014u, 73u, 0u},
    {"CONFIG.SYS", 118u, 81u, 0u},
    {"DUNGEON.DAT", 33990u, 82u, 0u},
    {"GRAPHICS.DAT", 332785u, 116u, 0u},
    {"FTLCODE", 156564u, 441u, 0u},
    {"AUTOEXEC.BAT", 3u, 594u, 0u},
    {"COMMAND.X", 24736u, 595u, 0u}
};

static int file_exists(const char *path) {
    FILE *file;
    if (!path || !path[0]) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    fclose(file);
    return 1;
}

static const char *configured_or_home_path(const char *environment_name,
                                           const char *suffix,
                                           char *buffer,
                                           size_t capacity) {
    const char *configured = getenv(environment_name);
    const char *home = getenv("HOME");
    if (configured && configured[0]) return configured;
    if (!home || !home[0] || !suffix || !buffer || capacity == 0u) return NULL;
    if (snprintf(buffer, capacity, "%s/%s", home, suffix) >= (int)capacity) {
        return NULL;
    }
    return buffer;
}

static int compare_reference_file(const char *path,
                                  const uint8_t *bytes,
                                  size_t byte_count) {
    uint8_t *reference = NULL;
    FILE *file;
    long file_size;
    int result = -1;
    file = fopen(path, "rb");
    if (!file) return -1;
    if (fseek(file, 0, SEEK_END) != 0 || (file_size = ftell(file)) < 0 ||
        (size_t)file_size != byte_count || fseek(file, 0, SEEK_SET) != 0) {
        goto done;
    }
    reference = (uint8_t *)malloc(byte_count ? byte_count : 1u);
    if (!reference || fread(reference, 1u, byte_count, file) != byte_count) {
        goto done;
    }
    result = memcmp(reference, bytes, byte_count) == 0 ? 0 : -1;
done:
    free(reference);
    fclose(file);
    return result;
}

static ExpectedRootFile *expected_file(const char *name) {
    size_t i;
    for (i = 0u; i < sizeof(g_expected) / sizeof(g_expected[0]); ++i) {
        if (strcmp(name, g_expected[i].name) == 0) return &g_expected[i];
    }
    return NULL;
}

int main(void) {
    static const char *const archive_suffix =
        ".firestaff/data/dm1/Game,Dungeon_Master,JP,X68000,Software.7z";
    static const char *const cracked_member =
        "Dungeon Master for X68000 3.0 (Japanese) Cracked.hdm";
    static const char *const swoosh_reference_suffix =
        "ReDMCSB/Reference/Original/X30J/SWOOSH";
    char archive_buffer[2048];
    char reference_buffer[2048];
    const char *archive_path;
    const char *reference_path;
    uint8_t *image = NULL;
    size_t image_size = 0u;
    uint8_t *swoosh = NULL;
    uint8_t *ftlcode = NULL;
    uint8_t *graphics = NULL;
    FirestaffX68kHdm hdm;
    uint16_t cursor = 0u;
    size_t entries = 0u;
    size_t i;
    int result = 1;

    archive_path = configured_or_home_path(
        "FIRESTAFF_X68K_HDM_ARCHIVE", archive_suffix,
        archive_buffer, sizeof(archive_buffer));
    reference_path = configured_or_home_path(
        "FIRESTAFF_X68K_SWOOSH_REFERENCE", swoosh_reference_suffix,
        reference_buffer, sizeof(reference_buffer));
    if (!file_exists(archive_path) || !file_exists(reference_path)) {
        puts("SKIP: authentic X68000 archive or ReDMCSB SWOOSH reference is unavailable");
        result = 77;
        goto done;
    }
    if (!firestaff_7z_read_member(archive_path, cracked_member,
                                  &image, &image_size)) {
        fputs("FAIL: could not read the named cracked HDM member from the original 7z\n",
              stderr);
        goto done;
    }
    if (FirestaffX68kHdm_Open(image, image_size, &hdm) != 0) {
        fputs("FAIL: cracked HDM did not match the bounded DM1 X68000 layout\n",
              stderr);
        goto done;
    }

    for (;;) {
        FirestaffX68kHdmRootEntry entry;
        ExpectedRootFile *expected;
        uint8_t *contents;
        int next = FirestaffX68kHdm_NextRootEntry(&hdm, &cursor, &entry);
        if (next == 1) break;
        if (next != 0) {
            fputs("FAIL: malformed X68000 root directory\n", stderr);
            goto done;
        }
        ++entries;
        expected = expected_file(entry.name);
        if (!expected || expected->seen || entry.size != expected->size ||
            entry.first_cluster != expected->first_cluster) {
            fprintf(stderr, "FAIL: unexpected X68000 root entry %s\n", entry.name);
            goto done;
        }
        expected->seen = 1u;
        if ((entry.attributes & 0x10u) != 0u) continue;
        contents = (uint8_t *)malloc(entry.size ? entry.size : 1u);
        if (!contents || FirestaffX68kHdm_ReadRootFile(
                &hdm, &entry, contents, entry.size) != 0) {
            free(contents);
            fprintf(stderr, "FAIL: invalid FAT chain for %s\n", entry.name);
            goto done;
        }
        if (strcmp(entry.name, "SWOOSH") == 0) swoosh = contents;
        else if (strcmp(entry.name, "FTLCODE") == 0) ftlcode = contents;
        else if (strcmp(entry.name, "GRAPHICS.DAT") == 0) graphics = contents;
        else free(contents);
    }
    if (entries != sizeof(g_expected) / sizeof(g_expected[0])) {
        fputs("FAIL: X68000 root directory did not contain the expected file set\n",
              stderr);
        goto done;
    }
    for (i = 0u; i < sizeof(g_expected) / sizeof(g_expected[0]); ++i) {
        if (!g_expected[i].seen) {
            fprintf(stderr, "FAIL: missing X68000 root entry %s\n", g_expected[i].name);
            goto done;
        }
    }
    if (!swoosh || !ftlcode || !graphics ||
        memcmp(image + 0x12000u, swoosh, 9784u) != 0 ||
        memcmp(image + 0x70800u, ftlcode, 156564u) != 0 ||
        graphics[0] != 0x80u || graphics[1] != 0x01u ||
        graphics[2] != 0x02u || graphics[3] != 0x32u) {
        fputs("FAIL: extracted X68000 files did not match raw-file offsets or GRAPHICS.DAT header\n",
              stderr);
        goto done;
    }
    if (compare_reference_file(reference_path, swoosh, 9784u) != 0) {
        fputs("FAIL: extracted SWOOSH differs from ReDMCSB X30J reference bytes\n",
              stderr);
        goto done;
    }
    printf("PASS: read-only X68000 HDM import authenticated %lu root entries; "
           "SWOOSH matches ReDMCSB and FTL offsets are preserved\n",
           (unsigned long)entries);
    result = 0;

done:
    free(graphics);
    free(ftlcode);
    free(swoosh);
    free(image);
    return result;
}
