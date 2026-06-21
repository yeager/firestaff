#include <stdio.h>
#include <string.h>

#include "memory_creature_ai_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) \
    do { \
        if (cond) { \
            ++g_pass; \
        } else { \
            ++g_fail; \
            fprintf(stderr, "FAIL: %s\n", msg); \
        } \
    } while (0)

static void build_scenario(struct CreatureTickInput_Compat* in,
                           int creatureType,
                           int gx,
                           int gy,
                           int mapIndex,
                           int distance,
                           int losClear)
{
    int i;
    memset(in, 0, sizeof(*in));
    in->groupSlotIndex = 0;
    in->creatureType = creatureType;
    in->groupMapIndex = mapIndex;
    in->groupMapX = gx;
    in->groupMapY = gy;
    in->groupCells = 0x01;
    in->groupCurrentHealth[0] = 100;
    in->partyMapIndex = mapIndex;
    in->partyMapX = gx + distance;
    in->partyMapY = gy;
    in->partyChampionsAlive = 0x0F;
    for (i = 0; i < 4; ++i) {
        in->partyChampionCurrentHealth[i] = 60;
    }
    in->currentTickLow = 1000;
    in->losClearFlag = losClear ? 1 : 0;
    in->primaryDir = 1;   /* east toward party */
    in->secondaryDir = 2; /* south */
}

int main(void)
{
    const struct CreatureBehaviorProfile_Compat* skeleton =
        CREATURE_GetProfile_Compat(CREATURE_TYPE_SKELETON);

    printf("DM1 V1 AI Pathfinding CTest Gate\n");
    printf("Source: ReDMCSB GROUP.C F0202/F0203, one-step greedy cascade\n\n");

    CHECK(skeleton != 0, "profile: skeleton exists");
    if (!skeleton) {
        return 1;
    }

    {
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        int dir = -2;
        int rc;

        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyWallMask = 0;
        F0730_COMBAT_RngInit_Compat(&rng, 42);
        rc = F0799_CREATURE_PickMoveDirection_Compat(
            &in, skeleton, 1, 2, 0, &rng, &dir);
        CHECK(rc == 1 && dir == 1,
              "F0799: primary direction open -> east");
    }

    {
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        int dir = -2;
        int rc;

        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyWallMask = 0x02; /* east blocked */
        F0730_COMBAT_RngInit_Compat(&rng, 0);
        rc = F0799_CREATURE_PickMoveDirection_Compat(
            &in, skeleton, 1, 2, 0, &rng, &dir);
        CHECK(rc == 1 && dir == 2,
              "F0799: blocked primary + roll2=0 -> secondary south");
    }

    {
        struct CreatureTickInput_Compat in;
        int blocker = -1;
        int rc;

        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyDoorMask = 0x02;
        rc = F0798_CREATURE_IsDirectionOpen_Compat(
            &in, skeleton, 1, 0, &blocker);
        CHECK(rc == 0 && blocker == 3,
              "F0798: closed door blocks non-material skeleton");
    }

    {
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        int dir = -2;
        int rc;

        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyCreatureMask = 0x02; /* creature east */
        in.adjacencyWallMask = 0x04;     /* wall south */
        F0730_COMBAT_RngInit_Compat(&rng, 0);
        rc = F0799_CREATURE_PickMoveDirection_Compat(
            &in, skeleton, 1, 2, 0, &rng, &dir);
        CHECK(rc == 1 && dir == 3,
              "F0799: primary and secondary blocked -> opposite west");
    }

    {
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        int dir = -2;
        int rc;

        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyWallMask = 0x0F;
        F0730_COMBAT_RngInit_Compat(&rng, 7);
        rc = F0799_CREATURE_PickMoveDirection_Compat(
            &in, skeleton, 1, 2, 0, &rng, &dir);
        CHECK(rc == 0 && dir == -1,
              "F0799: all directions blocked -> no move");
    }

    printf("--- Results: %d PASS, %d FAIL ---\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
