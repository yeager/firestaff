#include "dm1_v1_food_water_pc34_compat.h"

void m11_fw_init(M11_FoodWaterState* s, int count) {
    if (!s) return;
    if (count < 0) count = 0;
    if (count > M11_MAX_CHAMPIONS) count = M11_MAX_CHAMPIONS;
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
 * M11_FOOD_DECAY_PER_TICK and water by M11_WATER_DECAY_PER_TICK. */
void m11_fw_tick(M11_FoodWaterState* s, int gameTick) {
    (void)gameTick; /* tick count for future use */
    if (!s) return;
    for (int i = 0; i < s->count; i++) {
        M11_FoodWater* fw = &s->champions[i];

        /* Food decay per tick */
        fw->food -= M11_FOOD_DECAY_PER_TICK;
        if (fw->food < 0) fw->food = 0;

        /* Water decay per tick */
        fw->water -= M11_WATER_DECAY_PER_TICK;
        if (fw->water < 0) fw->water = 0;

        /* Update status flags */
        fw->starved = (fw->food == 0) ? 1 : 0;
        fw->thirsty = (fw->water == 0) ? 1 : 0;
    }
}

int m11_fw_eat(M11_FoodWaterState* s, int champ, int foodAmt, int nowMs) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    M11_FoodWater* fw = &s->champions[champ];
    fw->food += foodAmt;
    if (fw->food > 1000) fw->food = 1000;
    fw->lastEatMs = nowMs;
    fw->starved = 0;
    return 1;
}

int m11_fw_drink(M11_FoodWaterState* s, int champ, int waterAmt, int nowMs) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    M11_FoodWater* fw = &s->champions[champ];
    fw->water += waterAmt;
    if (fw->water > 1000) fw->water = 1000;
    fw->lastDrinkMs = nowMs;
    fw->thirsty = 0;
    return 1;
}

int m11_fw_get_food(const M11_FoodWaterState* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    return s->champions[champ].food;
}

int m11_fw_get_water(const M11_FoodWaterState* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    return s->champions[champ].water;
}

int m11_fw_is_starved(const M11_FoodWaterState* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    return s->champions[champ].starved;
}

int m11_fw_is_thirsty(const M11_FoodWaterState* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    return s->champions[champ].thirsty;
}

/* BUG-032 fix: starvation/thirst applies HP damage per tick via combat system,
 * not further food reduction. Returns damage to apply (caller passes to combat). */
int m11_fw_starvation_damage(const M11_FoodWaterState* s, int champ) {
    if (!s || champ < 0 || champ >= s->count) return 0;
    const M11_FoodWater* fw = &s->champions[champ];
    int damage = 0;
    /* ReDMCSB: starvation/thirst causes 2 HP damage per tick */
    if (fw->starved) damage += 2;
    if (fw->thirsty) damage += 2;
    return damage;
}