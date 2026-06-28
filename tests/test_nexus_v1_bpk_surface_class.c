#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_bpx_bpk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static void wr32_be(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static void wr16_be(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffu);
}

/* Build a synthetic 4-entry BPPK/BMPD archive with one directory trailer
 * (mode 10) and three PRS3-bearing picture entries (one each of the
 * four pixel modes: 6/14/22/30). Mirrors the observed MENU.BPK layout
 * from pass1082. */
static void make_synthetic_4entry_bpk(uint8_t *data, size_t cap) {
    const uint32_t entry0_off = 64U;
    const uint32_t entry1_off = 96U;
    const uint32_t entry2_off = 128U;
    const uint32_t entry3_off = 160U;
    const uint32_t payload_off = 192U;
    if (cap < payload_off + 64U) return;
    memset(data, 0, cap);
    memcpy(data + 0, "BPPK", 4);
    wr32_be(data + 4, (uint32_t)cap);
    memcpy(data + 12, "BMPD", 4);
    wr32_be(data + 16, (uint32_t)cap - 20U);
    wr32_be(data + 20, 4U);
    wr32_be(data + 24, entry0_off);
    wr32_be(data + 28, entry1_off);
    wr32_be(data + 32, entry2_off);
    wr32_be(data + 36, entry3_off);

    /* Entry 0: directory trailer (mode tag 10). The first 8 bytes of
     * the 20-byte prefix are BE uint32 offsets to the last two real
     * picture entries (mimicking the real MENU.BPK contract). */
    {
        uint8_t *p = data + entry0_off;
        wr32_be(p + 0, entry2_off); /* points to entry 2 */
        wr32_be(p + 4, entry3_off); /* points to entry 3 */
        p[15] = 0x00;               /* height (irrelevant) */
        p[19] = NEXUS_V1_BPK_MODE_TRAILER;
    }

    /* Entry 1: 4x4 indexed 8bpp (mode 6) — 16 pixels = 16 unpacked bytes. */
    {
        uint8_t *p = data + entry1_off;
        wr16_be(p + 12, 4U);
        p[15] = 4U;
        p[19] = NEXUS_V1_BPK_MODE_8BPP;
        memcpy(p + 20, "PRS3", 4);
        wr32_be(p + 24, 1U);          /* version */
        wr32_be(p + 28, 16U);         /* pixel count */
    }

    /* Entry 2: 8x4 RGB565 (mode 14) — 32 pixels = 64 unpacked bytes. */
    {
        uint8_t *p = data + entry2_off;
        wr16_be(p + 12, 8U);
        p[15] = 4U;
        p[19] = NEXUS_V1_BPK_MODE_16BPP;
        memcpy(p + 20, "PRS3", 4);
        wr32_be(p + 24, 1U);
        wr32_be(p + 28, 32U);
    }

    /* Entry 3: 2x3 RGB888 (mode 22) — 6 pixels = 18 unpacked bytes. */
    {
        uint8_t *p = data + entry3_off;
        wr16_be(p + 12, 2U);
        p[15] = 3U;
        p[19] = NEXUS_V1_BPK_MODE_24BPP;
        memcpy(p + 20, "PRS3", 4);
        wr32_be(p + 24, 1U);
        wr32_be(p + 28, 6U);
    }

    /* Payload region: opaque bytes (uncompressed blobs only). */
    memset(data + payload_off, 0xa5, 64U);
}

/* ---- Surface-class lookup tests ---- */

static void test_mode_to_surface_class(void) {
    expect(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_8BPP) ==
               NEXUS_V1_BPK_SURFACE_INDEXED_8BPP,
           "mode 6 -> SURFACE_INDEXED_8BPP");
    expect(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_16BPP) ==
               NEXUS_V1_BPK_SURFACE_RGB565,
           "mode 14 -> SURFACE_RGB565");
    expect(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_24BPP) ==
               NEXUS_V1_BPK_SURFACE_RGB888,
           "mode 22 -> SURFACE_RGB888");
    expect(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_32BPP) ==
               NEXUS_V1_BPK_SURFACE_RGBA8888,
           "mode 30 -> SURFACE_RGBA8888");
    expect(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_TRAILER) ==
               NEXUS_V1_BPK_SURFACE_DIRECTORY_TRAILER,
           "mode 10 -> SURFACE_DIRECTORY_TRAILER");
    expect(nexus_v1_bpk_mode_to_surface_class(7) ==
               NEXUS_V1_BPK_SURFACE_UNKNOWN,
           "mode 7 -> SURFACE_UNKNOWN");
    expect(nexus_v1_bpk_mode_to_surface_class(0) ==
               NEXUS_V1_BPK_SURFACE_UNKNOWN,
           "mode 0 -> SURFACE_UNKNOWN");
    expect(nexus_v1_bpk_mode_to_surface_class(255) ==
               NEXUS_V1_BPK_SURFACE_UNKNOWN,
           "mode 255 -> SURFACE_UNKNOWN");
}

static void test_mode_to_bpp(void) {
    expect(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_8BPP) == 1U,
           "mode 6 -> 1 bpp");
    expect(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_16BPP) == 2U,
           "mode 14 -> 2 bpp");
    expect(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_24BPP) == 3U,
           "mode 22 -> 3 bpp");
    expect(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_32BPP) == 4U,
           "mode 30 -> 4 bpp");
    expect(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_TRAILER) == 0U,
           "mode 10 -> 0 bpp");
    expect(nexus_v1_bpk_mode_to_bpp(7) == 0U, "mode 7 -> 0 bpp (unknown)");
    expect(nexus_v1_bpk_mode_to_bpp(0) == 0U, "mode 0 -> 0 bpp (unknown)");
}

/* ---- Synthetic 4-entry BPK surface_estimate test ---- */

static void test_synthetic_surface_estimate(void) {
    uint8_t data[256];
    Nexus_V1_BpkSurfaceEntry entries[8];
    Nexus_V1_BpkSurfaceEstimate summary;
    int rc;

    memset(entries, 0, sizeof(entries));
    memset(&summary, 0, sizeof(summary));
    make_synthetic_4entry_bpk(data, sizeof(data));

    rc = nexus_v1_bpk_archive_surface_estimate(data, sizeof(data),
                                                entries,
                                                (uint32_t)(sizeof(entries) /
                                                    sizeof(entries[0])),
                                                &summary);
    expect(rc == 0, "synthetic 4-entry BPK surface_estimate returns 0");

    /* The trailer (entry 0) is skipped. We have entries 1/2/3 only. */
    expect(summary.total_with_surface == 3U,
           "synthetic archive: 3 PRS3-bearing entries with a surface");
    expect(summary.trailer_skipped == 1U,
           "synthetic archive: 1 directory-trailer entry skipped");
    expect(summary.unknown_skipped == 0U,
           "synthetic archive: 0 unknown-mode entries");
    expect(summary.used == 3U, "synthetic archive: 3 entries written");

    /* Entry 1 (8bpp 4x4): rowstride = 4 bytes, surface = 16 bytes. */
    expect(entries[0].entry_index == 1U, "first surface entry is index 1");
    expect(entries[0].mode == NEXUS_V1_BPK_MODE_8BPP &&
               entries[0].width == 4U && entries[0].height == 4U,
           "first surface entry is 4x4 indexed 8bpp");
    expect(entries[0].pixel_count == 16U,
           "first surface entry pixel_count = 16");
    expect(entries[0].layout.bpp == 1U &&
               entries[0].layout.rowstride == 4U &&
               entries[0].layout.surface_bytes == 16U,
           "first surface entry: bpp=1 rowstride=4 surface=16");
    expect(entries[0].layout.surface_class ==
               NEXUS_V1_BPK_SURFACE_INDEXED_8BPP,
           "first surface entry class is INDEXED_8BPP");

    /* Entry 2 (RGB565 8x4): rowstride = 16 bytes, surface = 64 bytes. */
    expect(entries[1].entry_index == 2U, "second surface entry is index 2");
    expect(entries[1].mode == NEXUS_V1_BPK_MODE_16BPP &&
               entries[1].width == 8U && entries[1].height == 4U,
           "second surface entry is 8x4 RGB565");
    expect(entries[1].pixel_count == 32U, "second surface pixel_count = 32");
    expect(entries[1].layout.bpp == 2U &&
               entries[1].layout.rowstride == 16U &&
               entries[1].layout.surface_bytes == 64U,
           "second surface entry: bpp=2 rowstride=16 surface=64");
    expect(entries[1].layout.surface_class ==
               NEXUS_V1_BPK_SURFACE_RGB565,
           "second surface entry class is RGB565");

    /* Entry 3 (RGB888 2x3): rowstride = 6 bytes, surface = 18 bytes. */
    expect(entries[2].entry_index == 3U, "third surface entry is index 3");
    expect(entries[2].mode == NEXUS_V1_BPK_MODE_24BPP &&
               entries[2].width == 2U && entries[2].height == 3U,
           "third surface entry is 2x3 RGB888");
    expect(entries[2].pixel_count == 6U, "third surface pixel_count = 6");
    expect(entries[2].layout.bpp == 3U &&
               entries[2].layout.rowstride == 6U &&
               entries[2].layout.surface_bytes == 18U,
           "third surface entry: bpp=3 rowstride=6 surface=18");
    expect(entries[2].layout.surface_class ==
               NEXUS_V1_BPK_SURFACE_RGB888,
           "third surface entry class is RGB888");

    /* Total surface bytes: 16 + 64 + 18 = 98. */
    expect(summary.total_surface_bytes == 98U,
           "synthetic archive total surface bytes = 98");
}

/* ---- Synthetic BPX3 directory-trailer entry ---- */

static void test_bpx3_trailer_entry(void) {
    /* BPX3 archive: 2 entries, where entry 0 is a directory trailer.
     *   bytes 0..4   : "BPX3" magic
     *   bytes 4..6   : version u16 = 1
     *   bytes 6..8   : count u16 = 2
     *   bytes 8..12  : table_offset u32 = 16
     *   bytes 12..16 : data_offset u32 = 80  (16 + 2 * 32)
     *   bytes 16..48 : entry 0 (trailer, mode 10)
     *   bytes 48..80 : entry 1 (16x15 14bpp picture)
     *   bytes 80..  : payload blob
     */
    uint8_t buf[160];
    const uint32_t table_offset = NEXUS_V1_BPX0_HEADER_SIZE;
    const uint32_t data_offset = NEXUS_V1_BPX0_HEADER_SIZE +
        (2u * NEXUS_V1_BPX0_ENTRY_SIZE);
    const uint8_t payload[16] = {0xa0, 0xa1, 0xa2, 0xa3, 0xb0, 0xb1, 0xb2, 0xb3,
                                 0xc0, 0xc1, 0xc2, 0xc3, 0xd0, 0xd1, 0xd2, 0xd3};
    Nexus_V1_BpxBpkArchive archive;
    const Nexus_V1_BpxBpkEntry *trailer;
    const Nexus_V1_BpxBpkEntry *picture;
    int rc;

    memset(buf, 0, sizeof(buf));
    memcpy(buf, "BPX3", 4);
    wr16_be(buf + 4, 1);
    wr16_be(buf + 6, 2);
    wr32_be(buf + 8, table_offset);
    wr32_be(buf + 12, data_offset);

    /* Entry 0: directory trailer. */
    {
        uint8_t *r = buf + table_offset;
        memset(r, 0, NEXUS_V1_BPX0_ENTRY_SIZE);
        memcpy(r, "TRAILER", 7);
        /* width u16 == 0, mode tag byte == MODE_TRAILER (10),
         * height u8 == 0, reserved == 0. */
        r[18] = NEXUS_V1_BPK_MODE_TRAILER;
    }

    /* Entry 1: 16x15 14bpp picture. */
    {
        uint8_t *r = buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE;
        memset(r, 0, NEXUS_V1_BPX0_ENTRY_SIZE);
        memcpy(r, "MENU.PIC", 8);
        wr16_be(r + 16, 16U);
        r[18] = NEXUS_V1_BPK_MODE_16BPP;
        r[19] = 15U;
        wr32_be(r + 20, 16U * 15U);
        wr32_be(r + 24, data_offset);
        wr32_be(r + 28, (uint32_t)sizeof(payload));
    }

    memcpy(buf + data_offset, payload, sizeof(payload));

    memset(&archive, 0, sizeof(archive));
    rc = nexus_v1_bpx_prs3_parse(buf, data_offset + sizeof(payload),
                                 &archive);
    expect(rc == NEXUS_V1_BPX_BPK_OK, "BPX3 with trailer entry parses ok");
    expect(archive.entry_count == 2U, "BPX3 has 2 entries");
    expect(archive.format == NEXUS_V1_BPX_BPK_FORMAT_SYNTHETIC_PRS3,
           "BPX3 format is recorded as SYNTHETIC_PRS3");

    trailer = &archive.entries[0];
    expect(trailer->method == NEXUS_V1_BPX_BPK_METHOD_DIRECTORY_TRAILER,
           "trailer entry method is DIRECTORY_TRAILER");
    expect(trailer->mode == NEXUS_V1_BPK_MODE_TRAILER,
           "trailer entry mode tag == 10");
    expect(trailer->has_prs3_magic == 0,
           "trailer entry has no PRS3 magic");
    expect(trailer->offset == 0U && trailer->packed_size == 0U &&
               trailer->unpacked_size == 0U,
           "trailer entry carries no payload");
    expect(trailer->width == 0U && trailer->height == 0U &&
               trailer->pixel_count == 0U,
           "trailer entry has zero width/height/pixel_count");

    picture = &archive.entries[1];
    expect(picture->method == NEXUS_V1_BPX_BPK_METHOD_PRS3_UNKNOWN,
           "picture entry method is PRS3_UNKNOWN");
    expect(picture->width == 16U && picture->height == 15U &&
               picture->mode == NEXUS_V1_BPK_MODE_16BPP &&
               picture->pixel_count == 16U * 15U &&
               picture->has_prs3_magic == 1,
           "picture entry preserves 16x15 14bpp / 240 pixels / PRS3 marker");
    expect(picture->offset == data_offset &&
               picture->packed_size == (uint32_t)sizeof(payload) &&
               picture->unpacked_size == 16U * 15U * 2U,
           "picture entry carries bounded packed span and 2-byte RGB565 surface size");
}

/* Trailer entries must reject width != 0 / height != 0 / nonzero tail. */
static void test_bpx3_trailer_rejections(void) {
    uint8_t buf[96];
    Nexus_V1_BpxBpkArchive archive;

    /* Bad trailer: width != 0. */
    memset(buf, 0, sizeof(buf));
    memcpy(buf, "BPX3", 4);
    wr16_be(buf + 4, 1);
    wr16_be(buf + 6, 1);
    wr32_be(buf + 8, NEXUS_V1_BPX0_HEADER_SIZE);
    wr32_be(buf + 12, NEXUS_V1_BPX0_HEADER_SIZE + NEXUS_V1_BPX0_ENTRY_SIZE);
    memcpy(buf + 16, "TRAILER", 7);
    wr16_be(buf + 16 + 16, 1U); /* width must be 0 for a trailer */
    buf[16 + 18] = NEXUS_V1_BPK_MODE_TRAILER;
    buf[16 + 19] = 0U;
    expect(nexus_v1_bpx_prs3_parse(buf, sizeof(buf), &archive) ==
               NEXUS_V1_BPX_BPK_ERR_BOUNDS,
           "BPX3 rejects trailer with nonzero width");

    /* Bad trailer: nonzero reserved (offset). */
    memset(buf, 0, sizeof(buf));
    memcpy(buf, "BPX3", 4);
    wr16_be(buf + 4, 1);
    wr16_be(buf + 6, 1);
    wr32_be(buf + 8, NEXUS_V1_BPX0_HEADER_SIZE);
    wr32_be(buf + 12, NEXUS_V1_BPX0_HEADER_SIZE + NEXUS_V1_BPX0_ENTRY_SIZE);
    memcpy(buf + 16, "TRAILER", 7);
    buf[16 + 18] = NEXUS_V1_BPK_MODE_TRAILER;
    buf[16 + 24] = 0xFFu; /* reserved must be zero */
    expect(nexus_v1_bpx_prs3_parse(buf, sizeof(buf), &archive) ==
               NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED,
           "BPX3 rejects trailer with nonzero reserved bytes");
}

static void test_bpx3_prs3_span_rejections(void) {
    uint8_t buf[160];
    const uint32_t table_offset = NEXUS_V1_BPX0_HEADER_SIZE;
    const uint32_t data_offset = NEXUS_V1_BPX0_HEADER_SIZE +
        (2u * NEXUS_V1_BPX0_ENTRY_SIZE);
    Nexus_V1_BpxBpkArchive archive;

    memset(buf, 0, sizeof(buf));
    memcpy(buf, "BPX3", 4);
    wr16_be(buf + 4, 1);
    wr16_be(buf + 6, 2);
    wr32_be(buf + 8, table_offset);
    wr32_be(buf + 12, data_offset);

    /* Entry 0: 4x4 indexed picture at data_offset, 8 packed bytes. */
    memcpy(buf + table_offset, "PIC0", 4);
    wr16_be(buf + table_offset + 16, 4U);
    buf[table_offset + 18] = NEXUS_V1_BPK_MODE_8BPP;
    buf[table_offset + 19] = 4U;
    wr32_be(buf + table_offset + 20, 16U);
    wr32_be(buf + table_offset + 24, data_offset);
    wr32_be(buf + table_offset + 28, 8U);

    /* Entry 1: 4x4 indexed picture whose span overlaps entry 0. */
    memcpy(buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE, "PIC1", 4);
    wr16_be(buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE + 16, 4U);
    buf[table_offset + NEXUS_V1_BPX0_ENTRY_SIZE + 18] =
        NEXUS_V1_BPK_MODE_8BPP;
    buf[table_offset + NEXUS_V1_BPX0_ENTRY_SIZE + 19] = 4U;
    wr32_be(buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE + 20, 16U);
    wr32_be(buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE + 24,
            data_offset + 4U);
    wr32_be(buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE + 28, 8U);
    expect(nexus_v1_bpx_prs3_parse(buf, sizeof(buf), &archive) ==
               NEXUS_V1_BPX_BPK_ERR_BOUNDS,
           "BPX3 rejects overlapping PRS3 packed payload spans");

    /* Move entry 1 past entry 0, then make it run beyond EOF. */
    wr32_be(buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE + 24,
            data_offset + 8U);
    wr32_be(buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE + 28, 4096U);
    expect(nexus_v1_bpx_prs3_parse(buf, sizeof(buf), &archive) ==
               NEXUS_V1_BPX_BPK_ERR_BOUNDS,
           "BPX3 rejects PRS3 packed payload span past EOF");

    /* Zero-size payloads are not a useful synthetic PRS3 contract. */
    wr32_be(buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE + 28, 0U);
    expect(nexus_v1_bpx_prs3_parse(buf, sizeof(buf), &archive) ==
               NEXUS_V1_BPX_BPK_ERR_BOUNDS,
           "BPX3 rejects zero-size PRS3 packed payload span");
}

/* ---- Optional real MENU.BPK receipt ---- */

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *fp = fopen(path, "rb");
    long size;
    uint8_t *data;
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    size = ftell(fp);
    if (size <= 0) { fclose(fp); return 0; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return 0; }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) { fclose(fp); return 0; }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data); fclose(fp); return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static void test_optional_local_menu_bpk(void) {
    const char *home = getenv("HOME");
    char path[1024];
    uint8_t *data = NULL;
    size_t size = 0;
    Nexus_V1_BpkSurfaceEntry entries[200];
    Nexus_V1_BpkSurfaceEstimate summary;
    uint32_t indexed = 0U, rgb565 = 0U, rgb888 = 0U, rgba32 = 0U;
    uint64_t expected_total = 0U;

    if (!home || !home[0]) {
        puts("SKIP: HOME is unset; no local Nexus MENU.BPK check");
        return;
    }
    if (snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/MENU.BPK",
                 home) < 0) return;
    if (!read_file(path, &data, &size)) {
        puts("SKIP: local Nexus MENU.BPK not present");
        return;
    }

    memset(entries, 0, sizeof(entries));
    memset(&summary, 0, sizeof(summary));
    expect(nexus_v1_bpk_archive_surface_estimate(
               data, size, entries,
               (uint32_t)(sizeof(entries) / sizeof(entries[0])),
               &summary) == 0,
           "local MENU.BPK surface_estimate returns 0");
    expect(summary.total_with_surface == 162U,
           "local MENU.BPK: 162 PRS3-bearing entries with a surface");
    expect(summary.trailer_skipped == 1U,
           "local MENU.BPK: 1 directory-trailer entry skipped");
    expect(summary.unknown_skipped == 0U,
           "local MENU.BPK: 0 unknown-mode entries");
    expect(summary.used == 162U,
           "local MENU.BPK: 162 entries written to the output array");

    /* Cross-check the mode bucket counts against the mode_distribution. */
    for (uint32_t i = 0; i < summary.used; ++i) {
        switch (entries[i].layout.surface_class) {
        case NEXUS_V1_BPK_SURFACE_INDEXED_8BPP: ++indexed; break;
        case NEXUS_V1_BPK_SURFACE_RGB565:       ++rgb565; break;
        case NEXUS_V1_BPK_SURFACE_RGB888:       ++rgb888; break;
        case NEXUS_V1_BPK_SURFACE_RGBA8888:     ++rgba32; break;
        default: break;
        }
        expected_total += entries[i].layout.surface_bytes;
        /* Every entry's rowstride must equal width * bpp. */
        expect(entries[i].layout.rowstride ==
                   (uint32_t)entries[i].width *
                       entries[i].layout.bpp,
               "local MENU.BPK rowstride == width * bpp");
        /* Every entry's surface_bytes must equal width * height * bpp. */
        expect(entries[i].layout.surface_bytes ==
                   (uint32_t)entries[i].width *
                       (uint32_t)entries[i].height *
                       entries[i].layout.bpp,
               "local MENU.BPK surface_bytes == w*h*bpp");
    }
    expect(indexed == 14U, "local MENU.BPK: 14 indexed 8bpp entries");
    expect(rgb565 == 62U, "local MENU.BPK: 62 RGB565 entries");
    expect(rgb888 == 39U, "local MENU.BPK: 39 RGB888 entries");
    expect(rgba32 == 47U, "local MENU.BPK: 47 RGBA8888 entries");
    expect(expected_total == summary.total_surface_bytes,
           "local MENU.BPK: per-entry sum equals summary total");
    expect(summary.total_surface_bytes > 0U,
           "local MENU.BPK: summary total surface bytes > 0");

    free(data);
}

int main(void) {
    test_mode_to_surface_class();
    test_mode_to_bpp();
    test_synthetic_surface_estimate();
    test_bpx3_trailer_entry();
    test_bpx3_trailer_rejections();
    test_bpx3_prs3_span_rejections();
    test_optional_local_menu_bpk();

    if (g_failures) return 1;
    puts("test_nexus_v1_bpk_surface_class: PASS");
    return 0;
}
