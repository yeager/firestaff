#include "theron_v1_later_record_correlation.h"

#include <string.h>

#define THERON_V1_RAW_SECTOR_BYTES 2352u
#define THERON_V1_MODE1_USER_DATA_OFFSET 16u
#define THERON_V1_MODE1_USER_DATA_BYTES 2048u

static uint32_t theron_v1_later_record_fnv1a_u32(uint32_t hash,
                                                  uint32_t value) {
    for (unsigned int shift = 0u; shift < 32u; shift += 8u) {
        hash ^= (uint8_t)(value >> shift);
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t theron_v1_later_record_fnv1a_bytes(const uint8_t *bytes,
                                                    size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t index;

    if (!bytes) return 0u;
    for (index = 0u; index < byte_count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static int theron_v1_later_record_mode1_sector_is_valid(
    const uint8_t *sector) {
    size_t index;

    if (!sector || sector[0] != 0x00u || sector[11] != 0x00u ||
        sector[15] != 0x01u) {
        return 0;
    }
    for (index = 1u; index < 11u; ++index) {
        if (sector[index] != 0xffu) return 0;
    }
    return 1;
}

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
    out_correlation->resolved_selector_hash = 2166136261u;
    for (index = 0u; index < manifest->descriptor_count; ++index) {
        uint32_t resolved_record;

        selector = manifest->descriptors[index].word2;
        if (selector == 0u) continue;
        ++out_correlation->nonzero_selector_count;
        if ((uint32_t)selector > UINT32_MAX -
                                      out_correlation->derived_record_base) {
            ++out_correlation->out_of_bounds_selector_count;
            continue;
        }
        resolved_record = out_correlation->derived_record_base + selector;
        if (resolved_record >= out_correlation->raw_sector_count) {
            ++out_correlation->out_of_bounds_selector_count;
            continue;
        }
        ++out_correlation->resolved_selector_count;
        out_correlation->resolved_selector_hash =
            theron_v1_later_record_fnv1a_u32(
                out_correlation->resolved_selector_hash, (uint32_t)index);
        out_correlation->resolved_selector_hash =
            theron_v1_later_record_fnv1a_u32(
                out_correlation->resolved_selector_hash, resolved_record);
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

int theron_v1_stage3_descriptor_record_boundary_from_manifest(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_V1Stage3ManifestEvidence *manifest,
    size_t descriptor_ordinal,
    Theron_V1Stage3DescriptorRecordBoundary *out_boundary) {
    Theron_V1LaterRecordCorrelation correlation;
    const Theron_V1Stage3ManifestWordTriple *descriptor;
    uint32_t resolved_record;
    size_t raw_offset;
    const uint8_t *sector;

    if (!out_boundary) return 0;
    memset(out_boundary, 0, sizeof(*out_boundary));
    if (!track02_data || !manifest ||
        track02_size % THERON_V1_RAW_SECTOR_BYTES != 0u ||
        !theron_v1_later_record_correlation_from_manifest(
            manifest, track02_size, &correlation) ||
        !correlation.valid || !correlation.self_reference_proven ||
        !correlation.self_resolved_record_in_bounds ||
        descriptor_ordinal >= manifest->descriptor_count) {
        return 0;
    }

    descriptor = &manifest->descriptors[descriptor_ordinal];
    if (descriptor->word2 == 0u ||
        (uint32_t)descriptor->word2 >
            UINT32_MAX - correlation.derived_record_base) {
        return 0;
    }
    resolved_record = correlation.derived_record_base + descriptor->word2;
    if (resolved_record >= correlation.raw_sector_count ||
        (size_t)resolved_record > SIZE_MAX / THERON_V1_RAW_SECTOR_BYTES) {
        return 0;
    }
    raw_offset = (size_t)resolved_record * THERON_V1_RAW_SECTOR_BYTES;
    if (raw_offset > track02_size ||
        THERON_V1_RAW_SECTOR_BYTES > track02_size - raw_offset) {
        return 0;
    }
    sector = track02_data + raw_offset;
    if (!theron_v1_later_record_mode1_sector_is_valid(sector)) return 0;

    out_boundary->valid = 1;
    out_boundary->variant = manifest->variant;
    out_boundary->descriptor_ordinal = descriptor_ordinal;
    out_boundary->descriptor = *descriptor;
    out_boundary->derived_record_base = correlation.derived_record_base;
    out_boundary->resolved_track02_record = resolved_record;
    out_boundary->raw_sector = resolved_record;
    out_boundary->raw_offset = raw_offset;
    out_boundary->user_data_offset = raw_offset +
        THERON_V1_MODE1_USER_DATA_OFFSET;
    out_boundary->user_data_bytes = THERON_V1_MODE1_USER_DATA_BYTES;
    out_boundary->user_data_hash = theron_v1_later_record_fnv1a_bytes(
        sector + THERON_V1_MODE1_USER_DATA_OFFSET,
        THERON_V1_MODE1_USER_DATA_BYTES);
    out_boundary->record_coordinate_proven = 1;
    out_boundary->mode1_user_data_proven = out_boundary->user_data_hash != 0u;
    out_boundary->descriptor_semantics_proven = 0;
    if (!out_boundary->mode1_user_data_proven) {
        memset(out_boundary, 0, sizeof(*out_boundary));
        return 0;
    }
    return 1;
}
