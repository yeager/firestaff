#include "dm1_v1_viewport_wall_field_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_provenance_null_rejected(void)
{
    assert(dm1_v1_viewport_dungeon_provenance_is_valid_pc34(NULL) == 0);
}

static void test_provenance_empty_rejected(void)
{
    DM1_V1_ViewportDungeonProvenancePc34 prov;

    memset(&prov, 0, sizeof(prov));
    assert(dm1_v1_viewport_dungeon_provenance_is_valid_pc34(&prov) == 0);
}

static void test_provenance_unowned_rejected(void)
{
    unsigned char raw[4] = {0xAB, 0xCD, 0xEF, 0x01};
    DM1_V1_ViewportDungeonProvenancePc34 prov;

    memset(&prov, 0, sizeof(prov));
    prov.dungeonDatOwned = 0;
    prov.rawBytes = raw;
    prov.rawByteCount = 4;
    prov.squareByteOffset = 0;
    prov.squareByte = 0xAB;
    assert(dm1_v1_viewport_dungeon_provenance_is_valid_pc34(&prov) == 0);
}

static void test_wall_receipt_null_rejected(void)
{
    assert(dm1_v1_viewport_wall_original_material_receipt_pc34(
        NULL, NULL, 0, NULL, 0, NULL) == 0);
}

static void test_wall_receipt_invalid_wall_rejected(void)
{
    DM1_ViewportSideWallHostReceiptPc34 wall;
    DM1_V1_ViewportWallFieldMaterialReceiptPc34 receipt;

    memset(&wall, 0, sizeof(wall));
    memset(&receipt, 0, sizeof(receipt));
    assert(dm1_v1_viewport_wall_original_material_receipt_pc34(
        &wall, NULL, 0, NULL, 0, &receipt) == 0);
    assert(receipt.valid == 0);
}

static void test_wall_ornament_null_rejected(void)
{
    assert(dm1_v1_viewport_wall_ornament_original_material_receipt_pc34(
        0, 0, 0, NULL, 0, NULL, 0, NULL) == 0);
}

static void test_wall_ornament_index_zero_rejected(void)
{
    DM1_V1_ViewportWallFieldMaterialReceiptPc34 receipt;

    memset(&receipt, 0, sizeof(receipt));
    assert(dm1_v1_viewport_wall_ornament_original_material_receipt_pc34(
        0, 0, 100, NULL, 0, NULL, 0, &receipt) == 0);
    assert(receipt.valid == 0);
}

static void test_field_receipt_null_rejected(void)
{
    assert(dm1_v1_viewport_field_original_material_receipt_pc34(
        NULL, NULL, 0, NULL, 0, NULL) == 0);
}

static void test_field_receipt_null_provenance_rejected(void)
{
    DM1_FieldRenderPlanPc34 plan;
    DM1_V1_ViewportWallFieldMaterialReceiptPc34 receipt;

    memset(&plan, 0, sizeof(plan));
    memset(&receipt, 0, sizeof(receipt));
    assert(dm1_v1_viewport_field_original_material_receipt_pc34(
        &plan, NULL, 0, NULL, 0, &receipt) == 0);
    assert(receipt.valid == 0);
}

int main(void)
{
    test_provenance_null_rejected();
    test_provenance_empty_rejected();
    test_provenance_unowned_rejected();
    test_wall_receipt_null_rejected();
    test_wall_receipt_invalid_wall_rejected();
    test_wall_ornament_null_rejected();
    test_wall_ornament_index_zero_rejected();
    test_field_receipt_null_rejected();
    test_field_receipt_null_provenance_rejected();

    puts("ok: DM1 viewport wall/field material receipt rejection gates callable");
    return 0;
}
