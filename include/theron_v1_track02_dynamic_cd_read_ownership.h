#ifndef THERON_V1_TRACK02_DYNAMIC_CD_READ_OWNERSHIP_H
#define THERON_V1_TRACK02_DYNAMIC_CD_READ_OWNERSHIP_H

#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_track02_raw_media_intake.h"

/* Normalized ownership of the one observed IPL dynamic CD_READ. The record
 * is reconstructed from the captured CL/DL/CH bytes and tied to its raw
 * MODE1 sector; neither the destination nor payload receives a format claim. */
typedef struct {
    int valid;
    int raw_cue_bin_identity_consumed;
    int loader_trace_consumed;
    int register_record_normalized;
    int raw_sector_window_owned;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint8_t record_cl;
    uint8_t record_dl;
    uint8_t record_ch;
    uint32_t track02_record;
    uint16_t destination;
    size_t raw_sector;
    size_t raw_offset;
    size_t user_data_offset;
    size_t user_data_bytes;
    uint32_t destination_span_checksum;
    uint32_t full_payload_checksum;
    int level_object_semantics_allowed;
    int bitmap_palette_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02DynamicCdReadOwnershipReceipt;

/* Joins only already authenticated CUE/BIN and Mednafen loader receipts.
 * Any register, MD5, raw-sector, CUE-window, or payload-span drift rejects. */
int theron_v1_track02_dynamic_cd_read_ownership_normalize(
    const Theron_V1Track02RawMediaIntakeReceipt *intake,
    const Theron_V1RawLoaderTraceReceipt *trace,
    Theron_V1Track02DynamicCdReadOwnershipReceipt *out);

#endif
