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

typedef struct InventoryObjectWeightLinePc34Compat {
    unsigned int weightTenths;
    unsigned int wholeKilograms;
    unsigned int tenthsKilograms;
    const char* sourceEvidence;
} InventoryObjectWeightLinePc34Compat;

typedef struct InventoryObjectDescriptionLayoutPc34Compat {
    unsigned int headerTextZoneIndex;
    unsigned int bodyTextZoneIndex;
    unsigned int iconZoneIndex;
    unsigned int circleZoneIndex;
    unsigned int bodyStartY;
    unsigned int lineHeight;
    unsigned int wrapLimitChars;
    unsigned int textColor;
    const char* sourceEvidence;
} InventoryObjectDescriptionLayoutPc34Compat;

typedef enum InventoryObjectEyePanelRoutePc34Compat {
    INVENTORY_OBJECT_EYE_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34_COMPAT = 0,
    INVENTORY_OBJECT_EYE_PANEL_ROUTE_SCROLL_TEXT_PC34_COMPAT = 1,
    INVENTORY_OBJECT_EYE_PANEL_ROUTE_CONTAINER_CHEST_PC34_COMPAT = 2
} InventoryObjectEyePanelRoutePc34Compat;

typedef enum InventoryJunkEyeSpecialStatePc34Compat {
    INVENTORY_JUNK_EYE_SPECIAL_NONE_PC34_COMPAT = 0,
    INVENTORY_JUNK_EYE_SPECIAL_COMPASS_PC34_COMPAT = 1,
    INVENTORY_JUNK_EYE_SPECIAL_WATERSKIN_PC34_COMPAT = 2
} InventoryJunkEyeSpecialStatePc34Compat;

typedef struct InventoryObjectEyePanelRouteDescriptionPc34Compat {
    InventoryObjectEyePanelRoutePc34Compat route;
    const char* sourceEvidence;
} InventoryObjectEyePanelRouteDescriptionPc34Compat;

typedef struct InventoryArmourEyeDescriptionPc34Compat {
    unsigned int isArmour;
    unsigned int potentialAttributesMask;
    unsigned int actualAttributesMask;
    const char* sourceEvidence;
} InventoryArmourEyeDescriptionPc34Compat;

typedef struct InventoryJunkEyeDescriptionPc34Compat {
    unsigned int isJunk;
    InventoryJunkEyeSpecialStatePc34Compat specialState;
    unsigned int potentialAttributesMask;
    unsigned int actualAttributesMask;
    const char* sourceEvidence;
} InventoryJunkEyeDescriptionPc34Compat;

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
InventoryObjectEyePanelRoutePc34Compat INVENTORY_Compat_ObjectEyePanelRoute(unsigned int thingType,
                                                                               InventoryObjectEyePanelRouteDescriptionPc34Compat* outDescription);
unsigned int INVENTORY_Compat_ArmourEyePotentialAttributesMask(void);
unsigned int INVENTORY_Compat_ArmourEyeActualAttributesMask(unsigned int cursed,
                                                           unsigned int broken);
int INVENTORY_Compat_FormatArmourEyeDescription(unsigned int thingType,
                                                unsigned int cursed,
                                                unsigned int broken,
                                                const char* objectName,
                                                char* outNameText,
                                                size_t outNameTextSize,
                                                char* outAttributeText,
                                                size_t outAttributeTextSize,
                                                InventoryArmourEyeDescriptionPc34Compat* outDescription);
unsigned int INVENTORY_Compat_JunkEyePotentialAttributesMask(unsigned int iconIndex);
unsigned int INVENTORY_Compat_JunkEyeActualAttributesMask(unsigned int iconIndex,
                                                         unsigned int allowedSlots);
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
                                              InventoryJunkEyeDescriptionPc34Compat* outDescription);
int INVENTORY_Compat_FormatObjectWeightLine(unsigned int weightTenths,
                                            char* outText,
                                            size_t outTextSize,
                                            InventoryObjectWeightLinePc34Compat* outDescription);
int INVENTORY_Compat_GetObjectDescriptionLayout(InventoryObjectDescriptionLayoutPc34Compat* outLayout);
size_t INVENTORY_Compat_WrapObjectDescriptionText(const char* text,
                                                  char* outLines,
                                                  size_t outLineCount,
                                                  size_t outLineStride,
                                                  InventoryObjectDescriptionLayoutPc34Compat* outLayout);
const char* INVENTORY_Compat_GetItemIdentificationEvidence(void);

#endif
