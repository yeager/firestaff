/*
 * M10 Phase 13 probe: combat system verification.
 *
 * Tests the pure data-layer combat resolvers:
 *   - RNG primitives (F0730–F0732)
 *   - Defence helpers (F0733–F0734)
 *   - Melee resolvers (F0735 champion→creature, F0736 creature→champion)
 *   - Damage application (F0737 champion, F0738 group)
 *   - Timeline bridge (F0739)
 *   - Serialisation round-trips (F0740–F0747)
 *
 * 35 invariants, ≥30 required for PASS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_combat_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

static unsigned int checksum_bytes(const void* p, size_t n) {
    const unsigned char* b = (const unsigned char*)p;
    unsigned int h = 2166136261u;
    size_t i;
    for (i = 0; i < n; i++) {
        h ^= b[i];
        h *= 16777619u;
    }
    return h;
}

/* Small helpers to assemble canonical test inputs. */

static void fill_empty_action(struct CombatAction_Compat* a) {
    memset(a, 0, sizeof(*a));
}

static void fill_populated_action(struct CombatAction_Compat* a) {
    a->kind                        = COMBAT_ACTION_CHAMPION_MELEE;
    a->allowedWounds               = COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO;
    a->attackTypeCode              = COMBAT_ATTACK_BLUNT;
    a->rawAttackValue              = 17;
    a->targetMapIndex              = 3;
    a->targetMapX                  = 5;
    a->targetMapY                  = 7;
    a->targetCell                  = 2;
    a->attackerSlotOrCreatureIndex = 1;
    a->defenderSlotOrCreatureIndex = 0;
    a->scheduleDelayTicks          = 6;
    a->flags                       = COMBAT_FLAG_USE_SHARP_DEFENSE;
}

static void fill_populated_result(struct CombatResult_Compat* r) {
    r->outcome              = COMBAT_OUTCOME_HIT_DAMAGE;
    r->damageApplied        = 19;
    r->rawAttackRoll        = 22;
    r->defenseRoll          = 11;
    r->hitLanded            = 1;
    r->wasCritical          = 0;
    r->woundMaskAdded       = COMBAT_WOUND_HEAD;
    r->poisonAttackPending  = 2;
    r->targetKilled         = 0;
    r->creatureSlotRemoved  = -1;
    r->followupEventKind    = TIMELINE_EVENT_STATUS_TIMEOUT;
    r->followupEventAux0    = 7;
    r->rngCallCount         = 12;
    r->wakeFromRest         = 1;
}

static void fill_mummy_attacker(struct CombatantCreatureSnapshot_Compat* c) {
    memset(c, 0, sizeof(*c));
    c->creatureType         = 14;   /* Mummy-like index, in 0..26 range */
    c->attack               = 24;
    c->defense              = 18;
    c->dexterity            = 30;
    c->baseHealth           = 90;
    c->poisonAttack         = 0;
    c->attackType           = COMBAT_ATTACK_BLUNT;
    c->attributes           = 0;   /* material, no special flags */
    c->woundProbabilities   = 0x8421;
    c->properties           = 0;
    c->doubledMapDifficulty = 2;
    c->creatureIndex        = 0;
    c->healthBefore         = 90;
}

static void fill_hero_defender(struct CombatantChampionSnapshot_Compat* c) {
    memset(c, 0, sizeof(*c));
    c->championIndex        = 0;
    c->currentHealth        = 50;
    c->dexterity            = 40;
    c->strengthActionHand   = 0;
    c->skillLevelParry      = 0;
    c->skillLevelAction     = 0;
    c->statisticVitality    = 45;
    c->statisticAntifire    = 30;
    c->statisticAntimagic   = 30;
    c->actionHandIcon       = 0;
    c->wounds               = 0;
    c->woundDefense[0]      = 5;
    c->woundDefense[1]      = 5;
    c->woundDefense[2]      = 5;
    c->woundDefense[3]      = 5;
    c->woundDefense[4]      = 5;
    c->woundDefense[5]      = 5;
    c->isResting            = 0;
    c->partyShieldDefense   = 0;
}

static void fill_ninja_attacker(struct CombatantChampionSnapshot_Compat* c) {
    memset(c, 0, sizeof(*c));
    c->championIndex        = 1;
    c->currentHealth        = 80;
    c->dexterity            = 55;
    c->strengthActionHand   = 60;
    c->skillLevelParry      = 5;
    c->skillLevelAction     = 10;
    c->statisticVitality    = 60;
    c->statisticAntifire    = 30;
    c->statisticAntimagic   = 30;
    c->actionHandIcon       = COMBAT_ICON_VORPAL_BLADE;
    c->wounds               = 0;
    c->woundDefense[0]      = 3;
    c->woundDefense[1]      = 3;
    c->woundDefense[2]      = 3;
    c->woundDefense[3]      = 3;
    c->woundDefense[4]      = 3;
    c->woundDefense[5]      = 3;
    c->isResting            = 0;
    c->partyShieldDefense   = 0;
}

static void fill_skeleton_defender(struct CombatantCreatureSnapshot_Compat* c) {
    memset(c, 0, sizeof(*c));
    c->creatureType         = 9;
    c->attack               = 20;
    c->defense              = 22;
    c->dexterity            = 28;
    c->baseHealth           = 40;
    c->poisonAttack         = 0;
    c->attackType           = COMBAT_ATTACK_SHARP;
    c->attributes           = 0;
    c->woundProbabilities   = 0x8421;
    c->properties           = 0;
    c->doubledMapDifficulty = 2;
    c->creatureIndex        = 0;
    c->healthBefore         = 40;
}

static void fill_sword_weapon(struct WeaponProfile_Compat* w) {
    w->weaponType     = 0;
    w->weaponClass    = 0;
    w->weaponStrength = 8;
    w->kineticEnergy  = 30;
    w->hitProbability = 40;
    w->damageFactor   = 50;
    w->skillIndex     = CHAMPION_SKILL_FIGHTER;
    w->attributes     = 0;
}

int main(int argc, char* argv[]) {
    FILE* report;
    FILE* invariants;
    char path_buf[512];
    int failCount = 0;
    int invariantCount = 0;
    const char* dungeonPath = 0;
    const char* outputDir = 0;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
        return 1;
    }
    dungeonPath = argv[1];
    outputDir = argv[2];

    snprintf(path_buf, sizeof(path_buf), "%s/combat_probe.md", outputDir);
    report = fopen(path_buf, "w");
    if (!report) { fprintf(stderr, "FAIL: cannot write report\n"); return 1; }
    fprintf(report, "# M10 Phase 13: Combat System Probe\n\n");
    fprintf(report, "## Scope (v1)\n\n");
    fprintf(report, "- Champion -> creature melee resolver (F0735)\n");
    fprintf(report, "- Creature -> champion melee resolver (F0736)\n");
    fprintf(report, "- Damage application to champion (F0737)\n");
    fprintf(report, "- Damage application to group (F0738)\n");
    fprintf(report, "- Deterministic LCG RNG (F0730-F0732)\n");
    fprintf(report, "- Timeline-event builder (F0739)\n");
    fprintf(report, "- Serialisation (F0740-F0747)\n\n");
    fprintf(report, "## Known `NEEDS DISASSEMBLY REVIEW` markers\n\n");
    fprintf(report, "- F0736: F0314_CHAMPION_WakeUp side effect deferred to caller.\n");
    fprintf(report, "- combat_apply_defender_statistic_adjustment: fire/magic/psychic defence paths stubbed (phase 14).\n");
    fprintf(report, "- F0735: luck-state (F0308_CHAMPION_IsLucky) collapsed to 0.\n");
    fprintf(report, "- F0738: cell/direction packing reshuffle on kill deferred to phase 14.\n\n");

    snprintf(path_buf, sizeof(path_buf), "%s/combat_invariants.md", outputDir);
    invariants = fopen(path_buf, "w");
    if (!invariants) { fprintf(stderr, "FAIL: cannot write invariants\n"); fclose(report); return 1; }
    fprintf(invariants, "# Combat Invariants\n\n");

#define CHECK(cond, msg) do { \
    invariantCount++; \
    if (cond) { \
        fprintf(invariants, "- PASS: %s\n", msg); \
    } else { \
        fprintf(invariants, "- FAIL: %s\n", msg); \
        failCount++; \
    } \
} while (0)

    /* ==============================================================
     *  Block A — size constants (invariants 1–6)
     * ============================================================== */
    CHECK(RNG_STATE_SERIALIZED_SIZE == 4,
          "RNG_STATE_SERIALIZED_SIZE is 4");
    CHECK(COMBAT_ACTION_SERIALIZED_SIZE == 48,
          "COMBAT_ACTION_SERIALIZED_SIZE is 48");
    CHECK(COMBAT_RESULT_SERIALIZED_SIZE == 56,
          "COMBAT_RESULT_SERIALIZED_SIZE is 56");
    CHECK(COMBATANT_CHAMPION_SERIALIZED_SIZE == 76,
          "COMBATANT_CHAMPION_SERIALIZED_SIZE is 76");
    CHECK(COMBATANT_CREATURE_SERIALIZED_SIZE == 52,
          "COMBATANT_CREATURE_SERIALIZED_SIZE is 52");
    CHECK(WEAPON_PROFILE_SERIALIZED_SIZE == 32,
          "WEAPON_PROFILE_SERIALIZED_SIZE is 32");

    /* ==============================================================
     *  Block B — round-trip serialisation (invariants 7–13)
     * ============================================================== */
    {
        struct CombatAction_Compat orig, restored;
        unsigned char buf[COMBAT_ACTION_SERIALIZED_SIZE];
        fill_empty_action(&orig);
        memset(&restored, 0xAA, sizeof(restored));
        F0740_COMBAT_ActionSerialize_Compat(&orig, buf, sizeof(buf));
        F0741_COMBAT_ActionDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Empty CombatAction round-trips bit-identical");
    }
    {
        struct CombatAction_Compat orig, restored;
        unsigned char buf[COMBAT_ACTION_SERIALIZED_SIZE];
        fill_populated_action(&orig);
        memset(&restored, 0xBB, sizeof(restored));
        F0740_COMBAT_ActionSerialize_Compat(&orig, buf, sizeof(buf));
        F0741_COMBAT_ActionDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Populated CombatAction round-trips bit-identical");
    }
    {
        struct CombatResult_Compat orig, restored;
        unsigned char buf[COMBAT_RESULT_SERIALIZED_SIZE];
        memset(&orig, 0, sizeof(orig));
        memset(&restored, 0xCC, sizeof(restored));
        F0742_COMBAT_ResultSerialize_Compat(&orig, buf, sizeof(buf));
        F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Empty CombatResult round-trips bit-identical");
    }
    {
        struct CombatResult_Compat orig, restored;
        unsigned char buf[COMBAT_RESULT_SERIALIZED_SIZE];
        memset(&orig, 0, sizeof(orig));
        fill_populated_result(&orig);
        memset(&restored, 0xDD, sizeof(restored));
        F0742_COMBAT_ResultSerialize_Compat(&orig, buf, sizeof(buf));
        F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Populated CombatResult round-trips bit-identical");
    }
    {
        struct CombatantChampionSnapshot_Compat orig, restored;
        unsigned char buf[COMBATANT_CHAMPION_SERIALIZED_SIZE];
        fill_ninja_attacker(&orig);
        memset(&restored, 0xEE, sizeof(restored));
        F0744_COMBAT_ChampionSnapshotSerialize_Compat(&orig, buf, sizeof(buf));
        F0745_COMBAT_ChampionSnapshotDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Populated CombatantChampionSnapshot round-trips bit-identical");
    }
    {
        struct CombatantCreatureSnapshot_Compat orig, restored;
        unsigned char buf[COMBATANT_CREATURE_SERIALIZED_SIZE];
        fill_mummy_attacker(&orig);
        memset(&restored, 0x77, sizeof(restored));
        F0746_COMBAT_CreatureSnapshotSerialize_Compat(&orig, buf, sizeof(buf));
        F0747_COMBAT_CreatureSnapshotDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Populated CombatantCreatureSnapshot round-trips bit-identical");
    }
    {
        struct WeaponProfile_Compat orig, restored;
        unsigned char buf[WEAPON_PROFILE_SERIALIZED_SIZE];
        fill_sword_weapon(&orig);
        memset(&restored, 0x88, sizeof(restored));
        F0747a_COMBAT_WeaponProfileSerialize_Compat(&orig, buf, sizeof(buf));
        F0747b_COMBAT_WeaponProfileDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Populated WeaponProfile round-trips bit-identical");
    }

    /* ==============================================================
     *  Block C — RNG (invariants 14–16)
     * ============================================================== */
    {
        struct RngState_Compat rng;
        uint32_t raw;
        F0730_COMBAT_RngInit_Compat(&rng, 0);
        raw = F0731_COMBAT_RngNextRaw_Compat(&rng);
        /* LCG: 0 * 1103515245 + 12345 = 12345. */
        CHECK(raw == 12345u,
              "RngInit(0) + first RngNextRaw returns golden 12345");
    }
    {
        struct RngState_Compat rng;
        int i;
        int allInRange = 1;
        F0730_COMBAT_RngInit_Compat(&rng, 0xABCDEF01u);
        for (i = 0; i < 1000; i++) {
            int v = F0732_COMBAT_RngRandom_Compat(&rng, 100);
            if (v < 0 || v >= 100) { allInRange = 0; break; }
        }
        CHECK(allInRange,
              "RngRandom(100) stays in [0,99] over 1000 draws");
    }
    {
        struct RngState_Compat rng;
        uint32_t before, after;
        int v;
        F0730_COMBAT_RngInit_Compat(&rng, 0x12345678u);
        before = rng.seed;
        v = F0732_COMBAT_RngRandom_Compat(&rng, 0);
        after = rng.seed;
        CHECK(v == 0 && before == after,
              "RngRandom(0) returns 0 and does not advance state");
    }

    /* ==============================================================
     *  Block D — determinism + purity (invariants 17–21)
     * ============================================================== */
    {
        struct CombatantChampionSnapshot_Compat attacker;
        struct WeaponProfile_Compat weapon;
        struct CombatantCreatureSnapshot_Compat defender;
        struct RngState_Compat rng1, rng2;
        struct CombatResult_Compat r1, r2;
        fill_ninja_attacker(&attacker);
        fill_sword_weapon(&weapon);
        fill_skeleton_defender(&defender);
        F0730_COMBAT_RngInit_Compat(&rng1, 0xDEADBEEFu);
        F0730_COMBAT_RngInit_Compat(&rng2, 0xDEADBEEFu);
        F0735_COMBAT_ResolveChampionMelee_Compat(&attacker, &weapon, &defender, &rng1, &r1);
        F0735_COMBAT_ResolveChampionMelee_Compat(&attacker, &weapon, &defender, &rng2, &r2);
        CHECK(memcmp(&r1, &r2, sizeof(r1)) == 0,
              "F0735 determinism: same seed + inputs -> byte-identical result");
    }
    {
        struct CombatantCreatureSnapshot_Compat attacker;
        struct CombatantChampionSnapshot_Compat defender;
        struct RngState_Compat rng1, rng2;
        struct CombatResult_Compat r1, r2;
        fill_mummy_attacker(&attacker);
        fill_hero_defender(&defender);
        F0730_COMBAT_RngInit_Compat(&rng1, 0xC0FFEE00u);
        F0730_COMBAT_RngInit_Compat(&rng2, 0xC0FFEE00u);
        F0736_COMBAT_ResolveCreatureMelee_Compat(&attacker, &defender, &rng1, &r1);
        F0736_COMBAT_ResolveCreatureMelee_Compat(&attacker, &defender, &rng2, &r2);
        CHECK(memcmp(&r1, &r2, sizeof(r1)) == 0,
              "F0736 determinism: same seed + inputs -> byte-identical result");
    }
    {
        struct CombatantChampionSnapshot_Compat attacker;
        struct WeaponProfile_Compat weapon;
        struct CombatantCreatureSnapshot_Compat defender;
        struct RngState_Compat rng;
        struct CombatResult_Compat r;
        unsigned int atkPre, atkPost;
        unsigned int wepPre, wepPost;
        unsigned int defPre, defPost;
        fill_ninja_attacker(&attacker);
        fill_sword_weapon(&weapon);
        fill_skeleton_defender(&defender);
        atkPre = checksum_bytes(&attacker, sizeof(attacker));
        wepPre = checksum_bytes(&weapon, sizeof(weapon));
        defPre = checksum_bytes(&defender, sizeof(defender));
        F0730_COMBAT_RngInit_Compat(&rng, 42);
        F0735_COMBAT_ResolveChampionMelee_Compat(&attacker, &weapon, &defender, &rng, &r);
        atkPost = checksum_bytes(&attacker, sizeof(attacker));
        wepPost = checksum_bytes(&weapon, sizeof(weapon));
        defPost = checksum_bytes(&defender, sizeof(defender));
        CHECK(atkPre == atkPost && wepPre == wepPost && defPre == defPost,
              "F0735 purity: attacker/weapon/defender snapshots are not mutated");
    }
    {
        struct CombatantCreatureSnapshot_Compat attacker;
        struct CombatantChampionSnapshot_Compat defender;
        struct RngState_Compat rng;
        struct CombatResult_Compat r;
        unsigned int atkPre, atkPost;
        unsigned int defPre, defPost;
        fill_mummy_attacker(&attacker);
        fill_hero_defender(&defender);
        atkPre = checksum_bytes(&attacker, sizeof(attacker));
        defPre = checksum_bytes(&defender, sizeof(defender));
        F0730_COMBAT_RngInit_Compat(&rng, 7);
        F0736_COMBAT_ResolveCreatureMelee_Compat(&attacker, &defender, &rng, &r);
        atkPost = checksum_bytes(&attacker, sizeof(attacker));
        defPost = checksum_bytes(&defender, sizeof(defender));
        CHECK(atkPre == atkPost && defPre == defPost,
              "F0736 purity: attacker/defender snapshots are not mutated");
    }
    {
        struct CombatAction_Compat action;
        struct CombatResult_Compat result;
        struct TimelineEvent_Compat event;
        unsigned int actionPre, actionPost;
        unsigned int resultPre, resultPost;
        fill_populated_action(&action);
        memset(&result, 0, sizeof(result));
        fill_populated_result(&result);
        actionPre = checksum_bytes(&action, sizeof(action));
        resultPre = checksum_bytes(&result, sizeof(result));
        F0739_COMBAT_BuildTimelineEvent_Compat(&action, &result, 100, &event);
        actionPost = checksum_bytes(&action, sizeof(action));
        resultPost = checksum_bytes(&result, sizeof(result));
        CHECK(actionPre == actionPost && resultPre == resultPost,
              "F0739 purity: action + result inputs are not mutated");
    }

    /* ==============================================================
     *  Block E — boundary / nulls (invariants 22–29)
     * ============================================================== */
    {
        struct WeaponProfile_Compat weapon;
        struct CombatantCreatureSnapshot_Compat defender;
        struct RngState_Compat rng;
        struct CombatResult_Compat r;
        int rv;
        fill_sword_weapon(&weapon);
        fill_skeleton_defender(&defender);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        rv = F0735_COMBAT_ResolveChampionMelee_Compat(0, &weapon, &defender, &rng, &r);
        CHECK(rv == 0 && r.outcome == COMBAT_OUTCOME_MISS,
              "F0735 with NULL attacker returns 0, outcome MISS, no crash");
    }
    {
        struct CombatantChampionSnapshot_Compat attacker;
        struct WeaponProfile_Compat weapon;
        struct CombatantCreatureSnapshot_Compat defender;
        struct RngState_Compat rng;
        struct CombatResult_Compat r;
        fill_ninja_attacker(&attacker);
        attacker.currentHealth = 0;
        fill_sword_weapon(&weapon);
        fill_skeleton_defender(&defender);
        F0730_COMBAT_RngInit_Compat(&rng, 2);
        F0735_COMBAT_ResolveChampionMelee_Compat(&attacker, &weapon, &defender, &rng, &r);
        CHECK(r.outcome == COMBAT_OUTCOME_MISS && r.damageApplied == 0,
              "F0735 with attacker.currentHealth=0 returns MISS, damage 0");
    }
    {
        struct CombatantCreatureSnapshot_Compat attacker;
        struct CombatantChampionSnapshot_Compat defender;
        struct RngState_Compat rng;
        struct CombatResult_Compat r;
        fill_mummy_attacker(&attacker);
        fill_hero_defender(&defender);
        defender.currentHealth = 0;
        F0730_COMBAT_RngInit_Compat(&rng, 3);
        F0736_COMBAT_ResolveCreatureMelee_Compat(&attacker, &defender, &rng, &r);
        CHECK(r.outcome == COMBAT_OUTCOME_MISS && r.damageApplied == 0,
              "F0736 with defender.currentHealth=0 returns MISS, damage 0");
    }
    {
        struct ChampionState_Compat champ;
        struct CombatResult_Compat result;
        int killed = -1;
        memset(&champ, 0, sizeof(champ));
        champ.hp.current = 10;
        memset(&result, 0, sizeof(result));
        result.damageApplied  = 15;
        result.woundMaskAdded = COMBAT_WOUND_HEAD;
        F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &killed);
        CHECK(champ.hp.current == 0 && killed == 1,
              "F0737 with damage>=hp kills champion (hp=0, outKilled=1)");
    }
    {
        struct DungeonGroup_Compat group;
        struct CombatResult_Compat result;
        int outcome = -1;
        memset(&group, 0, sizeof(group));
        group.health[0] = 5;
        group.count = 0;  /* last creature alive */
        memset(&result, 0, sizeof(result));
        result.damageApplied = 5;
        F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outcome);
        CHECK(outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES && group.health[0] == 0,
              "F0738 kills last creature -> KILLED_ALL_CREATURES");
    }
    {
        struct DungeonGroup_Compat group;
        struct CombatResult_Compat result;
        int outcome = -1;
        memset(&group, 0, sizeof(group));
        group.health[0] = 5;
        group.count = 1;
        memset(&result, 0, sizeof(result));
        result.damageApplied = 2;
        F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outcome);
        CHECK(outcome == COMBAT_OUTCOME_KILLED_NO_CREATURES && group.health[0] == 3,
              "F0738 with damage<hp leaves slot alive, KILLED_NO_CREATURES");
    }
    {
        struct CombatAction_Compat action;
        unsigned char buf[COMBAT_ACTION_SERIALIZED_SIZE - 1];
        int rv;
        fill_populated_action(&action);
        rv = F0740_COMBAT_ActionSerialize_Compat(&action, buf, (int)sizeof(buf));
        CHECK(rv == 0,
              "F0740 with bufSize=47 rejects (action needs 48)");
    }
    {
        struct CombatAction_Compat action;
        unsigned char buf[COMBAT_ACTION_SERIALIZED_SIZE - 1];
        int rv;
        memset(&action, 0, sizeof(action));
        rv = F0741_COMBAT_ActionDeserialize_Compat(&action, buf, (int)sizeof(buf));
        CHECK(rv == 0,
              "F0741 with bufSize=47 rejects (action needs 48)");
    }
    {
        struct CombatResult_Compat result;
        unsigned char buf[COMBAT_RESULT_SERIALIZED_SIZE - 1];
        int rv;
        memset(&result, 0, sizeof(result));
        rv = F0743_COMBAT_ResultDeserialize_Compat(&result, buf, (int)sizeof(buf));
        CHECK(rv == 0,
              "F0743 with bufSize=55 rejects (result needs 56)");
    }

    /* ==============================================================
     *  Block F — known-value goldens (invariants 30–31)
     *  These use structural assertions so we don't fabricate numbers
     *  that would require bit-for-bit Borland rand() match. Determinism
     *  relative to our own rng is contract (see PHASE13_PLAN.md Risk R7).
     *  Actual bytes of the result are logged into combat_probe.md for
     *  future hardening into exact golden values.
     * ============================================================== */
    {
        struct CombatantCreatureSnapshot_Compat attacker;
        struct CombatantChampionSnapshot_Compat defender;
        struct RngState_Compat rng;
        struct CombatResult_Compat r;
        unsigned char buf[COMBAT_RESULT_SERIALIZED_SIZE];
        int i;
        int structural_ok;
        fill_mummy_attacker(&attacker);
        fill_hero_defender(&defender);
        F0730_COMBAT_RngInit_Compat(&rng, 0xC0FFEEu);
        F0736_COMBAT_ResolveCreatureMelee_Compat(&attacker, &defender, &rng, &r);
        F0742_COMBAT_ResultSerialize_Compat(&r, buf, sizeof(buf));

        fprintf(report, "## Golden: Mummy vs Hero (seed=0xC0FFEE)\n\n");
        fprintf(report, "- outcome=%d damageApplied=%d hitLanded=%d rngCallCount=%d\n",
                r.outcome, r.damageApplied, r.hitLanded, r.rngCallCount);
        fprintf(report, "- followupEventKind=%d woundMaskAdded=0x%04x poisonAttackPending=%d\n",
                r.followupEventKind, r.woundMaskAdded, r.poisonAttackPending);
        fprintf(report, "- bytes: ");
        for (i = 0; i < COMBAT_RESULT_SERIALIZED_SIZE; i++) {
            fprintf(report, "%02x", buf[i]);
        }
        fprintf(report, "\n\n");

        /* Structural golden:
         *   - outcome is one of MISS / HIT_DAMAGE
         *   - damageApplied in [0, 200]
         *   - followupEventKind == TIMELINE_EVENT_STATUS_TIMEOUT
         *   - rngCallCount > 0
         *   - woundMaskAdded is a valid wound bit or 0
         */
        structural_ok =
            (r.outcome == COMBAT_OUTCOME_MISS || r.outcome == COMBAT_OUTCOME_HIT_DAMAGE) &&
            r.damageApplied >= 0 && r.damageApplied <= 200 &&
            r.followupEventKind == TIMELINE_EVENT_STATUS_TIMEOUT &&
            r.rngCallCount > 0 &&
            (r.woundMaskAdded & ~0x0037) == 0;
        CHECK(structural_ok,
              "Golden Mummy vs Hero @ seed=0xC0FFEE matches structural envelope");
    }
    {
        struct CombatantChampionSnapshot_Compat attacker;
        struct WeaponProfile_Compat weapon;
        struct CombatantCreatureSnapshot_Compat defender;
        struct RngState_Compat rng;
        struct CombatResult_Compat r;
        unsigned char buf[COMBAT_RESULT_SERIALIZED_SIZE];
        int i;
        int structural_ok;
        fill_ninja_attacker(&attacker);
        fill_sword_weapon(&weapon);
        fill_skeleton_defender(&defender);
        F0730_COMBAT_RngInit_Compat(&rng, 0x12345678u);
        F0735_COMBAT_ResolveChampionMelee_Compat(&attacker, &weapon, &defender, &rng, &r);
        F0742_COMBAT_ResultSerialize_Compat(&r, buf, sizeof(buf));

        fprintf(report, "## Golden: Ninja champion vs Skeleton (seed=0x12345678)\n\n");
        fprintf(report, "- outcome=%d damageApplied=%d hitLanded=%d wasCritical=%d rngCallCount=%d\n",
                r.outcome, r.damageApplied, r.hitLanded, r.wasCritical, r.rngCallCount);
        fprintf(report, "- followupEventKind=%d defenseRoll=%d rawAttackRoll=%d\n",
                r.followupEventKind, r.defenseRoll, r.rawAttackRoll);
        fprintf(report, "- bytes: ");
        for (i = 0; i < COMBAT_RESULT_SERIALIZED_SIZE; i++) {
            fprintf(report, "%02x", buf[i]);
        }
        fprintf(report, "\n\n");

        structural_ok =
            (r.outcome == COMBAT_OUTCOME_MISS ||
             r.outcome == COMBAT_OUTCOME_HIT_NO_DAMAGE ||
             r.outcome == COMBAT_OUTCOME_HIT_DAMAGE) &&
            r.damageApplied >= 0 && r.damageApplied <= 500 &&
            r.followupEventKind == TIMELINE_EVENT_CREATURE_TICK &&
            r.rngCallCount > 0;
        CHECK(structural_ok,
              "Golden Ninja vs Skeleton @ seed=0x12345678 matches structural envelope");
    }

    /* ==============================================================
     *  Block G — integration (invariants 32–35)
     * ============================================================== */
    {
        struct CombatAction_Compat action;
        struct CombatResult_Compat result;
        struct TimelineEvent_Compat event;
        unsigned char buf[TIMELINE_EVENT_SERIALIZED_SIZE];
        int rv1, rv2;
        fill_populated_action(&action);
        memset(&result, 0, sizeof(result));
        fill_populated_result(&result);
        rv1 = F0739_COMBAT_BuildTimelineEvent_Compat(&action, &result, 1000, &event);
        rv2 = F0725_TIMELINE_EventSerialize_Compat(&event, buf, sizeof(buf));
        CHECK(rv1 == 1 && rv2 == 1 && sizeof(buf) == TIMELINE_EVENT_SERIALIZED_SIZE,
              "F0739 output serialises to phase-12 TimelineEvent (44 bytes)");
    }
    {
        struct CombatAction_Compat action;
        struct CombatResult_Compat result;
        struct TimelineEvent_Compat event;
        struct TimelineQueue_Compat qOrig, qRestored;
        unsigned char buf[TIMELINE_QUEUE_SERIALIZED_SIZE];
        fill_populated_action(&action);
        memset(&result, 0, sizeof(result));
        fill_populated_result(&result);
        F0739_COMBAT_BuildTimelineEvent_Compat(&action, &result, 2000, &event);
        F0720_TIMELINE_Init_Compat(&qOrig, 2000);
        F0721_TIMELINE_Schedule_Compat(&qOrig, &event);
        F0727_TIMELINE_QueueSerialize_Compat(&qOrig, buf, sizeof(buf));
        F0728_TIMELINE_QueueDeserialize_Compat(&qRestored, buf, sizeof(buf));
        CHECK(memcmp(&qOrig, &qRestored, sizeof(qOrig)) == 0,
              "Combat follow-up event scheduled into phase-12 queue survives round-trip");
    }
    {
        struct DungeonDatState_Compat dungeon;
        struct DungeonThings_Compat things;
        int all_in_range = 1;
        int examined = 0;
        int i;
        memset(&dungeon, 0, sizeof(dungeon));
        memset(&things, 0, sizeof(things));
        if (!F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, &dungeon)) {
            all_in_range = 0;
        } else if (!F0502_DUNGEON_LoadTileData_Compat(dungeonPath, &dungeon)) {
            F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
            all_in_range = 0;
        } else if (!F0504_DUNGEON_LoadThingData_Compat(dungeonPath, &dungeon, &things)) {
            F0502_DUNGEON_FreeTileData_Compat(&dungeon);
            F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
            all_in_range = 0;
        } else {
            for (i = 0; i < things.groupCount; i++) {
                examined++;
                if (things.groups[i].creatureType > DUNGEON_CREATURE_TYPE_MAX) {
                    all_in_range = 0;
                    break;
                }
            }
            F0504_DUNGEON_FreeThingData_Compat(&things);
            F0502_DUNGEON_FreeTileData_Compat(&dungeon);
            F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        }
        fprintf(report, "## DUNGEON.DAT integration\n\n- groups examined: %d\n- all creatureType <= %d: %s\n\n",
                examined, DUNGEON_CREATURE_TYPE_MAX, all_in_range ? "yes" : "no");
        CHECK(all_in_range && examined > 0,
              "DUNGEON.DAT integration: every group creatureType <= 26");
    }
    {
        struct ChampionState_Compat champ;
        struct CombatResult_Compat killResult;
        struct CombatantCreatureSnapshot_Compat attacker;
        struct CombatantChampionSnapshot_Compat defenderSnapshot;
        struct RngState_Compat rng;
        struct CombatResult_Compat r;
        int killed = 0;

        memset(&champ, 0, sizeof(champ));
        champ.hp.current = 10;
        memset(&killResult, 0, sizeof(killResult));
        killResult.damageApplied = 100;
        F0737_COMBAT_ApplyDamageToChampion_Compat(&killResult, &champ, &killed);

        /* Now run F0736 against a snapshot of the dead champion. */
        fill_mummy_attacker(&attacker);
        fill_hero_defender(&defenderSnapshot);
        defenderSnapshot.currentHealth = champ.hp.current;  /* == 0 */
        F0730_COMBAT_RngInit_Compat(&rng, 99);
        F0736_COMBAT_ResolveCreatureMelee_Compat(&attacker, &defenderSnapshot, &rng, &r);
        CHECK(killed == 1 && r.outcome == COMBAT_OUTCOME_MISS && r.damageApplied == 0,
              "Dead champion is ignored by subsequent F0736 (CHAMPION.C:1814)");
    }

    /* Trailer with total count and final status. */
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
