/*
 * Nexus V1 DMDF embedded BITMAP / palette / string gate
 * =====================================================
 *
 * Source-lock:
 *   include/nexus_v1_dmdf_model.h, src/nexus/nexus_v1_dmdf_model.c
 *   docs/nexus_v1_phase2_data_formats_H2321.md §6.5 / §8.2
 *
 * ReDMCSB has no Saturn/Nexus implementation. This parser-level gate is
 * intentionally bounded and decode-free: it verifies that a synthetic DMDF
 * tail carrying BITM + PLTB + STRB blocks can be discovered safely, and that
 * malformed candidates do not read past EOF. The optional real .MNS branch
 * only receipts scan/texture-tail bounds when local user data is present.
 */

#include "nexus_v1_dmdf_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                      \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }           \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }           \
} while (0)

static void wb16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wb32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v & 0xffu);
}

static int build_bitmap_block(uint8_t *out, int cap,
                              uint32_t width, uint32_t height,
                              uint32_t bpp, uint32_t palette_index) {
    uint64_t pixels = (uint64_t)width * (uint64_t)height;
    uint64_t payload = (bpp == 4U) ? ((pixels + 1U) / 2U)
                                   : (pixels * (uint64_t)(bpp / 8U));
    uint64_t total = 32U + payload;
    uint32_t i;

    if (total > (uint64_t)cap || total > 0xffffffffU) return -1;
    memset(out, 0, (size_t)cap);
    wb32(out + 0, NEXUS_DMDF_BITMAP_BLOCK_MAGIC);
    wb32(out + 4, (uint32_t)total);
    wb32(out + 8, width);
    wb32(out + 12, height);
    wb32(out + 16, bpp);
    wb32(out + 20, 0x00000005U);       /* synthetic metadata flags */
    wb32(out + 24, palette_index);
    wb32(out + 28, 0U);
    for (i = 0; i < (uint32_t)payload; i++) {
        out[32U + i] = (uint8_t)((i * 17U) & 0xffU);
    }
    return (int)total;
}

static int build_palette_block(uint8_t *out, int cap) {
    int i;
    if (cap < 16 + 16 * 2) return -1;
    memset(out, 0, (size_t)cap);
    wb32(out + 0, NEXUS_DMDF_PALETTE_BLOCK_MAGIC);
    wb32(out + 4, 16U + 16U * 2U);
    wb32(out + 8, 16U);
    wb32(out + 12, 2U);
    for (i = 0; i < 16; i++) {
        uint16_t v = (uint16_t)(0x1000U + (uint32_t)i);
        wb16(out + 16 + i * 2, v);
    }
    return 16 + 16 * 2;
}

static int build_string_block(uint8_t *out, int cap) {
    const char *a = "SCORPION";
    const char *b = "TEXTURE";
    int body = 12 + 2 * 4 + 2 * 4;
    int total = body + 9 + 8;
    int i;

    if (cap < total) return -1;
    memset(out, 0, (size_t)cap);
    wb32(out + 0, NEXUS_DMDF_STRING_BLOCK_MAGIC);
    wb32(out + 4, (uint32_t)total);
    wb32(out + 8, 2U);
    wb32(out + 12, (uint32_t)body);
    wb32(out + 16, (uint32_t)(body + 9));
    wb32(out + 20, 9U);
    wb32(out + 24, 8U);
    for (i = 0; i < 9; i++) out[body + i] = (uint8_t)a[i];
    for (i = 0; i < 8; i++) out[body + 9 + i] = (uint8_t)b[i];
    return total;
}

static int build_synthetic_dmdf(uint8_t *out, int cap,
                                int *out_texture_offset,
                                int *out_total) {
    const int data_off = 48;
    const int vertex_face_bytes = 8 + 10 + 6;
    int pos = data_off + vertex_face_bytes;
    int n;

    if (cap < 512) return 0;
    memset(out, 0, (size_t)cap);
    wb32(out + 0, NEXUS_DMDF_MAGIC);
    wb32(out + 8, 3U);                 /* section_count */
    wb32(out + 28, (uint32_t)data_off);
    wb32(out + data_off + 0, 1U);      /* vertex_count */
    wb32(out + data_off + 4, 1U);      /* face_count */
    wb16(out + data_off + 8, 0x0010U);
    wb16(out + data_off + 10, 0x0020U);
    wb16(out + data_off + 12, 0x0030U);
    wb16(out + data_off + 14, 0x0001U);
    wb16(out + data_off + 16, 0x0002U);
    wb16(out + data_off + 18, 0U);
    wb16(out + data_off + 20, 0U);
    wb16(out + data_off + 22, 0U);

    n = build_bitmap_block(out + pos, cap - pos, 16U, 8U, 4U, 2U);
    if (n < 0) return 0;
    pos += n;

    n = build_palette_block(out + pos, cap - pos);
    if (n < 0) return 0;
    pos += n;

    n = build_string_block(out + pos, cap - pos);
    if (n < 0) return 0;
    pos += n;

    /* Malformed candidate: scanner must count it, then keep moving. */
    if (pos + 8 > cap) return 0;
    wb32(out + pos, NEXUS_DMDF_BITMAP_BLOCK_MAGIC);
    wb32(out + pos + 4, 31U);
    pos += 8;

    wb32(out + 4, (uint32_t)pos);
    if (out_texture_offset) *out_texture_offset = data_off + vertex_face_bytes;
    if (out_total) *out_total = pos;
    return 1;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *fp;
    long n;
    uint8_t *data;

    if (!path || !out_data || !out_size) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    n = ftell(fp);
    if (n <= 0) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)n);
    if (!data) {
        fclose(fp);
        return 0;
    }
    if (fread(data, 1, (size_t)n, fp) != (size_t)n) {
        free(data);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)n;
    return 1;
}

static void test_bitmap_block_happy_path(void) {
    uint8_t buf[256];
    int sz = build_bitmap_block(buf, (int)sizeof(buf), 16U, 8U, 4U, 7U);
    Nexus_DMDFBitmapBlock blk;

    CHECK(sz > 0, "4bpp BITM fixture built");
    CHECK(nexus_v1_dmdf_parse_bitmap_block(buf, sz, 0, &blk) == 1,
          "4bpp BITM parses");
    CHECK(blk.valid == 1, "BITM valid flag set");
    CHECK(blk.width == 16U && blk.height == 8U, "BITM dimensions retained");
    CHECK(blk.bpp == 4U, "BITM bpp retained");
    CHECK(blk.payload_bytes == 64U, "4bpp BITM payload is width*height/2");
    CHECK(blk.payload_offset == 32U, "BITM payload offset = 32");
    CHECK(blk.palette_index == 7U, "BITM palette index retained");
}

static void test_bitmap_block_rejections(void) {
    uint8_t buf[320];
    Nexus_DMDFBitmapBlock blk;
    int sz;

    sz = build_bitmap_block(buf, (int)sizeof(buf), 16U, 8U, 16U, 0U);
    CHECK(sz > 0, "16bpp BITM fixture built");
    CHECK(nexus_v1_dmdf_parse_bitmap_block(buf, sz, 0, &blk) == 1,
          "16bpp BITM parses");
    CHECK(blk.payload_bytes == 16U * 8U * 2U,
          "16bpp payload is width*height*2");

    sz = build_bitmap_block(buf, (int)sizeof(buf), 24U, 8U, 4U, 0U);
    CHECK(sz > 0, "non-power-of-two fixture built");
    CHECK(nexus_v1_dmdf_parse_bitmap_block(buf, sz, 0, &blk) == 0,
          "BITM non-power-of-two width rejected");

    sz = build_bitmap_block(buf, (int)sizeof(buf), 16U, 8U, 12U, 0U);
    CHECK(sz > 0, "unsupported-bpp fixture built");
    CHECK(nexus_v1_dmdf_parse_bitmap_block(buf, sz, 0, &blk) == 0,
          "BITM unsupported bpp rejected");

    sz = build_bitmap_block(buf, (int)sizeof(buf), 16U, 8U, 8U, 0U);
    CHECK(sz > 0, "size-mismatch base fixture built");
    wb32(buf + 4, 32U + 15U);
    CHECK(nexus_v1_dmdf_parse_bitmap_block(buf, sz, 0, &blk) == 0,
          "BITM declared size/payload mismatch rejected");

    CHECK(nexus_v1_dmdf_parse_bitmap_block(NULL, sz, 0, &blk) == 0,
          "BITM NULL data rejected");
    CHECK(nexus_v1_dmdf_parse_bitmap_block(buf, sz, -1, &blk) == 0,
          "BITM negative offset rejected");
    CHECK(nexus_v1_dmdf_parse_bitmap_block(buf, 12, 0, &blk) == 0,
          "BITM truncated header rejected");
    CHECK(nexus_v1_dmdf_parse_bitmap_block(buf, sz, 0, NULL) == 0,
          "BITM NULL output rejected");
}

static void test_embedded_scan_and_raw_tail(void) {
    uint8_t buf[768];
    int texture_off = 0;
    int total = 0;
    Nexus_DMDFRawTexturePayload raw;
    Nexus_DMDFEmbeddedScan scan;

    CHECK(build_synthetic_dmdf(buf, (int)sizeof(buf), &texture_off, &total),
          "synthetic DMDF with BITM/PLTB/STRB tail built");
    CHECK(nexus_v1_dmdf_estimate_raw_texture_payload(buf, total, &raw) == 1,
          "raw texture payload estimate succeeds");
    CHECK(raw.valid == 1, "raw texture payload marked valid");
    CHECK(raw.offset == (uint32_t)texture_off, "raw texture offset matches");
    CHECK(raw.bytes == (uint32_t)(total - texture_off),
          "raw texture byte count spans embedded tail");

    CHECK(nexus_v1_dmdf_scan_embedded_blocks(buf, total, texture_off, 0,
                                             &scan) == 1,
          "embedded scanner walks synthetic texture tail");
    CHECK(scan.valid == 1, "embedded scan marked valid");
    CHECK(scan.bitmap_block_count == 1U, "scanner counts one BITM block");
    CHECK(scan.palette_block_count == 1U, "scanner counts one PLTB block");
    CHECK(scan.string_block_count == 1U, "scanner counts one STRB block");
    CHECK(scan.invalid_candidate_count == 1U,
          "scanner counts malformed trailing BITM candidate");
    CHECK(scan.first_bitmap_offset == (uint32_t)texture_off,
          "first BITM offset is texture tail start");

    CHECK(nexus_v1_dmdf_scan_embedded_blocks(buf, total, texture_off, 40,
                                             &scan) == 1,
          "embedded scanner honors max_bytes window");
    CHECK(scan.bitmap_block_count == 0U && scan.palette_block_count == 0U &&
              scan.string_block_count == 0U,
          "short scan window does not overread partial BITM");
}

static void test_material_decode(void) {
    uint8_t buf[768];
    int texture_off = 0;
    int total = 0;
    Nexus_DMDFMaterialBank bank;

    memset(&bank, 0, sizeof(bank));
    CHECK(build_synthetic_dmdf(buf, (int)sizeof(buf), &texture_off, &total),
          "material decode DMDF fixture built");
    CHECK(nexus_v1_dmdf_decode_material_bank(buf, total, &bank) == 1,
          "BITM and PLTB decode into a material bank");
    CHECK(bank.valid == 1 && bank.surface_count == 1 &&
          bank.surfaces[0].valid == 1,
          "decoded material bank exposes its first surface");
    CHECK(bank.surfaces[0].width == 16 && bank.surfaces[0].height == 8 &&
          bank.surfaces[0].pixels[0] == 2 && bank.surfaces[0].pixels[1] == 2,
          "packed 4bpp texels resolve through the BITM palette slot");
    CHECK(bank.surfaces[0].palette[2] == 0xff200000U,
          "PLTB BGR555 entry becomes the raster palette colour");
    nexus_v1_dmdf_free_material_bank(&bank);
    CHECK(bank.valid == 0 && bank.surface_count == 0,
          "material bank free clears owned raster surfaces");
}

static void test_texture_section_boundary(void) {
    uint8_t buf[256];
    Nexus_DMDFTextureSection section;

    memset(buf, 0, sizeof(buf));
    buf[0] = 'D'; buf[1] = 'M'; buf[2] = 'D'; buf[3] = 'F';
    /* Retail MNS header word at 0x24 points at the top-level TEXT section. */
    buf[0x24] = 0; buf[0x25] = 0; buf[0x26] = 0; buf[0x27] = 48;
    buf[48] = 'T'; buf[49] = 'E'; buf[50] = 'X'; buf[51] = 'T';
    buf[55] = 192; /* section byte count */
    buf[59] = 1;  /* declared entry count */
    buf[63] = 1;  /* opaque flags */
    buf[75] = 56; /* pixel data offset relative to TEXT */
    buf[79] = 32; /* descriptor table offset relative to TEXT */
    buf[83] = 0xa8; /* material 0, opaque format flags */
    buf[87] = 8;    /* width */
    buf[89] = 8;    /* height in high half-word */
    buf[95] = 56;   /* descriptor pixel offset */
    CHECK(nexus_v1_dmdf_parse_texture_section(buf, (int)sizeof(buf),
                                               &section) == 1,
          "top-level TEXT section parses");
    CHECK(section.valid == 1 && section.offset == 48 && section.bytes == 192,
          "TEXT section preserves bounded range");
    CHECK(section.declared_entry_count == 1 && section.flags == 1 &&
          section.descriptor_count == 1 &&
          section.material_ids_unique && section.unique_material_id_count == 1 &&
          section.first_material_id == 0 && section.last_material_id == 0 &&
          section.descriptors[0].width == 8 && section.descriptors[0].height == 8,
          "TEXT section preserves opaque metadata");
    buf[55] = 96;
    CHECK(nexus_v1_dmdf_parse_texture_section(buf, 64, &section) == 0,
          "truncated TEXT section is rejected");
}

static void test_texture_section_duplicate_material_rejected(void) {
    uint8_t buf[256];
    Nexus_DMDFTextureSection section;
    Nexus_DMDFMaterialBank bank;

    memset(buf, 0, sizeof(buf));
    buf[0] = 'D'; buf[1] = 'M'; buf[2] = 'D'; buf[3] = 'F';
    wb32(buf + 0x24, 48U);
    wb32(buf + 48, NEXUS_DMDF_TEXTURE_SECTION_MAGIC);
    wb32(buf + 52, 208U);
    wb32(buf + 56, 2U);
    wb32(buf + 72, 72U);
    wb32(buf + 76, 32U);
    /* Two otherwise valid descriptors deliberately target material 7. */
    wb32(buf + 80, 0x00070000U); wb32(buf + 84, 8U);
    wb32(buf + 88, 0x00080000U); wb32(buf + 92, 72U);
    wb32(buf + 100, 0x00070000U); wb32(buf + 104, 8U);
    wb32(buf + 108, 0x00080000U); wb32(buf + 112, 72U);
    CHECK(nexus_v1_dmdf_parse_texture_section(buf, (int)sizeof(buf),
                                               &section) == 1 &&
          !section.material_ids_unique && section.unique_material_id_count == 1,
          "TEXT parser retains duplicate material-id ambiguity");
    memset(&bank, 0, sizeof(bank));
    CHECK(nexus_v1_dmdf_decode_text_material_bank(buf, (int)sizeof(buf),
                                                   &bank) == 0 &&
          !bank.valid && bank.surface_count == 0,
          "duplicate TEXT material IDs cannot overwrite a source surface");
}

static void test_texture_section_out_of_range_material_rejected(void) {
    uint8_t buf[256];
    Nexus_DMDFTextureSection section;
    Nexus_DMDFMaterialBank bank;

    memset(buf, 0, sizeof(buf));
    buf[0] = 'D'; buf[1] = 'M'; buf[2] = 'D'; buf[3] = 'F';
    wb32(buf + 0x24, 48U);
    wb32(buf + 48, NEXUS_DMDF_TEXTURE_SECTION_MAGIC);
    wb32(buf + 52, 192U);
    wb32(buf + 56, 1U);
    wb32(buf + 72, 56U);
    wb32(buf + 76, 32U);
    /* The descriptor itself is structurally valid, but its original
     * material ID cannot fit the fixed 256-entry host bank. */
    wb32(buf + 80, 0x01000000U); wb32(buf + 84, 8U);
    wb32(buf + 88, 0x00080000U); wb32(buf + 92, 56U);
    CHECK(nexus_v1_dmdf_parse_texture_section(buf, (int)sizeof(buf),
                                               &section) == 1 &&
          section.descriptors[0].material_id == 256U,
          "TEXT parser retains structurally valid out-of-bank source ID");
    memset(&bank, 0, sizeof(bank));
    CHECK(nexus_v1_dmdf_decode_text_material_bank(buf, (int)sizeof(buf),
                                                   &bank) == 0 &&
          !bank.valid && bank.surface_count == 0,
          "out-of-bank TEXT material blocks the whole source route");
}

static void test_optional_real_mns(void) {
    const char *path = getenv("FIRESTAFF_NEXUS_MNS");
    char fallback[1024];
    uint8_t *data = NULL;
    size_t size = 0;
    Nexus_DMDFEmbeddedScan scan;
    Nexus_DMDFRawTexturePayload raw;
    Nexus_DMDFTextureSection section;
    Nexus_DMDFMaterialBank bank;

    if (!path || path[0] == '\0') {
        const char *home = getenv("HOME");
        if (home && home[0] &&
            snprintf(fallback, sizeof(fallback),
                     "%s/.firestaff/data/nexus/SCORPION.MNS", home) > 0) {
            path = fallback;
        }
    }

    if (!path || !read_file(path, &data, &size)) {
        printf("  SKIP: no optional Nexus .MNS at FIRESTAFF_NEXUS_MNS or ~/.firestaff/data/nexus/SCORPION.MNS\n");
        return;
    }

    CHECK(nexus_v1_dmdf_is_valid(data, (int)size) == 1,
          "optional real .MNS has DMDF magic");
    CHECK(nexus_v1_dmdf_scan_embedded_blocks(data, (int)size, 0, 0, &scan) == 1,
          "optional real .MNS scan is bounds-safe");
    CHECK(scan.bytes_scanned == (uint32_t)size,
          "optional real .MNS scan covers file window");
    if (nexus_v1_dmdf_estimate_raw_texture_payload(data, (int)size, &raw)) {
        CHECK(raw.valid == 1, "optional real .MNS raw texture estimate valid");
        printf("  NOTE: real .MNS raw texture tail offset=%u bytes=%u, tagged blocks BITM=%u PLTB=%u STRB=%u invalid=%u\n",
               raw.offset, raw.bytes, scan.bitmap_block_count,
               scan.palette_block_count, scan.string_block_count,
               scan.invalid_candidate_count);
    } else {
        printf("  SKIP: optional real .MNS raw texture tail not classified by current stride contract\n");
    }
    if (nexus_v1_dmdf_parse_texture_section(data, (int)size, &section)) {
        CHECK(section.valid == 1 && section.bytes >= 16U,
              "optional real .MNS TEXT section is bounded");
        CHECK((uint64_t)section.offset + section.bytes <= size,
              "optional real .MNS TEXT section stays inside file");
        CHECK(section.material_ids_unique &&
              section.unique_material_id_count == section.descriptor_count,
              "optional real .MNS TEXT material IDs have one source bank slot each");
        printf("  NOTE: real .MNS TEXT section offset=%u bytes=%u entries=%u flags=%u\n",
               section.offset, section.bytes, section.declared_entry_count,
               section.flags);
    }
    memset(&bank, 0, sizeof(bank));
    CHECK(nexus_v1_dmdf_decode_text_material_bank(data, (int)size, &bank) == 1,
          "optional real .MNS TEXT descriptors decode original surfaces");
    CHECK(bank.valid == 1 && bank.surface_count > 0,
          "optional real .MNS exposes decoded original surfaces");
    nexus_v1_dmdf_free_material_bank(&bank);
    free(data);
}

int main(void) {
    printf("=========================================================\n");
    printf("Nexus V1 DMDF embedded BITMAP / palette / string gate\n");
    printf("=========================================================\n");

    test_bitmap_block_happy_path();
    test_bitmap_block_rejections();
    test_embedded_scan_and_raw_tail();
    test_material_decode();
    test_texture_section_boundary();
    test_texture_section_duplicate_material_rejected();
    test_texture_section_out_of_range_material_rejected();
    test_optional_real_mns();

    printf("\nResults: %d PASS, %d FAIL\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
