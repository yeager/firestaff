#include "dm1_v1_fmtowns_oicon_descriptor.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void test_count_and_bounds(void) {
    assert(DM1_V1_FMTOWNS_OICON_KIND_COUNT == 224);
    /* Out-of-range returns 0xff sentinel. */
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(224) == 0xff);
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(1000) == 0xff);
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(65535) == 0xff);
    assert(dm1_v1_fmtowns_oicon_is_thing_pc34(224) == 0);
    assert(dm1_v1_fmtowns_oicon_is_thing_pc34(1000) == 0);
}

static void test_specific_kinds(void) {
    /* Byte-exact spot checks from the disassembly-recovered table. */
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(0) == 0);
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(5) == 42);
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(23) == 43);
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(24) == 7);
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(68) == 1);   /* end of first cluster */
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(190) == 128);
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(202) == 255);
    assert(dm1_v1_fmtowns_oicon_kind_at_pc34(223) == 10);
}

static void test_is_thing_dispatch(void) {
    /* Byte 5 = 42 -> THING. */
    assert(dm1_v1_fmtowns_oicon_is_thing_pc34(5) == 1);
    /* Byte 0 = 0 -> generic LOAD_ICON. */
    assert(dm1_v1_fmtowns_oicon_is_thing_pc34(0) == 0);
    /* Count THINGs across the whole table — must be 77. */
    int things = 0;
    for (uint16_t i = 0; i < 224; ++i) {
        if (dm1_v1_fmtowns_oicon_is_thing_pc34(i)) ++things;
    }
    assert(things == 77);
}

static void test_real_data_round_trip(void) {
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    FILE *fp;
    uint8_t descriptor[6];
    if (!path || !path[0]) { puts("SKIP: no EDM.EXP"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }
    for (uint16_t i = 0; i < 224; ++i) {
        if (fseek(fp, 0x200 + 0x224db + i * 6, SEEK_SET) != 0) {
            fclose(fp); puts("SKIP: seek failed"); return;
        }
        if (fread(descriptor, 1, 6, fp) != 6) {
            fclose(fp); puts("SKIP: read failed"); return;
        }
        assert(descriptor[0] == dm1_v1_fmtowns_oicon_kind[i]);
    }
    fclose(fp);
    puts("PASS: real EDM.EXP OICON descriptor kinds match shipped table");
}

int main(void) {
    test_count_and_bounds();
    test_specific_kinds();
    test_is_thing_dispatch();
    test_real_data_round_trip();
    printf("All dm1_v1_fmtowns_oicon_descriptor tests passed.\n");
    return 0;
}
