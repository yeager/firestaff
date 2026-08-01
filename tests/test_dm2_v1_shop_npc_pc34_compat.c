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

int main(void)
{
    printf("test_dm2_v1_shop_npc_pc34_compat:\n");
    test_null_safety();
    test_shop_panel();
    test_shop_floor();
    test_merchant();
    test_merchant_guard();
    test_not_shop();
    printf("All shop/NPC tests passed.\n");
    return 0;
}
