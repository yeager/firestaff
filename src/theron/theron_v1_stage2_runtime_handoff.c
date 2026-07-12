#include "theron_v1_stage2_runtime_handoff.h"
#include "theron_v1_stage3_irq2_dispatch.h"
#include "theron_v1_stage3_mode1_header.h"

#include <string.h>

/* Original stage-two code clears $26ff then TII-copies it through $37ff
 * immediately before `jmp $3800` (theron-us-stage2-huc6280.asm:176-181). */
#define THERON_V1_STAGE3_WORK_RAM_CLEAR_START 0x2700u
#define THERON_V1_STAGE3_WORK_RAM_CLEAR_BYTES 0x1100u
#define THERON_V1_STAGE3_WORK_RAM_CLEAR_END 0x3800u
#define THERON_V1_RAW_SECTOR_BYTES 2352u
#define THERON_V1_MODE1_USER_DATA_OFFSET 16u
#define THERON_V1_MODE1_USER_DATA_BYTES 2048u
#define THERON_V1_IPL_PRELOAD_TABLE_USER_OFFSET 0xdcu
#define THERON_V1_STAGE2_CD_EXEC_TABLE_USER_OFFSET 0xd5u
#define THERON_V1_STAGE2_CD_READ_SETUP_USER_OFFSET 0x80u
#define THERON_V1_IPL_PRELOAD_SETUP_USER_OFFSET 0xc1u
#define THERON_V1_IPL_PRELOAD_CALL_DELTA 12u
#define THERON_V1_IPL_PRELOAD_RECORD 0x0003e3u
#define THERON_V1_IPL_PRELOAD_SECTOR_COUNT 2u

static int theron_v1_mode1_sector_is_valid(const uint8_t *sector) {
    size_t index;

    if (!sector || sector[0] != 0x00u || sector[11] != 0x00u ||
        sector[15] != 0x01u) {
        return 0;
    }
    for (index = 1u; index < 11u; ++index) {
        if (sector[index] != 0xffu) return 0;
    }
    return 1;
}

static void theron_v1_mode1_user_data_summary(
    const uint8_t *first_sector,
    const uint8_t *second_sector,
    size_t *out_first_nonzero_offset,
    size_t *out_nonzero_byte_count,
    uint32_t *out_hash) {
    const uint8_t *sectors[2] = {first_sector, second_sector};
    uint32_t hash = 2166136261u;
    size_t first_nonzero = 2u * THERON_V1_MODE1_USER_DATA_BYTES;
    size_t nonzero_count = 0u;
    size_t sector_index;

    for (sector_index = 0u; sector_index < 2u; ++sector_index) {
        size_t index;
        for (index = 0u; index < THERON_V1_MODE1_USER_DATA_BYTES; ++index) {
            uint8_t value = sectors[sector_index][
                THERON_V1_MODE1_USER_DATA_OFFSET + index];
            size_t logical_offset = sector_index *
                THERON_V1_MODE1_USER_DATA_BYTES + index;
            hash ^= value;
            hash *= 16777619u;
            if (value != 0u) {
                if (first_nonzero == 2u * THERON_V1_MODE1_USER_DATA_BYTES) {
                    first_nonzero = logical_offset;
                }
                ++nonzero_count;
            }
        }
    }
    if (out_first_nonzero_offset) *out_first_nonzero_offset = first_nonzero;
    if (out_nonzero_byte_count) *out_nonzero_byte_count = nonzero_count;
    if (out_hash) *out_hash = hash;
}

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
    Theron_V1Stage3Mode1HeaderReceipt mode1_header;
    size_t preload_raw_sector;
    size_t preload_table_offset;
    size_t preload_raw_offset;
    size_t preload_call_offset;
    size_t stage2_exec_table_offset;
    size_t stage2_read_setup_offset;
    static const uint8_t preload_return_sequence[] = {
        0x20u, 0x09u, 0xe0u, /* JSR $e009 */
        0xc9u, 0x00u,       /* CMP #$00 */
        0xd0u, 0xd5u,       /* BNE $40a9 */
        0x60u               /* RTS */
    };
    static const uint8_t stage2_read_setup[] = {
        0xa9u, 0x01u, 0x85u, 0xf8u, /* AL = 1 */
        0xa9u, 0x01u, 0x85u, 0xffu, /* DH = local RAM */
        0xa9u, 0x00u, 0x85u, 0xfau,
        0xa9u, 0x38u, 0x85u, 0xfbu, /* BX = $3800 */
        0x20u, 0x09u, 0xe0u        /* JSR $e009 */
    };
    static const uint8_t stage2_post_read_transfer[] = {
        0xc9u, 0x00u, 0xd0u, 0xe7u, /* retry on nonzero status */
        0xadu, 0x8au, 0x27u, 0xaeu, 0x8bu, 0x27u,
        0xacu, 0x8cu, 0x27u, 0xc8u, 0x9cu, 0xffu, 0x26u,
        0x73u, 0xffu, 0x26u, 0x00u, 0x27u, 0x00u, 0x11u,
        0x4cu, 0x00u, 0x38u        /* JMP $3800 */
    };

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
    memset(&mode1_header, 0, sizeof(mode1_header));
    if (theron_v1_track02_find_ipl_loader(
            track02_data, track02_size, md5_hex, &loader) !=
            THERON_TRACK02_SIGNAL_OK || !loader.valid ||
        loader.variant != variant ||
        loader.stage2_record != THERON_TRACK02_IPL_STAGE2_RECORD ||
        loader.cd_read_cpu_address != THERON_TRACK02_IPL_CD_READ_CPU_ADDRESS ||
        loader.cd_read_user_data_offset !=
            THERON_V1_IPL_PRELOAD_SETUP_USER_OFFSET ||
        loader.cd_read_system_card_address !=
            THERON_TRACK02_IPL_CD_READ_SYSTEM_CARD_ADDRESS ||
        loader.cd_read_destination != THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM ||
        loader.cd_read_local_destination !=
            THERON_TRACK02_IPL_CD_READ_LOCAL_DESTINATION ||
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
        !theron_v1_stage3_mode1_header_from_original_media(
            track02_data, track02_size, &payload, &mode1_header) ||
        !mode1_header.valid || mode1_header.variant != variant ||
        mode1_header.track02_record != payload.track02_record ||
        mode1_header.raw_sector != payload.raw_sector ||
        !theron_v1_stage3_irq2_dispatch_from_original_media(
            track02_data, track02_size, &payload, &dispatch) ||
        !dispatch.valid || !dispatch.irq2_dispatch_proven) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        return 0;
    }
    if (loader.data_track_index01_raw_sector > SIZE_MAX -
            THERON_V1_IPL_PRELOAD_RECORD ||
        loader.executable_raw_sector > SIZE_MAX / THERON_V1_RAW_SECTOR_BYTES ||
        loader.stage2_raw_sector > SIZE_MAX / THERON_V1_RAW_SECTOR_BYTES ||
        loader.executable_raw_sector * THERON_V1_RAW_SECTOR_BYTES >
            track02_size ||
        loader.stage2_raw_sector * THERON_V1_RAW_SECTOR_BYTES >
            track02_size ||
        THERON_V1_MODE1_USER_DATA_OFFSET + THERON_V1_IPL_PRELOAD_TABLE_USER_OFFSET +
            4u > track02_size -
                loader.executable_raw_sector * THERON_V1_RAW_SECTOR_BYTES ||
        THERON_V1_MODE1_USER_DATA_OFFSET + THERON_V1_STAGE2_CD_READ_SETUP_USER_OFFSET +
            sizeof(stage2_read_setup) + sizeof(stage2_post_read_transfer) >
            track02_size - loader.stage2_raw_sector * THERON_V1_RAW_SECTOR_BYTES) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        return 0;
    }
    preload_raw_sector = loader.data_track_index01_raw_sector +
        THERON_V1_IPL_PRELOAD_RECORD;
    if (preload_raw_sector > SIZE_MAX / THERON_V1_RAW_SECTOR_BYTES ||
        preload_raw_sector + THERON_V1_IPL_PRELOAD_SECTOR_COUNT >
            track02_size / THERON_V1_RAW_SECTOR_BYTES) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        return 0;
    }
    preload_table_offset = loader.executable_raw_sector *
        THERON_V1_RAW_SECTOR_BYTES + THERON_V1_MODE1_USER_DATA_OFFSET +
        THERON_V1_IPL_PRELOAD_TABLE_USER_OFFSET;
    preload_call_offset = loader.executable_raw_sector *
        THERON_V1_RAW_SECTOR_BYTES + THERON_V1_MODE1_USER_DATA_OFFSET +
        loader.cd_read_user_data_offset + THERON_V1_IPL_PRELOAD_CALL_DELTA;
    preload_raw_offset = preload_raw_sector * THERON_V1_RAW_SECTOR_BYTES;
    stage2_exec_table_offset = loader.executable_raw_sector *
        THERON_V1_RAW_SECTOR_BYTES + THERON_V1_MODE1_USER_DATA_OFFSET +
        THERON_V1_STAGE2_CD_EXEC_TABLE_USER_OFFSET;
    stage2_read_setup_offset = loader.stage2_raw_sector *
        THERON_V1_RAW_SECTOR_BYTES + THERON_V1_MODE1_USER_DATA_OFFSET +
        THERON_V1_STAGE2_CD_READ_SETUP_USER_OFFSET;
    if (memcmp(track02_data + preload_table_offset,
               "\x00\xe3\x03\x02", 4u) != 0 ||
        memcmp(track02_data + stage2_exec_table_offset,
               "\x00\xe7\x03\x11", 4u) != 0 ||
        memcmp(track02_data + stage2_read_setup_offset, stage2_read_setup,
               sizeof(stage2_read_setup)) != 0 ||
        memcmp(track02_data + stage2_read_setup_offset +
                   sizeof(stage2_read_setup), stage2_post_read_transfer,
               sizeof(stage2_post_read_transfer)) != 0 ||
        memcmp(track02_data + preload_call_offset, preload_return_sequence,
               sizeof(preload_return_sequence)) != 0 ||
        !theron_v1_mode1_sector_is_valid(track02_data + preload_raw_offset) ||
        !theron_v1_mode1_sector_is_valid(track02_data + preload_raw_offset +
                                         THERON_V1_RAW_SECTOR_BYTES)) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        return 0;
    }
    out_handoff->physical_stage3_entry_verified = 1;
    out_handoff->stage3_entry_opcode = dispatch.opcode;
    out_handoff->stage3_irq2_selector = dispatch.irq2_selector;
    out_handoff->stage3_continuation_address = dispatch.continuation_address;
    out_handoff->ipl_preload_local_read_verified = 1;
    out_handoff->ipl_preload_cpu_address = loader.cd_read_cpu_address;
    out_handoff->ipl_preload_destination = loader.cd_read_local_destination;
    out_handoff->ipl_preload_record_proven = 1;
    out_handoff->ipl_preload_record = THERON_V1_IPL_PRELOAD_RECORD;
    out_handoff->ipl_preload_sector_count = THERON_V1_IPL_PRELOAD_SECTOR_COUNT;
    out_handoff->ipl_preload_raw_sector = preload_raw_sector;
    out_handoff->ipl_preload_returns_to_ipl_proven = 1;
    out_handoff->stage2_cd_exec_table_verified = 1;
    out_handoff->stage2_cd_read_setup_verified = 1;
    out_handoff->stage2_post_read_transfer_verified = 1;
    out_handoff->ipl_preload_user_data_bytes =
        THERON_V1_IPL_PRELOAD_SECTOR_COUNT * THERON_V1_MODE1_USER_DATA_BYTES;
    theron_v1_mode1_user_data_summary(
        track02_data + preload_raw_offset,
        track02_data + preload_raw_offset + THERON_V1_RAW_SECTOR_BYTES,
        &out_handoff->ipl_preload_first_nonzero_offset,
        &out_handoff->ipl_preload_nonzero_byte_count,
        &out_handoff->ipl_preload_user_data_hash);
    if (out_handoff->ipl_preload_first_nonzero_offset >=
            out_handoff->ipl_preload_user_data_bytes ||
        out_handoff->ipl_preload_nonzero_byte_count == 0u ||
        out_handoff->ipl_preload_user_data_hash == 0u) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        return 0;
    }
    out_handoff->stage3_mode1_header_verified = 1;
    out_handoff->stage3_minute_bcd = mode1_header.minute_bcd;
    out_handoff->stage3_second_bcd = mode1_header.second_bcd;
    out_handoff->stage3_frame_bcd = mode1_header.frame_bcd;
    return 1;
}
