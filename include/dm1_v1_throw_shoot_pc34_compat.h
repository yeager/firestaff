#ifndef FIRESTAFF_DM1_V1_THROW_SHOOT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_THROW_SHOOT_PC34_COMPAT_H

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_THROW_WEAPON_CLASS_MAX_KINETIC_XP_PC34 = 12,
    DM1_THROW_MIN_ATTACK_PC34 = 40,
    DM1_THROW_MAX_ATTACK_PC34 = 200,
    DM1_THROW_MIN_STEP_ENERGY_PC34 = 5,
    DM1_THROW_DEFAULT_WEAPON_KINETIC_PC34 = 1,
    DM1_THROW_BASE_XP_PC34 = 8,
    DM1_THROW_WEAPON_XP_PC34 = 4,
    DM1_JUNK_WATERSKIN_PC34 = 1,
    DM1_POTION_VEN_PC34 = 3,
    DM1_POTION_FUL_BOMB_PC34 = 19
};

typedef struct {
    int championIndex;
    int championCell;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int partyDirection;
    int gameTick;
    int subtype;
    int category;
    int kineticEnergy;
    int impactAttack;
    int attackTypeCode;
    int launchCell;
    int launchDirection;
    int stepEnergy;
    int launcherStrength;
    unsigned short carriedThing;
    int potionPower;
} DM1_ProjectileCreateRequestPc34;

int dm1_v1_throwing_stamina_cost_from_weight_pc34(int objectWeight);
int dm1_v1_throw_armour_weight_f0140_pc34(int armourType);
int dm1_v1_throw_junk_base_weight_f0140_pc34(int junkType);
int dm1_v1_throw_junk_weight_f0140_pc34(int junkType, int chargeCount);
int dm1_v1_throw_xp_for_object_pc34(int isWeapon,
                                    int hasWeaponInfo,
                                    int weaponClass,
                                    int weaponKineticEnergy);
int dm1_v1_throw_side_pc34(int championCell, int partyDirection);
int dm1_v1_throw_kinetic_energy_pc34(int baseStrength,
                                     int throwSkillLevel,
                                     int hasWeaponInfo,
                                     int weaponClass,
                                     int weaponKineticEnergy,
                                     int rng16);
int dm1_v1_throw_attack_pc34(int throwSkillLevel, int rng32);
int dm1_v1_throw_step_energy_pc34(int throwSkillLevel);
int dm1_v1_thrown_potion_projectile_subtype_pc34(int potionType,
                                                 int* outSubtype);
int dm1_v1_shoot_step_energy_pc34(int actionClass, int* outStepEnergy);
int dm1_v1_shoot_ammunition_matches_pc34(int actionWeaponClass,
                                         int readyWeaponClass);
int dm1_v1_projectile_launch_cell_pc34(int championCell, int direction);
int dm1_v1_shoot_attack_pc34(int weaponShootAttack, int shootSkillLevel);
int dm1_v1_legacy_throw_attack_probe_pc34(int baseAttack, int throwSkillLevel);
int dm1_v1_build_projectile_create_input_pc34(
    const DM1_ProjectileCreateRequestPc34* req,
    struct ProjectileCreateInput_Compat* outInput);
const char* dm1_v1_projectile_subtype_name_pc34(int subtype);
int dm1_v1_projectile_impact_source_sound_index_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result);
int dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(int weaponType);

#ifdef __cplusplus
}
#endif

#endif
