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

static uint32_t theron_v1_later_record_fnv1a_u16(uint32_t hash,
                                                  uint16_t value) {
    hash ^= (uint8_t)value;
    hash *= 16777619u;
    hash ^= (uint8_t)(value >> 8u);
    return hash * 16777619u;
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

static uint32_t theron_v1_later_record_fnv1a_chain(uint32_t hash,
                                                    const uint8_t *bytes,
                                                    size_t byte_count) {
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
    size_t index;
    size_t descriptor_source_offset;
    const uint8_t *descriptor_source;
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

    if (manifest->raw_sector != manifest->track02_record ||
        manifest->raw_sector > SIZE_MAX / THERON_V1_RAW_SECTOR_BYTES ||
        manifest->raw_offset != manifest->raw_sector *
            THERON_V1_RAW_SECTOR_BYTES ||
        manifest->user_data_offset != manifest->raw_offset +
            THERON_V1_MODE1_USER_DATA_OFFSET) {
        return 0;
    }

    descriptor = &manifest->descriptors[descriptor_ordinal];
    if (descriptor->word2 == 0u ||
        (uint32_t)descriptor->word2 >
            UINT32_MAX - correlation.derived_record_base) {
        return 0;
    }
    if (descriptor_ordinal > (SIZE_MAX - 4u) / 6u ||
        manifest->user_data_offset > track02_size ||
        4u + descriptor_ordinal * 6u >
            track02_size - manifest->user_data_offset ||
        6u > track02_size - (manifest->user_data_offset + 4u +
                              descriptor_ordinal * 6u)) {
        return 0;
    }
    descriptor_source_offset = manifest->user_data_offset + 4u +
        descriptor_ordinal * 6u;
    descriptor_source = track02_data + descriptor_source_offset;
    if (((uint16_t)descriptor_source[0] << 8u | descriptor_source[1]) !=
            descriptor->word0 ||
        ((uint16_t)descriptor_source[2] << 8u | descriptor_source[3]) !=
            descriptor->word1 ||
        ((uint16_t)descriptor_source[4] << 8u | descriptor_source[5]) !=
            descriptor->word2) {
        return 0;
    }
    resolved_record = correlation.derived_record_base + descriptor->word2;
    if (resolved_record >= correlation.raw_sector_count) {
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
    out_boundary->descriptor_source_raw_offset = descriptor_source_offset;
    out_boundary->descriptor_source_bytes = 6u;
    out_boundary->descriptor_source_hash = theron_v1_later_record_fnv1a_bytes(
        descriptor_source, out_boundary->descriptor_source_bytes);
    out_boundary->descriptor_source_bytes_proven =
        out_boundary->descriptor_source_hash != 0u;
    out_boundary->record_coordinate_proven = 1;
    out_boundary->mode1_user_data_proven = out_boundary->user_data_hash != 0u;
    out_boundary->selector_first_ordinal = manifest->descriptor_count;
    out_boundary->selector_row_hash = 2166136261u;
    for (index = 0u; index < manifest->descriptor_count; ++index) {
        const Theron_V1Stage3ManifestWordTriple *row =
            &manifest->descriptors[index];

        if (row->word2 != descriptor->word2) continue;
        if (out_boundary->selector_first_ordinal == manifest->descriptor_count) {
            out_boundary->selector_first_ordinal = index;
        }
        out_boundary->selector_last_ordinal = index;
        ++out_boundary->selector_occurrence_count;
        out_boundary->selector_row_hash = theron_v1_later_record_fnv1a_u32(
            out_boundary->selector_row_hash, (uint32_t)index);
        out_boundary->selector_row_hash = theron_v1_later_record_fnv1a_u16(
            out_boundary->selector_row_hash, row->word0);
        out_boundary->selector_row_hash = theron_v1_later_record_fnv1a_u16(
            out_boundary->selector_row_hash, row->word1);
        out_boundary->selector_row_hash = theron_v1_later_record_fnv1a_u16(
            out_boundary->selector_row_hash, row->word2);
    }
    out_boundary->selector_aliases_proven =
        out_boundary->selector_occurrence_count != 0u &&
        out_boundary->selector_first_ordinal <= descriptor_ordinal &&
        descriptor_ordinal <= out_boundary->selector_last_ordinal &&
        out_boundary->selector_row_hash != 0u;
    out_boundary->descriptor_semantics_proven = 0;
    if (!out_boundary->mode1_user_data_proven ||
        !out_boundary->descriptor_source_bytes_proven ||
        !out_boundary->selector_aliases_proven) {
        memset(out_boundary, 0, sizeof(*out_boundary));
        return 0;
    }
    return 1;
}

int theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_V1Stage3ManifestEvidence *manifest,
    Theron_V1Stage3DescriptorCorpusMediaCorrelation *out_correlation) {
    Theron_V1LaterRecordCorrelation correlation;
    uint32_t resolved_records[THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT];
    size_t index;
    size_t alias_index;

    if (!out_correlation) return 0;
    memset(out_correlation, 0, sizeof(*out_correlation));
    if (!track02_data || !manifest ||
        track02_size % THERON_V1_RAW_SECTOR_BYTES != 0u ||
        !theron_v1_later_record_correlation_from_manifest(
            manifest, track02_size, &correlation) ||
        !correlation.valid || !correlation.self_reference_proven ||
        !correlation.self_resolved_record_in_bounds) {
        return 0;
    }
    if (manifest->raw_sector != manifest->track02_record ||
        manifest->raw_sector > SIZE_MAX / THERON_V1_RAW_SECTOR_BYTES ||
        manifest->raw_offset != manifest->raw_sector *
            THERON_V1_RAW_SECTOR_BYTES ||
        manifest->user_data_offset != manifest->raw_offset +
            THERON_V1_MODE1_USER_DATA_OFFSET) {
        return 0;
    }

    out_correlation->valid = 1;
    out_correlation->variant = manifest->variant;
    out_correlation->stage3_track02_record = manifest->track02_record;
    out_correlation->derived_record_base = correlation.derived_record_base;
    out_correlation->descriptor_count = manifest->descriptor_count;
    out_correlation->resolved_record_hash = 2166136261u;
    out_correlation->resolved_user_data_hash = 2166136261u;
    for (index = 0u; index < manifest->descriptor_count; ++index) {
        const Theron_V1Stage3ManifestWordTriple *descriptor =
            &manifest->descriptors[index];
        uint32_t resolved_record;
        size_t raw_offset;
        const uint8_t *sector;

        if (descriptor->word2 == 0u) {
            ++out_correlation->zero_selector_count;
            continue;
        }
        ++out_correlation->nonzero_selector_count;
        if ((uint32_t)descriptor->word2 >
            UINT32_MAX - correlation.derived_record_base) {
            memset(out_correlation, 0, sizeof(*out_correlation));
            return 0;
        }
        resolved_record =
            correlation.derived_record_base + descriptor->word2;
        if (resolved_record >= correlation.raw_sector_count) {
            memset(out_correlation, 0, sizeof(*out_correlation));
            return 0;
        }
        raw_offset = (size_t)resolved_record * THERON_V1_RAW_SECTOR_BYTES;
        if (raw_offset > track02_size ||
            THERON_V1_RAW_SECTOR_BYTES > track02_size - raw_offset) {
            memset(out_correlation, 0, sizeof(*out_correlation));
            return 0;
        }
        sector = track02_data + raw_offset;
        if (!theron_v1_later_record_mode1_sector_is_valid(sector)) {
            memset(out_correlation, 0, sizeof(*out_correlation));
            return 0;
        }
        resolved_records[out_correlation->resolved_record_count] =
            resolved_record;
        ++out_correlation->resolved_record_count;
        if (out_correlation->resolved_record_count == 1u ||
            resolved_record < out_correlation->min_resolved_record) {
            out_correlation->min_resolved_record = resolved_record;
        }
        if (resolved_record > out_correlation->max_resolved_record) {
            out_correlation->max_resolved_record = resolved_record;
        }
        out_correlation->resolved_record_hash =
            theron_v1_later_record_fnv1a_u32(
                out_correlation->resolved_record_hash, (uint32_t)index);
        out_correlation->resolved_record_hash =
            theron_v1_later_record_fnv1a_u32(
                out_correlation->resolved_record_hash, resolved_record);
        out_correlation->resolved_user_data_hash =
            theron_v1_later_record_fnv1a_chain(
                out_correlation->resolved_user_data_hash,
                sector + THERON_V1_MODE1_USER_DATA_OFFSET,
                THERON_V1_MODE1_USER_DATA_BYTES);
    }
    out_correlation->distinct_record_count =
        out_correlation->resolved_record_count;
    for (index = 0u; index < out_correlation->resolved_record_count; ++index) {
        for (alias_index = 0u; alias_index < index; ++alias_index) {
            if (resolved_records[alias_index] == resolved_records[index]) {
                --out_correlation->distinct_record_count;
                break;
            }
        }
    }
    out_correlation->corpus_media_proven =
        out_correlation->nonzero_selector_count != 0u &&
        out_correlation->resolved_record_count ==
            out_correlation->nonzero_selector_count &&
        out_correlation->distinct_record_count != 0u &&
        out_correlation->min_resolved_record <=
            out_correlation->max_resolved_record &&
        out_correlation->resolved_record_hash != 0u &&
        out_correlation->resolved_user_data_hash != 0u;
    out_correlation->descriptor_semantics_proven = 0;
    if (!out_correlation->corpus_media_proven) {
        memset(out_correlation, 0, sizeof(*out_correlation));
        return 0;
    }
    return 1;
}

int theron_v1_stage3_descriptor_record_span_from_corpus(
    const Theron_V1Stage3ManifestEvidence *manifest,
    const Theron_V1Stage3DescriptorCorpusMediaCorrelation *corpus,
    Theron_V1Stage3DescriptorRecordSpan *out_span) {
    uint32_t base;
    uint32_t span_slots;
    size_t index;
    size_t nonzero_count = 0u;

    if (!out_span) return 0;
    memset(out_span, 0, sizeof(*out_span));
    if (!manifest || !corpus || !manifest->valid || !corpus->valid ||
        !corpus->corpus_media_proven || corpus->descriptor_semantics_proven ||
        manifest->variant != corpus->variant ||
        manifest->track02_record != corpus->stage3_track02_record ||
        manifest->descriptor_count != corpus->descriptor_count ||
        manifest->descriptor_count !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT ||
        manifest->first_descriptor.word2 == 0u ||
        manifest->track02_record < manifest->first_descriptor.word2) {
        return 0;
    }
    base = manifest->track02_record - manifest->first_descriptor.word2;
    if (base != corpus->derived_record_base ||
        corpus->resolved_record_count != corpus->nonzero_selector_count ||
        corpus->distinct_record_count == 0u ||
        corpus->min_resolved_record > corpus->max_resolved_record) {
        return 0;
    }
    span_slots = corpus->max_resolved_record - corpus->min_resolved_record + 1u;
    if (span_slots == 0u ||
        span_slots > THERON_V1_STAGE3_RECORD_SPAN_SLOT_CAPACITY) {
        return 0;
    }

    out_span->valid = 1;
    out_span->variant = corpus->variant;
    out_span->stage3_track02_record = corpus->stage3_track02_record;
    out_span->derived_record_base = base;
    out_span->min_referenced_record = corpus->min_resolved_record;
    out_span->max_referenced_record = corpus->max_resolved_record;
    out_span->span_record_slots = span_slots;
    out_span->slot_flag_hash = 2166136261u;
    for (index = 0u; index < manifest->descriptor_count; ++index) {
        uint16_t selector = manifest->descriptors[index].word2;
        uint32_t resolved_record;
        uint32_t slot;

        if (selector == 0u) continue;
        ++nonzero_count;
        if ((uint32_t)selector > UINT32_MAX - base) {
            memset(out_span, 0, sizeof(*out_span));
            return 0;
        }
        resolved_record = base + selector;
        if (resolved_record < out_span->min_referenced_record ||
            resolved_record > out_span->max_referenced_record) {
            memset(out_span, 0, sizeof(*out_span));
            return 0;
        }
        slot = resolved_record - out_span->min_referenced_record;
        out_span->referenced_slot_bits[slot / 8u] |=
            (uint8_t)(1u << (slot % 8u));
    }
    if (nonzero_count != corpus->nonzero_selector_count) {
        memset(out_span, 0, sizeof(*out_span));
        return 0;
    }
    for (index = 0u; index < span_slots; ++index) {
        uint8_t flag =
            (uint8_t)((out_span->referenced_slot_bits[index / 8u] >>
                       (index % 8u)) & 1u);

        if (flag != 0u) {
            ++out_span->referenced_record_count;
        }
        out_span->slot_flag_hash ^= flag;
        out_span->slot_flag_hash *= 16777619u;
    }
    if (out_span->referenced_record_count != corpus->distinct_record_count) {
        memset(out_span, 0, sizeof(*out_span));
        return 0;
    }
    out_span->unreferenced_slot_count =
        out_span->span_record_slots - out_span->referenced_record_count;
    out_span->span_topology_proven =
        out_span->referenced_record_count != 0u &&
        out_span->slot_flag_hash != 0u;
    out_span->descriptor_semantics_proven = 0;
    if (!out_span->span_topology_proven) {
        memset(out_span, 0, sizeof(*out_span));
        return 0;
    }
    return 1;
}

int theron_v1_stage3_descriptor_record_span_contains(
    const Theron_V1Stage3DescriptorRecordSpan *span,
    uint32_t track02_record) {
    uint32_t slot;

    if (!span || !span->valid || !span->span_topology_proven ||
        span->span_record_slots == 0u ||
        span->span_record_slots > THERON_V1_STAGE3_RECORD_SPAN_SLOT_CAPACITY ||
        track02_record < span->min_referenced_record ||
        track02_record > span->max_referenced_record) {
        return 0;
    }
    slot = track02_record - span->min_referenced_record;
    return (span->referenced_slot_bits[slot / 8u] >> (slot % 8u)) & 1u;
}
