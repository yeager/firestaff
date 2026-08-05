#include "csb_v1_fmtowns_graphics_dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

static uint8_t *load_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return buf;
}

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static void test_probe_null(void) {
    ASSERT(csb_v1_fmtowns_graphics_probe(NULL, 0) == 0,
           "probe rejects NULL");
    uint8_t small[4] = {0x01, 0x80, 0xd8, 0x02};
    ASSERT(csb_v1_fmtowns_graphics_probe(small, 4) == 0,
           "probe rejects too-small file");
}

static void test_probe_wrong_marker(void) {
    uint8_t buf[8] = {0x00, 0x00, 0xd8, 0x02, 0, 0, 0, 0};
    ASSERT(csb_v1_fmtowns_graphics_probe(buf, 400000) == 0,
           "probe rejects wrong marker");
}

static void test_probe_wrong_count(void) {
    uint8_t buf[8] = {0x01, 0x80, 0x00, 0x01, 0, 0, 0, 0};
    ASSERT(csb_v1_fmtowns_graphics_probe(buf, 400000) == 0,
           "probe rejects wrong item count");
}

static void test_real_data(void) {
    char path_en[512], path_jp[512];
    const char *home = getenv("HOME");
    uint8_t *data;
    size_t size;
    CSB_V1_FmtownsGraphicsReceipt receipt;

    if (!home) {
        printf("SKIP: HOME not set\n");
        return;
    }
    snprintf(path_en, sizeof(path_en),
             "%s/.firestaff/data/csb/fmtowns/CDATA/GRAPHICS.DAT", home);
    snprintf(path_jp, sizeof(path_jp),
             "%s/.firestaff/data/csb/fmtowns/CJDATA/GRAPHICS.DAT", home);

    /* English version */
    data = load_file(path_en, &size);
    if (!data) {
        printf("SKIP: FM Towns CSB English GRAPHICS.DAT not available\n");
        return;
    }

    ASSERT(csb_v1_fmtowns_graphics_probe(data, size) == 1,
           "probe accepts FM Towns EN GRAPHICS.DAT");
    ASSERT(size == CSB_FMTOWNS_GRAPHICS_EN_EXPECTED_SIZE,
           "EN file size matches expected");

    ASSERT(csb_v1_fmtowns_graphics_receipt(data, size, &receipt) == 0,
           "receipt succeeds for EN");
    ASSERT(receipt.is_fmtowns == 1, "receipt marks FM Towns");
    ASSERT(receipt.item_count == 728, "receipt has 728 items");
    printf("  EN: %u images, %u empty, %u data records\n",
           receipt.image_item_count, receipt.empty_item_count,
           receipt.data_item_count);
    ASSERT(receipt.image_item_count > 650, "EN has >650 image items");
    ASSERT(receipt.header_fnv1a != 0, "EN header hash non-zero");
    ASSERT(receipt.payload_fnv1a != 0, "EN payload hash non-zero");

    /* Decode C001 title (item 1: 320x153) */
    {
        uint16_t cw = rd16le(data + 4 + 728*4 + 1*4);
        uint16_t ch = rd16le(data + 4 + 728*4 + 1*4 + 2);
        uint8_t *pixels;
        CSB_V1_FmtownsItemDecodeReceipt irec;
        size_t pixel_count;

        ASSERT(cw == 320 && ch == 153, "C001 container dims 320x153");
        pixel_count = (size_t)cw * ch;
        pixels = (uint8_t *)calloc(1, pixel_count);
        ASSERT(pixels != NULL, "pixel alloc");

        if (pixels) {
            int ok = csb_v1_fmtowns_graphics_decode_item(
                data, size, 1u, pixels, pixel_count, &irec);
            ASSERT(ok == 1, "C001 decode succeeds");
            ASSERT(irec.valid == 1, "C001 receipt valid");
            ASSERT(irec.width == 320, "C001 width 320");
            ASSERT(irec.height == 153, "C001 height 153");
            ASSERT(irec.is_image == 1, "C001 is image");
            ASSERT(irec.pixel_count == pixel_count, "C001 pixel count");
            ASSERT(irec.pixel_fnv1a != 0, "C001 pixel hash non-zero");

            /* Verify non-trivial content (title has visible pixels) */
            {
                size_t nonzero = 0, j;
                for (j = 0; j < pixel_count; j++) {
                    if (pixels[j] != 0) nonzero++;
                }
                ASSERT(nonzero > pixel_count / 4,
                       "C001 has >25% non-zero pixels");
                printf("  C001: %zu/%zu non-zero pixels (%.1f%%)\n",
                       nonzero, pixel_count,
                       100.0 * (double)nonzero / (double)pixel_count);
            }
            free(pixels);
        }
    }

    /* Decode C004 entrance (item 4: 320x200) */
    {
        uint16_t cw = rd16le(data + 4 + 728*4 + 4*4);
        uint16_t ch = rd16le(data + 4 + 728*4 + 4*4 + 2);
        uint8_t *pixels;
        CSB_V1_FmtownsItemDecodeReceipt irec;
        size_t pixel_count;

        ASSERT(cw == 320 && ch == 200, "C004 container dims 320x200");
        pixel_count = (size_t)cw * ch;
        pixels = (uint8_t *)calloc(1, pixel_count);
        if (pixels) {
            int ok = csb_v1_fmtowns_graphics_decode_item(
                data, size, 4u, pixels, pixel_count, &irec);
            ASSERT(ok == 1, "C004 decode succeeds");
            ASSERT(irec.width == 320 && irec.height == 200,
                   "C004 dims 320x200");
            free(pixels);
        }
    }

    /* Batch decode all image items */
    {
        size_t payload_start = 4u + 728u * 2u + 728u * 2u + 728u * 4u;
        size_t item_off = payload_start;
        uint16_t decoded = 0, failed = 0, skipped = 0;
        uint16_t i;
        for (i = 0; i < 728; i++) {
            uint16_t comp = rd16le(data + 4 + i * 2);
            uint16_t cw = rd16le(data + 4 + 728*4 + i*4);
            uint16_t ch = rd16le(data + 4 + 728*4 + i*4 + 2);

            if (comp == 0 || cw == 0 || ch == 0 || cw > 640 || ch > 400 ||
                comp < 4) {
                skipped++;
                item_off += comp;
                continue;
            }
            /* Skip data records where item header doesn't match container */
            {
                uint16_t iw = rd16le(data + item_off);
                uint16_t ih = rd16le(data + item_off + 2);
                if (iw != cw || ih != ch) {
                    skipped++;
                    item_off += comp;
                    continue;
                }
            }

            size_t pixel_count = (size_t)cw * ch;
            uint8_t *pixels = (uint8_t *)calloc(1, pixel_count);
            if (pixels) {
                int ok = csb_v1_fmtowns_img2_decode(
                    data + item_off, comp, cw, ch,
                    pixels, pixel_count, NULL);
                if (ok) decoded++;
                else { failed++; printf("  FAIL item %u: %ux%u comp=%u\n", i, cw, ch, comp); }
                free(pixels);
            }
            item_off += comp;
        }
        printf("  Batch: %u decoded, %u failed, %u skipped\n",
               decoded, failed, skipped);
        ASSERT(decoded >= 650, "batch decode >=650 items");
    }

    free(data);

    /* Japanese version */
    data = load_file(path_jp, &size);
    if (data) {
        ASSERT(csb_v1_fmtowns_graphics_probe(data, size) == 1,
               "probe accepts FM Towns JP GRAPHICS.DAT");
        ASSERT(size == CSB_FMTOWNS_GRAPHICS_JP_EXPECTED_SIZE,
               "JP file size matches expected");
        ASSERT(csb_v1_fmtowns_graphics_receipt(data, size, &receipt) == 0,
               "receipt succeeds for JP");
        ASSERT(receipt.item_count == 728, "JP has 728 items");
        printf("  JP: %u images, %u empty, %u data records\n",
               receipt.image_item_count, receipt.empty_item_count,
               receipt.data_item_count);
        free(data);
    } else {
        printf("SKIP: FM Towns CSB Japanese GRAPHICS.DAT not available\n");
    }
}

int main(void) {
    test_probe_null();
    test_probe_wrong_marker();
    test_probe_wrong_count();
    test_real_data();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
