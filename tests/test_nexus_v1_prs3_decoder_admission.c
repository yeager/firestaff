#include "nexus_v1_prs3_decoder_admission.h"

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

static int nexus_data_path(const char *name, char *out, size_t out_size)
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

static void test_real_retail_decoder_admission_blocks_without_proof(void)
{
    char menu_path[1024];
    char dm_bin_path[1024];
    uint8_t *menu = NULL;
    uint8_t *dm_bin = NULL;
    size_t menu_size = 0U;
    size_t dm_bin_size = 0U;
    Nexus_V1_Prs3DecoderAdmissionInput input;
    Nexus_V1_Prs3DecoderAdmissionReceipt receipt;

    if (!nexus_data_path("MENU.BPK", menu_path, sizeof(menu_path)) ||
        !nexus_data_path("DM.BIN", dm_bin_path, sizeof(dm_bin_path)) ||
        !read_file(menu_path, &menu, &menu_size) ||
        !read_file(dm_bin_path, &dm_bin, &dm_bin_size)) {
        puts("SKIP: local Nexus MENU.BPK/DM.BIN corpus not present");
        free(menu);
        free(dm_bin);
        return;
    }

    memset(&input, 0, sizeof(input));
    input.menu_bpk = menu;
    input.menu_bpk_size = menu_size;
    input.menu_bpk_source_verified = 1;
    input.dm_bin = dm_bin;
    input.dm_bin_size = dm_bin_size;
    input.dm_bin_source_verified = 1;

    CHECK(nexus_v1_prs3_decoder_admission_evaluate(&input, &receipt) == 0,
          "retail MENU.BPK/DM.BIN blocked at differential trial stage");
    CHECK(receipt.status ==
              NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_DIFFERENTIAL &&
              strcmp(nexus_v1_prs3_decoder_admission_status_name(receipt.status),
                     "blocked-differential") == 0,
          "decoder admission blocked-differential with MSB trailing matches");
    CHECK(receipt.dm_bin_v1_loader_bound &&
              receipt.dm_bin_v1_callee_offset == 85376U &&
              receipt.dm_bin_control_test_offset == 85450U &&
              receipt.dm_bin_nonzero_read_offset == 85460U &&
              receipt.dm_bin_output_store_offset == 85464U &&
              receipt.nonzero_direct_byte_path_proven &&
              receipt.zero_side_two_byte_merge_proven &&
              receipt.zero_side_has_no_direct_output_store,
          "retail DM.BIN PRS3 loader operations are bound");
    CHECK(receipt.menu_bpk_v1_streams_bound &&
              receipt.menu_bpk_entry_count == 163U &&
              receipt.menu_bpk_prs3_stream_count == 162U &&
              receipt.first_stream_entry_index == 1U &&
              receipt.first_stream_size > 0U &&
              receipt.first_expected_output_bytes == 240U,
          "retail MENU.BPK PRS3 streams are bound");
    CHECK(receipt.lsb_trial_evaluated > 0U &&
              receipt.msb_trial_evaluated > 0U &&
              !receipt.simple_lsb_msb_decoder_disproven &&
              receipt.lsb_trial_complete_exact == 0U &&
              receipt.lsb_trial_complete_trailing == 0U &&
              receipt.msb_trial_complete_exact == 1U &&
              receipt.msb_trial_complete_trailing == 108U &&
              receipt.lsb_trial_failures > 0U &&
              receipt.msb_trial_failures > 0U,
          "MSB trial has one exact frame and 108 trailing matches; simple decoder remains unproven");
    CHECK(!receipt.expected_output_bound &&
              !receipt.decoder_ready &&
              !receipt.decoder_promoted &&
              receipt.decoded_pixels_emitted == 0U &&
              !receipt.runtime_upload_permitted &&
              !receipt.structure2_pixel_intake_permitted &&
              !receipt.fallback_visuals_permitted,
          "differential block prevents output proof and decoder stage");

    input.menu_bpk_source_verified = 0;
    CHECK(nexus_v1_prs3_decoder_admission_evaluate(&input, &receipt) == 0 &&
              receipt.status == NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_SOURCE,
          "unverified MENU.BPK source rejects decoder admission");
    input.menu_bpk_source_verified = 1;
    input.dm_bin_source_verified = 0;
    CHECK(nexus_v1_prs3_decoder_admission_evaluate(&input, &receipt) == 0 &&
              receipt.status == NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_SOURCE,
          "unverified DM.BIN source rejects decoder admission");

    free(menu);
    free(dm_bin);
}

int main(void)
{
    test_real_retail_decoder_admission_blocks_without_proof();
    if (failures) {
        fprintf(stderr, "Nexus PRS3 decoder admission: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus PRS3 decoder admission: PASS");
    return 0;
}
