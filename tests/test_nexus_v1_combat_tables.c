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

    /* Stat indices (DM.BIN 0x03B5CD) */
    {
        const uint8_t *si = nexus_v1_combat_stat_indices();
        expect(si[0] == 0, "si[0]=0");
        expect(si[5] == 5, "si[5]=5");
    }

    /* Damage thresholds (DM.BIN 0x03B5A8) */
    {
        const uint8_t *dt = nexus_v1_combat_damage_thresholds();
        expect(dt[0] == 0x80, "dt[0]=0x80");
        expect(dt[4] == 0x80, "dt[4]=0x80");
        expect(dt[5] == 0x00, "dt[5]=0x00");
    }

    /* Class param A (DM.BIN 0x03B5B6) */
    {
        const uint16_t *pa = nexus_v1_combat_class_param_a();
        expect(pa[0] == 4,  "pa[0]=4");
        expect(pa[1] == 18, "pa[1]=18");
        expect(pa[2] == 11, "pa[2]=11");
        expect(pa[3] == 25, "pa[3]=25");
    }

    /* Class param B (DM.BIN 0x03B5BE) */
    {
        const uint16_t *pb = nexus_v1_combat_class_param_b();
        expect(pb[0] == 0,  "pb[0]=0");
        expect(pb[1] == 5,  "pb[1]=5");
        expect(pb[2] == 40, "pb[2]=40");
        expect(pb[3] == 26, "pb[3]=26");
    }

    /* Type indices (DM.BIN 0x03B5E2) */
    {
        const uint8_t *ti = nexus_v1_combat_type_indices();
        expect(ti[0] == 1,  "ti[0]=1");
        expect(ti[1] == 21, "ti[1]=21");
        expect(ti[6] == 19, "ti[6]=19");
    }

    /* Flag bits (DM.BIN 0x03B608) */
    {
        const uint32_t *fb = nexus_v1_combat_flag_bits();
        expect(fb[0] == 0x20, "fb[0]=0x20");
        expect(fb[1] == 0x10, "fb[1]=0x10");
        expect(fb[2] == 0x08, "fb[2]=0x08");
        expect(fb[3] == 0x04, "fb[3]=0x04");
    }

    /* Action perm (DM.BIN 0x03B618) */
    {
        const uint8_t *ap = nexus_v1_combat_action_perm();
        expect(ap[0] == 0, "ap[0]=0");
        expect(ap[1] == 4, "ap[1]=4");
        expect(ap[4] == 1, "ap[4]=1");
    }

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_combat_tables: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: DM.BIN combat tables verified (all 11 tables from 0x03B5A0-0x03B620)");
    return 0;
}
