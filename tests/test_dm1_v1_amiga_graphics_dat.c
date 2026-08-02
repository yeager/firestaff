#include "dm1_v1_amiga_graphics_dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL %s\n", msg); } \
} while (0)

static void test_null_rejection(void) {
    CHECK(dm1_v1_amiga_graphics_probe(NULL, 0) == 0, "null_data");
    CHECK(dm1_v1_amiga_graphics_probe(NULL, 1000) == 0, "null_data_nonzero_size");
}

static void test_small_rejection(void) {
    uint8_t buf[4] = {0x02, 0x3f, 0x00, 0x00};
    CHECK(dm1_v1_amiga_graphics_probe(buf, 4) == 0, "too_small");
}

static void test_wrong_count(void) {
    uint8_t buf[400000];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x00; buf[1] = 0x01; /* count=1 in BE */
    CHECK(dm1_v1_amiga_graphics_probe(buf, 400000) == 0, "wrong_count");
}

static void test_pc34_format_rejection(void) {
    uint8_t buf[400000];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x01; buf[1] = 0x80; /* 0x8001 LE marker */
    CHECK(dm1_v1_amiga_graphics_probe(buf, 400000) == 0, "pc34_marker");
}

static void test_synthetic_probe(void) {
    /* Build a synthetic Amiga GRAPHICS.DAT with 575 graphics, all 0 bytes */
    uint16_t count = 575;
    size_t header = 2 + (size_t)count * 4;
    uint8_t *buf = calloc(header, 1);
    buf[0] = (uint8_t)(count >> 8);
    buf[1] = (uint8_t)(count & 0xff);
    /* All comp/decomp are 0, data area = 0, size = header */
    CHECK(dm1_v1_amiga_graphics_probe(buf, header) == 0, "synthetic_too_small");
    free(buf);
}

static void test_synthetic_valid(void) {
    uint16_t count = 575;
    size_t header = 2 + (size_t)count * 4;
    size_t data_per_item = 700;
    size_t total = header + (size_t)count * data_per_item;
    if (total < 350000 || total > 420000) {
        printf("SKIP synthetic_valid: size %zu out of range\n", total);
        return;
    }
    uint8_t *buf = calloc(total, 1);
    buf[0] = (uint8_t)(count >> 8);
    buf[1] = (uint8_t)(count & 0xff);
    for (uint16_t i = 0; i < count; i++) {
        uint16_t sz = (uint16_t)data_per_item;
        /* comp sizes (BE) */
        buf[2 + i * 2 + 0] = (uint8_t)(sz >> 8);
        buf[2 + i * 2 + 1] = (uint8_t)(sz & 0xff);
        /* decomp sizes (BE) */
        buf[2 + count * 2 + i * 2 + 0] = (uint8_t)(sz >> 8);
        buf[2 + count * 2 + i * 2 + 1] = (uint8_t)(sz & 0xff);
    }
    CHECK(dm1_v1_amiga_graphics_probe(buf, total) == 1, "synthetic_valid_probe");

    DM1_V1_AmigaGraphicsReceipt r;
    CHECK(dm1_v1_amiga_graphics_receipt(buf, total, &r) == 0, "synthetic_valid_receipt");
    CHECK(r.is_amiga == 1, "synthetic_is_amiga");
    CHECK(r.graphic_count == 575, "synthetic_count");
    CHECK(r.lang == DM1_AMIGA_LANG_UNKNOWN, "synthetic_lang_unknown");
    free(buf);
}

static void test_receipt_null(void) {
    CHECK(dm1_v1_amiga_graphics_receipt(NULL, 0, NULL) == -1, "receipt_null");
}

static void test_compressed_rejection(void) {
    uint16_t count = 575;
    size_t header = 2 + (size_t)count * 4;
    size_t total = header + 400000;
    uint8_t *buf = calloc(total, 1);
    buf[0] = (uint8_t)(count >> 8);
    buf[1] = (uint8_t)(count & 0xff);
    /* comp[0] != decomp[0] */
    buf[2] = 0x00; buf[3] = 0x10;
    buf[2 + count * 2] = 0x00; buf[2 + count * 2 + 1] = 0x20;
    CHECK(dm1_v1_amiga_graphics_probe(buf, total) == 0, "compressed_rejected");
    free(buf);
}

int main(void) {
    test_null_rejection();
    test_small_rejection();
    test_wrong_count();
    test_pc34_format_rejection();
    test_synthetic_probe();
    test_synthetic_valid();
    test_receipt_null();
    test_compressed_rejection();
    printf("dm1_v1_amiga_graphics_dat: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
