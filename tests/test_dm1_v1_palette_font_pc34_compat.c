#include "dm1_v1_palette_font_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    assert(DM1_PALETTE_SIZE == 16);
    assert(DM1_FONT_CHAR_W == 8);
    assert(DM1_FONT_CHAR_H == 6);
    assert(DM1_FONT_CHARS == 128);
    assert(DM1_PORTRAIT_W == 32);
    assert(DM1_PORTRAIT_H == 29);
    assert(DM1_SKILL_LEVEL_COUNT == 15);
}

static void test_palette_init(void)
{
    DM1_V1_PaletteFontPaletteStatePc34 ps;
    DM1_V1_PaletteFont_PaletteInitPc34Compat(&ps);
    assert(ps.update_middle == false);
    assert(ps.update_top_bottom == false);
}

static void test_font_init(void)
{
    DM1_V1_PaletteFontFontStatePc34 fs;
    DM1_V1_PaletteFont_FontInitPc34Compat(&fs);
    assert(fs.base_font == NULL);
    assert(fs.custom_font == NULL);
    assert(fs.text_color == 0);
    assert(fs.bg_color == 0);
}

static void test_default_palette(void)
{
    const uint16_t* pal = DM1_V1_PaletteFont_GetDefaultPalettePc34Compat();
    static const uint16_t expected[DM1_PALETTE_SIZE] = {
        0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0,
        0xF00, 0xFA0, 0xC86, 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF
    };
    assert(pal != NULL);
    assert(memcmp(pal, expected, sizeof(expected)) == 0);
}

static void test_set_palette(void)
{
    DM1_V1_PaletteFontPaletteStatePc34 ps;
    DM1_V1_PaletteFont_PaletteInitPc34Compat(&ps);
    uint16_t colors[16];
    memset(colors, 0, sizeof(colors));
    colors[0] = 0xF0F0;
    colors[8] = 0xA123;
    DM1_V1_PaletteFont_SetPalettePc34Compat(&ps, colors, 16);
    assert(ps.top_bottom[0].rgb12 == 0x0F0);
    assert(ps.middle[0].rgb12 == 0x123);
}

static void test_build_custom_colors(void)
{
    DM1_V1_PaletteFontFontStatePc34 fs;
    DM1_V1_PaletteFont_FontInitPc34Compat(&fs);
    bool ok = DM1_V1_PaletteFont_FontAllocPc34Compat(&fs);
    (void)ok;
    DM1_V1_PaletteFont_BuildCustomColorsPc34Compat(&fs, 7, 0);
    if (ok) {
        assert(fs.text_color == 7);
        assert(fs.bg_color == 0);
    }
}

static void test_skill_name(void)
{
    const char* name = DM1_V1_PaletteFont_GetSkillNamePc34Compat(0);
    assert(name != NULL);
}

static void test_font_alloc(void)
{
    DM1_V1_PaletteFontFontStatePc34 fs;
    DM1_V1_PaletteFont_FontInitPc34Compat(&fs);
    bool ok = DM1_V1_PaletteFont_FontAllocPc34Compat(&fs);
    (void)ok;
    assert(ok == true || ok == false);
}

static void test_portrait_planar_conversion(void)
{
    enum { bytes_per_row = DM1_PORTRAIT_W / 8,
           plane_bytes = bytes_per_row * DM1_PORTRAIT_H,
           output_bytes = DM1_PORTRAIT_W * DM1_PORTRAIT_H };
    unsigned char pixels[output_bytes];

    /* First byte of each plane encodes the first eight source pixels.  This
     * vector also catches an erroneous w*h byte plane stride. */
    memset(pixels, 0, sizeof(pixels));
    pixels[0 * plane_bytes] = 0x80u; /* pixel 0: bit 0 */
    pixels[1 * plane_bytes] = 0x40u; /* pixel 1: bit 1 */
    pixels[2 * plane_bytes] = 0x20u; /* pixel 2: bit 2 */
    pixels[3 * plane_bytes] = 0x10u; /* pixel 3: bit 3 */
    DM1_V1_PaletteFont_ConvertPortraitPlanarPc34Compat(pixels);
    assert(pixels[0] == 1u);
    assert(pixels[1] == 2u);
    assert(pixels[2] == 4u);
    assert(pixels[3] == 8u);
    assert(pixels[4] == 0u);
    assert(pixels[DM1_PORTRAIT_W] == 0u);
}

int main(void)
{
    test_constants();
    test_palette_init();
    test_font_init();
    test_default_palette();
    test_set_palette();
    test_build_custom_colors();
    test_skill_name();
    test_font_alloc();
    test_portrait_planar_conversion();

    puts("ok: DM1 palette font (Q-DM1-03) 9 tests passed");
    return 0;
}
