#include "dm1_v1_object_world_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_count_constants(void) {
    assert(DM1_OBJ_NAME_COUNT == 199);
    assert(DM1_OBJ_INFO_COUNT == 180);
    assert(DM1_WEAPON_INFO_COUNT == 46);
    assert(DM1_ARMOUR_INFO_COUNT == 58);
    assert(DM1_CREATURE_INFO_COUNT == 27);
    assert(DM1_DOOR_INFO_COUNT == 4);
}

static void test_init(void) {
    DM1_V1_ObjectWorldStatePc34 state;
    memset(&state, 0xFF, sizeof(state));
    DM1_V1_ObjectWorld_InitPc34Compat(&state);
    assert(state.loaded == false);
}

static void test_get_object_name_unloaded(void) {
    DM1_V1_ObjectWorldStatePc34 state;
    DM1_V1_ObjectWorld_InitPc34Compat(&state);
    const char* name = DM1_V1_ObjectWorld_GetObjectNamePc34Compat(&state, 0);
    (void)name;
    assert(name == NULL);
}

static void test_get_object_info_unloaded(void) {
    DM1_V1_ObjectWorldStatePc34 state;
    DM1_V1_ObjectWorld_InitPc34Compat(&state);
    const DM1_V1_ObjectWorldObjectInfoPc34* info =
        DM1_V1_ObjectWorld_GetObjectInfoPc34Compat(&state, 0);
    (void)info;
    /* Returns pointer to zeroed struct, not NULL */
    assert(info != NULL);
}

static void test_get_weapon_unloaded(void) {
    DM1_V1_ObjectWorldStatePc34 state;
    DM1_V1_ObjectWorld_InitPc34Compat(&state);
    const DM1_V1_ObjectWorldWeaponInfoPc34* w =
        DM1_V1_ObjectWorld_GetWeaponPc34Compat(&state, 0);
    (void)w;
    assert(w != NULL);
}

static void test_get_armour_unloaded(void) {
    DM1_V1_ObjectWorldStatePc34 state;
    DM1_V1_ObjectWorld_InitPc34Compat(&state);
    const DM1_V1_ObjectWorldArmourInfoPc34* a =
        DM1_V1_ObjectWorld_GetArmourPc34Compat(&state, 0);
    (void)a;
    assert(a != NULL);
}

static void test_get_creature_unloaded(void) {
    DM1_V1_ObjectWorldStatePc34 state;
    DM1_V1_ObjectWorld_InitPc34Compat(&state);
    const DM1_V1_ObjectWorldCreatureInfoPc34* c =
        DM1_V1_ObjectWorld_GetCreaturePc34Compat(&state, 0);
    (void)c;
    assert(c != NULL);
}

static void test_get_door_unloaded(void) {
    DM1_V1_ObjectWorldStatePc34 state;
    DM1_V1_ObjectWorld_InitPc34Compat(&state);
    const DM1_V1_ObjectWorldDoorInfoPc34* d =
        DM1_V1_ObjectWorld_GetDoorPc34Compat(&state, 0);
    (void)d;
    assert(d != NULL);
}

static void test_cleanup(void) {
    DM1_V1_ObjectWorldStatePc34 state;
    DM1_V1_ObjectWorld_InitPc34Compat(&state);
    DM1_V1_ObjectWorld_CleanupPc34Compat(&state);
    assert(state.loaded == false);
}

int main(void) {
    test_count_constants();
    test_init();
    test_get_object_name_unloaded();
    test_get_object_info_unloaded();
    test_get_weapon_unloaded();
    test_get_armour_unloaded();
    test_get_creature_unloaded();
    test_get_door_unloaded();
    test_cleanup();
    puts("ok: DM1 object world (Q-DM1-04) 9 tests passed");
    return 0;
}
