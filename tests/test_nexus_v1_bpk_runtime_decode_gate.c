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
                                  int prs3,
                                  uint8_t payload_bytes)
{
    const uint32_t entry_offset = 28U;
    const uint32_t prs3_header = prs3 ? NEXUS_V1_BPK_PRS3_HEADER_BYTES : 0U;
    size_t size = (size_t)entry_offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
                  prs3_header + payload_bytes;
    uint8_t *entry;
    uint32_t payload_offset;

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

    if (prs3) {
        payload_offset = entry_offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES;
        write_be32(data + payload_offset, NEXUS_V1_BPK_MAGIC_PRS3);
        write_be32(data + payload_offset + 4U, NEXUS_V1_BPK_PRS3_VERSION);
        write_be32(data + payload_offset + 8U, 4U);
        if (payload_bytes >= 4U) {
            write_be32(data + payload_offset + NEXUS_V1_BPK_PRS3_HEADER_BYTES,
                       payload_bytes);
        }
    }
    return size;
}

static void check_ready_stored_decode(void)
{
    uint8_t archive[80];
    Nexus_V1_BpkRuntimeDecodeReceipt receipt;
    size_t size = build_one_entry_bpk(
        archive, sizeof(archive), NEXUS_V1_BPK_MODE_8BPP, 0, 4U);

    check(size > 0U, "ready stored decode fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_decode_receipt(
              archive, size, &receipt) == 0,
          "ready stored decode receipt parses");
    check(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED &&
              strcmp(nexus_v1_bpk_runtime_decode_route_name(receipt.route),
                     "ready-stored") == 0 &&
              receipt.ready_stored_surfaces == 1U &&
              !receipt.decode_blocked,
          "stored MENU.BPK surfaces are the only ready decode route here");
}

static void check_no_surface_decode_blocks(void)
{
    uint8_t archive[80];
    Nexus_V1_BpkRuntimeDecodeReceipt receipt;
    size_t size = build_one_entry_bpk(
        archive, sizeof(archive), NEXUS_V1_BPK_MODE_TRAILER, 0, 0U);

    check(size > 0U, "no-surface decode fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_decode_receipt(
              archive, size, &receipt) == 0,
          "no-surface decode receipt parses");
    check(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_NO_SURFACES &&
              strcmp(nexus_v1_bpk_runtime_decode_route_name(receipt.route),
                     "no-surfaces") == 0 &&
              receipt.surface_entries == 0U &&
              receipt.decode_blocked,
          "no-surface MENU.BPK decode is fail-closed before renderer handoff");
}

static void check_truncated_decode_blocks(void)
{
    uint8_t archive[80];
    Nexus_V1_BpkRuntimeDecodeReceipt receipt;
    size_t size = build_one_entry_bpk(
        archive, sizeof(archive), NEXUS_V1_BPK_MODE_8BPP, 0, 2U);

    check(size > 0U, "truncated stored decode fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_decode_receipt(
              archive, size, &receipt) == 0,
          "truncated stored decode receipt parses");
    check(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED &&
              receipt.blocked_truncated_surfaces == 1U &&
              receipt.decode_blocked,
          "short stored MENU.BPK payload blocks runtime decode");
}

static void check_prs3_decode_blocks_without_pixels(void)
{
    uint8_t archive[96];
    Nexus_V1_BpkRuntimeDecodeReceipt receipt;
    size_t size = build_one_entry_bpk(
        archive, sizeof(archive), NEXUS_V1_BPK_MODE_8BPP, 1, 4U);

    check(size > 0U, "PRS3 decode fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_decode_receipt(
              archive, size, &receipt) == 0,
          "PRS3 decode receipt parses");
    check(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3 &&
              strcmp(nexus_v1_bpk_runtime_decode_route_name(receipt.route),
                     "blocked-prs3") == 0 &&
              receipt.requires_prs3_decoder &&
              receipt.prs3_stream_plans == 1U &&
              receipt.prs3_decode_failures == 1U &&
              receipt.prs3_decode_successes == 0U &&
              receipt.prs3_evidence_only &&
              receipt.prs3_decoder_promoted == 0 &&
              receipt.prs3_decoded_pixels_emitted == 0U &&
              receipt.renderer_handoff_blocked &&
              !receipt.fallback_visuals_permitted &&
              receipt.decode_blocked,
          "PRS3 MENU.BPK surfaces stay blocked until decoder proof exists");
}

int main(void)
{
    check_ready_stored_decode();
    check_no_surface_decode_blocks();
    check_truncated_decode_blocks();
    check_prs3_decode_blocks_without_pixels();

    if (failures) {
        fprintf(stderr, "Nexus BPK runtime decode gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus BPK runtime decode gate: PASS");
    return 0;
}
