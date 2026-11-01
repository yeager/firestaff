#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_shop_npc_pc34_compat.h"

static void test_null_safety(void)
{
    int r = dm2_v1_classify_shop_element(0, 0, NULL);
    assert(r == 0);
    printf("  PASS: null_safety\n");
}

static void test_shop_panel(void)
{
    DM2_V1_ShopClassification c;
    int r = dm2_v1_classify_shop_element(DM2_ACTUATOR_TYPE_SHOP_PANEL, -1, &c);
    assert(r == 1);
    assert(c.is_shop_panel == 1);
    assert(c.is_shop_floor == 0);
    printf("  PASS: shop_panel\n");
}

static void test_shop_floor(void)
{
    DM2_V1_ShopClassification c;
    int r = dm2_v1_classify_shop_element(DM2_ACTUATOR_FLOOR_TYPE_SHOP, -1, &c);
    assert(r == 1);
    assert(c.is_shop_floor == 1);
    printf("  PASS: shop_floor\n");
}

static void test_merchant(void)
{
    DM2_V1_ShopClassification c;
    int r = dm2_v1_classify_shop_element(-1, DM2_AI_REF_MERCHANT, &c);
    assert(r == 1);
    assert(c.is_merchant_npc == 1);
    printf("  PASS: merchant\n");
}

static void test_merchant_guard(void)
{
    DM2_V1_ShopClassification c;
    int r = dm2_v1_classify_shop_element(-1, DM2_AI_REF_MERCHANT_GUARD, &c);
    assert(r == 1);
    assert(c.is_merchant_guard == 1);
    printf("  PASS: merchant_guard\n");
}

static void test_not_shop(void)
{
    DM2_V1_ShopClassification c;
    int r = dm2_v1_classify_shop_element(0x01, 5, &c);
    assert(r == 0);
    assert(c.is_shop_panel == 0);
    assert(c.is_merchant_npc == 0);
    printf("  PASS: not_shop\n");
}

static void set_mask_bit(uint8_t mask[64], uint16_t item)
{
    mask[item >> 3] |= (uint8_t)(1u << (item & 7u));
}

static int reject_item_three(uint16_t item, uint16_t raw_entry, void *user)
{
    (void)raw_entry;
    (void)user;
    return item == 3u ? 0 : 1;
}

static void test_source_stock_cycle(void)
{
    const uint8_t list[] = { 0x02u, 0x80u, 3u, 0u, 4u, 0u };
    uint8_t mask[64] = { 0 };
    DM2_V1_ShopStockSelection selection;

    set_mask_bit(mask, 4u);
    assert(dm2_v1_shop_select_masked_stock(
        list, 3u, 0, 2u, mask, 1, NULL, NULL, &selection) == 1);
    assert(selection.item == 4u);
    assert(selection.list_index == 2u);
    assert(selection.raw_list_entry == 4u);
    assert(selection.from_list == 1u);

    memset(mask, 0, sizeof(mask));
    set_mask_bit(mask, 3u);
    set_mask_bit(mask, 4u);
    assert(dm2_v1_shop_select_masked_stock(
        list, 3u, 0, 2u, mask, 1, reject_item_three, NULL,
        &selection) == 1);
    assert(selection.item == 4u);

    memset(mask, 0, sizeof(mask));
    set_mask_bit(mask, 2u);
    assert(dm2_v1_shop_select_masked_stock(
        list, 3u, 0, 3u, mask, -1, NULL, NULL, &selection) == 1);
    assert(selection.item == 2u);
    assert(selection.list_index == 0u);
    assert(selection.raw_list_entry == 0x8002u);
    {
        const uint8_t big_endian_list[] = {
            0x80u, 0x02u, 0x00u, 0x03u, 0x00u, 0x04u
        };
        memset(mask, 0, sizeof(mask));
        set_mask_bit(mask, 2u);
        assert(dm2_v1_shop_select_masked_stock(
            big_endian_list, 3u, 1, 3u, mask, -1, NULL, NULL,
            &selection) == 1);
        assert(selection.item == 2u);
        assert(selection.raw_list_entry == 0x8002u);
    }
    printf("  PASS: source_stock_cycle\n");
}

static void test_source_stock_listless_and_empty(void)
{
    uint8_t mask[64] = { 0 };
    DM2_V1_ShopStockSelection selection;

    set_mask_bit(mask, 0u);
    assert(dm2_v1_shop_select_masked_stock(
        NULL, 0u, 0, 511u, mask, 1, NULL, NULL, &selection) == 1);
    assert(selection.item == 0u);
    assert(selection.list_index == 0u);
    assert(selection.from_list == 0u);
    memset(mask, 0, sizeof(mask));
    assert(dm2_v1_shop_select_masked_stock(
        NULL, 0u, 0, 0u, mask, 1, NULL, NULL, &selection) == 0);
    assert(dm2_v1_shop_select_masked_stock(
        NULL, 0u, 0, 0u, mask, 1, NULL, NULL, NULL) == 0);
    printf("  PASS: source_stock_listless_and_empty\n");
}

int main(void)
{
    printf("test_dm2_v1_shop_npc_pc34_compat:\n");
    test_null_safety();
    test_shop_panel();
    test_shop_floor();
    test_merchant();
    test_merchant_guard();
    test_not_shop();
    test_source_stock_cycle();
    test_source_stock_listless_and_empty();
    printf("All shop/NPC tests passed.\n");
    return 0;
}
