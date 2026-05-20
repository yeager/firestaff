#ifndef INVENTORY_ITEM_IDENTIFICATION_PC34_COMPAT_H
#define INVENTORY_ITEM_IDENTIFICATION_PC34_COMPAT_H

#include <stddef.h>

typedef struct InventoryPotionEyeDescriptionPc34Compat {
    unsigned int isPotion;
    unsigned int usesPowerPrefix;
    char powerSymbol;
    const char* sourceEvidence;
} InventoryPotionEyeDescriptionPc34Compat;

typedef struct InventoryWeaponEyeDescriptionPc34Compat {
    unsigned int isWeapon;
    unsigned int potentialAttributesMask;
    unsigned int actualAttributesMask;
    const char* sourceEvidence;
} InventoryWeaponEyeDescriptionPc34Compat;

unsigned int INVENTORY_Compat_PotionEyeShowsPowerPrefix(unsigned int thingType,
                                                       unsigned int iconIndex,
                                                       unsigned int priestSkillLevel);
char INVENTORY_Compat_PotionPowerSymbol(unsigned int potionPower);
int INVENTORY_Compat_FormatPotionEyeDescription(unsigned int thingType,
                                                unsigned int iconIndex,
                                                unsigned int potionPower,
                                                unsigned int priestSkillLevel,
                                                const char* objectName,
                                                char* outText,
                                                size_t outTextSize,
                                                InventoryPotionEyeDescriptionPc34Compat* outDescription);
unsigned int INVENTORY_Compat_WeaponEyePotentialAttributesMask(void);
unsigned int INVENTORY_Compat_WeaponEyeActualAttributesMask(unsigned int cursed,
                                                           unsigned int poisoned,
                                                           unsigned int broken);
int INVENTORY_Compat_FormatWeaponEyeDescription(unsigned int thingType,
                                                unsigned int cursed,
                                                unsigned int poisoned,
                                                unsigned int broken,
                                                const char* objectName,
                                                char* outNameText,
                                                size_t outNameTextSize,
                                                char* outAttributeText,
                                                size_t outAttributeTextSize,
                                                InventoryWeaponEyeDescriptionPc34Compat* outDescription);
const char* INVENTORY_Compat_GetItemIdentificationEvidence(void);

#endif
