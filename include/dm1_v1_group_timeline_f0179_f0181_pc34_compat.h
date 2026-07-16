#ifndef FIRESTAFF_DM1_V1_GROUP_TIMELINE_F0179_F0181_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GROUP_TIMELINE_F0179_F0181_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_event_timer_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0179_ASPECT_FLIP_BITMAP_PC34 = 0x40,
    DM1_V1_F0179_ASPECT_IS_ATTACKING_PC34 = 0x80,
    DM1_V1_F0179_ASPECT_HORIZONTAL_MASK_PC34 = 0x07,
    DM1_V1_F0179_ASPECT_VERTICAL_MASK_PC34 = 0x38,
    DM1_V1_F0179_ASPECT_VERTICAL_SHIFT_PC34 = 3,
    DM1_V1_F0181_GROUP_EVENT_FIRST_PC34 =
        DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE,
    DM1_V1_F0181_GROUP_EVENT_LAST_PC34 =
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3
};

typedef struct {
    uint8_t aspect[4];
    uint16_t graphicInfo;
    uint32_t gameTime;
    uint16_t randomValue;
    uint16_t updateDelay;
    uint8_t eventType;
    uint8_t mapIndex;
    uint8_t mapX;
    uint8_t mapY;
} DM1_V1_GroupAspectUpdateInputF0179Pc34;

typedef struct {
    int valid;
    int updatedCreatureIndex;
    uint8_t previousAspect;
    uint8_t updatedAspect;
    uint32_t updateTime;
    struct DM1_Event_V1 nextEvent;
} DM1_V1_GroupAspectUpdateResultF0179Pc34;

typedef struct {
    uint32_t gameTime;
    uint8_t mapIndex;
    uint8_t mapX;
    uint8_t mapY;
    uint8_t groupCell;
    uint8_t groupThingIndex;
    uint8_t priority;
} DM1_V1_StartWanderingInputF0180Pc34;

typedef struct {
    int valid;
    uint32_t updateTime;
    struct DM1_Event_V1 event;
} DM1_V1_StartWanderingResultF0180Pc34;

typedef struct {
    size_t scannedEventCount;
    size_t deletedEventCount;
} DM1_V1_DeleteEventsResultF0181Pc34;

const char *DM1_V1_F0179_F0180_F0181_SourceEvidencePc34(void);

int F0179_GROUP_GetCreatureAspectUpdateTime(
    const DM1_V1_GroupAspectUpdateInputF0179Pc34 *input,
    DM1_V1_GroupAspectUpdateResultF0179Pc34 *out);

int F0180_GROUP_StartWandering(
    const DM1_V1_StartWanderingInputF0180Pc34 *input,
    DM1_V1_StartWanderingResultF0180Pc34 *out);

int F0181_GROUP_DeleteEvents(
    struct DM1_Event_V1 *events,
    size_t eventCount,
    int mapX,
    int mapY,
    DM1_V1_DeleteEventsResultF0181Pc34 *out);

#ifdef __cplusplus
}
#endif

#endif
