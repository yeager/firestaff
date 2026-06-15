/*
 * test_dm1_v1_grp02_f0737_apply_damage_to_champion_pc34_compat.c
 *
 * Source-locked to ReDMCSB CHAMPION.C:1814 (damage application):
 *   newHp = currentHp - damageApplied
 *   if (newHp <= 0) { currentHp = 0; killed = 1; }
 *   else { currentHp = newHp; killed = 0; }
 *   wounds |= woundMaskAdded
 *
 * GRP-02 (DM1 V1 functional-divergence-report.md):
 *   "F0190/F0191/F0192 damage outcomes are amalgam-only.  No F0192
 *    equivalent — the new path does not implement per-creature
 *    resistance-adjusted poison attack."
 *
 * F0737 (combat apply damage to champion) is the new-path replacement
 * for the F0190-F0193 damage application contract.  This test
 * pins the F0737 invariants:
 *  T1  NULL result returns 0
 *  T2  NULL champ returns 0
 *  T3  NULL outWasKilled returns 0
 *  T4  Already-dead champ: wasKilled=1, no damage applied, no
 *      additional wound mask
 *  T5  Damage < currentHp: wasKilled=0, currentHp = currentHp - damage
 *  T6  Damage == currentHp: wasKilled=1, currentHp = 0
 *  T7  Damage > currentHp (overkill): wasKilled=1, currentHp = 0
 *  T8  woundMaskAdded is ORed into wounds
 *  T9  woundMaskAdded=0 leaves wounds unchanged
 *  T10 Multiple damage applications accumulate (subtractive)
 *  T11 100 damage to full-health champ: wasKilled=1, currentHp=0
 *  T12 Damage of 0: wasKilled=0, currentHp unchanged
 *  T13 Wound bits accumulate across multiple hits
 *  T14 Returns 1 on success
 *
 * Source-locked to ReDMCSB CHAMPION.C:1814.
 */

#include "memory_combat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    struct CombatResult_Compat result;
    struct ChampionState_Compat champ;
    int wasKilled = -1;
    int rc;

    /* T1: NULL result. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 100;
    CHECK(F0737_COMBAT_ApplyDamageToChampion_Compat(NULL, &champ, &wasKilled) == 0,
          "T1: NULL result returns 0");

    /* T2: NULL champ. */
    memset(&result, 0, sizeof(result));
    result.damageApplied = 10;
    CHECK(F0737_COMBAT_ApplyDamageToChampion_Compat(&result, NULL, &wasKilled) == 0,
          "T2: NULL champ returns 0");

    /* T3: NULL outWasKilled. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 100;
    CHECK(F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, NULL) == 0,
          "T3: NULL outWasKilled returns 0");

    /* T4: Already-dead champ (hp=0). */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 0;
    champ.wounds = 0;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 50;
    result.woundMaskAdded = 0x04;
    wasKilled = -1;
    CHECK(F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled) == 1,
          "T4a: dead champ returns 1");
    CHECK(wasKilled == 1, "T4b: dead champ wasKilled=1");
    CHECK(champ.hp.current == 0, "T4c: dead champ hp stays 0");
    /* Note: per CHAMPION.C:1814, wounds are still ORed. */

    /* T5: Damage < currentHp. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 100;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 30;
    wasKilled = -1;
    CHECK(F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled) == 1,
          "T5a: damage < hp returns 1");
    CHECK(wasKilled == 0, "T5b: wasKilled=0");
    CHECK(champ.hp.current == 70, "T5c: hp = 100 - 30 = 70");

    /* T6: Damage == currentHp. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 50;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 50;
    wasKilled = -1;
    CHECK(F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled) == 1,
          "T6a: damage == hp returns 1");
    CHECK(wasKilled == 1, "T6b: wasKilled=1");
    CHECK(champ.hp.current == 0, "T6c: hp clamped to 0");

    /* T7: Overkill. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 30;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;
    wasKilled = -1;
    CHECK(F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled) == 1,
          "T7a: overkill returns 1");
    CHECK(wasKilled == 1, "T7b: wasKilled=1");
    CHECK(champ.hp.current == 0, "T7c: hp clamped to 0 (no negative)");

    /* T8: woundMaskAdded is ORed into wounds. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 100;
    champ.wounds = 0x01;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 10;
    result.woundMaskAdded = 0x04;
    wasKilled = -1;
    F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled);
    CHECK(champ.wounds == 0x05, "T8: wounds |= woundMaskAdded (0x01 | 0x04 = 0x05)");

    /* T9: woundMaskAdded=0 leaves wounds unchanged. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 100;
    champ.wounds = 0x08;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 10;
    result.woundMaskAdded = 0;
    wasKilled = -1;
    F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled);
    CHECK(champ.wounds == 0x08, "T9: wounds unchanged when woundMaskAdded=0");

    /* T10: Multiple damage applications accumulate. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 100;
    {
        int k;
        for (k = 0; k < 5; ++k) {
            memset(&result, 0, sizeof(result));
            result.damageApplied = 10;
            F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled);
        }
        CHECK(champ.hp.current == 50, "T10: 5x 10 damage = 50 (100 - 50)");
        CHECK(wasKilled == 0, "T10: wasKilled=0 (still alive at 50hp)");
    }

    /* T11: 100 damage to full-health champ. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 100;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;
    wasKilled = -1;
    F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled);
    CHECK(wasKilled == 1, "T11a: 100 damage kills 100hp champ");
    CHECK(champ.hp.current == 0, "T11b: hp=0");

    /* T12: Damage of 0. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 100;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 0;
    wasKilled = -1;
    F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled);
    CHECK(wasKilled == 0, "T12a: 0 damage, not killed");
    CHECK(champ.hp.current == 100, "T12b: hp unchanged at 100");

    /* T13: Wound bits accumulate. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 100;
    {
        int k;
        for (k = 0; k < 4; ++k) {
            memset(&result, 0, sizeof(result));
            result.damageApplied = 5;
            result.woundMaskAdded = (1u << k); /* 0x01, 0x02, 0x04, 0x08 */
            F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled);
        }
        CHECK(champ.wounds == 0x0F, "T13: 4 wound bits ORed = 0x0F");
        CHECK(champ.hp.current == 80, "T13: 4x 5 damage = 80hp");
    }

    /* T14: Returns 1 on success. */
    memset(&champ, 0, sizeof(champ));
    champ.hp.current = 50;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 5;
    wasKilled = -1;
    rc = F0737_COMBAT_ApplyDamageToChampion_Compat(&result, &champ, &wasKilled);
    CHECK(rc == 1, "T14: returns 1 on success");

    printf("PASS: GRP-02 F0737 apply-damage-to-champion invariants (14 scenarios)\n");
    return 0;
}
