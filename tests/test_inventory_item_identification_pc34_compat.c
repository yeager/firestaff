#include <stdio.h>
#include <string.h>

#include "inventory_item_identification_pc34_compat.h"

#define DM1_THING_TYPE_WEAPON 5u
#define DM1_THING_TYPE_POTION 8u
#define DM1_THING_TYPE_JUNK 10u
#define DM1_ICON_POTION_ROS 154u
#define DM1_ICON_POTION_WATER_FLASK 163u
#define DM1_ICON_POTION_EMPTY_FLASK 195u

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

    printf("inventoryItemIdentificationInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
