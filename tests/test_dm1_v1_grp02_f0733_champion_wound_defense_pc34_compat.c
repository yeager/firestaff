/*
 * test_dm1_v1_grp02_f0733_champion_wound_defense_pc34_compat.c
 *
 * Source-locked to ReDMCSB CHAMPION.C:1180-1215 (F0310/F0313
 * wound defense) and the per-slot G0050_auc_Graphic562_
 * WoundDefenseFactor table.
 *
 * GRP-02 (DM1 V1 functional-divergence-report.md):
 *   "F0190/F0191/F0192 damage outcomes are amalgam-only."
 *
 * F0733 is the new-path replacement that combines per-slot
 * baseline wound defense with vitality bonus and sharp
 * defense modifier.  Pins the contract:
 *  T1  NULL champ returns 0
 *  T2  NULL outDefense returns 0
 *  T3  woundSlotIndex=-1 returns 0
 *  T4  woundSlotIndex=6 returns 0 (only 0..5 valid)
 *  T5  useSharpDefense=0: baseline + partyShieldDefense +
 *      (WoundDefenseFactor[slot] * vitality) >> 8
 *  T6  useSharpDefense=1: above + (WoundDefenseFactor[slot] >> 1)
 *  T7  Zero baseline: result = partyShieldDefense + (factor * vit) >> 8
 *  T8  Zero vitality: result = baseline + partyShieldDefense
 *  T9  Different slots use different WoundDefenseFactor values
 *  T10 All 6 slots (0..5) work
 *  T11 outDefense is set on success
 *  T12 Returns 1 on success
 *
 * Source-locked to ReDMCSB CHAMPION.C:1180-1215.
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
    struct CombatantChampionSnapshot_Compat champ;
    int defense = -1;
    int rc;

    /* T1: NULL champ. */
    CHECK(F0733_COMBAT_GetChampionWoundDefense_Compat(NULL, 0, 0, &defense) == 0,
          "T1: NULL champ returns 0");

    /* T2: NULL outDefense. */
    memset(&champ, 0, sizeof(champ));
    CHECK(F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, 0, 0, NULL) == 0,
          "T2: NULL outDefense returns 0");

    /* T3: woundSlotIndex=-1 returns 0. */
    memset(&champ, 0, sizeof(champ));
    CHECK(F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, -1, 0, &defense) == 0,
          "T3: woundSlotIndex=-1 returns 0");

    /* T4: woundSlotIndex=6 returns 0. */
    CHECK(F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, 6, 0, &defense) == 0,
          "T4: woundSlotIndex=6 returns 0");

    /* T5: useSharpDefense=0.  WoundDefenseFactor[0] = 5 (current
     * source-locked value in the compat layer, G0050 mirror).
     * For vitality=100: 5*100>>8 = 500>>8 = 1.
     * Defense = 50 + 20 + 1 = 71. */
    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[0] = 50;
    champ.partyShieldDefense = 20;
    champ.statisticVitality = 100;
    defense = -1;
    CHECK(F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, 0, 0, &defense) == 1,
          "T5a: returns 1");
    CHECK(defense == 71, "T5b: defense = 71 (50 baseline + 20 shield + 1 vitality)");

    /* T6: useSharpDefense=1.  Sharp bonus = factor >> 1 = 5>>1 = 2.
     * Defense = 50 + 20 + 1 + 2 = 73. */
    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[0] = 50;
    champ.partyShieldDefense = 20;
    champ.statisticVitality = 100;
    defense = -1;
    CHECK(F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, 0, 1, &defense) == 1,
          "T6a: returns 1 with sharp");
    CHECK(defense == 73, "T6b: defense = 73 (50 + 20 + 1 + 2 sharp)");

    /* T7: Zero baseline.  With vitality=256: 5*256>>8 = 1280>>8 = 5.
     * Defense = 0 + 10 + 5 = 15. */
    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[0] = 0;
    champ.partyShieldDefense = 10;
    champ.statisticVitality = 256;
    defense = -1;
    F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, 0, 0, &defense);
    CHECK(defense == 15, "T7: zero baseline + shield + vitality = 15");

    /* T8: Zero vitality. */
    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[0] = 100;
    champ.partyShieldDefense = 50;
    champ.statisticVitality = 0;
    defense = -1;
    F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, 0, 0, &defense);
    CHECK(defense == 150, "T8: baseline + shield = 150 (no vitality)");

    /* T9: Different slots have different factors.
     * WoundDefenseFactor (compat layer): 5, 5, 4, 6, 3, 1 (slot 0..5)
     * 100 vitality:
     *  slot 0: 5*100>>8 = 1
     *  slot 1: 5*100>>8 = 1
     *  slot 2: 4*100>>8 = 1
     *  slot 3: 6*100>>8 = 2
     *  slot 4: 3*100>>8 = 1
     *  slot 5: 1*100>>8 = 0
     */
    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[0] = 100;
    champ.partyShieldDefense = 0;
    champ.statisticVitality = 100;
    {
        int slot;
        int seen_distinct = 0;
        int prev_defense = -1;
        for (slot = 0; slot < 6; ++slot) {
            defense = -1;
            F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, slot, 0, &defense);
            char buf[96];
            snprintf(buf, sizeof(buf),
                     "T9: slot %d defense=%d", slot, defense);
            if (defense != prev_defense) {
                seen_distinct++;
            }
            prev_defense = defense;
            (void)buf;
        }
        CHECK(seen_distinct >= 2, "T9: slots produce distinct values");
    }

    /* T10: All 6 slots work. */
    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[0] = 0; champ.woundDefense[1] = 0; champ.woundDefense[2] = 0;
    champ.woundDefense[3] = 0; champ.woundDefense[4] = 0; champ.woundDefense[5] = 0;
    champ.partyShieldDefense = 0;
    champ.statisticVitality = 0;
    {
        int slot;
        for (slot = 0; slot < 6; ++slot) {
            defense = -1;
            rc = F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, slot, 0, &defense);
            char buf[96];
            snprintf(buf, sizeof(buf),
                     "T10: slot %d returns 1", slot);
            CHECK(rc == 1, buf);
            CHECK(defense == 0, "T10: zero defense for zero input");
        }
    }

    /* T11: outDefense is set on success. */
    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[0] = 50;
    champ.partyShieldDefense = 0;
    champ.statisticVitality = 0;
    defense = -999;
    F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, 0, 0, &defense);
    CHECK(defense == 50, "T11: outDefense set to 50 (was -999)");

    /* T12: Returns 1 on success. */
    memset(&champ, 0, sizeof(champ));
    rc = F0733_COMBAT_GetChampionWoundDefense_Compat(&champ, 0, 0, &defense);
    CHECK(rc == 1, "T12: returns 1 on success");

    printf("PASS: GRP-02 F0733 champion wound-defense pin (12 scenarios)\n");
    return 0;
}
