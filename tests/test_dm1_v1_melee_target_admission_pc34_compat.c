#include "dm1_v1_melee_target_admission_pc34_compat.h"

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
    struct DM1ActiveGroup_Compat active;
    struct TimelineEvent_Compat event;
    struct RngState_Compat rng;
    struct RngState_Compat rngBefore;
    DM1_MeleeTargetAdmissionInputPc34 input;
    DM1_MeleeTargetAdmissionReceiptPc34 receipt;
    unsigned char rawC04[16] = {
        0xfe, 0xff, 0xff, 0xff, 2, 0x04, 0, 0,
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
    group.creatureType = 2;
    group.cells = 0x04;
    group.count = 1;
    group.direction = 1;
    active.groupThingIndex = 0;
    active.cells = 0x04;
    active.directions = 1;
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = 55u;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = 0;
    event.aux1 = 2;
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    rng.seed = 0x13579bdfu;
    rngBefore = rng;
    memset(&input, 0, sizeof(input));
    input.things = &things;
    input.groupIndex = 0;
    input.group = &group;
    input.activeGroup = &active;
    input.event = &event;
    input.partyMapIndex = 0;
    input.partyMapX = 2;
    input.partyMapY = 1;
    input.creatureIndex = 0;
    input.rng = &rng;

    ok &= check(dm1_v1_melee_target_admit_f0229_f0230_pc34(
                    &input, &receipt) && receipt.valid &&
                    receipt.targetMapIndex == 0 && receipt.targetMapX == 2 &&
                    receipt.targetMapY == 1 && receipt.rawC04FNV1a != 0u &&
                    receipt.c29C41FNV1a != 0u && receipt.orderedCells[0] >= 0 &&
                    receipt.orderedCells[0] <= 3,
                "F0229/F0230 admits matching C04 C38 and party coordinates");
    ok &= check(memcmp(&rng, &rngBefore, sizeof(rng)) == 0,
                "F0229 preview does not consume F0230 live RNG");
    rawC04[5] = 0x05;
    ok &= check(dm1_v1_melee_target_admit_f0229_f0230_pc34(
                    &input, &receipt) && !receipt.valid,
                "C04 cell drift blocks melee target admission");
    rawC04[5] = 0x04;
    input.partyMapIndex = 1;
    ok &= check(dm1_v1_melee_target_admit_f0229_f0230_pc34(
                    &input, &receipt) && !receipt.valid,
                "cross-map party target cannot reach F0230 preparation");

    if (!ok) return 1;
    puts("PASS: DM1 F0229/F0230 PC34 melee target admission");
    return 0;
}
