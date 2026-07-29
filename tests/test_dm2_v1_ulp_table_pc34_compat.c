#include "dm2_v1_ulp_table_pc34_compat.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_getp_setp(void) {
    DM2_V1_UlpEntry entries[4];
    memset(entries, 0, sizeof(entries));
    DM2_V1_UlpTable table = { entries, 4 };

    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    assert(dm2_v1_ulp_setp(&table, 0, data));
    assert(dm2_v1_ulp_setp(&table, 2, data + 4));

    DM2_V1_UlpGetpReceipt r;
    assert(dm2_v1_ulp_getp(&table, 0, &r));
    assert(r.valid && r.ptr == data);

    assert(dm2_v1_ulp_getp(&table, 2, &r));
    assert(r.valid && r.ptr == data + 4);
}

static void test_setl_islen(void) {
    DM2_V1_UlpEntry entries[4];
    memset(entries, 0, sizeof(entries));
    DM2_V1_UlpTable table = { entries, 4 };

    assert(dm2_v1_ulp_setl(&table, 1, 42));

    DM2_V1_UlpIslenReceipt r;
    assert(dm2_v1_ulp_islen(&table, 1, &r));
    assert(r.valid && r.is_length);

    assert(dm2_v1_ulp_islen(&table, 0, &r));
    assert(r.valid && !r.is_length);
}

static void test_out_of_range(void) {
    DM2_V1_UlpEntry entries[2];
    memset(entries, 0, sizeof(entries));
    DM2_V1_UlpTable table = { entries, 2 };

    DM2_V1_UlpGetpReceipt r;
    assert(!dm2_v1_ulp_getp(&table, 5, &r));
    assert(r.blocked_out_of_range);

    assert(!dm2_v1_ulp_setp(&table, 5, NULL));
    assert(!dm2_v1_ulp_setl(&table, 5, 10));
}

static void test_query_raw_data_length_as_length(void) {
    DM2_V1_UlpEntry entries[2];
    memset(entries, 0, sizeof(entries));
    DM2_V1_UlpTable table = { entries, 2 };

    dm2_v1_ulp_setl(&table, 0, 256);

    DM2_V1_UlpQueryRawDataLengthReceipt r;
    assert(dm2_v1_ulp_query_raw_data_length(&table, 0, &r));
    assert(r.valid);
    assert(r.raw_length == (256 | (int32_t)0x80000000));
    assert(r.effective_length == r.raw_length);
}

static void test_query_raw_data_length_as_pointer(void) {
    DM2_V1_UlpEntry entries[2];
    memset(entries, 0, sizeof(entries));
    DM2_V1_UlpTable table = { entries, 2 };

    uint8_t data[4] = {0};
    dm2_v1_ulp_setp(&table, 0, data);

    DM2_V1_UlpQueryRawDataLengthReceipt r;
    assert(dm2_v1_ulp_query_raw_data_length(&table, 0, &r));
    assert(r.valid);
    assert(r.effective_length == 0);
}

static void test_null_table(void) {
    DM2_V1_UlpGetpReceipt r1;
    assert(!dm2_v1_ulp_getp(NULL, 0, &r1));

    DM2_V1_UlpIslenReceipt r2;
    assert(!dm2_v1_ulp_islen(NULL, 0, &r2));
}

int main(void) {
    test_getp_setp();
    test_setl_islen();
    test_out_of_range();
    test_query_raw_data_length_as_length();
    test_query_raw_data_length_as_pointer();
    test_null_table();
    printf("All dm2_v1_ulp_table tests passed.\n");
    return 0;
}
