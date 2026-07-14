#include "csb_v1_f0196_group_initialize_active_groups_adapter_pc34_compat.h"

#include <stdio.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

static void fill_slots(CsbV1F0196ActiveGroupSlotPc34Compat *slots,
                       size_t count,
                       int16_t value)
{
    size_t index;

    for (index = 0u; index < count; ++index) {
        slots[index].group_thing_index = value;
    }
}

static int slots_are(CsbV1F0196ActiveGroupSlotPc34Compat *slots,
                     size_t count,
                     int16_t value)
{
    size_t index;

    for (index = 0u; index < count; ++index) {
        if (slots[index].group_thing_index != value) return 0;
    }
    return 1;
}

int main(void)
{
    CsbV1F0196ActiveGroupSlotPc34Compat slots[128];
    CsbV1F0196GroupInitializeActiveGroupsAdapterPc34Compat adapter;

    fill_slots(slots, 128u, 42);
    adapter.new_game = 1;
    adapter.maximum_active_group_count = 60u;
    adapter.slots = slots;
    adapter.slot_capacity = 128u;
    CHECK(csb_v1_f0196_group_initialize_active_groups_adapter_pc34_compat(
              &adapter) == 110,
          "new game initializes the CSB source minimum allocation");
    CHECK(adapter.maximum_active_group_count == 110u,
          "new game sets the source active-group limit");
    CHECK(slots_are(slots, 110u, -1),
          "source allocation range receives the unused sentinel");
    CHECK(slots[110].group_thing_index == 42,
          "adapter does not write beyond the source allocation range");

    fill_slots(slots, 128u, 7);
    adapter.new_game = 0;
    adapter.maximum_active_group_count = 60u;
    CHECK(csb_v1_f0196_group_initialize_active_groups_adapter_pc34_compat(
              &adapter) == 110 && adapter.maximum_active_group_count == 60u,
          "existing game preserves its limit but allocates the source minimum");
    CHECK(slots_are(slots, 110u, -1) && slots[110].group_thing_index == 7,
          "existing game clears exactly its effective allocation");

    fill_slots(slots, 128u, 9);
    adapter.maximum_active_group_count = 128u;
    CHECK(csb_v1_f0196_group_initialize_active_groups_adapter_pc34_compat(
              &adapter) == 128 && slots_are(slots, 128u, -1),
          "configured limits above 110 control the initialized allocation");

    fill_slots(slots, 128u, 13);
    adapter.maximum_active_group_count = 110u;
    adapter.slot_capacity = 109u;
    CHECK(csb_v1_f0196_group_initialize_active_groups_adapter_pc34_compat(
              &adapter) == -1 && slots_are(slots, 128u, 13),
          "insufficient caller storage fails without partial initialization");
    adapter.slot_capacity = 128u;
    adapter.slots = NULL;
    CHECK(csb_v1_f0196_group_initialize_active_groups_adapter_pc34_compat(
              &adapter) == -1,
          "null caller storage is rejected");

    if (failures == 0) {
        puts("PASS: CSB F0196 bounded active-group initialization adapter");
    }
    return failures == 0 ? 0 : 1;
}
