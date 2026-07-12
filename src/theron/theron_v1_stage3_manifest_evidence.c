#include "theron_v1_stage3_manifest_evidence.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8u) | bytes[1]);
}

static uint32_t fnv1a32(const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t index;

    for (index = 0u; index < byte_count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

int theron_v1_stage3_manifest_evidence_from_payload(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1Stage3ManifestEvidence *out_evidence) {
    const uint8_t *manifest;
    size_t index;
    size_t previous_word2 = 0u;

    if (!out_evidence) return 0;
    memset(out_evidence, 0, sizeof(*out_evidence));
    if (!track02_data || !payload || !payload->valid ||
        payload->user_data_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        payload->manifest_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES ||
        payload->manifest_entry_count !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT ||
        payload->user_data_offset > track02_size ||
        payload->manifest_bytes > track02_size - payload->user_data_offset) {
        return 0;
    }

    manifest = track02_data + payload->user_data_offset;
    out_evidence->valid = 1;
    out_evidence->variant = payload->variant;
    out_evidence->track02_record = payload->track02_record;
    out_evidence->user_data_offset = payload->user_data_offset;
    out_evidence->prefix_word0 = read_be16(manifest);
    out_evidence->prefix_word1 = read_be16(manifest + 2u);
    out_evidence->descriptor_bytes = payload->manifest_bytes - 4u;
    out_evidence->descriptor_count = payload->manifest_entry_count;
    out_evidence->descriptor_hash = fnv1a32(manifest + 4u,
                                            out_evidence->descriptor_bytes);

    for (index = 0u; index < out_evidence->descriptor_count; ++index) {
        const uint8_t *encoded = manifest + 4u + index * 6u;
        Theron_V1Stage3ManifestWordTriple *descriptor =
            &out_evidence->descriptors[index];

        descriptor->word0 = read_be16(encoded);
        descriptor->word1 = read_be16(encoded + 2u);
        descriptor->word2 = read_be16(encoded + 4u);
        if (descriptor->word2 == 0u) {
            ++out_evidence->zero_word2_count;
        }
        if (index > 0u && descriptor->word2 <= previous_word2) {
            ++out_evidence->nonmonotonic_word2_transitions;
        }
        previous_word2 = descriptor->word2;
    }
    out_evidence->first_descriptor = out_evidence->descriptors[0];
    out_evidence->last_descriptor =
        out_evidence->descriptors[out_evidence->descriptor_count - 1u];
    return 1;
}

int theron_v1_stage3_manifest_compare(
    const Theron_V1Stage3ManifestEvidence *first,
    const Theron_V1Stage3ManifestEvidence *second,
    Theron_V1Stage3ManifestComparison *out_comparison) {
    size_t index;

    if (!out_comparison) return 0;
    memset(out_comparison, 0, sizeof(*out_comparison));
    if (!first || !second || !first->valid || !second->valid ||
        first->variant == second->variant ||
        first->descriptor_count !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT ||
        second->descriptor_count != first->descriptor_count) {
        return 0;
    }

    out_comparison->valid = 1;
    out_comparison->first_variant = first->variant;
    out_comparison->second_variant = second->variant;
    out_comparison->compared_descriptor_count = first->descriptor_count;
    out_comparison->first_descriptor_hash = first->descriptor_hash;
    out_comparison->second_descriptor_hash = second->descriptor_hash;
    for (index = 0u; index < first->descriptor_count; ++index) {
        if (memcmp(&first->descriptors[index], &second->descriptors[index],
                   sizeof(first->descriptors[index])) == 0) {
            ++out_comparison->byte_identical_descriptor_count;
        }
    }
    out_comparison->differing_descriptor_count =
        out_comparison->compared_descriptor_count -
        out_comparison->byte_identical_descriptor_count;
    return 1;
}
