#ifndef FIRESTAFF_DM1_V1_INVENTORY_CONSUMABLES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_CONSUMABLES_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_CONSUMABLE_FOOD_WATER_MAX_PC34 2048
#define DM1_CONSUMABLE_POTION_EMPTY_FLASK_PC34 20

enum {
    DM1_CONSUMABLE_STAT_STRENGTH = 0,
    DM1_CONSUMABLE_STAT_DEXTERITY = 1,
    DM1_CONSUMABLE_STAT_WISDOM = 2,
    DM1_CONSUMABLE_STAT_VITALITY = 3,
    DM1_CONSUMABLE_STAT_COUNT = 4
};

typedef enum {
    DM1_CONSUMABLE_RESULT_NONE = 0,
    DM1_CONSUMABLE_RESULT_WATER_JUNK,
    DM1_CONSUMABLE_RESULT_FOOD_JUNK,
    DM1_CONSUMABLE_RESULT_POTION
} DM1ConsumableResultKindPc34;

typedef struct {
    int16_t statistic[DM1_CONSUMABLE_STAT_COUNT];
    int16_t currentHealth;
    int16_t maximumHealth;
    int16_t currentStamina;
    int16_t maximumStamina;
    int16_t currentMana;
    int16_t maximumMana;
    int16_t shieldDefense;
    int16_t food;
    int16_t water;
    uint16_t wounds;
    uint16_t poisonDose;
} DM1ConsumableChampionPc34;

typedef struct {
    DM1ConsumableResultKindPc34 kind;
    int consumed;
    int removeLeaderHandObject;
    int playSwallowSound;
    int amountApplied;
    int eventType;
    int eventDelay;
    int potionTypeAfter;
    int potionPowerAfter;
    int chargeCountAfter;
    const char* evidence;
} DM1ConsumableResultPc34;

const char* dm1_inventory_consumables_source_evidence_pc34(void);
int dm1_inventory_food_amount_from_icon_pc34(int iconIndex);
int dm1_inventory_junk_food_icon_from_type_pc34(int junkType);

int dm1_inventory_consume_water_junk_pc34(DM1ConsumableChampionPc34* champion,
                                          int iconIndex,
                                          int chargeCount,
                                          DM1ConsumableResultPc34* outResult);
int dm1_inventory_consume_food_junk_pc34(DM1ConsumableChampionPc34* champion,
                                         int iconIndex,
                                         DM1ConsumableResultPc34* outResult);
int dm1_inventory_consume_potion_pc34(DM1ConsumableChampionPc34* champion,
                                      int potionType,
                                      int potionPower,
                                      const uint16_t* woundRandomMasks,
                                      int woundRandomMaskCount,
                                      DM1ConsumableResultPc34* outResult);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_CONSUMABLES_PC34_COMPAT_H */
