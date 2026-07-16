#ifndef FIRESTAFF_DM1_V1_ARMOUR_DEFENSE_F0143_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ARMOUR_DEFENSE_F0143_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t defense;
    uint8_t sharpDefense;
    uint16_t attributes;
} DM1_V1_F0143_ArmourInfoPc34;

const char* DM1_V1_F0143_SourceEvidencePc34(void);

int F0143_DUNGEON_GetArmourDefense(
    const DM1_V1_F0143_ArmourInfoPc34* armourInfo,
    int useSharpDefense);

int DM1_V1_Dungeon_GetArmourDefenseF0143Pc34Compat(
    const DM1_V1_F0143_ArmourInfoPc34* armourInfo,
    int useSharpDefense);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_ARMOUR_DEFENSE_F0143_PC34_COMPAT_H */
