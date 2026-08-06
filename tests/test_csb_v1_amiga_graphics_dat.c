#include "csb_v1_amiga_graphics_dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL %s\n", msg); } \
} while (0)

static void test_null_rejection(void) {
    CHECK(csb_v1_amiga_graphics_probe(NULL, 0) == 0, "null_data");
    CHECK(csb_v1_amiga_graphics_probe(NULL, 400000) == 0, "null_nonzero");
}

static void test_small_rejection(void) {
    uint8_t buf[4] = {0x80, 0x01, 0x02, 0xED};
    CHECK(csb_v1_amiga_graphics_probe(buf, 4) == 0, "too_small");
}

static void test_wrong_marker(void) {
    uint8_t buf[400000];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x01; buf[1] = 0x80; /* LE marker, not BE */
    buf[2] = 0x02; buf[3] = 0xED; /* count=749 BE */
    CHECK(csb_v1_amiga_graphics_probe(buf, 400000) == 0, "le_marker_rejected");
}

static void test_wrong_count(void) {
    uint8_t buf[400000];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x80; buf[1] = 0x01;
    buf[2] = 0x00; buf[3] = 0x01; /* count=1 */
    CHECK(csb_v1_amiga_graphics_probe(buf, 400000) == 0, "count_too_low");
}

static void test_synthetic_valid(void) {
    uint16_t count = 749;
    size_t header = 4 + (size_t)count * 8;
    size_t total = 400000;
    uint8_t *buf = calloc(total, 1);
    buf[0] = 0x80; buf[1] = 0x01;
    buf[2] = (uint8_t)(count >> 8); buf[3] = (uint8_t)(count & 0xff);
    CHECK(csb_v1_amiga_graphics_probe(buf, total) == 1, "synthetic_probe");

    CSB_V1_AmigaGraphicsReceipt r;
    CHECK(csb_v1_amiga_graphics_receipt(buf, total, &r) == 0, "synthetic_receipt");
    CHECK(r.is_amiga == 1, "synthetic_is_amiga");
    CHECK(r.item_count == 749, "synthetic_count");
    CHECK(r.lang == CSB_AMIGA_LANG_UNKNOWN, "synthetic_lang");
    buf[4] = 0; buf[5] = 4; /* item 0 compressed size */
    buf[4 + (size_t)count * 2] = 0; buf[5 + (size_t)count * 2] = 6;
    {
        CSB_V1_AmigaGraphicsItem item;
        CHECK(csb_v1_amiga_graphics_item(buf, total, 0, &item) == 1,
              "item_table_accepts_source_record");
        CHECK(item.dataOffset == header && item.compressedByteCount == 4 &&
              item.decompressedByteCount == 6, "item_table_reports_be_fields");
        CHECK(csb_v1_amiga_graphics_item(buf, total, count, &item) == 0,
              "item_table_rejects_out_of_range_index");
    }
    free(buf);
}

static void test_receipt_null(void) {
    CHECK(csb_v1_amiga_graphics_receipt(NULL, 0, NULL) == -1, "receipt_null");
}

int main(void) {
    test_null_rejection();
    test_small_rejection();
    test_wrong_marker();
    test_wrong_count();
    test_synthetic_valid();
    test_receipt_null();
    printf("csb_v1_amiga_graphics_dat: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
