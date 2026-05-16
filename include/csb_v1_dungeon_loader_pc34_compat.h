
#ifndef FIRESTAFF_CSB_V1_DUNGEON_LOADER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_DUNGEON_LOADER_PC34_COMPAT_H

#include <stdint.h>

/* CSB dungeon.dat shares the DM1 format with extensions:
 * - More dungeon levels (12 vs DM1's 14, but different layout)
 * - DSA (Dungeon Scripting Architecture) embedded scripts
 * - Custom background references per room
 * - Extended creature type table
 *
 * Source: CSBWin/CSBCode.cpp DBank::Initialize (TAG00332a)
 * Secondary: ReDMCSB DUNGEON.C F0148-F0170 (shared format)
 */

#define CSB_V1_MAX_LEVELS 12
#define CSB_V1_MAX_SQUARE_SIZE 32
#define CSB_V1_THING_TYPE_GROUP 4
#define CSB_V1_THING_TYPE_DSA 15  /* CSB-specific: DSA script thing */

typedef struct {
    int level_count;
    int level_offsets[CSB_V1_MAX_LEVELS];
    int level_widths[CSB_V1_MAX_LEVELS];
    int level_heights[CSB_V1_MAX_LEVELS];
    uint8_t *raw_data;
    int raw_size;
    /* DSA scripts */
    int dsa_count;
    uint16_t *dsa_offsets;
} CSB_V1_DungeonData;

int csb_v1_dungeon_load(CSB_V1_DungeonData *out, const uint8_t *dat, int dat_size);
int csb_v1_dungeon_get_square_type(const CSB_V1_DungeonData *d, int level, int x, int y);
int csb_v1_dungeon_get_first_thing(const CSB_V1_DungeonData *d, int level, int x, int y);
void csb_v1_dungeon_free(CSB_V1_DungeonData *d);

const char *csb_v1_dungeon_source_evidence(void);

#endif

