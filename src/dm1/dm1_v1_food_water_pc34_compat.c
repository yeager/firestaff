#include "dm1_v1_food_water_pc34_compat.h"

void DM1_V1_FoodWater_InitPc34Compat(DM1_V1_FoodWaterStatePc34* s, int count) {
    if (!s) return;
    if (count < 0) count = 0;
    if (count > DM1_V1_FOOD_WATER_MAX_CHAMPIONS_PC34) count = DM1_V1_FOOD_WATER_MAX_CHAMPIONS_PC34;
    s->count = count;
    for (int i = 0; i < count; i++) {
        s->champions[i].food = 1000;
        s->champions[i].water = 1000;
        s->champions[i].lastEatMs = 0;
        s->champions[i].lastDrinkMs = 0;
        s->champions[i].starved = 0;
        s->champions[i].thirsty = 0;
    }
}

/* BUG-031 fix: game-tick-based decay per ReDMCSB GAMELOOP.C/CHAMPION.C.
 * Called once per game tick, not wall-clock. Decrement food by
 * DM1_V1_FOOD_DECAY_PER_TICK_PC34 and water by DM1_V1_WATER_DECAY_PER_TICK_PC34. */
void DM1_V1_FoodWater_TickPc34Compat(DM1_V1_FoodWaterStatePc34* s, int gameTick) {
    (void)gameTick; /* tick count for future use */
    if (!s) return;
    for (int i = 0; i < s->count; i++) {
        DM1_V1_FoodWaterPc34* fw = &s->champions[i];

        /* Food decay per tick */
        fw->food -= DM1_V1_FOOD_DECAY_PER_TICK_PC34;
        if (fw->food < 0) fw->food = 0;

        /* Water decay per tick */
        fw->water -= DM1_V1_WATER_DECAY_PER_TICK_PC34;
        if (fw->water < 0) fw->water = 0;

        /* Update status flags */
        fw->starved = (fw->food == 0) ? 1 : 0;
        fw->thirsty = (fw->water == 0) ? 1 : 0;
    }
}

int DM1_V1_FoodWater_EatPc34Compat(DM1_V1_FoodWaterStatePc34* s, int champ, int foodAmt, int nowMs) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    DM1_V1_FoodWaterPc34* fw = &s->champions[champ];
    fw->food += foodAmt;
    if (fw->food > 1000) fw->food = 1000;
    fw->lastEatMs = nowMs;
    fw->starved = 0;
    return 1;
}

int DM1_V1_FoodWater_DrinkPc34Compat(DM1_V1_FoodWaterStatePc34* s, int champ, int waterAmt, int nowMs) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    DM1_V1_FoodWaterPc34* fw = &s->champions[champ];
    fw->water += waterAmt;
    if (fw->water > 1000) fw->water = 1000;
    fw->lastDrinkMs = nowMs;
    fw->thirsty = 0;
    return 1;
}

int DM1_V1_FoodWater_GetFoodPc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    return s->champions[champ].food;
}

int DM1_V1_FoodWater_GetWaterPc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    return s->champions[champ].water;
}

int DM1_V1_FoodWater_IsStarvedPc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    return s->champions[champ].starved;
}

int DM1_V1_FoodWater_IsThirstyPc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    return s->champions[champ].thirsty;
}

/* BUG-032 fix: starvation/thirst applies HP damage per tick via combat system,
 * not further food reduction. Returns damage to apply (caller passes to combat). */
int DM1_V1_FoodWater_StarvationDamagePc34Compat(const DM1_V1_FoodWaterStatePc34* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    const DM1_V1_FoodWaterPc34* fw = &s->champions[champ];
    int damage = 0;
    /* ReDMCSB: starvation/thirst causes 2 HP damage per tick */
    if (fw->starved) damage += 2;
    if (fw->thirsty) damage += 2;
    return damage;
}
