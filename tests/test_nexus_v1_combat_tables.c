#include "nexus_v1_creatures.h"
#include <stdio.h>

static int g_failures;

static void expect(int cond, const char *msg) {
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); ++g_failures; }
}

int main(void) {
    const uint8_t *perm, *xp, *bits;
    const uint16_t *items;

    /* Attack permutation table (DM.BIN 0x03B5A0) */
    perm = nexus_v1_combat_attack_perm();
    expect(perm[0] == 0x02, "perm[0]=2");
    expect(perm[1] == 0x03, "perm[1]=3");
    expect(perm[4] == 0x07, "perm[4]=7");
    expect(perm[7] == 0x06, "perm[7]=6");

    /* Experience thresholds (DM.BIN 0x03B5D8) */
    xp = nexus_v1_combat_xp_thresholds();
    expect(xp[0] == 40,  "xp[0]=40");
    expect(xp[1] == 80,  "xp[1]=80");
    expect(xp[2] == 120, "xp[2]=120");
    expect(xp[3] == 160, "xp[3]=160");
    expect(xp[4] == 200, "xp[4]=200");
    expect(xp[5] == 240, "xp[5]=240");

    /* Stat bitmask table (DM.BIN 0x03B5C6) */
    bits = nexus_v1_combat_stat_bits();
    expect(bits[0] == 1,  "bits[0]=1");
    expect(bits[1] == 2,  "bits[1]=2");
    expect(bits[2] == 4,  "bits[2]=4");
    expect(bits[3] == 8,  "bits[3]=8");
    expect(bits[4] == 16, "bits[4]=16");
    expect(bits[5] == 32, "bits[5]=32");

    /* Special combat items (DM.BIN 0x03B5D2) */
    items = nexus_v1_combat_special_items();
    expect(items[0] == 0x95, "item[0]=0x95 (149)");
    expect(items[1] == 0x98, "item[1]=0x98 (152)");
    expect(items[2] == 0x97, "item[2]=0x97 (151)");

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_combat_tables: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: DM.BIN combat tables verified (attack perm + XP thresholds + stat bits + special items)");
    return 0;
}
