#ifndef INVENTORY_ITEM_IDENTIFICATION_PC34_COMPAT_H
#define INVENTORY_ITEM_IDENTIFICATION_PC34_COMPAT_H

#include <stddef.h>

typedef struct InventoryPotionEyeDescriptionPc34Compat {
    unsigned int isPotion;
    unsigned int usesPowerPrefix;
    char powerSymbol;
    const char* sourceEvidence;
} InventoryPotionEyeDescriptionPc34Compat;

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
const char* INVENTORY_Compat_GetItemIdentificationEvidence(void);

#endif
