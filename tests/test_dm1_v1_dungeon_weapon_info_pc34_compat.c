#include "dm1_v1_dungeon_weapon_info_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_source_evidence(void) {
    const char *ev = dm1_v1_dungeon_weapon_info_source_evidence_pc34();
    assert(ev != NULL);
    assert(strlen(ev) > 0);
    /* Should reference DUNGEON.C F0158 */
    assert(strstr(ev, "F0158") != NULL);
    (void)ev;
}

static void test_weapon_info_enum_constants(void) {
    /* Verify attack type constants from combat header */
    assert(DM1_ATTACK_NORMAL == 0);
    assert(DM1_ATTACK_FIRE == 1);
    assert(DM1_ATTACK_MAGIC == 5);
}

static void test_weapon_info_struct_layout(void) {
    DM1_WeaponInfo info;
    memset(&info, 0, sizeof(info));
    info.strength = 10;
    info.kineticEnergy = 20;
    info.weaponClass = 3;
    info.weight = 5;
    info.attributes = 0x01;
    assert(info.strength == 10);
    assert(info.kineticEnergy == 20);
    assert(info.weaponClass == 3);
    assert(info.weight == 5);
    assert(info.attributes == 0x01);
    (void)info;
}

int main(void) {
    test_source_evidence();
    test_weapon_info_enum_constants();
    test_weapon_info_struct_layout();
    puts("ok: dm1_v1_dungeon_weapon_info_pc34_compat 3 tests passed");
    return 0;
}
