#include "memory_magic_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL: %s\n", msg); \
    } \
} while (0)

#define CHECK_EQ_INT(got, want, msg) do { \
    int got_value__ = (got); \
    int want_value__ = (want); \
    if (got_value__ == want_value__) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL: %s got=%d want=%d\n", \
                msg, got_value__, want_value__); \
    } \
} while (0)

#define CHECK_EQ_U32(got, want, msg) do { \
    unsigned int got_value__ = (unsigned int)(got); \
    unsigned int want_value__ = (unsigned int)(want); \
    if (got_value__ == want_value__) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL: %s got=0x%08x want=0x%08x\n", \
                msg, got_value__, want_value__); \
    } \
} while (0)

static struct CombatantChampionSnapshot_Compat make_resting_defender(void)
{
    struct CombatantChampionSnapshot_Compat champ;
    memset(&champ, 0, sizeof(champ));
    champ.championIndex = 0;
    champ.currentHealth = 120;
    champ.dexterity = 24;
    champ.skillLevelParry = 0;
    champ.skillLevelAction = 0;
    champ.statisticVitality = 20;
    champ.statisticAntifire = 0;
    champ.statisticAntimagic = 0;
    champ.actionHandIcon = 0;
    champ.isResting = 1;
    return champ;
}

static void test_poison_cloud_wakes_resting_champion(void)
{
    struct SpellEffect_Compat effect;
    struct CombatantChampionSnapshot_Compat champ;
    struct MagicState_Compat magic;
    struct CombatResult_Compat result;

    /*
     * ReDMCSB CHAMPION.C F0314: a resting defender wakes before damage is
     * applied.  F0759 keeps wakeFromRest and poisonAttackPending together
     * in the same resolved hit, which is the sleep/wake gate we want to pin.
     */
    memset(&effect, 0, sizeof(effect));
    effect.spellType = 7;  /* poison cloud */
    effect.impactAttack = 64;
    effect.poisonAttackPending = 3;
    effect.followupEventAux0 = 0x55AA;

    champ = make_resting_defender();
    memset(&magic, 0, sizeof(magic));

    CHECK(F0759_MAGIC_ApplySpellImpactToChampion_Compat(
              &effect, &champ, &magic, NULL, &result) == 1,
          "poison cloud spell impact resolves");
    CHECK(result.wakeFromRest == 1,
          "poison cloud wakes a resting champion");
    CHECK(result.poisonAttackPending == 3,
          "poison cloud preserves pending poison follow-up");
    CHECK(result.hitLanded == 1,
          "poison cloud lands as damage when unresisted");
    CHECK(result.damageApplied == 64,
          "poison cloud keeps the raw attack value in the compat layer");
    CHECK(result.followupEventAux0 == 0x55AA,
          "poison cloud keeps the follow-up event payload");
}

static void test_creature_melee_wakes_resting_champion(void)
{
    struct CombatantCreatureSnapshot_Compat attacker;
    struct CombatantChampionSnapshot_Compat defender;
    struct RngState_Compat rng;
    struct CombatResult_Compat result;

    /*
     * ReDMCSB PROJEXPL.C F0230 and CHAMPION.C F0314: creature damage wakes
     * the party before the rest-hit path continues.
     */
    memset(&attacker, 0, sizeof(attacker));
    attacker.attack = 40;
    attacker.dexterity = 8;
    attacker.doubledMapDifficulty = 0;
    attacker.attackType = COMBAT_ATTACK_NORMAL;
    attacker.woundProbabilities = 0x0000;

    defender = make_resting_defender();

    CHECK(F0730_COMBAT_RngInit_Compat(&rng, 1234u) == 1,
          "rng initialises for creature melee");
    CHECK(F0736_COMBAT_ResolveCreatureMelee_Compat(
              &attacker, &defender, &rng, &result) == 1,
          "creature melee resolves");
    CHECK(result.wakeFromRest == 1,
          "creature melee wakes a resting champion");
    CHECK(result.outcome == COMBAT_OUTCOME_HIT_DAMAGE,
          "creature melee still resolves as a hit");
    CHECK(result.damageApplied > 0,
          "creature melee applies non-zero damage");
}

static void test_creature_melee_f0230_late_reduction_fixture(void)
{
    struct CombatantCreatureSnapshot_Compat attacker;
    struct CombatantChampionSnapshot_Compat defender;
    struct RngState_Compat rng;
    struct CombatResult_Compat result;
    int i;

    /*
     * ReDMCSB PROJEXPL.C F0230 lines 1390-1404 + CHAMPION.C F0321
     * lines 1838-1900: after the staged creature attack roll, the PC34
     * path applies a final 50% random reduction; F0321 then calls F0313 for
     * the chosen wound bit (FEET) and scales attack by
     * (130 - avg_defense) / 64. Seed 4 takes the
     * 50%-reduction branch and the BLUNT (C3) attack type, so:
     *   atk after late reduction = 19
     *   F0313(FEET) consumes RANDOM((45 >> 3) + 1) after the F0230 rolls
     *   F0321 scale = 37 for Firestaff's deterministic RNG seed 4
     * rngCallCount and final seed lock the late random term, the F0313
     * vitality roll, and the post-scale output.
     */
    memset(&attacker, 0, sizeof(attacker));
    attacker.creatureType = 14;
    attacker.attack = 24;
    attacker.defense = 18;
    attacker.dexterity = 30;
    attacker.baseHealth = 90;
    attacker.attackType = COMBAT_ATTACK_BLUNT;
    attacker.woundProbabilities = 0x8421;
    attacker.doubledMapDifficulty = 2;
    attacker.healthBefore = 90;

    memset(&defender, 0, sizeof(defender));
    defender.championIndex = 0;
    defender.currentHealth = 50;
    defender.dexterity = 40;
    defender.statisticVitality = 45;
    defender.statisticAntifire = 30;
    defender.statisticAntimagic = 30;
    for (i = 0; i < 6; ++i) {
        defender.woundDefense[i] = 5;
    }

    CHECK(F0730_COMBAT_RngInit_Compat(&rng, 4u) == 1,
          "rng initialises for F0230 late-reduction fixture");
    CHECK(F0736_COMBAT_ResolveCreatureMelee_Compat(
              &attacker, &defender, &rng, &result) == 1,
          "F0230 late-reduction fixture resolves");
    CHECK(result.outcome == COMBAT_OUTCOME_HIT_DAMAGE,
          "F0230 fixture lands damage");
    CHECK_EQ_INT(result.damageApplied, 37,
                 "F0230 fixture applies exact F0321 armor+defense scale");
    CHECK(result.rawAttackRoll == 25,
          "F0230 fixture keeps exact dodge random term");
    CHECK(result.woundMaskAdded == COMBAT_WOUND_FEET,
          "F0230 fixture selects exact wound mask");
    CHECK_EQ_INT(result.rngCallCount, 12,
                 "F0230 fixture consumes late reduction random term");
    CHECK_EQ_U32(rng.seed, 0x8a798588u,
                 "F0230 fixture leaves exact rng seed");
}

static void test_creature_melee_f0321_armor_defense_scale_fixture(void)
{
    struct CombatantCreatureSnapshot_Compat attacker;
    struct CombatantChampionSnapshot_Compat defender;
    struct RngState_Compat rng;
    struct CombatResult_Compat result;

    /*
     * ReDMCSB CHAMPION.C F0321 lines 1838-1900: the F0230 handoff routes
     * through F0321 which (1) calls F0313 wound defense per allowed slot,
     * (2) averages, and (3) scales attack by (130 - avg_defense) / 64.
     * Seed 37 selects the HEAD wound bit for the SHARP (C4) attacker,
     * which exercises the useSharpDefense path of F0733. The fixture
     * has varied per-slot woundDefense and high vitality (80) so the
     * F0313 vitality term contributes non-trivially.
     *
     * Walk-through with seed 37:
     *   rand1=9, rand2=0 -> dexFails (resting skipped; attack lands)
     *   woundTest selects HEAD (slot 1) via prob table 0x8421
     *   staged attack roll: rnd16=10, atk stages -> atk=12
     *   reduceGate=0 -> no F0230 late reduction (atk stays 12)
     *   F0313(HEAD, sharp=1, vit=80, baseline=4) consumes
     *     RANDOM((80 >> 3) + 1), halves that random component because this
     *     is sharp damage, then applies the final half-scale.
     *   F0321 scale yields 23 for Firestaff's deterministic RNG seed 37.
     */
    memset(&attacker, 0, sizeof(attacker));
    attacker.creatureType = 7;
    attacker.attack = 35;
    attacker.defense = 18;
    attacker.dexterity = 28;
    attacker.baseHealth = 90;
    attacker.attackType = COMBAT_ATTACK_SHARP;
    attacker.woundProbabilities = 0x8421;
    attacker.doubledMapDifficulty = 0;
    attacker.healthBefore = 90;

    memset(&defender, 0, sizeof(defender));
    defender.championIndex = 1;
    defender.currentHealth = 50;
    defender.dexterity = 60;
    defender.statisticVitality = 80;
    defender.statisticAntifire = 30;
    defender.statisticAntimagic = 30;
    defender.woundDefense[0] = 2;
    defender.woundDefense[1] = 4;
    defender.woundDefense[2] = 6;
    defender.woundDefense[3] = 8;
    defender.woundDefense[4] = 10;
    defender.woundDefense[5] = 12;

    CHECK(F0730_COMBAT_RngInit_Compat(&rng, 37u) == 1,
          "rng initialises for F0321 armor+defense scale fixture");
    CHECK(F0736_COMBAT_ResolveCreatureMelee_Compat(
              &attacker, &defender, &rng, &result) == 1,
          "F0321 armor+defense scale fixture resolves");
    CHECK(result.outcome == COMBAT_OUTCOME_HIT_DAMAGE,
          "F0321 fixture lands damage");
    CHECK(result.woundMaskAdded == COMBAT_WOUND_HEAD,
          "F0321 fixture selects HEAD wound slot for F0313 lookup");
    CHECK_EQ_INT(result.damageApplied, 23,
                 "F0321 fixture applies exact (130 - defense) / 64 scale");
    CHECK_EQ_INT(result.rngCallCount, 11,
                 "F0321 fixture skips the F0230 late-reduction random term");
    CHECK_EQ_U32(rng.seed, 0xd11cf620u,
                 "F0321 fixture leaves exact rng seed");
}

int main(void)
{
    test_poison_cloud_wakes_resting_champion();
    test_creature_melee_wakes_resting_champion();
    test_creature_melee_f0230_late_reduction_fixture();
    test_creature_melee_f0321_armor_defense_scale_fixture();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
