#ifndef THERON_V1_STAGE3_MANIFEST_EVIDENCE_H
#define THERON_V1_STAGE3_MANIFEST_EVIDENCE_H

#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Read-only evidence for the executable payload entered at $3800 by the
 * original stage-two loader.  Its first 0x520 bytes form a 4-byte prefix and
 * 218 six-byte big-endian descriptors.  The descriptor words are deliberately
 * opaque: this module establishes no asset, object, level, palette, or CD
 * command semantics.
 */

typedef struct {
    uint16_t word0;
    uint16_t word1;
    uint16_t word2;
} Theron_V1Stage3ManifestWordTriple;

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t track02_record;
    size_t user_data_offset;
    uint16_t prefix_word0;
    uint16_t prefix_word1;
    size_t descriptor_bytes;
    size_t descriptor_count;
    size_t zero_word2_count;
    size_t nonmonotonic_word2_transitions;
    uint32_t descriptor_hash;
    Theron_V1Stage3ManifestWordTriple first_descriptor;
    Theron_V1Stage3ManifestWordTriple last_descriptor;
    Theron_V1Stage3ManifestWordTriple descriptors[
        THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT];
} Theron_V1Stage3ManifestEvidence;

typedef struct {
    int valid;
    Theron_Track02Variant first_variant;
    Theron_Track02Variant second_variant;
    size_t compared_descriptor_count;
    size_t byte_identical_descriptor_count;
    size_t differing_descriptor_count;
    uint32_t first_descriptor_hash;
    uint32_t second_descriptor_hash;
} Theron_V1Stage3ManifestComparison;

/* The caller must supply an authenticated raw Track 02 payload receipt. */
int theron_v1_stage3_manifest_evidence_from_payload(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1Stage3ManifestEvidence *out_evidence);

int theron_v1_stage3_manifest_compare(
    const Theron_V1Stage3ManifestEvidence *first,
    const Theron_V1Stage3ManifestEvidence *second,
    Theron_V1Stage3ManifestComparison *out_comparison);

#endif /* THERON_V1_STAGE3_MANIFEST_EVIDENCE_H */
