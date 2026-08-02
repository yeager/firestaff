
#include <stdio.h>
#include <string.h>
#include "nexus_v1_creature_names.h"

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

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus creature MNS name table verified (30 entries)\n");
    return 0;
}
