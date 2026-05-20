#include <stdio.h>
#include <string.h>

#include "inventory_item_identification_pc34_compat.h"

#define DM1_THING_TYPE_WEAPON 5u
#define DM1_THING_TYPE_ARMOUR 6u
#define DM1_THING_TYPE_SCROLL 7u
#define DM1_THING_TYPE_POTION 8u
#define DM1_THING_TYPE_CONTAINER 9u
#define DM1_THING_TYPE_JUNK 10u
#define DM1_ICON_POTION_ROS 154u
#define DM1_ICON_POTION_WATER_FLASK 163u
#define DM1_ICON_POTION_EMPTY_FLASK 195u
#define DM1_ICON_JUNK_COMPASS_EAST 1u
#define DM1_ICON_JUNK_WATERSKIN 9u
#define DM1_ICON_JUNK_APPLE 156u
#define DM1_ALLOWED_SLOT_MOUTH 0x0001u
#define DM1_ALLOWED_SLOT_BACKPACK 0x0500u

static int expect_description(unsigned int thingType,
                              unsigned int iconIndex,
                              unsigned int potionPower,
                              unsigned int priestSkillLevel,
                              const char* objectName,
                              const char* expectedText,
                              unsigned int expectedPrefix,
                              char expectedSymbol) {
    char text[64];
    InventoryPotionEyeDescriptionPc34Compat description;
    if (!INVENTORY_Compat_FormatPotionEyeDescription(thingType, iconIndex, potionPower,
                                                     priestSkillLevel, objectName, text,
                                                     sizeof(text), &description)) {
        return 0;
    }
    if (strcmp(text, expectedText) != 0) return 0;
    if (description.usesPowerPrefix != expectedPrefix) return 0;
    if (description.powerSymbol != expectedSymbol) return 0;
    if (!description.sourceEvidence || strstr(description.sourceEvidence, "PANEL.C:1182-1191") == NULL) return 0;
    return 1;
}

static int expect_weapon_description(unsigned int cursed,
                                     unsigned int poisoned,
                                     unsigned int broken,
                                     const char* objectName,
                                     const char* expectedName,
                                     const char* expectedAttributes,
                                     unsigned int expectedActualMask) {
    char name[64];
    char attributes[64];
    InventoryWeaponEyeDescriptionPc34Compat description;
    if (!INVENTORY_Compat_FormatWeaponEyeDescription(DM1_THING_TYPE_WEAPON, cursed, poisoned,
                                                     broken, objectName, name, sizeof(name),
                                                     attributes, sizeof(attributes), &description)) {
        return 0;
    }
    if (strcmp(name, expectedName) != 0) return 0;
    if (strcmp(attributes, expectedAttributes) != 0) return 0;
    if (description.isWeapon != 1u) return 0;
    if (description.potentialAttributesMask != 0x000Eu) return 0;
    if (description.actualAttributesMask != expectedActualMask) return 0;
    if (!description.sourceEvidence || strstr(description.sourceEvidence, "PANEL.C:1250-1254") == NULL) return 0;
    if (strstr(description.sourceEvidence, "PANEL.C:235-317") == NULL) return 0;
    return 1;
}


static int expect_route(unsigned int thingType,
                        InventoryObjectEyePanelRoutePc34Compat expectedRoute) {
    InventoryObjectEyePanelRouteDescriptionPc34Compat description;
    InventoryObjectEyePanelRoutePc34Compat route =
        INVENTORY_Compat_ObjectEyePanelRoute(thingType, &description);
    if (route != expectedRoute) return 0;
    if (description.route != expectedRoute) return 0;
    if (!description.sourceEvidence || strstr(description.sourceEvidence, "PANEL.C:1126-1134") == NULL) return 0;
    return 1;
}

static int expect_armour_description(unsigned int cursed,
                                     unsigned int broken,
                                     const char* objectName,
                                     const char* expectedName,
                                     const char* expectedAttributes,
                                     unsigned int expectedActualMask) {
    char name[64];
    char attributes[64];
    InventoryArmourEyeDescriptionPc34Compat description;
    if (!INVENTORY_Compat_FormatArmourEyeDescription(DM1_THING_TYPE_ARMOUR, cursed, broken,
                                                     objectName, name, sizeof(name),
                                                     attributes, sizeof(attributes), &description)) {
        return 0;
    }
    if (strcmp(name, expectedName) != 0) return 0;
    if (strcmp(attributes, expectedAttributes) != 0) return 0;
    if (description.isArmour != 1u) return 0;
    if (description.potentialAttributesMask != 0x000Cu) return 0;
    if (description.actualAttributesMask != expectedActualMask) return 0;
    if (!description.sourceEvidence || strstr(description.sourceEvidence, "PANEL.C:1272-1274") == NULL) return 0;
    if (strstr(description.sourceEvidence, "DEFS.H:1415-1433") == NULL) return 0;
    return 1;
}

static int expect_junk_description(unsigned int iconIndex,
                                   unsigned int chargeCount,
                                   unsigned int allowedSlots,
                                   const char* objectName,
                                   const char* expectedName,
                                   const char* expectedState,
                                   const char* expectedAttributes,
                                   InventoryJunkEyeSpecialStatePc34Compat expectedSpecial,
                                   unsigned int expectedPotentialMask,
                                   unsigned int expectedActualMask) {
    char name[64];
    char state[64];
    char attributes[64];
    InventoryJunkEyeDescriptionPc34Compat description;
    if (!INVENTORY_Compat_FormatJunkEyeDescription(DM1_THING_TYPE_JUNK, iconIndex, chargeCount,
                                                   allowedSlots, objectName, name, sizeof(name),
                                                   state, sizeof(state), attributes, sizeof(attributes),
                                                   &description)) {
        return 0;
    }
    if (strcmp(name, expectedName) != 0) return 0;
    if (strcmp(state, expectedState) != 0) return 0;
    if (strcmp(attributes, expectedAttributes) != 0) return 0;
    if (description.isJunk != 1u) return 0;
    if (description.specialState != expectedSpecial) return 0;
    if (description.potentialAttributesMask != expectedPotentialMask) return 0;
    if (description.actualAttributesMask != expectedActualMask) return 0;
    if (!description.sourceEvidence || strstr(description.sourceEvidence, "PANEL.C:1295-1320") == NULL) return 0;
    if (strstr(description.sourceEvidence, "PANEL.C:1322-1390") == NULL) return 0;
    if (strstr(description.sourceEvidence, "PANEL.C:1412-1413") == NULL) return 0;
    return 1;
}

static int expect_weight_line(unsigned int weightTenths,
                              const char* expectedText,
                              unsigned int expectedWhole,
                              unsigned int expectedTenths) {
    char text[32];
    InventoryObjectWeightLinePc34Compat description;
    if (!INVENTORY_Compat_FormatObjectWeightLine(weightTenths, text, sizeof(text), &description)) {
        return 0;
    }
    if (strcmp(text, expectedText) != 0) return 0;
    if (description.weightTenths != weightTenths) return 0;
    if (description.wholeKilograms != expectedWhole) return 0;
    if (description.tenthsKilograms != expectedTenths) return 0;
    if (!description.sourceEvidence || strstr(description.sourceEvidence, "PANEL.C:1444-1469") == NULL) return 0;
    if (strstr(description.sourceEvidence, "DUNGEON.C:1082-1133") == NULL) return 0;
    if (strstr(description.sourceEvidence, "CHAMDRAW.C:349-392") == NULL) return 0;
    return 1;
}

int main(void) {
    int ok = 1;
    char tiny[4];
    char name[64];
    char attributes[64];
    printf("probe=firestaff_inventory_item_identification_source_lock\n");
    printf("sourceEvidence=%s\n", INVENTORY_Compat_GetItemIdentificationEvidence());

    ok &= expect_description(DM1_THING_TYPE_POTION, DM1_ICON_POTION_ROS, 80u, 2u,
                             "ROS POTION", "a ROS POTION", 1u, 'a');
    ok &= expect_description(DM1_THING_TYPE_POTION, DM1_ICON_POTION_ROS, 80u, 1u,
                             "ROS POTION", "ROS POTION", 0u, '\0');
    ok &= expect_description(DM1_THING_TYPE_POTION, DM1_ICON_POTION_WATER_FLASK, 120u, 4u,
                             "WATER FLASK", "WATER FLASK", 0u, '\0');
    ok &= expect_description(DM1_THING_TYPE_POTION, DM1_ICON_POTION_EMPTY_FLASK, 0u, 4u,
                             "EMPTY FLASK", "_ EMPTY FLASK", 1u, '_');
    ok &= expect_description(DM1_THING_TYPE_JUNK, 168u, 80u, 4u,
                             "APPLE", "APPLE", 0u, '\0');

    if (INVENTORY_Compat_PotionPowerSymbol(40u) != '`') ok = 0;
    if (INVENTORY_Compat_PotionEyeShowsPowerPrefix(DM1_THING_TYPE_POTION, DM1_ICON_POTION_ROS, 2u) != 1u) ok = 0;
    if (INVENTORY_Compat_PotionEyeShowsPowerPrefix(DM1_THING_TYPE_POTION, DM1_ICON_POTION_WATER_FLASK, 2u) != 0u) ok = 0;
    if (INVENTORY_Compat_FormatPotionEyeDescription(DM1_THING_TYPE_POTION, DM1_ICON_POTION_ROS,
                                                    80u, 2u, "ROS POTION", tiny,
                                                    sizeof(tiny), NULL) != 0) ok = 0;

    ok &= expect_weapon_description(0u, 1u, 1u, "POISON DART",
                                    "POISON DART", "(POISONED AND BROKEN)", 0x0006u);
    ok &= expect_weapon_description(1u, 1u, 1u, "DAGGER",
                                    "DAGGER", "(POISONED, BROKEN AND CURSED)", 0x000Eu);
    ok &= expect_weapon_description(0u, 0u, 0u, "SWORD", "SWORD", "", 0x0000u);
    if (INVENTORY_Compat_WeaponEyePotentialAttributesMask() != 0x000Eu) ok = 0;
    if (INVENTORY_Compat_WeaponEyeActualAttributesMask(1u, 0u, 1u) != 0x000Cu) ok = 0;
    if (INVENTORY_Compat_FormatWeaponEyeDescription(DM1_THING_TYPE_JUNK, 1u, 1u, 1u,
                                                    "APPLE", name, sizeof(name), attributes,
                                                    sizeof(attributes), NULL) != 0) ok = 0;


    ok &= expect_route(DM1_THING_TYPE_SCROLL,
                       INVENTORY_OBJECT_EYE_PANEL_ROUTE_SCROLL_TEXT_PC34_COMPAT);
    ok &= expect_route(DM1_THING_TYPE_CONTAINER,
                       INVENTORY_OBJECT_EYE_PANEL_ROUTE_CONTAINER_CHEST_PC34_COMPAT);
    ok &= expect_route(DM1_THING_TYPE_JUNK,
                       INVENTORY_OBJECT_EYE_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34_COMPAT);

    ok &= expect_armour_description(1u, 1u, "PLATE MAIL", "PLATE MAIL",
                                    "(BROKEN AND CURSED)", 0x000Cu);
    ok &= expect_armour_description(1u, 0u, "CROWN OF NERRA", "CROWN OF NERRA",
                                    "(CURSED)", 0x0008u);
    ok &= expect_armour_description(0u, 0u, "SANDALS", "SANDALS", "", 0x0000u);
    if (INVENTORY_Compat_ArmourEyePotentialAttributesMask() != 0x000Cu) ok = 0;
    if (INVENTORY_Compat_ArmourEyeActualAttributesMask(1u, 0u) != 0x0008u) ok = 0;
    if (INVENTORY_Compat_FormatArmourEyeDescription(DM1_THING_TYPE_WEAPON, 1u, 1u,
                                                    "DAGGER", name, sizeof(name), attributes,
                                                    sizeof(attributes), NULL) != 0) ok = 0;

    ok &= expect_junk_description(DM1_ICON_JUNK_COMPASS_EAST, 0u, 0u,
                                  "COMPASS", "COMPASS", "PARTY FACING EAST", "",
                                  INVENTORY_JUNK_EYE_SPECIAL_COMPASS_PC34_COMPAT, 0x0000u, 0x0000u);
    ok &= expect_junk_description(DM1_ICON_JUNK_WATERSKIN, 0u, 0u,
                                  "WATERSKIN", "WATERSKIN", "(EMPTY)", "",
                                  INVENTORY_JUNK_EYE_SPECIAL_WATERSKIN_PC34_COMPAT, 0x0000u, 0x0000u);
    ok &= expect_junk_description(DM1_ICON_JUNK_WATERSKIN, 2u, 0u,
                                  "WATERSKIN", "WATERSKIN", "(ALMOST FULL)", "",
                                  INVENTORY_JUNK_EYE_SPECIAL_WATERSKIN_PC34_COMPAT, 0x0000u, 0x0000u);
    ok &= expect_junk_description(DM1_ICON_JUNK_APPLE, 0u, DM1_ALLOWED_SLOT_BACKPACK | DM1_ALLOWED_SLOT_MOUTH,
                                  "APPLE", "APPLE", "", "(CONSUMABLE)",
                                  INVENTORY_JUNK_EYE_SPECIAL_NONE_PC34_COMPAT, 0x0001u, DM1_ALLOWED_SLOT_BACKPACK | DM1_ALLOWED_SLOT_MOUTH);
    ok &= expect_junk_description(DM1_ICON_JUNK_APPLE, 0u, DM1_ALLOWED_SLOT_BACKPACK,
                                  "ROPE", "ROPE", "", "",
                                  INVENTORY_JUNK_EYE_SPECIAL_NONE_PC34_COMPAT, 0x0001u, DM1_ALLOWED_SLOT_BACKPACK);
    if (INVENTORY_Compat_JunkEyePotentialAttributesMask(DM1_ICON_JUNK_WATERSKIN) != 0x0000u) ok = 0;
    if (INVENTORY_Compat_JunkEyePotentialAttributesMask(DM1_ICON_JUNK_APPLE) != 0x0001u) ok = 0;
    if (INVENTORY_Compat_FormatJunkEyeDescription(DM1_THING_TYPE_ARMOUR, DM1_ICON_JUNK_APPLE, 0u,
                                                  DM1_ALLOWED_SLOT_MOUTH, "APPLE", name, sizeof(name),
                                                  attributes, sizeof(attributes), attributes,
                                                  sizeof(attributes), NULL) != 0) ok = 0;

    ok &= expect_weight_line(0u, "WEIGHS 0.0 KG.", 0u, 0u);
    ok &= expect_weight_line(1u, "WEIGHS 0.1 KG.", 0u, 1u);
    ok &= expect_weight_line(50u, "WEIGHS 5.0 KG.", 5u, 0u);
    ok &= expect_weight_line(123u, "WEIGHS 12.3 KG.", 12u, 3u);
    if (INVENTORY_Compat_FormatObjectWeightLine(123u, tiny, sizeof(tiny), NULL) != 0) ok = 0;

    printf("inventoryItemIdentificationInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
