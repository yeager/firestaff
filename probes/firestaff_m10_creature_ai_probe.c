/*
 * M10 Phase 16 probe — Creature AI / monster behavior verification.
 *
 * Validates the pure creature-tick data layer:
 *   - Struct sizes + MEDIA016 LSB-first round-trips
 *     (F0805/F0806, F0807/F0808, F0809a/F0809b)
 *   - Perception (F0790-F0792) sight/smell/invisibility boundaries
 *   - State machine transitions (F0793) + orchestrator integration
 *   - Target selection (F0796)
 *   - Pathfinding cascade (F0798/F0799): primary/secondary/opposite/fallback/blocked
 *   - Attack emission (F0800) + cooldown honouring
 *   - Next-tick emission (F0802) + infinite-loop clamp (delay >= 1)
 *   - Stub-path meta-sweep over the 24 non-fully-implemented creature types
 *   - Boundary + purity + CRC32 invariants
 *   - 100-iteration RNG loop-guard sweep (R7 mitigation)
 *   - Real DUNGEON.DAT integration: one real creature group
 *
 * 40 invariants, target >= 30, shipped 40.
 *
 * Authoritative plan: PHASE16_PLAN.md §5.
 *
 * NOTE: F0736_COMBAT_ResolveCreatureMelee_Compat consumes a
 * CombatantCreatureSnapshot_Compat (not a CombatAction_Compat). The
 * Phase 13 contract is: the AI emits the CombatAction to schedule the
 * hit; the resolver runs on a pre-built creature snapshot carrying the
 * same profile values. Invariant 31 asserts this structural parity —
 * the emitted rawAttackValue / attackTypeCode match the snapshot the
 * resolver is fed, and F0736 returns 1 on the snapshot.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "memory_creature_ai_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

/* ---------- Local CRC32 (IEEE 802.3, MSB-first, poly 0xEDB88320 reflected) -- */

static uint32_t local_crc32(const unsigned char* buf, size_t n) {
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;
    int b;
    for (i = 0; i < n; i++) {
        crc ^= (uint32_t)buf[i];
        for (b = 0; b < 8; b++) {
            crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1u));
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

/* ---------- Fixture builders ----------------------------------------------- */

static void zero_state(struct CreatureAIState_Compat* s) {
    memset(s, 0, sizeof(*s));
}

static void fill_state_populated(struct CreatureAIState_Compat* s) {
    memset(s, 0, sizeof(*s));
    s->stateKind             = AI_STATE_APPROACH;
    s->creatureType          = CREATURE_TYPE_SKELETON;
    s->groupMapIndex         = 3;
    s->groupMapX             = 5;
    s->groupMapY             = 7;
    s->groupCells            = 0x33;
    s->groupDirection        = 1;
    s->targetChampionIndex   = 2;
    s->lastSeenPartyMapX     = 11;
    s->lastSeenPartyMapY     = 13;
    s->lastSeenPartyTick     = 0x12345678;
    s->fearCounter           = 17;
    s->turnCounter           = 42;
    s->attackCooldownTicks   = 3;
    s->movementCooldownTicks = 2;
    s->aggressionScore       = 75;
    s->rngCallCount          = 9;
    s->reserved0             = 0x7EADBEEF;
}

static void fill_input_populated(struct CreatureTickInput_Compat* in) {
    int i;
    memset(in, 0, sizeof(*in));
    in->groupSlotIndex       = 11;
    in->creatureType         = CREATURE_TYPE_MUMMY;
    in->groupMapIndex        = 2;
    in->groupMapX            = 10;
    in->groupMapY            = 12;
    in->groupCells           = 0x05;
    for (i = 0; i < 4; i++) in->groupCurrentHealth[i] = 40 + i;
    in->partyMapIndex        = 2;
    in->partyMapX            = 11;
    in->partyMapY            = 12;
    in->partyChampionsAlive  = 0x0F;
    for (i = 0; i < 4; i++) in->partyChampionCurrentHealth[i] = 50 + i * 3;
    in->adjacencyWallMask    = 0x02;
    in->adjacencyDoorMask    = 0x04;
    in->adjacencyPitMask     = 0x08;
    in->adjacencyCreatureMask= 0x01;
    in->onFluxcageFlag       = 0;
    in->onPoisonCloudFlag    = 0;
    in->onPitFlag            = 0;
    in->selfDamagePerTick    = 0;
    in->currentTickLow       = 0x00ABCDEF;
    in->freezeLifeTicks      = 0;
    in->partyInvisibility    = 0;
    in->losClearFlag         = 1;
    in->primaryDir           = 1;
    in->secondaryDir         = 2;
}

/*
 * Build an input scenario with the group at (gx, gy, mapIndex) and the
 * party 'distance' tiles east (along +x). All four champions alive, full
 * health. No hazards. LoS clear. primary=east, secondary=south.
 */
static void build_scenario(
    struct CreatureTickInput_Compat* in,
    int creatureType, int gx, int gy, int mapIndex, int distance, int losClear)
{
    int i;
    memset(in, 0, sizeof(*in));
    in->groupSlotIndex       = 0;
    in->creatureType         = creatureType;
    in->groupMapIndex        = mapIndex;
    in->groupMapX            = gx;
    in->groupMapY            = gy;
    in->groupCells           = 0x01;
    in->groupCurrentHealth[0]= 100;
    in->partyMapIndex        = mapIndex;
    in->partyMapX            = gx + distance;  /* east */
    in->partyMapY            = gy;
    in->partyChampionsAlive  = 0x0F;
    for (i = 0; i < 4; i++) in->partyChampionCurrentHealth[i] = 60;
    in->currentTickLow       = 1000;
    in->losClearFlag         = losClear ? 1 : 0;
    in->primaryDir           = 1;   /* east toward party */
    in->secondaryDir         = 2;   /* south */
}

static int is_full_tier(int t) {
    return (t == CREATURE_TYPE_STONE_GOLEM) ||
           (t == CREATURE_TYPE_MUMMY)       ||
           (t == CREATURE_TYPE_SKELETON);
}

int main(int argc, char* argv[]) {
    FILE* report;
    FILE* invariants;
    char path_buf[512];
    int failCount = 0;
    int invariantCount = 0;
    const char* dungeonPath;
    const char* outputDir;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
        return 1;
    }
    dungeonPath = argv[1];
    outputDir   = argv[2];

    snprintf(path_buf, sizeof(path_buf), "%s/creature_ai_probe.md", outputDir);
    report = fopen(path_buf, "w");
    if (!report) { fprintf(stderr, "FAIL: cannot write report\n"); return 1; }
    fprintf(report, "# M10 Phase 16: Creature AI / Monster Behavior Probe\n\n");
    fprintf(report, "## Scope (v1)\n\n");
    fprintf(report, "- CREATURE_TICK pure-transform orchestrator (F0804)\n");
    fprintf(report, "- Perception + state machine + target selection (F0790-F0796)\n");
    fprintf(report, "- Pathfinding cascade (F0798/F0799)\n");
    fprintf(report, "- Action emission (F0800/F0801/F0802/F0803)\n");
    fprintf(report, "- Serialisation + round-trip (F0805..F0809)\n");
    fprintf(report, "- Three full creature types: Stone Golem (C09), Mummy (C10), Skeleton (C12)\n");
    fprintf(report, "- 24 stub creature types: reschedule only, no action\n");
    fprintf(report, "- Infinite-loop guard (delay >= 1 on every emission)\n");
    fprintf(report, "- Real DUNGEON.DAT integration spot-check\n\n");
    fprintf(report, "## Known NEEDS DISASSEMBLY REVIEW (still open)\n\n");
    fprintf(report, "- Fontanel `F0229_GROUP_SetOrderedCellsToAttack` cell-ordering and \"archenemy\" bias — v1 uses lowest-index champion (memory_creature_ai_pc34_compat.c F0796).\n");
    fprintf(report, "- Night-vision sight-range palette correction — caller pre-bakes corrected sightRange into the profile (memory_creature_ai_pc34_compat.c F0792).\n");
    fprintf(report, "- FLEE movement consequence (negated smell-direction) — v1 only decrements fearCounter (F0804 AI_STATE_FLEE branch).\n");
    fprintf(report, "- Spell-casting (Vexirk / Lord Chaos / Materializer) — stub path; SpellCastRequest always zero in v1.\n");
    fprintf(report, "- Fontanel `F0202` fake-wall phase-through — caller must pre-clear adjacencyWallMask for NON_MATERIAL creatures (F0798).\n");
    fprintf(report, "- DungeonGroup item-drop resolution — `dropItemsPending` marker emitted but drop list is Phase 17+.\n");
    fprintf(report, "- Reaction events 29/30/31 (DANGER_ON_SQUARE, HIT_BY_PROJECTILE, PARTY_IS_ADJACENT) — `reactionPending` marker only; caller synthesises.\n\n");

    snprintf(path_buf, sizeof(path_buf), "%s/creature_ai_invariants.md", outputDir);
    invariants = fopen(path_buf, "w");
    if (!invariants) {
        fprintf(stderr, "FAIL: cannot write invariants\n");
        fclose(report);
        return 1;
    }
    fprintf(invariants, "# Creature AI Invariants\n\n");

#define CHECK(cond, msg) do { \
    invariantCount++; \
    if (cond) { \
        fprintf(invariants, "- PASS: %s\n", msg); \
    } else { \
        fprintf(invariants, "- FAIL: %s\n", msg); \
        failCount++; \
    } \
} while (0)

    /* ================================================================
     *  Block A — sizes + platform (invariants 1-5)
     * ================================================================ */
    CHECK(sizeof(int) == 4 && sizeof(uint32_t) == 4,
          "sizeof(int)==4 && sizeof(uint32_t)==4");
    CHECK(CREATURE_AI_STATE_SERIALIZED_SIZE == 72 &&
          sizeof(struct CreatureAIState_Compat) == 72,
          "CREATURE_AI_STATE_SERIALIZED_SIZE == 72 and matches sizeof");
    CHECK(CREATURE_TICK_INPUT_SERIALIZED_SIZE == 128 &&
          sizeof(struct CreatureTickInput_Compat) == 128,
          "CREATURE_TICK_INPUT_SERIALIZED_SIZE == 128 and matches sizeof");
    CHECK(CREATURE_TICK_RESULT_SERIALIZED_SIZE == 176 &&
          sizeof(struct CreatureTickResult_Compat) == 176,
          "CREATURE_TICK_RESULT_SERIALIZED_SIZE == 176 and matches sizeof");
    CHECK(CREATURE_BEHAVIOR_PROFILE_SIZE == 64 &&
          sizeof(struct CreatureBehaviorProfile_Compat) == 64,
          "CREATURE_BEHAVIOR_PROFILE_SIZE == 64 and matches sizeof");

    /* ================================================================
     *  Block B — round-trips (invariants 6-9)
     * ================================================================ */
    {
        struct CreatureAIState_Compat a, b;
        unsigned char buf[CREATURE_AI_STATE_SERIALIZED_SIZE];
        int w, r;
        zero_state(&a);
        memset(&b, 0xAA, sizeof(b));
        w = F0805_CREATURE_AIStateSerialize_Compat(&a, buf, sizeof(buf));
        r = F0806_CREATURE_AIStateDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(w == CREATURE_AI_STATE_SERIALIZED_SIZE &&
              r == CREATURE_AI_STATE_SERIALIZED_SIZE &&
              memcmp(&a, &b, sizeof(a)) == 0,
              "Zero CreatureAIState_Compat round-trips bit-identical");
    }
    {
        struct CreatureAIState_Compat a, b;
        unsigned char buf[CREATURE_AI_STATE_SERIALIZED_SIZE];
        int w, r;
        fill_state_populated(&a);
        memset(&b, 0xCC, sizeof(b));
        w = F0805_CREATURE_AIStateSerialize_Compat(&a, buf, sizeof(buf));
        r = F0806_CREATURE_AIStateDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(w == CREATURE_AI_STATE_SERIALIZED_SIZE &&
              r == CREATURE_AI_STATE_SERIALIZED_SIZE &&
              memcmp(&a, &b, sizeof(a)) == 0,
              "Populated CreatureAIState_Compat round-trips bit-identical (all 18 fields)");
    }
    {
        struct CreatureTickInput_Compat a, b;
        unsigned char buf[CREATURE_TICK_INPUT_SERIALIZED_SIZE];
        int w, r;
        fill_input_populated(&a);
        memset(&b, 0x55, sizeof(b));
        w = F0807_CREATURE_TickInputSerialize_Compat(&a, buf, sizeof(buf));
        r = F0808_CREATURE_TickInputDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(w == CREATURE_TICK_INPUT_SERIALIZED_SIZE &&
              r == CREATURE_TICK_INPUT_SERIALIZED_SIZE &&
              memcmp(&a, &b, sizeof(a)) == 0,
              "Populated CreatureTickInput_Compat round-trips bit-identical (32 int32 incl. health arrays)");
    }
    {
        /* Build a result via F0804 run to guarantee realism, then round-trip. */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat a, b;
        unsigned char buf[CREATURE_TICK_RESULT_SERIALIZED_SIZE];
        int w, r;
        zero_state(&sIn); sIn.stateKind = AI_STATE_APPROACH;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 5, 5, 0, 1, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 0x1234);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &a);
        memset(&b, 0x77, sizeof(b));
        w = F0809a_CREATURE_TickResultSerialize_Compat(&a, buf, sizeof(buf));
        r = F0809b_CREATURE_TickResultDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(w == CREATURE_TICK_RESULT_SERIALIZED_SIZE &&
              r == CREATURE_TICK_RESULT_SERIALIZED_SIZE &&
              memcmp(&a, &b, sizeof(a)) == 0,
              "Populated CreatureTickResult_Compat round-trips bit-identical (header + CombatAction + movement + TimelineEvent)");
    }

    /* ================================================================
     *  Block C — state-machine determinism (invariants 10-12)
     * ================================================================ */
    {
        struct CreatureAIState_Compat sIn, s1, s2;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat r1, r2;
        struct CreatureTickResult_Compat out1, out2;
        zero_state(&sIn); sIn.stateKind = AI_STATE_APPROACH;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 10, 10, 0, 3, 1);
        F0730_COMBAT_RngInit_Compat(&r1, 0xC0FFEE);
        F0730_COMBAT_RngInit_Compat(&r2, 0xC0FFEE);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &r1, &s1, &out1);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &r2, &s2, &out2);
        CHECK(memcmp(&s1, &s2, sizeof(s1)) == 0 &&
              memcmp(&out1, &out2, sizeof(out1)) == 0 &&
              r1.seed == r2.seed,
              "F0804 determinism: same (stateIn, input, rngSeed) twice -> bit-identical stateOut and result");
    }
    {
        struct CreatureAIState_Compat s;
        struct CreatureTickInput_Compat in;
        int next = -1, aggr = 0;
        zero_state(&s); s.stateKind = AI_STATE_IDLE;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 3, 1);
        in.groupCurrentHealth[0] = 100;
        F0793_CREATURE_ComputeNextState_Compat(&s, &in, /*visible*/1, /*smell*/0,
                                                &next, &aggr);
        CHECK(next == AI_STATE_WANDER && aggr > 0,
              "F0793 IDLE + partyVisible=1 + alive -> WANDER with aggressionDelta > 0");
    }
    {
        /* APPROACH + adjacent + cooldown=0 -> orchestrator promotes to ATTACK
         * and emits a CombatAction. */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        zero_state(&sIn); sIn.stateKind = AI_STATE_APPROACH;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 5, 5, 0, 1, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 7);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        CHECK(out.resultKind == AI_RESULT_ATTACKED &&
              out.emittedCombatAction == 1 &&
              sOut.stateKind == AI_STATE_ATTACK,
              "Orchestrator: APPROACH + adjacent + cooldown=0 -> ATTACK + CombatAction emitted");
    }

    /* ================================================================
     *  Block D — perception boundaries (invariants 13-16)
     * ================================================================ */
    {
        /* Stone Golem sightRange = 3; d=3, LoS clear -> visible=1 */
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_STONE_GOLEM);
        struct CreatureTickInput_Compat in;
        int visible = -1, dist = -1, smell = -1;
        build_scenario(&in, CREATURE_TYPE_STONE_GOLEM, 0, 0, 0, 3, 1);
        F0792_CREATURE_Perceive_Compat(&in, prof, &visible, &dist, &smell);
        CHECK(prof && prof->sightRange == 3 && visible == 1 && dist == 3 && smell == 0,
              "C09 Stone Golem: distance==sightRange(3) + LoS clear -> visible=1, smell=0");
    }
    {
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_STONE_GOLEM);
        struct CreatureTickInput_Compat in;
        int visible = -1, dist = -1, smell = -1;
        build_scenario(&in, CREATURE_TYPE_STONE_GOLEM, 0, 0, 0, 4, 1);
        F0792_CREATURE_Perceive_Compat(&in, prof, &visible, &dist, &smell);
        CHECK(visible == 0 && dist == 0,
              "C09 Stone Golem: distance=sightRange+1 -> visible=0 (out of sight)");
    }
    {
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_SKELETON);
        struct CreatureTickInput_Compat in;
        int visible = -1, dist = -1, smell = -1;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 0, 0, 0, 1, 0);
        F0792_CREATURE_Perceive_Compat(&in, prof, &visible, &dist, &smell);
        CHECK(visible == 0,
              "losClearFlag=0 -> visible=0 even at distance=1 (LoS gate enforced)");
    }
    {
        /* Mummy has smellRange=4; at distance 2, smell is still on.
         * Without SEE_INVISIBLE + invisibility=1 -> visible=0 but smell=1. */
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_MUMMY);
        struct CreatureTickInput_Compat in;
        int visible = -1, dist = -1, smell = -1;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 2, 1);
        in.partyInvisibility = 1;
        F0792_CREATURE_Perceive_Compat(&in, prof, &visible, &dist, &smell);
        CHECK(prof && (prof->attributes & CREATURE_ATTR_MASK_SEE_INVISIBLE) == 0 &&
              visible == 0 && smell == 1,
              "C10 Mummy: partyInvisibility=1 without SEE_INVISIBLE -> visible=0, smell-fallback=1");
    }

    /* ================================================================
     *  Block E — target selection (invariants 17-20)
     * ================================================================ */
    {
        struct CreatureTickInput_Compat in;
        int idx = -2;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 1, 1);
        in.partyChampionsAlive = 0x0F;
        CHECK(F0796_CREATURE_PickChampion_Compat(&in, &idx) == 1 && idx == 0,
              "F0796: all 4 champions alive -> index 0 (tie-break: lowest-index)");
    }
    {
        struct CreatureTickInput_Compat in;
        int idx = -2;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 1, 1);
        in.partyChampionsAlive         = 0x0E;   /* champ 0 dead (bit 0 clear) */
        in.partyChampionCurrentHealth[0] = 0;
        CHECK(F0796_CREATURE_PickChampion_Compat(&in, &idx) == 1 && idx == 1,
              "F0796: champ 0 dead -> returns index 1");
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
              "F0796: all 4 champions dead -> returns 0, outIdx=-1");
    }
    {
        /* "Sleeping" is a champion-state concept outside this input
         * struct (R1 deferral). If alive mask is set and health > 0
         * the champion is pickable regardless of rest state. */
        struct CreatureTickInput_Compat in;
        int idx = -2;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 0, 0, 0, 1, 1);
        in.partyChampionsAlive = 0x0F;
        in.partyChampionCurrentHealth[0] = 60;
        CHECK(F0796_CREATURE_PickChampion_Compat(&in, &idx) == 1 && idx == 0,
              "F0796: sleeping-but-alive champion still eligible (v1 does not weight by sleep)");
    }

    /* ================================================================
     *  Block F — pathfinding cascade (invariants 21-25)
     * ================================================================ */
    {
        /* Primary (east) open -> F0799 returns primary. */
        struct CreatureTickInput_Compat in;
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_SKELETON);
        struct RngState_Compat rng;
        int dir = -2;
        int rc;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyWallMask = 0;
        F0730_COMBAT_RngInit_Compat(&rng, 42);
        rc = F0799_CREATURE_PickMoveDirection_Compat(
                 &in, prof, 1, 2, 0, &rng, &dir);
        CHECK(rc == 1 && dir == 1,
              "F0799: primary direction open -> returns primaryDir (east=1)");
    }
    {
        /* Primary blocked (wall east), secondary open (south);
         * rng seeded to 0 -> first F0732(rng,2) returns 0 -> take secondary. */
        struct CreatureTickInput_Compat in;
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_SKELETON);
        struct RngState_Compat rng;
        int dir = -2;
        int rc;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyWallMask = 0x02;   /* bit 1 = east blocked */
        F0730_COMBAT_RngInit_Compat(&rng, 0);  /* seed 0: first F0732(rng,2) -> 0 */
        rc = F0799_CREATURE_PickMoveDirection_Compat(
                 &in, prof, /*pri=*/1, /*sec=*/2, 0, &rng, &dir);
        CHECK(rc == 1 && dir == 2,
              "F0799: primary blocked, secondary open, roll2=0 -> takes secondary (south=2)");
    }
    {
        /* Closed door in primary direction, non-NON_MATERIAL creature
         * (Skeleton): F0798 returns blocker=3 (door). */
        struct CreatureTickInput_Compat in;
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_SKELETON);
        int blocker = -1;
        int rc;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyDoorMask = 0x02;
        rc = F0798_CREATURE_IsDirectionOpen_Compat(&in, prof, 1, 0, &blocker);
        CHECK(rc == 0 && blocker == 3 &&
              (prof->attributes & CREATURE_ATTR_MASK_NON_MATERIAL) == 0,
              "F0798: closed door in primary for non-NON_MATERIAL creature -> blocker=3");
    }
    {
        /* Primary has another creature, secondary wall, opposite open -> opp. */
        struct CreatureTickInput_Compat in;
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_SKELETON);
        struct RngState_Compat rng;
        int dir = -2;
        int rc;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyCreatureMask = 0x02;  /* creature east */
        in.adjacencyWallMask     = 0x04;  /* wall south */
        F0730_COMBAT_RngInit_Compat(&rng, 0);
        rc = F0799_CREATURE_PickMoveDirection_Compat(
                 &in, prof, /*pri=*/1, /*sec=*/2, 0, &rng, &dir);
        CHECK(rc == 1 && dir == 3,
              "F0799: creature-blocked primary + wall secondary + opposite open -> returns opposite (west=3)");
    }
    {
        /* All four directions blocked. */
        struct CreatureTickInput_Compat in;
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_SKELETON);
        struct RngState_Compat rng;
        int dir = -2;
        int rc;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        in.adjacencyWallMask = 0x0F;   /* all 4 blocked by wall */
        F0730_COMBAT_RngInit_Compat(&rng, 0xC0FFEE);
        rc = F0799_CREATURE_PickMoveDirection_Compat(
                 &in, prof, 1, 2, 0, &rng, &dir);
        CHECK(rc == 0 && dir == -1,
              "F0799: all 4 directions blocked -> returns 0, outDir=-1 (no crash)");
    }

    /* ================================================================
     *  Block G — attack emission + integration (invariants 26-31)
     * ================================================================ */
    {
        /* Adjacent mummy, ATTACK state, cooldown=0 -> emit. */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_MUMMY);
        zero_state(&sIn); sIn.stateKind = AI_STATE_ATTACK;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 5, 5, 0, 1, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        CHECK(out.emittedCombatAction == 1 &&
              out.outAction.kind == COMBAT_ACTION_CREATURE_MELEE &&
              out.outAction.rawAttackValue == prof->baseAttack &&
              out.outAction.attackerSlotOrCreatureIndex == in.groupSlotIndex,
              "Adjacent + ATTACK + cooldown=0 -> CombatAction kind=CREATURE_MELEE, rawAttack=profile.baseAttack");
    }
    {
        /* distance=2 + ATTACK stateIn -> no emit; orchestrator falls back
         * to APPROACH for this tick (and the next tick will try to move). */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        zero_state(&sIn); sIn.stateKind = AI_STATE_ATTACK;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 5, 5, 0, 2, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        CHECK(out.emittedCombatAction == 0 &&
              out.outAction.kind == 0 &&
              sOut.stateKind == AI_STATE_APPROACH,
              "Non-adjacent (d=2) + ATTACK stateIn -> no CombatAction, state -> APPROACH");
    }
    {
        /* Adjacent + ATTACK + cooldown>0 -> no emit, stays ATTACK. */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        zero_state(&sIn);
        sIn.stateKind           = AI_STATE_ATTACK;
        sIn.attackCooldownTicks = 5;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 5, 5, 0, 1, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        CHECK(out.emittedCombatAction == 0 &&
              sOut.stateKind == AI_STATE_ATTACK &&
              sOut.attackCooldownTicks < sIn.attackCooldownTicks,
              "Adjacent + ATTACK + attackCooldownTicks>0 -> no emit, stays ATTACK, cooldown decrements");
    }
    {
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        zero_state(&sIn); sIn.stateKind = AI_STATE_APPROACH;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        CHECK(out.outNextTick.kind == TIMELINE_EVENT_CREATURE_TICK &&
              (uint32_t)out.outNextTick.fireAtTick > (uint32_t)in.currentTickLow &&
              ((uint32_t)out.outNextTick.fireAtTick - (uint32_t)in.currentTickLow) >= 1u,
              "outNextTick.kind=CREATURE_TICK and fireAtTick - currentTickLow >= 1");
    }
    {
        /* Feed outNextTick through Phase 12 queue round-trip. */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        struct TimelineQueue_Compat q, qRoundTrip;
        unsigned char buf[TIMELINE_QUEUE_SERIALIZED_SIZE];
        int ok;
        zero_state(&sIn); sIn.stateKind = AI_STATE_APPROACH;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 5, 5, 0, 2, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 0xBEEF);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        F0720_TIMELINE_Init_Compat(&q, (uint32_t)in.currentTickLow);
        F0721_TIMELINE_Schedule_Compat(&q, &out.outNextTick);
        ok = (F0727_TIMELINE_QueueSerialize_Compat(&q, buf, sizeof(buf)) == 1);
        memset(&qRoundTrip, 0xAA, sizeof(qRoundTrip));
        ok = ok &&
             (F0728_TIMELINE_QueueDeserialize_Compat(&qRoundTrip, buf, sizeof(buf)) == 1);
        CHECK(ok && memcmp(&q, &qRoundTrip, sizeof(q)) == 0 && q.count == 1,
              "Phase 12 integration: outNextTick scheduled into queue round-trips via F0727/F0728");
    }
    {
        /* Phase 13 integration: build a creature snapshot using the same
         * profile values and let F0736 resolve melee. Assert it returns 1
         * and produces a consistent outcome. */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng, combatRng;
        struct CreatureTickResult_Compat out;
        struct CombatantCreatureSnapshot_Compat attacker;
        struct CombatantChampionSnapshot_Compat defender;
        struct CombatResult_Compat result;
        const struct CreatureBehaviorProfile_Compat* prof =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_MUMMY);
        int f0736rc;
        zero_state(&sIn); sIn.stateKind = AI_STATE_ATTACK;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 5, 5, 0, 1, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);

        memset(&attacker, 0, sizeof(attacker));
        attacker.creatureType         = CREATURE_TYPE_MUMMY;
        attacker.attack               = prof->baseAttack;
        attacker.defense              = prof->baseDefense;
        attacker.dexterity            = prof->dexterity;
        attacker.baseHealth           = prof->baseHealth;
        attacker.poisonAttack         = prof->poisonAttack;
        attacker.attackType           = prof->attackType;
        attacker.attributes           = prof->attributes;
        attacker.woundProbabilities   = prof->woundProbabilities;
        attacker.doubledMapDifficulty = 4;
        attacker.creatureIndex        = 0;
        attacker.healthBefore         = prof->baseHealth;

        memset(&defender, 0, sizeof(defender));
        defender.championIndex      = 0;
        defender.currentHealth      = 60;
        defender.dexterity          = 40;
        defender.statisticVitality  = 50;
        defender.statisticAntifire  = 50;
        defender.statisticAntimagic = 50;
        {
            int i;
            for (i = 0; i < 6; i++) defender.woundDefense[i] = 5;
        }

        F0730_COMBAT_RngInit_Compat(&combatRng, 0xCAFE);
        memset(&result, 0, sizeof(result));
        f0736rc = F0736_COMBAT_ResolveCreatureMelee_Compat(
                      &attacker, &defender, &combatRng, &result);
        CHECK(f0736rc == 1 &&
              out.emittedCombatAction == 1 &&
              out.outAction.attackTypeCode == prof->attackType &&
              out.outAction.rawAttackValue == prof->baseAttack &&
              (result.outcome == COMBAT_OUTCOME_MISS          ||
               result.outcome == COMBAT_OUTCOME_HIT_NO_DAMAGE ||
               result.outcome == COMBAT_OUTCOME_HIT_DAMAGE    ||
               result.outcome == COMBAT_OUTCOME_CHAMPION_DOWN),
              "Phase 13 integration: emitted action parameters match creature snapshot; F0736 resolves with a valid outcome");
    }

    /* ================================================================
     *  Block H — 24 stub creature types meta-sweep (invariant 32)
     * ================================================================ */
    {
        int allStubOk = 1;
        int count = 0;
        int type;
        for (type = 0; type < CREATURE_TYPE_COUNT; type++) {
            struct CreatureAIState_Compat sIn, sOut;
            struct CreatureTickInput_Compat in;
            struct RngState_Compat rng;
            struct CreatureTickResult_Compat out;
            const struct CreatureBehaviorProfile_Compat* prof;
            int rc;
            if (is_full_tier(type)) continue;
            prof = CREATURE_GetProfile_Compat(type);
            if (!prof || prof->implementationTier != CREATURE_IMPL_TIER_STUB) {
                allStubOk = 0; break;
            }
            zero_state(&sIn); sIn.stateKind = AI_STATE_APPROACH;
            build_scenario(&in, type, 5, 5, 0, 1, 1);
            F0730_COMBAT_RngInit_Compat(&rng, 0xDEAD + type);
            rc = F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
            if (rc != 1 ||
                out.resultKind           != AI_RESULT_NO_ACTION ||
                out.emittedCombatAction  != 0 ||
                out.emittedMovement      != 0 ||
                out.outNextTick.kind     != TIMELINE_EVENT_CREATURE_TICK ||
                (uint32_t)out.outNextTick.fireAtTick - (uint32_t)in.currentTickLow < 1u) {
                allStubOk = 0;
                break;
            }
            count++;
        }
        CHECK(allStubOk && count == CREATURE_TYPE_COUNT - 3,
              "Stub-tier meta: 24 non-full creatureTypes all return NO_ACTION + valid next-tick");
    }

    /* ================================================================
     *  Block I — boundary + purity (invariants 33-38)
     * ================================================================ */
    {
        struct CreatureAIState_Compat sOut;
        struct CreatureTickResult_Compat out;
        struct RngState_Compat rng;
        struct CreatureTickInput_Compat in;
        struct CreatureAIState_Compat sOutExpected;
        struct CreatureTickResult_Compat outExpected;
        int rc;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 1, 1, 0, 1, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        memset(&sOut, 0xAB, sizeof(sOut));
        memset(&out,  0xAB, sizeof(out));
        memcpy(&sOutExpected, &sOut, sizeof(sOutExpected));
        memcpy(&outExpected,  &out,  sizeof(outExpected));
        rc = F0804_CREATURE_Tick_Compat(
                 /*stateIn=*/0, &in, &rng, &sOut, &out);
        /* Contract: returns 0; outputs must not be overwritten by the
         * zero-memset inside the body (the NULL check fires first). */
        CHECK(rc == 0 &&
              memcmp(&sOut, &sOutExpected, sizeof(sOut)) == 0 &&
              memcmp(&out,  &outExpected,  sizeof(out))  == 0,
              "F0804(stateIn=NULL, ...) -> returns 0 and leaves outputs untouched");
    }
    {
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        int rc;
        zero_state(&sIn);
        build_scenario(&in, /*type=*/CREATURE_TYPE_COUNT, 1, 1, 0, 1, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        rc = F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        CHECK(rc == 0,
              "F0804 with creatureType=CREATURE_TYPE_COUNT(27) -> returns 0 (out of range)");
    }
    {
        /* DEAD stateIn is terminal: no next-tick scheduled. */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        zero_state(&sIn); sIn.stateKind = AI_STATE_DEAD;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 1, 1, 0, 1, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        CHECK(out.resultKind    == AI_RESULT_DIED &&
              out.outNextTick.kind == 0 &&   /* TIMELINE_EVENT_INVALID marker (0 = unset) */
              out.dropItemsPending  == 1,
              "F0804 DEAD stateIn -> resultKind=DIED, dropItemsPending=1, no next CREATURE_TICK");
    }
    {
        /* ATTACK state, adjacent, but all champions dead -> no emit. */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        int i;
        zero_state(&sIn); sIn.stateKind = AI_STATE_ATTACK;
        build_scenario(&in, CREATURE_TYPE_MUMMY, 1, 1, 0, 1, 1);
        in.partyChampionsAlive = 0;
        for (i = 0; i < 4; i++) in.partyChampionCurrentHealth[i] = 0;
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        CHECK(out.emittedCombatAction == 0 &&
              (out.resultKind == AI_RESULT_NO_ACTION ||
               out.resultKind == AI_RESULT_DIED),
              "F0804: all champions dead -> no CombatAction emitted even in ATTACK state");
    }
    {
        /* Purity: stateIn + in buffer CRC unchanged across F0804. */
        struct CreatureAIState_Compat sIn, sOut;
        struct CreatureTickInput_Compat in;
        struct RngState_Compat rng;
        struct CreatureTickResult_Compat out;
        unsigned char snapshot[sizeof(sIn) + sizeof(in)];
        uint32_t crcBefore, crcAfter;
        zero_state(&sIn); sIn.stateKind = AI_STATE_APPROACH;
        build_scenario(&in, CREATURE_TYPE_SKELETON, 5, 5, 0, 2, 1);
        memcpy(snapshot,                 &sIn, sizeof(sIn));
        memcpy(snapshot + sizeof(sIn),   &in,  sizeof(in));
        crcBefore = local_crc32(snapshot, sizeof(snapshot));
        F0730_COMBAT_RngInit_Compat(&rng, 0xFEED);
        F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        memcpy(snapshot,                 &sIn, sizeof(sIn));
        memcpy(snapshot + sizeof(sIn),   &in,  sizeof(in));
        crcAfter = local_crc32(snapshot, sizeof(snapshot));
        CHECK(crcBefore == crcAfter,
              "Purity: CRC32 of (stateIn, input) buffer unchanged across F0804 (const-ptr contract)");
    }
    {
        /* Profile table is read-only. Snapshot 27 profiles; sweep F0804
         * over all types; re-snapshot and compare CRC. */
        unsigned char snap1[CREATURE_TYPE_COUNT * sizeof(struct CreatureBehaviorProfile_Compat)];
        unsigned char snap2[CREATURE_TYPE_COUNT * sizeof(struct CreatureBehaviorProfile_Compat)];
        uint32_t crc1, crc2;
        int type;
        for (type = 0; type < CREATURE_TYPE_COUNT; type++) {
            const struct CreatureBehaviorProfile_Compat* p =
                CREATURE_GetProfile_Compat(type);
            memcpy(snap1 + type * sizeof(*p), p, sizeof(*p));
        }
        crc1 = local_crc32(snap1, sizeof(snap1));
        for (type = 0; type < CREATURE_TYPE_COUNT; type++) {
            struct CreatureAIState_Compat sIn, sOut;
            struct CreatureTickInput_Compat in;
            struct RngState_Compat rng;
            struct CreatureTickResult_Compat out;
            zero_state(&sIn); sIn.stateKind = AI_STATE_APPROACH;
            build_scenario(&in, type, 5, 5, 0, 2, 1);
            F0730_COMBAT_RngInit_Compat(&rng, 0xAAA0 + type);
            F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
        }
        for (type = 0; type < CREATURE_TYPE_COUNT; type++) {
            const struct CreatureBehaviorProfile_Compat* p =
                CREATURE_GetProfile_Compat(type);
            memcpy(snap2 + type * sizeof(*p), p, sizeof(*p));
        }
        crc2 = local_crc32(snap2, sizeof(snap2));
        CHECK(crc1 == crc2,
              "Purity: 27-entry profile-table CRC32 unchanged after 27-type F0804 sweep");
    }

    /* ================================================================
     *  Block J — loop guard + real DUNGEON.DAT spot-check (39-40)
     * ================================================================ */
    {
        /* 100 random valid inputs, all must produce delay >= 1. */
        struct RngState_Compat rngFeeder;
        int iter;
        int allOk = 1;
        F0730_COMBAT_RngInit_Compat(&rngFeeder, 0xC0FFEEu);
        for (iter = 0; iter < 100; iter++) {
            struct CreatureAIState_Compat sIn, sOut;
            struct CreatureTickInput_Compat in;
            struct RngState_Compat rng;
            struct CreatureTickResult_Compat out;
            int type     = F0732_COMBAT_RngRandom_Compat(&rngFeeder, CREATURE_TYPE_COUNT);
            int stateKind= F0732_COMBAT_RngRandom_Compat(&rngFeeder, AI_STATE_COUNT);
            int d        = F0732_COMBAT_RngRandom_Compat(&rngFeeder, 6) + 1;  /* 1..6 */
            int los      = F0732_COMBAT_RngRandom_Compat(&rngFeeder, 2);
            int cool     = F0732_COMBAT_RngRandom_Compat(&rngFeeder, 4);
            int fear     = F0732_COMBAT_RngRandom_Compat(&rngFeeder, 10);
            int rc;

            zero_state(&sIn);
            sIn.stateKind           = stateKind;
            sIn.attackCooldownTicks = cool;
            sIn.fearCounter         = fear;
            build_scenario(&in, type, 5, 5, 0, d, los);
            F0730_COMBAT_RngInit_Compat(&rng, (uint32_t)(0x10000u + iter));
            rc = F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
            if (rc != 1) { allOk = 0; break; }
            /* DEAD terminal explicitly has no next tick. */
            if (sIn.stateKind == AI_STATE_DEAD ||
                out.resultKind == AI_RESULT_DIED) {
                continue;
            }
            if (out.outNextTick.kind != TIMELINE_EVENT_CREATURE_TICK) {
                allOk = 0; break;
            }
            if (((uint32_t)out.outNextTick.fireAtTick -
                 (uint32_t)in.currentTickLow) < 1u) {
                allOk = 0; break;
            }
        }
        CHECK(allOk,
              "Loop-guard (R7): 100 random inputs all produce fireAtTick - currentTickLow >= 1");
    }
    {
        /* DUNGEON.DAT spot-check: find first group of a fully-implemented
         * creature type; run F0804 with party at Manhattan distance 3 and
         * LoS clear; assert transition is IDLE -> WANDER. */
        struct DungeonDatState_Compat dungeon;
        struct DungeonThings_Compat things;
        int found = 0;
        int foundType = -1;
        int rc_ok = 0;
        memset(&dungeon, 0, sizeof(dungeon));
        memset(&things,  0, sizeof(things));
        if (F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, &dungeon) &&
            F0502_DUNGEON_LoadTileData_Compat(dungeonPath, &dungeon) &&
            F0504_DUNGEON_LoadThingData_Compat(dungeonPath, &dungeon, &things)) {

            int i;
            /* Prefer skeleton (C12), then mummy (C10), then stone golem (C09). */
            const int prefs[3] = {
                CREATURE_TYPE_SKELETON,
                CREATURE_TYPE_MUMMY,
                CREATURE_TYPE_STONE_GOLEM
            };
            int p;
            for (p = 0; p < 3 && !found; p++) {
                for (i = 0; i < things.groupCount; i++) {
                    if ((int)things.groups[i].creatureType == prefs[p]) {
                        foundType = prefs[p];
                        found = 1;
                        break;
                    }
                }
            }
            if (found) {
                struct CreatureAIState_Compat sIn, sOut;
                struct CreatureTickInput_Compat in;
                struct RngState_Compat rng;
                struct CreatureTickResult_Compat out;
                zero_state(&sIn); sIn.stateKind = AI_STATE_IDLE;
                build_scenario(&in, foundType, 5, 5, 0, 3, 1);
                in.groupSlotIndex = 42;  /* explicit slot for aux0 check */
                F0730_COMBAT_RngInit_Compat(&rng, 0xD34D);
                F0804_CREATURE_Tick_Compat(&sIn, &in, &rng, &sOut, &out);
                rc_ok = (sOut.stateKind == AI_STATE_WANDER &&
                         out.outNextTick.kind == TIMELINE_EVENT_CREATURE_TICK &&
                         out.outNextTick.aux0 == 42 &&
                         out.outNextTick.aux1 == foundType &&
                         out.outNextTick.aux2 == AI_STATE_WANDER);
            }
            fprintf(report,
                    "## DUNGEON.DAT integration\n\n- groupCount: %d\n- found creatureType: %d\n- IDLE->WANDER OK: %s\n\n",
                    things.groupCount, foundType, rc_ok ? "yes" : "no");

            F0504_DUNGEON_FreeThingData_Compat(&things);
            F0502_DUNGEON_FreeTileData_Compat(&dungeon);
            F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        } else {
            fprintf(report, "## DUNGEON.DAT integration\n\n- could not load DUNGEON.DAT at %s\n\n", dungeonPath);
        }
        CHECK(found && rc_ok,
              "DUNGEON.DAT spot-check: real group with fully-implemented type -> IDLE+visible(d=3) transitions to WANDER; outNextTick aux0/aux1/aux2 match slot/type/state");
    }

    /* ================================================================
     *  Trailer
     * ================================================================ */
    fprintf(invariants, "\nInvariant count: %d\n", invariantCount);
    if (failCount == 0) {
        fprintf(invariants, "Status: PASS\n");
    } else {
        fprintf(invariants, "Status: FAIL (%d failures)\n", failCount);
    }
    fclose(invariants);
    fclose(report);
    return failCount > 0 ? 1 : 0;
}
