#ifndef NEXUS_V1_SAL_CONTAINER_PROVENANCE_H
#define NEXUS_V1_SAL_CONTAINER_PROVENANCE_H

#include "nexus_v1_slev_sal_asset_discovery.h"

#include <stdint.h>

#define NEXUS_V1_SAL_CONTAINER_HEADER_BYTES 8U

/* Source-bound container facts only. The payload interval remains opaque and
 * cannot authorize a codec, sample layout, or playback. */
typedef struct {
    int valid;
    uint8_t magic[NEXUS_V1_SAL_CONTAINER_HEADER_BYTES];
    uint64_t source_fnv1a64;
    uint64_t source_byte_count;
    uint64_t descriptor_offset;
    uint64_t descriptor_length;
    uint64_t descriptor_fnv1a64;
    int codec_proven;
    int playback_permitted;
} Nexus_V1_SalContainerProvenanceReceipt;

int nexus_v1_sal_container_provenance_parse(
    const uint8_t *bytes, uint64_t byte_count, uint64_t expected_fnv1a64,
    Nexus_V1_SalContainerProvenanceReceipt *out_receipt);

/* Reads a direct SAL identity only while computing the receipt; no source
 * payload is retained after return. */
int nexus_v1_sal_container_provenance_from_direct_identity(
    const Nexus_V1_SlevSalDirectIdentity *identity,
    Nexus_V1_SalContainerProvenanceReceipt *out_receipt);

#endif
