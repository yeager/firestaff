#include "csb_v1_teleporter_rotation_runtime_pc34_compat.h"

#include <string.h>

static int normalize_dir(int value)
{
    return value & 3;
}

uint16_t csb_v1_teleporter_rotation_pack_values_pc34_compat(
    int value0, int value1, int value2, int value3)
{
    return (uint16_t)((normalize_dir(value0) << 0) |
                      (normalize_dir(value1) << 2) |
                      (normalize_dir(value2) << 4) |
                      (normalize_dir(value3) << 6));
}

int csb_v1_teleporter_rotation_get_packed_value_pc34_compat(
    uint16_t packed_values,
    int index)
{
    if (index < 0 || index > 3) return -1;
    return (int)((packed_values >> (index * 2)) & 3u);
}

static uint16_t set_packed_value(uint16_t packed_values, int index, int value)
{
    uint16_t mask;

    if (index < 0 || index > 3) return packed_values;
    mask = (uint16_t)(3u << (index * 2));
    packed_values &= (uint16_t)~mask;
    packed_values |= (uint16_t)(normalize_dir(value) << (index * 2));
    return packed_values;
}

uint16_t csb_v1_teleporter_rotation_thing_with_cell_pc34_compat(
    uint16_t thing,
    int cell)
{
    /* ReDMCSB DEFS.H:398/402 M011_CELL and M015_THING_WITH_NEW_CELL. */
    return (uint16_t)((thing & 0x3FFFu) | ((uint16_t)normalize_dir(cell) << 14));
}

int csb_v1_teleporter_rotation_thing_cell_pc34_compat(uint16_t thing)
{
    /* ReDMCSB DEFS.H:398 M011_CELL. */
    return (int)((thing >> 14) & 3u);
}

int csb_v1_teleporter_rotation_apply_party_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *teleporter,
    CSB_V1_TeleporterRotationRuntimePartyResultPc34 *out_result)
{
    CSB_V1_TeleporterRotationRuntimePartyResultPc34 result;
    int old_dir;
    int target_dir;

    if (!profile || !teleporter) return -1;

    memset(&result, 0, sizeof(result));
    old_dir = profile->party_dir & 3;
    result.old_party_x = profile->party_x;
    result.old_party_y = profile->party_y;
    result.old_party_map_index = profile->current_level;
    result.old_party_dir = old_dir;
    result.used_absolute_rotation = teleporter->absolute_rotation ? 1 : 0;
    result.audible_buzz_requested = teleporter->audible ? 1 : 0;

    target_dir = teleporter->absolute_rotation
        ? normalize_dir(teleporter->rotation)
        : normalize_dir(old_dir + teleporter->rotation);

    /* ReDMCSB MOVESENS.C:493-518 moves the party to the teleporter target,
     * then routes absolute/relative teleporter rotation through CHAMPION.C
     * F0284.  csb_v1_runtime_rotate_party is Firestaff's F0284 runtime
     * boundary and preserves champion Cell/Direction deltas. */
    profile->party_x = teleporter->target_map_x;
    profile->party_y = teleporter->target_map_y;
    profile->current_level = teleporter->target_map_index;
    if (profile->party_state_valid) {
        profile->party_state.PartyMapX = profile->party_x;
        profile->party_state.PartyMapY = profile->party_y;
    }
    if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
        return -1;
    }

    result.new_party_x = profile->party_x;
    result.new_party_y = profile->party_y;
    result.new_party_map_index = profile->current_level;
    result.new_party_dir = profile->party_dir & 3;
    result.party_state_changed =
        result.old_party_x != result.new_party_x ||
        result.old_party_y != result.new_party_y ||
        result.old_party_map_index != result.new_party_map_index ||
        result.old_party_dir != result.new_party_dir;

    if (out_result) *out_result = result;
    return 0;
}

int csb_v1_teleporter_rotation_apply_group_pc34_compat(
    const CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *teleporter,
    const CSB_V1_TeleporterRotationRuntimeGroupPc34 *group,
    CSB_V1_TeleporterRotationRuntimeGroupResultPc34 *out_result)
{
    CSB_V1_TeleporterRotationRuntimeGroupResultPc34 result;
    int creature_count;
    int i;
    uint16_t new_dirs;
    uint16_t new_cells;

    if (!teleporter || !group || !out_result) return -1;
    if (group->count < 0 || group->count >= CSB_V1_MAX_CHAMPIONS) return -1;

    memset(&result, 0, sizeof(result));
    creature_count = group->count + 1;
    new_dirs = group->directions_packed;
    new_cells = group->cells_packed;
    result.used_absolute_rotation = teleporter->absolute_rotation ? 1 : 0;

    /* ReDMCSB MOVESENS.C:33-111 F0262 rotates each creature's packed
     * direction and, unless the group uses C0xFF_SINGLE_CENTERED_CREATURE,
     * rotates the packed cell value by the same relative delta. */
    for (i = 0; i < creature_count; ++i) {
        int old_dir = csb_v1_teleporter_rotation_get_packed_value_pc34_compat(
            group->directions_packed, i);
        int new_dir = teleporter->absolute_rotation
            ? normalize_dir(teleporter->rotation)
            : normalize_dir(old_dir + teleporter->rotation);
        int relative_rotation = normalize_dir(new_dir - old_dir);

        new_dirs = set_packed_value(new_dirs, i, new_dir);

        if (group->cells_packed !=
            CSB_V1_TELEPORTER_ROTATION_SINGLE_CENTERED_CREATURE_PC34) {
            int old_cell =
                csb_v1_teleporter_rotation_get_packed_value_pc34_compat(
                    group->cells_packed, i);

            if (group->creature_size ==
                    CSB_V1_TELEPORTER_ROTATION_SIZE_QUARTER_SQUARE_PC34 &&
                !teleporter->absolute_rotation) {
                relative_rotation = normalize_dir(teleporter->rotation);
            }
            if (relative_rotation) {
                new_cells = set_packed_value(
                    new_cells, i, old_cell + relative_rotation);
            }
        }
    }

    result.directions_packed = new_dirs;
    result.cells_packed = new_cells;
    result.move_group_result = 1;
    if (group->source_map_index == group->party_map_index &&
        group->behavior == CSB_V1_TELEPORTER_ROTATION_BEHAVIOR_ATTACK_PC34) {
        result.move_group_result = group->active_group_index + 2;
    }

    *out_result = result;
    return 0;
}

int csb_v1_teleporter_rotation_apply_projectile_pc34_compat(
    const CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *teleporter,
    uint16_t projectile_thing,
    int move_result_direction,
    CSB_V1_TeleporterRotationRuntimeProjectileResultPc34 *out_result)
{
    CSB_V1_TeleporterRotationRuntimeProjectileResultPc34 result;
    int new_direction;

    if (!teleporter || !out_result) return -1;
    memset(&result, 0, sizeof(result));
    result.thing = projectile_thing;
    result.used_absolute_rotation = teleporter->absolute_rotation ? 1 : 0;

    /* ReDMCSB MOVESENS.C:113-134 F0263 sets projectile direction to the
     * absolute rotation, or applies relative rotation to both direction and
     * the thing cell via M015_THING_WITH_NEW_CELL. */
    if (teleporter->absolute_rotation) {
        new_direction = normalize_dir(teleporter->rotation);
    } else {
        new_direction = normalize_dir(move_result_direction + teleporter->rotation);
        result.thing = csb_v1_teleporter_rotation_thing_with_cell_pc34_compat(
            projectile_thing,
            csb_v1_teleporter_rotation_thing_cell_pc34_compat(projectile_thing) +
                teleporter->rotation);
    }
    result.direction = new_direction;
    *out_result = result;
    return 0;
}

int csb_v1_teleporter_rotation_apply_object_cell_pc34_compat(
    const CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *teleporter,
    uint16_t thing,
    int source_map_x,
    CSB_V1_TeleporterRotationRuntimeObjectResultPc34 *out_result)
{
    CSB_V1_TeleporterRotationRuntimeObjectResultPc34 result;

    if (!teleporter || !out_result) return -1;
    memset(&result, 0, sizeof(result));
    result.thing = thing;

    /* ReDMCSB MOVESENS.C:529-530 rotates non-projectile object cells only
     * for relative teleporters and explicitly exempts projectile-associated
     * objects (CM2_MAPX_PROJECTILE_ASSOCIATED_OBJECT). */
    if (source_map_x ==
        CSB_V1_TELEPORTER_ROTATION_SOURCE_PROJECTILE_ASSOCIATED_OBJECT_PC34) {
        result.associated_projectile_object_exempt = 1;
    } else if (!teleporter->absolute_rotation) {
        result.thing = csb_v1_teleporter_rotation_thing_with_cell_pc34_compat(
            thing,
            csb_v1_teleporter_rotation_thing_cell_pc34_compat(thing) +
                teleporter->rotation);
        result.cell_rotated = 1;
    }

    *out_result = result;
    return 0;
}

const char *csb_v1_teleporter_rotation_runtime_source_evidence_pc34_compat(void)
{
    return "ReDMCSB MOVESENS.C F0262 lines 33-111 rotates teleported group directions/cells; "
           "MOVESENS.C F0263 lines 113-134 rotates projectile direction and thing cell; "
           "MOVESENS.C F0267 lines 493-530 applies party/object teleporter absolute/relative rotation; "
           "DEFS.H lines 398/402 define M011_CELL and M015_THING_WITH_NEW_CELL; "
           "DEFS.H lines 1367/1377 define C0xFF_SINGLE_CENTERED_CREATURE and C6_BEHAVIOR_ATTACK; "
           "DEFS.H lines 2886-2887 define CM1/CM2 source-map sentinels; "
           "CHAMPION.C F0284 lines 117-130 rotates party champion Cell/Direction.";
}
