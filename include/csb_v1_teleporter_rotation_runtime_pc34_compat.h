#ifndef FIRESTAFF_CSB_V1_TELEPORTER_ROTATION_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_TELEPORTER_ROTATION_RUNTIME_PC34_COMPAT_H

#include <stdint.h>

#include "csb_v1_runtime_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_TELEPORTER_ROTATION_SINGLE_CENTERED_CREATURE_PC34 = 0xFF,
    CSB_V1_TELEPORTER_ROTATION_SIZE_QUARTER_SQUARE_PC34 = 0,
    CSB_V1_TELEPORTER_ROTATION_BEHAVIOR_ATTACK_PC34 = 6,
    CSB_V1_TELEPORTER_ROTATION_SOURCE_PROJECTILE_ASSOCIATED_OBJECT_PC34 = -2
};

typedef struct {
    int target_map_x;
    int target_map_y;
    int target_map_index;
    int rotation;
    int absolute_rotation;
    int audible;
} CSB_V1_TeleporterRotationRuntimeTeleporterPc34;

typedef struct {
    int old_party_x;
    int old_party_y;
    int old_party_map_index;
    int old_party_dir;
    int new_party_x;
    int new_party_y;
    int new_party_map_index;
    int new_party_dir;
    int used_absolute_rotation;
    int audible_buzz_requested;
    int party_state_changed;
} CSB_V1_TeleporterRotationRuntimePartyResultPc34;

typedef struct {
    int count;
    int creature_size;
    uint16_t directions_packed;
    uint16_t cells_packed;
    int behavior;
    int active_group_index;
    int source_map_index;
    int party_map_index;
} CSB_V1_TeleporterRotationRuntimeGroupPc34;

typedef struct {
    uint16_t directions_packed;
    uint16_t cells_packed;
    int move_group_result;
    int used_absolute_rotation;
} CSB_V1_TeleporterRotationRuntimeGroupResultPc34;

typedef struct {
    uint16_t thing;
    int direction;
    int used_absolute_rotation;
} CSB_V1_TeleporterRotationRuntimeProjectileResultPc34;

typedef struct {
    uint16_t thing;
    int cell_rotated;
    int associated_projectile_object_exempt;
} CSB_V1_TeleporterRotationRuntimeObjectResultPc34;

uint16_t csb_v1_teleporter_rotation_pack_values_pc34_compat(
    int value0, int value1, int value2, int value3);
int csb_v1_teleporter_rotation_get_packed_value_pc34_compat(
    uint16_t packed_values,
    int index);
uint16_t csb_v1_teleporter_rotation_thing_with_cell_pc34_compat(
    uint16_t thing,
    int cell);
int csb_v1_teleporter_rotation_thing_cell_pc34_compat(uint16_t thing);

int csb_v1_teleporter_rotation_apply_party_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *teleporter,
    CSB_V1_TeleporterRotationRuntimePartyResultPc34 *out_result);

int csb_v1_teleporter_rotation_apply_group_pc34_compat(
    const CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *teleporter,
    const CSB_V1_TeleporterRotationRuntimeGroupPc34 *group,
    CSB_V1_TeleporterRotationRuntimeGroupResultPc34 *out_result);

int csb_v1_teleporter_rotation_apply_projectile_pc34_compat(
    const CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *teleporter,
    uint16_t projectile_thing,
    int move_result_direction,
    CSB_V1_TeleporterRotationRuntimeProjectileResultPc34 *out_result);

int csb_v1_teleporter_rotation_apply_object_cell_pc34_compat(
    const CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *teleporter,
    uint16_t thing,
    int source_map_x,
    CSB_V1_TeleporterRotationRuntimeObjectResultPc34 *out_result);

const char *csb_v1_teleporter_rotation_runtime_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
