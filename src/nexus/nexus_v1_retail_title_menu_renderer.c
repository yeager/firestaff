#include "nexus_v1_retail_title_menu_renderer.h"

#include <string.h>

enum {
    NEXUS_TITLE_BIN_BYTES = 112216,
    NEXUS_TITLE_MENU_ENGLISH_ICON_PALETTE = 0x1486,
    NEXUS_TITLE_MENU_JAPANESE_NEW_GAME_ICON_PALETTE = 0x15d6,
    NEXUS_TITLE_MENU_JAPANESE_ICON_PALETTE = 0x1b16,
    NEXUS_TITLE_MENU_TEXT_PALETTE = 0x02f6,
    NEXUS_TITLE_MENU_ICON_COLOR_BASE = 224,
    NEXUS_TITLE_MENU_INACTIVE_TEXT_COLOR_BASE = 208,
    NEXUS_TITLE_MENU_TEXT_COLOR_BASE = 240
};

#define NEXUS_TITLE_BIN_ENGLISH_FNV1A64 UINT64_C(0xf512153befe61770)
#define NEXUS_TITLE_BIN_JAPANESE_FNV1A64 UINT64_C(0x064414f722d0409c)

typedef struct {
    uint32_t source_offset;
    uint16_t width;
    uint16_t height;
    int16_t x;
    int16_t y;
    uint32_t palette_offset;
    uint8_t colour_base;
    uint8_t flip_x;
} Nexus_V1_RetailTitleMenuSprite;

static void load_capture_palette(const uint8_t *source,
                                 size_t source_offset,
                                 uint8_t colour_base,
                                 uint32_t palette[256]);
static int draw_sprite(const uint8_t *source,
                       size_t source_size,
                       const Nexus_V1_RetailTitleMenuSprite *sprite,
                       Nexus_Framebuffer *framebuffer);

typedef struct {
    uint32_t source_offset;
    uint32_t palette_offset;
} Nexus_V1_RetailMenuMarkerFrame;

static const Nexus_V1_RetailMenuMarkerFrame k_english_configuration_marker_frames[] = {
    {0x15f6,0x15d6}, {0x1746,0x1726}, {0x1896,0x1876}, {0x19e6,0x19c6},
    {0x1b36,0x1b16}, {0x1c86,0x1c66}, {0x1dd6,0x1db6}, {0x0b76,0x0b56},
    {0x0cc6,0x0ca6}, {0x0e16,0x0df6}, {0x0f66,0x0f46}, {0x10b6,0x1096},
    {0x1206,0x11e6}, {0x1356,0x1336}, {0x14a6,0x1486}
};

static const int k_english_configuration_marker_frame_start[] = {
    0, 2, 4, 7, 9, 12, 14, 17, 19, 22, 24, 27, 29, 32, 34
};

static const Nexus_V1_RetailTitleMenuSprite k_new_game_sprites[] = {
    {0x14a6, 24, 24, 104, 144, NEXUS_TITLE_MENU_ENGLISH_ICON_PALETTE, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x14a6, 24, 24, 191, 144, NEXUS_TITLE_MENU_ENGLISH_ICON_PALETTE, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x0446, 64,  8, 129, 153, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x0576, 64,  8, 128, 165, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x07d6,104,  8, 109, 177, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0}
};

static const Nexus_V1_RetailTitleMenuSprite k_english_continue_sprites[] = {
    {0x0316, 64,  8, 129, 153, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x14a6, 24, 24, 104, 156, NEXUS_TITLE_MENU_ENGLISH_ICON_PALETTE, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x14a6, 24, 24, 191, 156, NEXUS_TITLE_MENU_ENGLISH_ICON_PALETTE, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x06a6, 64,  8, 128, 165, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x07d6,104,  8, 109, 177, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0}
};

static const Nexus_V1_RetailTitleMenuSprite k_english_configuration_sprites[] = {
    {0x0316, 64,  8, 129, 153, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x0576, 64,  8, 128, 165, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x1dd6, 24, 24,  85, 168, 0x1db6, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x1dd6, 24, 24, 211, 168, 0x1db6, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 1},
    {0x09a6,104,  8, 109, 177, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0}
};

static int render_english_configuration_page(
    const uint8_t *source,
    size_t source_size,
    const Nexus_V1_RetailTitleMenuState *state,
    Nexus_Framebuffer *framebuffer,
    int animation_frame)
{
    Nexus_V1_RetailTitleMenuSprite sprites[18];
    const uint32_t active_palette = 0x170aa;
    const uint32_t inactive_palette = 0x172d2;
    const int row_y[] = {60, 83, 106, 128, 175};
    const int marker_right_x[] = {234, 295, 271, 233, 68};
    size_t count = 0;
    size_t index;
    uint32_t music_on_palette = state->music_enabled ? active_palette : inactive_palette;
    uint32_t music_off_palette = state->music_enabled ? inactive_palette : active_palette;
    uint8_t music_on_base = state->music_enabled
                                ? NEXUS_TITLE_MENU_TEXT_COLOR_BASE
                                : NEXUS_TITLE_MENU_INACTIVE_TEXT_COLOR_BASE;
    uint8_t music_off_base = state->music_enabled
                                 ? NEXUS_TITLE_MENU_INACTIVE_TEXT_COLOR_BASE
                                 : NEXUS_TITLE_MENU_TEXT_COLOR_BASE;
    uint32_t sound_mono_palette = state->stereo_enabled ? inactive_palette : active_palette;
    uint32_t sound_stereo_palette = state->stereo_enabled ? active_palette : inactive_palette;
    uint8_t sound_mono_base = state->stereo_enabled
                                  ? NEXUS_TITLE_MENU_INACTIVE_TEXT_COLOR_BASE
                                  : NEXUS_TITLE_MENU_TEXT_COLOR_BASE;
    uint8_t sound_stereo_base = state->stereo_enabled
                                    ? NEXUS_TITLE_MENU_TEXT_COLOR_BASE
                                    : NEXUS_TITLE_MENU_INACTIVE_TEXT_COLOR_BASE;
    uint32_t level_normal_palette = state->easy_level_enabled ? inactive_palette : active_palette;
    uint32_t level_easy_palette = state->easy_level_enabled ? active_palette : inactive_palette;
    uint8_t level_normal_base = state->easy_level_enabled
                                   ? NEXUS_TITLE_MENU_INACTIVE_TEXT_COLOR_BASE
                                   : NEXUS_TITLE_MENU_TEXT_COLOR_BASE;
    uint8_t level_easy_base = state->easy_level_enabled
                                  ? NEXUS_TITLE_MENU_TEXT_COLOR_BASE
                                  : NEXUS_TITLE_MENU_INACTIVE_TEXT_COLOR_BASE;
    uint32_t gauge_on_palette = state->monster_gauge_enabled ? active_palette : inactive_palette;
    uint32_t gauge_off_palette = state->monster_gauge_enabled ? inactive_palette : active_palette;
    uint8_t gauge_on_base = state->monster_gauge_enabled
                                ? NEXUS_TITLE_MENU_TEXT_COLOR_BASE
                                : NEXUS_TITLE_MENU_INACTIVE_TEXT_COLOR_BASE;
    uint8_t gauge_off_base = state->monster_gauge_enabled
                                 ? NEXUS_TITLE_MENU_INACTIVE_TEXT_COLOR_BASE
                                 : NEXUS_TITLE_MENU_TEXT_COLOR_BASE;
    int marker_phase = animation_frame % 37;
    size_t marker_frame = 0;

    if (state->configuration_row < NEXUS_V1_RETAIL_CONFIGURATION_MUSIC ||
        state->configuration_row > NEXUS_V1_RETAIL_CONFIGURATION_EXIT ||
        (state->music_enabled != 0 && state->music_enabled != 1) ||
        (state->stereo_enabled != 0 && state->stereo_enabled != 1) ||
        (state->easy_level_enabled != 0 && state->easy_level_enabled != 1) ||
        (state->monster_gauge_enabled != 0 &&
         state->monster_gauge_enabled != 1)) {
        return 0;
    }
    if (marker_phase < 0) {
        marker_phase += 37;
    }
    while (marker_frame + 1 < sizeof(k_english_configuration_marker_frame_start) /
                                      sizeof(k_english_configuration_marker_frame_start[0]) &&
           marker_phase >= k_english_configuration_marker_frame_start[marker_frame + 1]) {
        ++marker_frame;
    }

#define ADD_CONFIG_SPRITE(offset, w, h, x, y, palette, base, flip) \
    do { \
        sprites[count++] = (Nexus_V1_RetailTitleMenuSprite){ \
            (offset), (w), (h), (x), (y), (palette), (base), (flip) \
        }; \
    } while (0)

    ADD_CONFIG_SPRITE(0x1a592,232,37,45,20,active_palette,
                      NEXUS_TITLE_MENU_TEXT_COLOR_BASE,0);
    ADD_CONFIG_SPRITE(0x170ca,56,18,31,65,0x170aa,
                      NEXUS_TITLE_MENU_TEXT_COLOR_BASE,0);
    ADD_CONFIG_SPRITE(0x172f2,24,13,175,67,music_on_palette,music_on_base,0);
    ADD_CONFIG_SPRITE(0x1748a,32,14,205,66,music_off_palette,music_off_base,0);
    ADD_CONFIG_SPRITE(0x1787e,56,17,31,88,0x170aa,
                      NEXUS_TITLE_MENU_TEXT_COLOR_BASE,0);
    ADD_CONFIG_SPRITE(0x17a8a,64,12,174,89,sound_mono_palette,sound_mono_base,0);
    ADD_CONFIG_SPRITE(0x17dea,56,12,243,89,sound_stereo_palette,sound_stereo_base,0);
    ADD_CONFIG_SPRITE(0x183f2,104,16,29,111,0x170aa,
                      NEXUS_TITLE_MENU_TEXT_COLOR_BASE,0);
    ADD_CONFIG_SPRITE(0x18762,56,13,175,111,level_normal_palette,level_normal_base,0);
    ADD_CONFIG_SPRITE(0x18a9a,40,13,236,111,level_easy_palette,level_easy_base,0);
    ADD_CONFIG_SPRITE(0x19132,128,18,31,132,0x170aa,
                      NEXUS_TITLE_MENU_TEXT_COLOR_BASE,0);
    ADD_CONFIG_SPRITE(0x195e2,24,13,175,134,gauge_on_palette,gauge_on_base,0);
    ADD_CONFIG_SPRITE(0x1977a,32,14,204,133,gauge_off_palette,gauge_off_base,0);
    ADD_CONFIG_SPRITE(0x1a40e,40,17,31,179,0x170aa,
                      NEXUS_TITLE_MENU_TEXT_COLOR_BASE,0);
    if (state->configuration_row <= NEXUS_V1_RETAIL_CONFIGURATION_EXIT) {
        int marker_y = row_y[state->configuration_row];
        const Nexus_V1_RetailMenuMarkerFrame *marker =
            &k_english_configuration_marker_frames[marker_frame];
        ADD_CONFIG_SPRITE(marker->source_offset,24,24,10,marker_y,
                          marker->palette_offset,
                          NEXUS_TITLE_MENU_ICON_COLOR_BASE,0);
        ADD_CONFIG_SPRITE(marker->source_offset,24,24,
                          marker_right_x[state->configuration_row],marker_y,
                          marker->palette_offset,
                          NEXUS_TITLE_MENU_ICON_COLOR_BASE,1);
    }
#undef ADD_CONFIG_SPRITE

    if (count > sizeof(sprites) / sizeof(sprites[0])) {
        return 0;
    }
    for (index = 0; index < count; ++index) {
        if ((size_t)sprites[index].source_offset +
                ((size_t)sprites[index].width * sprites[index].height) / 2U >
                source_size ||
            source_size < (size_t)sprites[index].palette_offset + 32U) {
            return 0;
        }
        load_capture_palette(source, sprites[index].palette_offset,
                             sprites[index].colour_base, framebuffer->palette);
        if (!draw_sprite(source, source_size, &sprites[index], framebuffer)) {
            return 0;
        }
    }
    return 1;
}

static const Nexus_V1_RetailTitleMenuSprite k_japanese_continue_sprites[] = {
    {0x0316, 64,  8, 129, 153, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x1b36, 24, 24, 104, 156, NEXUS_TITLE_MENU_JAPANESE_ICON_PALETTE, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x1b36, 24, 24, 191, 156, NEXUS_TITLE_MENU_JAPANESE_ICON_PALETTE, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x06a6, 64,  8, 128, 165, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x07d6,104,  8, 109, 177, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0}
};

static const Nexus_V1_RetailTitleMenuSprite k_japanese_new_game_sprites[] = {
    {0x15f6, 24, 24, 104, 144, NEXUS_TITLE_MENU_JAPANESE_NEW_GAME_ICON_PALETTE, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x15f6, 24, 24, 191, 144, NEXUS_TITLE_MENU_JAPANESE_NEW_GAME_ICON_PALETTE, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x0446, 64,  8, 129, 153, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x0576, 64,  8, 128, 165, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x07d6,104,  8, 109, 177, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0}
};

static const Nexus_V1_RetailTitleMenuSprite k_japanese_configuration_sprites[] = {
    {0x0316, 64,  8, 129, 153, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x0576, 64,  8, 128, 165, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0},
    {0x1746, 24, 24,  85, 168, 0x1726, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 0},
    {0x1746, 24, 24, 211, 168, 0x1726, NEXUS_TITLE_MENU_ICON_COLOR_BASE, 1},
    {0x09a6,104,  8, 109, 177, NEXUS_TITLE_MENU_TEXT_PALETTE, NEXUS_TITLE_MENU_TEXT_COLOR_BASE, 0}
};

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) {
        value ^= bytes[index];
        value *= UINT64_C(1099511628211);
    }
    return value;
}

static uint8_t blend_channel_over_black(uint16_t bgr555, unsigned shift)
{
    unsigned component = ((unsigned)bgr555 >> shift) & 31U;
    /* Captured Japanese and English menu sprites use VDP1 colour calculation
     * ratio 31/32 over the authenticated black back-screen at visible texels. */
    return (uint8_t)(((component << 3U) * 31U) / 32U);
}

static void load_capture_palette(const uint8_t *source,
                                 size_t source_offset,
                                 uint8_t colour_base,
                                 uint32_t palette[256])
{
    unsigned index;
    for (index = 1; index < 16; ++index) {
        uint16_t word = read_be16(source + source_offset + index * 2U);
        uint8_t red = blend_channel_over_black(word, 0);
        uint8_t green = blend_channel_over_black(word, 5);
        uint8_t blue = blend_channel_over_black(word, 10);
        palette[colour_base + index] = UINT32_C(0xff000000) |
            ((uint32_t)red << 16U) | ((uint32_t)green << 8U) | blue;
    }
}

static int draw_sprite(const uint8_t *source,
                       size_t source_size,
                       const Nexus_V1_RetailTitleMenuSprite *sprite,
                       Nexus_Framebuffer *framebuffer)
{
    size_t byte_count;
    const uint8_t *pixels;
    uint8_t colour_base;
    int y;
    if (!source || !sprite || !framebuffer || sprite->width == 0 ||
        sprite->height == 0) {
        return 0;
    }
    byte_count = ((size_t)sprite->width * sprite->height) / 2U;
    if ((size_t)sprite->source_offset > source_size ||
        byte_count > source_size - (size_t)sprite->source_offset) {
        return 0;
    }
    pixels = source + sprite->source_offset;
    colour_base = sprite->colour_base;
    for (y = 0; y < sprite->height; ++y) {
        int x;
        int screen_y = sprite->y + y;
        if (screen_y < 0 || screen_y >= NEXUS_FB_H) {
            continue;
        }
        for (x = 0; x < sprite->width; ++x) {
            int screen_x = sprite->x + x;
            size_t texel_index = (size_t)y * sprite->width + (size_t)x;
            uint8_t packed;
            uint8_t texel;
            if (screen_x < 0 || screen_x >= NEXUS_FB_W) {
                continue;
            }
            if (sprite->flip_x) {
                texel_index = (size_t)y * sprite->width +
                              (size_t)(sprite->width - 1 - x);
            }
            packed = pixels[texel_index >> 1U];
            texel = (texel_index & 1U) ? (packed & 15U) : (packed >> 4U);
            if (texel != 0) {
                framebuffer->color_buffer[(size_t)screen_y * NEXUS_FB_W +
                                          (size_t)screen_x] =
                    (uint8_t)(colour_base + texel);
            }
        }
    }
    return 1;
}

int nexus_v1_retail_title_menu_render(const Nexus_TitleScreen *title,
                                     const Nexus_V1_RetailTitleMenuState *state,
                                     Nexus_Framebuffer *framebuffer,
                                     int animation_frame)
{
    const Nexus_V1_RetailTitleMenuSprite *sprites;
    size_t sprite_count;
    size_t index;
    const uint8_t *source;
    size_t source_size;
    uint64_t source_hash;
    if (!title || !state || !framebuffer || !state->menu_active ||
        title->retail_menu_source_region == NEXUS_V1_TITLE_MENU_SOURCE_NONE ||
        !title->retail_menu_source_bytes ||
        title->retail_menu_source_size != NEXUS_TITLE_BIN_BYTES) {
        return 0;
    }
    source = title->retail_menu_source_bytes;
    source_size = title->retail_menu_source_size;
    source_hash = fnv1a64(source, source_size);
    if (state->configuration_active) {
        if (title->retail_menu_source_region !=
                NEXUS_V1_TITLE_MENU_SOURCE_ENGLISH ||
            source_hash != NEXUS_TITLE_BIN_ENGLISH_FNV1A64) {
            return 0;
        }
        return render_english_configuration_page(source, source_size, state,
                                                  framebuffer, animation_frame);
    }
    if (title->retail_menu_source_region == NEXUS_V1_TITLE_MENU_SOURCE_JAPANESE) {
        if (source_hash != NEXUS_TITLE_BIN_JAPANESE_FNV1A64) {
            return 0;
        }
        if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME) {
            sprites = k_japanese_new_game_sprites;
            sprite_count = sizeof(k_japanese_new_game_sprites) /
                           sizeof(k_japanese_new_game_sprites[0]);
        } else if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE) {
            sprites = k_japanese_continue_sprites;
            sprite_count = sizeof(k_japanese_continue_sprites) /
                           sizeof(k_japanese_continue_sprites[0]);
        } else if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
            sprites = k_japanese_configuration_sprites;
            sprite_count = sizeof(k_japanese_configuration_sprites) /
                           sizeof(k_japanese_configuration_sprites[0]);
        } else {
            return 0;
        }
    } else if (title->retail_menu_source_region != NEXUS_V1_TITLE_MENU_SOURCE_ENGLISH ||
               source_hash != NEXUS_TITLE_BIN_ENGLISH_FNV1A64) {
        return 0;
    } else if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME) {
        sprites = k_new_game_sprites;
        sprite_count = sizeof(k_new_game_sprites) / sizeof(k_new_game_sprites[0]);
    } else if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE) {
        sprites = k_english_continue_sprites;
        sprite_count = sizeof(k_english_continue_sprites) /
                       sizeof(k_english_continue_sprites[0]);
    } else if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
        sprites = k_english_configuration_sprites;
        sprite_count = sizeof(k_english_configuration_sprites) /
                       sizeof(k_english_configuration_sprites[0]);
    } else {
        return 0;
    }
    for (index = 0; index < sprite_count; ++index) {
        if (source_size < (size_t)sprites[index].palette_offset + 32U) {
            return 0;
        }
        load_capture_palette(source, sprites[index].palette_offset,
                             sprites[index].colour_base, framebuffer->palette);
        if (!draw_sprite(source, source_size, &sprites[index], framebuffer)) {
            return 0;
        }
    }
    return 1;
}
