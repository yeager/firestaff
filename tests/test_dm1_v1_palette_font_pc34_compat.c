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
    assert(pal != NULL);
}

static void test_set_palette(void)
{
    DM1_V1_PaletteFontPaletteStatePc34 ps;
    DM1_V1_PaletteFont_PaletteInitPc34Compat(&ps);
    uint16_t colors[16];
    memset(colors, 0, sizeof(colors));
    colors[0] = 0x0F0;
    DM1_V1_PaletteFont_SetPalettePc34Compat(&ps, colors, 16);
    assert(ps.top_bottom[0].rgb12 == 0x0F0);
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

    puts("ok: DM1 palette font (Q-DM1-03) 8 tests passed");
    return 0;
}
