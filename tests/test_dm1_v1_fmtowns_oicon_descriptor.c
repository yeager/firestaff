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

static void test_full_descriptor_accessor(void) {
    /* Every valid index returns a non-NULL 6-byte pointer whose
     * byte 0 matches the kind table (the two tables must stay in
     * sync). */
    for (uint16_t i = 0; i < 224; ++i) {
        const uint8_t *rec = dm1_v1_fmtowns_oicon_descriptor_at_pc34(i);
        assert(rec != NULL);
        assert(rec[0] == dm1_v1_fmtowns_oicon_kind[i]);
    }
    assert(dm1_v1_fmtowns_oicon_descriptor_at_pc34(224) == NULL);
    assert(dm1_v1_fmtowns_oicon_descriptor_at_pc34(65535) == NULL);
    /* Spot-check specific full records from the recovered table. */
    const uint8_t *r0  = dm1_v1_fmtowns_oicon_descriptor_at_pc34(0);
    assert(r0[0]==0 && r0[1]==0 && r0[2]==5 && r0[3]==144 && r0[4]==0 && r0[5]==0);
    const uint8_t *r5  = dm1_v1_fmtowns_oicon_descriptor_at_pc34(5);
    assert(r5[0]==42 && r5[1]==0 && r5[2]==5 && r5[3]==152 && r5[4]==0 && r5[5]==67);
    const uint8_t *r68 = dm1_v1_fmtowns_oicon_descriptor_at_pc34(68);
    assert(r68[0]==1 && r68[1]==64 && r68[2]==0 && r68[3]==80 && r68[4]==0 && r68[5]==23);
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
        /* ALL 6 bytes per record must match the shipped table. */
        for (int j = 0; j < 6; ++j) {
            assert(descriptor[j] == dm1_v1_fmtowns_oicon_descriptor[i][j]);
        }
    }
    fclose(fp);
    puts("PASS: real EDM.EXP full 6-byte OICON descriptors match shipped table");
}

int main(void) {
    test_count_and_bounds();
    test_specific_kinds();
    test_is_thing_dispatch();
    test_full_descriptor_accessor();
    test_real_data_round_trip();
    printf("All dm1_v1_fmtowns_oicon_descriptor tests passed.\n");
    return 0;
}
