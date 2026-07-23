#include "dm1_v1_group_state_bundle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", label); \
        return 1; \
    } \
} while (0)

static int make_group_thing(int index)
{
    return (THING_TYPE_GROUP << 10) | index;
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[2];
    unsigned char raw[32];
    DM1_V1_SourceActiveGroupPc34Compat rows[2];
    DM1_V1_GroupStateBundleReceiptPc34Compat receipt;
    struct CreatureAIState_Compat before;

    memset(&world, 0x5a, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(raw, 0, sizeof(raw));
    memset(rows, 0, sizeof(rows));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 6;
    groups[1].next = THING_ENDOFLIST;
    groups[1].creatureType = 12;
    things.loaded = 1;
    things.groups = groups;
    things.groupCount = 2;
    things.rawThingData[THING_TYPE_GROUP] = raw;
    things.thingCounts[THING_TYPE_GROUP] = 2;
    world.things = &things;
    world.partyMapIndex = 3;

    CHECK(dm1_v1_group_state_initialize_f0196_pc34(&world, &receipt),
          "F0196 initializes the source-owned PC34 active range");
    CHECK(receipt.valid && receipt.activeGroupCount == 0 &&
              receipt.sourceCapacity == DM1_PC34_ACTIVE_GROUP_CAPACITY &&
              world.creatureAI[0].reserved0 == -1 &&
              world.creatureAI[DM1_PC34_ACTIVE_GROUP_CAPACITY - 1].reserved0 == -1 &&
              world.creatureAI[DM1_PC34_ACTIVE_GROUP_CAPACITY].reserved0 ==
                  (int)0x5a5a5a5a,
          "F0196 preserves host-only spare state and emits a receipt");

    rows[0].groupThing = make_group_thing(0);
    rows[0].cells = 0xe4;
    rows[0].directions = 0x1b;
    rows[0].lastMoveTime = 71;
    rows[0].delayFleeingFromTarget = 9;
    rows[0].targetMapX = 7;
    rows[0].targetMapY = 8;
    rows[0].priorMapX = 2;
    rows[0].priorMapY = 4;
    rows[0].homeMapX = 1;
    rows[0].homeMapY = 5;
    rows[0].aspect[0] = 0x80u;
    rows[1] = rows[0];
    rows[1].groupThing = make_group_thing(1);
    rows[1].cells = 0xff;
    rows[1].directions = 0x55;

    CHECK(dm1_v1_group_state_apply_save_handoff_pc34(
              &world, rows, 2, 4, &receipt),
          "F0435 C04 handoff admits all source-owned rows atomically");
    CHECK(receipt.valid && receipt.activeGroupCount == 2 &&
              receipt.sourceCapacity == 4 && receipt.fingerprint != 0u &&
              world.creatureAICount == 2 &&
              world.pc34ActiveGroupSourceCount == 4 &&
              world.creatureAI[0].reserved0 == 0 &&
              world.creatureAI[0].groupCells == 0xe4 &&
              world.pc34ActiveGroupDirections[0] == 0x1bu &&
              world.pc34ActiveGroupHomeMapY[0] == 5u &&
              world.creatureAI[1].reserved0 == 1,
          "F0435 retains cells, packed directions, owner, and home coordinates");

    raw[5] = 0x33u;
    raw[15] = 0x02u;
    CHECK(dm1_v1_group_state_write_f0146_f0148_pc34(
              &world, 3, 0, 0xc6u, 0x96u, &receipt),
          "F0146/F0148 update the party-map ACTIVE_GROUP owner together");
    CHECK(world.creatureAI[0].groupCells == 0xc6 &&
              world.creatureAI[0].groupDirection == 0x96 &&
              world.pc34ActiveGroupDirections[0] == 0x96u &&
              raw[5] == 0x33u && raw[15] == 0x02u,
          "party-map transaction does not leak into raw C04");
    CHECK(dm1_v1_group_state_write_f0146_f0148_pc34(
              &world, 2, 0, 0x7eu, 0xffu, &receipt),
          "F0146/F0148 update non-party raw C04 together");
    CHECK(groups[0].cells == 0x7eu && groups[0].direction == 3u &&
              raw[5] == 0x7eu && (raw[15] & 3u) == 3u,
          "non-party transaction commits normalized raw C04 values");

    before = world.creatureAI[0];
    rows[1].directions = 0x100;
    CHECK(!dm1_v1_group_state_apply_save_handoff_pc34(
               &world, rows, 2, 4, &receipt),
          "malformed second C04 row rejects before runtime publication");
    CHECK(memcmp(&before, &world.creatureAI[0], sizeof(before)) == 0 &&
              world.creatureAICount == 2 && world.pc34ActiveGroupSourceCount == 4,
          "rejected C04 handoff rolls back the entire existing group state");

    puts("PASS: DM1 F0145/F0146/F0147/F0196 group-state save bundle");
    return 0;
}
