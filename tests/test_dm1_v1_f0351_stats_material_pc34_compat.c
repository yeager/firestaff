#include "dm1_v1_f0351_stats_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_source_struct(void)
{
    DM1_V1_F0351SourceSurfacePc34 s;
    memset(&s, 0, sizeof(s));
    assert(s.graphicsDatOwned == 0);
    assert(s.indexedPixels == NULL);
    assert(s.pixelsFNV1a == 0);
}

static void test_glyph_struct(void)
{
    DM1_V1_F0351GlyphSourcePc34 g;
    memset(&g, 0, sizeof(g));
    assert(g.bits == NULL);
    assert(g.bitsFNV1a == 0);
}

static void test_receipt_struct(void)
{
    DM1_V1_F0351StatsMaterialReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.suppressSyntheticFallback == 0);
    assert(r.materialFingerprint == 0);
}

static void test_fnv1a_null(void)
{
    uint32_t h = dm1_v1_f0351_stats_material_fnv1a_pc34(NULL, 0);
    (void)h;
    assert(h == 0u);
}

static void test_fnv1a_data(void)
{
    unsigned char data[] = {0xAA, 0xBB};
    uint32_t h = dm1_v1_f0351_stats_material_fnv1a_pc34(data, 2);
    (void)h;
    assert(h != 0u);
}

static void test_receipt_empty(void)
{
    DM1_V1_F0351StatsMaterialReceiptPc34 r;
    int ok = dm1_v1_f0351_stats_material_receipt_pc34(NULL, 0, NULL, 0, &r);
    (void)ok;
    assert(ok == 0);
}

int main(void)
{
    test_source_struct();
    test_glyph_struct();
    test_receipt_struct();
    test_fnv1a_null();
    test_fnv1a_data();
    test_receipt_empty();

    puts("ok: DM1 F0351 stats material (Q-DM1-06) 6 tests passed");
    return 0;
}
