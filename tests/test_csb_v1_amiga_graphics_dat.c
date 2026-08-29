#include "csb_v1_amiga_graphics_dat.h"
#include "firestaff_amiga_adf.h"
#include "firestaff_zip_extract.h"
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

static void test_img1_decode(void) {
    const uint16_t count = 700u;
    const size_t header = 4u + (size_t)count * 8u;
    const size_t total = 300000u;
    uint8_t *buf = calloc(total, 1u);
    uint8_t pixels[1];
    uint16_t width = 0u;
    uint16_t height = 0u;

    if (!buf) {
        CHECK(0, "img1_decode_allocation");
        return;
    }
    buf[0] = 0x80u; buf[1] = 0x01u;
    buf[2] = (uint8_t)(count >> 8u); buf[3] = (uint8_t)count;
    /* One literal 1x1 IMG1 pixel: big-endian dimensions followed by 0,7. */
    buf[4] = 0u; buf[5] = 5u;
    buf[4u + (size_t)count * 2u] = 0u;
    buf[5u + (size_t)count * 2u] = 5u;
    buf[header + 0u] = 0u; buf[header + 1u] = 1u;
    buf[header + 2u] = 0u; buf[header + 3u] = 1u;
    buf[header + 4u] = 0x07u;
    CHECK(csb_v1_amiga_graphics_decode_item(buf, total, 0u, pixels,
                                             sizeof(pixels), &width, &height) == 1 &&
              width == 1u && height == 1u && pixels[0] == 7u,
          "img1_decode_direct_be_record");
    CHECK(csb_v1_amiga_graphics_decode_item(buf, total, 0u, pixels, 0u,
                                             &width, &height) == 0,
          "img1_decode_rejects_undersized_destination");
    buf[4] = 0u; buf[5] = 4u;
    CHECK(csb_v1_amiga_graphics_decode_item(buf, total, 0u, pixels,
                                             sizeof(pixels), &width, &height) == 0,
          "img1_decode_rejects_compressed_record_without_expansion_owner");
    free(buf);
}

/* Opt-in corpus regression: the selected A35E GRAPHICS.DAT must expose at
 * least one direct original IMG1 record.  This consumes no constructed
 * campaign data and leaves compressed records fail-closed. */
static void test_real_a35e_img1_if_available(void) {
    const char *path = getenv("FIRESTAFF_CSB_AMIGA35E_GRAPHICS_DAT");
    FILE *file = NULL;
    long length;
    uint8_t *bytes = NULL;
    uint8_t *pixels = NULL;
    CSB_V1_AmigaGraphicsReceipt receipt;
    uint16_t item_index;
    int decoded = 0;

    if (!path || !path[0]) {
        puts("SKIP real_a35e_img1: FIRESTAFF_CSB_AMIGA35E_GRAPHICS_DAT is unset");
        return;
    }
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        CHECK(0, "real_a35e_img1_open");
        if (file) fclose(file);
        return;
    }
    bytes = malloc((size_t)length);
    pixels = malloc(640u * 400u);
    if (!bytes || !pixels || fread(bytes, 1u, (size_t)length, file) !=
            (size_t)length ||
        csb_v1_amiga_graphics_receipt(bytes, (size_t)length, &receipt) != 0 ||
        receipt.version != CSB_AMIGA_VER_3_5 ||
        receipt.lang != CSB_AMIGA_LANG_EN) {
        CHECK(0, "real_a35e_img1_authentication");
        free(pixels);
        free(bytes);
        fclose(file);
        return;
    }
    for (item_index = 0u; item_index < receipt.item_count; ++item_index) {
        uint16_t width = 0u;
        uint16_t height = 0u;
        if (csb_v1_amiga_graphics_decode_item(bytes, (size_t)length,
                                               item_index, pixels, 640u * 400u,
                                               &width, &height)) {
            decoded = width > 0u && height > 0u;
            break;
        }
    }
    CHECK(decoded, "real_a35e_img1_direct_record_decodes");
    {
        uint16_t width = 0u;
        uint16_t height = 0u;
        /* ReDMCSB DEFS.H C017_GRAPHIC_INVENTORY and PANEL.C F0347 bind
         * this exact 224x136 source backdrop to the inventory viewport. */
        CHECK(csb_v1_amiga_graphics_decode_item(bytes, (size_t)length, 17u,
                                                 pixels, 640u * 400u,
                                                 &width, &height) == 1 &&
                  width == 224u && height == 136u,
              "real_a35e_c017_inventory_decodes_at_source_dimensions");
    }
    {
        uint16_t width = 0u;
        uint16_t height = 0u;
        /* REVIVE.C F0280 copies this 8x3 atlas as 32x29 champion portraits
         * before PANEL.C F0354 places the selected portrait in a status box.
         * Keep the native A35E source dimensions explicit so a future Amiga
         * candidate/HUD consumer cannot substitute the PC34 asset shape. */
        CHECK(csb_v1_amiga_graphics_decode_item(bytes, (size_t)length, 26u,
                                                 pixels, 640u * 400u,
                                                 &width, &height) == 1 &&
                  width == 256u && height == 87u,
              "real_a35e_c026_portrait_atlas_decodes_at_source_dimensions");
    }
    {
        uint16_t width = 0u;
        uint16_t height = 0u;
        /* REVIVE.C F0281 places C027 in C101 with C04 transparency.  It has
         * the same 144x73 panel geometry as C040, but distinct source bytes. */
        CHECK(csb_v1_amiga_graphics_decode_item(bytes, (size_t)length, 27u,
                                                 pixels, 640u * 400u,
                                                 &width, &height) == 1 &&
                  width == 144u && height == 73u,
              "real_a35e_c027_rename_panel_decodes_at_source_dimensions");
    }
    {
        uint16_t width = 0u;
        uint16_t height = 0u;
        /* ReDMCSB PANEL.C F0346 overlays C040 at panel-relative (80,52),
         * with C06 as transparency, on top of the C017 inventory panel. */
        CHECK(csb_v1_amiga_graphics_decode_item(bytes, (size_t)length, 40u,
                                                 pixels, 640u * 400u,
                                                 &width, &height) == 1 &&
                  width == 144u && height == 73u,
              "real_a35e_c040_resurrect_panel_decodes_at_source_dimensions");
    }
    {
        uint16_t width = 0u;
        uint16_t height = 0u;
        /* ReDMCSB DEFS.H C013_GRAPHIC_MOVEMENT_ARROWS and PANEL.C F0395
         * bind this exact 87x45 source panel to C009_ZONE_MOVEMENT_ARROWS. */
        CHECK(csb_v1_amiga_graphics_decode_item(bytes, (size_t)length, 13u,
                                                 pixels, 640u * 400u,
                                                 &width, &height) == 1 &&
                  width == 87u && height == 45u,
              "real_a35e_c013_movement_panel_decodes_at_source_dimensions");
    }
    free(pixels);
    free(bytes);
    fclose(file);
}

typedef struct {
    int found;
    int valid;
    int inventory_decoded;
    uint16_t inventory_width;
    uint16_t inventory_height;
    CSB_V1_AmigaGraphicsReceipt receipt;
} RealCsbAmigaGraphicsReceipt;

static int real_csb_graphics_visitor(const char *name, const uint8_t *bytes,
                                     size_t size, void *user_data) {
    RealCsbAmigaGraphicsReceipt *result =
        (RealCsbAmigaGraphicsReceipt *)user_data;
    if (!name || !bytes || !result || strcmp(name, "Graphics.DAT") != 0) {
        return 1;
    }
    result->found = 1;
    result->valid = csb_v1_amiga_graphics_receipt(bytes, size,
                                                   &result->receipt) == 0;
    if (result->valid) {
        uint8_t *pixels = malloc(640u * 400u);
        if (pixels) {
            result->inventory_decoded = csb_v1_amiga_graphics_decode_item(
                bytes, size, 17u, pixels, 640u * 400u,
                &result->inventory_width, &result->inventory_height);
            free(pixels);
        }
    }
    return 0;
}

/* The supplied CSB Amiga preservation archive contains the original A disk.
 * Read its canonical `Graphics.DAT` through the native AmigaDOS visitor in
 * RAM; do not create a loose ADF or game-data file. */
static void test_real_amiga_adf_graphics_receipt(void) {
    const char *archive = getenv("FIRESTAFF_CSB_AMIGA_ADF_ARCHIVE");
    uint8_t *adf = NULL;
    size_t adf_size = 0U;
    FILE *stream;
    RealCsbAmigaGraphicsReceipt result;

    if (!archive || !archive[0]) {
        puts("SKIP real_csb_amiga_graphics: archive not configured");
        return;
    }
    stream = fopen(archive, "rb");
    if (!stream) {
        puts("SKIP real_csb_amiga_graphics: archive unavailable");
        return;
    }
    fclose(stream);

    memset(&result, 0, sizeof(result));
    CHECK(firestaff_zip_extract_by_suffix(archive,
                                           "Chaos Strikes Back (FTL) A.adf",
                                           &adf, &adf_size) == 0,
          "real_csb_amiga_adf_member");
    if (!adf) return;
    CHECK(firestaff_amiga_adf_visit_ofs_files(adf, adf_size,
                                               real_csb_graphics_visitor,
                                               &result) >= 0,
          "real_csb_amiga_adf_visit");
    free(adf);
    CHECK(result.found == 1, "real_csb_amiga_graphics_found");
    CHECK(result.valid == 1, "real_csb_amiga_graphics_receipt");
    if (!result.valid) return;
    CHECK(result.receipt.is_amiga == 1, "real_csb_amiga_is_amiga");
    CHECK(result.receipt.item_count >= 700u && result.receipt.item_count <= 800u,
          "real_csb_amiga_item_count");
    CHECK(result.receipt.lang != CSB_AMIGA_LANG_UNKNOWN,
          "real_csb_amiga_known_language");
    CHECK(result.receipt.version != CSB_AMIGA_VER_UNKNOWN,
          "real_csb_amiga_known_version");
    CHECK(result.inventory_decoded == 1,
          "real_csb_amiga_c017_inventory_decodes");
    CHECK(result.inventory_width == 224u && result.inventory_height == 136u,
          "real_csb_amiga_c017_inventory_source_dimensions");
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
    test_img1_decode();
    test_real_a35e_img1_if_available();
    test_real_amiga_adf_graphics_receipt();
    test_receipt_null();
    printf("csb_v1_amiga_graphics_dat: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
