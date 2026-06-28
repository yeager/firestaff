/*
 * firestaff_nexus_v1_menumenu_bpk_inspect_probe.c
 * =================================================
 *
 * pass1082 — real-asset MENU.BPK byte-level boundary inspection.
 *
 * Walks the observed DM Nexus MENU.BPK structure (BPPK outer wrapper,
 * BMPD directory, 163 big-endian candidate offsets, 162 PRS3 payload
 * markers, one raw directory-trailer entry) without claiming any PRS3
 * decompression. Locks:
 *
 *   - BPPK magic + outer_size == file_size
 *   - BMPD magic + bmpd_size == outer_size - 16
 *   - count == 163 (header + 162 picture entries)
 *   - 20-byte entry prefix: width @ 12..14 (BE u16), height @ 15,
 *     mode tag @ 19 (one of {6, 14, 22, 30, 10})
 *   - PRS3 sub-header: magic @ +20, version @ +24 == 0x00000001,
 *     pixel count @ +28 == width * height (162/162 entries)
 *   - entry[0] is a directory trailer (mode tag 10) whose prefix
 *     points to the last two real entries
 *   - mode distribution: 6 -> 14, 14 -> 62, 22 -> 39, 30 -> 47,
 *     trailer -> 1 (sum = 163)
 *
 * Source-lock / provenance:
 *   docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md:291-306
 *     (MENU.BPK identified as packed, game-specific, no formal
 *      compression analysis documented)
 *   docs/VERIFIED_HASHES.md:103
 *     (size 89060 / sha256 740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636)
 *   ReDMCSB has no Saturn/Nexus implementation, so the structural
 *     invariants below are reverse-engineered from the real MENU.BPK
 *     byte layout and explicitly do NOT decode the PRS3 stream.
 *
 * Optional path: when `~/.firestaff/data/nexus/MENU.BPK` is present the
 * probe enforces the observed distribution; otherwise it falls back to
 * a data-free synthetic BPX3 archive that locks the same width /
 * height / mode / PRS3-magic contract so the boundary shape can be
 * exercised in CI without copyrighted assets.
 */

#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_bpx_bpk.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                      \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }          \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }          \
} while (0)

static void wb16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)((v >> 8) & 0xffu);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wb32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)((v >> 24) & 0xffu);
    p[1] = (uint8_t)((v >> 16) & 0xffu);
    p[2] = (uint8_t)((v >> 8) & 0xffu);
    p[3] = (uint8_t)(v & 0xffu);
}

/* Build a synthetic BPX3 archive that exercises the new MENU.BPK-shaped
 * PRS3 contract: each entry carries width/height/mode/pixel_count, an
 * exact packed payload span, and an implicit PRS3 magic + 0x00000001
 * version. */
static size_t make_synthetic_bpx3(uint8_t *buf, size_t cap) {
    const uint32_t table_offset = NEXUS_V1_BPX0_HEADER_SIZE;
    /* 3 entries of NEXUS_V1_BPX0_ENTRY_SIZE bytes each, so the table
     * ends at offset 16 + 96 = 112 and data starts there. */
    const uint32_t data_offset = NEXUS_V1_BPX0_HEADER_SIZE +
        (3u * NEXUS_V1_BPX0_ENTRY_SIZE);
    /* Three PRS3 entries followed by three opaque payload blobs. */
    const uint8_t blob0[16] = {0xa0, 0xa1, 0xa2, 0xa3, 0xb0, 0xb1, 0xb2, 0xb3,
                               0xc0, 0xc1, 0xc2, 0xc3, 0xd0, 0xd1, 0xd2, 0xd3};
    const uint8_t blob1[8]  = {0x10, 0x11, 0x12, 0x13,
                               0x20, 0x21, 0x22, 0x23};
    const uint8_t blob2[4]  = {0xee, 0xef, 0xf0, 0xf1};
    uint8_t *r;

    if (cap < data_offset + sizeof(blob0) + sizeof(blob1) + sizeof(blob2)) {
        return 0;
    }
    memset(buf, 0, cap);
    memcpy(buf, "BPX3", 4);
    wb16(buf + 4, 1);                  /* version */
    wb16(buf + 6, 3);                  /* entry_count */
    wb32(buf + 8, table_offset);       /* table_offset */
    wb32(buf + 12, data_offset);       /* data_offset */

    /* Entry 0: 16x15 14bpp, 240 pixels. */
    r = buf + table_offset;
    memset(r, 0, NEXUS_V1_BPX0_ENTRY_SIZE);
    strncpy((char *)r, "MENU.A", 15);
    wb16(r + 16, 16);                  /* width */
    r[18] = NEXUS_V1_BPK_MODE_16BPP;   /* mode tag */
    r[19] = 15;                        /* height */
    wb32(r + 20, 16u * 15u);           /* pixel_count */
    wb32(r + 24, data_offset);         /* payload offset */
    wb32(r + 28, (uint32_t)sizeof(blob0)); /* packed payload size */

    /* Entry 1: 54x31 6bpp, 1674 pixels. */
    r = buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE;
    memset(r, 0, NEXUS_V1_BPX0_ENTRY_SIZE);
    strncpy((char *)r, "MENU.B", 15);
    wb16(r + 16, 54);
    r[18] = NEXUS_V1_BPK_MODE_8BPP;
    r[19] = 31;
    wb32(r + 20, 54u * 31u);
    wb32(r + 24, (uint32_t)(data_offset + sizeof(blob0)));
    wb32(r + 28, (uint32_t)sizeof(blob1));

    /* Entry 2: 8x8 32bpp, 64 pixels. */
    r = buf + table_offset + 2u * NEXUS_V1_BPX0_ENTRY_SIZE;
    memset(r, 0, NEXUS_V1_BPX0_ENTRY_SIZE);
    strncpy((char *)r, "MENU.C", 15);
    wb16(r + 16, 8);
    r[18] = NEXUS_V1_BPK_MODE_32BPP;
    r[19] = 8;
    wb32(r + 20, 8u * 8u);
    wb32(r + 24, (uint32_t)(data_offset + sizeof(blob0) + sizeof(blob1)));
    wb32(r + 28, (uint32_t)sizeof(blob2));

    /* Payloads (opaque, intentionally unsupported). */
    memcpy(buf + data_offset, blob0, sizeof(blob0));
    memcpy(buf + data_offset + sizeof(blob0), blob1, sizeof(blob1));
    memcpy(buf + data_offset + sizeof(blob0) + sizeof(blob1),
           blob2, sizeof(blob2));

    return data_offset + sizeof(blob0) + sizeof(blob1) + sizeof(blob2);
}

static int read_optional_menu_bpk(const char *home,
                                  uint8_t **out_data,
                                  size_t *out_size) {
    char path[1024];
    FILE *fp;
    long size;
    uint8_t *data;

    if (!home || !home[0]) return 0;
    if (snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/MENU.BPK",
                 home) <= 0) return 0;
    fp = fopen(path, "rb");
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

static void test_synthetic_bpx3_contract(void) {
    uint8_t buf[256];
    size_t size;
    Nexus_V1_BpxBpkArchive archive;
    const Nexus_V1_BpxBpkEntry *entry;
    int rc;

    printf("\n--- synthetic BPX3 contract (data-free) ---\n");

    size = make_synthetic_bpx3(buf, sizeof(buf));
    CHECK(size > 0, "synthetic BPX3 build returns non-zero size");

    rc = nexus_v1_bpx_prs3_parse(buf, size, &archive);
    CHECK(rc == NEXUS_V1_BPX_BPK_OK, "synthetic BPX3 parses");
    CHECK(archive.format == NEXUS_V1_BPX_BPK_FORMAT_SYNTHETIC_PRS3,
          "synthetic BPX3 format is recorded as SYNTHETIC_PRS3");
    CHECK(archive.entry_count == 3, "synthetic BPX3 has 3 entries");

    entry = nexus_v1_bpx_bpk_find_entry(&archive, "MENU.A");
    CHECK(entry != NULL &&
          entry->width == 16 && entry->height == 15 &&
          entry->mode == NEXUS_V1_BPK_MODE_16BPP &&
          entry->pixel_count == 16u * 15u &&
          entry->packed_size == 16u &&
          entry->unpacked_size == (16u * 15u * 2u) &&
          entry->has_prs3_magic == 1 &&
          entry->method == NEXUS_V1_BPX_BPK_METHOD_PRS3_UNKNOWN,
          "MENU.A: 16x15 mode=14 pixels=240 packed=16 unpacked=480 PRS3");

    entry = nexus_v1_bpx_bpk_find_entry(&archive, "MENU.B");
    CHECK(entry != NULL &&
          entry->width == 54 && entry->height == 31 &&
          entry->mode == NEXUS_V1_BPK_MODE_8BPP &&
          entry->pixel_count == 54u * 31u,
          "MENU.B: width=54 height=31 mode=6 pixels=1674");

    entry = nexus_v1_bpx_bpk_find_entry(&archive, "MENU.C");
    CHECK(entry != NULL &&
          entry->mode == NEXUS_V1_BPK_MODE_32BPP &&
          entry->pixel_count == 64u &&
          entry->packed_size == 4u &&
          entry->unpacked_size == 64u * 4u,
          "MENU.C: width=8 height=8 mode=30 pixels=64 packed=4 unpacked=256");

    /* Reject bad magic. */
    buf[0] = 'X';
    rc = nexus_v1_bpx_prs3_parse(buf, size, &archive);
    CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BAD_MAGIC,
          "synthetic BPX3 rejects bad magic");
    buf[0] = 'B';

    /* Reject mismatched pixel_count. */
    {
        uint8_t bad[256];
        size_t bad_size = make_synthetic_bpx3(bad, sizeof(bad));
        wb32(bad + 16 + 20, 999u); /* corrupt pixel count for entry 0 */
        rc = nexus_v1_bpx_prs3_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BOUNDS,
              "synthetic BPX3 rejects width*height mismatch");
    }

    /* Reject unknown mode tag. */
    {
        uint8_t bad[256];
        size_t bad_size = make_synthetic_bpx3(bad, sizeof(bad));
        bad[16 + 18] = 7u; /* 7 is not 6/14/22/30 */
        rc = nexus_v1_bpx_prs3_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_METHOD,
              "synthetic BPX3 rejects unknown mode tag");
    }

    /* Reject a zero-length PRS3 payload span. */
    {
        uint8_t bad[256];
        size_t bad_size = make_synthetic_bpx3(bad, sizeof(bad));
        wb32(bad + 16 + 28, 0u);
        rc = nexus_v1_bpx_prs3_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BOUNDS,
              "synthetic BPX3 rejects zero packed payload size");
    }

    /* Reject overlapping PRS3 payload spans in table order. */
    {
        uint8_t bad[256];
        size_t bad_size = make_synthetic_bpx3(bad, sizeof(bad));
        wb32(bad + 16 + 28, 20u); /* entry 0 now overlaps entry 1 */
        rc = nexus_v1_bpx_prs3_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BOUNDS,
              "synthetic BPX3 rejects overlapping packed payload spans");
    }
}

static void test_optional_real_menumenu_bpk(void) {
    const char *home = getenv("HOME");
    uint8_t *data = NULL;
    size_t size = 0;

    printf("\n--- optional real MENU.BPK (no asset loaded in CI) ---\n");
    if (!read_optional_menu_bpk(home, &data, &size)) {
        printf("  SKIP: real MENU.BPK not present\n");
        return;
    }

    Nexus_V1_BpkArchiveInfo info;
    Nexus_V1_BpkModeDistribution dist;
    Nexus_V1_BpkPrs3Info prs3;
    Nexus_V1_BpkEntryPrefix prefix;
    int rc;

    printf("  loaded %zu bytes from ~/.firestaff/data/nexus/MENU.BPK\n", size);

    rc = nexus_v1_bpk_archive_parse(data, size, &info);
    CHECK(rc == 0, "real MENU.BPK BPPK/BMPD directory parses");
    CHECK(info.outer_size == 89060U, "real MENU.BPK outer size = 89060");
    CHECK(info.bmpd_size == 88524U, "real MENU.BPK BMPD size = 88524");
    CHECK(info.entry_count_hint == 163U, "real MENU.BPK entry count = 163");
    CHECK(info.prs3_payload_count == 162U,
          "real MENU.BPK PRS3 candidate count = 162");
    CHECK(info.raw_payload_count == 1U,
          "real MENU.BPK raw candidate count = 1");

    rc = nexus_v1_bpk_archive_mode_distribution(data, size, &dist);
    CHECK(rc == 0, "mode distribution walks without error");
    CHECK(dist.total_with_prefix == 163U,
          "all 163 entries span the 20-byte prefix");
    CHECK(dist.mode_count[NEXUS_V1_BPK_MODE_8BPP] == 14U,
          "mode tag 6: 14 entries");
    CHECK(dist.mode_count[NEXUS_V1_BPK_MODE_16BPP] == 62U,
          "mode tag 14: 62 entries");
    CHECK(dist.mode_count[NEXUS_V1_BPK_MODE_24BPP] == 39U,
          "mode tag 22: 39 entries");
    CHECK(dist.mode_count[NEXUS_V1_BPK_MODE_32BPP] == 47U,
          "mode tag 30: 47 entries");
    CHECK(dist.mode_count[NEXUS_V1_BPK_MODE_TRAILER] == 1U,
          "mode tag 10: 1 entry (the directory trailer)");
    CHECK(dist.trailer_found == 1, "directory trailer is detected");
    CHECK(dist.trailer_index == 0U,
          "directory trailer lives at entry index 0");

    /* Entry 1: first PRS3 entry after the trailer. */
    rc = nexus_v1_bpk_archive_get_entry_prefix(data, size, 1U, &prefix);
    CHECK(rc == 0, "entry[1] prefix readable");
    CHECK(prefix.prefix_complete == 1, "entry[1] spans >= 20 bytes");
    CHECK(prefix.width == 16 && prefix.height == 15 &&
          prefix.mode == NEXUS_V1_BPK_MODE_16BPP,
          "entry[1] width=16 height=15 mode=14");

    rc = nexus_v1_bpk_archive_inspect_prs3(data, size, 1U, &prs3);
    CHECK(rc == 0, "entry[1] PRS3 inspect returns ok");
    CHECK(prs3.has_prs3 == 1, "entry[1] has PRS3 magic");
    CHECK(prs3.prs3_version_matches == 1,
          "entry[1] PRS3 version == 0x00000001");
    CHECK(prs3.prs3_pixel_count == 240U, "entry[1] PRS3 pixel count = 240");
    CHECK(prs3.prefix_pixels == 240U, "entry[1] prefix pixels = 240");
    CHECK(prs3.pixel_count_matches == 1,
          "entry[1] prefix pixels == PRS3 pixel count");
    CHECK(prs3.payload_available == 1,
          "entry[1] payload is available after PRS3 header");

    /* Entry 162: last PRS3 entry. */
    rc = nexus_v1_bpk_archive_inspect_prs3(data, size, 162U, &prs3);
    CHECK(rc == 0, "entry[162] PRS3 inspect returns ok");
    CHECK(prs3.has_prs3 == 1, "entry[162] has PRS3 magic");
    CHECK(prs3.prs3_version_matches == 1,
          "entry[162] PRS3 version == 0x00000001");
    CHECK(prs3.pixel_count_matches == 1,
          "entry[162] prefix pixels == PRS3 pixel count");

    /* Entry 0: directory trailer has no PRS3 magic. */
    rc = nexus_v1_bpk_archive_inspect_prs3(data, size, 0U, &prs3);
    CHECK(rc == 0, "entry[0] PRS3 inspect returns ok (no PRS3 marker)");
    CHECK(prs3.has_prs3 == 0,
          "entry[0] directory trailer has no PRS3 marker");
    CHECK(prs3.prs3_version_matches == 0,
          "entry[0] PRS3 version is not asserted");
    CHECK(prs3.pixel_count_matches == 0,
          "entry[0] pixel-count cross-check is not asserted");

    /* The directory trailer's prefix[0..8] must hold offset[161] and
     * offset[162] (88,384 and 88,448), pointing at the last two real
     * picture entries. This is the observed MENU.BPK contract. */
    {
        Nexus_V1_BpkEntryPrefix trailer;
        rc = nexus_v1_bpk_archive_get_entry_prefix(data, size, 0U, &trailer);
        CHECK(rc == 0, "entry[0] trailer prefix readable");
        CHECK(trailer.mode == NEXUS_V1_BPK_MODE_TRAILER,
              "entry[0] mode tag is the unique TRAILER (10)");
        /* Raw bytes 0..3 and 4..7 are BE uint32 file offsets. We don't
         * expose those on the prefix struct, so spot-check the raw
         * 20-byte prefix bytes instead. */
        CHECK(trailer.raw[0] == 0x00u && trailer.raw[1] == 0x01u &&
              trailer.raw[2] == 0x59u && trailer.raw[3] == 0x40u,
              "entry[0] trailer prefix[0..4] = 0x00015940 (= offset[161])");
        CHECK(trailer.raw[4] == 0x00u && trailer.raw[5] == 0x01u &&
              trailer.raw[6] == 0x59u && trailer.raw[7] == 0x80u,
              "entry[0] trailer prefix[4..8] = 0x00015980 (= offset[162])");
    }

    /* Verify prefix width*height matches PRS3 pixel count for every
     * PRS3-bearing entry (162 entries). This is the strongest single
     * signal we have that the reverse-engineered 20-byte prefix layout
     * is right. */
    {
        uint32_t prs3_seen = 0;
        uint32_t pix_matches = 0;
        for (uint32_t i = 0; i < info.entry_count_hint; ++i) {
            Nexus_V1_BpkPrs3Info pi;
            if (nexus_v1_bpk_archive_inspect_prs3(data, size, i, &pi) != 0) {
                continue;
            }
            if (!pi.has_prs3) continue;
            ++prs3_seen;
            if (pi.pixel_count_matches) ++pix_matches;
        }
        CHECK(prs3_seen == 162U,
              "every PRS3-bearing entry was inspected (162/162)");
        CHECK(pix_matches == 162U,
              "every PRS3 entry has prefix_pixels == prs3_pixel_count");
    }

    free(data);
}

int main(void) {
    printf("=== Nexus V1 MENU.BPK boundary inspection probe (pass1082) ===\n");

    test_synthetic_bpx3_contract();
    test_optional_real_menumenu_bpk();

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
