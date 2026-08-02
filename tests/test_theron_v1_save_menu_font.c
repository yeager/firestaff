#include "theron_v1_save_menu_font.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_save_menu_font_glyph_count() == 80);

    assert(theron_v1_save_menu_font_glyph(0x2A) == NULL);
    assert(theron_v1_save_menu_font_glyph(0x7B) == NULL);

    const uint8_t *plus = theron_v1_save_menu_font_glyph('+');
    assert(plus != NULL);
    assert(plus[4] == 0xFE);

    const uint8_t *a_upper = theron_v1_save_menu_font_glyph('A');
    assert(a_upper != NULL);
    assert(a_upper[0] == 0x00);
    assert(a_upper[1] == 0x38);

    const uint8_t *zero = theron_v1_save_menu_font_glyph('0');
    assert(zero != NULL);

    const uint8_t *z_lower = theron_v1_save_menu_font_glyph('z');
    assert(z_lower != NULL);

    int nonzero = 0;
    for (unsigned int ch = 0x2B; ch <= 0x7A; ch++) {
        const uint8_t *g = theron_v1_save_menu_font_glyph(ch);
        assert(g != NULL);
        for (int i = 0; i < 8; i++) {
            if (g[i] != 0) { nonzero++; break; }
        }
    }
    assert(nonzero >= 47);

    printf("PASS: theron_v1_save_menu_font\n");
    return 0;
}
