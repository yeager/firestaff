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

/* Synthetic storage exercises receipt validation only, not original-media
 * rendering or parity. The original-media gate covers loaded asset bytes. */
static void test_required_materials(void)
{
    static unsigned char pixels[144 * 73];
    static unsigned char font[768];
    const int ids[6] = {10, 9, 20, 30, 31, 32};
    const int widths[6] = {87, 87, 144, 34, 46, 96};
    const int heights[6] = {45, 25, 73, 9, 9, 15};
    DM1_V1_HudSourceSurfacePc34 surfaces[6];
    DM1_V1_HudGlyphSourcePc34 glyph;
    DM1_V1_F0344F0658HudMaterialReceiptPc34 receipt;
    int i, rc;
    memset(surfaces, 0, sizeof(surfaces));
    memset(&glyph, 0, sizeof(glyph));
    for (i = 0; i < 6; ++i) {
        surfaces[i].graphicsDatOwned = 1;
        surfaces[i].graphicIndex = ids[i];
        surfaces[i].width = widths[i];
        surfaces[i].height = heights[i];
        surfaces[i].indexedPixelCount = widths[i] * heights[i];
        surfaces[i].indexedPixels = pixels;
        surfaces[i].pixelsFNV1a = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(
            pixels, surfaces[i].indexedPixelCount);
    }
    glyph.graphicsDatOwned = 1;
    glyph.graphicIndex = 695;
    glyph.bits = font;
    glyph.byteCount = sizeof(font);
    glyph.bitsFNV1a = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(font, sizeof(font));
    rc = dm1_v1_f0344_f0658_hud_material_receipt_pc34(surfaces, 6, &glyph, 1, &receipt);
    assert(rc && receipt.valid && receipt.operationCount == 10);
    assert(receipt.operations[3].graphicIndex == 9 && receipt.operations[3].zoneIndex == 13);
    for (i = 4; i <= 5; ++i) {
        const DM1_V1_HudMaterialOperationPc34* op = &receipt.operations[i];
        assert(op->graphicIndex == 695 && op->sourceX == 0 && op->sourceY == 0);
        assert(op->sourceW == 0 && op->sourceH == 0);
        assert(op->paletteForeground == 4 && op->paletteBackground == 0);
        assert(op->zoneIndex == (i == 4 ? 255 : 261));
        assert(op->zoneCount == (i == 4 ? 6 : 4));
    }
    for (i = 0; i < 6; ++i) {
        surfaces[i].graphicsDatOwned = 0;
        rc = dm1_v1_f0344_f0658_hud_material_receipt_pc34(surfaces, 6, &glyph, 1, &receipt);
        assert(!rc && !receipt.valid);
        surfaces[i].graphicsDatOwned = 1;
        ++surfaces[i].pixelsFNV1a;
        rc = dm1_v1_f0344_f0658_hud_material_receipt_pc34(surfaces, 6, &glyph, 1, &receipt);
        assert(!rc && !receipt.valid);
        --surfaces[i].pixelsFNV1a;
    }
    glyph.graphicsDatOwned = 0;
    rc = dm1_v1_f0344_f0658_hud_material_receipt_pc34(surfaces, 6, &glyph, 1, &receipt);
    assert(!rc && !receipt.valid);
    glyph.graphicsDatOwned = 1;
    ++glyph.bitsFNV1a;
    rc = dm1_v1_f0344_f0658_hud_material_receipt_pc34(surfaces, 6, &glyph, 1, &receipt);
    assert(!rc && !receipt.valid);
    (void)rc;
}

int main(void)
{
    test_fnv1a_null();
    test_fnv1a_deterministic();
    test_fnv1a_different_input();
    test_receipt_null_rejected();
    test_receipt_empty_surfaces_rejected();
    test_receipt_unowned_surfaces_rejected();
    test_required_materials();

    puts("ok: DM1 F0344/F0658 HUD material (Q-DM1-07) 7 tests passed");
    return 0;
}
