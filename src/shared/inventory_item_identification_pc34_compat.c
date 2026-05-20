#include "inventory_item_identification_pc34_compat.h"

#include <stdio.h>

#define DM1_THING_TYPE_POTION_PC34_COMPAT 8u
#define DM1_THING_TYPE_WEAPON_PC34_COMPAT 5u
#define DM1_ICON_POTION_WATER_FLASK_PC34_COMPAT 163u
#define DM1_DESCRIPTION_POISONED_MASK_PC34_COMPAT 0x0002u
#define DM1_DESCRIPTION_BROKEN_MASK_PC34_COMPAT   0x0004u
#define DM1_DESCRIPTION_CURSED_MASK_PC34_COMPAT   0x0008u

static const char* kAttributeNames[] = {
    "CONSUMABLE",
    "POISONED",
    "BROKEN",
    "CURSED"
};

static const char* kItemIdentificationEvidence =
    "ReDMCSB source lock: PANEL.C:1055-1138 F0342 routes scrolls to F0341, containers to F0333, and other leader-hand objects to object descriptions; PANEL.C:1182-1191 prefixes non-water-flask potion names with '_' + Power/40 when the inventory champion Priest skill is > 1, then falls back to G0352 object names otherwise; PANEL.C:1198-1200 prints the description and icon; PANEL.C:1276-1294 marks potions consumable through G0237 object info; DUNGEON.C:83-98 and 103 define potion object-info icon rows including water flask C163 and empty flask C195; DUNGEON.C:1157-1158 maps potion things to C002 + Potion.Type; OBJECT.C:25-119 loads G0352 object names.";
static const char* kWeaponIdentificationEvidence =
    "ReDMCSB source lock: PANEL.C:1126-1138 F0342 fetches object data, clears the panel, rejects scroll/container branches, and enters object-description mode; PANEL.C:1189-1200 uses G0352 object names for non-potion objects and prints the description/icon; PANEL.C:235-317 F0336 counts matching potential/actual description bits and builds a parenthesized attribute string in bit order with comma/AND joins; PANEL.C:1250-1254 gives weapons potential CURSED|POISONED|BROKEN and actual Cursed<<3 | Poisoned<<1 | Broken<<2; PANEL.C:1420-1427 draws the built attribute line when any potential bit is set; DUNGEON.C:1153-1154 maps weapon things to C023 + Weapon.Type; OBJECT.C:25-119 loads G0352 object names.";

static int append_text(char* outText, size_t outTextSize, size_t* used, const char* text) {
    int written;
    if (!outText || !used || !text || *used >= outTextSize) return 0;
    written = snprintf(outText + *used, outTextSize - *used, "%s", text);
    if (written < 0 || (size_t)written >= outTextSize - *used) return 0;
    *used += (size_t)written;
    return 1;
}

static int format_attributes(unsigned int potentialMask,
                             unsigned int actualMask,
                             char* outText,
                             size_t outTextSize) {
    unsigned int matchingCount = 0u;
    unsigned int emitted = 0u;
    size_t used = 0u;
    unsigned int i;

    if (!outText || outTextSize == 0u) return 0;
    outText[0] = '\0';

    for (i = 0u; i < 4u; ++i) {
        const unsigned int mask = 1u << i;
        if ((mask & potentialMask & actualMask) != 0u) matchingCount++;
    }
    if (matchingCount == 0u) return 1;

    if (!append_text(outText, outTextSize, &used, "(")) return 0;
    for (i = 0u; i < 4u; ++i) {
        const unsigned int mask = 1u << i;
        if ((mask & potentialMask & actualMask) != 0u) {
            if (!append_text(outText, outTextSize, &used, kAttributeNames[i])) return 0;
            emitted++;
            if (matchingCount - emitted > 1u) {
                if (!append_text(outText, outTextSize, &used, ", ")) return 0;
            } else if (matchingCount - emitted == 1u) {
                if (!append_text(outText, outTextSize, &used, " AND ")) return 0;
            }
        }
    }
    return append_text(outText, outTextSize, &used, ")");
}

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

unsigned int INVENTORY_Compat_WeaponEyePotentialAttributesMask(void) {
    return DM1_DESCRIPTION_CURSED_MASK_PC34_COMPAT |
           DM1_DESCRIPTION_POISONED_MASK_PC34_COMPAT |
           DM1_DESCRIPTION_BROKEN_MASK_PC34_COMPAT;
}

unsigned int INVENTORY_Compat_WeaponEyeActualAttributesMask(unsigned int cursed,
                                                           unsigned int poisoned,
                                                           unsigned int broken) {
    return ((cursed ? 1u : 0u) << 3) |
           ((poisoned ? 1u : 0u) << 1) |
           ((broken ? 1u : 0u) << 2);
}

int INVENTORY_Compat_FormatWeaponEyeDescription(unsigned int thingType,
                                                unsigned int cursed,
                                                unsigned int poisoned,
                                                unsigned int broken,
                                                const char* objectName,
                                                char* outNameText,
                                                size_t outNameTextSize,
                                                char* outAttributeText,
                                                size_t outAttributeTextSize,
                                                InventoryWeaponEyeDescriptionPc34Compat* outDescription) {
    int written;
    const unsigned int potentialMask = INVENTORY_Compat_WeaponEyePotentialAttributesMask();
    const unsigned int actualMask = INVENTORY_Compat_WeaponEyeActualAttributesMask(cursed, poisoned, broken);
    if (thingType != DM1_THING_TYPE_WEAPON_PC34_COMPAT || !outNameText || outNameTextSize == 0u ||
        !outAttributeText || outAttributeTextSize == 0u) {
        return 0;
    }
    if (!objectName) objectName = "";

    written = snprintf(outNameText, outNameTextSize, "%s", objectName);
    if (written < 0 || (size_t)written >= outNameTextSize) return 0;
    if (!format_attributes(potentialMask, actualMask, outAttributeText, outAttributeTextSize)) return 0;

    if (outDescription) {
        outDescription->isWeapon = 1u;
        outDescription->potentialAttributesMask = potentialMask;
        outDescription->actualAttributesMask = actualMask;
        outDescription->sourceEvidence = kWeaponIdentificationEvidence;
    }
    return 1;
}

const char* INVENTORY_Compat_GetItemIdentificationEvidence(void) {
    return kItemIdentificationEvidence;
}
