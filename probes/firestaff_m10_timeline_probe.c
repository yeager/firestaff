/*
 * M10 Phase 12 probe: timeline / turn scheduler verification.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_timeline_pc34_compat.h"

static unsigned int checksum_bytes(const void* p, size_t n) {
    const unsigned char* b = (const unsigned char*)p;
    unsigned int h = 2166136261u;
    size_t i;
    for (i = 0; i < n; i++) {
        h ^= b[i];
        h *= 16777619u;
    }
    return h;
}

static struct TimelineEvent_Compat make_event(
    int kind,
    unsigned int fireAtTick,
    int mapIndex,
    int mapX,
    int mapY,
    int cell,
    int aux0,
    int aux1,
    int aux2,
    int aux3,
    int aux4)
{
    struct TimelineEvent_Compat event;
    event.kind = kind;
    event.fireAtTick = fireAtTick;
    event.mapIndex = mapIndex;
    event.mapX = mapX;
    event.mapY = mapY;
    event.cell = cell;
    event.aux0 = aux0;
    event.aux1 = aux1;
    event.aux2 = aux2;
    event.aux3 = aux3;
    event.aux4 = aux4;
    return event;
}

static int schedule_event(
    struct TimelineQueue_Compat* queue,
    int kind,
    unsigned int fireAtTick,
    int mapIndex,
    int mapX,
    int mapY,
    int cell,
    int aux0,
    int aux1,
    int aux2,
    int aux3,
    int aux4)
{
    struct TimelineEvent_Compat event = make_event(
        kind, fireAtTick, mapIndex, mapX, mapY, cell, aux0, aux1, aux2, aux3, aux4);
    return F0721_TIMELINE_Schedule_Compat(queue, &event);
}

int main(int argc, char* argv[]) {
    FILE* report;
    FILE* invariants;
    char path_buf[512];
    int failCount = 0;
    int invariantCount = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_dir>\n", argv[0]);
        return 1;
    }

    snprintf(path_buf, sizeof(path_buf), "%s/timeline_probe.md", argv[1]);
    report = fopen(path_buf, "w");
    if (!report) { fprintf(stderr, "FAIL: cannot write report\n"); return 1; }
    fprintf(report, "# M10 Phase 12: Timeline Probe\n\n");
    fprintf(report, "## Event kinds implemented\n\n");
    fprintf(report, "- CREATURE_TICK\n");
    fprintf(report, "- DOOR_ANIMATE\n");
    fprintf(report, "- PROJECTILE_MOVE\n");
    fprintf(report, "- EXPLOSION_ADVANCE\n");
    fprintf(report, "- SPELL_TICK\n");
    fprintf(report, "- HUNGER_THIRST\n");
    fprintf(report, "- MAGIC_LIGHT_DECAY\n");
    fprintf(report, "- MOVE_TIMER\n");
    fprintf(report, "- SENSOR_DELAYED\n");
    fprintf(report, "- DOOR_DESTRUCTION\n");
    fprintf(report, "- SQUARE_STATE\n");
    fprintf(report, "- GROUP_GENERATOR\n");
    fprintf(report, "- STATUS_TIMEOUT\n");
    fprintf(report, "- REMOVE_FLUXCAGE\n");
    fprintf(report, "- PLAY_SOUND\n");
    fprintf(report, "- WATCHDOG\n");
    fclose(report);

    snprintf(path_buf, sizeof(path_buf), "%s/timeline_invariants.md", argv[1]);
    invariants = fopen(path_buf, "w");
    if (!invariants) { fprintf(stderr, "FAIL: cannot write invariants\n"); return 1; }
    fprintf(invariants, "# Timeline Invariants\n\n");

#define CHECK(cond, msg) do { \
    invariantCount++; \
    if (cond) { \
        fprintf(invariants, "- PASS: %s\n", msg); \
    } else { \
        fprintf(invariants, "- FAIL: %s\n", msg); \
        failCount++; \
    } \
} while (0)

    CHECK(TIMELINE_EVENT_SERIALIZED_SIZE == 44,
          "TimelineEvent serialised size is 44 bytes");
    CHECK(TIMELINE_QUEUE_SERIALIZED_SIZE == 11272,
          "TimelineQueue serialised size is 11272 bytes (8 + 256*44)");

    {
        struct TimelineEvent_Compat orig = make_event(
            TIMELINE_EVENT_CREATURE_TICK, 123, 1, 2, 3, 0, 10, 11, 12, 13, 14);
        struct TimelineEvent_Compat restored;
        unsigned char buf[TIMELINE_EVENT_SERIALIZED_SIZE];
        memset(&restored, 0xAA, sizeof(restored));
        CHECK(F0725_TIMELINE_EventSerialize_Compat(&orig, buf, sizeof(buf)) == 1,
              "Event serialize succeeds for creature tick");
        CHECK(F0726_TIMELINE_EventDeserialize_Compat(&restored, buf, sizeof(buf)) == 1,
              "Event deserialize succeeds for creature tick");
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Event round-trip is bit-identical for creature tick");
    }
    {
        struct TimelineEvent_Compat orig = make_event(
            TIMELINE_EVENT_DOOR_ANIMATE, 456, 0, 7, 8, 1, 20, 21, 22, 23, 24);
        struct TimelineEvent_Compat restored;
        unsigned char buf[TIMELINE_EVENT_SERIALIZED_SIZE];
        memset(&restored, 0xBB, sizeof(restored));
        F0725_TIMELINE_EventSerialize_Compat(&orig, buf, sizeof(buf));
        F0726_TIMELINE_EventDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Event round-trip is bit-identical for door animate");
        CHECK(restored.kind == TIMELINE_EVENT_DOOR_ANIMATE,
              "Door animate kind survives round-trip");
    }
    {
        struct TimelineEvent_Compat orig = make_event(
            TIMELINE_EVENT_PROJECTILE_MOVE, 789, 2, 4, 5, 3, 30, 31, 32, 33, 34);
        struct TimelineEvent_Compat restored;
        unsigned char buf[TIMELINE_EVENT_SERIALIZED_SIZE];
        memset(&restored, 0xCC, sizeof(restored));
        F0725_TIMELINE_EventSerialize_Compat(&orig, buf, sizeof(buf));
        F0726_TIMELINE_EventDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Event round-trip is bit-identical for projectile move");
        CHECK(restored.kind == TIMELINE_EVENT_PROJECTILE_MOVE,
              "Projectile move kind survives round-trip");
    }

    {
        struct TimelineQueue_Compat orig;
        struct TimelineQueue_Compat restored;
        unsigned char buf[TIMELINE_QUEUE_SERIALIZED_SIZE];
        F0720_TIMELINE_Init_Compat(&orig, 99);
        memset(&restored, 0xDD, sizeof(restored));
        CHECK(F0727_TIMELINE_QueueSerialize_Compat(&orig, buf, sizeof(buf)) == 1,
              "Empty queue serialize succeeds");
        CHECK(F0728_TIMELINE_QueueDeserialize_Compat(&restored, buf, sizeof(buf)) == 1,
              "Empty queue deserialize succeeds");
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Empty queue round-trip is bit-identical");
    }

    {
        struct TimelineQueue_Compat orig;
        struct TimelineQueue_Compat restored;
        unsigned char buf[TIMELINE_QUEUE_SERIALIZED_SIZE];
        F0720_TIMELINE_Init_Compat(&orig, 77);
        schedule_event(&orig, TIMELINE_EVENT_SPELL_TICK, 30, 0, 1, 1, 0, 1, 2, 3, 4, 5);
        schedule_event(&orig, TIMELINE_EVENT_MOVE_TIMER, 12, 0, 2, 2, 1, 6, 7, 8, 9, 10);
        schedule_event(&orig, TIMELINE_EVENT_SENSOR_DELAYED, 44, 1, 3, 3, 2, 11, 12, 13, 14, 15);
        CHECK(F0727_TIMELINE_QueueSerialize_Compat(&orig, buf, sizeof(buf)) == 1,
              "Populated queue serialize succeeds");
        CHECK(F0728_TIMELINE_QueueDeserialize_Compat(&restored, buf, sizeof(buf)) == 1,
              "Populated queue deserialize succeeds");
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Populated queue round-trip is bit-identical");
    }

    {
        struct TimelineQueue_Compat queue;
        struct TimelineEvent_Compat event = make_event(TIMELINE_EVENT_SPELL_TICK, 50, 0, 4, 5, 0, 1, 2, 3, 4, 5);
        struct TimelineEvent_Compat eventCopy = event;
        F0720_TIMELINE_Init_Compat(&queue, 10);
        CHECK(F0721_TIMELINE_Schedule_Compat(&queue, &event) == 1,
              "Schedule succeeds on empty queue");
        CHECK(queue.count == 1,
              "Schedule grows queue by 1");
        CHECK(memcmp(&queue.events[0], &event, sizeof(event)) == 0,
              "Scheduled event lands at correct position in empty queue");
        CHECK(memcmp(&event, &eventCopy, sizeof(event)) == 0,
              "Schedule does not mutate input event struct");
    }

    {
        struct TimelineQueue_Compat queue;
        struct TimelineEvent_Compat out;
        F0720_TIMELINE_Init_Compat(&queue, 0);
        schedule_event(&queue, TIMELINE_EVENT_MAGIC_LIGHT_DECAY, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        schedule_event(&queue, TIMELINE_EVENT_DOOR_ANIMATE, 10, 0, 1, 1, 1, 0, 0, 0, 0, 0);
        schedule_event(&queue, TIMELINE_EVENT_PROJECTILE_MOVE, 20, 0, 2, 2, 2, 0, 0, 0, 0, 0);
        CHECK(F0723_TIMELINE_Pop_Compat(&queue, &out) == 1,
              "Pop succeeds on non-empty queue");
        CHECK(out.fireAtTick == 10 && out.kind == TIMELINE_EVENT_DOOR_ANIMATE,
              "Pop extracts smallest fireAtTick first");
        CHECK(F0723_TIMELINE_Pop_Compat(&queue, &out) == 1,
              "Second pop succeeds on non-empty queue");
        CHECK(out.fireAtTick == 20 && out.kind == TIMELINE_EVENT_PROJECTILE_MOVE,
              "Next smallest event follows after pop");
    }

    {
        struct TimelineQueue_Compat queue;
        struct TimelineEvent_Compat out;
        F0720_TIMELINE_Init_Compat(&queue, 5);
        CHECK(F0723_TIMELINE_Pop_Compat(&queue, &out) == 0,
              "Pop on empty queue returns 0");
        CHECK(out.kind == TIMELINE_EVENT_INVALID,
              "Pop on empty queue returns TIMELINE_EVENT_INVALID");
    }

    {
        struct TimelineQueue_Compat queue;
        struct TimelineEvent_Compat out;
        unsigned int before;
        unsigned int after;
        F0720_TIMELINE_Init_Compat(&queue, 33);
        schedule_event(&queue, TIMELINE_EVENT_HUNGER_THIRST, 88, 0, 0, 0, 0, 41, 42, 43, 44, 45);
        schedule_event(&queue, TIMELINE_EVENT_MAGIC_LIGHT_DECAY, 99, 0, 0, 0, 0, 46, 47, 48, 49, 50);
        before = checksum_bytes(&queue, sizeof(queue));
        CHECK(F0722_TIMELINE_Peek_Compat(&queue, &out) == 1,
              "Peek succeeds on non-empty queue");
        after = checksum_bytes(&queue, sizeof(queue));
        CHECK(out.fireAtTick == 88 && out.kind == TIMELINE_EVENT_HUNGER_THIRST,
              "Peek returns next event to fire");
        CHECK(before == after,
              "Peek does not mutate queue");
    }

    {
        struct TimelineQueue_Compat queue;
        int beforeCount;
        F0720_TIMELINE_Init_Compat(&queue, 100);
        schedule_event(&queue, TIMELINE_EVENT_MOVE_TIMER, 101, 0, 9, 9, 0, 0, 0, 0, 0, 0);
        beforeCount = queue.count;
        CHECK(F0724_TIMELINE_Tick_Compat(&queue, 17) == 1,
              "Tick succeeds with valid queue");
        CHECK(queue.nowTick == 117,
              "Tick advances nowTick precisely");
        CHECK(queue.count == beforeCount,
              "Tick does not fire events or pop anything");
    }

    {
        struct TimelineQueue_Compat a;
        struct TimelineQueue_Compat b;
        unsigned char bytesA[TIMELINE_QUEUE_SERIALIZED_SIZE];
        unsigned char bytesB[TIMELINE_QUEUE_SERIALIZED_SIZE];
        F0720_TIMELINE_Init_Compat(&a, 200);
        F0720_TIMELINE_Init_Compat(&b, 200);
        schedule_event(&a, TIMELINE_EVENT_CREATURE_TICK, 250, 0, 1, 0, 0, 9, 8, 7, 6, 5);
        schedule_event(&a, TIMELINE_EVENT_DOOR_ANIMATE, 201, 0, 2, 0, 1, 1, 2, 3, 4, 5);
        schedule_event(&a, TIMELINE_EVENT_PROJECTILE_MOVE, 249, 0, 3, 0, 2, 6, 7, 8, 9, 10);
        schedule_event(&b, TIMELINE_EVENT_CREATURE_TICK, 250, 0, 1, 0, 0, 9, 8, 7, 6, 5);
        schedule_event(&b, TIMELINE_EVENT_DOOR_ANIMATE, 201, 0, 2, 0, 1, 1, 2, 3, 4, 5);
        schedule_event(&b, TIMELINE_EVENT_PROJECTILE_MOVE, 249, 0, 3, 0, 2, 6, 7, 8, 9, 10);
        F0727_TIMELINE_QueueSerialize_Compat(&a, bytesA, sizeof(bytesA));
        F0727_TIMELINE_QueueSerialize_Compat(&b, bytesB, sizeof(bytesB));
        CHECK(memcmp(bytesA, bytesB, sizeof(bytesA)) == 0,
              "Same schedule sequence yields identical queue bytes");
    }

    {
        struct TimelineQueue_Compat queue;
        int i;
        int ok256 = 1;
        F0720_TIMELINE_Init_Compat(&queue, 0);
        for (i = 0; i < TIMELINE_QUEUE_CAPACITY; i++) {
            if (!schedule_event(&queue, TIMELINE_EVENT_STATUS_TIMEOUT, (unsigned int)(i + 1), 0, i, 0, 0, i, i, i, i, i)) {
                ok256 = 0;
                break;
            }
        }
        CHECK(ok256 == 1,
              "Scheduling 256 events succeeds");
        CHECK(queue.count == TIMELINE_QUEUE_CAPACITY,
              "Queue count reaches capacity 256");
        CHECK(schedule_event(&queue, TIMELINE_EVENT_WATCHDOG, 999, 0, 0, 0, 0, 0, 0, 0, 0, 0) == 0,
              "Scheduling 257th event returns 0");
    }

    {
        struct TimelineQueue_Compat queue;
        struct TimelineEvent_Compat out;
        F0720_TIMELINE_Init_Compat(&queue, 0);
        schedule_event(&queue, TIMELINE_EVENT_CREATURE_TICK, 100, 0, 1, 1, 0, 111, 0, 0, 0, 0);
        schedule_event(&queue, TIMELINE_EVENT_DOOR_ANIMATE, 100, 0, 2, 2, 1, 222, 0, 0, 0, 0);
        schedule_event(&queue, TIMELINE_EVENT_PROJECTILE_MOVE, 100, 0, 3, 3, 2, 333, 0, 0, 0, 0);
        F0723_TIMELINE_Pop_Compat(&queue, &out);
        CHECK(out.kind == TIMELINE_EVENT_CREATURE_TICK && out.aux0 == 111,
              "Equal-tick events pop in insertion order (first)");
        F0723_TIMELINE_Pop_Compat(&queue, &out);
        CHECK(out.kind == TIMELINE_EVENT_DOOR_ANIMATE && out.aux0 == 222,
              "Equal-tick events pop in insertion order (second)");
        F0723_TIMELINE_Pop_Compat(&queue, &out);
        CHECK(out.kind == TIMELINE_EVENT_PROJECTILE_MOVE && out.aux0 == 333,
              "Equal-tick events pop in insertion order (third)");
    }

    {
        struct TimelineQueue_Compat queue;
        unsigned char buf[TIMELINE_QUEUE_SERIALIZED_SIZE - 1];
        F0720_TIMELINE_Init_Compat(&queue, 0);
        CHECK(F0727_TIMELINE_QueueSerialize_Compat(&queue, buf, (int)sizeof(buf)) == 0,
              "Queue serialize rejects too-small buffer");
    }
    {
        struct TimelineEvent_Compat event = make_event(TIMELINE_EVENT_PLAY_SOUND, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        unsigned char buf[TIMELINE_EVENT_SERIALIZED_SIZE - 1];
        CHECK(F0725_TIMELINE_EventSerialize_Compat(&event, buf, (int)sizeof(buf)) == 0,
              "Event serialize rejects too-small buffer");
    }
    {
        struct TimelineQueue_Compat queue;
        CHECK(F0720_TIMELINE_Init_Compat(&queue, 314) == 1,
              "Init succeeds");
        CHECK(queue.nowTick == 314 && queue.count == 0,
              "Init sets nowTick and clears count");
    }

    fprintf(invariants, "\nInvariant count: %d\n", invariantCount);
    if (failCount == 0) {
        fprintf(invariants, "Status: PASS\n");
    } else {
        fprintf(invariants, "Status: FAIL (%d failures)\n", failCount);
    }
    fclose(invariants);
    return failCount > 0 ? 1 : 0;
}
