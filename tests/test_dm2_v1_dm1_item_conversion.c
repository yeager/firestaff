#include "dm2_v1_dm1_item_conversion.h"
#include <assert.h>
#include <stdio.h>

static void test_compass(void)
{
    DM2_V1_DM1ItemConvReceipt r;
    for (int i = 0; i < 4; i++) {
        assert(dm2_v1_dm1_item_conv_lookup(i, &r));
        assert(r.valid);
        assert(r.item_db == DM2_V1_DB_CATEGORY_MISC_ITEM);
        assert(r.item_id == 5);
    }
}

static void test_torch(void)
{
    DM2_V1_DM1ItemConvReceipt r;
    for (int i = 4; i < 8; i++) {
        assert(dm2_v1_dm1_item_conv_lookup(i, &r));
        assert(r.item_db == DM2_V1_DB_CATEGORY_WEAPON);
        assert(r.item_id == 2);
    }
}

static void test_firestaff(void)
{
    DM2_V1_DM1ItemConvReceipt r;
    assert(dm2_v1_dm1_item_conv_lookup(27, &r));
    assert(r.item_db == DM2_V1_DB_CATEGORY_WEAPON);
    assert(r.item_id == 45);
}

static void test_scrolls(void)
{
    DM2_V1_DM1ItemConvReceipt r;
    assert(dm2_v1_dm1_item_conv_lookup(30, &r));
    assert(r.item_db == DM2_V1_DB_CATEGORY_SCROLL);
    assert(r.item_id == 0);
}

static void test_dagger(void)
{
    DM2_V1_DM1ItemConvReceipt r;
    assert(dm2_v1_dm1_item_conv_lookup(32, &r));
    assert(r.item_db == DM2_V1_DB_CATEGORY_WEAPON);
    assert(r.item_id == 8);
}

static void test_potions(void)
{
    DM2_V1_DM1ItemConvReceipt r;
    for (int i = 0; i < 20; i++) {
        assert(dm2_v1_dm1_item_conv_lookup(148 + i, &r));
        assert(r.item_db == DM2_V1_DB_CATEGORY_POTION);
        assert(r.item_id == i);
    }
}

static void test_keys(void)
{
    DM2_V1_DM1ItemConvReceipt r;
    for (int i = 0; i < 16; i++) {
        assert(dm2_v1_dm1_item_conv_lookup(176 + i, &r));
        assert(r.item_db == DM2_V1_DB_CATEGORY_MISC_ITEM);
        assert(r.item_id == 9 + i);
    }
}

static void test_special_items(void)
{
    DM2_V1_DM1ItemConvReceipt r;
    assert(dm2_v1_dm1_item_conv_lookup(140, &r));
    assert(r.item_db == DM2_V1_DB_CATEGORY_CLOTHING);
    assert(r.item_id == 53);

    assert(dm2_v1_dm1_item_conv_lookup(144, &r));
    assert(r.item_db == DM2_V1_DB_CATEGORY_CONTAINER);
    assert(r.item_id == 0);

    assert(dm2_v1_dm1_item_conv_lookup(197, &r));
    assert(r.item_db == DM2_V1_DB_CATEGORY_MISC_ITEM);
    assert(r.item_id == 51);
}

static void test_out_of_bounds(void)
{
    DM2_V1_DM1ItemConvReceipt r;
    assert(!dm2_v1_dm1_item_conv_lookup(-1, &r));
    assert(!r.valid);
    assert(!dm2_v1_dm1_item_conv_lookup(199, &r));
    assert(!r.valid);
    assert(!dm2_v1_dm1_item_conv_lookup(0, NULL));
}

static void test_table_size(void)
{
    assert(DM2_V1_DM1_ITEM_CONV_TABLE_SIZE == 199);
    assert(dm2_v1_dm1_item_conv_table[198].item_db == DM2_V1_DB_CATEGORY_MISC_ITEM);
    assert(dm2_v1_dm1_item_conv_table[198].item_id == 52);
}

int main(void)
{
    test_compass();
    test_torch();
    test_firestaff();
    test_scrolls();
    test_dagger();
    test_potions();
    test_keys();
    test_special_items();
    test_out_of_bounds();
    test_table_size();
    assert(dm2_v1_dm1_item_conversion_source_evidence() != NULL);
    printf("All dm2_v1_dm1_item_conversion tests passed.\n");
    return 0;
}
