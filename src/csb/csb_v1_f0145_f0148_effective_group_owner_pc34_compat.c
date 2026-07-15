#include "csb_v1_f0145_f0148_effective_group_owner_pc34_compat.h"

static int csb_v1_f0145_f0148_owner_is_valid(
    const CsbV1F0145F0148EffectiveGroupOwnerPc34Compat *owner)
{
    if (!owner || !owner->group_record ||
        owner->record_size < CSB_V1_F0145_F0148_C04_RECORD_SIZE_PC34 ||
        ((owner->group_thing >> 10) & 0x0fu) !=
            CSB_V1_F0145_F0148_THING_TYPE_GROUP_PC34) {
        return 0;
    }
    if (owner->map_index == owner->party_map_index) {
        const uint8_t active_index = owner->group_record[
            CSB_V1_F0145_F0148_C04_ACTIVE_GROUP_INDEX_OFFSET_PC34];
        if (!owner->active_groups ||
            active_index >= owner->active_group_count ||
            !owner->active_groups[active_index].valid) {
            return 0;
        }
    }
    return 1;
}

int csb_v1_f0145_f0148_effective_group_read_pc34_compat(
    const CsbV1F0145F0148EffectiveGroupOwnerPc34Compat *owner,
    CsbV1F0145F0148EffectiveGroupValuesPc34Compat *out_values)
{
    static const uint8_t group_directions[4] = { 0x00u, 0x55u, 0xaau, 0xffu };

    if (!csb_v1_f0145_f0148_owner_is_valid(owner) || !out_values) {
        return 0;
    }
    if (owner->map_index == owner->party_map_index) {
        const uint8_t active_index = owner->group_record[
            CSB_V1_F0145_F0148_C04_ACTIVE_GROUP_INDEX_OFFSET_PC34];
        out_values->cells = owner->active_groups[active_index].cells;
        out_values->directions = owner->active_groups[active_index].directions;
    } else {
        out_values->cells = owner->group_record[
            CSB_V1_F0145_F0148_C04_ACTIVE_GROUP_INDEX_OFFSET_PC34];
        out_values->directions = group_directions[
            owner->group_record[
                CSB_V1_F0145_F0148_C04_DIRECTION_OFFSET_PC34] & 0x03u];
    }
    return 1;
}

int csb_v1_f0145_f0148_effective_group_write_pc34_compat(
    CsbV1F0145F0148EffectiveGroupOwnerPc34Compat *owner,
    const CsbV1F0145F0148EffectiveGroupMutationPc34Compat *mutation)
{
    if (!csb_v1_f0145_f0148_owner_is_valid(owner) || !mutation ||
        (!mutation->write_cells && !mutation->write_directions)) {
        return 0;
    }
    if (owner->map_index == owner->party_map_index) {
        const uint8_t active_index = owner->group_record[
            CSB_V1_F0145_F0148_C04_ACTIVE_GROUP_INDEX_OFFSET_PC34];
        CSB_V1_RuntimeActiveGroupState next = owner->active_groups[active_index];

        if (mutation->write_cells) next.cells = mutation->cells;
        if (mutation->write_directions) next.directions = mutation->directions;
        owner->active_groups[active_index] = next;
    } else {
        uint8_t next_cells = owner->group_record[
            CSB_V1_F0145_F0148_C04_ACTIVE_GROUP_INDEX_OFFSET_PC34];
        uint8_t next_direction = owner->group_record[
            CSB_V1_F0145_F0148_C04_DIRECTION_OFFSET_PC34];

        if (mutation->write_cells) next_cells = mutation->cells;
        if (mutation->write_directions) {
            next_direction = (uint8_t)((next_direction & 0xfcu) |
                                       (mutation->directions & 0x0003u));
        }
        owner->group_record[
            CSB_V1_F0145_F0148_C04_ACTIVE_GROUP_INDEX_OFFSET_PC34] = next_cells;
        owner->group_record[
            CSB_V1_F0145_F0148_C04_DIRECTION_OFFSET_PC34] = next_direction;
    }
    return 1;
}
