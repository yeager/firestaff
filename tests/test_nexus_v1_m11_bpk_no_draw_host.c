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

static void build_engine(Nexus_V1_Engine *engine,
                         Nexus_V1_BpkRuntimeUploadRow *row)
{
    memset(engine, 0, sizeof(*engine));
    memset(row, 0, sizeof(*row));
    engine->menu_bpk_package_fnv1a64 = UINT64_C(0x1122334455667788);
    engine->menu_bpk_upload_receipt_valid = 1;
    engine->menu_bpk_upload_row_count = 1;
    engine->menu_bpk_upload_receipt.first_prs3_entry_index = 3U;
    engine->menu_bpk_upload_receipt.first_prs3_payload_offset = 64U;
    engine->menu_bpk_upload_receipt.first_prs3_payload_size = 20U;
    engine->menu_bpk_upload_receipt.first_prs3_payload_fnv1a64 =
        UINT64_C(0x8877665544332211);
    engine->menu_bpk_upload_receipt.first_prs3_version =
        NEXUS_V1_BPK_PRS3_VERSION;
    engine->menu_bpk_upload_receipt.first_prs3_pixel_count = 4U;
    engine->menu_bpk_upload_receipt.first_prs3_header_first_u32 = 12U;
    engine->menu_bpk_upload_receipt.first_prs3_header_minus_payload = 0U;
    row->entry_index = 3U;
    row->payload_offset = 64U;
    row->payload_size = 20U;
    row->payload_fnv1a64 = UINT64_C(0x8877665544332211);
    row->stream_offset = 72U;
    row->stream_size = 12U;
    row->header_first_u32 = 12U;
    row->header_minus_payload = 0U;
    row->prs3_version = NEXUS_V1_BPK_PRS3_VERSION;
    row->prs3_pixel_count = 4U;
    row->mode = NEXUS_V1_BPK_MODE_8BPP;
    row->expected_output_bytes = 4U;
    row->prs3_header_valid = 1;
    row->compression.valid = 1;
    row->compression.entry_index = row->entry_index;
    row->compression.mode_flags = row->mode;
    row->compression.declared_pixel_count = row->prs3_pixel_count;
    row->compression.declared_output_bytes = row->expected_output_bytes;
    row->compression.compressed_offset = 76U;
    row->compression.compressed_length = 8U;
    row->compression.compressed_fnv1a64 = UINT64_C(0x1);
    engine->menu_bpk_upload_receipt.first_prs3_compression = row->compression;
    row->decode_blocked = 1;
    row->evidence_only = 1;
    row->renderer_handoff_blocked = 1;
    row->upload_blocked = 1;
    engine->menu_bpk_upload_rows[0] = *row;
}

int main(void)
{
    Nexus_V1_Engine engine;
    Nexus_V1_BpkRuntimeUploadRow row;
    Nexus_V1_BpkRuntimeUploadRow selected_row;
    Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt presentation;
    Nexus_V1_LauncherM11MenuBpkNoDrawHostReceipt host;
    uint64_t package;

    build_engine(&engine, &row);
    package = engine.menu_bpk_package_fnv1a64;
    check(nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
              &engine.menu_bpk_upload_receipt, &row, &presentation) == 1,
          "source-validated PRS3 row creates presentation evidence");
    check(nexus_v1_launcher_admit_m11_menu_bpk_no_draw_host(
              &engine, 7U, package, &presentation, &host) == 1 &&
              host.valid && host.no_draw_only && host.draw_disabled,
          "exact package and epoch admit M11 no-draw host evidence");
    check(nexus_v1_launcher_consume_m11_menu_bpk_no_draw_host(
              &engine, 7U, package, &host) == 1 &&
              host.presentation.entry_index == 3U && host.draw_disabled,
          "M11 host consumes only the admitted receipt without draw promotion");
    ++engine.menu_bpk_upload_rows[0].compression.compressed_fnv1a64;
    check(!nexus_v1_launcher_consume_m11_menu_bpk_no_draw_host(
              &engine, 7U, package, &host) && !host.valid,
          "stale compressed row cannot be reused across host consumption");
    --engine.menu_bpk_upload_rows[0].compression.compressed_fnv1a64;
    check(nexus_v1_launcher_consume_m11_menu_bpk_no_draw_host(
              &engine, 7U, package, &host) == 1 &&
              host.presentation.compression.declared_output_bytes == 4U,
          "restored current row rebuilds the complete no-draw presentation");
    check(!nexus_v1_launcher_consume_m11_menu_bpk_no_draw_host(
               &engine, 8U, package, &host) && !host.valid,
          "stale route epoch rejects host consumption");
    check(!nexus_v1_launcher_consume_m11_menu_bpk_no_draw_host(
               &engine, 7U, package + 1U, &host) && !host.valid,
          "package drift rejects host consumption");

    presentation.entry_index = 4U;
    check(!nexus_v1_launcher_admit_m11_menu_bpk_no_draw_host(
               &engine, 8U, package, &presentation, &host) && !host.valid &&
              !engine.menu_bpk_no_draw_host_valid,
          "cross-entry admission rejects and clears prior host state");
    presentation.entry_index = 3U;
    check(nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
              &engine.menu_bpk_upload_receipt, &row, &presentation) == 1 &&
              nexus_v1_launcher_admit_m11_menu_bpk_no_draw_host(
                  &engine, 9U, package, &presentation, &host) == 1 &&
              host.no_draw_only && host.draw_disabled,
          "clean later epoch re-admits no-draw evidence only");

    selected_row = row;
    selected_row.entry_index = 4U;
    selected_row.payload_offset = 84U;
    selected_row.payload_size = 24U;
    selected_row.payload_fnv1a64 = UINT64_C(0x1122334455667788);
    selected_row.stream_offset = 92U;
    selected_row.stream_size = 16U;
    selected_row.header_first_u32 = 16U;
    selected_row.expected_output_bytes = 8U;
    selected_row.prs3_pixel_count = 8U;
    selected_row.compression.entry_index = selected_row.entry_index;
    selected_row.compression.declared_pixel_count =
        selected_row.prs3_pixel_count;
    selected_row.compression.declared_output_bytes =
        selected_row.expected_output_bytes;
    selected_row.compression.compressed_offset = 96U;
    selected_row.compression.compressed_length = 12U;
    selected_row.compression.compressed_fnv1a64 = UINT64_C(0x2);
    engine.menu_bpk_upload_rows[1] = selected_row;
    engine.menu_bpk_upload_row_count = 2;
    check(nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
              &engine.menu_bpk_upload_receipt, &selected_row,
              &presentation) == 1 && presentation.entry_index == 4U &&
              presentation.compression.declared_output_bytes == 8U &&
              nexus_v1_launcher_admit_m11_menu_bpk_no_draw_host(
                  &engine, 10U, package, &presentation, &host) == 1 &&
              host.presentation.entry_index == 4U &&
              host.presentation.compression.compressed_fnv1a64 ==
                  UINT64_C(0x2),
          "exact non-first PRS3 entry is selected as no-draw M11 evidence");

    ++presentation.compression.compressed_fnv1a64;
    check(!nexus_v1_launcher_admit_m11_menu_bpk_no_draw_host(
              &engine, 11U, package, &presentation, &host) && !host.valid &&
              !engine.menu_bpk_no_draw_host_valid,
          "selected compressed-body drift rejects and clears M11 host state");

    check(nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
              &engine.menu_bpk_upload_receipt, &selected_row,
              &presentation) == 1,
          "selected row can be rebuilt from engine-owned provenance");
    ++presentation.compression.declared_output_bytes;
    check(!nexus_v1_launcher_admit_m11_menu_bpk_no_draw_host(
              &engine, 11U, package, &presentation, &host) && !host.valid &&
              !engine.menu_bpk_no_draw_host_valid,
          "selected declared-output drift rejects and clears M11 host state");

    if (failures) {
        fprintf(stderr, "Nexus M11 BPK no-draw host: %d failure(s)\n", failures);
        return 1;
    }
    puts("Nexus M11 BPK no-draw host: PASS");
    return 0;
}
