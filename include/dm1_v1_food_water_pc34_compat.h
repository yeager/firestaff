#ifndef FIRESTAFF_DM1_V1_FOOD_WATER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_FOOD_WATER_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Per-tick decay rates — ReDMCSB: ~3 food and ~3 water per game tick */
#define DM1_V1_FOOD_DECAY_PER_TICK_PC34  3
#define DM1_V1_WATER_DECAY_PER_TICK_PC34 3

#define DM1_V1_FOOD_WATER_MAX_CHAMPIONS_PC34 4
#define DM1_V1_FOOD_DECAY_MS_PC34 60000  /* lose 1 food per minute in-game */
#define DM1_V1_WATER_DECAY_MS_PC34 45000 /* lose 1 water per 45s */

typedef struct {
    int food;
    int water;
    int lastEatMs;
    int lastDrinkMs;
    int starved;
    int thirsty;
} DM1_V1_FoodWaterPc34;

typedef struct {
    DM1_V1_FoodWaterPc34 champions[DM1_V1_FOOD_WATER_MAX_CHAMPIONS_PC34];
    int count;
} DM1_V1_FoodWaterStatePc34;

void DM1_V1_FoodWater_InitPc34Compat(DM1_V1_FoodWaterStatePc34* s, int count);
void DM1_V1_FoodWater_TickPc34Compat(DM1_V1_FoodWaterStatePc34* s, int nowMs);
int DM1_V1_FoodWater_EatPc34Compat(DM1_V1_FoodWaterStatePc34* s, int champ, int foodAmt, int nowMs);
int DM1_V1_FoodWater_DrinkPc34Compat(DM1_V1_FoodWaterStatePc34* s, int champ, int waterAmt, int nowMs);
int DM1_V1_FoodWater_GetFoodPc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ);
int DM1_V1_FoodWater_GetWaterPc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ);
int DM1_V1_FoodWater_IsStarvedPc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ);
int DM1_V1_FoodWater_IsThirstyPc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ);
int DM1_V1_FoodWater_StarvationDamagePc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ);

typedef DM1_V1_FoodWaterPc34 M11_FoodWater;
typedef DM1_V1_FoodWaterStatePc34 M11_FoodWaterState;

#define M11_FOOD_DECAY_PER_TICK DM1_V1_FOOD_DECAY_PER_TICK_PC34
#define M11_WATER_DECAY_PER_TICK DM1_V1_WATER_DECAY_PER_TICK_PC34
#define M11_FOOD_DECAY_MS DM1_V1_FOOD_DECAY_MS_PC34
#define M11_WATER_DECAY_MS DM1_V1_WATER_DECAY_MS_PC34

#define m11_fw_init DM1_V1_FoodWater_InitPc34Compat
#define m11_fw_tick DM1_V1_FoodWater_TickPc34Compat
#define m11_fw_eat DM1_V1_FoodWater_EatPc34Compat
#define m11_fw_drink DM1_V1_FoodWater_DrinkPc34Compat
#define m11_fw_get_food DM1_V1_FoodWater_GetFoodPc34Compat
#define m11_fw_get_water DM1_V1_FoodWater_GetWaterPc34Compat
#define m11_fw_is_starved DM1_V1_FoodWater_IsStarvedPc34Compat
#define m11_fw_is_thirsty DM1_V1_FoodWater_IsThirstyPc34Compat
#define m11_fw_starvation_damage DM1_V1_FoodWater_StarvationDamagePc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_FOOD_WATER_PC34_COMPAT_H */
