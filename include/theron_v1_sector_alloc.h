#ifndef THERON_V1_SECTOR_ALLOC_H
#define THERON_V1_SECTOR_ALLOC_H

#include <stddef.h>
#include <stdint.h>

#define THERON_SECTOR_ALLOC_COUNT 42u

typedef struct {
    uint16_t data_size;
    uint16_t sector_start;
    uint16_t sector_count;
} Theron_SectorAlloc;

const Theron_SectorAlloc *theron_v1_sector_alloc(unsigned int index);
size_t theron_v1_sector_alloc_count(void);

#endif
