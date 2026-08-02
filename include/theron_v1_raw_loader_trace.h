#ifndef THERON_V1_RAW_LOADER_TRACE_H
#define THERON_V1_RAW_LOADER_TRACE_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_capture_manifest.h"
#include "theron_v1_startup_media.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_loader_intake.h"

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
    /* The complete source descriptor row selected by the observed loader
     * call. word2 is the selector above; word0/word1 remain raw, unclassified
     * table bytes. The row is retained only after its resolved MODE1 sector
     * matches the same captured CD-read payload. */
    uint16_t descriptor_word0;
    uint16_t descriptor_word1;
    uint32_t descriptor_record_user_data_hash;
    int descriptor_row_media_bound;
    int descriptor_semantics_proven;
    size_t descriptor_source_raw_offset;
    size_t descriptor_source_bytes;
    uint32_t descriptor_source_hash;
    int descriptor_source_bytes_proven;
    size_t descriptor_selector_occurrence_count;
    size_t descriptor_selector_first_ordinal;
    size_t descriptor_selector_last_ordinal;
    uint32_t descriptor_selector_row_hash;
    int descriptor_selector_aliases_proven;
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

/* One provenance-marked Mednafen transcript can retain the authenticated
 * Stage 2 loader row, one later $e009 call/return, and one complete raw CD
 * sector fingerprint in observation order. This receipt binds that sector's
 * bytes to the selector-resolved Track 02 record. It is deliberately a
 * loader/media coordinate receipt only: it does not identify a payload as a
 * dungeon, object table, graphics, palette, bitmap, or transition. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t stage3_track02_record;
    /* The same original-media Stage 3 sector begins with `BRK $ff` at
     * $3800. A later read is post-Stage-3 provenance only after the capture
     * records the IRQ2 continuation at $3802. These are control-flow
     * coordinates, not payload semantics. */
    uint16_t stage3_entry_pc;
    uint8_t stage3_irq2_selector;
    uint16_t stage3_continuation_pc;
    uint16_t stage3_post_irq2_next_pc;
    int stage3_post_irq2_resume_verified;
    uint32_t later_track02_record;
    uint16_t descriptor_selector;
    size_t descriptor_selector_ordinal;
    /* Exact raw descriptor row selected by the observed later CD_READ.
     * word2 is `descriptor_selector`; word0/word1 are held without a format
     * claim. The hash identifies that selected sector's original user data. */
    uint16_t descriptor_word0;
    uint16_t descriptor_word1;
    uint32_t descriptor_record_user_data_hash;
    int descriptor_row_media_bound;
    int descriptor_semantics_proven;
    size_t descriptor_source_raw_offset;
    size_t descriptor_source_bytes;
    uint32_t descriptor_source_hash;
    int descriptor_source_bytes_proven;
    size_t descriptor_selector_occurrence_count;
    size_t descriptor_selector_first_ordinal;
    size_t descriptor_selector_last_ordinal;
    uint32_t descriptor_selector_row_hash;
    int descriptor_selector_aliases_proven;
    uint16_t caller_pc;
    uint16_t return_pc;
    /* Exact caller instruction bytes observed by the capture hook. This proves
     * the e009 call edge itself, but assigns no gameplay-transition meaning. */
    uint8_t later_caller_opcode;
    uint16_t later_caller_target;
    int later_caller_control_verified;
    /* The register bytes supplied by this exact caller to the System Card.
     * They are retained as an opaque callsite/sector witness, not a route. */
    uint8_t later_record_cl;
    uint8_t later_record_dl;
    uint8_t later_record_ch;
    uint8_t sector_count;
    int observed_raw_sector_lba;
    uint32_t observed_raw_sector_checksum;
    uint32_t observed_raw_sector_span_checksum;
    /* The later call's DH/BX values and a post-return RAM fingerprint. The
     * hash is checked against the selected MODE1 user-data prefix only. */
    uint16_t later_local_destination;
    size_t later_destination_span_bytes;
    uint32_t later_destination_span_checksum;
    int later_destination_local_ram_verified;
    int later_destination_media_span_verified;
    /* A complete one-sector local-RAM witness emitted only by the
     * instrumented original Mednafen run after the same $e009 return. It
     * binds bytes, not any dungeon/object/bitmap interpretation. */
    size_t later_destination_payload_bytes;
    uint32_t later_destination_payload_checksum;
    int later_destination_payload_verified;
    /* One observed caller control edge after the System Card return. The
     * resume PC must equal return_pc; neither PC assigns gameplay meaning. */
    uint16_t later_post_return_resume_pc;
    uint16_t later_post_return_next_pc;
    int later_post_return_step_verified;
    int observation_order_verified;
    int selector_sector_bytes_verified;
} Theron_V1RawLoaderTraceCoalescedLaterReceipt;

/* One game-owned post-Stage-3 loader dispatch can be admitted only when an
 * instrumented original capture preserves the complete chain from its READ(6)
 * command through a FIFO-origin main-RAM byte and a later game-owned read of
 * that exact cell. The byte is intentionally opaque: this is not a level,
 * object, bitmap, palette, grid, or transition decoder. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    unsigned int dispatch_sequence;
    uint16_t dispatch_logical_pc;
    uint32_t dispatch_physical_pc;
    unsigned int scsi_generation;
    unsigned int scsi_lba;
    unsigned int scsi_sector_count;
    uint32_t raw_track02_record;
    unsigned int source_offset;
    unsigned long long fifo_sequence;
    uint32_t physical_destination;
    uint32_t reader_physical_pc;
    uint8_t source_byte;
    int cdb_read6_verified;
    int fifo_to_game_ram_verified;
    int game_ram_consumer_verified;
    int payload_semantics_proven;
} Theron_V1RawLoaderTraceGamePayloadReceipt;

/* A byte-level join between an authenticated game-RAM payload receipt and the
 * one source-locked initial envelope. It proves only that a captured byte
 * lies within the already-bounded raw-media envelope; it does not decode that
 * envelope or promote dungeon, object, graphics, palette, or grid semantics. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t track02_record;
    size_t raw_sector;
    size_t raw_sector_offset;
    size_t envelope_offset;
    uint8_t source_byte;
    int game_payload_chain_verified;
    int source_envelope_overlap_verified;
    int level_semantics_proven;
} Theron_V1RawLoaderTraceInitialEnvelopeByteReceipt;

/* A byte-level join between an authenticated game-RAM payload receipt and
 * the directly adjacent source-locked continuation in the same initial
 * loader sector. Its placement establishes an observed consumer of the
 * bounded continuation only. It neither calls that continuation an object
 * table nor assigns record grammar, palette, bitmap, grid, or visual meaning
 * to the byte. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t track02_record;
    size_t raw_sector;
    size_t raw_sector_offset;
    size_t continuation_offset;
    uint8_t source_byte;
    int game_payload_chain_verified;
    int source_continuation_overlap_verified;
    int object_table_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeByteReceipt;

#define THERON_V1_RAW_LOADER_INITIAL_POST_ENVELOPE_PREFIX_BYTES 12u

/* A contiguous, source-owned prefix of the post-envelope continuation seen
 * through one game-owned CD-to-RAM chain. The prefix is retained solely as a
 * future grammar-capture anchor; it is not an object-table header. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t track02_record;
    size_t raw_sector;
    unsigned int dispatch_sequence;
    unsigned int scsi_generation;
    unsigned int scsi_lba;
    unsigned int scsi_sector_count;
    uint8_t bytes[THERON_V1_RAW_LOADER_INITIAL_POST_ENVELOPE_PREFIX_BYTES];
    uint32_t bytes_hash;
    int contiguous_capture_chain_verified;
    int object_table_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopePrefixReceipt;

/* An observed HuC6280 TII transfer whose source begins at the directly
 * adjacent continuation in the authenticated `$3800` loader sector. This
 * establishes byte movement only: destination memory and its eventual
 * consumer are not assigned a level, object, palette, bitmap, or grid role. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t track02_record;
    uint16_t transfer_pc;
    uint32_t transfer_physical_pc;
    uint16_t source_address;
    uint16_t destination_address;
    size_t byte_count;
    uint32_t source_checksum;
    int manifest_bound;
    int source_continuation_transfer_verified;
    int object_table_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt;

/* A later main-RAM JSR that enters exactly the destination of an already
 * source-bound continuation TII transfer. This proves a code-stage handoff
 * from original CD bytes, but not a level/object record grammar. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt transfer;
    uint16_t call_pc;
    uint32_t call_physical_pc;
    uint16_t call_target;
    uint16_t return_instruction_pc;
    uint32_t return_instruction_physical_pc;
    uint16_t post_return_pc;
    uint32_t post_return_physical_pc;
    uint8_t post_return_opcode;
    int continuation_execution_proven;
    int continuation_termination_instruction_proven;
    int continuation_post_return_target_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeExecutionReceipt;

/* The next routine call observed exactly at the authenticated copied-code
 * return point. `execution.transfer` keeps the original Track 02 byte route;
 * the called routine remains unclassified until a separate source trace
 * proves how it consumes data. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeExecutionReceipt execution;
    uint16_t call_pc;
    uint32_t call_physical_pc;
    uint16_t call_target;
    int post_return_call_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallReceipt;

/* A source-observed return from the routine invoked at the authenticated
 * copied-code return point. This retains only bounded control-flow facts. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallReceipt call;
    uint16_t return_instruction_pc;
    uint32_t return_instruction_physical_pc;
    uint16_t post_return_pc;
    uint32_t post_return_physical_pc;
    uint8_t post_return_opcode;
    int post_return_call_termination_proven;
    int post_return_call_return_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallTerminationReceipt;

/* The first subsequently observed main-RAM JSR after the authenticated caller
 * resumed. It is a trace-order receipt only; the target remains opaque. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallTerminationReceipt
        termination;
    uint16_t call_pc;
    uint32_t call_physical_pc;
    uint16_t call_target;
    int caller_next_call_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallReceipt;

/* The observed main-RAM entry instruction for the next caller routine.
 * This proves target execution but deliberately not routine semantics. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallReceipt call;
    uint16_t entry_pc;
    uint32_t entry_physical_pc;
    uint8_t entry_opcode;
    int caller_next_call_entry_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallEntryReceipt;

/* The next observed main-RAM instruction after the authenticated caller-next
 * entry. It is a control-flow observation, not a decoder or data receipt. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallEntryReceipt entry;
    uint16_t next_pc;
    uint32_t next_physical_pc;
    uint8_t next_opcode;
    int caller_next_entry_next_instruction_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextEntryNextReceipt;

/* A subsequent TII that re-copies a bounded interval of the authenticated
 * Track 02-derived continuation. The moved bytes remain opaque. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextEntryNextReceipt next;
    uint16_t transfer_pc;
    uint32_t transfer_physical_pc;
    uint16_t source_address;
    uint16_t destination_address;
    size_t byte_count;
    uint16_t original_source_address;
    uint32_t source_checksum;
    int source_track02_bytes_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferReceipt;

/* The first observed main-RAM routine call after an admitted Track 02-derived
 * TII, constrained to the copied destination. No routine semantics follow. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferReceipt transfer;
    uint16_t call_pc;
    uint32_t call_physical_pc;
    uint16_t call_target;
    int transfer_destination_call_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallReceipt;

/* The observed main-RAM entry for the routine called at the authenticated TII
 * destination. It retains control-flow facts only. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallReceipt call;
    uint16_t entry_pc;
    uint32_t entry_physical_pc;
    uint8_t entry_opcode;
    int transfer_destination_call_entry_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryReceipt;

/* The exact copied Track 02 byte observed as the entry opcode at the bound
 * TII destination. This proves byte-to-execution only, not its meaning. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryReceipt
        entry;
    uint16_t copied_source_address;
    uint16_t original_source_address;
    uint8_t copied_source_byte;
    int copied_source_byte_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyReceipt;

/* The immediate observed successor to the copied destination entry. Both
 * observed opcodes must map back into the same bounded Track 02 copy span. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyReceipt
        entry_copy;
    uint16_t next_pc;
    uint32_t next_physical_pc;
    uint16_t original_source_address;
    uint8_t next_source_byte;
    int copied_successor_byte_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyNextReceipt;

/* The next observed step after the copied entry successor. Its source byte is
 * still retained only as copied Track 02 provenance. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyNextReceipt
        successor;
    uint16_t next_pc;
    uint32_t next_physical_pc;
    uint16_t original_source_address;
    uint8_t next_source_byte;
    int copied_successor_next_byte_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopySuccessorReceipt;

/* An observed HuC6280 BRA at the copied entry, with its target derived from
 * the two exact source bytes. This remains control flow only. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyReceipt
        entry_copy;
    uint16_t target_pc;
    uint8_t displacement;
    uint16_t original_displacement_address;
    int copied_entry_branch_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchReceipt;

/* An observed instruction fetch at the exact target of the source-bound BRA.
 * The target opcode remains opaque control-flow evidence. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchReceipt
        branch;
    uint16_t target_pc;
    uint32_t target_physical_pc;
    uint8_t target_opcode;
    int copied_entry_branch_target_executed;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetReceipt;

/* The first observed JSR after the executed copied-entry BRA target. Its
 * destination is control-flow evidence only and has no record semantics. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetReceipt
        branch_target;
    uint16_t control_pc;
    uint32_t control_physical_pc;
    uint16_t jsr_target;
    int copied_entry_branch_target_jsr_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrReceipt;

/* A post-BRA JSR that emits an observed CD data-register write followed by a
 * READ(6) and FIFO byte from the corresponding hash-verified Track 02 record.
 * This binds control and media coordinates, not record semantics. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrReceipt
        branch_target_jsr;
    uint8_t cd_register_value;
    uint32_t scsi_generation;
    uint32_t scsi_lba;
    uint32_t track02_record;
    uint16_t source_offset;
    uint8_t source_byte;
    int jsr_cd_register_write_observed;
    int read6_record_source_verified;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdReceipt;

/* An observed main-RAM consumer read of the exact FIFO byte that the post-BRA
 * JSR CD read transferred. This proves only that later original loader code
 * read that byte from its main-RAM destination; it assigns no record,
 * dungeon, level, object, palette, bitmap, or audio semantics. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdReceipt
        jsr_cd;
    uint32_t consumer_generation;
    uint32_t consumer_lba;
    uint32_t consumer_physical_address;
    uint16_t consumer_reader_pc;
    uint32_t consumer_reader_physical_pc;
    uint16_t source_offset;
    uint8_t source_byte;
    int loader_consumer_read_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerReceipt;

/* The first observed main-RAM control transfer after the bound consumer
 * read. Its destination remains opaque control-flow evidence only; no
 * routine, record, level, object, palette, bitmap, or rendering meaning is
 * claimed. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerReceipt
        consumer;
    uint16_t control_pc;
    uint32_t control_physical_pc;
    uint16_t control_target;
    int consumer_control_transfer_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReceipt;

/* The observed execution entry of the control transfer that followed the
 * bound consumer read. This proves the control target was actually fetched
 * in main RAM; it assigns no routine, record, level, object, palette,
 * bitmap, or rendering semantics. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReceipt
        control;
    uint16_t entry_pc;
    uint32_t entry_physical_pc;
    uint8_t entry_opcode;
    int consumer_control_entry_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryReceipt;

/* The next observed main-RAM instruction after that control entry. This is
 * execution ordering only; no opcode, routine, record, level, object,
 * palette, bitmap, or rendering semantics are claimed. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryReceipt
        entry;
    uint16_t next_pc;
    uint32_t next_physical_pc;
    uint8_t next_opcode;
    int consumer_control_entry_next_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryNextReceipt;

/* The bounded return of that control routine: exactly one main-RAM RTS whose
 * linked post-RTS resume lands at the exact control call return address.
 * Other routines' RTS/post-RTS rows remain opaque. This proves a bounded
 * control-routine return only, not a record, consumer, level, object,
 * palette, bitmap, or rendering grammar. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryNextReceipt
        next;
    uint16_t return_instruction_pc;
    uint32_t return_instruction_physical_pc;
    uint16_t post_return_pc;
    uint32_t post_return_physical_pc;
    uint8_t post_return_opcode;
    int consumer_control_return_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnReceipt;

/* An observed main-RAM consumer read of the FIFO byte adjacent to the first
 * bound consumer byte, observed after the bounded control return resumed the
 * loader path. This proves only that the resumed loader read the next
 * source-adjacent byte from its main-RAM destination; it assigns no record,
 * dungeon, level, object, palette, bitmap, or audio semantics. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnReceipt
        control_return;
    uint32_t consumer_generation;
    uint32_t consumer_lba;
    uint32_t track02_record;
    uint32_t consumer_physical_address;
    uint16_t consumer_reader_pc;
    uint32_t consumer_reader_physical_pc;
    uint16_t source_offset;
    uint8_t source_byte;
    int resumed_loader_consumer_read_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerReceipt;

/* The first observed main-RAM control transfer after the resumed consumer
 * read. Its destination remains opaque control-flow evidence only; no
 * routine, record, level, object, palette, bitmap, or rendering meaning is
 * claimed. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerReceipt
        consumer;
    uint16_t control_pc;
    uint32_t control_physical_pc;
    uint16_t control_target;
    int resumed_consumer_control_transfer_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReceipt;

/* The observed execution entry of that resumed control transfer. This proves
 * the resumed control target was actually fetched in main RAM; it assigns no
 * routine, record, level, object, palette, bitmap, or rendering semantics. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReceipt
        control;
    uint16_t entry_pc;
    uint32_t entry_physical_pc;
    uint8_t entry_opcode;
    int resumed_control_entry_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryReceipt;

/* The next observed main-RAM instruction after that resumed control entry.
 * This is execution ordering only; no opcode, routine, record, level,
 * object, palette, bitmap, or rendering semantics are claimed. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryReceipt
        entry;
    uint16_t next_pc;
    uint32_t next_physical_pc;
    uint8_t next_opcode;
    int resumed_control_entry_next_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryNextReceipt;

/* The bounded return of that resumed control routine: exactly one main-RAM
 * RTS whose linked post-RTS resume lands at the exact resumed control call
 * return address. Other routines' RTS/post-RTS rows remain opaque. This
 * proves a bounded control-routine return only, not a record, consumer,
 * level, object, palette, bitmap, or rendering grammar. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryNextReceipt
        next;
    uint16_t return_instruction_pc;
    uint32_t return_instruction_physical_pc;
    uint16_t post_return_pc;
    uint32_t post_return_physical_pc;
    uint8_t post_return_opcode;
    int resumed_control_return_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnReceipt;

/* An observed main-RAM consumer read of the FIFO byte two positions after
 * the first bound consumer byte, observed after the resumed control
 * routine's bounded return resumed the loader path again. This proves only
 * that the twice-resumed loader read the next source-adjacent byte from its
 * main-RAM destination; it assigns no record, dungeon, level, object,
 * palette, bitmap, or audio semantics. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnReceipt
        control_return;
    uint32_t consumer_generation;
    uint32_t consumer_lba;
    uint32_t track02_record;
    uint32_t consumer_physical_address;
    uint16_t consumer_reader_pc;
    uint32_t consumer_reader_physical_pc;
    uint16_t source_offset;
    uint8_t source_byte;
    int twice_resumed_loader_consumer_read_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerReceipt;

#define THERON_V1_RAW_LOADER_LOOP_CONTINUATION_ITERATIONS 2u

/* One generalized per-byte consume/dispatch iteration of the loader loop: an
 * opaque main-RAM control transfer after the previous consumer read, the
 * adjacent call-entry and next-instruction rows proving the target window was
 * actually fetched in main RAM, exactly one bounded RTS whose post-RTS row
 * resumes at the exact call return address, and the consumer read of the next
 * source-adjacent FIFO byte joined to its own media-re-verified receipt row.
 * The consumer reader PC must equal the resumed return address, so the loop
 * back-edge is explicit: the resumed loader path itself performs the next
 * read. All fields remain byte/control-flow provenance only. */
typedef struct {
    uint16_t control_pc;
    uint32_t control_physical_pc;
    uint16_t control_target;
    uint16_t entry_pc;
    uint32_t entry_physical_pc;
    uint8_t entry_opcode;
    uint16_t next_pc;
    uint32_t next_physical_pc;
    uint8_t next_opcode;
    uint16_t return_instruction_pc;
    uint32_t return_instruction_physical_pc;
    uint16_t post_return_pc;
    uint32_t post_return_physical_pc;
    uint8_t post_return_opcode;
    uint32_t track02_record;
    uint16_t source_offset;
    uint8_t source_byte;
    uint32_t consumer_physical_address;
    uint16_t consumer_reader_pc;
    uint32_t consumer_reader_physical_pc;
} Theron_V1RawLoaderTraceLoopContinuationIterationReceipt;

/* The generalized loop continuation: the loader's per-byte consume/dispatch
 * pattern bound for a fixed number of further iterations after the
 * twice-resumed consumer read. Every iteration requires the full
 * control-transfer, fetched-window, bounded-return, and joined consumer
 * sequence; a missing iteration, out-of-order observation, off-target or
 * duplicated resume, media-mismatched receipt byte, different-byte consumer,
 * out-of-order sequence, different transfer or destination, non-main-RAM
 * reader, or a reader that is not the resumed path fails closed. This is
 * still byte- and control-flow provenance only: no record, routine ABI,
 * level, object, palette, bitmap, or rendering semantics are proven, and
 * where the loop terminates or dispatches into a record consumer remains
 * unproven. */
typedef struct {
    int valid;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerReceipt
        consumer;
    uint32_t consumer_generation;
    uint32_t consumer_lba;
    uint16_t first_source_offset;
    Theron_V1RawLoaderTraceLoopContinuationIterationReceipt
        iterations[THERON_V1_RAW_LOADER_LOOP_CONTINUATION_ITERATIONS];
    int loop_continuation_proven;
    int level_or_object_semantics_proven;
} Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerLoopContinuationReceipt;

#define THERON_V1_RAW_LOADER_INITIAL_ENVELOPE_HEADER_BYTES 12u

/* A contiguous, source-owned prefix of the initial envelope observed through
 * one game-owned CD-to-RAM chain. The bytes remain opaque; in particular this
 * receipt does not interpret dimensions, extension words, or any record
 * grammar. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t track02_record;
    size_t raw_sector;
    unsigned int dispatch_sequence;
    unsigned int scsi_generation;
    unsigned int scsi_lba;
    unsigned int scsi_sector_count;
    uint8_t bytes[THERON_V1_RAW_LOADER_INITIAL_ENVELOPE_HEADER_BYTES];
    uint32_t bytes_hash;
    int contiguous_capture_chain_verified;
    int header_semantics_proven;
} Theron_V1RawLoaderTraceInitialEnvelopeHeaderReceipt;

/* Narrow composition of two independently source-locked facts: an ordered,
 * complete-sector later $e009 receipt and the one authenticated initial-level
 * envelope. It accepts only an observed one-sector read of record 0x0b52,
 * the record containing that envelope in both known raw Track 02 variants.
 * This proves a loader/CD admission for the existing AKUTUBA level-0
 * route; it does not prove a destination RAM address, a game transition,
 * object-tail ownership, bitmap, or palette behavior. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t observed_track02_record;
    uint16_t descriptor_selector;
    size_t descriptor_selector_ordinal;
    uint16_t descriptor_word0;
    uint16_t descriptor_word1;
    uint32_t descriptor_record_user_data_hash;
    int descriptor_row_media_bound;
    int descriptor_semantics_proven;
    size_t descriptor_source_raw_offset;
    size_t descriptor_source_bytes;
    uint32_t descriptor_source_hash;
    int descriptor_source_bytes_proven;
    size_t descriptor_selector_occurrence_count;
    size_t descriptor_selector_first_ordinal;
    size_t descriptor_selector_last_ordinal;
    uint32_t descriptor_selector_row_hash;
    int descriptor_selector_aliases_proven;
    int coalesced_loader_cd_receipt_proven;
    int initial_level_record_proven;
    int complete_initial_level_envelope_proven;
    /* The current original stage-two disassembly proves that execution jumps
     * to the complete $3800 sector after this read. It does not prove that a
     * level-shaped subrange or following bytes are a level/object record. */
    int initial_level_semantics_proven;
    /* The exact post-$e009 one-sector local-RAM witness that authorizes this
     * admission. It remains opaque loader/media provenance, not a payload
     * grammar or a dungeon/object/visual claim. */
    size_t complete_payload_bytes;
    uint32_t complete_payload_checksum;
    int complete_payload_witness_proven;
    /* The source-locked `$e009` sector witness must also pass the narrow
     * loader-intake boundary before a consumer may treat it as an initial
     * level handoff. This remains an opaque transfer fact: intake explicitly
     * does not decode the payload or admit its semantics. */
    Theron_V1Track02LoaderIntakeReceipt loader_intake;
    /* Atomically copied from the same full, rehashed user-data sector as the
     * intake receipt. It is opaque runtime input, never a decoded level. */
    Theron_V1Track02LoaderPayloadReceipt loader_payload;
    Theron_V1Track02LoaderLevelEnvelopeReceipt loader_level_envelope;
    /* Byte-faithful continuation of the same authenticated sector. It remains
     * opaque until an original loader consumer proves object semantics. */
    Theron_V1Track02LoaderPostEnvelopeReceipt loader_post_envelope;
    Theron_V1Track02LoaderSemanticGateReceipt loader_semantic_gate;
    Theron_Track02InitialLevelObjectBoundaryReceipt initial_level_boundary;
    /* Observational only until a game-owned consumer proves level semantics. */
    Theron_Track02InitialLevelLoaderRoute initial_level_route;
    int object_tail_semantics_proven;
    int fallback_visuals_allowed;
    /* Populated only after the same manifest that admitted the Track 02 file
     * has also revalidated its System Card and coalesced Mednafen trace. */
    int capture_manifest_bound;
    char capture_manifest_system_card_md5[33];
    char capture_manifest_trace_md5[33];
    uint32_t capture_manifest_binding_hash;
    uint32_t receipt_hash;
} Theron_V1RawLoaderTraceInitialLevelHandoffReceipt;

/* Parses a provenance-marked instrumented Mednafen trace.  It validates the
 * existing dynamic CD_READ/IRQ2 gate first, then requires the captured
 * CD_READ, $3800 destination span, and IRQ2 controller state in their
 * observed order before recording VCE stores and completed HuC6260
 * colour-table words. Completed VCE words establish hardware output order,
 * not Track 02 source-byte provenance. */
int theron_v1_raw_loader_trace_ingest_mednafen_capture(
    const char *capture,
    const char *track02_md5,
    Theron_V1RawLoaderTraceReceipt *out);

/* Bounded file wrapper for an explicit trace path. */
int theron_v1_raw_loader_trace_import_mednafen_capture_file(
    const char *path,
    const char *track02_md5,
    Theron_V1RawLoaderTraceReceipt *out);

/* Resolve an observed System Card SCSI LBA/offset byte to the hash-verified
 * Track 02 payload layout. Raw BIN images admit only MODE1 user-data bytes;
 * ISO images use direct 2048-byte user-data offsets. */
int theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    uint32_t source_lba,
    uint32_t source_offset,
    uint32_t *out_track02_record,
    uint8_t *out_byte);

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

/* Consumes exactly one coalesced original Mednafen transcript and a
 * hash-verified Track 02 image identity. The transcript must retain the
 * authenticated Stage 2 $4090->$4093 row before the later $e009 dispatch,
 * a raw-sector fingerprint, its matching return, and one observed raw caller
 * control edge from that return target. The call row must also retain the
 * observed JSR opcode and e009 target. */
int theron_v1_raw_loader_trace_bind_coalesced_later_e009_raw_sector(
    const char *capture,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceCoalescedLaterReceipt *out);

/* Accepts exactly one source-marked Mednafen CD transcript containing the
 * bounded game-owned `$3840` -> `$e009` call, its seven-byte READ(6) CDB,
 * an original FIFO-to-main-RAM observation, and a game-owned read of the same
 * RAM cell. The known US CUE coordinate is verified as `raw = LBA - 3009`.
 * No record grammar is inferred. */
int theron_v1_raw_loader_trace_bind_game_owned_fifo_payload(
    const char *capture,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceGamePayloadReceipt *out);

/* Bounded file wrapper for one staged original consumer transcript.  The
 * imported receipt is still byte provenance only: no object, level, bitmap,
 * palette, or runtime visual semantics are inferred from the consumer. */
int theron_v1_raw_loader_trace_import_game_owned_fifo_payload_file(
    const char *path,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceGamePayloadReceipt *out);

/* Correlates an admitted game-RAM payload byte with the source-locked Hall of
 * Records initial envelope. It compares physical raw-sector coordinates, not
 * descriptor-relative record numbers, so Track 02 INDEX 01 offsets cannot be
 * silently confused with file-sector offsets. */
int theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope(
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payload,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialEnvelopeByteReceipt *out);

/* Correlates one admitted game-RAM payload byte with the bounded continuation
 * immediately following the initial envelope. This is evidence of a source
 * consumer, not object-table or level semantics. */
int theron_v1_raw_loader_trace_correlate_game_payload_initial_post_envelope(
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payload,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeByteReceipt *out);

/* Requires a complete ordered capture of the first continuation bytes from
 * one CD dispatch. It records no object-table grammar. */
int theron_v1_raw_loader_trace_correlate_game_payload_initial_post_envelope_prefix(
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payloads,
    size_t payload_count,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopePrefixReceipt *out);

/* Binds one provenance-marked original Mednafen main-RAM-loader `TII` row to
 * the beginning of the authenticated post-envelope continuation. The row may
 * establish byte movement only; it never promotes destination semantics. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_tii_transfer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt *out);

/* Bounded file wrapper for one explicit main-RAM-loader capture sidecar. */
int theron_v1_raw_loader_trace_import_initial_post_envelope_tii_transfer_file(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *path,
    Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt *out);

/* Requires an original main-RAM-loader `JSR` after the authenticated TII
 * whose target is exactly that transfer's destination, followed by one RTS
 * inside that copied span and the observed main-RAM instruction at the JSR
 * return address. This is control-flow provenance only. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_execution(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeExecutionReceipt *out);

/* Requires the captured post-RTS instruction itself to be a JSR and binds
 * its exact target from the adjacent original loader trace row. This does not
 * identify the called routine or any payload semantics. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallReceipt *out);

/* Requires an observed main-RAM RTS after the bound post-return routine call
 * and the resulting original trace row at that JSR's return address. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call_termination(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallTerminationReceipt *out);

/* Binds the first subsequent original main-RAM JSR after the authenticated
 * post-return caller resumed. It does not identify the target's semantics. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallReceipt *out);

/* Requires the observed next-caller JSR to execute at its target in main RAM.
 * The target instruction is retained without decoding its routine or data. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call_entry(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallEntryReceipt *out);

/* Requires the next observed original main-RAM instruction after the bound
 * caller-next routine entry. No opcode or data semantics are promoted. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_entry_next(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextEntryNextReceipt *out);

/* Requires the successor to execute TII and to copy only a bounded interval
 * of the already source-bound Track 02 continuation. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferReceipt *out);

/* Requires the first observed main-RAM JSR after the admitted TII to call its
 * copied destination. This proves transfer-to-execution only. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallReceipt *out);

/* Requires the original trace to execute the bound TII-destination JSR at
 * its exact target in main RAM. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryReceipt *out);

/* Requires the observed destination entry opcode to equal the first byte of
 * the exact Track 02-derived interval copied by the bound TII. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyReceipt *out);

/* Requires the next observed instruction after the copied destination entry
 * to remain inside that copy span and match its original Track 02 byte. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy_next(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyNextReceipt *out);

/* Requires the next trace step after the copied entry successor to remain in
 * the same copied span and retain its exact original Track 02 source byte. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy_successor(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopySuccessorReceipt *out);

/* Requires a traced HuC6280 BRA at the copied entry whose displacement and
 * target match the exact Track 02-derived copy bytes. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchReceipt *out);

/* Requires the emulator to subsequently fetch the exact target of the
 * source-bound copied-entry BRA. The fetched opcode is intentionally opaque. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetReceipt *out);

/* Requires a source-observed JSR after the executed copied-entry BRA target.
 * The JSR destination stays opaque until independent CD/record evidence exists. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrReceipt *out);

/* Requires a post-BRA JSR's observed CD data-register write to precede one
 * source-verified READ(6)/FIFO Track 02 sector receipt. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdReceipt *out);

/* Requires a later main-RAM consumer read of the exact FIFO byte joined to
 * the post-BRA JSR CD read, at the same transfer destination and with the
 * same observed value. The read remains an opaque byte observation. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerReceipt *out);

/* Requires the first observed main-RAM control transfer after the bound
 * consumer read. The transfer destination stays opaque until independent
 * record/consumer evidence exists. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReceipt *out);

/* Requires an observed main-RAM call-entry row proving the control transfer
 * target was actually executed. The entry stays an opaque execution fact. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryReceipt *out);

/* Requires the next observed main-RAM instruction after that control entry.
 * Execution ordering only; no opcode or data semantics are claimed. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryNextReceipt *out);

/* Requires exactly one main-RAM RTS whose linked post-RTS row resumes at the
 * exact control call return address. Other routines' returns stay opaque. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnReceipt *out);

/* Requires a resumed main-RAM consumer read of the FIFO byte adjacent to the
 * first bound consumer byte, observed after the bounded control return and
 * joined to its own source-verified FIFO receipt row. The read remains an
 * opaque byte observation. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerReceipt *out);

/* Requires the first observed main-RAM control transfer after the resumed
 * consumer read. The transfer destination stays opaque until independent
 * record/consumer evidence exists. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReceipt *out);

/* Requires an observed main-RAM call-entry row proving the resumed control
 * transfer target was actually executed. The entry stays an opaque
 * execution fact. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryReceipt *out);

/* Requires the next observed main-RAM instruction after that resumed
 * control entry. Execution ordering only; no opcode or data semantics are
 * claimed. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryNextReceipt *out);

/* Requires exactly one main-RAM RTS whose linked post-RTS row resumes at
 * the exact resumed control call return address. Other routines' returns
 * stay opaque. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnReceipt *out);

/* Requires a twice-resumed main-RAM consumer read of the FIFO byte two
 * positions after the first bound consumer byte, observed after the resumed
 * control routine's bounded return and joined to its own source-verified
 * FIFO receipt row. The read remains an opaque byte observation. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerReceipt *out);

/* Requires the loader's per-byte consume/dispatch loop to continue for a
 * fixed number of further iterations after the twice-resumed consumer read.
 * Each iteration binds an opaque main-RAM control transfer, its adjacent
 * fetched-entry window, exactly one bounded return resuming at the call
 * return address, and the consumer read of the next source-adjacent FIFO
 * byte joined to its media-re-verified receipt, with the consumer reader
 * equal to the resumed return address. The loop's termination or dispatch
 * into a record consumer stays unproven; no semantics are claimed. */
int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerLoopContinuationReceipt *out);

/* Requires a complete, ordered 12-byte game-RAM capture of the initial
 * envelope prefix from one observed CD dispatch. It is a source/capture
 * receipt only and never decodes the header. */
int theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope_header(
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payloads,
    size_t payload_count,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialEnvelopeHeaderReceipt *out);

/* Promotes the existing source-locked initial-level loader route only after
 * a coalesced original loader/CD receipt selects its exact one-sector record.
 * The trace receipt is not a substitute for a gameplay-transition capture,
 * and the returned route retains the existing opaque
 * object-tail and no-fallback restrictions. */
int theron_v1_raw_loader_trace_bind_initial_level_handoff(
    const Theron_V1RawLoaderTraceCoalescedLaterReceipt *coalesced_receipt,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *out);

/* Revalidates the immutable fields carried by an initial-level admission
 * receipt before a runtime consumer uses it. In particular, it requires the
 * full 2048-byte local-RAM witness and checks that the receipt hash covers
 * that witness. It does not inspect or interpret the payload bytes. */
int theron_v1_raw_loader_trace_initial_level_handoff_is_complete(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *receipt);

/* Adds the three hash-verified capture artifacts to an already source-bound
 * `$e009` receipt. The handoff remains opaque loader/media provenance. */
int theron_v1_raw_loader_trace_bind_capture_manifest_to_initial_level_handoff(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *source,
    const Theron_V1CaptureManifest *manifest,
    const char *track02_path,
    const char *track02_md5,
    const char *system_card_path,
    const char *system_card_md5,
    const char *trace_path,
    const char *trace_md5,
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *out);

int theron_v1_raw_loader_trace_manifest_initial_level_handoff_is_complete(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *receipt);

/* Validates the explicit V2 capture-manifest identity for an ordered
 * Mednafen loader transcript before it is parsed. The caller must rehash
 * these files before passing their measured MD5 values. This binds only the
 * original raw Track 02, System Card 3.0, and trace artifacts; it assigns no
 * payload, dungeon, object, graphics, palette, or transition meaning to the
 * receipt. */
int theron_v1_raw_loader_trace_capture_manifest_matches(
    const Theron_V1CaptureManifest *manifest,
    const char *track02_path,
    const char *track02_md5,
    const char *system_card_path,
    const char *system_card_md5,
    const char *trace_path,
    const char *trace_md5);

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
