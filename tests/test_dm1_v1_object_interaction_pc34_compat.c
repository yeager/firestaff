#include "dm1_v1_object_interaction_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

static int tests_passed = 0;
static int tests_failed = 0;
#define TEST(name) do { printf("  %-50s", name); } while(0)
#define PASS() do { printf("[PASS]\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("[FAIL] %s\n", msg); tests_failed++; } while(0)

static DM1ConsumableChampionPc34 base_consumable_champion(void) {
    DM1ConsumableChampionPc34 c;
    memset(&c, 0, sizeof(c));
    c.statistic[DM1_CONSUMABLE_STAT_STRENGTH] = 30;
    c.statistic[DM1_CONSUMABLE_STAT_DEXTERITY] = 31;
    c.statistic[DM1_CONSUMABLE_STAT_WISDOM] = 32;
    c.statistic[DM1_CONSUMABLE_STAT_VITALITY] = 33;
    c.currentHealth = 40;
    c.maximumHealth = 100;
    c.currentStamina = 50;
    c.maximumStamina = 120;
    c.currentMana = 80;
    c.maximumMana = 100;
    c.food = 1000;
    c.water = 1000;
    c.wounds = 0x00F0u;
    c.poisonDose = 12;
    return c;
}

static void test_spawn_and_pickup(void) {
    TEST("Spawn object and pick up");
    M11_ObjectState* state = (M11_ObjectState*)calloc(1, sizeof(M11_ObjectState));
    assert(state);
    m11_obj_init(state);
    int idx __attribute__((unused)) = m11_obj_spawn(state, DM1_OBJTYPE_WEAPON, 5, 3, 0, 25);
    assert(idx >= 0);
    assert(m11_obj_is_valid(state, idx));
    int w = 0;
    int ok __attribute__((unused)) = m11_obj_pickup(state, idx, &w);
    assert(ok == 0);
    assert(w == 25);
    free(state);
    PASS();
}

static void test_drop_and_get_at(void) {
    TEST("Drop object and find at position");
    M11_ObjectState* state = (M11_ObjectState*)calloc(1, sizeof(M11_ObjectState));
    assert(state);
    m11_obj_init(state);
    int idx __attribute__((unused)) = m11_obj_spawn(state, DM1_OBJTYPE_POTION, 2, 4, 0, 5);
    int found[8];
    int n __attribute__((unused)) = m11_obj_get_at(state, 2, 4, 0, found, 8);
    assert(n >= 1);
    free(state);
    PASS();
}

static void test_examine(void) {
    TEST("Examine object description");
    M11_ObjectState* state = (M11_ObjectState*)calloc(1, sizeof(M11_ObjectState));
    assert(state);
    m11_obj_init(state);
    int idx __attribute__((unused)) = m11_obj_spawn(state, DM1_OBJTYPE_TORCH, 0, 0, 0, 10);
    char desc[128];
    int ok __attribute__((unused)) = m11_obj_examine(state, idx, desc, sizeof(desc));
    assert(ok == 0);
    assert(strlen(desc) > 0);
    free(state);
    PASS();
}

static void test_type_name(void) {
    TEST("Object type names");
    assert(strcmp(m11_obj_type_name(DM1_OBJTYPE_WEAPON), "Weapon") == 0 ||
           strcmp(m11_obj_type_name(DM1_OBJTYPE_WEAPON), "weapon") == 0);
    PASS();
}

static void test_use_delegates_to_consumables(void) {
    M11_ObjectState* state;
    DM1ConsumableChampionPc34 champ;
    DM1ConsumableResultPc34 result;
    int idx;

    TEST("Use potion delegates to F0349 consumables");
    state = (M11_ObjectState*)calloc(1, sizeof(M11_ObjectState));
    assert(state);
    m11_obj_init(state);
    idx = m11_obj_spawn(state, DM1_OBJTYPE_POTION, 1, 1, 0, 80);
    assert(idx >= 0);
    state->objects[idx].stackCount = 6; /* ROS potion */
    champ = base_consumable_champion();
    memset(&result, 0, sizeof(result));
    assert(m11_obj_use(state, 0, idx, &champ, &result) == 1);
    assert(champ.statistic[DM1_CONSUMABLE_STAT_DEXTERITY] == 42);
    assert(result.kind == DM1_CONSUMABLE_RESULT_POTION);
    assert(result.potionTypeAfter == DM1_CONSUMABLE_POTION_EMPTY_FLASK_PC34);
    free(state);
    PASS();

    TEST("Use food delegates to F0349 consumables");
    state = (M11_ObjectState*)calloc(1, sizeof(M11_ObjectState));
    assert(state);
    m11_obj_init(state);
    idx = m11_obj_spawn(state, DM1_OBJTYPE_FOOD, 1, 1, 0, 171); /* cheese icon */
    assert(idx >= 0);
    champ = base_consumable_champion();
    memset(&result, 0, sizeof(result));
    assert(m11_obj_use(state, 0, idx, &champ, &result) == 1);
    assert(champ.food == 1820);
    assert(result.kind == DM1_CONSUMABLE_RESULT_FOOD_JUNK);
    assert(result.removeLeaderHandObject == 1);
    free(state);
    PASS();

    TEST("Use water delegates to F0349 consumables");
    state = (M11_ObjectState*)calloc(1, sizeof(M11_ObjectState));
    assert(state);
    m11_obj_init(state);
    idx = m11_obj_spawn(state, DM1_OBJTYPE_WATER, 1, 1, 0, 9); /* waterskin icon */
    assert(idx >= 0);
    state->objects[idx].stackCount = 3;
    champ = base_consumable_champion();
    memset(&result, 0, sizeof(result));
    assert(m11_obj_use(state, 0, idx, &champ, &result) == 1);
    assert(champ.water == 1800);
    assert(result.kind == DM1_CONSUMABLE_RESULT_WATER_JUNK);
    assert(result.chargeCountAfter == 2);
    free(state);
    PASS();

    TEST("Use equipment remains slot-system owned");
    state = (M11_ObjectState*)calloc(1, sizeof(M11_ObjectState));
    assert(state);
    m11_obj_init(state);
    idx = m11_obj_spawn(state, DM1_OBJTYPE_WEAPON, 1, 1, 0, 25);
    assert(idx >= 0);
    champ = base_consumable_champion();
    memset(&result, 0, sizeof(result));
    assert(m11_obj_use(state, 0, idx, &champ, &result) == 0);
    free(state);
    PASS();
}

int main(void) {
    printf("=== DM1 V1 Object Interaction Tests ===\n");
    test_spawn_and_pickup();
    test_drop_and_get_at();
    test_examine();
    test_type_name();
    test_use_delegates_to_consumables();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
