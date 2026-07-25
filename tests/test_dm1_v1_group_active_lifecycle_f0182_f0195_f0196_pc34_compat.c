#include "dm1_v1_group_active_lifecycle_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int count;
    uint16_t thing[4];
    int16_t x[4];
    int16_t y[4];
} EventLog;

static void log_group_event(void* context, uint16_t thing, int16_t x, int16_t y)
{
    EventLog* log = (EventLog*)context;
    assert(log != NULL);
    assert(log->count < 4);
    log->thing[log->count] = thing;
    log->x[log->count] = x;
    log->y[log->count] = y;
    ++log->count;
}

static void log_delete_events(void* context, int16_t x, int16_t y)
{
    EventLog* log = (EventLog*)context;
    assert(log != NULL);
    assert(log->count < 4);
    log->thing[log->count] = 0xFFFFu;
    log->x[log->count] = x;
    log->y[log->count] = y;
    ++log->count;
}

static uint16_t get_next_thing(void* context, uint16_t thing)
{
    (void)context;
    if (thing == 0x0002u) {
        return 0x1007u;
    }
    if (thing == 0x1007u) {
        return 0x1008u;
    }
    return DM1_V1_F0195_THING_END_OF_LIST_PC34;
}

static void test_f0182_clears_all_attack_bits_and_deletes_events(void)
{
    DM1_V1_F0182_ActiveGroupPc34Compat group = {
        {0x80u, 0x81u, 0x7Fu, 0xFFu}
    };
    EventLog deleted = {0, {0}, {0}, {0}};

    F0182_GROUP_StopAttacking(&group, 12, 9, log_delete_events, &deleted);

    assert(group.aspect[0] == 0x00u);
    assert(group.aspect[1] == 0x01u);
    assert(group.aspect[2] == 0x7Fu);
    assert(group.aspect[3] == 0x7Fu);
    assert(deleted.count == 1);
    assert(deleted.x[0] == 12);
    assert(deleted.y[0] == 9);
}

static void test_f0195_scans_square_first_group_and_source_event_order(void)
{
    const uint8_t squares[] = {
        0x00u, 0x10u,
        0x10u, 0x00u
    };
    const uint16_t firstThings[] = {
        0x0002u,
        0x100Au
    };
    EventLog deleted = {0, {0}, {0}, {0}};
    EventLog added = {0, {0}, {0}, {0}};
    EventLog wandering = {0, {0}, {0}, {0}};
    DM1_V1_F0195_AddAllActiveGroupsInputPc34Compat input;

    memset(&input, 0, sizeof(input));
    input.mapSquares = squares;
    input.mapSquareCount = sizeof(squares);
    input.squareFirstThings = firstThings;
    input.squareFirstThingCount = 2u;
    input.mapWidth = 2;
    input.mapHeight = 2;
    input.getNextThing = get_next_thing;
    input.deleteEvents = log_group_event;
    input.deleteEventsContext = &deleted;
    input.addActiveGroup = log_group_event;
    input.addActiveGroupContext = &added;
    input.startWandering = log_group_event;
    input.startWanderingContext = &wandering;

    assert(F0195_GROUP_AddAllActiveGroups(&input) == 2);

    assert(deleted.count == 2);
    assert(added.count == 2);
    assert(wandering.count == 2);
    assert(added.thing[0] == 0x1007u);
    assert(added.x[0] == 0);
    assert(added.y[0] == 1);
    assert(added.thing[1] == 0x100Au);
    assert(added.x[1] == 1);
    assert(added.y[1] == 0);
}

static void test_f0196_new_game_and_minimum_capacity(void)
{
    DM1_V1_F0196_ActiveGroupSlotPc34Compat slots[112];
    DM1_V1_F0196_InitializeActiveGroupsInputPc34Compat input;
    int i;

    for (i = 0; i < 112; ++i) {
        slots[i].groupThingIndex = 123;
    }

    input.newGame = 1;
    input.maximumActiveGroupCount = 7;
    input.slots = slots;
    input.slotCapacity = 112u;

    assert(F0196_GROUP_InitializeActiveGroups(&input) == 110);
    assert(input.maximumActiveGroupCount == 110);
    for (i = 0; i < 110; ++i) {
        assert(slots[i].groupThingIndex == -1);
    }
    assert(slots[111].groupThingIndex == 123);
}

static void test_f0196_rejects_small_storage_before_write(void)
{
    DM1_V1_F0196_ActiveGroupSlotPc34Compat slots[2] = {{7}, {8}};
    DM1_V1_F0196_InitializeActiveGroupsInputPc34Compat input;

    input.newGame = 0;
    input.maximumActiveGroupCount = 5;
    input.slots = slots;
    input.slotCapacity = 2u;

    assert(F0196_GROUP_InitializeActiveGroups(&input) == -1);
    assert(slots[0].groupThingIndex == 7);
    assert(slots[1].groupThingIndex == 8);
}

static void test_source_evidence_names_all_three_symbols(void)
{
    const char* evidence = DM1_V1_GroupActiveLifecycle_SourceEvidencePc34();
    (void)evidence;

    assert(evidence != NULL);
    assert(strstr(evidence, "F0182") != NULL);
    assert(strstr(evidence, "F0195") != NULL);
    assert(strstr(evidence, "F0196") != NULL);
}

int main(void)
{
    test_f0182_clears_all_attack_bits_and_deletes_events();
    test_f0195_scans_square_first_group_and_source_event_order();
    test_f0196_new_game_and_minimum_capacity();
    test_f0196_rejects_small_storage_before_write();
    test_source_evidence_names_all_three_symbols();

    puts("ok: DM1 F0182/F0195/F0196 active-group lifecycle");
    return 0;
}
