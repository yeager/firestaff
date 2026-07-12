#include "theron_v1_stage2_runtime_handoff.h"
#include "theron_v1_stage3_irq2_dispatch.h"

#include <string.h>

/* Original stage-two code clears $26ff then TII-copies it through $37ff
 * immediately before `jmp $3800` (theron-us-stage2-huc6280.asm:176-181). */
#define THERON_V1_STAGE3_WORK_RAM_CLEAR_START 0x2700u
#define THERON_V1_STAGE3_WORK_RAM_CLEAR_BYTES 0x1100u
#define THERON_V1_STAGE3_WORK_RAM_CLEAR_END 0x3800u

int theron_v1_stage2_runtime_handoff_from_dynamic_payload(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1Stage2RuntimeHandoff *out_handoff) {

    uint32_t expected_record;

    if (!out_handoff) return 0;
    memset(out_handoff, 0, sizeof(*out_handoff));
    if (!payload || !payload->valid) return 0;

    if (payload->variant == THERON_TRACK02_VARIANT_JP_BIN) {
        expected_record = THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP;
    } else if (payload->variant == THERON_TRACK02_VARIANT_US_BIN) {
        expected_record = THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US;
    } else {
        return 0;
    }

    if (payload->track02_record != expected_record ||
        payload->raw_sector == 0u || payload->user_data_offset == 0u ||
        payload->user_data_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        payload->header_word0 != 0x00ffu || payload->header_word1 != 0x0308u ||
        payload->manifest_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES ||
        payload->manifest_entry_count !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT ||
        payload->nonzero_byte_count == 0u || payload->user_data_hash == 0u) {
        return 0;
    }

    out_handoff->valid = 1;
    out_handoff->variant = payload->variant;
    out_handoff->track02_record = payload->track02_record;
    out_handoff->raw_sector = payload->raw_sector;
    out_handoff->user_data_offset = payload->user_data_offset;
    out_handoff->user_data_bytes = payload->user_data_bytes;
    out_handoff->load_address = THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION;
    out_handoff->entry_address = THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION;
    out_handoff->execute_after_load = 1;
    out_handoff->cleared_work_ram_start = THERON_V1_STAGE3_WORK_RAM_CLEAR_START;
    out_handoff->cleared_work_ram_bytes = THERON_V1_STAGE3_WORK_RAM_CLEAR_BYTES;
    out_handoff->cleared_work_ram_end = THERON_V1_STAGE3_WORK_RAM_CLEAR_END;
    out_handoff->work_ram_cleared_before_entry = 1;
    out_handoff->header_word0 = payload->header_word0;
    out_handoff->header_word1 = payload->header_word1;
    out_handoff->manifest_bytes = payload->manifest_bytes;
    out_handoff->manifest_entry_count = payload->manifest_entry_count;
    out_handoff->user_data_hash = payload->user_data_hash;
    out_handoff->manifest_entries_semantically_unbound = 1;
    return 1;
}

int theron_v1_stage2_runtime_handoff_from_original_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_V1Stage2RuntimeHandoff *out_handoff) {
    Theron_Track02Variant variant;
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1Stage3Irq2DispatchReceipt dispatch;

    if (!out_handoff) return 0;
    memset(out_handoff, 0, sizeof(*out_handoff));
    if (!track02_data || track02_size == 0u || !md5_hex || !md5_hex[0]) {
        return 0;
    }
    variant = theron_v1_track02_variant_for_md5(md5_hex);
    if (variant != THERON_TRACK02_VARIANT_JP_BIN &&
        variant != THERON_TRACK02_VARIANT_US_BIN) {
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&payload, 0, sizeof(payload));
    memset(&dispatch, 0, sizeof(dispatch));
    if (theron_v1_track02_find_ipl_loader(
            track02_data, track02_size, md5_hex, &loader) !=
            THERON_TRACK02_SIGNAL_OK || !loader.valid ||
        loader.variant != variant ||
        loader.stage2_record != THERON_TRACK02_IPL_STAGE2_RECORD ||
        loader.stage2_sector_count != THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT ||
        loader.stage2_destination != THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM ||
        loader.stage2_load_address != THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS ||
        loader.stage2_entry_address != THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS ||
        loader.stage2_cd_read_cpu_address !=
            THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS ||
        loader.stage2_cd_read_sector_count != 1u ||
        loader.stage2_cd_read_destination !=
            THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM ||
        loader.stage2_cd_read_local_destination !=
            THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION ||
        !loader.stage2_cd_read_record_proven ||
        !loader.stage2_cd_read_dynamic_boundary_valid ||
        loader.stage2_cd_read_live_record_register_mask !=
            THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_MASK ||
        loader.vram_transfer_proven ||
        theron_v1_track02_inspect_stage2_dynamic_payload(
            track02_data, track02_size, md5_hex, &payload) !=
            THERON_TRACK02_SIGNAL_OK ||
        payload.track02_record != loader.stage2_cd_read_record ||
        payload.raw_sector != loader.stage2_cd_read_raw_sector ||
        !theron_v1_stage2_runtime_handoff_from_dynamic_payload(
            &payload, out_handoff) ||
        !theron_v1_stage3_irq2_dispatch_from_original_media(
            track02_data, track02_size, &payload, &dispatch) ||
        !dispatch.valid || !dispatch.irq2_dispatch_proven) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        return 0;
    }
    out_handoff->physical_stage3_entry_verified = 1;
    out_handoff->stage3_entry_opcode = dispatch.opcode;
    out_handoff->stage3_irq2_selector = dispatch.irq2_selector;
    out_handoff->stage3_continuation_address = dispatch.continuation_address;
    return 1;
}
