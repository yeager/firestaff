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
    DM1_V1_ObjectStatePc34* state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    int idx __attribute__((unused)) = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_WEAPON, 5, 3, 0, 25);
    assert(idx >= 0);
    assert(DM1_V1_Object_IsValidPc34Compat(state, idx));
    int w = 0;
    int ok __attribute__((unused)) = DM1_V1_Object_PickupPc34Compat(state, idx, &w);
    assert(ok == 0);
    assert(w == 25);
    free(state);
    PASS();
}

static void test_drop_and_get_at(void) {
    TEST("Drop object and find at position");
    DM1_V1_ObjectStatePc34* state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    int idx __attribute__((unused)) = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_POTION, 2, 4, 0, 5);
    int found[8];
    int n __attribute__((unused)) = DM1_V1_Object_GetAtPc34Compat(state, 2, 4, 0, found, 8);
    assert(n >= 1);
    free(state);
    PASS();
}

static void test_drop_relinks_exactly_once(void) {
    DM1_V1_ObjectStatePc34* state;
    int idx;
    int found[4];

    TEST("Drop unlinks old floor list before relinking");
    state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    idx = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_WEAPON, 2, 4, 0, 5);
    assert(idx >= 0);
    assert(DM1_V1_Object_DropPc34Compat(state, idx, 7, 9, 1) == 0);
    assert(DM1_V1_Object_GetAtPc34Compat(state, 2, 4, 0, found, 4) == 0);
    assert(DM1_V1_Object_GetAtPc34Compat(state, 7, 9, 1, found, 4) == 1);
    assert(found[0] == idx);
    /* A repeated drop on the same square must not create a second list entry. */
    assert(DM1_V1_Object_DropPc34Compat(state, idx, 7, 9, 1) == 0);
    assert(DM1_V1_Object_GetAtPc34Compat(state, 7, 9, 1, found, 4) == 1);
    assert(found[0] == idx);
    free(state);
    PASS();
}

static void test_drop_to_full_cell_is_atomic(void) {
    DM1_V1_ObjectStatePc34* state;
    int object;
    int i;

    TEST("Drop to a full floor cell preserves carried object");
    state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    object = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_KEY, 1, 1, 0, 1);
    assert(object >= 0);
    for (i = 0; i < DM1_V1_MAX_FLOOR_OBJECTS_PC34; ++i) {
        assert(DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_MISC, 8, 8, 0, 1) >= 0);
    }
    assert(DM1_V1_Object_DropPc34Compat(state, object, 8, 8, 0) == -1);
    assert(state->objects[object].x == 1);
    assert(state->objects[object].y == 1);
    assert(state->objects[object].level == 0);
    assert(state->floors[0][1][1].floorCount == 1);
    assert(state->floors[0][1][1].floorObjects[0].objectId == object);
    assert(state->floors[0][8][8].floorCount == DM1_V1_MAX_FLOOR_OBJECTS_PC34);
    free(state);
    PASS();
}

static void test_drop_rejects_orphaned_floor_coordinate(void) {
    DM1_V1_ObjectStatePc34* state;
    int object;
    int found[4];

    TEST("Drop rejects an object whose claimed source cell does not own it");
    state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    object = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_KEY, 1, 1, 0, 1);
    assert(object >= 0);
    assert(DM1_V1_Object_PickupPc34Compat(state, object, NULL) == 0);
    /* Deliberately corrupt only the compact coordinate mirror. The source
     * list remains authoritative, so F0164 must reject the attempted move. */
    state->objects[object].x = 1;
    state->objects[object].y = 1;
    state->objects[object].level = 0;
    assert(DM1_V1_Object_DropPc34Compat(state, object, 7, 9, 0) == -1);
    assert(DM1_V1_Object_GetAtPc34Compat(state, 7, 9, 0, found, 4) == 0);
    free(state);
    PASS();
}

static void test_examine(void) {
    TEST("Examine object description");
    DM1_V1_ObjectStatePc34* state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    int idx __attribute__((unused)) = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_TORCH, 0, 0, 0, 10);
    char desc[128];
    int ok __attribute__((unused)) = DM1_V1_Object_ExaminePc34Compat(state, idx, desc, sizeof(desc));
    assert(ok == 0);
    assert(strlen(desc) > 0);
    free(state);
    PASS();
}

static void test_type_name(void) {
    TEST("Object type names");
    assert(strcmp(DM1_V1_Object_TypeNamePc34Compat(DM1_OBJTYPE_WEAPON), "Weapon") == 0 ||
           strcmp(DM1_V1_Object_TypeNamePc34Compat(DM1_OBJTYPE_WEAPON), "weapon") == 0);
    PASS();
}

static void test_use_delegates_to_consumables(void) {
    DM1_V1_ObjectStatePc34* state;
    DM1ConsumableChampionPc34 champ;
    (void)champ;
    DM1ConsumableResultPc34 result;
    int idx;

    TEST("Use potion delegates to F0349 consumables");
    state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    idx = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_POTION, 1, 1, 0, 80);
    assert(idx >= 0);
    state->objects[idx].stackCount = 6; /* ROS potion */
    champ = base_consumable_champion();
    memset(&result, 0, sizeof(result));
    assert(DM1_V1_Object_UsePc34Compat(state, 0, idx, &champ, &result) == 1);
    assert(champ.statistic[DM1_CONSUMABLE_STAT_DEXTERITY] == 42);
    assert(result.kind == DM1_CONSUMABLE_RESULT_POTION);
    assert(result.potionTypeAfter == DM1_CONSUMABLE_POTION_EMPTY_FLASK_PC34);
    free(state);
    PASS();

    TEST("Use food delegates to F0349 consumables");
    state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    idx = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_FOOD, 1, 1, 0, 171); /* cheese icon */
    assert(idx >= 0);
    champ = base_consumable_champion();
    memset(&result, 0, sizeof(result));
    assert(DM1_V1_Object_UsePc34Compat(state, 0, idx, &champ, &result) == 1);
    assert(champ.food == 1820);
    assert(result.kind == DM1_CONSUMABLE_RESULT_FOOD_JUNK);
    assert(result.removeLeaderHandObject == 1);
    free(state);
    PASS();

    TEST("Use water delegates to F0349 consumables");
    state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    idx = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_WATER, 1, 1, 0, 9); /* waterskin icon */
    assert(idx >= 0);
    state->objects[idx].stackCount = 3;
    champ = base_consumable_champion();
    memset(&result, 0, sizeof(result));
    assert(DM1_V1_Object_UsePc34Compat(state, 0, idx, &champ, &result) == 1);
    assert(champ.water == 1800);
    assert(result.kind == DM1_CONSUMABLE_RESULT_WATER_JUNK);
    assert(result.chargeCountAfter == 2);
    free(state);
    PASS();

    TEST("Use equipment remains slot-system owned");
    state = (DM1_V1_ObjectStatePc34*)calloc(1, sizeof(DM1_V1_ObjectStatePc34));
    assert(state);
    DM1_V1_Object_InitPc34Compat(state);
    idx = DM1_V1_Object_SpawnPc34Compat(state, DM1_OBJTYPE_WEAPON, 1, 1, 0, 25);
    assert(idx >= 0);
    champ = base_consumable_champion();
    memset(&result, 0, sizeof(result));
    assert(DM1_V1_Object_UsePc34Compat(state, 0, idx, &champ, &result) == 0);
    free(state);
    PASS();
}

int main(void) {
    printf("=== DM1 V1 Object Interaction Tests ===\n");
    test_spawn_and_pickup();
    test_drop_and_get_at();
    test_drop_relinks_exactly_once();
    test_drop_to_full_cell_is_atomic();
    test_drop_rejects_orphaned_floor_coordinate();
    test_examine();
    test_type_name();
    test_use_delegates_to_consumables();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
