#ifndef THERON_V1_LEVEL_DESCRIPTOR_H
#define THERON_V1_LEVEL_DESCRIPTOR_H

#include <stddef.h>
#include <stdint.h>

/* Level descriptor table from Track 02 BIN UD 0x619900.
 * 43 records of 6 bytes each describing dungeon level geometry. */

#define THERON_LEVEL_DESCRIPTOR_COUNT 43u

typedef struct {
    uint8_t  flags;
    uint8_t  map_width;
    uint16_t data_size;
    uint8_t  reserved;
    uint8_t  cumulative_column_index;
} Theron_LevelDescriptor;

const Theron_LevelDescriptor *theron_v1_level_descriptor(unsigned int index);
size_t theron_v1_level_descriptor_count(void);

#endif
