#include "inventory_item_identification_pc34_compat.h"

#include <stdio.h>

#define DM1_THING_TYPE_POTION_PC34_COMPAT 8u
#define DM1_ICON_POTION_WATER_FLASK_PC34_COMPAT 163u

static const char* kItemIdentificationEvidence =
    "ReDMCSB source lock: PANEL.C:1055-1138 F0342 routes scrolls to F0341, containers to F0333, and other leader-hand objects to object descriptions; PANEL.C:1182-1191 prefixes non-water-flask potion names with '_' + Power/40 when the inventory champion Priest skill is > 1, then falls back to G0352 object names otherwise; PANEL.C:1198-1200 prints the description and icon; PANEL.C:1276-1294 marks potions consumable through G0237 object info; DUNGEON.C:83-98 and 103 define potion object-info icon rows including water flask C163 and empty flask C195; DUNGEON.C:1157-1158 maps potion things to C002 + Potion.Type; OBJECT.C:25-119 loads G0352 object names.";

unsigned int INVENTORY_Compat_PotionEyeShowsPowerPrefix(unsigned int thingType,
                                                       unsigned int iconIndex,
                                                       unsigned int priestSkillLevel) {
    return thingType == DM1_THING_TYPE_POTION_PC34_COMPAT &&
           iconIndex != DM1_ICON_POTION_WATER_FLASK_PC34_COMPAT &&
           priestSkillLevel > 1u;
}

char INVENTORY_Compat_PotionPowerSymbol(unsigned int potionPower) {
    return (char)('_' + (potionPower / 40u));
}

int INVENTORY_Compat_FormatPotionEyeDescription(unsigned int thingType,
                                                unsigned int iconIndex,
                                                unsigned int potionPower,
                                                unsigned int priestSkillLevel,
                                                const char* objectName,
                                                char* outText,
                                                size_t outTextSize,
                                                InventoryPotionEyeDescriptionPc34Compat* outDescription) {
    int written;
    const unsigned int showsPrefix = INVENTORY_Compat_PotionEyeShowsPowerPrefix(
        thingType, iconIndex, priestSkillLevel);
    const char powerSymbol = INVENTORY_Compat_PotionPowerSymbol(potionPower);
    if (!outText || outTextSize == 0u) {
        return 0;
    }
    if (!objectName) {
        objectName = "";
    }

    if (showsPrefix) {
        written = snprintf(outText, outTextSize, "%c %s", powerSymbol, objectName);
    } else {
        written = snprintf(outText, outTextSize, "%s", objectName);
    }

    if (outDescription) {
        outDescription->isPotion = (thingType == DM1_THING_TYPE_POTION_PC34_COMPAT) ? 1u : 0u;
        outDescription->usesPowerPrefix = showsPrefix;
        outDescription->powerSymbol = showsPrefix ? powerSymbol : '\0';
        outDescription->sourceEvidence = kItemIdentificationEvidence;
    }
    return written >= 0 && (size_t)written < outTextSize;
}

const char* INVENTORY_Compat_GetItemIdentificationEvidence(void) {
    return kItemIdentificationEvidence;
}
