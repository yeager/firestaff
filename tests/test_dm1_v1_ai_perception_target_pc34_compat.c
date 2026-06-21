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

static void zero_state(struct CreatureAIState_Compat* s)
{
    memset(s, 0, sizeof(*s));
}

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
    in->primaryDir = 1;
    in->secondaryDir = 2;
}

int main(void)
{
    printf("DM1 V1 AI Perception/Target CTest Gate\n");
    printf("Source: ReDMCSB GROUP.C F0197/F0198/F0200/F0201, target selection seam\n\n");

    {
        struct CreatureAIState_Compat sIn, s1, s2;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat r1, r2;
        struct CreatureTickResult_Compat out1, out2;

        zero_state(&sIn);
        sIn.stateKind = AI_STATE_APPROACH;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 10, 10, 0, 3, 1);
        F0730_COMBAT_RngInit_Compat(&r1, 0xC0FFEEu);
        F0730_COMBAT_RngInit_Compat(&r2, 0xC0FFEEu);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &r1, &s1, &out1);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &r2, &s2, &out2);
        CHECK(memcmp(&s1, &s2, sizeof(s1)) == 0 &&
              memcmp(&out1, &out2, sizeof(out1)) == 0 &&
              r1.seed == r2.seed,
              "F0804: same state/input/rng -> deterministic output");
    }

    {
        struct CreatureAIState_Compat s;
        struct CreatureTickInput_Compat in;
        int next = -1;
        int aggr = 0;

        zero_state(&s);
        s.stateKind = AI_STATE_IDLE;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 3, 1);
        F0793_CREATURE_ComputeNextState_Compat(
            &s, &in, 1, 0, &next, &aggr);
        CHECK(next == AI_STATE_WANDER && aggr > 0,
              "F0793: IDLE + visible living party -> WANDER");
    }

    {
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;

        zero_state(&sIn);
        sIn.stateKind = AI_STATE_APPROACH;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 5, 5, 0, 1, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 7);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        CHECK(out.resultKind == AI_RESULT_ATTACKED &&
              out.emittedCombatAction == 1 &&
              sOut.stateKind == AI_STATE_ATTACK,
              "F0804: adjacent approach promotes to attack action");
    }

    {
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_STONE_GOLEM);
        struct CreatureTickInput_Compat in;
        int visible = -1;
        int dist = -1;
        int smell = -1;

        build_scenario(&in, CREATURE_TYPE_STONE_GOLEM, 0, 0, 0, 3, 1);
        F0792_CREATURE_Perceive_Compat(&in, prof, &visible, &dist, &smell);
        CHECK(prof && prof->sightRange == 3 && visible == 1 &&
              dist == 3 && smell == 0,
              "F0792: distance at sightRange with LoS -> visible");
    }

    {
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_STONE_GOLEM);
        struct CreatureTickInput_Compat in;
        int visible = -1;
        int dist = -1;
        int smell = -1;

        build_scenario(&in, CREATURE_TYPE_STONE_GOLEM, 0, 0, 0, 4, 1);
        F0792_CREATURE_Perceive_Compat(&in, prof, &visible, &dist, &smell);
        CHECK(visible == 0 && dist == 0,
              "F0792: distance beyond sightRange -> not visible");
    }

    {
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_SKELETON);
        struct CreatureTickInput_Compat in;
        int visible = -1;
        int dist = -1;
        int smell = -1;

        build_scenario(&in, CREATURE_TYPE_SKELETON, 0, 0, 0, 1, 0);
        F0792_CREATURE_Perceive_Compat(&in, prof, &visible, &dist, &smell);
        CHECK(visible == 0,
              "F0792: blocked LoS prevents sight even at distance 1");
    }

    {
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_MUMMY);
        struct CreatureTickInput_Compat in;
        int visible = -1;
        int dist = -1;
        int smell = -1;

        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 2, 1);
        in.partyInvisibility = 1;
        F0792_CREATURE_Perceive_Compat(&in, prof, &visible, &dist, &smell);
        CHECK(prof && (prof->attributes & CREATURE_ATTR_MASK_SEE_INVISIBLE) == 0 &&
              visible == 0 && smell == 1,
              "F0792: invisibility blocks sight but smell fallback remains");
    }

    {
        struct CreatureTickInput_Compat in;
        int idx = -2;

        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 1, 1);
        in.partyChampionsAlive = 0x0F;
        in.primaryDir = 1; /* east row: { 1, 0, 3, 2 } */
        CHECK(F0796_CREATURE_PickChampion_Compat(&in, &idx) == 1 && idx == 1,
              "F0796: east-facing source cell order targets champion 1 first");
    }

    {
        struct CreatureTickInput_Compat in;
        int idx = -2;

        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 1, 1);
        in.partyChampionsAlive = 0x0E;
        in.partyChampionCurrentHealth[0] = 0;
        CHECK(F0796_CREATURE_PickChampion_Compat(&in, &idx) == 1 && idx == 1,
              "F0796: dead champion 0 skipped by ordered-cell target pick");
    }

    {
        struct CreatureTickInput_Compat in;
        int idx = 42;
        int ret;

        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 1, 1);
        in.partyChampionsAlive = 0x00;
        in.partyChampionCurrentHealth[0] = 0;
        in.partyChampionCurrentHealth[1] = 0;
        in.partyChampionCurrentHealth[2] = 0;
        in.partyChampionCurrentHealth[3] = 0;
        ret = F0796_CREATURE_PickChampion_Compat(&in, &idx);
        CHECK(ret == 0 && idx == -1,
              "F0796: all champions dead -> no target");
    }

    printf("--- Results: %d PASS, %d FAIL ---\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
