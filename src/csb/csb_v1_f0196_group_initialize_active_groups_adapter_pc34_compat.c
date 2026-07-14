#include "csb_v1_f0196_group_initialize_active_groups_adapter_pc34_compat.h"

int csb_v1_f0196_group_initialize_active_groups_adapter_pc34_compat(
    CsbV1F0196GroupInitializeActiveGroupsAdapterPc34Compat *adapter)
{
    size_t initialized_count;
    size_t index;

    if (!adapter || !adapter->slots) return -1;

    if (adapter->new_game) {
        adapter->maximum_active_group_count =
            CSB_V1_F0196_GROUP_MINIMUM_ACTIVE_GROUP_CAPACITY;
    }

    initialized_count = adapter->maximum_active_group_count;
    if (initialized_count < CSB_V1_F0196_GROUP_MINIMUM_ACTIVE_GROUP_CAPACITY) {
        initialized_count = CSB_V1_F0196_GROUP_MINIMUM_ACTIVE_GROUP_CAPACITY;
    }
    if (initialized_count > adapter->slot_capacity) return -1;

    for (index = 0u; index < initialized_count; ++index) {
        adapter->slots[index].group_thing_index = -1;
    }
    return (int)initialized_count;
}
