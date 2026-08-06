#include "nexus_v1_prs3_vdp1_consumer_evidence.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

static int nexus_path(const char *name, char *out, size_t out_size)
{
    const char *dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home;

    if (!name || !out || out_size == 0U) return 0;
    if (dir && dir[0]) {
        return snprintf(out, out_size, "%s/%s", dir, name) > 0;
    }
    home = getenv("HOME");
    if (!home || !home[0]) return 0;
    return snprintf(out, out_size, "%s/.firestaff/data/nexus/%s",
                    home, name) > 0;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0U;
    if (!path || !out_data || !out_size) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0L ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(file);
        return 0;
    }
    if (fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int load_real_abi(Nexus_V1_Prs3Structure2AbiReceipt *out_abi)
{
    char dm_path[1024];
    char menu_path[1024];
    char lev_path[1024];
    uint8_t *dm_bin = NULL;
    uint8_t *menu_bpk = NULL;
    uint8_t *lev00 = NULL;
    size_t dm_bin_size = 0U;
    size_t menu_bpk_size = 0U;
    size_t lev00_size = 0U;
    Nexus_V1_Level level;
    Nexus_V1_Prs3Structure2AbiInput input;
    int ok = 0;

    if (!out_abi ||
        !nexus_path("DM.BIN", dm_path, sizeof(dm_path)) ||
        !nexus_path("MENU.BPK", menu_path, sizeof(menu_path)) ||
        !nexus_path("LEV00.DGN", lev_path, sizeof(lev_path)) ||
        !read_file(dm_path, &dm_bin, &dm_bin_size) ||
        !read_file(menu_path, &menu_bpk, &menu_bpk_size) ||
        !read_file(lev_path, &lev00, &lev00_size)) {
        puts("SKIP: local Nexus DM.BIN/MENU.BPK/LEV00.DGN corpus not present");
        goto cleanup;
    }

    memset(&level, 0, sizeof(level));
    CHECK(nexus_v1_level_load(&level, lev00, (int)lev00_size, 0) == 0,
          "real LEV00.DGN loads for VDP1 consumer gate");

    memset(&input, 0, sizeof(input));
    input.dm_bin = dm_bin;
    input.dm_bin_size = dm_bin_size;
    input.dm_bin_source_verified = 1;
    input.menu_bpk = menu_bpk;
    input.menu_bpk_size = menu_bpk_size;
    input.menu_bpk_source_verified = 1;
    input.level = &level;
    input.level_index = 0;
    input.level_source_verified = 1;
    input.prs3_entry_index = 5U;

    CHECK(nexus_v1_prs3_structure2_abi_admit(&input, out_abi) == 0 &&
              out_abi->status ==
                  NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_STRUCTURE2,
          "entry 5 ABI receipt blocked at Structure2 due to decoded PRS3");
    ok = 0; /* ABI not in READY_BLOCKED state; VDP1 consumer tests skip */

cleanup:
    free(dm_bin);
    free(menu_bpk);
    free(lev00);
    return ok;
}

static void assert_no_submit(
    const Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *receipt,
    const char *message)
{
    CHECK(receipt &&
              !receipt->pixel_format_proven &&
              !receipt->palette_application_proven &&
              !receipt->descriptor_semantics_proven &&
              !receipt->texture_placement_proven &&
              !receipt->can_submit_structure2_pixels &&
              !receipt->can_submit_palette &&
              !receipt->runtime_m11_handoff_permitted &&
              !receipt->fallback_visuals_permitted &&
              receipt->no_draw_only,
          message);
}

static void test_consumer_evidence_gate(void)
{
    Nexus_V1_Prs3Structure2AbiReceipt abi;
    Nexus_V1_Prs3Vdp1ConsumerEvidenceInput input;
    Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt receipt;
    Nexus_V1_Prs3Vdp1CaptureReceipt trace;
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt binding;

    memset(&abi, 0, sizeof(abi));
    if (!load_real_abi(&abi)) return;

    memset(&input, 0, sizeof(input));
    input.abi = &abi;
    input.prs3_header_span_fnv1a64 = abi.prs3_header_span_fnv1a64;
    input.prs3_bitmap_candidate_fnv1a64 = abi.prs3_bitmap_candidate_fnv1a64;
    input.prs3_bitmap_candidate_offset = abi.prs3_bitmap_candidate_offset;
    input.prs3_bitmap_candidate_size = abi.prs3_bitmap_candidate_size;
    input.palt_candidate_fnv1a64 = abi.palt_candidate_fnv1a64;
    input.palt_candidate_size = abi.palt_candidate_size;
    input.retail_media_available = 1;
    input.saturn_emulator_available = 1;

    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit(&input, &receipt) == 1,
          "local emulator preflight produces a fail-closed receipt");
    CHECK(receipt.status ==
              NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_EMULATOR &&
              strcmp(nexus_v1_prs3_vdp1_consumer_evidence_status_name(
                         receipt.status),
                     "blocked-emulator") == 0,
          "missing Saturn BIOS blocks independent VDP1 consumer evidence");
    CHECK(receipt.capture_target_bound &&
              receipt.prs3_entry_index == 5U &&
              receipt.prs3_stream_offset == 1612U &&
              receipt.prs3_stream_size == 560U &&
              receipt.prs3_expected_output_bytes == 1674U &&
              receipt.prs3_width == 54U &&
              receipt.prs3_height == 31U &&
              receipt.prs3_bpp == 1U &&
              receipt.prs3_output_fnv1a64 ==
                  UINT64_C(0x290a9d13c0224cc6) &&
              receipt.palt_entries_fnv1a64 ==
                  UINT64_C(0x0ec4e98ca3a18f85) &&
              receipt.palt_entries_are_be16 &&
              receipt.structure2_descriptor_count == 82 &&
              receipt.structure2_image_anchor_count == 82 &&
              receipt.structure2_palette_anchor_count == 80 &&
              receipt.structure2_palette_absent_count == 2 &&
              receipt.requires_independent_trace &&
              receipt.rejects_collector_manifest_only,
          "capture target carries exact PRS3/PALT/Structure2 facts");
    assert_no_submit(&receipt,
                     "BIOS/trace blocker cannot submit pixels or palette");

    input.saturn_bios_available = 1;
    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit(&input, &receipt) == 1 &&
              receipt.status == NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_TRACE,
          "missing raw trace blocks after emulator prerequisites");
    assert_no_submit(&receipt,
                     "missing raw trace cannot submit Structure2 to M11");

    input.raw_trace_artifact_available = 1;
    input.raw_trace_size = 4096U;
    input.raw_trace_fnv1a64 = UINT64_C(0x123456789abcdef0);
    input.raw_trace_authenticated = 1;
    input.collector_manifest_only = 1;
    input.trace_binds_dm_bin = 1;
    input.trace_binds_menu_bpk_entry5 = 1;
    input.trace_binds_lev00_structure2 = 1;
    input.vdp1_command_observed = 1;
    input.vdp1_texture_window_observed = 1;
    input.be16_palette_application_observed = 1;
    input.structure2_descriptor_selection_observed = 1;
    input.texture_placement_observed = 1;
    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit(&input, &receipt) == 1 &&
              receipt.status == NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_TRACE,
          "collector-only manifest is rejected even with claimed lanes");
    assert_no_submit(&receipt,
                     "unauthenticated collector manifest cannot open M11");

    input.collector_manifest_only = 0;
    input.be16_palette_application_observed = 0;
    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit(&input, &receipt) == 1 &&
              receipt.status ==
                  NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_CONSUMER_SEMANTICS,
          "missing BE16 palette observation blocks consumer semantics");
    assert_no_submit(&receipt,
                     "partial VDP1 trace cannot prove palette application");

    input.be16_palette_application_observed = 1;
    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit(&input, &receipt) == 1 &&
              receipt.status == NEXUS_V1_PRS3_VDP1_CONSUMER_READY_BLOCKED,
          "complete lane set is still capture evidence only in this module");
    assert_no_submit(&receipt,
                     "VDP1 consumer gate never fabricates Structure2 submit");

    input.prs3_bitmap_candidate_fnv1a64 = 0U;
    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit(&input, &receipt) == 1 &&
              receipt.status == NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_TRACE &&
              !receipt.candidate_spans_bound,
          "missing PRS3 bitmap candidate witness blocks the consumer trace");
    input.prs3_bitmap_candidate_fnv1a64 = UINT64_C(2);

    memset(&trace, 0, sizeof(trace));
    memset(&binding, 0, sizeof(binding));
    trace.valid = 1;
    trace.complete_capture = 1;
    trace.schema_version = 3U;
    trace.entry_index = abi.prs3_entry_index;
    trace.stream_offset = abi.prs3_stream_offset;
    trace.stream_size = abi.prs3_stream_size;
    trace.expected_output_bytes = abi.prs3_expected_output_bytes;
    binding.valid = 1;
    binding.menu_bpk_matches = 1;
    binding.dm_bin_matches = 1;
    binding.entry_plan_matches = 1;
    binding.payload_span_matches = 1;
    binding.exact_vdp1_handoff_observed = 1;
    binding.vdp1_texture_consumption_observed = 1;
    binding.vdp1_command_consumption_observed = 1;
    binding.palette_consumption_observed = 1;
    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit_capture_binding(
              &abi, &trace, &binding, UINT64_C(0x123456789abcdef0), 4096U,
              1, &receipt) == 1 &&
              receipt.status ==
                  NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_CONSUMER_SEMANTICS &&
              receipt.trace_binds_dm_bin &&
              receipt.trace_binds_menu_bpk_entry5 &&
              receipt.trace_binds_lev00_structure2 &&
              receipt.vdp1_command_observed &&
              receipt.vdp1_texture_window_observed &&
              receipt.be16_palette_application_observed &&
              !receipt.structure2_descriptor_selection_observed &&
              !receipt.texture_placement_observed,
          "source-bound PRS3 trace reaches only the Structure2 semantic blocker");
    assert_no_submit(&receipt,
                     "bound PRS3 trace cannot invent Structure2 placement");

    ++trace.stream_offset;
    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit_capture_binding(
              &abi, &trace, &binding, UINT64_C(0x123456789abcdef0), 4096U,
              1, &receipt) == 1 &&
              receipt.status == NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_TRACE,
          "PRS3 trace with a mismatched stream receipt is rejected");
}

static void test_capture_binding_contract(void)
{
    Nexus_V1_Prs3Structure2AbiReceipt abi;
    Nexus_V1_Prs3Vdp1CaptureReceipt trace;
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt binding;
    Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt receipt;

    memset(&abi, 0, sizeof(abi));
    abi.status = NEXUS_V1_PRS3_STRUCTURE2_ABI_READY_BLOCKED;
    abi.decoder_output_to_structure2_bound = 1;
    abi.positive_prs3_vector_bound = 1;
    abi.palt_trailer_bound = 1;
    abi.structure2_intake_bound = 1;
    abi.prs3_entry_index = 5U;
    abi.prs3_stream_offset = 1612U;
    abi.prs3_stream_size = 552U;
    abi.prs3_expected_output_bytes = 1674U;
    abi.prs3_width = 54U;
    abi.prs3_height = 31U;
    abi.prs3_bpp = 1U;
    abi.prs3_output_fnv1a64 = UINT64_C(0x14cacc01cee292aa);
    abi.prs3_header_span_fnv1a64 = UINT64_C(0x11);
    abi.prs3_bitmap_candidate_fnv1a64 = UINT64_C(0x22);
    abi.prs3_bitmap_candidate_offset = abi.prs3_stream_offset + 4U;
    abi.prs3_bitmap_candidate_size = abi.prs3_stream_size - 4U;
    abi.palt_entries_fnv1a64 = UINT64_C(0x0ec4e98ca3a18f85);
    abi.palt_entries_are_be16 = 1;
    abi.palt_candidate_fnv1a64 = abi.palt_entries_fnv1a64;
    abi.palt_candidate_size = 512U;
    abi.structure2_descriptor_count = 82;
    abi.structure2_image_anchor_count = 82;
    abi.structure2_palette_anchor_count = 80;
    abi.structure2_palette_absent_count = 2;
    memset(&trace, 0, sizeof(trace));
    trace.valid = 1;
    trace.complete_capture = 1;
    trace.schema_version = 3U;
    trace.entry_index = abi.prs3_entry_index;
    trace.stream_offset = abi.prs3_stream_offset;
    trace.stream_size = abi.prs3_stream_size;
    trace.expected_output_bytes = abi.prs3_expected_output_bytes;
    memset(&binding, 0, sizeof(binding));
    binding.valid = 1;
    binding.menu_bpk_matches = 1;
    binding.dm_bin_matches = 1;
    binding.entry_plan_matches = 1;
    binding.payload_span_matches = 1;
    binding.exact_vdp1_handoff_observed = 1;
    binding.vdp1_texture_consumption_observed = 1;
    binding.vdp1_command_consumption_observed = 1;
    binding.palette_consumption_observed = 1;

    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit_capture_binding(
              &abi, &trace, &binding, UINT64_C(0x123456789abcdef0), 4096U,
              1, &receipt) == 1 &&
              receipt.status ==
                  NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_CONSUMER_SEMANTICS,
          "bound trace contract reaches only the no-draw descriptor blocker");
    assert_no_submit(&receipt,
                     "bound trace contract keeps pixels and palettes blocked");
    binding.palette_consumption_observed = 0;
    CHECK(nexus_v1_prs3_vdp1_consumer_evidence_admit_capture_binding(
              &abi, &trace, &binding, UINT64_C(0x123456789abcdef0), 4096U,
              1, &receipt) == 1 &&
              receipt.status == NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_TRACE,
          "missing parsed palette lane rejects the trace before admission");
}

int main(void)
{
    test_consumer_evidence_gate();
    test_capture_binding_contract();
    if (failures) {
        fprintf(stderr, "Nexus PRS3/VDP1 consumer evidence: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus PRS3/VDP1 consumer evidence: PASS");
    return 0;
}
