#ifndef THERON_V1_TRACK02_SECTOR_RECORD_ADMISSION_H
#define THERON_V1_TRACK02_SECTOR_RECORD_ADMISSION_H

#include "theron_v1_track02_raw_media_intake.h"

/*
 * A source-owned Stage-3 descriptor row can resolve one later CD record when
 * an original Mednafen trace observes the corresponding $e009 transaction.
 * This receipt names physical coordinates and hashes only.  The descriptor
 * words and selected user data remain opaque, and no route is drawable.
 */
typedef enum {
    THERON_V1_TRACK02_SECTOR_RECORD_UNAVAILABLE = 0,
    THERON_V1_TRACK02_SECTOR_RECORD_REJECTED,
    THERON_V1_TRACK02_SECTOR_RECORD_READY
} Theron_V1Track02SectorRecordAdmissionStatus;

typedef struct {
    Theron_V1Track02SectorRecordAdmissionStatus status;
    int raw_cue_bin_identity_consumed;
    int stage3_directory_consumed;
    int observed_later_loader_consumed;
    int nonstartup_record_consumed;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint32_t stage3_track02_record;
    size_t descriptor_ordinal;
    uint16_t descriptor_word0;
    uint16_t descriptor_word1;
    uint16_t descriptor_selector;
    size_t descriptor_source_raw_offset;
    uint32_t descriptor_source_hash;
    uint32_t resolved_track02_record;
    size_t record_raw_offset;
    size_t record_user_data_offset;
    size_t record_user_data_bytes;
    uint32_t record_user_data_hash;
    uint16_t loader_caller_pc;
    uint16_t loader_return_pc;
    uint32_t observed_raw_sector_checksum;
    int level_object_semantics_allowed;
    int bitmap_palette_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02SectorRecordAdmissionReceipt;

/* Reopens the already-admitted raw CUE/BIN member, rehashes it, and admits
 * exactly one observed non-startup descriptor-selected record. Missing local
 * input reports UNAVAILABLE. Any trace, pointer, bounds, or byte drift is
 * REJECTED; this function never parses a level/object payload. */
int theron_v1_track02_sector_record_admit_from_trace(
    const Theron_V1Track02RawMediaIntakeReceipt *intake,
    const char *coalesced_trace_path,
    Theron_V1Track02SectorRecordAdmissionReceipt *out);

#endif
