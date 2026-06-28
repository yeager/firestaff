/*
 * firestaff_nexus_v1_bpk_surface_class_probe.c
 * ==============================================
 *
 * pass1083 — Nexus V1 MENU.BPK surface-class + BPX3 directory-trailer
 * boundary probe.
 *
 * Walks the observed DM Nexus MENU.BPK structure with the new
 * per-entry surface-class helpers without claiming PRS3 decompression:
 *
 *   - nexus_v1_bpk_mode_to_surface_class() maps the four observed
 *     pixel-mode tags (6/14/22/30) and the unique directory trailer
 *     (10) to a stable enum (INDEXED_8BPP / RGB565 / RGB888 / RGBA8888
 *     / DIRECTORY_TRAILER). Unknown mode tags return UNKNOWN.
 *   - nexus_v1_bpk_mode_to_bpp() maps the same four pixel-mode tags
 *     to 1/2/3/4 bytes per pixel; trailer + unknown modes return 0.
 *   - nexus_v1_bpk_archive_surface_estimate() walks every entry whose
 *     20-byte prefix is complete and reports (entry_index, mode,
 *     width, height, pixel_count, surface layout) per PRS3-bearing
 *     entry. Directory-trailer entries are skipped and counted.
 *   - nexus_v1_bpx_prs3_parse() (updated pass1083) recognises an
 *     explicit directory-trailer entry (mode tag 10) inside the
 *     synthetic BPX3 stream contract and tags it with
 *     NEXUS_V1_BPX_BPK_METHOD_DIRECTORY_TRAILER (no PRS3 magic).
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
 * probe exercises the real asset; otherwise it falls back to a
 * data-free synthetic BPX3 + BPPK archive that locks the same width /
 * height / mode / surface-class / trailer contract so the boundary
 * shape can be exercised in CI without copyrighted assets.
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

/* ---- Synthetic BPX3 with one directory-trailer + one PRS3 picture ---- */

static size_t make_synthetic_bpx3_with_trailer(uint8_t *buf, size_t cap) {
    const uint32_t table_offset = NEXUS_V1_BPX0_HEADER_SIZE;
    const uint32_t data_offset = NEXUS_V1_BPX0_HEADER_SIZE +
        (2u * NEXUS_V1_BPX0_ENTRY_SIZE);
    const uint8_t payload[16] = {0xa0, 0xa1, 0xa2, 0xa3, 0xb0, 0xb1, 0xb2, 0xb3,
                                 0xc0, 0xc1, 0xc2, 0xc3, 0xd0, 0xd1, 0xd2, 0xd3};

    if (cap < data_offset + sizeof(payload)) return 0;
    memset(buf, 0, cap);
    memcpy(buf, "BPX3", 4);
    wb16(buf + 4, 1);
    wb16(buf + 6, 2);
    wb32(buf + 8, table_offset);
    wb32(buf + 12, data_offset);

    /* Entry 0: directory trailer (mode 10). */
    {
        uint8_t *r = buf + table_offset;
        memset(r, 0, NEXUS_V1_BPX0_ENTRY_SIZE);
        memcpy(r, "TRAILER", 7);
        r[18] = NEXUS_V1_BPK_MODE_TRAILER;
    }

    /* Entry 1: 16x15 14bpp picture. */
    {
        uint8_t *r = buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE;
        memset(r, 0, NEXUS_V1_BPX0_ENTRY_SIZE);
        memcpy(r, "MENU.PIC", 8);
        wb16(r + 16, 16U);
        r[18] = NEXUS_V1_BPK_MODE_16BPP;
        r[19] = 15U;
        wb32(r + 20, 16U * 15U);
        wb32(r + 24, data_offset);
        wb32(r + 28, (uint32_t)sizeof(payload));
    }

    memcpy(buf + data_offset, payload, sizeof(payload));
    return data_offset + sizeof(payload);
}

static void test_mode_lookup_apis(void) {
    printf("\n--- mode -> surface_class / bpp lookup ---\n");

    CHECK(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_8BPP) ==
              NEXUS_V1_BPK_SURFACE_INDEXED_8BPP,
          "mode 6 -> SURFACE_INDEXED_8BPP");
    CHECK(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_16BPP) ==
              NEXUS_V1_BPK_SURFACE_RGB565,
          "mode 14 -> SURFACE_RGB565");
    CHECK(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_24BPP) ==
              NEXUS_V1_BPK_SURFACE_RGB888,
          "mode 22 -> SURFACE_RGB888");
    CHECK(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_32BPP) ==
              NEXUS_V1_BPK_SURFACE_RGBA8888,
          "mode 30 -> SURFACE_RGBA8888");
    CHECK(nexus_v1_bpk_mode_to_surface_class(NEXUS_V1_BPK_MODE_TRAILER) ==
              NEXUS_V1_BPK_SURFACE_DIRECTORY_TRAILER,
          "mode 10 -> SURFACE_DIRECTORY_TRAILER");
    CHECK(nexus_v1_bpk_mode_to_surface_class(7) ==
              NEXUS_V1_BPK_SURFACE_UNKNOWN,
          "mode 7 -> SURFACE_UNKNOWN");

    CHECK(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_8BPP) == 1U,
          "mode 6 -> 1 bpp");
    CHECK(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_16BPP) == 2U,
          "mode 14 -> 2 bpp");
    CHECK(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_24BPP) == 3U,
          "mode 22 -> 3 bpp");
    CHECK(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_32BPP) == 4U,
          "mode 30 -> 4 bpp");
    CHECK(nexus_v1_bpk_mode_to_bpp(NEXUS_V1_BPK_MODE_TRAILER) == 0U,
          "mode 10 -> 0 bpp");
    CHECK(nexus_v1_bpk_mode_to_bpp(0) == 0U, "mode 0 -> 0 bpp (unknown)");
    CHECK(nexus_v1_bpk_mode_to_bpp(255) == 0U, "mode 255 -> 0 bpp (unknown)");
}

static void test_synthetic_bpx3_trailer(void) {
    uint8_t buf[160];
    size_t size;
    Nexus_V1_BpxBpkArchive archive;
    const Nexus_V1_BpxBpkEntry *trailer;
    const Nexus_V1_BpxBpkEntry *picture;
    int rc;

    printf("\n--- synthetic BPX3 directory-trailer entry ---\n");

    size = make_synthetic_bpx3_with_trailer(buf, sizeof(buf));
    CHECK(size > 0, "synthetic BPX3 build returns non-zero size");

    rc = nexus_v1_bpx_prs3_parse(buf, size, &archive);
    CHECK(rc == NEXUS_V1_BPX_BPK_OK, "BPX3 with trailer parses");
    CHECK(archive.entry_count == 2U, "BPX3 has 2 entries");

    trailer = &archive.entries[0];
    CHECK(trailer->method == NEXUS_V1_BPX_BPK_METHOD_DIRECTORY_TRAILER,
          "entry[0] method is DIRECTORY_TRAILER");
    CHECK(trailer->mode == NEXUS_V1_BPK_MODE_TRAILER,
          "entry[0] mode tag == 10");
    CHECK(trailer->has_prs3_magic == 0,
          "entry[0] has no PRS3 magic");
    CHECK(trailer->offset == 0U && trailer->packed_size == 0U &&
              trailer->unpacked_size == 0U,
          "entry[0] carries no payload");
    CHECK(trailer->width == 0U && trailer->height == 0U &&
              trailer->pixel_count == 0U,
          "entry[0] has zero width/height/pixel_count");

    picture = &archive.entries[1];
    CHECK(picture->method == NEXUS_V1_BPX_BPK_METHOD_PRS3_UNKNOWN,
          "entry[1] method is PRS3_UNKNOWN");
    CHECK(picture->width == 16U && picture->height == 15U &&
              picture->mode == NEXUS_V1_BPK_MODE_16BPP &&
              picture->pixel_count == 16U * 15U,
          "entry[1] preserves 16x15 14bpp / 240 pixels");
    CHECK(picture->offset + picture->packed_size == (uint32_t)size &&
              picture->packed_size == 16U &&
              picture->unpacked_size == 16U * 15U * 2U,
          "entry[1] has bounded packed span and RGB565 unpacked byte count");

    /* Reject a trailer with nonzero reserved (offset field must stay 0). */
    {
        uint8_t bad[160];
        size_t bad_size = make_synthetic_bpx3_with_trailer(bad, sizeof(bad));
        bad[16 + 24] = 0xFFu; /* trailer reserved bytes must stay zero */
        rc = nexus_v1_bpx_prs3_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED,
              "BPX3 rejects trailer with nonzero reserved bytes");
    }

    /* Reject a trailer with nonzero width. */
    {
        uint8_t bad[160];
        size_t bad_size = make_synthetic_bpx3_with_trailer(bad, sizeof(bad));
        bad[16 + 16] = 0x01u; /* width must be 0 for a trailer */
        rc = nexus_v1_bpx_prs3_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BOUNDS,
              "BPX3 rejects trailer with nonzero width");
    }

    /* Reject a PRS3 picture with a packed span that runs past EOF. */
    {
        uint8_t bad[160];
        size_t bad_size = make_synthetic_bpx3_with_trailer(bad, sizeof(bad));
        wb32(bad + 16 + NEXUS_V1_BPX0_ENTRY_SIZE + 28, 4096U);
        rc = nexus_v1_bpx_prs3_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BOUNDS,
              "BPX3 rejects PRS3 packed payload span past EOF");
    }

    /* Reject a PRS3 picture with a zero packed span. */
    {
        uint8_t bad[160];
        size_t bad_size = make_synthetic_bpx3_with_trailer(bad, sizeof(bad));
        wb32(bad + 16 + NEXUS_V1_BPX0_ENTRY_SIZE + 28, 0U);
        rc = nexus_v1_bpx_prs3_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BOUNDS,
              "BPX3 rejects zero-size PRS3 packed payload span");
    }
}

static void read_optional_menu_bpk(const char *home,
                                   uint8_t **out_data,
                                   size_t *out_size) {
    char path[1024];
    FILE *fp;
    long size;
    uint8_t *data;

    *out_data = NULL;
    *out_size = 0;
    if (!home || !home[0]) return;
    if (snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/MENU.BPK",
                 home) <= 0) return;
    fp = fopen(path, "rb");
    if (!fp) return;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return; }
    size = ftell(fp);
    if (size <= 0) { fclose(fp); return; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return; }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) { fclose(fp); return; }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data); fclose(fp); return;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)size;
}

static void test_optional_real_menumenu_bpk(void) {
    const char *home = getenv("HOME");
    uint8_t *data = NULL;
    size_t size = 0;

    printf("\n--- optional real MENU.BPK (no asset loaded in CI) ---\n");
    read_optional_menu_bpk(home, &data, &size);
    if (!data) {
        printf("  SKIP: real MENU.BPK not present\n");
        return;
    }
    printf("  loaded %zu bytes from ~/.firestaff/data/nexus/MENU.BPK\n", size);

    Nexus_V1_BpkSurfaceEntry entries[200];
    Nexus_V1_BpkSurfaceEstimate summary;
    uint32_t indexed = 0U, rgb565 = 0U, rgb888 = 0U, rgba32 = 0U;
    uint64_t expected_total = 0U;

    memset(entries, 0, sizeof(entries));
    memset(&summary, 0, sizeof(summary));

    CHECK(nexus_v1_bpk_archive_surface_estimate(
              data, size, entries,
              (uint32_t)(sizeof(entries) / sizeof(entries[0])),
              &summary) == 0,
          "real MENU.BPK surface_estimate returns 0");
    CHECK(summary.total_with_surface == 162U,
          "real MENU.BPK: 162 PRS3-bearing entries with a surface");
    CHECK(summary.trailer_skipped == 1U,
          "real MENU.BPK: 1 directory-trailer entry skipped");
    CHECK(summary.unknown_skipped == 0U,
          "real MENU.BPK: 0 unknown-mode entries");
    CHECK(summary.used == 162U,
          "real MENU.BPK: 162 entries written to the output array");
    CHECK(summary.total_surface_bytes > 0U,
          "real MENU.BPK: summary total surface bytes > 0");

    /* Cross-check mode bucket counts against the pass1082 mode distribution. */
    for (uint32_t i = 0; i < summary.used; ++i) {
        switch (entries[i].layout.surface_class) {
        case NEXUS_V1_BPK_SURFACE_INDEXED_8BPP: ++indexed; break;
        case NEXUS_V1_BPK_SURFACE_RGB565:       ++rgb565; break;
        case NEXUS_V1_BPK_SURFACE_RGB888:       ++rgb888; break;
        case NEXUS_V1_BPK_SURFACE_RGBA8888:     ++rgba32; break;
        default: break;
        }
        expected_total += entries[i].layout.surface_bytes;
        CHECK(entries[i].layout.rowstride ==
                  (uint32_t)entries[i].width *
                      entries[i].layout.bpp,
              "real MENU.BPK rowstride == width * bpp");
        CHECK(entries[i].layout.surface_bytes ==
                  (uint32_t)entries[i].width *
                      (uint32_t)entries[i].height *
                      entries[i].layout.bpp,
              "real MENU.BPK surface_bytes == w*h*bpp");
        CHECK(entries[i].pixel_count ==
                  (uint32_t)entries[i].width *
                      (uint32_t)entries[i].height,
              "real MENU.BPK pixel_count == w*h");
    }
    CHECK(indexed == 14U, "real MENU.BPK: 14 indexed 8bpp entries");
    CHECK(rgb565 == 62U, "real MENU.BPK: 62 RGB565 entries");
    CHECK(rgb888 == 39U, "real MENU.BPK: 39 RGB888 entries");
    CHECK(rgba32 == 47U, "real MENU.BPK: 47 RGBA8888 entries");
    CHECK(expected_total == summary.total_surface_bytes,
          "real MENU.BPK: per-entry sum equals summary total");

    free(data);
}

int main(void) {
    printf("=== Nexus V1 MENU.BPK surface-class probe (pass1083) ===\n");

    test_mode_lookup_apis();
    test_synthetic_bpx3_trailer();
    test_optional_real_menumenu_bpk();

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
