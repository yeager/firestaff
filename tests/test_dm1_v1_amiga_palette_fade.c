#include "dm1_v1_amiga_palette_fade.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM1_V1_AmigaRgb4Palette admitted(const uint16_t *colors)
{
    DM1_V1_AmigaRgb4Palette result;
    memset(&result, 0, sizeof(result));
    result.colors = colors;
    result.color_count = DM1_V1_AMIGA_RGB4_COLOR_COUNT;
    result.original_amiga_palette = 1;
    result.source_bytes_verified = 1;
    result.no_synthetic_palette = 1;
    return result;
}

int main(void)
{
    uint16_t source[DM1_V1_AMIGA_RGB4_COLOR_COUNT] = {0};
    uint16_t target[DM1_V1_AMIGA_RGB4_COLOR_COUNT] = {0};
    DM1_V1_AmigaPaletteFade state;
    DM1_V1_AmigaPaletteFadeReceipt receipt;
    DM1_V1_AmigaRgb4Palette source_palette;
    DM1_V1_AmigaRgb4Palette target_palette;
    unsigned int frame;

    source[0] = 0x0fffu;
    source[1] = 0x0000u;
    source[2] = 0x0123u;
    target[0] = 0x0000u;
    target[1] = 0x0fffu;
    target[2] = 0x0765u;
    source_palette = admitted(source);
    target_palette = admitted(target);

    assert(dm1_v1_amiga_palette_fade_begin(&state, &source_palette,
                                            &target_palette));
    assert(dm1_v1_amiga_palette_fade_step(&state, &receipt));
    assert(receipt.accepted && receipt.frame_index == 1u);
    assert(receipt.source_palette_bound && receipt.target_palette_bound);
    assert(receipt.suppress_synthetic_fallback);
    assert(receipt.rgb4[0] == 0x0dddu);
    assert(receipt.rgb4[1] == 0x0222u);
    assert(receipt.rgb4[2] == 0x0345u);
    assert(strstr(receipt.source_evidence, "0x14306") != NULL);

    for (frame = 1u; frame < DM1_V1_AMIGA_RGB4_FADE_FRAMES; ++frame) {
        assert(dm1_v1_amiga_palette_fade_step(&state, &receipt));
    }
    assert(receipt.frame_index == DM1_V1_AMIGA_RGB4_FADE_FRAMES);
    assert(receipt.rgb4[0] == target[0]);
    assert(receipt.rgb4[1] == target[1]);
    assert(receipt.rgb4[2] == target[2]);
    assert(!dm1_v1_amiga_palette_fade_step(&state, &receipt));

    source_palette.source_bytes_verified = 0;
    assert(!dm1_v1_amiga_palette_fade_begin(&state, &source_palette,
                                             &target_palette));
    source_palette.source_bytes_verified = 1;
    target[3] = 0xf001u;
    assert(!dm1_v1_amiga_palette_fade_begin(&state, &source_palette,
                                             &target_palette));

    puts("ok: DM1 Amiga original RGB4 palette producer");
    return 0;
}
