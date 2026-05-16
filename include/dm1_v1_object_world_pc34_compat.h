#ifndef FIRESTAFF_DM1_V1_OBJECT_WORLD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_OBJECT_WORLD_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_OBJ_NAME_COUNT 199
#define DM1_OBJ_INFO_COUNT 180
#define DM1_WEAPON_INFO_COUNT 46
#define DM1_ARMOUR_INFO_COUNT 58
#define DM1_CREATURE_INFO_COUNT 27
#define DM1_DOOR_INFO_COUNT 4

typedef struct {
    uint16_t type;
    uint16_t weight;
    int16_t icon_index;
} M11_OW_ObjectInfo;

typedef struct {
    uint16_t wclass;
    uint16_t strength;
    uint16_t kineticEnergy;
    int16_t range;
} M11_OW_WeaponInfo;

typedef struct {
    uint16_t aclass;
    uint16_t defense;
    uint16_t weight;
} M11_OW_ArmourInfo;

typedef struct {
    uint16_t type;
    uint16_t hp;
    uint16_t attack;
    uint16_t defense;
    uint16_t speed;
    uint8_t side_attack;
    uint8_t preferred_distance;
} M11_OW_CreatureInfo;

typedef struct {
    uint16_t type;
    uint16_t resistance;
    bool destroyable;
} M11_OW_DoorInfo;

typedef struct {
    char* obj_names[DM1_OBJ_NAME_COUNT];
    M11_OW_ObjectInfo obj_info[DM1_OBJ_INFO_COUNT];
    M11_OW_WeaponInfo weapons[DM1_WEAPON_INFO_COUNT];
    M11_OW_ArmourInfo armour[DM1_ARMOUR_INFO_COUNT];
    M11_OW_CreatureInfo creatures[DM1_CREATURE_INFO_COUNT];
    M11_OW_DoorInfo doors[DM1_DOOR_INFO_COUNT];
    bool loaded;
} M11_OW_WorldState;

void m11_ow_init(M11_OW_WorldState* state);
bool m11_ow_load_object_names(M11_OW_WorldState* state, const uint8_t* data, size_t size);
const char* m11_ow_get_obj_name(const M11_OW_WorldState* state, uint16_t idx);
const M11_OW_ObjectInfo* m11_ow_get_obj_info(const M11_OW_WorldState* state, uint16_t idx);
const M11_OW_WeaponInfo* m11_ow_get_weapon(const M11_OW_WorldState* state, uint16_t idx);
const M11_OW_ArmourInfo* m11_ow_get_armour(const M11_OW_WorldState* state, uint16_t idx);
const M11_OW_CreatureInfo* m11_ow_get_creature(const M11_OW_WorldState* state, uint16_t idx);
const M11_OW_DoorInfo* m11_ow_get_door(const M11_OW_WorldState* state, uint16_t idx);
void m11_ow_cleanup(M11_OW_WorldState* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_OBJECT_WORLD_PC34_COMPAT_H */
