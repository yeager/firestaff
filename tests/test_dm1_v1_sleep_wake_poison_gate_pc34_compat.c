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

int main(void)
{
    test_poison_cloud_wakes_resting_champion();
    test_creature_melee_wakes_resting_champion();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
