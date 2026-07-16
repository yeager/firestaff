#ifndef FIRESTAFF_DM2_V1_FIND_LADDER_AROUND_H
#define FIRESTAFF_DM2_V1_FIND_LADDER_AROUND_H

#include <stdint.h>

#include "dm2_v1_dungeon_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM2_V1_LADDER_AROUND_NONE = 0,
    DM2_V1_LADDER_AROUND_UP = 1,
    DM2_V1_LADDER_AROUND_DOWN = 2
} DM2_V1_LadderAroundKind;

typedef struct {
    int valid;
    int found;
    int level;
    int origin_x;
    int origin_y;
    int ladder_x;
    int ladder_y;
    int search_slot;
    int manhattan_distance;
    int raw_tile;
    int square_type;
    DM2_V1_LadderAroundKind kind;
    int vertical_delta;
    uint32_t search_hash;
} DM2_V1_FindLadderAroundReceipt;

int dm2_v1_FIND_LADDER_AROUND(
    const DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    DM2_V1_FindLadderAroundReceipt *out);

const char *dm2_v1_FIND_LADDER_AROUND_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_FIND_LADDER_AROUND_H */
