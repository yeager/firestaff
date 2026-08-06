#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_launcher.h"

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

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;

    for (i = 0U; i < size; ++i) {
        hash ^= (uint64_t)data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static size_t build_prs3_bpk(uint8_t *data, size_t capacity)
{
    const uint32_t entry_offset = 28U;
    const uint32_t stream_bytes = 8U;
    const size_t size = entry_offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
        NEXUS_V1_BPK_PRS3_HEADER_BYTES + stream_bytes;
    uint8_t *entry;
    uint32_t prs3_offset;

    if (!data || capacity < size) {
        return 0U;
    }
    memset(data, 0, capacity);
    write_be32(data + 0U, NEXUS_V1_BPK_MAGIC_BPPK);
    write_be32(data + 4U, (uint32_t)size);
    write_be32(data + 12U, NEXUS_V1_BPK_MAGIC_BMPD);
    write_be32(data + 16U, (uint32_t)size - 12U);
    write_be32(data + 20U, 1U);
    write_be32(data + 24U, entry_offset);
    entry = data + entry_offset;
    entry[NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET + 1U] = 2U;
    entry[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 2U;
    entry[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_8BPP;
    prs3_offset = entry_offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES;
    write_be32(data + prs3_offset, NEXUS_V1_BPK_MAGIC_PRS3);
    write_be32(data + prs3_offset + 4U, NEXUS_V1_BPK_PRS3_VERSION);
    write_be32(data + prs3_offset + 8U, 4U);
    write_be32(data + prs3_offset + NEXUS_V1_BPK_PRS3_HEADER_BYTES, 4U);
    data[prs3_offset + NEXUS_V1_BPK_PRS3_HEADER_BYTES + 4U] = 0xa1U;
    data[prs3_offset + NEXUS_V1_BPK_PRS3_HEADER_BYTES + 5U] = 0xb2U;
    data[prs3_offset + NEXUS_V1_BPK_PRS3_HEADER_BYTES + 6U] = 0xc3U;
    data[prs3_offset + NEXUS_V1_BPK_PRS3_HEADER_BYTES + 7U] = 0xd4U;
    return size;
}

int main(void)
{
    uint8_t archive[96];
    Nexus_V1_BpkRuntimeUploadRow rows[2];
    Nexus_V1_BpkRuntimeUploadRow unknown_codec;
    Nexus_V1_BpkRuntimeUploadReceipt upload;
    Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt presentation;
    size_t size = build_prs3_bpk(archive, sizeof(archive));

    check(size > 0U, "fixture is bounded");
    check(nexus_v1_bpk_archive_runtime_upload_plan(
              archive, size, rows, 2U, &upload) == 0,
          "validated PRS3 upload row is built");
    check(upload.first_prs3_entry_index == rows[0].entry_index &&
              upload.first_prs3_payload_fnv1a64 == rows[0].payload_fnv1a64 &&
              rows[0].payload_fnv1a64 == fnv1a64(
                  archive + rows[0].payload_offset, rows[0].payload_size) &&
              rows[0].prs3_header_valid && rows[0].compression.valid &&
              rows[0].compression.compressed_length == 4U &&
              rows[0].compression.declared_output_bytes == 4U &&
              rows[0].compression.compressed_fnv1a64 == fnv1a64(
                  archive + rows[0].compression.compressed_offset,
                  rows[0].compression.compressed_length),
          "upload receipt binds bounded payload FNV and header facts");
    check(nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
              &upload, &rows[0], &presentation) && presentation.valid &&
              presentation.no_draw_only,
          "matching entry is admitted as no-draw presentation evidence");

    ++rows[0].payload_fnv1a64;
    check(!nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
               &upload, &rows[0], &presentation) && !presentation.valid,
          "payload FNV drift rejects and clears presentation evidence");
    --rows[0].payload_fnv1a64;

    ++rows[0].compression.compressed_fnv1a64;
    check(!nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
               &upload, &rows[0], &presentation) && !presentation.valid,
          "compressed-span FNV drift rejects presentation evidence");
    --rows[0].compression.compressed_fnv1a64;

    ++rows[0].compression.compressed_length;
    check(!nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
               &upload, &rows[0], &presentation) && !presentation.valid,
          "out-of-bounds compressed span rejects presentation evidence");
    --rows[0].compression.compressed_length;

    unknown_codec = rows[0];
    unknown_codec.compression.mode_flags = 0xffU;
    check(!nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
               &upload, &unknown_codec, &presentation) && !presentation.valid,
          "unknown compression mode rejects before M11");

    archive[28U + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = 0xffU;
    check(nexus_v1_bpk_archive_runtime_upload_plan(
              archive, size, rows, 2U, &upload) == 0 &&
              upload.unknown_prs3_mode_entries == 1U &&
              !nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
                  &upload, &rows[0], &presentation) && !presentation.valid,
          "archive unknown mode is rejected before M11 presentation admission");

    archive[28U + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_8BPP;
    write_be32(archive + 28U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 8U, 3U);
    check(nexus_v1_bpk_archive_runtime_upload_plan(
              archive, size, rows, 2U, &upload) == 0 &&
              upload.first_prs3_entry_index == UINT32_MAX &&
              !nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
                  &upload, &rows[0], &presentation) && !presentation.valid,
          "corrupt declared PRS3 output rejects before M11 presentation admission");

    unknown_codec = rows[0];
    unknown_codec.decode_blocked = 0;
    check(!nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
               &upload, &unknown_codec, &presentation) && !presentation.valid,
          "non-PRS3 or draw-capable rows cannot enter the no-draw route");

    rows[0].payload_offset = UINT32_MAX - 4U;
    upload.first_prs3_payload_offset = rows[0].payload_offset;
    check(!nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
               &upload, &rows[0], &presentation) && !presentation.valid,
          "overflowing payload span rejects presentation evidence");

    if (failures) {
        fprintf(stderr, "Nexus BPK no-draw presentation: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus BPK no-draw presentation: PASS");
    return 0;
}
