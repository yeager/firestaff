#include "dm1_v1_amiga_palette_fade.h"

#include <string.h>

static int palette_is_admitted(const DM1_V1_AmigaRgb4Palette *palette)
{
    size_t index;
    if (!palette || !palette->colors ||
        palette->color_count != DM1_V1_AMIGA_RGB4_COLOR_COUNT ||
        !palette->original_amiga_palette || !palette->source_bytes_verified ||
        !palette->no_synthetic_palette) {
        return 0;
    }
    for (index = 0u; index < DM1_V1_AMIGA_RGB4_COLOR_COUNT; ++index) {
        if ((palette->colors[index] & 0xf000u) != 0u) return 0;
    }
    return 1;
}

static uint16_t advance_component(uint16_t current, uint16_t target,
                                  unsigned int shift)
{
    const uint16_t mask = (uint16_t)(0x000fu << shift);
    const unsigned int now = (current & mask) >> shift;
    const unsigned int goal = (target & mask) >> shift;
    unsigned int next = now;

    if (now > goal) {
        next -= now - goal > 1u ? 2u : 1u;
    } else if (now < goal) {
        next += goal - now > 1u ? 2u : 1u;
    }
    return (uint16_t)((current & ~mask) | (uint16_t)(next << shift));
}

static uint16_t advance_rgb4(uint16_t current, uint16_t target)
{
    current = advance_component(current, target, 0u);
    current = advance_component(current, target, 4u);
    return advance_component(current, target, 8u);
}

int dm1_v1_amiga_palette_fade_begin(
    DM1_V1_AmigaPaletteFade *state,
    const DM1_V1_AmigaRgb4Palette *source,
    const DM1_V1_AmigaRgb4Palette *target)
{
    if (!state || !palette_is_admitted(source) || !palette_is_admitted(target)) {
        return 0;
    }
    memcpy(state->current, source->colors, sizeof(state->current));
    memcpy(state->target, target->colors, sizeof(state->target));
    state->frame_count = 0u;
    state->accepted = 1;
    return 1;
}

int dm1_v1_amiga_palette_fade_step(
    DM1_V1_AmigaPaletteFade *state,
    DM1_V1_AmigaPaletteFadeReceipt *out_receipt)
{
    size_t index;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !state->accepted ||
        state->frame_count >= DM1_V1_AMIGA_RGB4_FADE_FRAMES) {
        return 0;
    }
    for (index = 0u; index < DM1_V1_AMIGA_RGB4_COLOR_COUNT; ++index) {
        state->current[index] = advance_rgb4(state->current[index],
                                             state->target[index]);
    }
    ++state->frame_count;
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->source_palette_bound = 1;
        out_receipt->target_palette_bound = 1;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->frame_index = state->frame_count;
        memcpy(out_receipt->rgb4, state->current, sizeof(out_receipt->rgb4));
        out_receipt->source_evidence = dm1_v1_amiga_palette_fade_source_evidence();
    }
    return 1;
}

const char *dm1_v1_amiga_palette_fade_source_evidence(void)
{
    return "Dungeon Master Amiga English v2.0 dm (MD5 "
           "b2cf617509826cc45b7b8ccd16a376ac), in-memory 68000 trace: "
           "0x14306 copies the caller palette to -0x2048(A4), changes each "
           "0x0RGB component by one or two toward 8(A5), and calls the Copper "
           "builder at 0x14140 for exactly eight iterations.";
}
