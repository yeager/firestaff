#include "dm1_v1_f0663_smoke_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_V1_F0663_C488_POISON_SOURCE_PC34 == 488);
    assert(DM1_V1_F0663_C498_SMOKE_PATTERN_SMALL_PC34 == 498);
    assert(DM1_V1_F0663_C499_SMOKE_PATTERN_MEDIUM_PC34 == 499);
    assert(DM1_V1_F0663_C500_SMOKE_PATTERN_LARGE_PC34 == 500);
    assert(DM1_V1_F0663_SURFACE_COUNT_PC34 == 4);
    assert(DM1_V1_F0663_PALETTE_COUNT_PC34 == 16);
}
static void test_receipt_struct(void) {
    DM1_V1_F0663SmokeMaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.materialFingerprint == 0);
}
static void test_palette_changes(void) {
    const unsigned char* pc = dm1_v1_f0663_smoke_palette_changes_pc34();
    assert(pc != NULL);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0663_smoke_material_fnv1a_pc34(NULL, 0); (void)h; assert(h == 0u);
}
static void test_receipt_null(void) {
    DM1_V1_F0663SmokeMaterialReceiptPc34 r;
    int ok = dm1_v1_f0663_smoke_material_receipt_pc34(NULL, 0, NULL, 0, &r); (void)ok; assert(ok == 0);
}
static void test_single_surface_receipt(void) {
    unsigned char pixels[] = { 6, 7, 12, 1 };
    DM1_V1_F0663SourceSurfacePc34 surface;
    DM1_V1_F0663SmokeSurfaceReceiptPc34 receipt;
    const unsigned char* palette = dm1_v1_f0663_smoke_palette_changes_pc34();

    memset(&surface, 0, sizeof(surface));
    surface.graphicsDatOwned = 1;
    surface.graphicIndex = DM1_V1_F0663_C488_POISON_SOURCE_PC34;
    surface.width = 2;
    surface.height = 2;
    surface.indexedPixelCount = 4;
    surface.indexedPixels = pixels;
    surface.pixelsFNV1a = dm1_v1_f0663_smoke_material_fnv1a_pc34(pixels, 4);
    assert(dm1_v1_f0663_smoke_surface_receipt_pc34(
        &surface, DM1_V1_F0663_C488_POISON_SOURCE_PC34, palette,
        DM1_V1_F0663_PALETTE_COUNT_PC34, &receipt));
    assert(receipt.valid && receipt.suppressSyntheticFallback);
    assert(receipt.replacementSourceA == 6 &&
           receipt.replacementDestinationA == 12);
    surface.graphicIndex = 497;
    assert(!dm1_v1_f0663_smoke_surface_receipt_pc34(
        &surface, 497, palette, DM1_V1_F0663_PALETTE_COUNT_PC34, &receipt));
}
int main(void) {
    test_constants(); test_receipt_struct(); test_palette_changes(); test_fnv1a_null(); test_receipt_null(); test_single_surface_receipt();
    puts("ok: DM1 F0663 smoke material (Q-DM1-03) 6 tests passed"); return 0;
}
