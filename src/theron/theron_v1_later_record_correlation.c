#include "theron_v1_later_record_correlation.h"

#include <string.h>

#define THERON_V1_RAW_SECTOR_BYTES 2352u

int theron_v1_later_record_correlation_from_manifest(
    const Theron_V1Stage3ManifestEvidence *manifest,
    size_t raw_track02_size,
    Theron_V1LaterRecordCorrelation *out_correlation) {
    size_t index;
    uint16_t selector;

    if (!out_correlation) return 0;
    memset(out_correlation, 0, sizeof(*out_correlation));
    if (!manifest || !manifest->valid ||
        (manifest->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         manifest->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        manifest->descriptor_count !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT ||
        raw_track02_size < THERON_V1_RAW_SECTOR_BYTES) {
        return 0;
    }

    selector = manifest->first_descriptor.word2;
    if (selector == 0u || manifest->track02_record < selector) {
        return 0;
    }

    out_correlation->valid = 1;
    out_correlation->variant = manifest->variant;
    out_correlation->stage3_track02_record = manifest->track02_record;
    out_correlation->first_descriptor_selector = selector;
    out_correlation->derived_record_base = manifest->track02_record - selector;
    out_correlation->self_resolved_record =
        out_correlation->derived_record_base + selector;
    out_correlation->raw_sector_count =
        raw_track02_size / THERON_V1_RAW_SECTOR_BYTES;
    out_correlation->self_reference_proven =
        out_correlation->self_resolved_record == manifest->track02_record;
    out_correlation->self_resolved_record_in_bounds =
        out_correlation->self_resolved_record < out_correlation->raw_sector_count;
    for (index = 0u; index < manifest->descriptor_count; ++index) {
        if (manifest->descriptors[index].word2 != 0u) {
            ++out_correlation->nonzero_selector_count;
        }
    }
    return out_correlation->self_reference_proven &&
        out_correlation->self_resolved_record_in_bounds;
}

int theron_v1_later_record_correlation_compare(
    const Theron_V1LaterRecordCorrelation *first,
    const Theron_V1LaterRecordCorrelation *second,
    Theron_V1LaterRecordCorrelationComparison *out_comparison) {

    if (!out_comparison) return 0;
    memset(out_comparison, 0, sizeof(*out_comparison));
    if (!first || !second || !first->valid || !second->valid ||
        first->variant == second->variant ||
        first->first_descriptor_selector != second->first_descriptor_selector ||
        first->derived_record_base > second->derived_record_base) {
        return 0;
    }

    out_comparison->valid = 1;
    out_comparison->first_base = first->derived_record_base;
    out_comparison->second_base = second->derived_record_base;
    out_comparison->base_delta =
        second->derived_record_base - first->derived_record_base;
    out_comparison->shared_first_selector = first->first_descriptor_selector;
    out_comparison->both_self_references_proven =
        first->self_reference_proven && second->self_reference_proven;
    return 1;
}
