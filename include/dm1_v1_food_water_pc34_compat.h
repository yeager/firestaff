#ifndef FIRESTAFF_DM1_V1_FOOD_WATER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_FOOD_WATER_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Per-tick decay rates — ReDMCSB: ~3 food and ~3 water per game tick */
#define M11_FOOD_DECAY_PER_TICK  3
#define M11_WATER_DECAY_PER_TICK 3

#define M11_MAX_CHAMPIONS 4
#define M11_FOOD_DECAY_MS 60000  /* lose 1 food per minute in-game */
#define M11_WATER_DECAY_MS 45000 /* lose 1 water per 45s */

typedef struct {
    int food;
    int water;
    int lastEatMs;
    int lastDrinkMs;
    int starved;
    int thirsty;
} M11_FoodWater;

typedef struct {
    M11_FoodWater champions[M11_MAX_CHAMPIONS];
    int count;
} M11_FoodWaterState;

void m11_fw_init(M11_FoodWaterState* s, int count);
void m11_fw_tick(M11_FoodWaterState* s, int nowMs);
int m11_fw_eat(M11_FoodWaterState* s, int champ, int foodAmt, int nowMs);
int m11_fw_drink(M11_FoodWaterState* s, int champ, int waterAmt, int nowMs);
int m11_fw_get_food(const M11_FoodWaterState* s, int champ);
int m11_fw_get_water(const M11_FoodWaterState* s, int champ);
int m11_fw_is_starved(const M11_FoodWaterState* s, int champ);
int m11_fw_is_thirsty(const M11_FoodWaterState* s, int champ);
int m11_fw_starvation_damage(const M11_FoodWaterState* s, int champ);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_FOOD_WATER_PC34_COMPAT_H */