#ifndef NEXUS_V1_RES_H
#define NEXUS_V1_RES_H

#include <stdint.h>

#define NEXUS_RES_MAGIC        0x5245532A
#define NEXUS_RES_MAX_ENTRIES  64
#define NEXUS_RES_TAG_SIZE     4

typedef struct {
    char     tag[5];
    uint32_t index;
    uint32_t offset;
    uint32_t size;
} Nexus_V1_ResEntry;

typedef struct {
    int valid;
    uint32_t file_size;
    int entry_count;
    Nexus_V1_ResEntry entries[NEXUS_RES_MAX_ENTRIES];
} Nexus_V1_ResDecodeResult;

int nexus_v1_res_decode(const uint8_t *data, int data_size,
                         Nexus_V1_ResDecodeResult *out);

const Nexus_V1_ResEntry *nexus_v1_res_find(const Nexus_V1_ResDecodeResult *res,
                                            const char *tag, int index);

#endif
