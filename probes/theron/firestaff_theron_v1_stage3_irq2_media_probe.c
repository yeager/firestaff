#include "theron_v1_stage3_irq2_dispatch.h"
#include "theron_v1_stage2_runtime_handoff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The raw Track 02 parser also contains unrelated level-route entry points.
 * This media-only probe exercises only its IPL loader scanner. */
Theron_MapLoadResult theron_v1_level_load(Theron_V1_Level *level,
                                          const uint8_t *data,
                                          int data_size,
                                          int dungeon_id,
                                          int sub_level_index) {
    (void)level;
    (void)data;
    (void)data_size;
    (void)dungeon_id;
    (void)sub_level_index;
    return THERON_MAP_ERR_NULL;
}

void theron_v1_world_runtime_media_invalidate_cache(Theron_V1_World *world) {
    (void)world;
}

static int g_fail;

static void check(int condition, const char *name) {
    if (!condition) {
        ++g_fail;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static uint8_t *read_file_bytes(const char *path, size_t *out_size) {
    FILE *file = NULL;
    long size;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

int main(int argc, char **argv) {
    uint8_t *jp_bytes;
    uint8_t *us_bytes;
    size_t jp_size;
    size_t us_size;
    Theron_V1Stage3Irq2DispatchReceipt jp;
    Theron_V1Stage3Irq2DispatchReceipt us;
    Theron_Track02IplLoaderReceipt jp_loader;
    Theron_Track02IplLoaderReceipt us_loader;
    Theron_Track02Stage2DynamicPayloadReceipt jp_payload;
    Theron_Track02Stage2DynamicPayloadReceipt us_payload;
    Theron_V1Stage2RuntimeHandoff jp_handoff;
    Theron_V1Stage2RuntimeHandoff us_handoff;
    Theron_V1Stage2RuntimeHandoff jp_original_handoff;
    Theron_V1Stage2RuntimeHandoff us_original_handoff;

    if (argc != 3) {
        printf("[FAIL] expected JP and US raw Track02 paths\n");
        return 1;
    }
    jp_bytes = read_file_bytes(argv[1], &jp_size);
    us_bytes = read_file_bytes(argv[2], &us_size);
    check(jp_bytes && us_bytes, "raw JP/US Track02 files read");
    memset(&jp_payload, 0, sizeof(jp_payload));
    memset(&us_payload, 0, sizeof(us_payload));
    check(jp_bytes && theron_v1_track02_find_ipl_loader(
              jp_bytes, jp_size, "b7afb338ad31be1025b53f9aff12d73a",
              &jp_loader) == THERON_TRACK02_SIGNAL_OK &&
              jp_loader.stage2_record == 0x0003e7u &&
              jp_loader.stage2_sector_count == 17u &&
              jp_loader.stage2_load_address == 0x4000u &&
              jp_loader.stage2_entry_address == 0x4000u &&
              jp_loader.stage2_cd_read_cpu_address == 0x4090u &&
              jp_loader.stage2_cd_read_local_destination == 0x3800u &&
              jp_loader.stage2_cd_read_record == 0x0004dfu &&
              jp_loader.cd_read_cpu_address == 0x40cdu &&
              jp_loader.cd_read_local_destination == 0x3000u &&
              jp_loader.stage2_cd_read_dynamic_boundary_valid,
          "JP original IPL chain reaches the proven stage-three record");
    check(us_bytes && theron_v1_track02_find_ipl_loader(
              us_bytes, us_size, "f23601102138f87c33025877767ebf76",
              &us_loader) == THERON_TRACK02_SIGNAL_OK &&
              us_loader.stage2_record == 0x0003e7u &&
              us_loader.stage2_sector_count == 17u &&
              us_loader.stage2_load_address == 0x4000u &&
              us_loader.stage2_entry_address == 0x4000u &&
              us_loader.stage2_cd_read_cpu_address == 0x4090u &&
              us_loader.stage2_cd_read_local_destination == 0x3800u &&
              us_loader.stage2_cd_read_record == 0x0004e0u &&
              us_loader.cd_read_cpu_address == 0x40cdu &&
              us_loader.cd_read_local_destination == 0x3000u &&
              us_loader.stage2_cd_read_dynamic_boundary_valid,
          "US original IPL chain reaches the proven stage-three record");
    check(jp_bytes && theron_v1_track02_inspect_stage2_dynamic_payload(
              jp_bytes, jp_size, "b7afb338ad31be1025b53f9aff12d73a",
              &jp_payload) == THERON_TRACK02_SIGNAL_OK &&
              us_bytes && theron_v1_track02_inspect_stage2_dynamic_payload(
              us_bytes, us_size, "f23601102138f87c33025877767ebf76",
              &us_payload) == THERON_TRACK02_SIGNAL_OK,
          "JP/US stage-three payloads are read from original media");
    check(theron_v1_stage2_runtime_handoff_from_dynamic_payload(
              &jp_payload, &jp_handoff) &&
              theron_v1_stage2_runtime_handoff_from_dynamic_payload(
              &us_payload, &us_handoff) &&
              jp_handoff.cleared_work_ram_start == 0x2700u &&
              jp_handoff.cleared_work_ram_bytes == 0x1100u &&
              jp_handoff.cleared_work_ram_end == 0x3800u &&
              jp_handoff.work_ram_cleared_before_entry &&
              us_handoff.work_ram_cleared_before_entry,
          "stage-two transfer clears the proven work-RAM interval before entry");
    check(theron_v1_stage2_runtime_handoff_from_original_media(
              jp_bytes, jp_size, "b7afb338ad31be1025b53f9aff12d73a",
              &jp_original_handoff) &&
              theron_v1_stage2_runtime_handoff_from_original_media(
              us_bytes, us_size, "f23601102138f87c33025877767ebf76",
              &us_original_handoff) &&
              jp_original_handoff.physical_stage3_entry_verified &&
              jp_original_handoff.stage3_entry_opcode == 0x00u &&
              jp_original_handoff.stage3_irq2_selector == 0xffu &&
              jp_original_handoff.stage3_continuation_address == 0x3802u &&
              jp_original_handoff.ipl_preload_local_read_verified &&
              jp_original_handoff.ipl_preload_cpu_address == 0x40cdu &&
              jp_original_handoff.ipl_preload_destination == 0x3000u &&
              jp_original_handoff.ipl_preload_record_proven &&
              jp_original_handoff.ipl_preload_record == 0x0003e3u &&
              jp_original_handoff.ipl_preload_sector_count == 2u &&
              jp_original_handoff.ipl_preload_raw_sector == 0x4c3u &&
              jp_original_handoff.ipl_preload_returns_to_ipl_proven &&
              jp_original_handoff.ipl_preload_user_data_bytes == 4096u &&
              jp_original_handoff.ipl_preload_first_nonzero_offset == 243u &&
              jp_original_handoff.ipl_preload_nonzero_byte_count == 2911u &&
              jp_original_handoff.ipl_preload_user_data_hash != 0u &&
              jp_original_handoff.stage2_cd_exec_table_verified &&
              jp_original_handoff.stage2_cd_read_setup_verified &&
              jp_original_handoff.stage2_post_read_transfer_verified &&
              jp_original_handoff.stage3_mode1_header_verified &&
              jp_original_handoff.stage3_minute_bcd == 0x01u &&
              jp_original_handoff.stage3_second_bcd == 0x03u &&
              jp_original_handoff.stage3_frame_bcd == 0x38u &&
              us_original_handoff.physical_stage3_entry_verified &&
              us_original_handoff.ipl_preload_local_read_verified &&
              us_original_handoff.ipl_preload_record_proven &&
              us_original_handoff.ipl_preload_record == 0x0003e3u &&
              us_original_handoff.ipl_preload_sector_count == 2u &&
              us_original_handoff.ipl_preload_raw_sector == 0x4c4u &&
              us_original_handoff.ipl_preload_returns_to_ipl_proven &&
              us_original_handoff.ipl_preload_user_data_bytes == 4096u &&
              us_original_handoff.ipl_preload_first_nonzero_offset == 243u &&
              us_original_handoff.ipl_preload_nonzero_byte_count == 2911u &&
              us_original_handoff.ipl_preload_user_data_hash ==
                  jp_original_handoff.ipl_preload_user_data_hash &&
              us_original_handoff.stage2_cd_exec_table_verified &&
              us_original_handoff.stage2_cd_read_setup_verified &&
              us_original_handoff.stage2_post_read_transfer_verified &&
              us_original_handoff.stage3_mode1_header_verified &&
              us_original_handoff.stage3_minute_bcd == 0x00u &&
              us_original_handoff.stage3_second_bcd == 0x58u &&
              us_original_handoff.stage3_frame_bcd == 0x57u,
          "single original-media handoff binds IPL through stage-three entry");
    check(jp_bytes && theron_v1_stage3_irq2_dispatch_from_original_media(
              jp_bytes, jp_size, &jp_payload, &jp),
          "JP stage-three bytes authenticate BRK $ff IRQ2 entry");
    check(us_bytes && theron_v1_stage3_irq2_dispatch_from_original_media(
              us_bytes, us_size, &us_payload, &us),
          "US stage-three bytes authenticate BRK $ff IRQ2 entry");
    check(jp.valid && us.valid && jp.entry_address == 0x3800u &&
              jp.opcode == 0x00u && jp.irq2_selector == 0xffu &&
              jp.continuation_address == 0x3802u &&
              jp.manifest_not_linear_cpu_code && us.manifest_not_linear_cpu_code,
          "runtime receipt stays bounded at the proven IRQ2 transfer");
    jp_bytes[(size_t)0x4dfu * 2352u + 17u] = 0u;
    check(!theron_v1_stage3_irq2_dispatch_from_original_media(
              jp_bytes, jp_size, &jp_payload, &jp),
          "altered selector cannot reach the runtime dispatch receipt");
    free(jp_bytes);
    free(us_bytes);
    printf("--- %d failed ---\n", g_fail);
    return g_fail ? 1 : 0;
}
