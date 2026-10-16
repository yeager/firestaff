#ifndef DM1_V1_IIGS_DUNGEON_H
#define DM1_V1_IIGS_DUNGEON_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_IIGS_DUNGEON_MAX_BYTES (1024U * 1024U)

typedef struct DM1_V1_IigsDungeonReceipt {
    size_t decoded_size;
    uint16_t dungeon_id;
    uint8_t map_count;
    uint8_t start_map_index;
    uint8_t start_x;
    uint8_t start_y;
    uint8_t start_direction;
    uint8_t first_map_width;
    uint8_t first_map_height;
    uint16_t raw_map_data_byte_count;
    uint16_t text_data_word_count;
    uint16_t square_first_thing_count;
    int column_count;
} DM1_V1_IigsDungeonReceipt;

/* Inspect the native little-endian 8104 wrapper and enforce resource limits.
 * This only reports required output capacity, not body validity or identity.
 * Both compressed input and declared output are capped at 1 MiB. */
int dm1_v1_iigs_dungeon_query(const uint8_t *data, size_t size,
                             size_t *out_size);

/* Decode into caller-owned, nonoverlapping memory. Preserve the complete
 * native LE body, including its checksum trailer, and validate it through
 * the existing shared LE dungeon parser (maximum 32 maps). Temporary parser
 * allocations are freed before return. Returns 1 on success, 0 on failure.
 * On failure out_size/receipt are zero; output bytes may be partial.
 * out_size is required; receipt is optional. Authenticating source bytes is
 * the caller's responsibility. This primitive does not admit a platform.
 * Trailerless bodies, raw unwrapped bodies and save containers are not
 * supported by this intake API. */
int dm1_v1_iigs_dungeon_decode(const uint8_t *data, size_t size,
    uint8_t *out, size_t capacity, size_t *out_size,
    DM1_V1_IigsDungeonReceipt *receipt);

#ifdef __cplusplus
}
#endif
#endif
