#include "dm1_v1_group_los_direction_admission_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat group;
    struct CreatureAIState_Compat active;
    struct TimelineEvent_Compat event;
    struct RngState_Compat rng;
    DM1_GroupLosDirectionAdmissionInputPc34 input;
    DM1_GroupLosDirectionAdmissionReceiptPc34 receipt;
    unsigned char rawC04[16] = {
        0xfe, 0xff, 0xff, 0xff, 3, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1
    };
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(&group, 0, sizeof(group));
    memset(&active, 0, sizeof(active));
    memset(&event, 0, sizeof(event));
    memset(&rng, 0, sizeof(rng));
    things.loaded = 1;
    things.groups = &group;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.rawThingData[THING_TYPE_GROUP] = rawC04;
    group.next = THING_ENDOFLIST;
    group.slot = THING_NONE;
    group.creatureType = 3;
    group.cells = 0;
    group.direction = 1;
    active.reserved0 = 0;
    active.groupMapIndex = 0;
    active.groupMapX = 4;
    active.groupMapY = 4;
    active.groupCells = 0;
    active.groupDirection = 1;
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = 99u;
    event.mapIndex = 0;
    event.mapX = 4;
    event.mapY = 4;
    event.aux0 = 0;
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    rng.seed = 0x12345678u;
    memset(&input, 0, sizeof(input));
    input.things = &things;
    input.groupIndex = 0;
    input.activeGroup = &active;
    input.activeDirections = 1;
    input.event = &event;
    input.partyMapIndex = 0;
    input.partyMapX = 7;
    input.partyMapY = 4;
    input.rng = &rng;
    ok &= check(dm1_v1_group_los_direction_admit_f0227_f0228_pc34(
                    &input, &receipt) && receipt.valid &&
                    receipt.primaryDirection == 1 && receipt.rawC04FNV1a != 0u &&
                    receipt.c29C41FNV1a != 0u &&
                    receipt.rngBeforeFNV1a != receipt.rngAfterFNV1a,
                "F0227/F0228 admits matching raw C04, C29-C41 and RNG relation");
    rawC04[5] = 1;
    ok &= check(dm1_v1_group_los_direction_admit_f0227_f0228_pc34(
                    &input, &receipt) && !receipt.valid,
                "raw C04 drift blocks live direction consumption");
    rawC04[5] = 0;
    input.partyMapIndex = 1;
    ok &= check(dm1_v1_group_los_direction_admit_f0227_f0228_pc34(
                    &input, &receipt) && !receipt.valid,
                "cross-map party relation cannot consume F0228 direction");

    if (!ok) return 1;
    puts("PASS: DM1 F0227/F0228 PC34 live group direction admission");
    return 0;
}
