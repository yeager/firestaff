
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_v1_creature_names.h"

#define DM_BIN_MNS_TABLE_OFFSET 0x0385F0u
#define DM_BIN_MNS_TABLE_BYTES  0x0300u

static int verify_retail_dm_bin_table(void) {
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
    unsigned char bytes[DM_BIN_MNS_TABLE_BYTES];
    FILE *file;
    size_t cursor = 0;
    int index;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 0;
    }
    if (snprintf(path, sizeof(path), "%s/DM.BIN", root) >= (int)sizeof(path) ||
        !(file = fopen(path, "rb"))) {
        puts("SKIP: retail Nexus DM.BIN is not mounted");
        return 0;
    }
    if (fseek(file, (long)DM_BIN_MNS_TABLE_OFFSET, SEEK_SET) != 0 ||
        fread(bytes, 1, sizeof(bytes), file) != sizeof(bytes)) {
        fclose(file);
        fprintf(stderr, "FAIL: could not read DM.BIN MNS table at 0x%06X\n",
                DM_BIN_MNS_TABLE_OFFSET);
        return 1;
    }
    fclose(file);

    for (index = 0; index < NEXUS_CREATURE_NAME_COUNT; ++index) {
        size_t length;
        const char *expected = nexus_v1_creature_mns_name(index);

        while (cursor < sizeof(bytes) && bytes[cursor] == 0xFFu)
            ++cursor;
        if (cursor >= sizeof(bytes)) {
            fprintf(stderr, "FAIL: DM.BIN MNS table ended before entry %d\n", index);
            return 1;
        }
        length = 0;
        while (cursor + length < sizeof(bytes) && bytes[cursor + length] != '\0')
            ++length;
        if (length == sizeof(bytes) - cursor || length != strlen(expected) ||
            memcmp(expected, &bytes[cursor], length) != 0) {
            fprintf(stderr, "FAIL: DM.BIN MNS entry %d does not match %s\n",
                    index, expected);
            return 1;
        }
        cursor += length + 1;
    }
    printf("ok: retail DM.BIN MNS table matches all %d entries at 0x%06X\n",
           NEXUS_CREATURE_NAME_COUNT, DM_BIN_MNS_TABLE_OFFSET);
    return 0;
}

int main(void) {
    int fail = 0;

    /* Verify table size */
    if (NEXUS_CREATURE_NAME_COUNT != 30) {
        fprintf(stderr, "FAIL: count %d != 30\n", NEXUS_CREATURE_NAME_COUNT);
        fail++;
    }

    /* Spot-check known creatures from DM.BIN 0x0385F0 */
    if (strcmp(nexus_v1_creature_mns_name(0), "ANTMAN.MNS") != 0) {
        fprintf(stderr, "FAIL: index 0\n"); fail++;
    }
    if (strcmp(nexus_v1_creature_mns_name(15), "SCORPION.MNS") != 0) {
        fprintf(stderr, "FAIL: index 15\n"); fail++;
    }
    if (strcmp(nexus_v1_creature_mns_name(26), "CHAOS.MNS") != 0) {
        fprintf(stderr, "FAIL: index 26\n"); fail++;
    }
    if (strcmp(nexus_v1_creature_mns_name(29), "OBAKE.MNS") != 0) {
        fprintf(stderr, "FAIL: index 29\n"); fail++;
    }

    /* Material files at indices 16-17 */
    if (strcmp(nexus_v1_creature_mns_name(16), "SN_FLOOR.MNS") != 0) {
        fprintf(stderr, "FAIL: index 16 floor material\n"); fail++;
    }
    if (strcmp(nexus_v1_creature_mns_name(17), "SN_WALL.MNS") != 0) {
        fprintf(stderr, "FAIL: index 17 wall material\n"); fail++;
    }

    /* Bounds */
    if (nexus_v1_creature_mns_name(-1) != NULL) {
        fprintf(stderr, "FAIL: negative index\n"); fail++;
    }
    if (nexus_v1_creature_mns_name(30) != NULL) {
        fprintf(stderr, "FAIL: out of bounds\n"); fail++;
    }

    /* Bind the complete source table to the mounted European retail DM.BIN. */
    fail += verify_retail_dm_bin_table();

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus creature MNS name table verified (30 entries)\n");
    return 0;
}
