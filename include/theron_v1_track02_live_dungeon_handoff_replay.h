#ifndef THERON_V1_TRACK02_LIVE_DUNGEON_HANDOFF_REPLAY_H
#define THERON_V1_TRACK02_LIVE_DUNGEON_HANDOFF_REPLAY_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_bitmap_capture_runtime_admission.h"
#include "theron_v1_track02_capture_trace_runtime_admission.h"
#include "theron_v1_track02_palette_route.h"
#include "theron_v1_track02_raw_media_intake.h"

/* This external replay order is provenance only.  The last row names a
 * bounded record window, but never classifies its bytes as a level or object. */
typedef enum {
    THERON_V1_TRACK02_LIVE_REPLAY_MEDIA = 1,
    THERON_V1_TRACK02_LIVE_REPLAY_DYNAMIC_CD_READ,
    THERON_V1_TRACK02_LIVE_REPLAY_LOADER_CHAIN,
    THERON_V1_TRACK02_LIVE_REPLAY_PALETTE_OUTPUT,
    THERON_V1_TRACK02_LIVE_REPLAY_BITMAP_OUTPUT,
    THERON_V1_TRACK02_LIVE_REPLAY_DESTINATION_RECORD
} Theron_V1Track02LiveDungeonHandoffReplayEventKind;

#define THERON_V1_TRACK02_LIVE_REPLAY_EVENT_COUNT 6u

/* An event is an observed identity row, not CD payload data. */
typedef struct {
    Theron_V1Track02LiveDungeonHandoffReplayEventKind kind;
    unsigned int sequence;
    uint32_t primary_identity;
    uint32_t secondary_identity;
    size_t payload_offset;
    size_t payload_bytes;
} Theron_V1Track02LiveDungeonHandoffReplayEvent;

typedef struct {
    int valid;
    int raw_media_consumed;
    int dynamic_cd_read_consumed;
    int loader_chain_consumed;
    int palette_output_consumed;
    int bitmap_output_consumed;
    int destination_record_consumed;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint32_t dynamic_cd_read_record;
    uint32_t destination_loader_record;
    size_t destination_payload_offset;
    size_t destination_payload_bytes;
    uint32_t destination_window_checksum;
    int level_object_semantics_allowed;
    int pixel_decode_allowed;
    int render_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02LiveDungeonHandoffReplayReceipt;

/* Validates one complete external replay against existing authenticated
 * receipts. Every event must occur exactly once and in the declared order.
 * No caller can use this as a decoder, renderer, or fallback visual route. */
int theron_v1_track02_live_dungeon_handoff_replay_validate(
    const Theron_V1Track02RawMediaIntakeReceipt *media,
    const Theron_V1RawLoaderTraceReceipt *loader,
    const Theron_V1Track02PaletteRouteReceipt *palette,
    const Theron_V1BitmapCaptureRuntimeAdmissionReceipt *bitmap,
    const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *destination,
    const Theron_V1Track02LiveDungeonHandoffReplayEvent *events,
    size_t event_count,
    Theron_V1Track02LiveDungeonHandoffReplayReceipt *out);

#endif
