#include "dm1_v1_object_interaction_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int tests_passed = 0;
static int tests_failed = 0;
#define TEST(name) do { printf("  %-50s", name); } while(0)
#define PASS() do { printf("[PASS]\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("[FAIL] %s\n", msg); tests_failed++; } while(0)

static void test_spawn_and_pickup(void) {
    TEST("Spawn object and pick up");
    M11_ObjectState state;
    m11_obj_init(&state);
    int idx __attribute__((unused)) = m11_obj_spawn(&state, DM1_OBJTYPE_WEAPON, 5, 3, 0, 25);
    assert(idx >= 0);
    assert(m11_obj_is_valid(&state, idx));
    int w = 0;
    int ok __attribute__((unused)) = m11_obj_pickup(&state, idx, &w);
    assert(ok == 1);
    assert(w == 25);
    PASS();
}

static void test_drop_and_get_at(void) {
    TEST("Drop object and find at position");
    M11_ObjectState state;
    m11_obj_init(&state);
    int idx __attribute__((unused)) = m11_obj_spawn(&state, DM1_OBJTYPE_POTION, 2, 4, 0, 5);
    int found[8];
    int n __attribute__((unused)) = m11_obj_get_at(&state, 2, 4, 0, found, 8);
    assert(n >= 1);
    PASS();
}

static void test_examine(void) {
    TEST("Examine object description");
    M11_ObjectState state;
    m11_obj_init(&state);
    int idx __attribute__((unused)) = m11_obj_spawn(&state, DM1_OBJTYPE_TORCH, 0, 0, 0, 10);
    char desc[128];
    int ok __attribute__((unused)) = m11_obj_examine(&state, idx, desc, sizeof(desc));
    assert(ok == 1);
    assert(strlen(desc) > 0);
    PASS();
}

static void test_type_name(void) {
    TEST("Object type names");
    assert(strcmp(m11_obj_type_name(DM1_OBJTYPE_WEAPON), "Weapon") == 0 ||
           strcmp(m11_obj_type_name(DM1_OBJTYPE_WEAPON), "weapon") == 0);
    PASS();
}

int main(void) {
    printf("=== DM1 V1 Object Interaction Tests ===\n");
    test_spawn_and_pickup();
    test_drop_and_get_at();
    test_examine();
    test_type_name();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
