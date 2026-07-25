#include "dm1_v1_viewport_planar_fill_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_f0134_fill_rejects_invalid_material(void)
{
    DM1_V1_ViewportPlanarFillMaterialPc34 mat;

    memset(&mat, 0, sizeof(mat));
    assert(dm1_v1_viewport_fill_material_f0134_pc34(NULL, 0) == 0);
    assert(dm1_v1_viewport_fill_material_f0134_pc34(&mat, 0) == 0);

    mat.original_material_verified = 1;
    assert(dm1_v1_viewport_fill_material_f0134_pc34(&mat, 0) == 0);
}

static void test_f0134_fill_admits_valid_material(void)
{
    DM1_V1_ViewportPlanarFillMaterialPc34 mat;
    uint8_t buf[16];

    memset(buf, 0, sizeof(buf));
    memset(&mat, 0, sizeof(mat));
    mat.bitmap = buf;
    mat.bitmap_size = sizeof(buf);
    mat.row_bytes = 8;
    mat.pixel_height = 2;
    mat.original_material_verified = 1;

    assert(dm1_v1_viewport_fill_material_f0134_pc34(&mat, 5) == 1);
}

static void test_f0135_fillbox_rejects_invalid_material(void)
{
    int16_t box[4] = {0, 7, 0, 3};
    (void)box;

    assert(dm1_v1_viewport_fill_material_box_f0135_pc34(NULL, box, 0) == 0);
}

static void test_f0135_fillbox_admits_valid_material(void)
{
    DM1_V1_ViewportPlanarFillMaterialPc34 mat;
    uint8_t buf[16];
    int16_t box[4] = {0, 7, 0, 1};
    (void)box;

    memset(buf, 0, sizeof(buf));
    memset(&mat, 0, sizeof(mat));
    mat.bitmap = buf;
    mat.bitmap_size = sizeof(buf);
    mat.row_bytes = 8;
    mat.pixel_height = 2;
    mat.original_material_verified = 1;

    assert(dm1_v1_viewport_fill_material_box_f0135_pc34(&mat, box, 3) == 1);
}

static void test_source_evidence(void)
{
    const char *ev = dm1_v1_viewport_planar_fill_material_source_evidence_pc34();
    (void)ev;

    assert(ev != NULL);
    assert(strstr(ev, "F0134") != NULL);
    assert(strstr(ev, "F0135") != NULL);
}

int main(void)
{
    test_f0134_fill_rejects_invalid_material();
    test_f0134_fill_admits_valid_material();
    test_f0135_fillbox_rejects_invalid_material();
    test_f0135_fillbox_admits_valid_material();
    test_source_evidence();

    puts("ok: DM1 viewport planar fill material F0134/F0135 callable");
    return 0;
}
