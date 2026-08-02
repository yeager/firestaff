#ifndef THERON_V1_TRACK02_DUNGEON_DESCRIPTOR_H
#define THERON_V1_TRACK02_DUNGEON_DESCRIPTOR_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN, UD 0x274018 (pointer table) and
 * UD 0x274058-0x274170 (descriptor headers).
 * Theron-specific format: 16-byte header with FF terminator,
 * NOT standard DM1 DUNGEON.DAT format. */

#define THERON_TRACK02_DUNGEON_DESCRIPTOR_COUNT  7u

typedef struct {
    uint16_t sprite_offset;
    uint16_t constant_278a;
    uint16_t desc_offset;
    uint16_t field3;
} Theron_DungeonPointerRecord;

typedef struct {
    uint16_t field0;
    uint16_t field1;
    uint8_t  field2;
    uint8_t  field3;
    uint8_t  field4;
    uint8_t  field5;
    int      has_descriptor;
} Theron_DungeonDescriptor;

const Theron_DungeonPointerRecord *theron_v1_track02_dungeon_pointer(unsigned int index);
const Theron_DungeonDescriptor *theron_v1_track02_dungeon_descriptor(unsigned int index);
size_t theron_v1_track02_dungeon_descriptor_count(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_DUNGEON_DESCRIPTOR_H */
