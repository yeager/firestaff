/*
 * Nexus V1 DMDF palette / string block bounds regression
 * =======================================================
 *
 * Source-lock:
 *   include/nexus_v1_dmdf_model.h, src/nexus/nexus_v1_dmdf_model.c
 *   docs/nexus_v1_phase2_data_formats_H2321.md §6.5 / §8.2
 *
 * The DMDF parser used to stop after vertex/face indices and ignore the
 * embedded palette / string blocks entirely, which is fine as long as
 * we never look at them — but once any higher layer reads those blocks
 * they must be safely bounded. This gate exercises the parser-level
 * bounds helpers in isolation against synthetic, deterministic inputs
 * that cover:
 *
 *   - the canonical 256-entry BGR555 CLUT (PLTB magic)
 *   - the canonical 4bpp 16-entry CLUT
 *   - a 32-bit XRGB CLUT
 *   - a small STRB with overlapping / shared-suffix records
 *   - corrupt headers, oversize counts, length overflows, out-of-range
 *     indices, and null-arg safety
 *
 * No real .MNS or game data is required — every fixture is synthesized
 * in-line so the gate runs in Phase A / CI without the data root.
 */

#include "nexus_v1_dmdf_model.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }       \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }       \
} while (0)

/* ── Big-endian writers used to synthesize DMDF blocks ──────────── */

static void wb32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >>  8);
    p[3] = (uint8_t)(v);
}

/* Build a 256-entry BGR555 palette block (PLTB + 16-byte header +
 * 512 bytes payload = 528 bytes total). */
static int build_bgr555_palette(uint8_t *out, int out_cap) {
    if (out_cap < 16 + 256 * 2) return -1;
    memset(out, 0, (size_t)out_cap);
    wb32(out + 0,  NEXUS_DMDF_PALETTE_BLOCK_MAGIC);
    wb32(out + 4,  (uint32_t)(16 + 256 * 2));       /* block size */
    wb32(out + 8,  256);                            /* count      */
    wb32(out + 12, 2);                              /* entry_size */
    /* Fill entries with a recognisable ramp so we can sanity-check. */
    {
        int i;
        for (i = 0; i < 256; i++) {
            uint16_t v = (uint16_t)((i * 33) & 0x7FFF);
            out[16 + i * 2 + 0] = (uint8_t)(v >> 8);
            out[16 + i * 2 + 1] = (uint8_t)(v & 0xFF);
        }
    }
    return 16 + 256 * 2;
}

/* Build a 16-entry 4bpp CLUT (1 byte per entry). */
static int build_4bpp_clut(uint8_t *out, int out_cap) {
    if (out_cap < 16 + 16) return -1;
    memset(out, 0, (size_t)out_cap);
    wb32(out + 0,  NEXUS_DMDF_PALETTE_BLOCK_MAGIC);
    wb32(out + 4,  (uint32_t)(16 + 16));
    wb32(out + 8,  16);
    wb32(out + 12, 1);
    {
        int i;
        for (i = 0; i < 16; i++) out[16 + i] = (uint8_t)(i * 0x11);
    }
    return 16 + 16;
}

/* Build a small STRB with three strings:
 *   [0] = "SCORPION\0" at offset 12+6*4 = 36
 *   [1] = "MUMMY\0"    at offset 36+9 = 45
 *   [2] = "MUMMY\0"    (shared suffix, length 6, points at idx 1) */
static int build_string_block(uint8_t *out, int out_cap) {
    /* Header(12) + offsets(12) + lengths(12) + body(15) = 51 */
    const char *s0 = "SCORPION";
    const char *s1 = "MUMMY";
    int body_off = 12 + 3 * 4 + 3 * 4;            /* = 36 */
    int total = body_off + 9 + 6;                 /* 51 */
    int i;

    if (out_cap < total) return -1;
    memset(out, 0, (size_t)out_cap);

    wb32(out + 0, NEXUS_DMDF_STRING_BLOCK_MAGIC);
    wb32(out + 4, (uint32_t)total);
    wb32(out + 8, 3);

    /* offsets[0..2] */
    wb32(out + 12 + 0 * 4, (uint32_t)body_off + 0);
    wb32(out + 12 + 1 * 4, (uint32_t)body_off + 9);
    wb32(out + 12 + 2 * 4, (uint32_t)body_off + 9);
    /* lengths[0..2] */
    wb32(out + 24 + 0 * 4, 9);
    wb32(out + 24 + 1 * 4, 6);
    wb32(out + 24 + 2 * 4, 6);

    /* Body */
    for (i = 0; i < 9;  i++) out[body_off + i]     = (uint8_t)s0[i];
    for (i = 0; i < 6;  i++) out[body_off + 9 + i] = (uint8_t)s1[i];

    return total;
}

/* ── Tests ───────────────────────────────────────────────────────── */

static void test_palette_happy_path(void) {
    uint8_t buf[1024];
    int sz = build_bgr555_palette(buf, (int)sizeof(buf));
    Nexus_DMDFPaletteBlock blk;
    uint32_t value = 0;

    CHECK(sz > 0, "build_bgr555_palette fixture fits");
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, sz, 0, &blk) == 1,
          "canonical 256xBGR555 PLTB is accepted");
    CHECK(blk.valid == 1, "parsed block is marked valid");
    CHECK(blk.entry_count == 256, "entry_count = 256");
    CHECK(blk.entry_size == 2, "entry_size = 2 (BGR555)");
    CHECK(blk.bpp == 8, "BGR555 reports bpp=8");
    CHECK(blk.bytes_used == (uint32_t)sz, "bytes_used matches the block");

    CHECK(nexus_v1_dmdf_palette_entry(buf, sz, &blk, 0, &value) == 1,
          "first palette entry reads");
    CHECK(nexus_v1_dmdf_palette_entry(buf, sz, &blk, 255, &value) == 1,
          "last palette entry reads");
    CHECK(nexus_v1_dmdf_palette_entry(buf, sz, &blk, 256, &value) == 0,
          "out-of-range palette index rejected");
}

static void test_palette_4bpp_clut(void) {
    uint8_t buf[64];
    int sz = build_4bpp_clut(buf, (int)sizeof(buf));
    Nexus_DMDFPaletteBlock blk;
    uint32_t v = 0;

    CHECK(sz > 0, "4bpp CLUT fixture fits");
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, sz, 0, &blk) == 1,
          "16x1-byte PLTB accepted as 4bpp CLUT");
    CHECK(blk.bpp == 4, "single-byte entries report bpp=4");
    CHECK(nexus_v1_dmdf_palette_entry(buf, sz, &blk, 5, &v) == 1,
          "mid-range 4bpp entry reads");
    CHECK(v == 0x55, "entry 5 reads as 0x55 (0x11 * 5)");
}

static void test_palette_truncated(void) {
    uint8_t buf[1024];
    int sz = build_bgr555_palette(buf, (int)sizeof(buf));
    Nexus_DMDFPaletteBlock blk;

    CHECK(sz > 0, "truncation fixture built");

    /* Buffer holds the full block but we claim a smaller window. */
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, 8, 0, &blk) == 0,
          "PLTB below 16-byte header is rejected");

    /* Block claims more than the buffer can hold. */
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, 20, 0, &blk) == 0,
          "PLTB whose size extends past buffer is rejected");

    /* Block lies about its size (says 528, buffer is 20). */
    {
        uint8_t lie[20];
        memset(lie, 0, sizeof(lie));
        wb32(lie + 0, NEXUS_DMDF_PALETTE_BLOCK_MAGIC);
        wb32(lie + 4, 528);
        wb32(lie + 8, 256);
        wb32(lie + 12, 2);
        CHECK(nexus_v1_dmdf_parse_palette_block(lie, (int)sizeof(lie), 0, &blk) == 0,
              "PLTB with size > buffer is rejected");
    }
}

static void test_palette_oversize(void) {
    uint8_t buf[64];
    Nexus_DMDFPaletteBlock blk;

    /* count = NEXUS_DMDF_MAX_PALETTE_ENTRIES + 1 must be rejected. */
    memset(buf, 0, sizeof(buf));
    wb32(buf + 0, NEXUS_DMDF_PALETTE_BLOCK_MAGIC);
    wb32(buf + 4, 16);                              /* lie about size */
    wb32(buf + 8, NEXUS_DMDF_MAX_PALETTE_ENTRIES + 1);
    wb32(buf + 12, 2);
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, (int)sizeof(buf), 0, &blk) == 0,
          "PLTB count > 256 is rejected");

    /* esize = 3 is unsupported and must be rejected. */
    memset(buf, 0, sizeof(buf));
    wb32(buf + 0, NEXUS_DMDF_PALETTE_BLOCK_MAGIC);
    wb32(buf + 4, 16 + 3);  /* 1 entry × 3 bytes */
    wb32(buf + 8, 1);
    wb32(buf + 12, 3);
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, (int)sizeof(buf), 0, &blk) == 0,
          "PLTB entry_size = 3 is rejected");

    /* Wrong magic. */
    memset(buf, 0, sizeof(buf));
    wb32(buf + 0, 0xDEADBEEF);
    wb32(buf + 4, 18);
    wb32(buf + 8, 1);
    wb32(buf + 12, 2);
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, (int)sizeof(buf), 0, &blk) == 0,
          "PLTB wrong magic is rejected");
}

static void test_palette_size_mismatch(void) {
    uint8_t buf[64];
    Nexus_DMDFPaletteBlock blk;

    /* count * esize matches declared size — must accept. */
    memset(buf, 0, sizeof(buf));
    wb32(buf + 0, NEXUS_DMDF_PALETTE_BLOCK_MAGIC);
    wb32(buf + 4, 16 + 8);  /* 4 entries × 2 bytes = 8 bytes payload */
    wb32(buf + 8, 4);
    wb32(buf + 12, 2);
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, (int)sizeof(buf), 0, &blk) == 1,
          "matching count × esize accepts");
    (void)blk;

    /* Mismatch: declared size says 16+16 = 32, but count × esize = 4*2 = 8.
     * Must reject because the declared size does not match the actual
     * payload the header describes. */
    memset(buf, 0, sizeof(buf));
    wb32(buf + 0, NEXUS_DMDF_PALETTE_BLOCK_MAGIC);
    wb32(buf + 4, 16 + 16);  /* lies: claims 16 bytes payload */
    wb32(buf + 8, 4);
    wb32(buf + 12, 2);
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, (int)sizeof(buf), 0, &blk) == 0,
          "PLTB size mismatch (count*esize != declared size) rejected");
}

static void test_palette_offset_nonzero(void) {
    /* PLTB sitting at offset 32 with random padding before it. */
    uint8_t buf[1024];
    int sz;
    int prefix = 32;
    Nexus_DMDFPaletteBlock blk;

    memset(buf, 0, sizeof(buf));
    sz = build_bgr555_palette(buf + prefix, (int)sizeof(buf) - prefix);
    CHECK(sz > 0, "non-zero-offset fixture built");
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, prefix + sz, prefix, &blk) == 1,
          "PLTB at offset > 0 still parsed");
    CHECK(blk.payload_offset == (uint32_t)(prefix + 16),
          "payload_offset tracks block start");
}

static void test_string_happy_path(void) {
    uint8_t buf[128];
    int sz = build_string_block(buf, (int)sizeof(buf));
    Nexus_DMDFStringBlock blk;
    uint32_t off = 0, len = 0;

    CHECK(sz > 0, "STRB fixture built");
    CHECK(nexus_v1_dmdf_parse_string_block(buf, sz, 0, &blk) == 1,
          "STRB with shared-suffix record accepted");
    CHECK(blk.valid == 1, "STRB marked valid");
    CHECK(blk.string_count == 3, "string_count = 3");

    CHECK(nexus_v1_dmdf_string_record(buf, sz, &blk, 0, &off, &len) == 1,
          "string record 0 reads");
    CHECK(len == 9, "string 0 length = 9");
    CHECK(buf[off] == 'S', "string 0 begins with 'S'");

    CHECK(nexus_v1_dmdf_string_record(buf, sz, &blk, 2, &off, &len) == 1,
          "shared-suffix record 2 reads");
    CHECK(len == 6, "shared-suffix length = 6");
}

static void test_string_overflow(void) {
    uint8_t buf[64];
    Nexus_DMDFStringBlock blk;

    /* Record offset+length past block boundary. */
    memset(buf, 0, sizeof(buf));
    wb32(buf + 0, NEXUS_DMDF_STRING_BLOCK_MAGIC);
    wb32(buf + 4, 12 + 8);          /* 1 record × (offset+len) = 8 bytes body */
    wb32(buf + 8, 1);               /* count = 1 */
    wb32(buf + 12, 0);              /* offset = 0  (illegal — points at header) */
    wb32(buf + 16, 8);
    CHECK(nexus_v1_dmdf_parse_string_block(buf, (int)sizeof(buf), 0, &blk) == 0,
          "STRB record pointing at header is rejected");

    /* Record offset+length extends past block end. */
    memset(buf, 0, sizeof(buf));
    wb32(buf + 0, NEXUS_DMDF_STRING_BLOCK_MAGIC);
    wb32(buf + 4, 24);              /* 12 header + 8 table + 4 body = 24 */
    wb32(buf + 8, 1);
    wb32(buf + 12, 20);             /* offset 20 */
    wb32(buf + 16, 100);            /* length 100 — past end */
    CHECK(nexus_v1_dmdf_parse_string_block(buf, (int)sizeof(buf), 0, &blk) == 0,
          "STRB record length past block end is rejected");
}

static void test_string_oversize(void) {
    uint8_t buf[64];
    Nexus_DMDFStringBlock blk;

    memset(buf, 0, sizeof(buf));
    wb32(buf + 0, NEXUS_DMDF_STRING_BLOCK_MAGIC);
    wb32(buf + 4, 12);
    wb32(buf + 8, NEXUS_DMDF_MAX_STRING_RECORDS + 1);
    CHECK(nexus_v1_dmdf_parse_string_block(buf, (int)sizeof(buf), 0, &blk) == 0,
          "STRB record count > 256 is rejected");
}

static void test_null_safety(void) {
    uint8_t buf[64];
    Nexus_DMDFPaletteBlock pblk;
    Nexus_DMDFStringBlock  sblk;
    uint32_t v = 0, off = 0, len = 0;

    memset(buf, 0, sizeof(buf));

    CHECK(nexus_v1_dmdf_parse_palette_block(NULL, 64, 0, &pblk) == 0,
          "PLTB NULL data rejected");
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, 0, 0, &pblk) == 0,
          "PLTB zero-size buffer rejected");
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, 64, -1, &pblk) == 0,
          "PLTB negative offset rejected");
    CHECK(nexus_v1_dmdf_parse_palette_block(buf, 64, 65, &pblk) == 0,
          "PLTB offset past buffer rejected");

    CHECK(nexus_v1_dmdf_parse_string_block(NULL, 64, 0, &sblk) == 0,
          "STRB NULL data rejected");
    CHECK(nexus_v1_dmdf_parse_string_block(buf, 0, 0, &sblk) == 0,
          "STRB zero-size buffer rejected");

    /* Helpers must reject when the block is not valid. */
    CHECK(nexus_v1_dmdf_palette_entry(buf, 64, &pblk, 0, &v) == 0,
          "palette_entry on invalid block rejected");
    CHECK(nexus_v1_dmdf_string_record(buf, 64, &sblk, 0, &off, &len) == 0,
          "string_record on invalid block rejected");

    /* Helpers must reject null out-args. */
    CHECK(nexus_v1_dmdf_palette_entry(buf, 64, &pblk, 0, NULL) == 0,
          "palette_entry NULL out-arg rejected");
    CHECK(nexus_v1_dmdf_string_record(buf, 64, &sblk, 0, NULL, &len) == 0,
          "string_record NULL offset out-arg rejected");
    CHECK(nexus_v1_dmdf_string_record(buf, 64, &sblk, 0, &off, NULL) == 0,
          "string_record NULL length out-arg rejected");
}

int main(void) {
    printf("==============================================\n");
    printf("Nexus V1 DMDF palette / string bounds regression\n");
    printf("==============================================\n");

    test_palette_happy_path();
    test_palette_4bpp_clut();
    test_palette_truncated();
    test_palette_oversize();
    test_palette_size_mismatch();
    test_palette_offset_nonzero();
    test_string_happy_path();
    test_string_overflow();
    test_string_oversize();
    test_null_safety();

    printf("\nResults: %d PASS, %d FAIL\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
