#include "dm1_v1_floor_feature_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_palette_route_enum(void)
{
    assert(DM1_V1_FLOOR_FEATURE_PALETTE_NATIVE_PC34 == 0);
    assert(DM1_V1_FLOOR_FEATURE_PALETTE_FLOOR_ORNAMENT_D3_PC34 == 1);
    assert(DM1_V1_FLOOR_FEATURE_PALETTE_FLOOR_ORNAMENT_D2_PC34 == 2);
}

static void test_source_struct_layout(void)
{
    DM1_V1_FloorFeatureSourceMaterialPc34 m;
    memset(&m, 0, sizeof(m));
    assert(m.graphicsDatOwned == 0);
    assert(m.graphicIndex == 0);
    assert(m.indexedPixels == NULL);
    assert(m.pixelsFNV1a == 0);
}

static void test_receipt_struct_layout(void)
{
    DM1_V1_FloorFeatureMaterialReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.paletteRoute == 0);
    assert(r.sourcePixelsFNV1a == 0);
}

static void test_fnv1a_null(void)
{
    uint32_t h = DM1_V1_FloorFeatureFNV1aPc34(NULL, 0);
    (void)h;
    assert(h == 0u);
}

static void test_fnv1a_data(void)
{
    unsigned char data[] = {0x41, 0x42, 0x43};
    uint32_t h = DM1_V1_FloorFeatureFNV1aPc34(data, 3);
    (void)h;
    assert(h != 0u);
    uint32_t h2 = DM1_V1_FloorFeatureFNV1aPc34(data, 3);
    (void)h2;
    assert(h == h2);
}

static void test_find_source_null(void)
{
    uint32_t out = 99;
    int r = DM1_V1_FloorFeatureFindSourceMaterialPc34(NULL, 0, 0, 0, 0, 1, 1, &out);
    (void)r;
    assert(r == 0);
    assert(out == 0u);
}

static void test_find_source_no_match(void)
{
    DM1_V1_FloorFeatureSourceMaterialPc34 m;
    memset(&m, 0, sizeof(m));
    uint32_t out = 99;
    int r = DM1_V1_FloorFeatureFindSourceMaterialPc34(&m, 1, 0, 0, 0, 1, 1, &out);
    (void)r;
    assert(r == 0);
}

int main(void)
{
    test_palette_route_enum();
    test_source_struct_layout();
    test_receipt_struct_layout();
    test_fnv1a_null();
    test_fnv1a_data();
    test_find_source_null();
    test_find_source_no_match();

    puts("ok: DM1 floor feature material (Q-DM1-03) 7 tests passed");
    return 0;
}
