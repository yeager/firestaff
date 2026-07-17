#ifndef THERON_V1_TRACK02_LEVEL_OBJECT_TRACE_PREPARATION_H
#define THERON_V1_TRACK02_LEVEL_OBJECT_TRACE_PREPARATION_H

#include "theron_v1_track02_provenance_runtime_consumer.h"

/* Runtime preparation for a future original post-$3800 level/object trace.
 * It retains only authenticated trace windows and leaves every field decoder
 * and visual route disabled. */
typedef struct {
    int valid;
    int provenance_runtime_consumer_consumed;
    int original_consumer_trace_consumed;
    int exact_record_windows_verified;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint32_t loader_record;
    uint32_t consumer_trace_checksum;
    uint32_t dungeon_record_consumer_pc;
    size_t dungeon_record_payload_offset;
    size_t dungeon_record_byte_count;
    uint32_t dungeon_record_window_checksum;
    uint32_t object_table_consumer_pc;
    size_t object_table_payload_offset;
    size_t object_table_byte_count;
    uint32_t object_table_window_checksum;
    int level_field_decoder_required;
    int object_field_decoder_required;
    int level_admission_allowed;
    int object_admission_allowed;
    int bitmap_palette_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02LevelObjectTracePreparationReceipt;

/* Requires the existing original-trace grammar receipt to match the exact
 * runtime-owned loader record before retaining its bounded record windows.
 * This prepares admission only; it never interprets a level or object field. */
int theron_v1_track02_prepare_level_object_trace_runtime(
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02ObjectDungeonConsumerGrammarReceipt *grammar,
    Theron_V1Track02LevelObjectTracePreparationReceipt *out);

#endif
