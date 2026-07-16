#include "dm1_v1_group_active_state_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *label)
{
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

int main(void)
{
    struct DM1ActiveGroup_Compat active_groups[2];
    struct DungeonGroup_Compat groups[2];
    DM1_V1_F0183_AddActiveGroupReceipt_PC34 add_receipt;
    DM1_V1_F0184_RemoveActiveGroupReceipt_PC34 remove_receipt;
    int active_count = 0;

    memset(active_groups, 0x7f, sizeof(active_groups));
    memset(groups, 0, sizeof(groups));
    groups[0].cells = 0xe4u;
    groups[0].count = 1u;
    groups[0].direction = 2u;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    groups[1].cells = 0x55u;
    groups[1].count = 0u;
    groups[1].direction = 3u;
    groups[1].behavior = DM1_BEHAVIOR_USELESS3;

    check(F0183_DM1_GROUP_AddActiveGroup_Compat(
              active_groups, 2, &active_count, &groups[0], 0,
              7, 8, 0x1234u, &add_receipt) == 1,
          "F0183 adds first active group");
    check(active_count == 1 && add_receipt.valid,
          "F0183 increments active group count with receipt");
    check(active_groups[0].groupThingIndex == 0 &&
              active_groups[0].cells == 0xe4 &&
              active_groups[0].directions == 0x0a,
          "F0183 copies C04 cells and packs the source direction");
    check(active_groups[0].lastMoveTime == 0x34 &&
              active_groups[0].targetMapX == 7 &&
              active_groups[0].homeMapY == 8,
          "F0183 stores source square and low game-time byte");
    check(add_receipt.source_symbol &&
              strcmp(add_receipt.source_symbol,
                     "F0183_GROUP_AddActiveGroup") == 0,
          "F0183 receipt is source named");

    check(F0183_DM1_GROUP_AddActiveGroup_Compat(
              active_groups, 1, &active_count, &groups[1], 1,
              1, 2, 0u, &add_receipt) == 0,
          "F0183 fails closed at active-group capacity");
    check(active_count == 1 && active_groups[1].groupThingIndex ==
              (int)0x7f7f7f7f,
          "F0183 capacity failure does not write the next slot");

    active_groups[0].cells = 0xa5;
    active_groups[0].directions = 0x06;
    groups[0].behavior = DM1_BEHAVIOR_USELESS4;
    check(F0184_DM1_GROUP_RemoveActiveGroup_Compat(
              &active_groups[0], groups, 2, &remove_receipt) == 1,
          "F0184 removes active group");
    check(groups[0].cells == 0xa5 && groups[0].direction == 2,
          "F0184 writes active cells and low direction back to C04");
    check(groups[0].behavior == DM1_BEHAVIOR_WANDER &&
              remove_receipt.reset_behavior_to_wander,
          "F0184 resets C4+ behavior to wander");
    check(active_groups[0].groupThingIndex == -1 &&
              remove_receipt.retired_active_slot,
          "F0184 retires the active slot");
    check(remove_receipt.source_symbol &&
              strcmp(remove_receipt.source_symbol,
                     "F0184_GROUP_RemoveActiveGroup") == 0,
          "F0184 receipt is source named");

    active_groups[0].groupThingIndex = 3;
    groups[0].cells = 0x11u;
    check(F0184_DM1_GROUP_RemoveActiveGroup_Compat(
              &active_groups[0], groups, 2, &remove_receipt) == 0,
          "F0184 rejects out-of-range C04 references");
    check(groups[0].cells == 0x11u,
          "F0184 rejects before mutating raw C04 data");

    if (failures != 0) return 1;
    puts("PASS: DM1 F0183/F0184 active group source state");
    return 0;
}
