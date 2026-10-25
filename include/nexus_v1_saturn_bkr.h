#ifndef NEXUS_V1_SATURN_BKR_H
#define NEXUS_V1_SATURN_BKR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_V1_SATURN_BKR_MAX_IMAGE_BYTES (4U * 1024U * 1024U)
#define NEXUS_V1_SATURN_BKR_MAX_ENTRIES 128U

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

/* Parse a Saturn Backup RAM image and copy each game payload, in directory
 * order, to caller-owned storage. The destination must be at least image_size
 * bytes, which bounds the total copied payload for every valid card image.
 * No game-specific payload fields are interpreted. */
int nexus_v1_saturn_bkr_parse(
    const uint8_t *image, size_t image_size,
    Nexus_V1_SaturnBkrEntry *entries, size_t entry_capacity,
    uint8_t *payload_storage, size_t payload_capacity,
    Nexus_V1_SaturnBkrReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
