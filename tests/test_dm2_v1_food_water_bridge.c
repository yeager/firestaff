#include "dm2_v1_food_water_bridge.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_full_food_water(void)
{
    DM2_V1_FoodWaterInput in = {
        .food = DM2_V1_FOOD_WATER_MAX_VALUE,
        .water = DM2_V1_FOOD_WATER_MAX_VALUE,
        .poison = 0, .poisoned = 0
    };
    DM2_V1_FoodWaterReceipt r;
    assert(dm2_v1_food_water_bridge_compute(&in, -1, -1, &r));
    assert(r.valid);
    assert(r.food_pct == 100);
    assert(r.water_pct == 100);
    assert(r.food_color == 5);
    assert(r.water_color == 14);
    assert(!r.show_poison);
}

static void test_empty_food_water(void)
{
    DM2_V1_FoodWaterInput in = {
        .food = -1024, .water = -1024,
        .poison = 0, .poisoned = 0
    };
    DM2_V1_FoodWaterReceipt r;
    assert(dm2_v1_food_water_bridge_compute(&in, -1, -1, &r));
    assert(r.food_pct == 0);
    assert(r.water_pct == 0);
}

static void test_poisoned(void)
{
    DM2_V1_FoodWaterInput in = {
        .food = 1024, .water = 1024,
        .poison = 1536, .poisoned = 1
    };
    DM2_V1_FoodWaterReceipt r;
    assert(dm2_v1_food_water_bridge_compute(&in, -1, -1, &r));
    assert(r.show_poison);
    assert(r.poison_pct == 50);
    assert(r.poison_color == 8);
}

static void test_gdat_overrides(void)
{
    DM2_V1_FoodWaterInput in = {
        .food = 1024, .water = 1024
    };
    DM2_V1_FoodWaterReceipt r;
    assert(dm2_v1_food_water_bridge_compute(&in, 42, 99, &r));
    assert(r.food_color == 256 + 42);
    assert(r.water_color == 256 + 99);
}

static void test_null(void)
{
    DM2_V1_FoodWaterReceipt r;
    assert(!dm2_v1_food_water_bridge_compute(NULL, -1, -1, &r));
    assert(!r.valid);
}

int main(void)
{
    test_full_food_water();
    test_empty_food_water();
    test_poisoned();
    test_gdat_overrides();
    test_null();
    assert(dm2_v1_food_water_bridge_source_evidence() != NULL);
    printf("All dm2_v1_food_water_bridge tests passed.\n");
    return 0;
}
