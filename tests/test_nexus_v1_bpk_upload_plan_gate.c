#include "nexus_v1_bpk_archive.h"

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

static size_t build_one_entry_bpk(uint8_t *data,
                                  size_t capacity,
                                  uint8_t mode,
                                  uint8_t payload_bytes)
{
    const uint32_t entry_offset = 28U;
    size_t size = (size_t)entry_offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
                  payload_bytes;
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
    entry[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = mode;
    return size;
}

static size_t build_one_prs3_entry_bpk(uint8_t *data, size_t capacity)
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

static void check_ready_stored_route(void)
{
    uint8_t archive[64];
    Nexus_V1_BpkRuntimeUploadRow rows[1];
    Nexus_V1_BpkRuntimeUploadReceipt receipt;
    size_t size = build_one_entry_bpk(
        archive, sizeof(archive), NEXUS_V1_BPK_MODE_8BPP, 4U);

    check(size > 0U, "ready stored fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_upload_plan(
              archive, size, rows, 1U, &receipt) == 0,
          "ready stored upload plan parses");
    check(receipt.route == NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED &&
              strcmp(nexus_v1_bpk_runtime_upload_route_name(receipt.route),
                     "ready-stored") == 0 &&
              receipt.ready_uploads == 1U &&
              !receipt.truncated &&
              !receipt.blocks_real_menu_surface_render &&
              !receipt.fallback_visuals_permitted,
          "ready stored MENU.BPK plan is the only unblocked stored route");
}

static void check_truncated_payload_route(void)
{
    uint8_t archive[64];
    Nexus_V1_BpkRuntimeUploadRow rows[1];
    Nexus_V1_BpkRuntimeUploadReceipt receipt;
    size_t size = build_one_entry_bpk(
        archive, sizeof(archive), NEXUS_V1_BPK_MODE_8BPP, 2U);

    check(size > 0U, "truncated payload fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_upload_plan(
              archive, size, rows, 1U, &receipt) == 0,
          "truncated payload upload plan parses");
    check(receipt.route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_TRUNCATED &&
              receipt.blocked_truncated_uploads == 1U &&
              receipt.blocks_real_menu_surface_render &&
              !receipt.fallback_visuals_permitted,
          "truncated MENU.BPK payload blocks boot/menu rendering");
}

static void check_no_surface_route(void)
{
    uint8_t archive[64];
    Nexus_V1_BpkRuntimeUploadRow rows[1];
    Nexus_V1_BpkRuntimeUploadReceipt receipt;
    size_t size = build_one_entry_bpk(
        archive, sizeof(archive), NEXUS_V1_BPK_MODE_TRAILER, 0U);

    check(size > 0U, "no-surface fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_upload_plan(
              archive, size, rows, 1U, &receipt) == 0,
          "no-surface upload plan parses");
    check(receipt.route == NEXUS_V1_BPK_UPLOAD_ROUTE_NO_SURFACES &&
              strcmp(nexus_v1_bpk_runtime_upload_route_name(receipt.route),
                     "no-surfaces") == 0 &&
              receipt.surface_entries == 0U &&
              receipt.blocks_real_menu_surface_render &&
              !receipt.fallback_visuals_permitted,
          "no-surface MENU.BPK plan blocks boot/menu rendering");
}

static void check_capacity_route(void)
{
    uint8_t archive[64];
    Nexus_V1_BpkRuntimeUploadRow rows[1];
    Nexus_V1_BpkRuntimeUploadReceipt receipt;
    size_t size = build_one_entry_bpk(
        archive, sizeof(archive), NEXUS_V1_BPK_MODE_8BPP, 4U);

    check(size > 0U, "capacity fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_upload_plan(
              archive, size, rows, 0U, &receipt) == 0,
          "capacity-limited upload plan parses");
    check(receipt.route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_CAPACITY &&
              strcmp(nexus_v1_bpk_runtime_upload_route_name(receipt.route),
                     "blocked-capacity") == 0 &&
              receipt.truncated &&
              receipt.ready_uploads == 1U &&
              receipt.blocks_real_menu_surface_render &&
              !receipt.fallback_visuals_permitted,
          "capacity-limited MENU.BPK plan cannot claim ready stored routing");
}

static void check_prs3_upload_route_blocks_pixels(void)
{
    uint8_t archive[128];
    Nexus_V1_BpkRuntimeUploadRow rows[1];
    Nexus_V1_BpkRuntimeUploadReceipt receipt;
    size_t size = build_one_prs3_entry_bpk(archive, sizeof(archive));

    check(size > 0U, "PRS3 upload fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_upload_plan(
              archive, size, rows, 1U, &receipt) == 0,
          "PRS3 upload plan parses");
    check(receipt.route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3 &&
              strcmp(nexus_v1_bpk_runtime_upload_route_name(receipt.route),
                     "blocked-prs3") == 0 &&
              receipt.blocked_prs3_uploads == 1U &&
              receipt.ready_uploads == 0U &&
              receipt.prs3_evidence_only &&
              receipt.prs3_decoder_promoted == 0 &&
              receipt.prs3_decoded_pixels_emitted == 0U &&
              receipt.prs3_upload_blocked &&
              receipt.blocks_real_menu_surface_render &&
              !receipt.fallback_visuals_permitted,
          "PRS3 MENU.BPK upload route blocks renderer pixels");
    check(rows[0].status == NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_PRS3 &&
              rows[0].decode_blocked &&
              rows[0].evidence_only &&
              rows[0].renderer_handoff_blocked &&
              rows[0].upload_blocked &&
              rows[0].decoded_pixels_emitted == 0U &&
              !rows[0].fallback_visuals_permitted &&
              !rows[0].upload_ready,
          "PRS3 upload row remains diagnostic-only");
}

int main(void)
{
    check_ready_stored_route();
    check_truncated_payload_route();
    check_no_surface_route();
    check_capacity_route();
    check_prs3_upload_route_blocks_pixels();

    if (failures) {
        fprintf(stderr, "Nexus BPK upload plan gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus BPK upload plan gate: PASS");
    return 0;
}
