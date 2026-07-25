#include "dm1_v1_champion_panel_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_receipt_struct(void)
{
    Dm1V1ChampionPanelMaterialReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.suppressSyntheticFallback == 0);
    assert(r.m653GraphicIndex == 0);
    assert(r.materialFingerprint == 0);
}

static void test_fnv1a_null(void)
{
    uint32_t h = dm1_v1_champion_panel_material_fnv1a_pc34(NULL, 0);
    (void)h;
    assert(h == 0u);
}

static void test_fnv1a_data(void)
{
    unsigned char data[] = {0x11, 0x22};
    uint32_t h = dm1_v1_champion_panel_material_fnv1a_pc34(data, 2);
    (void)h;
    assert(h != 0u);
}

static void test_from_loader_null_receipt(void)
{
    int ok = dm1_v1_champion_panel_material_from_m11_loader_pc34(
        NULL, NULL, NULL, 0, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_from_loader_null_loader(void)
{
    Dm1V1ChampionPanelMaterialReceiptPc34 r;
    int ok = dm1_v1_champion_panel_material_from_m11_loader_pc34(
        NULL, NULL, NULL, 0, &r);
    (void)ok;
    assert(ok == 0);
    assert(r.valid == 0);
}

int main(void)
{
    test_receipt_struct();
    test_fnv1a_null();
    test_fnv1a_data();
    test_from_loader_null_receipt();
    test_from_loader_null_loader();

    puts("ok: DM1 champion panel material (Q-DM1-07) 5 tests passed");
    return 0;
}
