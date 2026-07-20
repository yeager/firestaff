#ifndef THERON_V1_LATER_RECORD_CORRELATION_H
#define THERON_V1_LATER_RECORD_CORRELATION_H

#include "theron_v1_stage3_manifest_evidence.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Original-media coordinate evidence for the first stage-three descriptor.
 * The descriptor's third word is retained as an opaque selector.  On both
 * authenticated raw variants, adding its derived base resolves precisely to
 * the already-proven stage-three sector.  This establishes only a bounded
 * record-coordinate correlation, never an object, level, palette, graphics,
 * or command interpretation for any descriptor word.
 */

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint16_t first_descriptor_selector;
    uint32_t derived_record_base;
    uint32_t self_resolved_record;
    size_t raw_sector_count;
    int self_reference_proven;
    int self_resolved_record_in_bounds;
    size_t nonzero_selector_count;
    /* All non-zero stage-three selectors are evaluated against the same
     * authenticated base. These are record-coordinate facts only: a resolved
     * selector does not identify a CD command, object table, bitmap, or
     * dungeon level. */
    size_t resolved_selector_count;
    size_t out_of_bounds_selector_count;
    uint32_t resolved_selector_hash;
} Theron_V1LaterRecordCorrelation;

typedef struct {
    int valid;
    uint32_t first_base;
    uint32_t second_base;
    uint32_t base_delta;
    uint16_t shared_first_selector;
    int both_self_references_proven;
} Theron_V1LaterRecordCorrelationComparison;

/* One source-owned descriptor-to-sector receipt.  The stage-three loader
 * manifest gives an opaque 16-bit selector; the established base resolves it
 * to one physical MODE1 Track 02 sector.  This keeps the original descriptor
 * words and the exact user-data identity together without assigning a level,
 * object, tile, palette, bitmap, or command grammar to either the descriptor
 * or the sector payload. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    size_t descriptor_ordinal;
    Theron_V1Stage3ManifestWordTriple descriptor;
    uint32_t derived_record_base;
    uint32_t resolved_track02_record;
    size_t raw_sector;
    size_t raw_offset;
    size_t user_data_offset;
    size_t user_data_bytes;
    uint32_t user_data_hash;
    /* Exact six-byte big-endian source row inside the authenticated Stage-3
     * loader sector. This records the physical descriptor-to-record transfer
     * boundary only; it does not classify the row or target payload. */
    size_t descriptor_source_raw_offset;
    size_t descriptor_source_bytes;
    uint32_t descriptor_source_hash;
    int descriptor_source_bytes_proven;
    /* Full table relationship for this selector. Multiple raw descriptor rows
     * can resolve to one physical record; these ordinal facts retain that
     * aliasing without assigning a meaning to the selector or row words. */
    size_t selector_occurrence_count;
    size_t selector_first_ordinal;
    size_t selector_last_ordinal;
    uint32_t selector_row_hash;
    int selector_aliases_proven;
    int record_coordinate_proven;
    int mode1_user_data_proven;
    int descriptor_semantics_proven;
} Theron_V1Stage3DescriptorRecordBoundary;

/* Maximum referenced-record span the topology receipt can carry.  A corrupt
 * or hostile descriptor table could name selectors spanning the full 16-bit
 * range; the binder fails closed above this capacity instead. */
#define THERON_V1_STAGE3_RECORD_SPAN_SLOT_CAPACITY 4096u
#define THERON_V1_STAGE3_RECORD_SPAN_BITMAP_BYTES \
    (THERON_V1_STAGE3_RECORD_SPAN_SLOT_CAPACITY / 8u)

/* Referenced-record span topology derived from a proven descriptor corpus.
 * The receipt carries the exact set of distinct records the authenticated
 * stage-three table references, packed as one bit per span slot, together
 * with aggregate counts and a per-slot flag hash.  This is a record
 * membership boundary only: an unreferenced slot is not proven absent from
 * any other loader path, and neither membership nor absence assigns a
 * level, object, tile, palette, bitmap, or command meaning. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint32_t derived_record_base;
    size_t referenced_record_count;
    uint32_t min_referenced_record;
    uint32_t max_referenced_record;
    size_t span_record_slots;
    size_t unreferenced_slot_count;
    /* FNV-1a over one 0/1 flag byte per span slot, in min..max order. */
    uint32_t slot_flag_hash;
    uint8_t referenced_slot_bits[THERON_V1_STAGE3_RECORD_SPAN_BITMAP_BYTES];
    int span_topology_proven;
    int descriptor_semantics_proven;
} Theron_V1Stage3DescriptorRecordSpan;

/* Full-corpus descriptor-to-media correlation.  Every non-zero stage-three
 * selector is resolved against the same authenticated base and its resolved
 * MODE1 sector is re-verified against the hash-gated Track 02 bytes: sync,
 * mode, and a chained user-data identity over every resolved record in
 * descriptor order.  This proves the complete loader record table's physical
 * media span only; no descriptor word, resolved record, or span boundary is
 * assigned a level, object, tile, palette, bitmap, or command meaning. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint32_t derived_record_base;
    size_t descriptor_count;
    size_t nonzero_selector_count;
    size_t zero_selector_count;
    size_t resolved_record_count;
    /* Distinct physical records across all resolved selectors; aliased
     * selectors repeat a record and do not raise this count. */
    size_t distinct_record_count;
    uint32_t min_resolved_record;
    uint32_t max_resolved_record;
    /* Chained FNV-1a over (descriptor ordinal, resolved record) pairs and
     * over every resolved record's 2048 MODE1 user bytes, both in
     * descriptor-table order with aliases included. */
    uint32_t resolved_record_hash;
    uint32_t resolved_user_data_hash;
    int corpus_media_proven;
    int descriptor_semantics_proven;
} Theron_V1Stage3DescriptorCorpusMediaCorrelation;

int theron_v1_later_record_correlation_from_manifest(
    const Theron_V1Stage3ManifestEvidence *manifest,
    size_t raw_track02_size,
    Theron_V1LaterRecordCorrelation *out_correlation);

int theron_v1_later_record_correlation_compare(
    const Theron_V1LaterRecordCorrelation *first,
    const Theron_V1LaterRecordCorrelation *second,
    Theron_V1LaterRecordCorrelationComparison *out_comparison);

/* Resolves one non-zero manifest selector against its authenticated Track 02
 * bytes.  The input must be raw 2352-byte MODE1 data; unknown variants,
 * malformed sectors, absent selectors, and changed bytes reject. */
int theron_v1_stage3_descriptor_record_boundary_from_manifest(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_V1Stage3ManifestEvidence *manifest,
    size_t descriptor_ordinal,
    Theron_V1Stage3DescriptorRecordBoundary *out_boundary);

/* Resolves the complete stage-three descriptor table against its
 * authenticated Track 02 bytes.  Every non-zero selector must resolve inside
 * the media and name a well-formed MODE1 sector; any out-of-bounds selector,
 * malformed envelope, or changed byte fails closed with a zeroed receipt.
 * The receipt carries only record-coordinate spans and identity hashes; it
 * never assigns a grammar to any descriptor word or resolved payload. */
int theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_V1Stage3ManifestEvidence *manifest,
    Theron_V1Stage3DescriptorCorpusMediaCorrelation *out_correlation);

/* Derives the referenced-record span topology from a proven corpus and the
 * same authenticated manifest.  The manifest is cross-checked against the
 * corpus aggregates (variant, stage-three record, derived base, non-zero
 * selector count, distinct count, min/max); a mismatch, an over-capacity
 * span, or an unproven corpus fails closed with a zeroed receipt.  The
 * derivation re-resolves selectors without re-reading media; full byte
 * re-verification remains the corpus binder's role. */
int theron_v1_stage3_descriptor_record_span_from_corpus(
    const Theron_V1Stage3ManifestEvidence *manifest,
    const Theron_V1Stage3DescriptorCorpusMediaCorrelation *corpus,
    Theron_V1Stage3DescriptorRecordSpan *out_span);

/* Membership query over a proven span: returns 1 only when the record is a
 * referenced record of the authenticated stage-three table, 0 for
 * unreferenced slots, out-of-span records, and invalid spans. */
int theron_v1_stage3_descriptor_record_span_contains(
    const Theron_V1Stage3DescriptorRecordSpan *span,
    uint32_t track02_record);

#endif /* THERON_V1_LATER_RECORD_CORRELATION_H */
