#include "inventory_item_identification_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define DM1_THING_TYPE_POTION_PC34_COMPAT 8u
#define DM1_THING_TYPE_CONTAINER_PC34_COMPAT 9u
#define DM1_THING_TYPE_JUNK_PC34_COMPAT 10u
#define DM1_THING_TYPE_WEAPON_PC34_COMPAT 5u
#define DM1_THING_TYPE_ARMOUR_PC34_COMPAT 6u
#define DM1_THING_TYPE_SCROLL_PC34_COMPAT 7u
#define DM1_ICON_POTION_WATER_FLASK_PC34_COMPAT 163u
#define DM1_ICON_JUNK_COMPASS_NORTH_PC34_COMPAT 0u
#define DM1_ICON_JUNK_COMPASS_WEST_PC34_COMPAT 3u
#define DM1_ICON_JUNK_WATER_PC34_COMPAT 8u
#define DM1_ICON_JUNK_WATERSKIN_PC34_COMPAT 9u
#define DM1_ALLOWED_SLOT_MOUTH_PC34_COMPAT 0x0001u
#define DM1_DESCRIPTION_CONSUMABLE_MASK_PC34_COMPAT 0x0001u
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
static const char* kObjectWeightLineEvidence =
    "ReDMCSB source lock: PANEL.C:1444-1469 appends the eye-panel weight line after type-specific attribute/state lines as WEIGHS + F0140_DUNGEON_GetObjectWeight/10 + decimal separator + remainder + ' KG.'; DUNGEON.C:1082-1133 F0140 returns object weights in tenths of kilograms for weapons, armour, junk, containers including contents, potions, and scrolls; CHAMDRAW.C:349-392 F0288 emits unpadded decimal digits when padding is false.";
static const char* kObjectRouteEvidence =
    "ReDMCSB source lock: PANEL.C:1126-1134 F0342 clears the panel, routes C07 scroll things to F0341 scroll text, routes C09 containers to F0333 chest open/draw, and only uses the object-description branch for the remaining leader-hand object families.";
static const char* kArmourIdentificationEvidence =
    "ReDMCSB source lock: PANEL.C:1126-1200 F0342 fetches object data/name and draws the object-description header; PANEL.C:1215-1242 defines English attribute text order; PANEL.C:1272-1274 gives armour potential CURSED|BROKEN and actual Cursed<<3 | Broken<<2; PANEL.C:1420-1427 emits the built attribute line; PANEL.C:235-317 F0336 formats matching bits with comma/AND joins; DEFS.H:1415-1433 defines ARMOUR Cursed/Broken fields and DEFS.H:2922-2926 defines description masks.";
static const char* kJunkIdentificationEvidence =
    "ReDMCSB source lock: PANEL.C:1295-1320 handles compass icons C000..C003 as PARTY FACING + direction with no attributes; PANEL.C:1322-1390 handles water/waterskin icons C008..C009 charge states EMPTY/ALMOST EMPTY/ALMOST FULL/FULL with no attributes; PANEL.C:1412-1413 gives other junk potential CONSUMABLE and actual ObjectInfo AllowedSlots; PANEL.C:1420-1427 emits the built attribute line; DUNGEON.C:1147-1158 maps junk/container/scroll object info; DEFS.H:1499-1515 defines JUNK ChargeCount/Cursed and DEFS.H:2922-2926 defines description masks.";
static const char* kObjectDescriptionLayoutEvidence =
    "ReDMCSB source lock: PANEL.C:172-228 F0335 resets object-description text with form-feed, uses C556_ZONE_OBJECT_DESCRIPTION as the PC34 body text origin, wraps body text by splitting strings longer than 18 characters at the previous space, prints in C13 lightest gray, and advances by G2088_C7_TextLineHeight; PANEL.C:1198-1208 prints the object name in C506_ZONE_OBJECT_DESCRIPTION, draws the icon in C505_ZONE_OBJECT_DESCRIPTION_ICON, then starts body lines at Y=87 when an extra name-line payload exists; PANEL.C:1244-1248 starts normal attribute/state/weight body lines at Y=87 for PC34; PANEL.C:1420-1469 emits attribute and weight body lines; DEFS.H:3873-3875 defines C504/C505/C506 zones, DEFS.H:3925 defines C556, and COORD.C:1758 fixes G2088_C7_TextLineHeight at 7.";

static const char* const kDirectionNames[] = { "NORTH", "EAST", "SOUTH", "WEST" };
static const char* const kWaterskinStateNames[] = { "(EMPTY)", "(ALMOST EMPTY)", "(ALMOST FULL)", "(FULL)" };

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

InventoryObjectEyePanelRoutePc34Compat INVENTORY_Compat_ObjectEyePanelRoute(unsigned int thingType,
                                                                               InventoryObjectEyePanelRouteDescriptionPc34Compat* outDescription) {
    InventoryObjectEyePanelRoutePc34Compat route = INVENTORY_OBJECT_EYE_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34_COMPAT;
    if (thingType == DM1_THING_TYPE_SCROLL_PC34_COMPAT) {
        route = INVENTORY_OBJECT_EYE_PANEL_ROUTE_SCROLL_TEXT_PC34_COMPAT;
    } else if (thingType == DM1_THING_TYPE_CONTAINER_PC34_COMPAT) {
        route = INVENTORY_OBJECT_EYE_PANEL_ROUTE_CONTAINER_CHEST_PC34_COMPAT;
    }
    if (outDescription) {
        outDescription->route = route;
        outDescription->sourceEvidence = kObjectRouteEvidence;
    }
    return route;
}

unsigned int INVENTORY_Compat_ArmourEyePotentialAttributesMask(void) {
    return DM1_DESCRIPTION_CURSED_MASK_PC34_COMPAT |
           DM1_DESCRIPTION_BROKEN_MASK_PC34_COMPAT;
}

unsigned int INVENTORY_Compat_ArmourEyeActualAttributesMask(unsigned int cursed,
                                                           unsigned int broken) {
    return ((cursed ? 1u : 0u) << 3) |
           ((broken ? 1u : 0u) << 2);
}

int INVENTORY_Compat_FormatArmourEyeDescription(unsigned int thingType,
                                                unsigned int cursed,
                                                unsigned int broken,
                                                const char* objectName,
                                                char* outNameText,
                                                size_t outNameTextSize,
                                                char* outAttributeText,
                                                size_t outAttributeTextSize,
                                                InventoryArmourEyeDescriptionPc34Compat* outDescription) {
    int written;
    const unsigned int potentialMask = INVENTORY_Compat_ArmourEyePotentialAttributesMask();
    const unsigned int actualMask = INVENTORY_Compat_ArmourEyeActualAttributesMask(cursed, broken);
    if (thingType != DM1_THING_TYPE_ARMOUR_PC34_COMPAT || !outNameText || outNameTextSize == 0u ||
        !outAttributeText || outAttributeTextSize == 0u) {
        return 0;
    }
    if (!objectName) objectName = "";

    written = snprintf(outNameText, outNameTextSize, "%s", objectName);
    if (written < 0 || (size_t)written >= outNameTextSize) return 0;
    if (!format_attributes(potentialMask, actualMask, outAttributeText, outAttributeTextSize)) return 0;

    if (outDescription) {
        outDescription->isArmour = 1u;
        outDescription->potentialAttributesMask = potentialMask;
        outDescription->actualAttributesMask = actualMask;
        outDescription->sourceEvidence = kArmourIdentificationEvidence;
    }
    return 1;
}

unsigned int INVENTORY_Compat_JunkEyePotentialAttributesMask(unsigned int iconIndex) {
    if ((iconIndex >= DM1_ICON_JUNK_COMPASS_NORTH_PC34_COMPAT &&
         iconIndex <= DM1_ICON_JUNK_COMPASS_WEST_PC34_COMPAT) ||
        (iconIndex >= DM1_ICON_JUNK_WATER_PC34_COMPAT &&
         iconIndex <= DM1_ICON_JUNK_WATERSKIN_PC34_COMPAT)) {
        return 0u;
    }
    return DM1_DESCRIPTION_CONSUMABLE_MASK_PC34_COMPAT;
}

unsigned int INVENTORY_Compat_JunkEyeActualAttributesMask(unsigned int iconIndex,
                                                         unsigned int allowedSlots) {
    if (INVENTORY_Compat_JunkEyePotentialAttributesMask(iconIndex) == 0u) {
        return 0u;
    }
    return allowedSlots;
}

int INVENTORY_Compat_FormatJunkEyeDescription(unsigned int thingType,
                                              unsigned int iconIndex,
                                              unsigned int chargeCount,
                                              unsigned int allowedSlots,
                                              const char* objectName,
                                              char* outNameText,
                                              size_t outNameTextSize,
                                              char* outStateText,
                                              size_t outStateTextSize,
                                              char* outAttributeText,
                                              size_t outAttributeTextSize,
                                              InventoryJunkEyeDescriptionPc34Compat* outDescription) {
    int written;
    InventoryJunkEyeSpecialStatePc34Compat special = INVENTORY_JUNK_EYE_SPECIAL_NONE_PC34_COMPAT;
    const unsigned int potentialMask = INVENTORY_Compat_JunkEyePotentialAttributesMask(iconIndex);
    const unsigned int actualMask = INVENTORY_Compat_JunkEyeActualAttributesMask(iconIndex, allowedSlots);
    if (thingType != DM1_THING_TYPE_JUNK_PC34_COMPAT || !outNameText || outNameTextSize == 0u ||
        !outStateText || outStateTextSize == 0u || !outAttributeText || outAttributeTextSize == 0u) {
        return 0;
    }
    if (!objectName) objectName = "";

    written = snprintf(outNameText, outNameTextSize, "%s", objectName);
    if (written < 0 || (size_t)written >= outNameTextSize) return 0;
    outStateText[0] = '\0';
    outAttributeText[0] = '\0';

    if (iconIndex >= DM1_ICON_JUNK_COMPASS_NORTH_PC34_COMPAT &&
        iconIndex <= DM1_ICON_JUNK_COMPASS_WEST_PC34_COMPAT) {
        special = INVENTORY_JUNK_EYE_SPECIAL_COMPASS_PC34_COMPAT;
        written = snprintf(outStateText, outStateTextSize, "PARTY FACING %s", kDirectionNames[iconIndex]);
        if (written < 0 || (size_t)written >= outStateTextSize) return 0;
    } else if (iconIndex >= DM1_ICON_JUNK_WATER_PC34_COMPAT &&
               iconIndex <= DM1_ICON_JUNK_WATERSKIN_PC34_COMPAT) {
        special = INVENTORY_JUNK_EYE_SPECIAL_WATERSKIN_PC34_COMPAT;
        if (chargeCount > 3u) chargeCount = 3u;
        written = snprintf(outStateText, outStateTextSize, "%s", kWaterskinStateNames[chargeCount]);
        if (written < 0 || (size_t)written >= outStateTextSize) return 0;
    } else if (!format_attributes(potentialMask, actualMask, outAttributeText, outAttributeTextSize)) {
        return 0;
    }

    if (outDescription) {
        outDescription->isJunk = 1u;
        outDescription->specialState = special;
        outDescription->potentialAttributesMask = potentialMask;
        outDescription->actualAttributesMask = actualMask;
        outDescription->sourceEvidence = kJunkIdentificationEvidence;
    }
    return 1;
}

static void fill_object_description_layout(InventoryObjectDescriptionLayoutPc34Compat* outLayout) {
    if (!outLayout) return;
    outLayout->headerTextZoneIndex = 506u;
    outLayout->bodyTextZoneIndex = 556u;
    outLayout->iconZoneIndex = 505u;
    outLayout->circleZoneIndex = 504u;
    outLayout->bodyStartY = 87u;
    outLayout->lineHeight = 7u;
    outLayout->wrapLimitChars = 18u;
    outLayout->textColor = 13u;
    outLayout->sourceEvidence = kObjectDescriptionLayoutEvidence;
}

int INVENTORY_Compat_FormatObjectWeightLine(unsigned int weightTenths,
                                            char* outText,
                                            size_t outTextSize,
                                            InventoryObjectWeightLinePc34Compat* outDescription) {
    int written;
    const unsigned int whole = weightTenths / 10u;
    const unsigned int tenths = weightTenths - (whole * 10u);
    if (!outText || outTextSize == 0u) {
        return 0;
    }

    written = snprintf(outText, outTextSize, "WEIGHS %u.%u KG.", whole, tenths);
    if (written < 0 || (size_t)written >= outTextSize) {
        return 0;
    }

    if (outDescription) {
        outDescription->weightTenths = weightTenths;
        outDescription->wholeKilograms = whole;
        outDescription->tenthsKilograms = tenths;
        outDescription->sourceEvidence = kObjectWeightLineEvidence;
    }
    return 1;
}

int INVENTORY_Compat_GetObjectDescriptionLayout(InventoryObjectDescriptionLayoutPc34Compat* outLayout) {
    if (!outLayout) return 0;
    fill_object_description_layout(outLayout);
    return 1;
}

size_t INVENTORY_Compat_WrapObjectDescriptionText(const char* text,
                                                  char* outLines,
                                                  size_t outLineCount,
                                                  size_t outLineStride,
                                                  InventoryObjectDescriptionLayoutPc34Compat* outLayout) {
    const char* cursor = text ? text : "";
    size_t count = 0u;
    fill_object_description_layout(outLayout);
    if (!outLines || outLineCount == 0u || outLineStride == 0u) return 0u;
    while (*cursor) {
        const size_t remaining = strlen(cursor);
        size_t split = remaining;
        if (remaining > 18u) {
            split = 17u;
            while (split > 0u && cursor[split] != ' ') split--;
            if (split == 0u) return 0u;
        }
        if (count >= outLineCount || split >= outLineStride) return 0u;
        memcpy(outLines + (count * outLineStride), cursor, split);
        outLines[(count * outLineStride) + split] = '\0';
        count++;
        if (remaining <= 18u) break;
        cursor += split;
        if (*cursor == ' ') cursor++;
    }
    return count;
}

const char* INVENTORY_Compat_GetItemIdentificationEvidence(void) {
    return kItemIdentificationEvidence;
}
