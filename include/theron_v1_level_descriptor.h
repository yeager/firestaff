#ifndef THERON_V1_LEVEL_DESCRIPTOR_H
#define THERON_V1_LEVEL_DESCRIPTOR_H

#include <stddef.h>
#include <stdint.h>

/* Level descriptor table from Track 02 BIN UD 0x619900.
 * 53 records of 6 bytes each describing tile graphics blocks.
 * sector_count = ceil(data_size / 2048). cumulative_sector_offset indexes
 * into the 7 level blocks at UDs {0x09F000..0x21F000}. */

#define THERON_LEVEL_DESCRIPTOR_COUNT 53u
#define THERON_LEVEL_DESCRIPTOR_USER_DATA_OFFSET 0x619900u
#define THERON_LEVEL_DESCRIPTOR_BYTES \
    (THERON_LEVEL_DESCRIPTOR_COUNT * 6u)

typedef struct {
    uint8_t  flags;
    uint8_t  sector_count;
    uint16_t data_size;
    uint8_t  reserved;
    uint8_t  cumulative_sector_offset;
} Theron_LevelDescriptor;

const Theron_LevelDescriptor *theron_v1_level_descriptor(unsigned int index);
size_t theron_v1_level_descriptor_count(void);

/* Read the descriptor bytes from an already authenticated US Track 02
 * MODE1 user-data stream.  This is a source receipt only: it does not decode
 * the referenced graphics blocks or promote their fields to runtime map
 * semantics. */
int theron_v1_level_descriptor_read_us_track02(
    const uint8_t *user_data,
    size_t user_data_size,
    Theron_LevelDescriptor *out,
    size_t out_count);

#endif
