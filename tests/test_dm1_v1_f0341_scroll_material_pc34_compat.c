#include "dm1_v1_f0341_scroll_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    assert(DM1_V1_F0341_C023_PANEL_PC34 == 23);
    assert(DM1_V1_F0341_M653_PC34 == 695);
    assert(DM1_V1_F0341_M653_LEGACY_PC34 == 557);
    assert(DM1_V1_F0341_M653_BYTES_PC34 == 768);
}

static void test_source_struct(void)
{
    DM1_V1_F0341SourceSurfacePc34 s;
    memset(&s, 0, sizeof(s));
    assert(s.graphicsDatOwned == 0);
    assert(s.indexedPixels == NULL);
    assert(s.pixelsFNV1a == 0);
}

static void test_glyph_struct(void)
{
    DM1_V1_F0341GlyphSourcePc34 g;
    memset(&g, 0, sizeof(g));
    assert(g.bits == NULL);
    assert(g.bitsFNV1a == 0);
}

static void test_receipt_struct(void)
{
    DM1_V1_F0341ScrollMaterialReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.suppressSyntheticFallback == 0);
    assert(r.materialFingerprint == 0);
}

static void test_fnv1a_null(void)
{
    uint32_t h = dm1_v1_f0341_scroll_material_fnv1a_pc34(NULL, 0);
    (void)h;
    assert(h == 0u);
}

static void test_fnv1a_data(void)
{
    unsigned char data[] = {0x10, 0x20, 0x30};
    uint32_t h = dm1_v1_f0341_scroll_material_fnv1a_pc34(data, 3);
    (void)h;
    assert(h != 0u);
}

static void test_receipt_empty(void)
{
    DM1_V1_F0341ScrollMaterialReceiptPc34 r;
    int ok = dm1_v1_f0341_scroll_material_receipt_pc34(NULL, 0, NULL, 0, &r);
    (void)ok;
    assert(ok == 0);
}

int main(void)
{
    test_constants();
    test_source_struct();
    test_glyph_struct();
    test_receipt_struct();
    test_fnv1a_null();
    test_fnv1a_data();
    test_receipt_empty();

    puts("ok: DM1 F0341 scroll material (Q-DM1-06) 7 tests passed");
    return 0;
}
