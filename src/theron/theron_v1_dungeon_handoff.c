#include "theron_v1_dungeon_handoff.h"

#include <string.h>

enum {
    TQR_DESCRIPTOR_BYTES = 18u,
    TQR_JP_CANDIDATE_RAW_OFFSET = 0x700c84u,
    TQR_US_CANDIDATE_RAW_OFFSET = 0x7015b4u,
    TQR_JP_DESCRIPTOR_RAW_OFFSET = 0x70ffd4u,
    TQR_US_DESCRIPTOR_RAW_OFFSET = 0x710904u,
    TQR_JP_CUE_INDEX01_RAW_SECTOR = 224u,
    TQR_US_CUE_INDEX01_RAW_SECTOR = 225u,
    TQR_INITIAL_ENVELOPE_RAW_SECTOR_OFFSET = 0x124u,
    TQR_INITIAL_BOUNDARY_RAW_SECTOR_OFFSET = 0x490u
};

static const uint8_t g_descriptor[TQR_DESCRIPTOR_BYTES] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20,
    0x10, 0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static int raw_range_present(size_t track_bytes, size_t offset, size_t bytes) {
    return offset <= track_bytes && bytes <= track_bytes - offset;
}

int theron_v1_dungeon_handoff_select_initial_level(
    const Theron_V1DungeonHandoffFacts *facts,
    Theron_V1DungeonHandoffReceipt *out_receipt) {
    Theron_V1DungeonHandoffReceipt receipt = {0};
    size_t candidate_offset;
    size_t descriptor_offset;
    uint32_t cue_index01_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!facts || !out_receipt || !facts->runtime_admission ||
        !facts->runtime_admission->attached ||
        !facts->runtime_admission->admitted || !facts->track02_hash_verified ||
        !facts->track02_md5 || !facts->raw_track02 ||
        facts->raw_track02_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u) {
        return 0;
    }

    if (strcmp(facts->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0) {
        candidate_offset = TQR_JP_CANDIDATE_RAW_OFFSET;
        descriptor_offset = TQR_JP_DESCRIPTOR_RAW_OFFSET;
        cue_index01_sector = TQR_JP_CUE_INDEX01_RAW_SECTOR;
    } else if (strcmp(facts->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0) {
        candidate_offset = TQR_US_CANDIDATE_RAW_OFFSET;
        descriptor_offset = TQR_US_DESCRIPTOR_RAW_OFFSET;
        cue_index01_sector = TQR_US_CUE_INDEX01_RAW_SECTOR;
    } else {
        return 0;
    }

    if (facts->cue_track02_index01_raw_sector != cue_index01_sector ||
        !raw_range_present(facts->raw_track02_bytes, candidate_offset,
                           THERON_V1_INITIAL_ENVELOPE_BYTES) ||
        !raw_range_present(facts->raw_track02_bytes, descriptor_offset,
                           sizeof(g_descriptor)) ||
        candidate_offset % THERON_V1_TRACK02_RAW_SECTOR_BYTES !=
            TQR_INITIAL_ENVELOPE_RAW_SECTOR_OFFSET ||
        (candidate_offset + THERON_V1_INITIAL_ENVELOPE_BYTES) %
            THERON_V1_TRACK02_RAW_SECTOR_BYTES !=
            TQR_INITIAL_BOUNDARY_RAW_SECTOR_OFFSET ||
        memcmp(facts->raw_track02 + descriptor_offset, g_descriptor,
               sizeof(g_descriptor)) != 0 ||
        facts->raw_track02[candidate_offset] != 0x00u ||
        facts->raw_track02[candidate_offset + 1u] != 0x20u ||
        facts->raw_track02[candidate_offset + 2u] != 0x00u ||
        facts->raw_track02[candidate_offset + 3u] != 0x1bu ||
        facts->raw_track02[candidate_offset + 4u] != 0x01u ||
        facts->raw_track02[candidate_offset + 5u] != 0x08u ||
        facts->raw_track02[candidate_offset + 6u] != 0xe9u ||
        facts->raw_track02[candidate_offset + 7u] != 0x38u ||
        facts->raw_track02[candidate_offset + 8u] != 0x00u ||
        facts->raw_track02[candidate_offset + 9u] != 0x26u) {
        return 0;
    }

    receipt.selected = 1;
    receipt.runtime_route_consumed = 1;
    receipt.record = (uint32_t)(candidate_offset /
        THERON_V1_TRACK02_RAW_SECTOR_BYTES - cue_index01_sector);
    receipt.record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    receipt.envelope_bytes = THERON_V1_INITIAL_ENVELOPE_BYTES;
    receipt.header_identifier = THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER;
    receipt.adjacent_boundary_opaque = 1;
    receipt.route = "raw_track02_initial_envelope";
    *out_receipt = receipt;
    return 1;
}
