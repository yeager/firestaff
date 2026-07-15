/* Emit source-bound capture requests for the retail MENU.BPK PRS3 streams.
 * This is a capture-planning tool only: it neither decodes nor renders PRS3. */
#include "asset_find_by_hash.h"
#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_prs3_capture_trace_schema.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RETAIL_MENU_BPK_MD5 "c2776768ff25287c79013a1452253ca0"
#define RETAIL_DM_BIN_MD5 "e88d60859f65f08fa622e1992b02280f"
#define PRS3_CAPTURE_TARGET_MAGIC "FIRESTAFF_NEXUS_PRS3_CAPTURE_TARGET_V1"

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long file_size;
    uint8_t *data;

    if (out_size) *out_size = 0U;
    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (file_size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)file_size);
    if (!data || fread(data, 1U, (size_t)file_size, file) != (size_t)file_size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)file_size;
    return data;
}

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;

    if (!data || size == 0U) return 0U;
    for (i = 0U; i < size; ++i) {
        hash ^= data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int main(int argc, char **argv)
{
    char menu_path[ASSET_PATH_MAX];
    char dm_path[ASSET_PATH_MAX];
    Nexus_V1_BpkArchiveInfo archive;
    Nexus_V1_Prs3CrossAssetFrameReceipt frames;
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2;
    uint8_t *menu = NULL;
    uint8_t *dm_bin = NULL;
    size_t menu_size = 0U;
    size_t dm_size = 0U;
    uint64_t menu_fnv;
    uint64_t dm_fnv;
    uint32_t index;
    int written = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <nexus-data-dir> <output-directory>\n", argv[0]);
        return 2;
    }
    if (!asset_find_by_md5(argv[1], RETAIL_MENU_BPK_MD5, menu_path,
                           sizeof(menu_path), 4) ||
        !asset_find_by_md5(argv[1], RETAIL_DM_BIN_MD5, dm_path,
                           sizeof(dm_path), 4) ||
        strstr(menu_path, "::") || strstr(dm_path, "::")) {
        fprintf(stderr, "could not find extracted canonical MENU.BPK and DM.BIN\n");
        return 1;
    }
    menu = read_file(menu_path, &menu_size);
    dm_bin = read_file(dm_path, &dm_size);
    if (!menu || !dm_bin ||
        nexus_v1_bpk_archive_parse(menu, menu_size, &archive) != 0 ||
        nexus_v1_prs3_cross_asset_frame_receipt_verified(
            dm_bin, dm_size, 1, menu, menu_size, 1, &frames) != 1 ||
        !frames.outer_v1_framing_matches || frames.decoder_promoted ||
        frames.menu_handoff_authorized ||
        nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
            dm_bin, dm_size, 1, &sh2) != 1 || !sh2.sh2_control_path_verified ||
        !sh2.sh2_stream_read_verified || !sh2.sh2_output_store_verified ||
        sh2.decoder_promoted) {
        fprintf(stderr, "retail PRS3 source framing is incomplete\n");
        free(menu);
        free(dm_bin);
        return 1;
    }
    menu_fnv = fnv1a64(menu, menu_size);
    dm_fnv = fnv1a64(dm_bin, dm_size);
    for (index = 0U; index < archive.candidate_offset_count; ++index) {
        Nexus_V1_BpkPrs3StreamPlan plan;
        char path[1024];
        FILE *file;

        if (nexus_v1_bpk_archive_prs3_stream_plan(menu, menu_size, index,
                                                   &plan) !=
            NEXUS_V1_BPK_PRS3_STREAM_OK) {
            continue;
        }
        if (plan.decode_blocked == 0 || !plan.header_first_readable ||
            plan.stream_size == 0U || plan.expected_output_bytes == 0U ||
            snprintf(path, sizeof(path), "%s/MENU-%03u.prs3.target", argv[2],
                     index) >= (int)sizeof(path) || !(file = fopen(path, "wb"))) {
            fprintf(stderr, "could not create PRS3 target %u\n", index);
            free(menu);
            free(dm_bin);
            return 1;
        }
        if (fprintf(file,
                    PRS3_CAPTURE_TARGET_MAGIC "\n"
                    "canonical_menu_bpk_md5=%s\ncanonical_dm_bin_md5=%s\n"
                    "menu_bpk_fnv1a64=%016llx\ndm_bin_fnv1a64=%016llx\n"
                    "entry_index=%x\nstream_offset=%x\nstream_size=%x\n"
                    "expected_output_bytes=%x\nmode=%x\nwidth=%x\nheight=%x\n"
                    "capture_kind=original_saturn_sh2_prs3\n"
                    "required_observations=input_reads,output_writes,vdp1_command,palette_state\n"
                    "dm_bin_v1_sh2_route_verified=1\n"
                    "decoder_promoted=0\nmenu_handoff_authorized=0\n"
                    "no_decode_or_render=1\n",
                    RETAIL_MENU_BPK_MD5, RETAIL_DM_BIN_MD5,
                    (unsigned long long)menu_fnv, (unsigned long long)dm_fnv,
                    index, plan.stream_offset, plan.stream_size,
                    plan.expected_output_bytes, plan.mode, plan.width,
                    plan.height) < 0) {
            fclose(file);
            remove(path);
            fprintf(stderr, "could not write PRS3 target %u\n", index);
            free(menu);
            free(dm_bin);
            return 1;
        }
        if (fclose(file) != 0) {
            remove(path);
            fprintf(stderr, "could not close PRS3 target %u\n", index);
            free(menu);
            free(dm_bin);
            return 1;
        }
        ++written;
    }
    free(menu);
    free(dm_bin);
    if (written != (int)archive.prs3_payload_count) {
        fprintf(stderr, "PRS3 target count does not match archive\n");
        return 1;
    }
    printf("wrote %d no-decode PRS3 capture targets\n", written);
    return 0;
}
