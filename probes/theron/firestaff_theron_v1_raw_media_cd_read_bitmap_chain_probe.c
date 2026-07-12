/*
 * Opt-in, read-only Track 02 CD-read chain probe.
 *
 * Inputs are explicit only:
 *   THERON_RAW_TRACK02=/absolute/path/to/authentic-track02.bin
 *   THERON_SYSTEM_CARD=/absolute/path/to/syscard3.pce
 *
 * It validates the documented IPL -> $4090 local-RAM CD_READ boundary, then
 * publishes only bitmap-route masks and checksums from the existing raw
 * bitmap receipt.  No filesystem scan, emulator, framebuffer, or fallback
 * executor is involved.
 */
#include "asset_status_m12.h"
#include "theron_v1_startup_media.h"
#include "theron_v1_system_card_irq2_entry_gate.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THERON_CHAIN_SYSCARD3_MD5 "ff1a674273fe3540ccef576376407d1d"
#define THERON_CHAIN_SYSCARD3_BYTES 0x40200u

static int read_file(const char *path, unsigned char **out_data,
                     size_t *out_size)
{
    FILE *file;
    long length;
    unsigned char *data;

    if (!path || !path[0] || !out_data || !out_size) {
        return 0;
    }
    *out_data = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) {
            fclose(file);
        }
        return 0;
    }
    data = (unsigned char *)malloc((size_t)length);
    if (!data || fread(data, 1u, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)length;
    return 1;
}

static int raw_track02_md5(const char *path, size_t bytes, char md5[33])
{
    return path && bytes > 0u &&
           bytes % THERON_TRACK02_RAW_SECTOR_BYTES == 0u &&
           m12_file_md5_hex(path, md5) &&
           (strcmp(md5, THERON_TRACK02_MD5_JP_BIN) == 0 ||
            strcmp(md5, THERON_TRACK02_MD5_US_BIN) == 0);
}

static int system_card_md5(const char *path, size_t bytes, char md5[33])
{
    return path && bytes == THERON_CHAIN_SYSCARD3_BYTES &&
           m12_file_md5_hex(path, md5) &&
           strcmp(md5, THERON_CHAIN_SYSCARD3_MD5) == 0;
}

int main(void)
{
    const char *track02_path = getenv("THERON_RAW_TRACK02");
    const char *system_card_path = getenv("THERON_SYSTEM_CARD");
    unsigned char *track02 = NULL;
    unsigned char *system_card = NULL;
    size_t track02_bytes = 0u;
    size_t system_card_bytes = 0u;
    char track02_md5[33];
    char system_card_md5_hex[33];
    Theron_Track02IplLoaderReceipt ipl;
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1SystemCardIrq2EntryGate system_card_gate;
    Theron_StartupMediaStateReceipt bitmap;
    int result = 1;

    if (!track02_path || !track02_path[0] || !system_card_path ||
        !system_card_path[0]) {
        printf("status=skip reason=explicit_raw_track02_and_system_card_required "
               "emulator=not_started fallback=not_run\n");
        return 0;
    }
    if (!read_file(track02_path, &track02, &track02_bytes) ||
        !raw_track02_md5(track02_path, track02_bytes, track02_md5)) {
        printf("status=blocked reason=raw_track02_missing_or_unverified "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (!read_file(system_card_path, &system_card, &system_card_bytes) ||
        !system_card_md5(system_card_path, system_card_bytes,
                         system_card_md5_hex)) {
        printf("status=blocked reason=system_card_missing_or_unverified "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (theron_v1_track02_find_ipl_loader(track02, track02_bytes, track02_md5,
                                           &ipl) != THERON_TRACK02_SIGNAL_OK ||
        !ipl.valid || !ipl.stage2_cd_read_record_proven ||
        !ipl.stage2_cd_read_dynamic_boundary_valid ||
        ipl.stage2_cd_read_cpu_address !=
            THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS ||
        ipl.stage2_cd_read_destination != THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM ||
        ipl.stage2_cd_read_local_destination !=
            THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION ||
        ipl.vram_transfer_proven) {
        printf("status=blocked reason=ipl_cd_read_receipt_invalid "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (theron_v1_track02_inspect_stage2_dynamic_payload(
            track02, track02_bytes, track02_md5, &payload) !=
            THERON_TRACK02_SIGNAL_OK ||
        !payload.valid || payload.track02_record != ipl.stage2_cd_read_record ||
        payload.raw_sector != ipl.stage2_cd_read_raw_sector ||
        payload.user_data_bytes != THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        !payload.user_data_hash) {
        printf("status=blocked reason=cd_read_sector_receipt_invalid "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (!theron_v1_system_card_irq2_entry_gate_from_original_media(
            &payload, system_card, system_card_bytes, system_card_md5_hex,
            &system_card_gate) || !system_card_gate.valid ||
        system_card_gate.stage3_track02_record != payload.track02_record ||
        !system_card_gate.selected_branch_unobserved) {
        printf("status=blocked reason=system_card_cd_read_gate_invalid "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }

    theron_v1_startup_media_capture_track02_state_receipt(
        track02, track02_bytes, track02_md5, &bitmap);
    if (!theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(&bitmap) ||
        bitmap.startup_bitmap_raw_route_mask == 0u ||
        bitmap.startup_bitmap_atlas_checksum == 0u) {
        printf("status=blocked reason=raw_bitmap_gate_incomplete "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }

    printf("status=ready cd_read=validated bitmap_route_mask=0x%x "
           "bitmap_candidate_hash=%08x emulator=not_started fallback=not_run\n",
           bitmap.startup_bitmap_raw_route_mask,
           bitmap.startup_bitmap_atlas_checksum);
    result = 0;

done:
    free(track02);
    free(system_card);
    return result;
}
