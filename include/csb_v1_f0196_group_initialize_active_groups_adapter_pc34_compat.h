#ifndef CSB_V1_F0196_GROUP_INITIALIZE_ACTIVE_GROUPS_ADAPTER_PC34_COMPAT_H
#define CSB_V1_F0196_GROUP_INITIALIZE_ACTIVE_GROUPS_ADAPTER_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * Bounded, caller-owned adapter for ReDMCSB GROUP.C
 * F0196_GROUP_InitializeActiveGroups (MEDIA725 CSB branch).
 *
 * ReDMCSB allocates at least 110 ACTIVE_GROUP records and clears each
 * GroupThingIndex to -1. This adapter deliberately leaves allocation to the
 * caller and fails before writing when its supplied storage is too small.
 */
#define CSB_V1_F0196_GROUP_MINIMUM_ACTIVE_GROUP_CAPACITY 110u

typedef struct {
    int16_t group_thing_index;
} CsbV1F0196ActiveGroupSlotPc34Compat;

typedef struct {
    int new_game;
    uint16_t maximum_active_group_count;
    CsbV1F0196ActiveGroupSlotPc34Compat *slots;
    size_t slot_capacity;
} CsbV1F0196GroupInitializeActiveGroupsAdapterPc34Compat;

/*
 * Returns the number of initialized slots, or -1 when the adapter is invalid
 * or its caller-owned storage cannot hold the source-required allocation.
 * A new game sets maximum_active_group_count to 110; otherwise that configured
 * limit is preserved while the allocated/initialized storage is still at
 * least 110 records.
 */
int csb_v1_f0196_group_initialize_active_groups_adapter_pc34_compat(
    CsbV1F0196GroupInitializeActiveGroupsAdapterPc34Compat *adapter);

#endif
