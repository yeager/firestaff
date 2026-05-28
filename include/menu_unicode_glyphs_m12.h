#ifndef FIRESTAFF_MENU_UNICODE_GLYPHS_M12_H
#define FIRESTAFF_MENU_UNICODE_GLYPHS_M12_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t codepoint;
    unsigned char width;
    unsigned char height;
    uint16_t rows[16];
} M12_UnicodeGlyph;

const M12_UnicodeGlyph* M12_FindUnicodeGlyph(uint32_t codepoint);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MENU_UNICODE_GLYPHS_M12_H */
