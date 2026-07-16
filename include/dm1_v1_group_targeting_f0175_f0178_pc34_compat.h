#ifndef FIRESTAFF_DM1_V1_GROUP_TARGETING_F0175_F0178_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GROUP_TARGETING_F0175_F0178_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0175_THING_END_OF_LIST_PC34 = 0xfffeu,
    DM1_V1_F0175_THING_TYPE_GROUP_PC34 = 4,
    DM1_V1_F0176_SINGLE_CENTERED_CREATURE_PC34 = 0xff,
    DM1_V1_F0176_CREATURE_SIZE_QUARTER_SQUARE_PC34 = 0,
    DM1_V1_F0176_CREATURE_SIZE_HALF_SQUARE_PC34 = 1,
    DM1_V1_F0176_CREATURE_SIZE_FULL_SQUARE_PC34 = 2,
    DM1_V1_F0176_MAX_CREATURE_SLOTS_PC34 = 4
};

typedef struct {
    uint16_t thing;
    uint16_t nextThing;
} DM1_V1_GroupThingNodeF0175Pc34;

typedef struct {
    int valid;
    uint16_t thing;
    int nodeIndex;
    int scannedNodeCount;
} DM1_V1_GroupThingResultF0175Pc34;

typedef struct {
    int creatureCountMinusOne;
    uint8_t packedCells;
    int groupDirection;
    int creatureSize;
    int creatureHealth[DM1_V1_F0176_MAX_CREATURE_SLOTS_PC34];
} DM1_V1_GroupCellContextF0176Pc34;

typedef struct {
    int valid;
    int creatureOrdinal;
    int creatureIndex;
    int queryCell;
    int matchedCell;
    int singleCentered;
} DM1_V1_CreatureOrdinalResultF0176Pc34;

typedef struct {
    const DM1_V1_GroupCellContextF0176Pc34 *group;
    int championCell;
    int targetDirection;
} DM1_V1_MeleeTargetInputF0177Pc34;

typedef struct {
    int valid;
    int creatureOrdinal;
    int creatureIndex;
    int firstLivingCreatureIndex;
    int orderedCells[4];
    int orderedCellCount;
    int singleCentered;
} DM1_V1_MeleeTargetResultF0177Pc34;

const char *DM1_V1_F0175_F0178_SourceEvidencePc34(void);

uint16_t F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(
    uint16_t groupValue,
    int creatureIndex,
    uint16_t creatureValue);

uint16_t F0175_GROUP_GetThing(
    const DM1_V1_GroupThingNodeF0175Pc34 *nodes,
    size_t nodeCount,
    uint16_t firstThing,
    DM1_V1_GroupThingResultF0175Pc34 *out);

int F0176_GROUP_GetCreatureOrdinalInCell(
    const DM1_V1_GroupCellContextF0176Pc34 *group,
    int cell,
    DM1_V1_CreatureOrdinalResultF0176Pc34 *out);

int F0177_GROUP_GetMeleeTargetCreatureOrdinal(
    const DM1_V1_MeleeTargetInputF0177Pc34 *input,
    DM1_V1_MeleeTargetResultF0177Pc34 *out);

#ifdef __cplusplus
}
#endif

#endif
