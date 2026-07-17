#ifndef THERON_V1_TRACK02_RAW_MEDIA_INTAKE_H
#define THERON_V1_TRACK02_RAW_MEDIA_INTAKE_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_track02.h"

#define THERON_V1_TRACK02_MEDIA_PATH_CAPACITY 512

typedef enum {
    THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE = 0,
    THERON_V1_TRACK02_MEDIA_INTAKE_REJECTED,
    THERON_V1_TRACK02_MEDIA_INTAKE_READY
} Theron_V1Track02MediaIntakeStatus;

/* File-backed, hash-authenticated Track 02 container receipt. It exposes
 * only CUE/sector coordinates and never reads a level, object, bitmap, or
 * palette payload. */
typedef struct {
    Theron_V1Track02MediaIntakeStatus status;
    int cue_consumed;
    int mode1_2352;
    int mode1_2048;
    int raw_trace_preparation_allowed;
    Theron_Track02Variant variant;
    char track02_md5[33];
    char media_path[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY];
    char payload_path[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY];
    uint32_t cue_index01_sector;
    size_t payload_bytes;
    size_t sector_count;
    size_t first_user_data_offset;
    size_t logical_user_data_window_bytes;
} Theron_V1Track02RawMediaIntakeReceipt;

/* Exact raw-sector coordinates that a loader-trace caller can consume after
 * intake has authenticated both the container and its CUE placement. This is
 * intentionally not a decoded byte stream: each MODE1/2352 sector retains its
 * 16-byte header and trailing non-user-data bytes in the source file. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    char payload_path[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY];
    uint32_t cue_index01_sector;
    size_t first_user_data_offset;
    size_t logical_user_data_window_bytes;
} Theron_V1Track02RawTraceMediaInput;

/* Inspects one explicit user-supplied .cue, .bin, or .iso path. CUE input
 * requires exactly one Track 02 MODE1/2352 or MODE1/2048 BINARY member with
 * exactly one INDEX 01. Raw trace preparation is enabled only for the known
 * raw BIN variants with their source-locked CUE Index 01 sector. Missing media
 * returns UNAVAILABLE; malformed, unknown, or mismatched media returns
 * REJECTED without a fallback route. */
int theron_v1_track02_raw_media_intake_discover(
    const char *media_path,
    Theron_V1Track02RawMediaIntakeReceipt *out);

/* Narrows a READY raw MODE1/2352 CUE receipt into source coordinates for the
 * existing loader-trace preparation. ISO and bare BIN input remain discovered
 * but are deliberately not eligible for this raw-trace route. */
int theron_v1_track02_raw_media_intake_prepare_trace_input(
    const Theron_V1Track02RawMediaIntakeReceipt *intake,
    Theron_V1Track02RawTraceMediaInput *out);

#endif
