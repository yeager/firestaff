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

#endif /* THERON_V1_LATER_RECORD_CORRELATION_H */
