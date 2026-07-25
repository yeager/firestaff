#include "dm1_v1_door_source_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_fnv1a_null(void)
{
    uint32_t h = DM1_V1_DoorSourcePixelsFNV1aPc34(NULL, 0);
    (void)h;
    assert(h == 0u);
}

static void test_fnv1a_zero_count(void)
{
    uint8_t data[4] = {1, 2, 3, 4};
    uint32_t h = DM1_V1_DoorSourcePixelsFNV1aPc34(data, 0);
    (void)h;
    assert(h == 0u);
}

static void test_fnv1a_deterministic(void)
{
    uint8_t data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint32_t h1 = DM1_V1_DoorSourcePixelsFNV1aPc34(data, 4);
    uint32_t h2 = DM1_V1_DoorSourcePixelsFNV1aPc34(data, 4);
    (void)h1; (void)h2;
    assert(h1 == h2);
    assert(h1 != 0u);
}

static void test_fnv1a_different_data(void)
{
    uint8_t a[4] = {1, 2, 3, 4};
    uint8_t b[4] = {4, 3, 2, 1};
    uint32_t ha = DM1_V1_DoorSourcePixelsFNV1aPc34(a, 4);
    uint32_t hb = DM1_V1_DoorSourcePixelsFNV1aPc34(b, 4);
    (void)ha; (void)hb;
    assert(ha != hb);
}

static void test_material_for_blit_null_materials(void)
{
    DM1_V1_DoorSourceBlitPc34 blit = {0, 0, 0, 10, 10};
    uint32_t hash = 0;
    int ok = DM1_V1_DoorSourceMaterialForBlitPc34(NULL, 0, &blit, &hash);
    (void)ok;
    assert(ok == 0);
}

static void test_material_for_blit_null_blit(void)
{
    DM1_V1_DoorSourceMaterialPc34 mat;
    memset(&mat, 0, sizeof(mat));
    uint32_t hash = 0;
    int ok = DM1_V1_DoorSourceMaterialForBlitPc34(&mat, 1, NULL, &hash);
    (void)ok;
    assert(ok == 0);
}

static void test_material_for_blit_bad_dims(void)
{
    DM1_V1_DoorSourceMaterialPc34 mat;
    memset(&mat, 0, sizeof(mat));
    DM1_V1_DoorSourceBlitPc34 blit = {0, 0, 0, 0, 0};
    int ok = DM1_V1_DoorSourceMaterialForBlitPc34(&mat, 1, &blit, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_material_for_blit_match(void)
{
    uint8_t pixels[16];
    memset(pixels, 0x42, sizeof(pixels));
    uint32_t expected_hash = DM1_V1_DoorSourcePixelsFNV1aPc34(pixels, 16);

    DM1_V1_DoorSourceMaterialPc34 mat;
    mat.graphicsDatOwned = 1;
    mat.graphicIndex = 5;
    mat.width = 16;
    mat.height = 1;
    mat.pixels = pixels;
    mat.pixelByteCount = 16;
    mat.pixelsFNV1a = expected_hash;

    DM1_V1_DoorSourceBlitPc34 blit = {5, 0, 0, 8, 1};
    uint32_t out_hash = 0;
    int ok = DM1_V1_DoorSourceMaterialForBlitPc34(&mat, 1, &blit, &out_hash);
    (void)ok;
    assert(ok == 1);
    assert(out_hash == expected_hash);
}

static void test_material_for_blit_wrong_graphic(void)
{
    uint8_t pixels[16];
    memset(pixels, 0x42, sizeof(pixels));
    uint32_t h = DM1_V1_DoorSourcePixelsFNV1aPc34(pixels, 16);

    DM1_V1_DoorSourceMaterialPc34 mat;
    mat.graphicsDatOwned = 1;
    mat.graphicIndex = 5;
    mat.width = 16;
    mat.height = 1;
    mat.pixels = pixels;
    mat.pixelByteCount = 16;
    mat.pixelsFNV1a = h;

    DM1_V1_DoorSourceBlitPc34 blit = {99, 0, 0, 8, 1};
    int ok = DM1_V1_DoorSourceMaterialForBlitPc34(&mat, 1, &blit, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_material_for_blit_not_owned(void)
{
    uint8_t pixels[16];
    memset(pixels, 0x42, sizeof(pixels));

    DM1_V1_DoorSourceMaterialPc34 mat;
    mat.graphicsDatOwned = 0;
    mat.graphicIndex = 5;
    mat.width = 16;
    mat.height = 1;
    mat.pixels = pixels;
    mat.pixelByteCount = 16;
    mat.pixelsFNV1a = DM1_V1_DoorSourcePixelsFNV1aPc34(pixels, 16);

    DM1_V1_DoorSourceBlitPc34 blit = {5, 0, 0, 8, 1};
    int ok = DM1_V1_DoorSourceMaterialForBlitPc34(&mat, 1, &blit, NULL);
    (void)ok;
    assert(ok == 0);
}

int main(void)
{
    test_fnv1a_null();
    test_fnv1a_zero_count();
    test_fnv1a_deterministic();
    test_fnv1a_different_data();
    test_material_for_blit_null_materials();
    test_material_for_blit_null_blit();
    test_material_for_blit_bad_dims();
    test_material_for_blit_match();
    test_material_for_blit_wrong_graphic();
    test_material_for_blit_not_owned();

    puts("ok: DM1 door source material (Q-DM1-03) 10 tests passed");
    return 0;
}
