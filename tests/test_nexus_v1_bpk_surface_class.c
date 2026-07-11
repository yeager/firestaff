#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_bpx_bpk.h"
#include "nexus_v1_dmdf_model.h"

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

static void make_synthetic_stored_bpk(uint8_t *data, size_t cap) {
    const uint32_t entry0_off = 64U;
    const uint32_t entry1_off = 96U;
    const uint32_t entry2_off = 132U;
    const uint32_t entry3_off = 216U;
    if (cap < 256U) return;
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

    data[entry0_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_TRAILER;

    wr16_be(data + entry1_off + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 4U);
    data[entry1_off + NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 4U;
    data[entry1_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_8BPP;
    for (uint32_t i = 0; i < 16U; ++i) {
        data[entry1_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + i] =
            (uint8_t)(0x10U + i);
    }

    wr16_be(data + entry2_off + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 8U);
    data[entry2_off + NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 4U;
    data[entry2_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_16BPP;
    for (uint32_t i = 0; i < 64U; ++i) {
        data[entry2_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + i] =
            (uint8_t)(0x40U + i);
    }

    wr16_be(data + entry3_off + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 2U);
    data[entry3_off + NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 3U;
    data[entry3_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_24BPP;
    for (uint32_t i = 0; i < 18U; ++i) {
        data[entry3_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + i] =
            (uint8_t)(0x80U + i);
    }
}

static void make_synthetic_prs3_literal_bpk(uint8_t *data, size_t cap) {
    const uint32_t trailer_off = 64U;
    const uint32_t surface_off = 96U;
    uint8_t *p;
    if (cap < 160U) return;
    memset(data, 0, cap);
    memcpy(data, "BPPK", 4);
    wr32_be(data + 4, (uint32_t)cap);
    memcpy(data + 12, "BMPD", 4);
    wr32_be(data + 16, (uint32_t)cap - 20U);
    wr32_be(data + 20, 2U);
    wr32_be(data + 24, trailer_off);
    wr32_be(data + 28, surface_off);
    data[trailer_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_TRAILER;
    p = data + surface_off;
    wr16_be(p + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 2U);
    p[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 2U;
    p[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_8BPP;
    memcpy(p + 20, "PRS3", 4);
    wr32_be(p + 24, 1U);
    wr32_be(p + 28, 4U);
    wr32_be(p + 32, 5U);
    /* Four PRS literal control bits, least-significant bit first. */
    p[36] = 0x0fU;
    p[37] = 0x11U;
    p[38] = 0x22U;
    p[39] = 0x33U;
    p[40] = 0x44U;
}

static void make_synthetic_prs3_rgb565_bpk(uint8_t *data, size_t cap) {
    const uint32_t trailer_off = 64U;
    const uint32_t surface_off = 96U;
    uint8_t *p;
    if (cap < 168U) return;
    memset(data, 0, cap);
    memcpy(data, "BPPK", 4);
    wr32_be(data + 4, (uint32_t)cap);
    memcpy(data + 12, "BMPD", 4);
    wr32_be(data + 16, (uint32_t)cap - 20U);
    wr32_be(data + 20, 2U);
    wr32_be(data + 24, trailer_off);
    wr32_be(data + 28, surface_off);
    data[trailer_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_TRAILER;
    p = data + surface_off;
    wr16_be(p + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 2U);
    p[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 2U;
    p[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_8BPP;
    memcpy(p + 20, "PRS3", 4);
    wr32_be(p + 24, 1U);
    wr32_be(p + 28, 4U);
    wr32_be(p + 32, 9U);
    /* Eight PRS literal control bits, then 4 RGB565 pixels:
     * red, green, blue, white. */
    p[36] = 0xffU;
    p[37] = 0xf8U; p[38] = 0x00U;
    p[39] = 0x07U; p[40] = 0xe0U;
    p[41] = 0x00U; p[42] = 0x1fU;
    p[43] = 0xffU; p[44] = 0xffU;
}

static void test_prs3_surface_decode(void) {
    uint8_t data[160];
    uint8_t pixels[4] = {0};
    Nexus_V1_BpkSurfaceEntry surface;
    size_t written = 0U;
    int rc;

    make_synthetic_prs3_literal_bpk(data, sizeof(data));
    rc = nexus_v1_bpk_archive_decode_surface(data, sizeof(data), 1U,
                                              pixels, sizeof(pixels),
                                              &surface, &written);
    expect(rc == NEXUS_V1_BPK_DECODE_OK,
           "PRS3 literal stream decodes to a surface");
    expect(written == sizeof(pixels) && pixels[0] == 0x11U &&
               pixels[1] == 0x22U && pixels[2] == 0x33U &&
               pixels[3] == 0x44U,
           "PRS3 decoder preserves literal surface pixels");
    expect(surface.width == 2U && surface.height == 2U &&
               surface.layout.surface_class == NEXUS_V1_BPK_SURFACE_INDEXED_8BPP,
           "PRS3 decoder returns the declared surface layout");

    data[132] = 0x00U;
    data[133] = 0xeeU;
    data[134] = 0x0fU;
    rc = nexus_v1_bpk_archive_decode_surface(data, sizeof(data), 1U,
                                              pixels, sizeof(pixels),
                                              NULL, &written);
    expect(rc == NEXUS_V1_BPK_DECODE_ERR_STREAM,
           "PRS3 decoder rejects an invalid back-reference without fallback");
}

static void test_prs3_candidate_evidence(void) {
    uint8_t data[160];
    Nexus_V1_BpkPrs3CandidateEvidence rows[2];
    Nexus_V1_BpkPrs3CandidateEvidenceSummary summary;

    make_synthetic_prs3_literal_bpk(data, sizeof(data));
    expect(nexus_v1_bpk_archive_prs3_candidate_evidence(
               data, sizeof(data), rows, 2U, &summary) == 0,
           "PRS3 candidate evidence accepts bounded synthetic fixture");
    expect(summary.archive_entries == 2U && summary.prs3_surfaces == 1U &&
               summary.evaluated == 1U && summary.complete_trailing == 1U &&
               summary.complete_exact == 0U && summary.decoder_promoted == 0,
           "PRS3 candidate evidence records trailing bytes without promotion");
    expect(rows[0].entry_index == 1U &&
               rows[0].status == NEXUS_V1_BPK_PRS3_CANDIDATE_COMPLETE_TRAILING &&
               rows[0].expected_output_bytes == 4U &&
               rows[0].body_bytes_consumed == 5U &&
               strcmp(nexus_v1_bpk_prs3_candidate_status_name(rows[0].status),
                      "complete-trailing") == 0,
           "PRS3 candidate receipt uses declared bpp byte target and stable status");

    /* The framing and trial opcode fields stay fixed. Only control-bit
     * consumption changes: 0xf0 supplies four MSB-first literal flags,
     * whereas the retired LSB-first traversal starts with a back-reference. */
    data[132] = 0xf0U;
    expect(nexus_v1_bpk_archive_prs3_candidate_evidence_with_bit_order(
               data, sizeof(data),
               NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_MSB_FIRST,
               rows, 2U, &summary) == 0,
           "PRS3 MSB-first candidate accepts the same bounded framing");
    expect(summary.bit_order == NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_MSB_FIRST &&
               summary.complete_trailing == 1U && summary.complete_exact == 0U &&
               summary.decoder_promoted == 0 &&
               strcmp(nexus_v1_bpk_prs3_candidate_bit_order_name(
                          summary.bit_order), "msb-first") == 0,
           "PRS3 MSB-first receipt remains diagnostic-only");
    expect(nexus_v1_bpk_archive_prs3_candidate_evidence_with_bit_order(
               data, sizeof(data),
               NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_LSB_FIRST,
               rows, 2U, &summary) == 0 &&
               summary.stream_failures == 1U && summary.complete_exact == 0U,
           "PRS3 LSB-first traversal rejects the MSB-only literal fixture");
    expect(nexus_v1_bpk_archive_prs3_candidate_evidence_with_bit_order(
               data, sizeof(data), (Nexus_V1_BpkPrs3CandidateBitOrder)99,
               rows, 2U, &summary) != 0,
           "PRS3 candidate rejects an unknown bit order");
}

static void test_prs3_material_import(void) {
    uint8_t data[160];
    Nexus_DMDFMaterialBank bank;
    int imported;

    memset(&bank, 0, sizeof(bank));
    bank.surfaces[0].valid = 1;
    bank.surfaces[0].palette[0x11] = 0xff112233U;
    bank.surface_count = 1;
    bank.valid = 1;
    make_synthetic_prs3_literal_bpk(data, sizeof(data));
    imported = nexus_v1_dmdf_import_bpk_material_bank(data, sizeof(data),
                                                       &bank);
    expect(imported == 1 && bank.surfaces[1].valid,
           "decoded PRS3 surface fills its vacant DGN material slot");
    expect(bank.surfaces[1].width == 2 && bank.surfaces[1].height == 2 &&
               bank.surfaces[1].pixels[0] == 0x11U &&
               bank.surfaces[1].palette[0x11] == 0xff112233U,
           "BPK material keeps decoded texels and the real DMDF CLUT");
    free(bank.surfaces[1].pixels);
}

static void test_truecolor_material_import(void) {
    uint8_t data[256];
    Nexus_DMDFMaterialBank bank;
    int imported;

    memset(&bank, 0, sizeof(bank));
    make_synthetic_stored_bpk(data, sizeof(data));
    imported = nexus_v1_dmdf_import_bpk_material_bank(data, sizeof(data),
                                                       &bank);
    expect(imported == 2 && bank.surfaces[2].valid &&
               bank.surfaces[3].valid,
           "stored RGB565/RGB888 BPK surfaces import without synthetic CLUT");
    expect(bank.surfaces[2].width == 8 && bank.surfaces[2].height == 4 &&
               bank.surfaces[2].pixels[0] != 0 &&
               bank.surfaces[2].palette[bank.surfaces[2].pixels[0]] != 0U,
           "stored RGB565 material is converted to indexed pixels plus source palette");
    expect(bank.surfaces[3].width == 2 && bank.surfaces[3].height == 3 &&
               bank.surfaces[3].pixels[0] != 0 &&
               bank.surfaces[3].palette[bank.surfaces[3].pixels[0]] != 0U,
           "stored RGB888 material is converted to indexed pixels plus source palette");
    free(bank.surfaces[2].pixels);
    free(bank.surfaces[3].pixels);

    memset(&bank, 0, sizeof(bank));
    bank.surfaces[0].valid = 1;
    bank.surfaces[0].palette[0xf8] = 0xffff0000U;
    bank.surface_count = 1;
    bank.valid = 1;
    make_synthetic_prs3_rgb565_bpk(data, sizeof(data));
    imported = nexus_v1_dmdf_import_bpk_material_bank(data, sizeof(data),
                                                       &bank);
    expect(imported == 1 && bank.surfaces[1].valid,
           "PRS3-decoded BPK surface imports into material bank");
    expect(bank.surfaces[1].width == 2 && bank.surfaces[1].height == 2 &&
               bank.surfaces[1].pixels[0] == 0xf8U &&
               bank.surfaces[1].palette[0xf8] == 0xffff0000U,
           "PRS3 import preserves decoded indexed pixels and DMDF CLUT");
    free(bank.surfaces[1].pixels);
}

static void test_material_host_route_and_category_coverage(void) {
    uint8_t data[256];
    Nexus_DMDFMaterialBank bank;
    Nexus_V1_BpkMaterialHostRouteReceipt host;
    Nexus_V1_DgnMaterialCategoryCoverageReceipt coverage;
    const uint8_t wall_refs[2] = {2U, 3U};
    const uint8_t ceiling_refs[2] = {1U, 2U};
    int rc;

    memset(&bank, 0, sizeof(bank));
    make_synthetic_stored_bpk(data, sizeof(data));
    rc = nexus_v1_dmdf_import_bpk_material_bank_host_route(
        data, sizeof(data), &bank, NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL,
        &host);
    expect(rc == 1, "stored BPK host route imports wall material surfaces");
    expect(host.category == NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL &&
               strcmp(nexus_v1_dgn_material_category_name(host.category),
                      "wall") == 0,
           "host route records wall material category");
    expect(host.upload_route == NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED &&
               host.archive_entries == 4U &&
               host.surface_entries == 3U &&
               host.ready_uploads == 3U,
           "host route carries stored BPK upload receipt");
    expect(host.before_surface_count == 0 &&
               host.after_surface_count == 2 &&
               host.imported_surface_count == 2 &&
               host.imported_truecolor_surface_count == 2 &&
               host.host_consumed_surfaces == 1,
           "host route records consumed truecolor material surfaces");
    expect(host.fallback_visuals_permitted == 0 &&
               host.blocks_real_surface_render == 0,
           "stored host route records material surfaces without fallback permission");

    memset(&coverage, 0, sizeof(coverage));
    expect(nexus_v1_dmdf_material_category_coverage_receipt(
               &bank, NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL,
               wall_refs, 2U, &coverage) == 1,
           "wall material coverage receipt builds");
    expect(coverage.category == NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL &&
               coverage.command_count == 2U &&
               coverage.material_surface_count == 2U &&
               coverage.missing_material_count == 0U &&
               coverage.covered == 1 &&
               coverage.fallback_visuals_permitted == 0,
           "wall coverage accepts all referenced BPK materials");

    memset(&coverage, 0, sizeof(coverage));
    expect(nexus_v1_dmdf_material_category_coverage_receipt(
               &bank, NEXUS_V1_DGN_MATERIAL_CATEGORY_CEILING,
               ceiling_refs, 2U, &coverage) == 1,
           "ceiling material coverage receipt builds");
    expect(coverage.category == NEXUS_V1_DGN_MATERIAL_CATEGORY_CEILING &&
               coverage.command_count == 2U &&
               coverage.material_surface_count == 1U &&
               coverage.missing_material_count == 1U &&
               coverage.first_missing_material_id == 1U &&
               coverage.covered == 0,
           "ceiling coverage reports first missing material without fallback");

    free(bank.surfaces[2].pixels);
    free(bank.surfaces[3].pixels);
}

static void test_prs3_host_route_receipt(void) {
    uint8_t data[160];
    Nexus_DMDFMaterialBank bank;
    Nexus_V1_BpkMaterialHostRouteReceipt host;
    Nexus_V1_DgnMaterialCategoryCoverageReceipt coverage;
    const uint8_t floor_refs[1] = {1U};
    int rc;

    memset(&bank, 0, sizeof(bank));
    bank.surfaces[0].valid = 1;
    bank.surfaces[0].palette[0x11] = 0xff112233U;
    bank.surface_count = 1;
    bank.valid = 1;

    make_synthetic_prs3_literal_bpk(data, sizeof(data));
    rc = nexus_v1_dmdf_import_bpk_material_bank_host_route(
        data, sizeof(data), &bank, NEXUS_V1_DGN_MATERIAL_CATEGORY_FLOOR,
        &host);
    expect(rc == 0, "PRS3 BPK host route refuses unsupported floor decode");
    expect(host.upload_route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3 &&
               host.ready_uploads == 0U &&
               host.blocked_prs3_uploads == 1U &&
               host.imported_prs3_surface_count == 0 &&
               host.host_consumed_surfaces == 0,
           "PRS3 host route records a decoder blocker, not an upload");
    expect(host.expected_upload_bytes == 4U &&
               host.extractable_upload_bytes == 0U &&
               host.fallback_visuals_permitted == 0,
           "PRS3 host route exposes no unsupported upload byte count");

    memset(&coverage, 0, sizeof(coverage));
    expect(nexus_v1_dmdf_material_category_coverage_receipt(
               &bank, NEXUS_V1_DGN_MATERIAL_CATEGORY_FLOOR,
               floor_refs, 1U, &coverage) == 1,
           "floor material coverage receipt builds");
    expect(coverage.category == NEXUS_V1_DGN_MATERIAL_CATEGORY_FLOOR &&
               coverage.command_count == 1U &&
               coverage.material_surface_count == 0U &&
               coverage.covered == 0,
           "floor coverage remains blocked without a PRS3 decoder");

    free(bank.surfaces[1].pixels);
}

static void test_prs3_host_route_blocks_corrupt_stream(void) {
    uint8_t data[160];
    Nexus_DMDFMaterialBank bank;
    Nexus_V1_BpkMaterialHostRouteReceipt host;
    int rc;

    memset(&bank, 0, sizeof(bank));
    bank.surfaces[0].valid = 1;
    bank.surfaces[0].palette[0x11] = 0xff112233U;
    bank.surface_count = 1;
    bank.valid = 1;
    make_synthetic_prs3_literal_bpk(data, sizeof(data));
    data[132] = 0x00U;
    data[133] = 0xeeU;
    data[134] = 0x0fU;

    rc = nexus_v1_dmdf_import_bpk_material_bank_host_route(
        data, sizeof(data), &bank, NEXUS_V1_DGN_MATERIAL_CATEGORY_FLOOR,
        &host);
    expect(rc == 0, "corrupt PRS3 host route does not import a material");
    expect(host.upload_route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3 &&
               host.blocks_real_surface_render == 1 &&
               host.fallback_visuals_permitted == 0 &&
               host.host_consumed_surfaces == 0 &&
               host.imported_surface_count == 0,
           "corrupt PRS3 remains a no-fallback material host-route blocker");
    expect(bank.surfaces[1].valid == 0,
           "corrupt PRS3 cannot populate a DGN material slot");
}

static void test_host_route_rejects_partial_material_archive(void) {
    uint8_t data[256];
    Nexus_DMDFMaterialBank bank;
    Nexus_V1_BpkMaterialHostRouteReceipt host;
    int rc;

    memset(&bank, 0, sizeof(bank));
    make_synthetic_stored_bpk(data, sizeof(data));
    /* Entry 2 remains a valid RGB565 source surface. Entry 3 is made
     * structurally short, so a non-transactional importer would consume
     * entry 2 before discovering the corrupt tail. */
    wr16_be(data + 216U + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 8U);

    rc = nexus_v1_dmdf_import_bpk_material_bank_host_route(
        data, sizeof(data), &bank, NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL,
        &host);
    expect(rc == 0, "mixed valid and truncated BPK archive is not consumed");
    expect(host.upload_route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_TRUNCATED &&
               host.blocks_real_surface_render == 1 &&
               host.host_consumed_surfaces == 0 &&
               host.before_surface_count == 0 &&
               host.after_surface_count == 0,
           "material host route keeps destination bank atomic on corrupt archive");
    expect(bank.surface_count == 0 && !bank.surfaces[2].valid,
           "valid prefix material cannot leak through a corrupt BPK host route");
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

static void test_surface_estimate_capacity_boundary(void) {
    uint8_t data[256];
    Nexus_V1_BpkSurfaceEntry entries[1];
    Nexus_V1_BpkSurfaceEstimate summary;
    int rc;

    memset(entries, 0xCD, sizeof(entries));
    memset(&summary, 0, sizeof(summary));
    make_synthetic_4entry_bpk(data, sizeof(data));

    rc = nexus_v1_bpk_archive_surface_estimate(data, sizeof(data),
                                                entries, 1U, &summary);
    expect(rc == 0, "capacity-1 surface_estimate returns 0");
    expect(summary.total_with_surface == 3U,
           "capacity-1 summary still counts all 3 surface entries");
    expect(summary.total_surface_bytes == 98U,
           "capacity-1 summary still totals all surface bytes");
    expect(summary.trailer_skipped == 1U,
           "capacity-1 summary still counts the trailer skip");
    expect(summary.used == 1U,
           "capacity-1 summary reports only 1 row written");
    expect(summary.truncated == 1,
           "capacity-1 summary marks the output rows truncated");
    expect(entries[0].entry_index == 1U &&
               entries[0].layout.surface_bytes == 16U,
           "capacity-1 output row is the first surface entry");

    memset(&summary, 0, sizeof(summary));
    rc = nexus_v1_bpk_archive_surface_estimate(data, sizeof(data),
                                                NULL, 0U, &summary);
    expect(rc == 0, "summary-only surface_estimate accepts NULL rows");
    expect(summary.total_with_surface == 3U,
           "summary-only surface_estimate counts all surface entries");
    expect(summary.used == 0U,
           "summary-only surface_estimate reports 0 rows written");
    expect(summary.truncated == 0,
           "summary-only surface_estimate does not claim row truncation");
}

static void test_runtime_render_receipt_blocks_prs3(void) {
    uint8_t data[256];
    Nexus_V1_BpkRuntimeRenderReceipt receipt;
    int rc;

    memset(&receipt, 0, sizeof(receipt));
    make_synthetic_4entry_bpk(data, sizeof(data));

    rc = nexus_v1_bpk_archive_runtime_render_receipt(
        data, sizeof(data), &receipt);
    expect(rc == 0, "runtime receipt returns 0 for synthetic BPK");
    expect(receipt.route == NEXUS_V1_BPK_RUNTIME_ROUTE_BLOCKED_PRS3,
           "runtime receipt blocks PRS3-compressed surfaces");
    expect(strcmp(nexus_v1_bpk_runtime_render_route_name(receipt.route),
                  "blocked-prs3") == 0,
           "runtime route name is blocked-prs3");
    expect(receipt.archive_entries == 4U, "runtime receipt: 4 entries");
    expect(receipt.prs3_entries == 3U, "runtime receipt: 3 PRS3 entries");
    expect(receipt.raw_entries == 1U, "runtime receipt: 1 raw trailer entry");
    expect(receipt.surface_entries == 3U, "runtime receipt: 3 surfaces");
    expect(receipt.prs3_surface_entries == 3U,
           "runtime receipt: all surfaces require PRS3");
    expect(receipt.trailer_entries == 1U, "runtime receipt: 1 trailer");
    expect(receipt.expected_surface_bytes == 98U,
           "runtime receipt: expected surface bytes = 98");
    expect(receipt.directory_trailer_found == 1,
           "runtime receipt finds directory trailer");
    expect(receipt.all_prs3_versions_match == 1,
           "runtime receipt: all PRS3 versions match");
    expect(receipt.all_prs3_pixel_counts_match == 1,
           "runtime receipt: all PRS3 pixel counts match");
    expect(receipt.requires_prs3_decoder == 1,
           "runtime receipt requires PRS3 decoder");
    expect(receipt.fallback_visuals_permitted == 0,
           "runtime receipt forbids fallback visuals for PRS3 route");
}

static void test_runtime_render_receipt_ready_for_stored_surfaces(void) {
    uint8_t data[256];
    Nexus_V1_BpkRuntimeRenderReceipt receipt;
    int rc;

    make_synthetic_stored_bpk(data, sizeof(data));

    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_v1_bpk_archive_runtime_render_receipt(
        data, sizeof(data), &receipt);
    expect(rc == 0, "stored-surface runtime receipt returns 0");
    expect(receipt.route == NEXUS_V1_BPK_RUNTIME_ROUTE_READY_STORED,
           "stored surfaces are render-ready without PRS3");
    expect(receipt.prs3_entries == 0U, "stored receipt: 0 PRS3 entries");
    expect(receipt.raw_entries == 4U, "stored receipt: 4 raw entries");
    expect(receipt.stored_surface_entries == 3U,
           "stored receipt: 3 stored surfaces");
    expect(receipt.stored_surface_bytes_available >= 98U,
           "stored receipt: enough stored bytes are available");
    expect(receipt.stored_surface_short_entries == 0U,
           "stored receipt: 0 short stored surfaces");
    expect(receipt.all_stored_surface_payloads_fit == 1,
           "stored receipt: all stored payloads fit");
    expect(receipt.requires_prs3_decoder == 0,
           "stored receipt does not require PRS3 decoder");
    expect(receipt.fallback_visuals_permitted == 1,
           "stored receipt permits normal render path");
}

static void test_runtime_render_receipt_blocks_truncated_stored_surfaces(void) {
    uint8_t data[256];
    Nexus_V1_BpkRuntimeRenderReceipt receipt;
    int rc;

    make_synthetic_4entry_bpk(data, sizeof(data));
    memset(data + 96U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(data + 128U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(data + 160U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);

    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_v1_bpk_archive_runtime_render_receipt(
        data, sizeof(data), &receipt);
    expect(rc == 0, "truncated stored-surface receipt returns 0");
    expect(receipt.route ==
               NEXUS_V1_BPK_RUNTIME_ROUTE_BLOCKED_STORED_TRUNCATED,
           "truncated stored surfaces block the runtime render route");
    expect(strcmp(nexus_v1_bpk_runtime_render_route_name(receipt.route),
                  "blocked-stored-truncated") == 0,
           "runtime route name is blocked-stored-truncated");
    expect(receipt.stored_surface_short_entries > 0U,
           "truncated stored receipt reports short surface entries");
    expect(receipt.all_stored_surface_payloads_fit == 0,
           "truncated stored receipt reports payload-fit failure");
    expect(receipt.fallback_visuals_permitted == 0,
           "truncated stored receipt forbids fallback visuals");
}

static void test_extract_stored_surface_bytes(void) {
    uint8_t data[256];
    uint8_t out[128];
    Nexus_V1_BpkSurfaceEntry surface;
    size_t written = 0U;
    int rc;

    make_synthetic_stored_bpk(data, sizeof(data));
    memset(out, 0, sizeof(out));
    memset(&surface, 0, sizeof(surface));

    rc = nexus_v1_bpk_archive_extract_stored_surface(
        data, sizeof(data), 2U, out, sizeof(out), &surface, &written);
    expect(rc == NEXUS_V1_BPK_EXTRACT_OK,
           "stored RGB565 surface extraction returns OK");
    expect(strcmp(nexus_v1_bpk_surface_extract_status_name(rc), "ok") == 0,
           "stored RGB565 extraction status name is ok");
    expect(written == 64U, "stored RGB565 extraction writes 64 bytes");
    expect(surface.entry_index == 2U &&
               surface.layout.surface_class == NEXUS_V1_BPK_SURFACE_RGB565 &&
               surface.layout.rowstride == 16U &&
               surface.layout.surface_bytes == 64U,
           "stored RGB565 extraction returns surface metadata");
    expect(out[0] == 0x40U && out[63] == 0x7FU,
           "stored RGB565 extraction copies exact payload bytes");

    rc = nexus_v1_bpk_archive_extract_stored_surface(
        data, sizeof(data), 2U, out, 32U, &surface, &written);
    expect(rc == NEXUS_V1_BPK_EXTRACT_ERR_OUTPUT_TOO_SMALL,
           "stored extraction rejects too-small output buffer");

    make_synthetic_4entry_bpk(data, sizeof(data));
    rc = nexus_v1_bpk_archive_extract_stored_surface(
        data, sizeof(data), 1U, out, sizeof(out), &surface, &written);
    expect(rc == NEXUS_V1_BPK_EXTRACT_ERR_PRS3,
           "stored extraction rejects PRS3-compressed entries");

    make_synthetic_stored_bpk(data, sizeof(data));
    rc = nexus_v1_bpk_archive_extract_stored_surface(
        data, sizeof(data), 0U, out, sizeof(out), &surface, &written);
    expect(rc == NEXUS_V1_BPK_EXTRACT_ERR_NOT_SURFACE,
           "stored extraction rejects directory trailer entry");
}

static void test_runtime_surface_handoff_blocks_prs3(void) {
    uint8_t data[256];
    Nexus_V1_BpkRuntimeSurfaceHandoff rows[4];
    Nexus_V1_BpkRuntimeSurfaceHandoffSummary summary;
    int rc;

    make_synthetic_4entry_bpk(data, sizeof(data));
    memset(rows, 0, sizeof(rows));
    memset(&summary, 0, sizeof(summary));

    rc = nexus_v1_bpk_archive_runtime_surface_handoff(
        data, sizeof(data), rows, 4U, &summary);
    expect(rc == 0, "runtime surface handoff returns 0 for PRS3 archive");
    expect(summary.archive_entries == 4U,
           "runtime surface handoff sees 4 archive entries");
    expect(summary.surface_entries == 3U,
           "runtime surface handoff sees 3 surface entries");
    expect(summary.ready_stored_surfaces == 0U,
           "runtime surface handoff has 0 ready stored surfaces");
    expect(summary.blocked_prs3_surfaces == 3U,
           "runtime surface handoff blocks 3 PRS3 surfaces");
    expect(summary.requires_prs3_decoder == 1,
           "runtime surface handoff requires PRS3 decoder");
    expect(summary.extractable_surface_bytes == 0U,
           "runtime surface handoff exposes no extractable PRS3 bytes");
    expect(rows[0].status == NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_PRS3,
           "first PRS3 surface row is blocked-prs3");
    expect(strcmp(nexus_v1_bpk_surface_handoff_status_name(rows[0].status),
                  "blocked-prs3") == 0,
           "blocked PRS3 handoff status name is stable");
    expect(rows[0].extractable == 0,
           "blocked PRS3 row is not extractable");
}

static void test_runtime_surface_handoff_ready_stored(void) {
    uint8_t data[256];
    Nexus_V1_BpkRuntimeSurfaceHandoff rows[4];
    Nexus_V1_BpkRuntimeSurfaceHandoffSummary summary;
    int rc;

    make_synthetic_stored_bpk(data, sizeof(data));
    memset(rows, 0, sizeof(rows));
    memset(&summary, 0, sizeof(summary));

    rc = nexus_v1_bpk_archive_runtime_surface_handoff(
        data, sizeof(data), rows, 4U, &summary);
    expect(rc == 0, "runtime surface handoff returns 0 for stored archive");
    expect(summary.surface_entries == 3U,
           "stored handoff sees 3 surface entries");
    expect(summary.ready_stored_surfaces == 3U,
           "stored handoff has 3 ready surfaces");
    expect(summary.blocked_prs3_surfaces == 0U,
           "stored handoff has 0 PRS3 blockers");
    expect(summary.blocked_truncated_surfaces == 0U,
           "stored handoff has 0 truncated blockers");
    expect(summary.expected_surface_bytes == 98U,
           "stored handoff expected surface bytes = 98");
    expect(summary.extractable_surface_bytes == 98U,
           "stored handoff extractable surface bytes = 98");
    expect(summary.requires_prs3_decoder == 0,
           "stored handoff does not require PRS3 decoder");
    expect(rows[0].entry_index == 1U &&
               rows[0].status ==
                   NEXUS_V1_BPK_SURFACE_HANDOFF_READY_STORED &&
               rows[0].extractable == 1 &&
               rows[0].payload_offset ==
                   96U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES &&
               rows[0].surface.layout.surface_bytes == 16U,
           "stored handoff row 0 exposes extractable entry 1 payload");
    expect(rows[1].entry_index == 2U &&
               rows[1].surface.layout.surface_class ==
                   NEXUS_V1_BPK_SURFACE_RGB565 &&
               rows[1].payload_size >= 64U,
           "stored handoff row 1 exposes RGB565 metadata");
    expect(strcmp(nexus_v1_bpk_surface_handoff_status_name(rows[1].status),
                  "ready-stored") == 0,
           "ready stored handoff status name is stable");
}

static void test_runtime_surface_handoff_truncated_and_capacity(void) {
    uint8_t data[256];
    Nexus_V1_BpkRuntimeSurfaceHandoff rows[1];
    Nexus_V1_BpkRuntimeSurfaceHandoffSummary summary;
    int rc;

    make_synthetic_4entry_bpk(data, sizeof(data));
    memset(data + 96U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(data + 128U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(data + 160U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(rows, 0, sizeof(rows));
    memset(&summary, 0, sizeof(summary));

    rc = nexus_v1_bpk_archive_runtime_surface_handoff(
        data, sizeof(data), rows, 1U, &summary);
    expect(rc == 0, "runtime surface handoff returns 0 for truncated archive");
    expect(summary.surface_entries == 3U,
           "truncated handoff still counts all 3 surface entries");
    expect(summary.blocked_truncated_surfaces == 2U,
           "truncated handoff blocks the two short stored surfaces");
    expect(summary.ready_stored_surfaces == 1U,
           "truncated handoff keeps the one complete stored surface ready");
    expect(summary.extractable_surface_bytes == 18U,
           "truncated handoff exposes only the complete 18-byte surface");
    expect(summary.used == 3U,
           "truncated handoff summary counts all rows despite capacity");
    expect(summary.truncated == 1,
           "truncated handoff marks row output capacity hit");
    expect(rows[0].status == NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_TRUNCATED,
           "first truncated stored row is blocked-truncated");
    expect(strcmp(nexus_v1_bpk_surface_handoff_status_name(rows[0].status),
                  "blocked-truncated") == 0,
           "blocked truncated handoff status name is stable");
}

static void test_runtime_decode_receipt_routes(void) {
    uint8_t data[256];
    Nexus_V1_BpkRuntimeDecodeReceipt receipt;
    int rc;

    make_synthetic_4entry_bpk(data, sizeof(data));
    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_v1_bpk_archive_runtime_decode_receipt(
        data, sizeof(data), &receipt);
    expect(rc == 0, "decode receipt returns 0 for PRS3 archive");
    expect(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3,
           "decode receipt routes PRS3 archive to blocked-prs3");
    expect(strcmp(nexus_v1_bpk_runtime_decode_route_name(receipt.route),
                  "blocked-prs3") == 0,
           "decode receipt blocked-prs3 route name is stable");
    expect(receipt.surface_entries == 3U &&
               receipt.blocked_prs3_surfaces == 3U &&
               receipt.prs3_stream_plans == 1U &&
               receipt.prs3_stream_plan_failures == 2U &&
               receipt.prs3_decode_attempts == 0U &&
               receipt.prs3_decode_successes == 0U &&
               receipt.prs3_decode_failures == 1U &&
               receipt.requires_prs3_decoder == 1 &&
               receipt.decode_blocked == 1,
           "decode receipt exposes PRS3 decode failure blockers");
    expect(receipt.first_blocked_entry == 1U &&
               receipt.first_blocked_stream_size == 0U &&
               receipt.first_blocked_expected_output_bytes == 16U,
           "decode receipt carries first blocked PRS3 failure facts");

    make_synthetic_prs3_literal_bpk(data, sizeof(data));
    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_v1_bpk_archive_runtime_decode_receipt(
        data, sizeof(data), &receipt);
    expect(rc == 0, "decode receipt returns 0 for literal PRS3 archive");
    expect(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3,
           "decode receipt keeps literal PRS3 archive blocked");
    expect(strcmp(nexus_v1_bpk_runtime_decode_route_name(receipt.route),
                  "blocked-prs3") == 0,
           "decode receipt blocked-prs3 route name is stable");
    expect(receipt.prs3_decode_attempts == 0U &&
               receipt.prs3_decode_successes == 0U &&
               receipt.prs3_decode_failures == 1U &&
               receipt.prs3_decoded_surface_bytes == 0U &&
               receipt.decode_blocked == 1,
           "decode receipt exposes the unsupported PRS3 decoder blocker");

    make_synthetic_stored_bpk(data, sizeof(data));
    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_v1_bpk_archive_runtime_decode_receipt(
        data, sizeof(data), &receipt);
    expect(rc == 0, "decode receipt returns 0 for stored archive");
    expect(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED,
           "decode receipt routes stored archive to ready-stored");
    expect(receipt.ready_stored_surfaces == 3U &&
               receipt.blocked_prs3_surfaces == 0U &&
               receipt.prs3_stream_plans == 0U &&
               receipt.prs3_decode_attempts == 0U &&
               receipt.decode_blocked == 0,
           "decode receipt exposes ready stored counts");

    make_synthetic_4entry_bpk(data, sizeof(data));
    memset(data + 96U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(data + 128U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(data + 160U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_v1_bpk_archive_runtime_decode_receipt(
        data, sizeof(data), &receipt);
    expect(rc == 0, "decode receipt returns 0 for truncated stored archive");
    expect(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED,
           "decode receipt routes short stored surfaces to blocked-truncated");
    expect(receipt.blocked_truncated_surfaces == 2U &&
               receipt.decode_blocked == 1,
           "decode receipt exposes truncated stored blockers");
}

static void test_runtime_upload_plan_routes(void) {
    uint8_t data[256];
    Nexus_V1_BpkRuntimeUploadRow rows[4];
    Nexus_V1_BpkRuntimeUploadReceipt receipt;
    int rc;

    make_synthetic_4entry_bpk(data, sizeof(data));
    memset(rows, 0, sizeof(rows));
    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_v1_bpk_archive_runtime_upload_plan(
        data, sizeof(data), rows, 4U, &receipt);
    expect(rc == 0, "upload plan returns 0 for PRS3 archive");
    expect(receipt.route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3,
           "upload plan routes PRS3 archive to blocked-prs3");
    expect(strcmp(nexus_v1_bpk_runtime_upload_route_name(receipt.route),
                  "blocked-prs3") == 0,
           "upload plan blocked-prs3 route name is stable");
    expect(receipt.surface_entries == 3U &&
               receipt.blocked_prs3_uploads == 3U &&
               receipt.ready_uploads == 0U &&
               receipt.blocks_real_menu_surface_render == 1 &&
               receipt.fallback_visuals_permitted == 0,
           "upload plan exposes PRS3 blockers and forbids fallback");
    expect(rows[0].entry_index == 1U &&
               rows[0].status ==
                   NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_PRS3 &&
               rows[0].decode_blocked == 1 &&
               rows[0].upload_ready == 0 &&
               rows[0].expected_output_bytes == 16U,
           "upload plan row carries first PRS3 upload blocker");

    make_synthetic_stored_bpk(data, sizeof(data));
    memset(rows, 0, sizeof(rows));
    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_v1_bpk_archive_runtime_upload_plan(
        data, sizeof(data), rows, 4U, &receipt);
    expect(rc == 0, "upload plan returns 0 for stored archive");
    expect(receipt.route == NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED,
           "upload plan routes stored archive to ready-stored");
    expect(receipt.ready_uploads == 3U &&
               receipt.blocked_prs3_uploads == 0U &&
               receipt.extractable_upload_bytes == 98U &&
               receipt.fallback_visuals_permitted == 0,
           "upload plan exposes stored upload bytes without fallback permission");
    expect(rows[0].entry_index == 1U &&
               rows[0].upload_ready == 1 &&
               rows[0].expected_output_bytes == 16U,
           "upload plan row carries ready stored upload");

    make_synthetic_4entry_bpk(data, sizeof(data));
    memset(data + 96U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(data + 128U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(data + 160U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, 0, 4U);
    memset(rows, 0, sizeof(rows));
    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_v1_bpk_archive_runtime_upload_plan(
        data, sizeof(data), rows, 1U, &receipt);
    expect(rc == 0, "upload plan returns 0 for truncated archive");
    expect(receipt.route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_TRUNCATED,
           "upload plan routes short stored surfaces to blocked-truncated");
    expect(receipt.blocked_truncated_uploads == 2U &&
               receipt.ready_uploads == 1U &&
               receipt.truncated == 1,
           "upload plan exposes truncated blockers and row capacity hit");
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
    Nexus_V1_BpkRuntimeRenderReceipt receipt;
    Nexus_V1_BpkRuntimeDecodeReceipt decode_receipt;
    Nexus_V1_BpkRuntimeUploadRow upload_rows[8];
    Nexus_V1_BpkRuntimeUploadReceipt upload_receipt;
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

    memset(&receipt, 0, sizeof(receipt));
    expect(nexus_v1_bpk_archive_runtime_render_receipt(
               data, size, &receipt) == 0,
           "local MENU.BPK runtime render receipt returns 0");
    expect(receipt.route == NEXUS_V1_BPK_RUNTIME_ROUTE_BLOCKED_PRS3,
           "local MENU.BPK runtime route blocks on PRS3 decoder");
    expect(receipt.archive_entries == 163U,
           "local MENU.BPK runtime receipt sees 163 entries");
    expect(receipt.prs3_entries == 162U,
           "local MENU.BPK runtime receipt sees 162 PRS3 entries");
    expect(receipt.surface_entries == 162U,
           "local MENU.BPK runtime receipt sees 162 surface entries");
    expect(receipt.prs3_surface_entries == 162U,
           "local MENU.BPK runtime receipt sees 162 PRS3 surfaces");
    expect(receipt.trailer_entries == 1U,
           "local MENU.BPK runtime receipt sees 1 trailer");
    expect(receipt.unknown_mode_entries == 0U,
           "local MENU.BPK runtime receipt sees 0 unknown modes");
    expect(receipt.expected_surface_bytes == summary.total_surface_bytes,
           "local MENU.BPK runtime surface total matches summary");
    expect(receipt.directory_trailer_found == 1,
           "local MENU.BPK runtime receipt finds directory trailer");
    expect(receipt.all_prs3_versions_match == 1,
           "local MENU.BPK runtime receipt validates PRS3 version words");
    expect(receipt.all_prs3_pixel_counts_match == 1,
           "local MENU.BPK runtime receipt validates PRS3 pixel counts");
    expect(receipt.requires_prs3_decoder == 1,
           "local MENU.BPK runtime receipt requires PRS3 decoder");
    expect(receipt.fallback_visuals_permitted == 0,
           "local MENU.BPK runtime receipt forbids fallback visuals");

    memset(&decode_receipt, 0, sizeof(decode_receipt));
    expect(nexus_v1_bpk_archive_runtime_decode_receipt(
               data, size, &decode_receipt) == 0,
           "local MENU.BPK runtime decode receipt returns 0");
    expect(decode_receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3,
           "local MENU.BPK decode route remains blocked-prs3");
    expect(decode_receipt.prs3_decode_attempts == 0U &&
               decode_receipt.prs3_decode_successes == 0U &&
               decode_receipt.prs3_decode_failures == 162U &&
               decode_receipt.prs3_decoded_surface_bytes == 0U,
           "local MENU.BPK records every unsupported PRS3 surface");
    expect(decode_receipt.first_blocked_entry == 1U &&
               decode_receipt.first_blocked_decode_status ==
                   NEXUS_V1_BPK_DECODE_ERR_STREAM,
           "local MENU.BPK decode receipt retains its first stream blocker");
    expect(decode_receipt.decode_blocked == 1,
           "local MENU.BPK decode receipt blocks upload without fallback");

    memset(upload_rows, 0, sizeof(upload_rows));
    memset(&upload_receipt, 0, sizeof(upload_receipt));
    expect(nexus_v1_bpk_archive_runtime_upload_plan(
               data, size, upload_rows,
               (uint32_t)(sizeof(upload_rows) / sizeof(upload_rows[0])),
               &upload_receipt) == 0,
           "local MENU.BPK upload plan returns 0");
    expect(upload_receipt.route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3,
           "local MENU.BPK upload route is blocked-prs3");
    expect(upload_receipt.surface_entries == 162U &&
               upload_receipt.blocked_prs3_uploads == 162U &&
               upload_receipt.ready_uploads == 0U &&
               upload_receipt.planned_rows == 162U,
           "local MENU.BPK upload plan exposes all blocked PRS3 surfaces");
    expect(upload_receipt.truncated == 1,
           "local MENU.BPK upload plan marks bounded row receipt");
    expect(upload_receipt.extractable_upload_bytes == 0U &&
               upload_receipt.fallback_visuals_permitted == 0,
           "local MENU.BPK upload plan exposes no unsupported upload bytes");
    expect(upload_rows[0].entry_index == 1U &&
               upload_rows[0].decode_blocked == 1 &&
               upload_rows[0].upload_ready == 0 &&
               upload_rows[0].expected_output_bytes ==
                   (uint32_t)upload_rows[0].width *
                       (uint32_t)upload_rows[0].height,
           "local MENU.BPK upload row carries a PRS3 blocker");

    free(data);
}

int main(void) {
    test_mode_to_surface_class();
    test_mode_to_bpp();
    test_synthetic_surface_estimate();
    test_surface_estimate_capacity_boundary();
    test_runtime_render_receipt_blocks_prs3();
    test_runtime_render_receipt_ready_for_stored_surfaces();
    test_runtime_render_receipt_blocks_truncated_stored_surfaces();
    test_extract_stored_surface_bytes();
    test_prs3_surface_decode();
    test_prs3_candidate_evidence();
    test_prs3_material_import();
    test_truecolor_material_import();
    test_material_host_route_and_category_coverage();
    test_prs3_host_route_receipt();
    test_prs3_host_route_blocks_corrupt_stream();
    test_host_route_rejects_partial_material_archive();
    test_runtime_surface_handoff_blocks_prs3();
    test_runtime_surface_handoff_ready_stored();
    test_runtime_surface_handoff_truncated_and_capacity();
    test_runtime_decode_receipt_routes();
    test_runtime_upload_plan_routes();
    test_bpx3_trailer_entry();
    test_bpx3_trailer_rejections();
    test_bpx3_prs3_span_rejections();
    test_optional_local_menu_bpk();

    if (g_failures) return 1;
    puts("test_nexus_v1_bpk_surface_class: PASS");
    return 0;
}
