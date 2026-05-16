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

void m11_fw_tick(M11_FoodWaterState* s, int nowMs) {
    if (!s) return;
    for (int i = 0; i < s->count; i++) {
        M11_FoodWater* fw = &s->champions[i];
        
        // Food decay
        if (nowMs >= fw->lastEatMs) {
            int elapsed = nowMs - fw->lastEatMs;
            int decay = elapsed / M11_FOOD_DECAY_MS;
            if (decay > 0) {
                fw->food -= decay;
                if (fw->food < 0) fw->food = 0;
                fw->lastEatMs += decay * M11_FOOD_DECAY_MS;
            }
        }
        
        // Water decay
        if (nowMs >= fw->lastDrinkMs) {
            int elapsed = nowMs - fw->lastDrinkMs;
            int decay = elapsed / M11_WATER_DECAY_MS;
            if (decay > 0) {
                fw->water -= decay;
                if (fw->water < 0) fw->water = 0;
                fw->lastDrinkMs += decay * M11_WATER_DECAY_MS;
            }
        }
        
        // Update status flags
        if (fw->food == 0) {
            fw->starved = 1;
        }
        if (fw->water == 0) {
            fw->thirsty = 1;
        }
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

void m11_fw_starve(M11_FoodWaterState* s, int champ, int nowMs) {
    if (!s || champ < 0 || champ >= s->count) return;
    M11_FoodWater* fw = &s->champions[champ];
    
    if (fw->starved || fw->thirsty) {
        int reduction = nowMs / 1000;
        
        if (fw->starved) {
            fw->food -= reduction;
            if (fw->food < 0) fw->food = 0;
        }
        
        if (fw->thirsty) {
            fw->water -= reduction;
            if (fw->water < 0) fw->water = 0;
        }
    }
}