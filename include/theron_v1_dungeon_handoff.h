#ifndef THERON_V1_DUNGEON_HANDOFF_H
#define THERON_V1_DUNGEON_HANDOFF_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_runtime_admission.h"

#define THERON_V1_INITIAL_ENVELOPE_RECORD 0x0b52u
#define THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET 0x114u
#define THERON_V1_INITIAL_ENVELOPE_BYTES 0x36cu
#define THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH 0x0020u
#define THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT 0x001bu
#define THERON_V1_INITIAL_ENVELOPE_HEADER_SEED 0x0108e938u
#define THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER 0x0026u

#define THERON_V1_TRACK02_RAW_SECTOR_BYTES 2352u
#define THERON_V1_TRACK02_MODE1_HEADER_BYTES 16u
#define THERON_V1_TRACK02_MD5_JP_BIN "b7afb338ad31be1025b53f9aff12d73a"
#define THERON_V1_TRACK02_MD5_US_BIN "f23601102138f87c33025877767ebf76"

typedef struct {
    const Theron_V1RuntimeAdmissionReceipt *runtime_admission;
    int track02_hash_verified;
    const char *track02_md5;
    const uint8_t *raw_track02;
    size_t raw_track02_bytes;
    uint32_t cue_track02_index01_raw_sector;
} Theron_V1DungeonHandoffFacts;

typedef struct {
    int selected;
    int runtime_route_consumed;
    uint32_t record;
    uint32_t record_user_data_offset;
    uint32_t envelope_bytes;
    uint16_t header_width;
    uint16_t header_height;
    uint32_t header_seed;
    uint16_t header_identifier;
    uint32_t cue_track02_index01_raw_sector;
    uint32_t track02_raw_sector;
    uint32_t raw_sector_offset;
    int raw_track02_md5_verified;
    int adjacent_boundary_opaque;
    const char *route;
} Theron_V1DungeonHandoffReceipt;

/* Compares an exact raw byte span with a supplied MD5 identity. This is an
 * integrity primitive only; it assigns no Track 02 record or payload role. */
int theron_v1_track02_raw_bytes_match_md5(const uint8_t *bytes,
                                          size_t byte_count,
                                          const char *expected_md5);

/* Selects only the source-locked startup envelope after independently hashing
 * the complete raw MODE1/2352 BIN against the declared JP or US identity, then
 * checking its exact placement, literal header, and CUE Index 01 sector. This
 * exports no envelope bytes and assigns no dungeon, object, bitmap, palette,
 * or grammar role to the following boundary. */
int theron_v1_dungeon_handoff_select_initial_level(
    const Theron_V1DungeonHandoffFacts *facts,
    Theron_V1DungeonHandoffReceipt *out_receipt);

#endif
