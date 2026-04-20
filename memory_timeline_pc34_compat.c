/*
 * Timeline / turn scheduler data layer for ReDMCSB PC 3.4 — Phase 12 of M10.
 */

#include <string.h>
#include "memory_timeline_pc34_compat.h"

static void write_i32_le(unsigned char* p, int value) {
    unsigned int u = (unsigned int)value;
    p[0] = (unsigned char)(u & 0xFF);
    p[1] = (unsigned char)((u >> 8) & 0xFF);
    p[2] = (unsigned char)((u >> 16) & 0xFF);
    p[3] = (unsigned char)((u >> 24) & 0xFF);
}

static int read_i32_le(const unsigned char* p) {
    unsigned int u =
        ((unsigned int)p[0]) |
        ((unsigned int)p[1] << 8) |
        ((unsigned int)p[2] << 16) |
        ((unsigned int)p[3] << 24);
    return (int)u;
}

static int timeline_event_is_valid(const struct TimelineEvent_Compat* event) {
    return event != 0;
}

static int timeline_queue_has_valid_count(const struct TimelineQueue_Compat* queue) {
    return (queue->count >= 0) && (queue->count <= TIMELINE_QUEUE_CAPACITY);
}

int F0720_TIMELINE_Init_Compat(
    struct TimelineQueue_Compat* queue,
    uint32_t nowTick)
{
    if (queue == 0) return 0;
    memset(queue, 0, sizeof(*queue));
    queue->nowTick = nowTick;
    return 1;
}

int F0721_TIMELINE_Schedule_Compat(
    struct TimelineQueue_Compat* queue,
    const struct TimelineEvent_Compat* event)
{
    int insertIndex;
    if (queue == 0 || !timeline_event_is_valid(event)) return 0;
    if (!timeline_queue_has_valid_count(queue)) return 0;
    if (queue->count >= TIMELINE_QUEUE_CAPACITY) return 0;

    insertIndex = queue->count;
    while (insertIndex > 0) {
        const struct TimelineEvent_Compat* prior = &queue->events[insertIndex - 1];
        if (prior->fireAtTick <= event->fireAtTick) {
            break;
        }
        queue->events[insertIndex] = queue->events[insertIndex - 1];
        insertIndex--;
    }
    queue->events[insertIndex] = *event;
    queue->count++;
    return 1;
}

int F0722_TIMELINE_Peek_Compat(
    const struct TimelineQueue_Compat* queue,
    struct TimelineEvent_Compat* outEvent)
{
    if (queue == 0 || outEvent == 0) return 0;
    if (!timeline_queue_has_valid_count(queue)) return 0;
    if (queue->count <= 0) {
        memset(outEvent, 0, sizeof(*outEvent));
        outEvent->kind = TIMELINE_EVENT_INVALID;
        return 0;
    }
    *outEvent = queue->events[0];
    return 1;
}

int F0723_TIMELINE_Pop_Compat(
    struct TimelineQueue_Compat* queue,
    struct TimelineEvent_Compat* outEvent)
{
    int i;
    if (queue == 0 || outEvent == 0) return 0;
    if (!timeline_queue_has_valid_count(queue)) return 0;
    if (queue->count <= 0) {
        memset(outEvent, 0, sizeof(*outEvent));
        outEvent->kind = TIMELINE_EVENT_INVALID;
        return 0;
    }
    *outEvent = queue->events[0];
    for (i = 1; i < queue->count; i++) {
        queue->events[i - 1] = queue->events[i];
    }
    memset(&queue->events[queue->count - 1], 0, sizeof(queue->events[queue->count - 1]));
    queue->count--;
    return 1;
}

int F0724_TIMELINE_Tick_Compat(
    struct TimelineQueue_Compat* queue,
    uint32_t deltaTicks)
{
    if (queue == 0) return 0;
    if (!timeline_queue_has_valid_count(queue)) return 0;
    queue->nowTick += deltaTicks;
    return 1;
}

int F0725_TIMELINE_EventSerialize_Compat(
    const struct TimelineEvent_Compat* event,
    unsigned char* outBuf,
    int outBufSize)
{
    if (!timeline_event_is_valid(event) || outBuf == 0) return 0;
    if (outBufSize < TIMELINE_EVENT_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf +  0, event->kind);
    write_i32_le(outBuf +  4, (int)event->fireAtTick);
    write_i32_le(outBuf +  8, event->mapIndex);
    write_i32_le(outBuf + 12, event->mapX);
    write_i32_le(outBuf + 16, event->mapY);
    write_i32_le(outBuf + 20, event->cell);
    write_i32_le(outBuf + 24, event->aux0);
    write_i32_le(outBuf + 28, event->aux1);
    write_i32_le(outBuf + 32, event->aux2);
    write_i32_le(outBuf + 36, event->aux3);
    write_i32_le(outBuf + 40, event->aux4);
    return 1;
}

int F0726_TIMELINE_EventDeserialize_Compat(
    struct TimelineEvent_Compat* event,
    const unsigned char* buf,
    int bufSize)
{
    if (event == 0 || buf == 0) return 0;
    if (bufSize < TIMELINE_EVENT_SERIALIZED_SIZE) return 0;

    event->kind = read_i32_le(buf +  0);
    event->fireAtTick = (uint32_t)read_i32_le(buf +  4);
    event->mapIndex = read_i32_le(buf +  8);
    event->mapX = read_i32_le(buf + 12);
    event->mapY = read_i32_le(buf + 16);
    event->cell = read_i32_le(buf + 20);
    event->aux0 = read_i32_le(buf + 24);
    event->aux1 = read_i32_le(buf + 28);
    event->aux2 = read_i32_le(buf + 32);
    event->aux3 = read_i32_le(buf + 36);
    event->aux4 = read_i32_le(buf + 40);
    return 1;
}

int F0727_TIMELINE_QueueSerialize_Compat(
    const struct TimelineQueue_Compat* queue,
    unsigned char* outBuf,
    int outBufSize)
{
    int i;
    if (queue == 0 || outBuf == 0) return 0;
    if (!timeline_queue_has_valid_count(queue)) return 0;
    if (outBufSize < TIMELINE_QUEUE_SERIALIZED_SIZE) return 0;

    memset(outBuf, 0, TIMELINE_QUEUE_SERIALIZED_SIZE);
    write_i32_le(outBuf + 0, (int)queue->nowTick);
    write_i32_le(outBuf + 4, queue->count);
    for (i = 0; i < TIMELINE_QUEUE_CAPACITY; i++) {
        unsigned char* slot = outBuf + 8 + (i * TIMELINE_EVENT_SERIALIZED_SIZE);
        if (i < queue->count) {
            F0725_TIMELINE_EventSerialize_Compat(&queue->events[i], slot, TIMELINE_EVENT_SERIALIZED_SIZE);
        }
    }
    return 1;
}

int F0728_TIMELINE_QueueDeserialize_Compat(
    struct TimelineQueue_Compat* queue,
    const unsigned char* buf,
    int bufSize)
{
    int i;
    if (queue == 0 || buf == 0) return 0;
    if (bufSize < TIMELINE_QUEUE_SERIALIZED_SIZE) return 0;

    memset(queue, 0, sizeof(*queue));
    queue->nowTick = (uint32_t)read_i32_le(buf + 0);
    queue->count = read_i32_le(buf + 4);
    if (!timeline_queue_has_valid_count(queue)) return 0;
    for (i = 0; i < queue->count; i++) {
        const unsigned char* slot = buf + 8 + (i * TIMELINE_EVENT_SERIALIZED_SIZE);
        F0726_TIMELINE_EventDeserialize_Compat(&queue->events[i], slot, TIMELINE_EVENT_SERIALIZED_SIZE);
    }
    return 1;
}
