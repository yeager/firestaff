#ifndef FIRESTAFF_DM1_V1_GROUP_ACTIVE_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GROUP_ACTIVE_LIFECYCLE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0182_ATTACKING_ASPECT_MASK_PC34 = 0x80,
    DM1_V1_F0195_SQUARE_THING_LIST_PRESENT_PC34 = 0x10,
    DM1_V1_F0195_GROUP_THING_TYPE_PC34 = 4,
    DM1_V1_F0195_THING_END_OF_LIST_PC34 = 0xFFFE,
    DM1_V1_F0196_MINIMUM_ACTIVE_GROUP_COUNT_PC34 = 110
};

typedef struct {
    uint8_t aspect[4];
} DM1_V1_F0182_ActiveGroupPc34Compat;

typedef void (*DM1_V1_F0182_DeleteEventsPc34Compat)(
    void* context,
    int16_t mapX,
    int16_t mapY);

void F0182_GROUP_StopAttacking(
    DM1_V1_F0182_ActiveGroupPc34Compat* activeGroup,
    int16_t mapX,
    int16_t mapY,
    DM1_V1_F0182_DeleteEventsPc34Compat deleteEvents,
    void* deleteEventsContext);

typedef uint16_t (*DM1_V1_F0195_GetNextThingPc34Compat)(
    void* context,
    uint16_t thing);

typedef void (*DM1_V1_F0195_GroupEventPc34Compat)(
    void* context,
    uint16_t groupThing,
    int16_t mapX,
    int16_t mapY);

typedef struct {
    const uint8_t* mapSquares;
    size_t mapSquareCount;
    const uint16_t* squareFirstThings;
    size_t squareFirstThingCount;
    int16_t mapWidth;
    int16_t mapHeight;
    DM1_V1_F0195_GetNextThingPc34Compat getNextThing;
    void* getNextThingContext;
    DM1_V1_F0195_GroupEventPc34Compat deleteEvents;
    void* deleteEventsContext;
    DM1_V1_F0195_GroupEventPc34Compat addActiveGroup;
    void* addActiveGroupContext;
    DM1_V1_F0195_GroupEventPc34Compat startWandering;
    void* startWanderingContext;
} DM1_V1_F0195_AddAllActiveGroupsInputPc34Compat;

int F0195_GROUP_AddAllActiveGroups(
    const DM1_V1_F0195_AddAllActiveGroupsInputPc34Compat* input);

typedef struct {
    int16_t groupThingIndex;
} DM1_V1_F0196_ActiveGroupSlotPc34Compat;

typedef struct {
    int newGame;
    uint16_t maximumActiveGroupCount;
    DM1_V1_F0196_ActiveGroupSlotPc34Compat* slots;
    size_t slotCapacity;
} DM1_V1_F0196_InitializeActiveGroupsInputPc34Compat;

int F0196_GROUP_InitializeActiveGroups(
    DM1_V1_F0196_InitializeActiveGroupsInputPc34Compat* input);

const char* DM1_V1_GroupActiveLifecycle_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
