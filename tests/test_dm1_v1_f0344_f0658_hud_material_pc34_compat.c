#include "dm1_v1_f0344_f0658_hud_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_fnv1a_null(void)
{
    uint32_t h = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(NULL, 0);
    (void)h;
    assert(h == 0);
}

static void test_fnv1a_deterministic(void)
{
    unsigned char data[] = { 0x41, 0x42, 0x43 };
    uint32_t h1 = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(data, 3);
    uint32_t h2 = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(data, 3);
    (void)h1;
    assert(h1 == h2);
    assert(h1 != 0);
}

static void test_fnv1a_different_input(void)
{
    unsigned char a[] = { 0x01 };
    unsigned char b[] = { 0x02 };
    uint32_t ha = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(a, 1);
    uint32_t hb = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(b, 1);
    (void)ha;
    assert(ha != hb);
}

static void test_receipt_null_rejected(void)
{
    DM1_V1_F0344F0658HudMaterialReceiptPc34 receipt;
    int rc;

    memset(&receipt, 0, sizeof(receipt));
    rc = dm1_v1_f0344_f0658_hud_material_receipt_pc34(
        NULL, 0, NULL, 0, &receipt);
    (void)rc;
    assert(rc == 0);

    rc = dm1_v1_f0344_f0658_hud_material_receipt_pc34(
        NULL, 0, NULL, 0, NULL);
    assert(rc == 0);
}

static void test_receipt_empty_surfaces_rejected(void)
{
    DM1_V1_HudSourceSurfacePc34 surfaces[1];
    DM1_V1_HudGlyphSourcePc34 glyphs[1];
    DM1_V1_F0344F0658HudMaterialReceiptPc34 receipt;
    int rc;

    memset(surfaces, 0, sizeof(surfaces));
    memset(glyphs, 0, sizeof(glyphs));
    memset(&receipt, 0, sizeof(receipt));
    rc = dm1_v1_f0344_f0658_hud_material_receipt_pc34(
        surfaces, 0, glyphs, 0, &receipt);
    (void)rc;
    assert(rc == 0);
}

static void test_receipt_unowned_surfaces_rejected(void)
{
    DM1_V1_HudSourceSurfacePc34 surfaces[1];
    DM1_V1_HudGlyphSourcePc34 glyphs[1];
    DM1_V1_F0344F0658HudMaterialReceiptPc34 receipt;
    int rc;

    memset(surfaces, 0, sizeof(surfaces));
    memset(glyphs, 0, sizeof(glyphs));
    memset(&receipt, 0, sizeof(receipt));
    surfaces[0].graphicsDatOwned = 0;
    rc = dm1_v1_f0344_f0658_hud_material_receipt_pc34(
        surfaces, 1, glyphs, 1, &receipt);
    (void)rc;
    assert(rc == 0);
}

int main(void)
{
    test_fnv1a_null();
    test_fnv1a_deterministic();
    test_fnv1a_different_input();
    test_receipt_null_rejected();
    test_receipt_empty_surfaces_rejected();
    test_receipt_unowned_surfaces_rejected();

    puts("ok: DM1 F0344/F0658 HUD material (Q-DM1-07) 6 tests passed");
    return 0;
}
