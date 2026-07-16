#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_dmdf_model.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void write_be32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

static size_t build_one_prs3_bpk(uint8_t *data, size_t capacity)
{
    const uint32_t entry_offset = 28U;
    const uint32_t payload_offset =
        entry_offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES;
    const uint32_t stream_offset =
        payload_offset + NEXUS_V1_BPK_PRS3_HEADER_BYTES;
    const uint8_t body[] = {0x0fU, 0x11U, 0x22U, 0x33U, 0x44U};
    const uint32_t stream_size = 4U + (uint32_t)sizeof(body);
    const uint32_t framed_size = stream_size + 4U;
    const size_t size = (size_t)stream_offset + stream_size;
    uint8_t *entry;

    if (!data || capacity < size) {
        return 0U;
    }
    memset(data, 0, capacity);
    write_be32(data + 0, NEXUS_V1_BPK_MAGIC_BPPK);
    write_be32(data + 4, (uint32_t)size);
    write_be32(data + 12, NEXUS_V1_BPK_MAGIC_BMPD);
    write_be32(data + 16, (uint32_t)(size - 12U));
    write_be32(data + 20, 1U);
    write_be32(data + 24, entry_offset);

    entry = data + entry_offset;
    entry[NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET] = 0U;
    entry[NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET + 1U] = 2U;
    entry[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 2U;
    entry[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_8BPP;

    write_be32(data + payload_offset, NEXUS_V1_BPK_MAGIC_PRS3);
    write_be32(data + payload_offset + 4U, NEXUS_V1_BPK_PRS3_VERSION);
    write_be32(data + payload_offset + 8U, 4U);
    write_be32(data + stream_offset, framed_size);
    memcpy(data + stream_offset + 4U, body, sizeof(body));
    return size;
}

static void check_prs3_bpk_cannot_promote_dgn_material(void)
{
    uint8_t archive[128];
    Nexus_DMDFMaterialBank bank;
    Nexus_V1_BpkMaterialHostRouteReceipt receipt;
    size_t size = build_one_prs3_bpk(archive, sizeof(archive));

    memset(&bank, 0, sizeof(bank));
    bank.valid = 1;
    bank.surface_count = 3;
    bank.surfaces[0].valid = 1;

    check(size > 0U, "PRS3 DGN material fixture is bounded");
    check(nexus_v1_dmdf_import_bpk_material_bank_host_route(
              archive, size, &bank, NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL,
              &receipt) == 0,
          "PRS3 BPK material host route refuses import");
    check(receipt.category == NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL &&
              receipt.upload_route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3 &&
              receipt.blocked_prs3_uploads == 1U &&
              receipt.prs3_evidence_only &&
              receipt.prs3_decoder_promoted == 0 &&
              receipt.prs3_decoded_pixels_emitted == 0U &&
              receipt.prs3_upload_blocked &&
              receipt.renderer_handoff_blocked &&
              receipt.blocks_real_surface_render &&
              receipt.before_surface_count == 3 &&
              receipt.after_surface_count == 3 &&
              receipt.imported_surface_count == 0 &&
              receipt.imported_prs3_surface_count == 0 &&
              !receipt.host_consumed_surfaces &&
              receipt.material_pixel_promotion_blocked &&
              receipt.material_promotion_blocked &&
              receipt.material_bank_mutation_blocked &&
              !receipt.fallback_visuals_permitted,
          "PRS3 BPK cannot mutate or promote DGN material bank");
    check(bank.surface_count == 3 &&
              bank.bpk_imported_surface_count == 0 &&
              bank.bpk_prs3_surface_count == 0 &&
              bank.surfaces[0].valid,
          "blocked PRS3 material route leaves existing bank untouched");
}

int main(void)
{
    check_prs3_bpk_cannot_promote_dgn_material();

    if (failures) {
        fprintf(stderr,
                "Nexus DGN material no-promotion gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus DGN material no-promotion gate: PASS");
    return 0;
}
