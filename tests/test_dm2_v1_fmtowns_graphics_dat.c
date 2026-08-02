#include "dm2_v1_fmtowns_graphics_dat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_probe_valid(void) {
    uint8_t buf[2 * 1024 * 1024];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x04; buf[1] = 0x80;
    buf[2] = 0x4f; buf[3] = 0x0d;
    assert(dm2_v1_fmtowns_gdat_probe(buf, sizeof(buf)) == 1);
}

static void test_probe_wrong_version(void) {
    uint8_t buf[2 * 1024 * 1024];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x05; buf[1] = 0x80;
    assert(dm2_v1_fmtowns_gdat_probe(buf, sizeof(buf)) == 0);
}

static void test_probe_too_small(void) {
    uint8_t buf[1024];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x04; buf[1] = 0x80;
    assert(dm2_v1_fmtowns_gdat_probe(buf, sizeof(buf)) == 0);
}

static void test_probe_null(void) {
    assert(dm2_v1_fmtowns_gdat_probe(NULL, 0) == 0);
}

static void test_receipt_valid(void) {
    uint8_t buf[2 * 1024 * 1024];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x04; buf[1] = 0x80;
    buf[2] = 0x10; buf[3] = 0x00;

    DM2_V1_FmtownsGdatReceipt r;
    assert(dm2_v1_fmtowns_gdat_receipt(buf, sizeof(buf), &r) == 0);
    assert(r.is_fmtowns == 1);
    assert(r.gdat_version == 4);
    assert(r.raw_data_count == 16);
    assert(r.file_size == sizeof(buf));
}

static void test_receipt_invalid(void) {
    uint8_t buf[1024];
    memset(buf, 0, sizeof(buf));
    DM2_V1_FmtownsGdatReceipt r;
    assert(dm2_v1_fmtowns_gdat_receipt(buf, sizeof(buf), &r) == -1);
    assert(r.is_fmtowns == 0);
}

int main(void) {
    test_probe_valid();
    test_probe_wrong_version();
    test_probe_too_small();
    test_probe_null();
    test_receipt_valid();
    test_receipt_invalid();
    printf("PASS: dm2_v1_fmtowns_graphics_dat\n");
    return 0;
}
