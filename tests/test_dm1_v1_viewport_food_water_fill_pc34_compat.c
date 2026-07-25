#include "dm1_v1_viewport_planar_fill_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM1_V1_ViewportPlanarFillMaterialPc34 make_mat(uint8_t *buf, size_t sz)
{
    DM1_V1_ViewportPlanarFillMaterialPc34 mat;

    memset(&mat, 0, sizeof(mat));
    memset(buf, 0, sz);
    mat.bitmap = buf;
    mat.bitmap_size = sz;
    mat.row_bytes = 8;
    mat.pixel_height = sz / 8;
    mat.original_material_verified = 1;
    return mat;
}

static void test_null_material_rejected(void)
{
    int16_t box[4] = {0, 31, 0, 3};
    (void)box;
    DM1_V1_ViewportFoodWaterFillReceiptPc34 receipt;
    (void)receipt;

    assert(dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
        NULL, box, 1024, 4, &receipt) == 0);
}

static void test_invalid_box_rejected(void)
{
    uint8_t buf[64];
    DM1_V1_ViewportPlanarFillMaterialPc34 mat = make_mat(buf, sizeof(buf));
    (void)mat;
    int16_t bad_box[4] = {10, 5, 0, 3};
    (void)bad_box;

    assert(dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
        &mat, bad_box, 1024, 4, NULL) == 0);
}

static void test_full_bar_at_max_amount(void)
{
    uint8_t buf[256];
    DM1_V1_ViewportPlanarFillMaterialPc34 mat = make_mat(buf, sizeof(buf));
    (void)mat;
    int16_t box[4] = {0, 31, 0, 3};
    (void)box;
    DM1_V1_ViewportFoodWaterFillReceiptPc34 receipt;

    memset(&receipt, 0, sizeof(receipt));
    assert(dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
        &mat, box, 2048, 4, &receipt) == 1);
    assert(receipt.drawn == 1);
    assert(receipt.proportional_units == 10000);
    assert(receipt.bar_color == 4);
}

static void test_negative_amount_shows_yellow(void)
{
    uint8_t buf[256];
    DM1_V1_ViewportPlanarFillMaterialPc34 mat = make_mat(buf, sizeof(buf));
    (void)mat;
    int16_t box[4] = {0, 31, 0, 3};
    (void)box;
    DM1_V1_ViewportFoodWaterFillReceiptPc34 receipt;

    memset(&receipt, 0, sizeof(receipt));
    assert(dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
        &mat, box, -100, 4, &receipt) == 1);
    assert(receipt.drawn == 1);
    assert(receipt.bar_color == 11);
}

static void test_very_negative_amount_shows_red(void)
{
    uint8_t buf[256];
    DM1_V1_ViewportPlanarFillMaterialPc34 mat = make_mat(buf, sizeof(buf));
    (void)mat;
    int16_t box[4] = {0, 31, 0, 3};
    (void)box;
    DM1_V1_ViewportFoodWaterFillReceiptPc34 receipt;

    memset(&receipt, 0, sizeof(receipt));
    assert(dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
        &mat, box, -600, 4, &receipt) == 1);
    assert(receipt.drawn == 1);
    assert(receipt.bar_color == 8);
}

static void test_shadow_box_offset(void)
{
    uint8_t buf[256];
    DM1_V1_ViewportPlanarFillMaterialPc34 mat = make_mat(buf, sizeof(buf));
    (void)mat;
    int16_t box[4] = {10, 41, 5, 8};
    (void)box;
    DM1_V1_ViewportFoodWaterFillReceiptPc34 receipt;

    memset(&receipt, 0, sizeof(receipt));
    assert(dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
        &mat, box, 2048, 4, &receipt) == 1);
    assert(receipt.shadow_box[0] == receipt.bar_box[0] + 2);
    assert(receipt.shadow_box[1] == receipt.bar_box[1] + 2);
    assert(receipt.shadow_box[2] == receipt.bar_box[2] + 2);
    assert(receipt.shadow_box[3] == receipt.bar_box[3] + 2);
}

static void test_out_of_domain_rejected(void)
{
    uint8_t buf[256];
    DM1_V1_ViewportPlanarFillMaterialPc34 mat = make_mat(buf, sizeof(buf));
    (void)mat;
    int16_t box[4] = {0, 31, 0, 3};
    (void)box;

    assert(dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
        &mat, box, 3000, 4, NULL) == 0);
    assert(dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
        &mat, box, -1025, 4, NULL) == 0);
}

int main(void)
{
    test_null_material_rejected();
    test_invalid_box_rejected();
    test_full_bar_at_max_amount();
    test_negative_amount_shows_yellow();
    test_very_negative_amount_shows_red();
    test_shadow_box_offset();
    test_out_of_domain_rejected();

    puts("ok: DM1 F0344 food/water fill bar callable");
    return 0;
}
