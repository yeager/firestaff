#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        return 1; \
    } \
} while (0)

int main(void)
{
    struct GameWorld_Compat world;
    DM1RuntimeEventReceipt_Compat source;
    DM1RuntimeEventReceipt_Compat rows[DM1_RUNTIME_EVENT_RECEIPT_CAPACITY];
    uint64_t cursor = 0;
    uint64_t dropped = 0;
    size_t count;
    unsigned int i;

    memset(&world, 0, sizeof(world));
    memset(&source, 0, sizeof(source));
    world.gameTick = 77;
    source.kind = DM1_RUNTIME_EVENT_CREATURE_MOVED;
    source.mapIndex = 3;
    source.groupIndex = 19;
    source.creatureIndex = -1;
    source.creatureType = 5;
    source.championIndex = -1;
    source.fromX = 10;
    source.fromY = 11;
    source.toX = 10;
    source.toY = 12;

    for (i = 0; i < DM1_RUNTIME_EVENT_RECEIPT_CAPACITY + 2u; ++i) {
        source.toY = 12 + (int)i;
        DM1_RuntimeEventReceiptPublish_Compat(&world, &source);
    }
    CHECK(world.dm1RuntimeEventSequence ==
              DM1_RUNTIME_EVENT_RECEIPT_CAPACITY + 2u,
          "publisher assigns monotonically increasing sequence numbers");
    CHECK(world.dm1RuntimeEventDropped == 2,
          "full ring counts overwritten receipts");

    count = DM1_RuntimeEventReceiptsSince_Compat(
        &world, &cursor, rows, 3, &dropped);
    CHECK(count == 3 && dropped == 2,
          "bounded drain reports overwritten history and respects capacity");
    CHECK(rows[0].sequence == 3 && rows[1].sequence == 4 &&
              rows[2].sequence == 5 && cursor == 5,
          "bounded drain returns retained receipts in sequence order");
    CHECK(rows[0].gameTick == 77 && rows[0].groupIndex == 19 &&
              rows[0].fromX == 10 && rows[0].toY == 14,
          "receipt keeps source transition identity and coordinates");

    count = DM1_RuntimeEventReceiptsSince_Compat(
        &world, &cursor, rows, DM1_RUNTIME_EVENT_RECEIPT_CAPACITY, &dropped);
    CHECK(count == DM1_RUNTIME_EVENT_RECEIPT_CAPACITY - 3u && dropped == 0,
          "subsequent drain returns remaining retained history exactly once");
    CHECK(rows[count - 1].sequence == DM1_RUNTIME_EVENT_RECEIPT_CAPACITY + 2u &&
              cursor == DM1_RUNTIME_EVENT_RECEIPT_CAPACITY + 2u,
          "drain cursor advances through newest receipt");

    puts("PASS: DM1 runtime receipt ring preserves sequenced source transitions");
    return 0;
}
