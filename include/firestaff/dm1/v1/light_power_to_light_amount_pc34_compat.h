#ifndef FIRESTAFF_DM1_V1_LIGHT_POWER_TO_LIGHT_AMOUNT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_LIGHT_POWER_TO_LIGHT_AMOUNT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0039_ai_Graphic562_LightPowerToLightAmount[16].
 *
 * G0039 is the per-light-power lookup table that maps a torch's
 * remaining light power (0..15) to its effective magical light amount
 * (0..100). The values form a near-saturating curve: 0, 5, 12, 24,
 * 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100. The dungeon-view
 * palette index (G0040) and the per-tick MagicalLightAmount delta
 * (G0407_s_Party.MagicalLightAmount) both depend on this table.
 *
 * Read sites:
 * - PANEL.C:412 F0337_INVENTORY_SetDungeonViewPalette:
 *       TotalLightAmount += (G0039[TorchLightPower] << Multiplier) >> 6
 * - CHAMPION.C:529 F0291_CHAMPION_DrawSlot:
 *       MagicalLightAmount -= G0039[2]   (Illumulet unequip)
 * - CHAMPION.C:645 F0291_CHAMPION_DrawSlot:
 *       MagicalLightAmount += G0039[2]   (Illumulet equip)
 * - MENU.C:1608 C038_ACTION_LIGHT:
 *       MagicalLightAmount += G0039[2]   (Light spell)
 * - MENU.C:1936 C5_SPELL_TYPE_OTHER_MAGIC_TORCH:
 *       MagicalLightAmount += G0039[LightPower]
 * - MENU.C:1941 C1_SPELL_TYPE_OTHER_DARKNESS:
 *       MagicalLightAmount -= G0039[LightPower]
 * - TIMELINE.C:1754 F0..._LIGHT_ProcessEvent:
 *       Diff = G0039[Strong] - G0039[Weaker]
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes). This gate is a non-mirror-candidate contract for
 * the G0039 light-power lookup table.
 */

#define DM1_V1_LIGHT_POWER_TABLE_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_LightPowerToLightAmountResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_LIGHT_POWER_TABLE_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntryZero;
    int lastEntry100;
    int monotonicallyNonDecreasing;
    int allWithinRange0_100;
    int illumuletConstant12;
    int lookupFunctionInRange;
    int lookupOutOfRangeReturnsZero;
    int diffHelperCorrect;
    int diffHelperSignAware;
    int diffHelperZeroWhenSame;
} DM1_V1_LightPowerToLightAmountResultPc34;

const int *
dm1_v1_light_power_to_light_amount_table_pc34(void);

int
dm1_v1_light_power_to_light_amount_size_pc34(void);

int
dm1_v1_light_power_to_light_amount_pc34(int light_power);

int
dm1_v1_light_power_to_light_amount_diff_pc34(
    int stronger_light_power,
    int weaker_light_power);

int
dm1_v1_light_power_to_light_amount_illumulet_index_pc34(void);

int
dm1_v1_light_power_to_light_amount_illumulet_amount_pc34(void);

int
dm1_v1_light_power_to_light_amount_max_value_pc34(void);

int
dm1_v1_light_power_to_light_amount_run_pc34(
    DM1_V1_LightPowerToLightAmountResultPc34 *out);

#endif