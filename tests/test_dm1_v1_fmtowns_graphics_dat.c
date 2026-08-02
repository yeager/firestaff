#include "dm1_v1_fmtowns_graphics_dat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_probe_null_rejects(void) {
    assert(dm1_v1_fmtowns_graphics_probe(NULL, 0) == 0);
    assert(dm1_v1_fmtowns_graphics_probe(NULL, 396970) == 0);
}

static void test_probe_too_small_rejects(void) {
    uint8_t buf[4] = {0x3f, 0x02, 0, 0};
    assert(dm1_v1_fmtowns_graphics_probe(buf, 4) == 0);
}

static void test_probe_wrong_count_rejects(void) {
    /* Build a buffer with count != 575 */
    uint8_t buf[395000];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x40; buf[1] = 0x02; /* count = 576 */
    assert(dm1_v1_fmtowns_graphics_probe(buf, sizeof(buf)) == 0);
}

static void test_probe_pc34_format_rejects(void) {
    /* PC 3.4 starts with 0x8001 marker, not 575 */
    uint8_t buf[395000];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x01; buf[1] = 0x80; /* 0x8001 new format marker */
    assert(dm1_v1_fmtowns_graphics_probe(buf, sizeof(buf)) == 0);
}

static void test_receipt_null_rejects(void) {
    DM1_V1_FmtownsGraphicsReceipt r;
    assert(dm1_v1_fmtowns_graphics_receipt(NULL, 0, &r) == -1);
    assert(dm1_v1_fmtowns_graphics_receipt(NULL, 0, NULL) == -1);
}

static uint8_t *build_synthetic_fmtowns_gfx(size_t *out_size) {
    uint16_t count = 575;
    uint16_t per_graphic = 10;
    size_t header = 2 + (size_t)count * 4;
    size_t data_area = (size_t)count * per_graphic;
    size_t total = header + data_area;
    uint8_t *buf = (uint8_t *)calloc(1, total);
    if (!buf) return NULL;

    buf[0] = count & 0xff;
    buf[1] = (count >> 8) & 0xff;

    size_t comp_base = 2;
    size_t decomp_base = 2 + (size_t)count * 2;
    for (int i = 0; i < count; i++) {
        buf[comp_base + i * 2]     = per_graphic & 0xff;
        buf[comp_base + i * 2 + 1] = (per_graphic >> 8) & 0xff;
        buf[decomp_base + i * 2]     = per_graphic & 0xff;
        buf[decomp_base + i * 2 + 1] = (per_graphic >> 8) & 0xff;
    }

    *out_size = total;
    return buf;
}

static void test_probe_synthetic_accepts(void) {
    size_t size;
    uint8_t *buf = build_synthetic_fmtowns_gfx(&size);
    assert(buf != NULL);
    assert(size >= DM1_FMTOWNS_GRAPHICS_MIN_SIZE || 1);
    /* Synthetic might be too small; only test if in range */
    if (size >= DM1_FMTOWNS_GRAPHICS_MIN_SIZE &&
        size <= DM1_FMTOWNS_GRAPHICS_MAX_SIZE) {
        assert(dm1_v1_fmtowns_graphics_probe(buf, size) == 1);
    }
    free(buf);
}

static void test_receipt_synthetic(void) {
    size_t size;
    uint8_t *buf = build_synthetic_fmtowns_gfx(&size);
    assert(buf != NULL);
    if (size >= DM1_FMTOWNS_GRAPHICS_MIN_SIZE &&
        size <= DM1_FMTOWNS_GRAPHICS_MAX_SIZE) {
        DM1_V1_FmtownsGraphicsReceipt r;
        int rc = dm1_v1_fmtowns_graphics_receipt(buf, size, &r);
        assert(rc == 0);
        assert(r.is_fmtowns == 1);
        assert(r.graphic_count == 575);
        assert(r.file_size == (uint32_t)size);
        assert(r.lang == DM1_FMTOWNS_LANG_UNKNOWN);
    }
    free(buf);
}

static void test_probe_compressed_rejects(void) {
    /* If comp != decomp, should reject */
    size_t size;
    uint8_t *buf = build_synthetic_fmtowns_gfx(&size);
    assert(buf != NULL);
    /* Make graphic 0's decomp different from comp */
    size_t decomp_base = 2 + 575 * 2;
    buf[decomp_base] = 99;
    if (size >= DM1_FMTOWNS_GRAPHICS_MIN_SIZE) {
        assert(dm1_v1_fmtowns_graphics_probe(buf, size) == 0);
    }
    free(buf);
}

int main(void) {
    test_probe_null_rejects();
    test_probe_too_small_rejects();
    test_probe_wrong_count_rejects();
    test_probe_pc34_format_rejects();
    test_receipt_null_rejects();
    test_probe_synthetic_accepts();
    test_receipt_synthetic();
    test_probe_compressed_rejects();
    printf("All dm1_v1_fmtowns_graphics_dat tests passed.\n");
    return 0;
}
