#include "dm1_v1_f0682_transparent_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_surface_struct(void)
{
    DM1_V1_F0682TransparentSurfacePc34 s;
    memset(&s, 0, sizeof(s));
    assert(s.graphicsDatOwned == 0);
    assert(s.graphicIndex == 0);
    assert(s.width == 0);
    assert(s.height == 0);
    assert(s.indexedPixelCount == 0);
    assert(s.indexedPixels == NULL);
    assert(s.pixelsFNV1a == 0);
}

static void test_receipt_struct(void)
{
    DM1_V1_F0682TransparentMaterialReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.suppressSyntheticFallback == 0);
    assert(r.graphicIndex == 0);
    assert(r.transparentColor == 0);
    assert(r.paletteChangeCount == 0);
    assert(r.sourceFingerprint == 0);
    assert(r.paletteFingerprint == 0);
}

static void test_fnv1a_null(void)
{
    uint32_t h = dm1_v1_f0682_transparent_material_fnv1a_pc34(NULL, 0);
    (void)h;
    assert(h == 0u);
}

static void test_fnv1a_data(void)
{
    unsigned char data[] = {0xAA, 0xBB, 0xCC};
    uint32_t h = dm1_v1_f0682_transparent_material_fnv1a_pc34(data, 3);
    (void)h;
    assert(h != 0u);
}

static void test_receipt_null_out(void)
{
    int ok = dm1_v1_f0682_transparent_material_receipt_pc34(NULL, 0, NULL, 0, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_receipt_null_surface(void)
{
    DM1_V1_F0682TransparentMaterialReceiptPc34 r;
    int ok = dm1_v1_f0682_transparent_material_receipt_pc34(NULL, 0, NULL, 0, &r);
    (void)ok;
    assert(ok == 0);
    assert(r.valid == 0);
}

int main(void)
{
    test_surface_struct();
    test_receipt_struct();
    test_fnv1a_null();
    test_fnv1a_data();
    test_receipt_null_out();
    test_receipt_null_surface();

    puts("ok: DM1 F0682 transparent material (Q-DM1-03) 6 tests passed");
    return 0;
}
