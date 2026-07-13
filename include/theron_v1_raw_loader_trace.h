#ifndef THERON_V1_RAW_LOADER_TRACE_H
#define THERON_V1_RAW_LOADER_TRACE_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_startup_media.h"
#include "theron_v1_track02.h"

/*
 * A loader receipt is accepted only from the instrumented original Mednafen
 * capture.  Earlier revisions accepted a hand-authored list of I/O rows;
 * that cannot prove where a palette byte originated and must never unlock
 * Track 02 rendering.
 */
typedef struct {
    int valid;
    char track02_md5[33];
    Theron_Track02Variant variant;
    uint32_t dynamic_cd_read_record;
    uint8_t dynamic_cd_read_record_cl;
    uint8_t dynamic_cd_read_record_dl;
    uint8_t dynamic_cd_read_record_ch;
    uint16_t dynamic_cd_read_destination;
    size_t dynamic_cd_read_destination_span_bytes;
    uint32_t dynamic_cd_read_destination_span_checksum;
    /* Physical provenance for the traced $3800 destination span.  This is
     * populated only after the trace checksum matches the hash-verified raw
     * Track 02 sector that the original loader selected at runtime. */
    size_t dynamic_cd_read_raw_sector;
    size_t dynamic_cd_read_raw_offset;
    size_t dynamic_cd_read_user_data_offset;
    /* Full one-sector Stage2 payload receipt derived from the same
     * hash-verified Track 02 image as the traced destination span. */
    int stage2_dynamic_payload_verified;
    size_t stage2_dynamic_payload_bytes;
    uint32_t stage2_dynamic_payload_checksum;
    unsigned int palette_store_count;
    unsigned int palette_register_mask;
    unsigned int palette_word_count;
    uint16_t first_palette_word_index;
    uint16_t first_palette_word_value;
    uint32_t palette_word_checksum;
    uint16_t first_palette_store_pc;
    uint8_t first_palette_store_accumulator;
    int dynamic_cd_read_verified;
    int dynamic_cd_read_registers_verified;
    /* Direct checksum of original System Card destination RAM after the
     * authenticated CD_READ returned. It proves record-to-RAM transfer only. */
    int dynamic_cd_read_destination_span_verified;
    int dynamic_cd_read_media_span_verified;
    int palette_store_observed_after_dynamic_read;
    /* Kept separate deliberately: a VCE store is not RAM/CD byte taint. */
    int palette_descriptor_relation_verified;
    /* The Soul Room route is an independently catalogued raw-media receipt.
     * Its byte envelope is recorded here only after it has been checked
     * disjoint from the traced Stage2 $3800 span. This does not claim that
     * the Stage2 CD_READ loaded, decoded, or selected that route. */
    int soul_room_raw_route_verified;
    size_t soul_room_first_raw_offset;
    size_t soul_room_last_raw_offset;
    uint32_t soul_room_checksum;
    int soul_room_route_disjoint_from_dynamic_span;
    unsigned int bitmap_route_mask;
    uint32_t bitmap_atlas_checksum;
} Theron_V1RawLoaderTraceReceipt;

/* Parses a provenance-marked instrumented Mednafen trace.  It validates the
 * existing dynamic CD_READ/IRQ2 gate first, then records only VCE stores and
 * completed HuC6260 colour-table words that appear after that read in the
 * same original capture.  Completed VCE words establish hardware output
 * order, not Track 02 source-byte provenance. */
int theron_v1_raw_loader_trace_ingest_mednafen_capture(
    const char *capture,
    const char *track02_md5,
    Theron_V1RawLoaderTraceReceipt *out);

/* Bounded file wrapper for an explicit trace path. */
int theron_v1_raw_loader_trace_import_mednafen_capture_file(
    const char *path,
    const char *track02_md5,
    Theron_V1RawLoaderTraceReceipt *out);

/* Binds an accepted Mednafen CD_READ trace to the exact bytes of a
 * hash-verified raw Track 02 image.  It authenticates only the one-sector
 * $3800 transfer already observed in the original trace; it does not infer
 * a palette source, bitmap decoder, object table, or later dungeon record. */
int theron_v1_raw_loader_trace_bind_track02_destination_span(
    const Theron_V1RawLoaderTraceReceipt *trace,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceReceipt *out);

/* Binds only compatible real-media startup bitmap receipts. In addition to
 * preserving the existing bitmap-route contract, this binds the inspected
 * Stage2 payload and independently catalogued Soul Room raw route to one
 * Track 02 identity, proving their byte envelopes are disjoint. It does not
 * infer a loader-to-route relation, dungeon/object meaning, or palette
 * source. Callers must inspect palette_descriptor_relation_verified before
 * drawing. */
int theron_v1_raw_loader_trace_final_bind(
    const Theron_V1RawLoaderTraceReceipt *trace,
    const Theron_StartupMediaStateReceipt *media,
    Theron_V1RawLoaderTraceReceipt *out);

#endif /* THERON_V1_RAW_LOADER_TRACE_H */
