#include "dm2_v1_system_helpers.h"

#include <assert.h>
#include <stdint.h>

static void test_return_1_and_game_ended(void)
{
    DM2_V1_SystemReceipt receipt;
    (void)receipt;

    assert(dm2_v1_RETURN_1(0, &receipt) == 1u);
    assert(receipt.handled == 1);
    assert(receipt.source_locked == 1);
    assert(receipt.valid == 1);
    assert(receipt.result == 1);
    assert(receipt.symbol != 0);
    assert(receipt.source_path != 0);

    assert(dm2_v1_IS_GAME_ENDED(7u, 7u, &receipt) == 1u);
    assert(receipt.valid == 1);
    assert(receipt.result == 1);

    assert(dm2_v1_IS_GAME_ENDED(6u, 7u, &receipt) == 0u);
    assert(receipt.valid == 1);
    assert(receipt.result == 0);
}

static void test_locate_other_level_uses_skproject_scan_and_tile_blockers(void)
{
    const DM2_V1_MapDefinitionCompat maps[] = {
        { 3, 10, 20, 15, 15 },
        { 4, 11, 22, 8, 8 },
        { 4, 12, 21, 12, 12 },
    };
    (void)maps;
    const uint16_t first_map_by_level[] = { UINT16_MAX, UINT16_MAX, UINT16_MAX, 0, 1 };
    (void)first_map_by_level;
    const int8_t scan_order[] = { 1, 2 };
    (void)scan_order;
    const uint8_t tile_types[] = { 0, 7, 0 };
    (void)tile_types;
    const uint8_t active_teleporters[] = { 0, 0, 0 };
    (void)active_teleporters;
    DM2_V1_LocateOtherLevelResult result;
    (void)result;
    DM2_V1_SystemReceipt receipt;
    (void)receipt;
    int16_t x = 3;
    (void)x;
    int16_t y = 4;
    (void)y;

    assert(dm2_v1_LOCATE_OTHER_LEVEL(
               maps,
               3,
               first_map_by_level,
               5,
               scan_order,
               2,
               tile_types,
               active_teleporters,
               0,
               1,
               &x,
               &y,
               &result,
               &receipt) == 2);
    assert(x == 1);
    assert(y == 3);
    assert(result.valid == 1);
    assert(result.map_index == 2);
    assert(result.scan_index == 1);
    assert(receipt.valid == 1);
    assert(receipt.result == 2);
}

static void test_locate_other_level_fail_closed_for_missing_level(void)
{
    const DM2_V1_MapDefinitionCompat maps[] = {
        { 3, 10, 20, 15, 15 },
    };
    (void)maps;
    const uint16_t first_map_by_level[] = { UINT16_MAX, UINT16_MAX, UINT16_MAX, 0 };
    (void)first_map_by_level;
    DM2_V1_SystemReceipt receipt;
    (void)receipt;
    int16_t x = 3;
    (void)x;
    int16_t y = 4;
    (void)y;

    assert(dm2_v1_LOCATE_OTHER_LEVEL(
               maps,
               1,
               first_map_by_level,
               4,
               0,
               0,
               0,
               0,
               0,
               1,
               &x,
               &y,
               0,
               &receipt) == -1);
    assert(receipt.blocked == 1);
    assert(x == 3);
    assert(y == 4);
}

static void test_guarantee_free_cpxheap_size(void)
{
    const int32_t chunks[] = { 10, 25, 40 };
    (void)chunks;
    DM2_V1_SystemReceipt receipt;
    (void)receipt;
    size_t next = 0;
    (void)next;
    int32_t current_free = 5;
    (void)current_free;

    assert(dm2_v1_GUARANTEE_FREE_CPXHEAP_SIZE(
               30,
               &current_free,
               chunks,
               3,
               &next,
               &receipt) == 1);
    assert(current_free == 40);
    assert(next == 2);
    assert(receipt.valid == 1);
    assert(receipt.result == 40);

    current_free = 5;
    next = 3;
    assert(dm2_v1_GUARANTEE_FREE_CPXHEAP_SIZE(
               30,
               &current_free,
               chunks,
               3,
               &next,
               &receipt) == 0);
    assert(receipt.blocked == 1);
    assert(receipt.result == 5);
}

int main(void)
{
    test_return_1_and_game_ended();
    test_locate_other_level_uses_skproject_scan_and_tile_blockers();
    test_locate_other_level_fail_closed_for_missing_level();
    test_guarantee_free_cpxheap_size();
    return 0;
}
