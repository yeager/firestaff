#include "nexus_v1_prs3_sh2_subset_trace.h"

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

static void test_real_subset_trace(void)
{
    char dm_path[1024];
    char menu_path[1024];
    uint8_t *dm_bin = NULL;
    uint8_t *menu_bpk = NULL;
    size_t dm_bin_size = 0U;
    size_t menu_bpk_size = 0U;
    Nexus_V1_Prs3Sh2SubsetTraceInput input;
    Nexus_V1_Prs3Sh2SubsetTraceReceipt receipt;

    if (!nexus_path("DM.BIN", dm_path, sizeof(dm_path)) ||
        !nexus_path("MENU.BPK", menu_path, sizeof(menu_path)) ||
        !read_file(dm_path, &dm_bin, &dm_bin_size) ||
        !read_file(menu_path, &menu_bpk, &menu_bpk_size)) {
        puts("SKIP: local Nexus DM.BIN/MENU.BPK corpus not present");
        free(dm_bin);
        free(menu_bpk);
        return;
    }

    memset(&input, 0, sizeof(input));
    input.dm_bin = dm_bin;
    input.dm_bin_size = dm_bin_size;
    input.dm_bin_source_verified = 1;
    input.menu_bpk = menu_bpk;
    input.menu_bpk_size = menu_bpk_size;
    input.menu_bpk_source_verified = 1;
    input.entry_index = 1U;

    CHECK(nexus_v1_prs3_sh2_subset_trace_run(&input, &receipt) == 1,
          "real DM.BIN/MENU.BPK subset trace runs for entry 1");
    CHECK(receipt.status == NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_EXECUTION &&
              strcmp(nexus_v1_prs3_sh2_subset_trace_status_name(
                         receipt.status), "blocked-execution") == 0,
          "entry 1 remains a bounded 480-byte barrier");
    CHECK(receipt.control_flow_bound && receipt.menu_stream_bound &&
              receipt.entry_index == 1U &&
              receipt.stream_size == 144U &&
              receipt.body_size == 144U &&
              receipt.source_includes_frame_word &&
              receipt.linear_output_store_instructions_proven &&
              receipt.expected_output_bytes == 240U,
          "subset trace binds first real MENU.BPK PRS3 body");
    CHECK(receipt.input_cursor_register == 12U &&
              receipt.output_base_register == 13U &&
              receipt.output_index_register == 6U &&
              receipt.remaining_source_counter_register == 14U &&
              receipt.control_word_register == 11U &&
              receipt.output_window_mask_register == 5U,
          "subset trace exposes observed source/output/control registers");
    CHECK(receipt.input_read_bytes > 0U &&
              receipt.final_r12_offset == receipt.input_read_bytes &&
              receipt.final_r14_remaining + receipt.input_read_bytes ==
                  receipt.body_size &&
              receipt.executed_steps == 105U &&
              receipt.input_read_bytes == 144U &&
              receipt.nonzero_control_count == 79U &&
              receipt.zero_control_count == 26U &&
              receipt.refill_count == 14U &&
              receipt.output_store_count == 237U &&
              receipt.ring_store_count == 237U &&
              receipt.linear_output_store_count == 237U &&
              receipt.output_index_advance_count == 237U &&
              receipt.zero_merge_count == 26U &&
              receipt.zero_indexed_read_count == 159U,
          "subset trace observes both control paths and register movement");
    CHECK(receipt.first_nonzero_input_byte == 0xc2U &&
              receipt.first_nonzero_input_byte ==
              receipt.first_nonzero_output_byte,
          "nonzero path preserves the observed source byte at store PC");
    CHECK(receipt.first_zero_byte0 == 0x00U &&
              receipt.first_zero_byte1 == 0x00U &&
              receipt.first_zero_merged_value == 0x0000U &&
              receipt.first_zero_merged_value ==
              (receipt.first_zero_byte0 |
               ((receipt.first_zero_byte1 << 4U) & 0x0f00U)),
          "zero-side dynamic merge matches the retail SH-2 algebra");
    CHECK(receipt.final_r6_index == 237U &&
              receipt.final_r10_output_offset == 237U &&
              receipt.final_r14_remaining == 0U &&
              receipt.output_prefix_fnv1a64 ==
                  UINT64_C(0x54ea460f9e362b35) &&
              receipt.control_trace_fnv1a64 ==
                  UINT64_C(0xfa394a4231f8e528),
          "subset trace output/control fingerprints match real corpus");
    CHECK(!receipt.full_expected_output_observed &&
              !receipt.positive_output_vector_bound &&
              !receipt.zero_side_copy_semantics_proven &&
              !receipt.original_saturn_execution_authenticated &&
              !receipt.opcode_grammar_proven &&
              !receipt.decoder_promoted &&
              !receipt.fallback_visuals_permitted,
          "entry 1 does not promote decoder without full output vector");

    input.entry_index = 5U;
    CHECK(nexus_v1_prs3_sh2_subset_trace_run(&input, &receipt) == 1,
          "real DM.BIN/MENU.BPK subset trace runs for entry 5");
    CHECK(receipt.status == NEXUS_V1_PRS3_SH2_SUBSET_READY_BLOCKED &&
              receipt.entry_index == 5U &&
              receipt.stream_size == 560U &&
              receipt.body_size == 560U &&
              receipt.expected_output_bytes == 1674U &&
              receipt.source_includes_frame_word &&
              receipt.linear_output_store_instructions_proven,
          "entry 5 binds a full real stream/output vector");
    CHECK(receipt.executed_steps == 329U &&
              receipt.input_read_bytes == 529U &&
              receipt.final_r14_remaining == 31U &&
              receipt.output_store_count == 1674U &&
              receipt.ring_store_count == 1674U &&
              receipt.linear_output_store_count == 1674U &&
              receipt.output_index_advance_count == 1674U &&
              receipt.nonzero_control_count == 171U &&
              receipt.zero_control_count == 158U &&
              receipt.refill_count == 42U &&
              receipt.zero_merge_count == 158U &&
              receipt.zero_indexed_read_count == 1503U,
          "entry 5 observes full source/output/control movement");
    CHECK(receipt.first_nonzero_input_byte == 0xa6U &&
              receipt.first_nonzero_output_byte == 0xa6U &&
              receipt.first_zero_byte0 == 0x00U &&
              receipt.first_zero_byte1 == 0x02U &&
              receipt.first_zero_merged_value == 0x0000U,
          "entry 5 first dynamic transfer and merge are retained");
    CHECK(receipt.final_r6_index == 1674U &&
              receipt.final_r10_output_offset == 1674U &&
              receipt.output_prefix_fnv1a64 ==
                  UINT64_C(0x290a9d13c0224cc6) &&
              receipt.control_trace_fnv1a64 ==
                  UINT64_C(0xf305b1060657bb06),
          "entry 5 full output vector fingerprint is stable");
    CHECK(receipt.full_expected_output_observed &&
              receipt.positive_output_vector_bound &&
              receipt.zero_side_copy_semantics_proven &&
              !receipt.original_saturn_execution_authenticated &&
              !receipt.opcode_grammar_proven &&
              !receipt.decoder_promoted &&
              !receipt.fallback_visuals_permitted,
          "full subset vector still does not promote runtime decoder");

    input.dm_bin_source_verified = 0;
    CHECK(nexus_v1_prs3_sh2_subset_trace_run(&input, &receipt) == 0 &&
              receipt.status == NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_INPUT,
          "unverified DM.BIN blocks subset trace");

    free(dm_bin);
    free(menu_bpk);
}

int main(void)
{
    test_real_subset_trace();
    if (failures) {
        fprintf(stderr, "Nexus PRS3 SH-2 subset trace: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus PRS3 SH-2 subset trace: PASS");
    return 0;
}
