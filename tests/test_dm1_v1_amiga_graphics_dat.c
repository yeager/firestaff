#include "dm1_v1_amiga_graphics_dat.h"
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

typedef struct {
    int found;
    int valid;
    int executable_found;
    unsigned int immediate_color_writes;
    unsigned int copper_color_base_writes;
    unsigned int copper_caller_palette_handoffs;
    DM1_V1_AmigaGraphicsReceipt receipt;
    uint8_t *bytes;
    size_t size;
} RealGraphicsReceipt;

static int real_graphics_visitor(const char *name, const uint8_t *bytes,
                                 size_t size, void *user_data) {
    RealGraphicsReceipt *result = (RealGraphicsReceipt *)user_data;
    const char *disassembly = getenv("FIRESTAFF_DM1_AMIGA_DISASSEMBLY");
    if (!name || !bytes || !result) {
        return 1;
    }
    /* Amiga OCS COLOR00..COLOR31 live at 0xdff180..0xdff1be.  This optional
     * in-memory probe records the original executable's palette route; it
     * never materializes an ADF member.  DM 2.0 constructs Copper entries
     * with ADD.L #$00dff180,D0 rather than embedding MOVE.W #rgb,COLORxx. */
    if (disassembly && disassembly[0] && strcmp(name, "dm") == 0) {
        result->executable_found = 1;
        printf("AMIGA-DISASM dm bytes=%zu\n", size);
        for (size_t i = 0u; i + 10u <= size; ++i) {
            unsigned int register_offset;
            unsigned int rgb4;
            if (bytes[i] != 0x33u || bytes[i + 1u] != 0xfcu ||
                bytes[i + 4u] != 0x00u || bytes[i + 5u] != 0xdfu ||
                bytes[i + 6u] != 0xf1u || bytes[i + 7u] < 0x80u ||
                bytes[i + 7u] > 0xbeu || (bytes[i + 7u] & 1u) != 0u) {
                continue;
            }
            rgb4 = ((unsigned int)bytes[i + 2u] << 8) | bytes[i + 3u];
            register_offset = (unsigned int)bytes[i + 7u] - 0x80u;
            printf("AMIGA-DISASM move.w #$%03x,COLOR%u @0x%zx\n",
                   rgb4 & 0xfffu, register_offset / 2u, i);
            ++result->immediate_color_writes;
        }
        for (size_t i = 0u; i + 6u <= size; ++i) {
            if (bytes[i + 0u] == 0xd0u && bytes[i + 1u] == 0xbcu &&
                bytes[i + 2u] == 0x00u && bytes[i + 3u] == 0xdfu &&
                bytes[i + 4u] == 0xf1u && bytes[i + 5u] == 0x80u) {
                printf("AMIGA-DISASM add.l #$00dff180,D0 @0x%zx\n", i);
                ++result->copper_color_base_writes;
            }
        }
        /* The builder's prologue transfers its caller's word-table pointer
         * from 12(A5), then its first loop reads 16 words from that table
         * before forming COLOR00..COLOR15 Copper addresses.  This is an
         * original dynamic gameplay-palette handoff, not a static palette
         * embedded in GRAPHICS.DAT or a legitimate PC-VGA fallback. */
        for (size_t i = 0u; i + 10u <= size; ++i) {
            if (bytes[i + 0u] == 0x4eu && bytes[i + 1u] == 0x55u &&
                bytes[i + 2u] == 0x00u && bytes[i + 3u] == 0x00u &&
                bytes[i + 4u] == 0x2fu && bytes[i + 5u] == 0x04u &&
                bytes[i + 6u] == 0x29u && bytes[i + 7u] == 0x6du &&
                bytes[i + 8u] == 0x00u && bytes[i + 9u] == 0x0cu) {
                printf("AMIGA-DISASM Copper caller palette handoff @0x%zx\n",
                       i);
                ++result->copper_caller_palette_handoffs;
            }
        }
        printf("AMIGA-DISASM COLOR immediate writes=%u, Copper COLOR base writes=%u, caller palette handoffs=%u\n",
               result->immediate_color_writes,
               result->copper_color_base_writes,
               result->copper_caller_palette_handoffs);
    }
    if (strcmp(name, "graphics.dat") != 0) return 1;
    result->found = 1;
    result->valid = dm1_v1_amiga_graphics_receipt(bytes, size,
                                                   &result->receipt) == 0;
    if (result->valid) {
        result->bytes = malloc(size);
        if (!result->bytes) return -1;
        memcpy(result->bytes, bytes, size);
        result->size = size;
    }
    return 0;
}

/* The supplied Amiga 2.0 preservation package is ZIP -> ZIP -> ADF. Read
 * its selected disk and GRAPHICS.DAT entirely in memory; no archive member
 * is materialized to the filesystem. */
static void test_real_amiga_v20_graphics_receipt(void) {
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    uint8_t *inner = NULL;
    uint8_t *adf = NULL;
    size_t inner_size = 0U;
    size_t adf_size = 0U;
    FILE *stream;
    RealGraphicsReceipt result;

    if (!archive || !archive[0]) {
        printf("SKIP real_amiga_v20_graphics: archive not configured\n");
        return;
    }
    stream = fopen(archive, "rb");
    if (!stream) {
        printf("SKIP real_amiga_v20_graphics: archive unavailable\n");
        return;
    }
    fclose(stream);

    memset(&result, 0, sizeof(result));
    CHECK(firestaff_zip_extract_by_suffix(
              archive, "Dungeon Master v2.0 (1988)(FTL).zip", &inner,
              &inner_size) == 0,
          "real_outer_zip_member");
    if (!inner) return;
    CHECK(firestaff_zip_extract_memory_by_suffix(
              inner, inner_size, "Dungeon Master v2.0 (1988)(FTL).adf",
              &adf, &adf_size) == 0,
          "real_inner_adf_member");
    free(inner);
    if (!adf) return;
    CHECK(firestaff_amiga_adf_visit_ofs_files(adf, adf_size,
                                               real_graphics_visitor,
                                               &result) >= 0,
          "real_adf_visit");
    free(adf);
    CHECK(result.found == 1, "real_graphics_found");
    if (getenv("FIRESTAFF_DM1_AMIGA_DISASSEMBLY")) {
        CHECK(result.executable_found == 1, "real_executable_found");
        CHECK(result.immediate_color_writes == 0u,
              "real_no_immediate_color_writes");
        CHECK(result.copper_color_base_writes == 4u,
              "real_copper_color_base_writes");
        CHECK(result.copper_caller_palette_handoffs == 1u,
              "real_copper_caller_palette_handoff");
    }
    CHECK(result.valid == 1, "real_graphics_receipt");
    if (!result.valid) return;
    CHECK(result.receipt.is_amiga == 1, "real_graphics_is_amiga");
    CHECK(result.receipt.graphic_count == DM1_AMIGA_GRAPHICS_EXPECTED_COUNT,
          "real_graphics_count");
    CHECK(result.receipt.lang == DM1_AMIGA_LANG_EN, "real_graphics_lang_en");
    CHECK(result.receipt.version == DM1_AMIGA_VER_2_0,
          "real_graphics_version_v20");
    {
        uint8_t pixels[640u * 400u];
        uint16_t width = 0u, height = 0u;
        size_t index;
        unsigned int nonzero = 0u;
        CHECK(dm1_v1_amiga_graphics_decode(result.bytes, result.size, 0u,
                                            pixels, sizeof(pixels),
                                            &width, &height) == 1,
              "real_item_000_decode");
        CHECK(width > 0u && height > 0u && width <= 640u && height <= 400u,
              "real_item_000_dimensions");
        for (index = 0u; index < (size_t)width * height; ++index)
            nonzero += pixels[index] != 0u;
        CHECK(nonzero > 0u, "real_item_000_source_pixels");
    }
    free(result.bytes);
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
    test_real_amiga_v20_graphics_receipt();
    printf("dm1_v1_amiga_graphics_dat: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
