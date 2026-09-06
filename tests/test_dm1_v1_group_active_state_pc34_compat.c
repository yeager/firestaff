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
    struct DM1CreatureInfo_Compat creature_info;
    struct TimelineEvent_Compat wander_event;
    struct RngState_Compat rng;
    DM1_V1_F0179_CreatureAspectUpdateReceipt_PC34 aspect_receipt;
    DM1_V1_F0180_StartWanderingReceipt_PC34 wander_receipt;
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
    memset(&creature_info, 0, sizeof(creature_info));
    creature_info.graphicInfo = 0;
    creature_info.animationTicks = 0x03a0;

    active_groups[0].groupThingIndex = 0;
    active_groups[0].aspect[0] = 0xff;
    active_groups[0].aspect[1] = 0x55;
    active_groups[0].aspect[2] = 0xaa;
    active_groups[0].aspect[3] = 0xff;
    rng.seed = 7u;
    check(F0179_DM1_GROUP_GetCreatureAspectUpdateTime_Compat(
              &active_groups[0], &groups[0], &creature_info, -1, 0,
              1000u, &rng, &aspect_receipt) == 1,
          "F0179 updates every creature when index is negative");
    check(active_groups[0].aspect[0] == 0 &&
              active_groups[0].aspect[1] == 0 &&
              active_groups[0].aspect[2] == 0xaa &&
              active_groups[0].aspect[3] == 0xff,
          "F0179 non-attack group update clears only source creature aspects");
    check(aspect_receipt.valid &&
              aspect_receipt.processed_first_index == 1 &&
              aspect_receipt.processed_last_index == 0 &&
              aspect_receipt.processed_count == 2 &&
              aspect_receipt.next_update_time >= 1010u &&
              aspect_receipt.next_update_time <= 1011u,
          "F0179 non-attack receipt preserves group span and timing window");
    check(aspect_receipt.source_symbol &&
              strcmp(aspect_receipt.source_symbol,
                     "F0179_GROUP_GetCreatureAspectUpdateTime") == 0,
          "F0179 receipt is source named");

    active_groups[0].aspect[1] = 0;
    rng.seed = 11u;
    check(F0179_DM1_GROUP_GetCreatureAspectUpdateTime_Compat(
              &active_groups[0], &groups[0], &creature_info, 1, 1,
              2000u, &rng, &aspect_receipt) == 1,
          "F0179 updates a single attacking creature");
    check(active_groups[0].aspect[0] == 0 &&
              active_groups[0].aspect[1] == 0x80 &&
              aspect_receipt.processed_count == 1 &&
              aspect_receipt.next_update_time >= 2003u &&
              aspect_receipt.next_update_time <= 2004u,
          "F0179 attacking update sets the attack latch and attack cadence");

    active_groups[0].aspect[0] = 0x11;
    rng.seed = 13u;
    check(F0179_DM1_GROUP_GetCreatureAspectUpdateTime_Compat(
              &active_groups[0], &groups[0], &creature_info, 2, 0,
              1u, &rng, &aspect_receipt) == 0,
          "F0179 rejects creature indices beyond raw C04 Count");
    check(active_groups[0].aspect[0] == 0x11,
          "F0179 rejects before mutating active aspects");

    check(F0180_DM1_GROUP_StartWandering_Compat(
              5, 12, 2, 7, 8, 100u, &wander_event,
              &wander_receipt) == 1,
          "F0180 builds a wander timeline event");
    check(wander_event.kind == TIMELINE_EVENT_CREATURE_REACTION &&
              wander_event.fireAtTick == 101u &&
              wander_event.mapIndex == 2 &&
              wander_event.mapX == 7 &&
              wander_event.mapY == 8 &&
              wander_event.aux0 == 5 &&
              wander_event.aux1 == 12 &&
              wander_event.aux2 == DM1_EVENT_UPDATE_BEHAVIOR_GROUP &&
              wander_event.aux3 == 0 && wander_event.aux4 == 0x100,
          "F0180 schedules C37 wander at game time plus one");
    check(wander_receipt.valid && wander_receipt.source_symbol &&
              strcmp(wander_receipt.source_symbol,
                     "F0180_GROUP_StartWandering") == 0,
          "F0180 receipt is source named");
    check(F0180_DM1_GROUP_StartWandering_Compat(
              -1, 12, 2, 7, 8, 100u, &wander_event,
              &wander_receipt) == 0,
          "F0180 rejects invalid group references");

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
    check(active_groups[0].lastMoveTime == 0xb5 &&
          add_receipt.last_move_time == 0xb5 &&
              active_groups[0].targetMapX == 7 &&
              active_groups[0].homeMapY == 8,
          "F0183 stores source square and byte-wrapped game time minus 127");
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
