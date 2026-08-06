#include "nexus_v1_prs3_loader_control_flow.h"

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

static void test_real_loader_control_flow(void)
{
    char dm_path[1024];
    char menu_path[1024];
    uint8_t *dm_bin = NULL;
    uint8_t *menu_bpk = NULL;
    size_t dm_bin_size = 0U;
    size_t menu_bpk_size = 0U;
    Nexus_V1_Prs3LoaderControlInput input;
    Nexus_V1_Prs3LoaderControlReceipt receipt;

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

    CHECK(nexus_v1_prs3_loader_control_flow_probe(&input, &receipt) == 1,
          "retail PRS3 loader control flow probe binds");
    CHECK(receipt.status == NEXUS_V1_PRS3_LOADER_CONTROL_READY_BLOCKED &&
              strcmp(nexus_v1_prs3_loader_control_status_name(receipt.status),
                     "ready-blocked") == 0,
          "control-flow probe remains ready-blocked without vectors");
    CHECK(receipt.executable_control_flow_bound &&
              receipt.callee_offset == 85376U &&
              receipt.control_reentry_offset == 85428U &&
              receipt.control_low_bit_test_offset == 85450U &&
              receipt.control_zero_branch_offset == 85452U &&
              receipt.control_zero_branch_target_offset == 85476U,
          "control re-entry and low-bit branch offsets are exact");
    CHECK(receipt.nonzero_counter_decrement_offset == 85458U &&
              receipt.nonzero_input_read_offset == 85460U &&
              receipt.nonzero_output_store_offset == 85464U &&
              receipt.nonzero_post_store_increment_offset == 85466U &&
              receipt.nonzero_reentry_branch_offset == 85472U &&
              receipt.nonzero_path_is_direct_source_to_output,
          "nonzero path is the proven direct source-to-output corridor");
    CHECK(receipt.zero_counter_decrement_offset == 85476U &&
              receipt.zero_first_input_read_offset == 85484U &&
              receipt.zero_second_input_read_offset == 85488U &&
              receipt.zero_merge_or_offset == 85500U &&
              receipt.zero_indexed_byte_read_offset == 85524U &&
              receipt.zero_repeat_branch_offset == 85534U &&
              receipt.zero_outer_loop_branch_offset == 85538U &&
              receipt.zero_side_two_source_bytes_merge &&
              receipt.zero_side_indexed_output_window_read &&
              receipt.zero_side_direct_output_store_absent &&
              !receipt.zero_side_copy_semantics_proven,
          "zero-side path is exact but copy semantics remain unproven");
    CHECK(receipt.input_cursor_register == 12U &&
              receipt.remaining_source_counter_register == 14U &&
              receipt.control_word_register == 11U &&
              receipt.control_mask_register == 3U &&
              receipt.output_base_register == 13U &&
              receipt.output_index_register == 6U &&
              receipt.output_effective_index_register == 0U &&
              receipt.output_window_mask_register == 5U &&
              receipt.zero_compare_value_register == 1U &&
              receipt.zero_repeat_counter_register == 10U,
          "source/output pointer and control registers are retained");
    CHECK(receipt.control_sentinel_word == 0x0100U &&
              receipt.zero_upper_mask_word == 0x0f00U &&
              receipt.zero_index_mask_word == 0x0fffU &&
              receipt.zero_side_linear_fnv1a64 == UINT64_C(0xe0cc325e85a0e63f),
          "literal masks and zero-side corridor fingerprint match retail bytes");
    CHECK(receipt.menu_streams_bound &&
              receipt.menu_prs3_stream_count == 162U &&
              receipt.first_menu_entry_index == 1U &&
              receipt.first_menu_stream_size == 144U &&
              receipt.first_menu_expected_output_bytes == 240U,
          "MENU.BPK stream plan is bound to the control-flow probe");
    CHECK(!receipt.expected_output_vector_bound &&
              !receipt.opcode_grammar_reviewed &&
              !receipt.original_saturn_execution_authenticated &&
              !receipt.decoder_implementation_permitted &&
              !receipt.decoder_promoted &&
              receipt.decoded_pixels_emitted == 0U &&
              !receipt.fallback_visuals_permitted,
          "no positive output vector means no decoder implementation");

    input.dm_bin_source_verified = 0;
    CHECK(nexus_v1_prs3_loader_control_flow_probe(&input, &receipt) == 0 &&
              receipt.status == NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_INPUT,
          "unverified DM.BIN blocks control-flow probe");
    input.dm_bin_source_verified = 1;
    input.menu_bpk_source_verified = 0;
    CHECK(nexus_v1_prs3_loader_control_flow_probe(&input, &receipt) == 0 &&
              receipt.status == NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_INPUT,
          "unverified MENU.BPK blocks stream binding");

    free(dm_bin);
    free(menu_bpk);
}

int main(void)
{
    test_real_loader_control_flow();
    if (failures) {
        fprintf(stderr, "Nexus PRS3 loader control flow: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus PRS3 loader control flow: PASS");
    return 0;
}
