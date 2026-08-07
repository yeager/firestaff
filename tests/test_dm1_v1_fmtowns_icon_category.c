#include "dm1_v1_fmtowns_icon_category.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void test_table_byte_exact(void) {
    static const uint16_t expected[7] = { 0, 32, 64, 96, 128, 160, 192 };
    for (int i = 0; i < 7; ++i) {
        assert(dm1_v1_fmtowns_icon_category_thresholds[i] == expected[i]);
    }
}

static void test_classify_first_of_each_category(void) {
    static const uint16_t bases[7] = { 0, 32, 64, 96, 128, 160, 192 };
    uint16_t cat = 0xffff, off = 0xffff;
    for (uint16_t c = 0; c < 7; ++c) {
        assert(dm1_v1_fmtowns_icon_classify_pc34(bases[c], &cat, &off) == 1);
        assert(cat == c);
        assert(off == 0);
    }
}

static void test_classify_last_of_each_category(void) {
    /* Last valid slot in category c is bases[c] + 31 = bases[c+1] - 1
     * (except category 6 which ends at 192+31 = 223). */
    uint16_t cat = 0xffff, off = 0xffff;
    for (uint16_t c = 0; c < 7; ++c) {
        uint16_t index = (uint16_t)(c * 32 + 31);
        assert(dm1_v1_fmtowns_icon_classify_pc34(index, &cat, &off) == 1);
        assert(cat == c);
        assert(off == 31);
    }
}

static void test_out_of_range_fails_closed(void) {
    uint16_t cat = 0xdead, off = 0xbeef;
    /* 224 is one past the last slot of category 6. */
    assert(dm1_v1_fmtowns_icon_classify_pc34(224, &cat, &off) == 0);
    assert(dm1_v1_fmtowns_icon_classify_pc34(1000, &cat, &off) == 0);
    assert(dm1_v1_fmtowns_icon_classify_pc34(65535, &cat, &off) == 0);
    /* NULL gates. */
    assert(dm1_v1_fmtowns_icon_classify_pc34(0, NULL, &off) == 0);
    assert(dm1_v1_fmtowns_icon_classify_pc34(0, &cat, NULL) == 0);
}

static void test_all_valid_indices_classify(void) {
    uint16_t cat = 0, off = 0;
    for (uint16_t i = 0; i < DM1_V1_FMTOWNS_ICON_INDEX_MAX; ++i) {
        assert(dm1_v1_fmtowns_icon_classify_pc34(i, &cat, &off) == 1);
        assert(cat < 7);
        assert(off < 32);
        /* Round-trip: index == cat * 32 + off. */
        assert((uint16_t)(cat * 32 + off) == i);
    }
}

static void test_real_data_round_trip(void) {
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    FILE *fp;
    uint8_t buf[14];
    uint16_t table[7];
    if (!path || !path[0]) { puts("SKIP: no EDM.EXP"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }
    if (fseek(fp, 0x200 + 0x2918c, SEEK_SET) != 0) { fclose(fp); puts("SKIP: seek failed"); return; }
    if (fread(buf, 1, 14, fp) != 14) { fclose(fp); puts("SKIP: read failed"); return; }
    fclose(fp);
    for (int i = 0; i < 7; ++i) {
        table[i] = (uint16_t)(buf[i*2] | (buf[i*2+1] << 8));
        assert(table[i] == dm1_v1_fmtowns_icon_category_thresholds[i]);
    }
    puts("PASS: real EDM.EXP icon-category table matches shipped constants");
}

int main(void) {
    test_table_byte_exact();
    test_classify_first_of_each_category();
    test_classify_last_of_each_category();
    test_out_of_range_fails_closed();
    test_all_valid_indices_classify();
    test_real_data_round_trip();
    printf("All dm1_v1_fmtowns_icon_category tests passed.\n");
    return 0;
}
