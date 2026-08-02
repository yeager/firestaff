#include "theron_v1_track02_dungeon_descriptor.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Pointer table: UD 0x274018, 8 entries x 8 bytes.
 * Descriptor headers: UD 0x274058 (AKUTUBA), 0x2740D7 (DRATOR),
 * 0x274102 (FORMIC), 0x274129 (SARMON), 0x274150 (SHADO).
 * THIEF and DEMON have null descriptor offsets. */

static const Theron_DungeonPointerRecord g_pointers[8] = {
    { 0x0172, 0x278A, 0x0058, 0x016B },
    { 0x0172, 0x278A, 0x00D7, 0x016B },
    { 0x0198, 0x278A, 0x0102, 0x016B },
    { 0x0172, 0x278A, 0x0129, 0x016B },
    { 0x01AE, 0x278A, 0x0150, 0x016B },
    { 0x01C2, 0x278A, 0x0000, 0x0000 },
    { 0x01CF, 0x278A, 0x0000, 0x0000 },
    { 0x01DC, 0x278A, 0x0000, 0x0000 },
};

static const Theron_DungeonDescriptor g_descriptors[THERON_TRACK02_DUNGEON_DESCRIPTOR_COUNT] = {
    /* AKUTUBA: UD 0x274058 */  { 47, 44, 3, 5, 14, 2, 1 },
    /* DRATOR:  UD 0x2740D7 */  { 27, 24, 2, 4, 16, 2, 1 },
    /* FORMIC:  UD 0x274102 */  { 23, 20, 2, 4, 16, 2, 1 },
    /* SARMON:  UD 0x274129 */  { 23, 20, 2, 4, 16, 2, 1 },
    /* SHADO:   UD 0x274150 */  {  0, 24, 8, 10, 18, 2, 1 },
    /* THIEF:   no descriptor */ {  0,  0, 0, 0,  0, 0, 0 },
    /* DEMON:   no descriptor */ {  0,  0, 0, 0,  0, 0, 0 },
};

const Theron_DungeonPointerRecord *theron_v1_track02_dungeon_pointer(unsigned int index) {
    if (index >= 8) return NULL;
    return &g_pointers[index];
}

const Theron_DungeonDescriptor *theron_v1_track02_dungeon_descriptor(unsigned int index) {
    if (index >= THERON_TRACK02_DUNGEON_DESCRIPTOR_COUNT) return NULL;
    return &g_descriptors[index];
}

size_t theron_v1_track02_dungeon_descriptor_count(void) {
    return THERON_TRACK02_DUNGEON_DESCRIPTOR_COUNT;
}
