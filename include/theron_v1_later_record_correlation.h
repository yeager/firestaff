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
} Theron_V1LaterRecordCorrelation;

typedef struct {
    int valid;
    uint32_t first_base;
    uint32_t second_base;
    uint32_t base_delta;
    uint16_t shared_first_selector;
    int both_self_references_proven;
} Theron_V1LaterRecordCorrelationComparison;

int theron_v1_later_record_correlation_from_manifest(
    const Theron_V1Stage3ManifestEvidence *manifest,
    size_t raw_track02_size,
    Theron_V1LaterRecordCorrelation *out_correlation);

int theron_v1_later_record_correlation_compare(
    const Theron_V1LaterRecordCorrelation *first,
    const Theron_V1LaterRecordCorrelation *second,
    Theron_V1LaterRecordCorrelationComparison *out_comparison);

#endif /* THERON_V1_LATER_RECORD_CORRELATION_H */
