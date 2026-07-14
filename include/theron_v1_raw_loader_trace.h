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

/* Immutable join of two already-authenticated facts: the original runtime's
 * observed $4090 CD_READ receipt and the matching MODE1/2048 Stage 3 sector
 * receipt from the hash-verified Track 02.  `stage3_handoff_record_proven`
 * establishes only the loader's executed record boundary.  In particular it
 * does not identify a Soul Room selection, a dungeon record, or any payload
 * format within the Stage 3 user-data window. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t stage3_track02_record;
    size_t stage3_raw_sector;
    size_t stage3_raw_offset;
    size_t stage3_user_data_offset;
    size_t stage3_user_data_bytes;
    uint32_t stage3_user_data_hash;
    size_t observed_destination_span_bytes;
    uint32_t observed_destination_span_checksum;
    int observed_cd_read_to_media_span_verified;
    int stage3_handoff_record_proven;
} Theron_V1RawLoaderTraceStage3SectorReceipt;

/* A later System Card $e009 call/return observed in the same authenticated
 * Mednafen lineage. This binds the executed CD record range to raw Track 02
 * user data, but deliberately assigns no dungeon, object, bitmap, palette,
 * or payload-format meaning to those bytes. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t stage3_track02_record;
    uint32_t later_track02_record;
    uint16_t descriptor_selector;
    size_t descriptor_selector_ordinal;
    uint16_t caller_pc;
    uint16_t return_pc;
    uint8_t sector_count;
    size_t first_raw_sector;
    size_t first_raw_offset;
    size_t first_user_data_offset;
    size_t user_data_bytes;
    uint32_t user_data_hash;
    int later_e009_return_verified;
    int later_cd_read_to_media_verified;
    int descriptor_selector_bound;
} Theron_V1RawLoaderTraceLaterSectorReceipt;

/* An independently observed SCSI raw-sector receipt from a provenance-marked
 * Mednafen CD sidecar. It proves that the complete captured physical CD sector
 * and its bounded leading span match the selector-resolved Track 02 record.
 * It does not establish that $e009 initiated that read, or assign any payload
 * format, dungeon, object, bitmap, palette, or transition meaning. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t later_track02_record;
    uint16_t descriptor_selector;
    size_t descriptor_selector_ordinal;
    int observed_raw_sector_lba;
    size_t observed_raw_sector_bytes;
    uint32_t observed_raw_sector_checksum;
    size_t observed_raw_sector_span_bytes;
    uint32_t observed_raw_sector_span_checksum;
    int same_capture_raw_sector_span_verified;
} Theron_V1RawLoaderTraceLaterRawSectorWitness;

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

/* Joins an existing media-bound trace with the existing Stage 3 MODE1 receipt.
 * The caller must obtain both inputs from the hash-verified raw Track 02;
 * this helper neither reads media nor decodes the user-data payload. */
int theron_v1_raw_loader_trace_stage3_sector_receipt_from_bound_span(
    const Theron_V1RawLoaderTraceReceipt *trace,
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1RawLoaderTraceStage3SectorReceipt *out);

/* Consumes one complete later `JSR $e009` dispatch/return envelope from the
 * original Mednafen capture and binds its captured record range to the same
 * hash-verified raw Track 02 identity as `trace`. The prior $4090->$3800
 * receipt must already be media-bound. The captured record must also resolve
 * through the original Stage 3 descriptor-selector coordinates.
 * This is a loader-coordinate handoff only; it cannot authorize a dungeon
 * load or rendering. */
int theron_v1_raw_loader_trace_bind_later_e009_sector(
    const Theron_V1RawLoaderTraceReceipt *trace,
    const char *capture,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceLaterSectorReceipt *out);

/* Binds a selector-coordinate receipt to one provenance-marked Mednafen SCSI
 * sidecar sector. The sidecar must contain exactly one complete-sector and
 * bounded-span fingerprint pair matching the selector-resolved raw Track 02
 * sector. This remains an independent CD/media observation, not an
 * e009-to-sector causality or capture-session identity claim. */
int theron_v1_raw_loader_trace_witness_later_e009_raw_sector(
    const Theron_V1RawLoaderTraceLaterSectorReceipt *later_receipt,
    const char *cd_capture,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceLaterRawSectorWitness *out);

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
