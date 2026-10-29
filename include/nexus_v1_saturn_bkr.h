#ifndef NEXUS_V1_SATURN_BKR_H
#define NEXUS_V1_SATURN_BKR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_V1_SATURN_BKR_MAX_IMAGE_BYTES (4U * 1024U * 1024U)
#define NEXUS_V1_SATURN_BKR_MAX_ENTRIES 128U
#define NEXUS_V1_SATURN_BKR_MAX_PARTY 4U

typedef struct {
    char name[12];
    uint8_t language;
    char comment[11];
    uint32_t timestamp_minutes;
    uint32_t payload_bytes;
    uint32_t payload_offset;
    uint32_t first_block;
    uint32_t allocated_block_count;
} Nexus_V1_SaturnBkrEntry;

typedef struct {
    int valid;
    uint32_t image_bytes;
    uint32_t block_bytes;
    uint32_t block_count;
    uint32_t entry_count;
    uint32_t payload_bytes;
} Nexus_V1_SaturnBkrReceipt;

/* Bounded receipt for the retail DMNEXUS__01 payload. Champion records stay
 * opaque. The party leader index is supported by the retail decompilation
 * and matching authentic payload/WorkRAM captures. */
typedef struct {
    int valid;
    uint32_t payload_bytes;
    uint32_t champion_count;
    uint32_t party_leader_index;
    int32_t champion_stamina_current[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    int32_t champion_stamina_max[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    int32_t champion_bounded_value[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    int32_t champion_bounded_limit[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    uint32_t champion_records_offset;
    uint32_t champion_record_stride;
    uint32_t champion_records_bytes;
    uint32_t declared_payload_span;
} Nexus_V1_SaturnBkrNexusPayloadReceipt;

/* Parse a Saturn Backup RAM image and copy each game payload, in directory
 * order, to caller-owned storage. The destination must be at least image_size
 * bytes, which bounds the total copied payload for every valid card image.
 * No game-specific payload fields are interpreted. */
int nexus_v1_saturn_bkr_parse(
    const uint8_t *image, size_t image_size,
    Nexus_V1_SaturnBkrEntry *entries, size_t entry_capacity,
    uint8_t *payload_storage, size_t payload_capacity,
    Nexus_V1_SaturnBkrReceipt *out_receipt);

/* Validate the source-relative offsets, roster count, leader index, record
 * stride, and bounded span observed in hash-verified retail payloads. Extract
 * only the current/maximum stamina pair and bounded value/limit pair whose
 * source and runtime operations are bound by retail SH-2 decompilation. Other
 * champion fields stay opaque. */
int nexus_v1_saturn_bkr_nexus_payload_probe(
    const Nexus_V1_SaturnBkrEntry *entry,
    const uint8_t *payload, size_t payload_size,
    Nexus_V1_SaturnBkrNexusPayloadReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
