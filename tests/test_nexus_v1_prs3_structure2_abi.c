#include "nexus_v1_prs3_structure2_abi.h"

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

static void test_real_prs3_structure2_abi_gate(void)
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
    Nexus_V1_Prs3Structure2AbiReceipt receipt;

    if (!nexus_path("DM.BIN", dm_path, sizeof(dm_path)) ||
        !nexus_path("MENU.BPK", menu_path, sizeof(menu_path)) ||
        !nexus_path("LEV00.DGN", lev_path, sizeof(lev_path)) ||
        !read_file(dm_path, &dm_bin, &dm_bin_size) ||
        !read_file(menu_path, &menu_bpk, &menu_bpk_size) ||
        !read_file(lev_path, &lev00, &lev00_size)) {
        puts("SKIP: local Nexus DM.BIN/MENU.BPK/LEV00.DGN corpus not present");
        free(dm_bin);
        free(menu_bpk);
        free(lev00);
        return;
    }

    memset(&level, 0, sizeof(level));
    CHECK(nexus_v1_level_load(&level, lev00, (int)lev00_size, 0) == 0,
          "real LEV00.DGN loads Structure2 descriptors");

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

    CHECK(nexus_v1_prs3_structure2_abi_admit(&input, &receipt) == 0,
          "entry 5 PRS3 vector blocked at Structure2 stage");
    CHECK(receipt.status == NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_STRUCTURE2 &&
              strcmp(nexus_v1_prs3_structure2_abi_status_name(receipt.status),
                     "blocked-structure2") == 0,
          "ABI gate blocked at Structure2 because intake sees decoded PRS3");
    CHECK(receipt.positive_prs3_vector_bound &&
              receipt.prs3_entry_index == 5U &&
              receipt.prs3_stream_offset == 1612U &&
              receipt.prs3_stream_size == 552U &&
              receipt.prs3_expected_output_bytes == 1674U &&
              receipt.prs3_width == 54U &&
              receipt.prs3_height == 31U &&
              receipt.prs3_bpp == 1U &&
              receipt.prs3_output_fnv1a64 ==
                  UINT64_C(0x14cacc01cee292aa) &&
              receipt.prs3_header_span_fnv1a64 != 0U &&
              receipt.prs3_bitmap_candidate_fnv1a64 != 0U &&
              receipt.prs3_bitmap_candidate_offset == 1616U &&
              receipt.prs3_bitmap_candidate_size == 548U &&
              receipt.prs3_input_read_bytes == 514U &&
              receipt.prs3_output_store_count == 1674U &&
              receipt.prs3_zero_merge_count == 164U &&
              receipt.prs3_zero_copy_count == 1527U,
          "entry 5 positive vector facts are retained");
    CHECK(receipt.palt_trailer_bound &&
              receipt.palt_entries_are_be16 &&
              receipt.palt_entries_fnv1a64 ==
                  UINT64_C(0x0ec4e98ca3a18f85) &&
              receipt.palt_candidate_fnv1a64 ==
                  receipt.palt_entries_fnv1a64 &&
              receipt.palt_candidate_size == 512U,
          "real PALT trailer is bound as raw source table only");
    CHECK(!receipt.structure2_intake_bound,
          "Structure2 intake not bound after decoded PRS3 blocks intake");
    CHECK(!receipt.decoder_output_to_structure2_bound &&
              !receipt.can_submit_structure2_pixels &&
              !receipt.can_submit_palette &&
              !receipt.runtime_m11_handoff_permitted &&
              !receipt.fallback_visuals_permitted &&
              receipt.no_draw_only,
          "Structure2 block prevents all downstream promotion");

    input.independent_saturn_trace_bound = 1;
    input.vdp1_consumer_semantics_proven = 1;
    input.pixel_format_proven = 1;
    input.palette_application_proven = 1;
    input.structure2_placement_proven = 1;
    CHECK(nexus_v1_prs3_structure2_abi_admit(&input, &receipt) == 0 &&
              receipt.status ==
                  NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_STRUCTURE2 &&
              !receipt.can_submit_structure2_pixels &&
              !receipt.can_submit_palette &&
              !receipt.runtime_m11_handoff_permitted &&
              !receipt.fallback_visuals_permitted,
          "auth booleans cannot overcome Structure2 block from decoded PRS3");

    input.independent_saturn_trace_bound = 0;
    input.vdp1_consumer_semantics_proven = 0;
    input.pixel_format_proven = 0;
    input.palette_application_proven = 0;
    input.structure2_placement_proven = 0;
    input.prs3_entry_index = 1U;
    CHECK(nexus_v1_prs3_structure2_abi_admit(&input, &receipt) == 0 &&
              receipt.status == NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_VECTOR,
          "entry 1 cannot enter Structure2 ABI without full vector");

    input.prs3_entry_index = 5U;
    input.menu_bpk_source_verified = 0;
    CHECK(nexus_v1_prs3_structure2_abi_admit(&input, &receipt) == 0 &&
              receipt.status == NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_INPUT,
          "unverified MENU.BPK source blocks ABI gate");

    free(dm_bin);
    free(menu_bpk);
    free(lev00);
}

int main(void)
{
    test_real_prs3_structure2_abi_gate();
    if (failures) {
        fprintf(stderr, "Nexus PRS3/Structure2 ABI: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus PRS3/Structure2 ABI: PASS");
    return 0;
}
