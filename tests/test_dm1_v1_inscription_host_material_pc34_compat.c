#include "dm1_v1_inscription_host_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_max_glyphs_constant(void)
{
    int v = DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34;
    assert(v == 128);
    (void)v;
}

static void test_receipt_struct_layout(void)
{
    DM1_V1_InscriptionHostMaterialReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    assert(receipt.valid == 0);
    assert(receipt.glyphByteCount == 0);
}

static void test_raster_cell_binding_null(void)
{
    DM1_V1_InscriptionRasterCellBindingPc34 binding;
    int result = DM1_V1_InscriptionBuildRasterCellBindingPc34(NULL, 0, 0, &binding);
    assert(result == 0);
    (void)result;
}

static void test_raster_cell_binding_bad_line(void)
{
    DM1_V1_InscriptionHostMaterialReceiptPc34 receipt;
    DM1_V1_InscriptionRasterCellBindingPc34 binding;
    int result;
    memset(&receipt, 0, sizeof(receipt));
    result = DM1_V1_InscriptionBuildRasterCellBindingPc34(&receipt, -1, 0, &binding);
    assert(result == 0);
    (void)result;
}

static void test_source_glyph_layout_gate_null(void)
{
    int result = DM1_V1_InscriptionSourceGlyphLayoutGatePc34(NULL, 0, 0, NULL);
    assert(result == 0);
    (void)result;
}

static void test_raster_gate_null(void)
{
    int result = DM1_V1_InscriptionHostMaterialRasterGatePc34(NULL, 0, 0);
    assert(result == 0);
    (void)result;
}

static void test_capture_source_raster_null(void)
{
    int result = DM1_V1_InscriptionCaptureSourceRasterPc34(NULL, NULL, 0, 0, NULL);
    assert(result == 0);
    (void)result;
}

int main(void)
{
    test_max_glyphs_constant();
    test_receipt_struct_layout();
    test_raster_cell_binding_null();
    test_raster_cell_binding_bad_line();
    test_source_glyph_layout_gate_null();
    test_raster_gate_null();
    test_capture_source_raster_null();
    puts("ok: DM1 inscription host material (Q-DM1-03) 7 tests passed");
    return 0;
}
