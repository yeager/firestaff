#ifndef REDMCSB_MEMORY_TIMELINE_PC34_COMPAT_H
#define REDMCSB_MEMORY_TIMELINE_PC34_COMPAT_H

/*
 * Timeline / turn scheduler data layer for ReDMCSB PC 3.4 — Phase 12 of M10.
 *
 * Pure queue primitives only: schedule, peek, pop, tick, serialise.
 * No event effects are executed in this phase.
 *
 * Event families referenced from original TIMELINE.C / DEFS.H include:
 *   - Door animation / destruction / square state updates
 *   - Group / creature movement and reactions
 *   - Projectile movement
 *   - Explosion advance / fluxcage removal
 *   - Party spell/status timers (light, invisibility, shields, poison,
 *     footprints, magic map)
 *   - Delayed sensor / actuator consequences
 */

#include <stdint.h>

#define TIMELINE_EVENT_INVALID         (-1)
#define TIMELINE_EVENT_CREATURE_TICK    1
#define TIMELINE_EVENT_DOOR_ANIMATE     2
#define TIMELINE_EVENT_PROJECTILE_MOVE  3
#define TIMELINE_EVENT_EXPLOSION_ADVANCE 4
#define TIMELINE_EVENT_SPELL_TICK       5
#define TIMELINE_EVENT_HUNGER_THIRST    6
#define TIMELINE_EVENT_MAGIC_LIGHT_DECAY 7
#define TIMELINE_EVENT_MOVE_TIMER       8
#define TIMELINE_EVENT_SENSOR_DELAYED   9
#define TIMELINE_EVENT_DOOR_DESTRUCTION 10
#define TIMELINE_EVENT_SQUARE_STATE     11
#define TIMELINE_EVENT_GROUP_GENERATOR  12
#define TIMELINE_EVENT_STATUS_TIMEOUT   13
#define TIMELINE_EVENT_REMOVE_FLUXCAGE  14
#define TIMELINE_EVENT_PLAY_SOUND       15
#define TIMELINE_EVENT_WATCHDOG         16

#define TIMELINE_QUEUE_CAPACITY 256
#define TIMELINE_EVENT_SERIALIZED_SIZE 44
#define TIMELINE_QUEUE_SERIALIZED_SIZE (8 + (TIMELINE_QUEUE_CAPACITY * TIMELINE_EVENT_SERIALIZED_SIZE))

struct TimelineEvent_Compat {
    int kind;
    uint32_t fireAtTick;
    int mapIndex;
    int mapX;
    int mapY;
    int cell;
    int aux0;
    int aux1;
    int aux2;
    int aux3;
    int aux4;
};

struct TimelineQueue_Compat {
    uint32_t nowTick;
    int count;
    struct TimelineEvent_Compat events[TIMELINE_QUEUE_CAPACITY];
};

int F0720_TIMELINE_Init_Compat(
    struct TimelineQueue_Compat* queue,
    uint32_t nowTick);

int F0721_TIMELINE_Schedule_Compat(
    struct TimelineQueue_Compat* queue,
    const struct TimelineEvent_Compat* event);

int F0722_TIMELINE_Peek_Compat(
    const struct TimelineQueue_Compat* queue,
    struct TimelineEvent_Compat* outEvent);

int F0723_TIMELINE_Pop_Compat(
    struct TimelineQueue_Compat* queue,
    struct TimelineEvent_Compat* outEvent);

int F0724_TIMELINE_Tick_Compat(
    struct TimelineQueue_Compat* queue,
    uint32_t deltaTicks);

int F0725_TIMELINE_EventSerialize_Compat(
    const struct TimelineEvent_Compat* event,
    unsigned char* outBuf,
    int outBufSize);

int F0726_TIMELINE_EventDeserialize_Compat(
    struct TimelineEvent_Compat* event,
    const unsigned char* buf,
    int bufSize);

int F0727_TIMELINE_QueueSerialize_Compat(
    const struct TimelineQueue_Compat* queue,
    unsigned char* outBuf,
    int outBufSize);

int F0728_TIMELINE_QueueDeserialize_Compat(
    struct TimelineQueue_Compat* queue,
    const unsigned char* buf,
    int bufSize);

#endif
