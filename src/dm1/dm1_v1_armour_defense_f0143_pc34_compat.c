#include "dm1_v1_armour_defense_f0143_pc34_compat.h"

const char* DM1_V1_F0143_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1232 F0143_DUNGEON_GetArmourDefense takes "
        "P0239_ps_ArmourInfo and P0240_B_UseSharpDefense\n"
        "DUNGEON.C:1237 stores the computed Defense local\n"
        "DUNGEON.C:G0239_as_Graphic559_ArmourInfo[58] supplies base "
        "armour defense and sharp-defense bits consumed by CHAMPION.C F0313";
}

int F0143_DUNGEON_GetArmourDefense(
    const DM1_V1_F0143_ArmourInfoPc34* armourInfo,
    int useSharpDefense)
{
    int defense;

    if (!armourInfo) {
        return 0;
    }

    defense = (int)armourInfo->defense;
    if (useSharpDefense) {
        defense += (int)armourInfo->sharpDefense;
    }
    return defense;
}

int DM1_V1_Dungeon_GetArmourDefenseF0143Pc34Compat(
    const DM1_V1_F0143_ArmourInfoPc34* armourInfo,
    int useSharpDefense)
{
    return F0143_DUNGEON_GetArmourDefense(armourInfo, useSharpDefense);
}
