#include "menu_unicode_glyphs_m12.h"

#include <stdint.h>
#include <stdio.h>

static int g_failures = 0;

static void expect_glyph(uint32_t codepoint, const char* label) {
    const M12_UnicodeGlyph* glyph = M12_FindUnicodeGlyph(codepoint);
    if (!glyph || glyph->width == 0 || glyph->height == 0) {
        fprintf(stderr, "missing glyph U+%04X for %s\n",
                (unsigned int)codepoint,
                label);
        ++g_failures;
    }
}

static void expect_glyph_top_row(uint32_t codepoint,
                                 uint16_t expected,
                                 const char* label) {
    const M12_UnicodeGlyph* glyph = M12_FindUnicodeGlyph(codepoint);
    if (!glyph || glyph->rows[0] != expected || glyph->rows[2] == 0x8100u) {
        fprintf(stderr, "incorrect bitmap for U+%04X (%s)\n",
                (unsigned int)codepoint,
                label);
        ++g_failures;
    }
}

static void expect_text_covered(const char* text) {
    int missing = M12_UnicodeTextMissingGlyphCount(text);
    if (missing != 0) {
        fprintf(stderr, "missing %d glyphs for startup language text: %s\n",
                missing,
                text ? text : "(null)");
        ++g_failures;
    }
}

int main(void) {
    const struct {
        uint32_t codepoint;
        const char* label;
    } required[] = {
        {0x00C4u, "SWEDISH capital A-diaeresis"},
        {0x00C5u, "SWEDISH capital A-ring"},
        {0x00C7u, "FRANCAIS cedilla"},
        {0x00D1u, "ESPANOL enye"},
        {0x00D6u, "SWEDISH capital O-diaeresis"},
        {0x00E4u, "SWEDISH small A-diaeresis"},
        {0x00E5u, "SWEDISH small A-ring"},
        {0x00F6u, "SWEDISH small O-diaeresis"},
        {0x00C5u, "NORSK BOKMAL ring"},
        {0x00CAu, "PORTUGUES circumflex"},
        {0x00DCu, "TURKCE diaeresis"},
        {0x010Cu, "CESTINA C-caron"},
        {0x0158u, "CESTINA R-caron"},
        {0x0160u, "CESTINA S-caron"},
        {0x017Du, "CESTINA Z-caron"},
        {0x0418u, "RUSSIAN I"},
        {0x0419u, "RUSSIAN short I"},
        {0x041Au, "RUSSIAN K"},
        {0x0420u, "RUSSIAN R"},
        {0x0421u, "RUSSIAN S"},
        {0x0423u, "RUSSIAN U"},
        {0x4E2Du, "Chinese zhong"},
        {0x4F53u, "Chinese ti"},
        {0x672Cu, "Japanese hon"},
        {0x7B80u, "Chinese jian"},
        {0xAD6Du, "Korean guk"},
        {0xC5B4u, "Korean eo"},
        {0xD55Cu, "Korean han"}
    };
    unsigned int i;
    for (i = 0; i < (unsigned int)(sizeof(required) / sizeof(required[0])); ++i) {
        expect_glyph(required[i].codepoint, required[i].label);
    }
    expect_glyph_top_row(0x00C4u, 0x2200u, "A-diaeresis dots");
    expect_glyph_top_row(0x00C5u, 0x1C00u, "A-ring");
    expect_glyph_top_row(0x00D6u, 0x2200u, "O-diaeresis dots");
    expect_glyph_top_row(0x00E4u, 0x2200u, "small A-diaeresis dots");
    expect_glyph_top_row(0x00E5u, 0x1C00u, "small A-ring");
    expect_glyph_top_row(0x00F6u, 0x2200u, "small O-diaeresis dots");
    expect_text_covered("INSTÄLLNINGAR SPRÅK TILLÅT MAPPÅTKOMST ÖPPNA");
    {
        const char* languageNames[] = {
            "ENGLISH",
            "SVENSKA",
            "FRANÇAIS",
            "DEUTSCH",
            "日本語",
            "简体中文",
            "ČEŠTINA",
            "DANSK",
            "ESPAÑOL",
            "SUOMI",
            "MAGYAR",
            "ITALIANO",
            "한국어",
            "NEDERLANDS",
            "NORSK BOKMÅL",
            "POLSKI",
            "PORTUGUÊS",
            "РУССКИЙ",
            "TÜRKÇE",
            "BAHASA INDONESIA"
        };
        for (i = 0; i < (unsigned int)(sizeof(languageNames) / sizeof(languageNames[0])); ++i) {
            expect_text_covered(languageNames[i]);
        }
    }
    if (g_failures) {
        return 1;
    }
    puts("m12 menu unicode language glyphs: ok");
    return 0;
}
