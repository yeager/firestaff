#include "nexus_v1_retail_title_menu_renderer.h"

#include <string.h>

enum {
    NEXUS_TITLE_BIN_ENGLISH_BYTES = 112216,
    NEXUS_TITLE_MENU_ICON_PALETTE = 0x1486,
    NEXUS_TITLE_MENU_TEXT_PALETTE = 0x02f6,
    NEXUS_TITLE_MENU_ICON_COLOR_BASE = 224,
    NEXUS_TITLE_MENU_TEXT_COLOR_BASE = 240
};

#define NEXUS_TITLE_BIN_ENGLISH_FNV1A64 UINT64_C(0xf512153befe61770)

typedef struct {
    uint32_t source_offset;
    uint16_t width;
    uint16_t height;
    int16_t x;
    int16_t y;
    uint8_t icon_palette;
} Nexus_V1_RetailTitleMenuSprite;

static const Nexus_V1_RetailTitleMenuSprite k_new_game_sprites[] = {
    {0x14a6, 24, 24, 104, 144, 1},
    {0x14a6, 24, 24, 191, 144, 1},
    {0x0446, 64,  8, 129, 153, 0},
    {0x0576, 64,  8, 128, 165, 0},
    {0x07d6,104,  8, 109, 177, 0}
};

static const Nexus_V1_RetailTitleMenuSprite k_continue_sprites[] = {
    {0x0316, 64,  8, 129, 153, 0},
    {0x14a6, 24, 24, 104, 156, 1},
    {0x14a6, 24, 24, 191, 156, 1},
    {0x06a6, 64,  8, 128, 165, 0},
    {0x07d6,104,  8, 109, 177, 0}
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
    /* Captured English menu sprites use VDP1 colour calculation ratio 31/32
     * over the authenticated black VDP2 back-screen at these visible texels. */
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
    colour_base = sprite->icon_palette
        ? NEXUS_TITLE_MENU_ICON_COLOR_BASE
        : NEXUS_TITLE_MENU_TEXT_COLOR_BASE;
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
                                     Nexus_Framebuffer *framebuffer)
{
    const Nexus_V1_RetailTitleMenuSprite *sprites;
    size_t sprite_count;
    size_t index;
    const uint8_t *source;
    size_t source_size;
    uint64_t source_hash;
    if (!title || !state || !framebuffer || !state->menu_active ||
        !title->retail_menu_source_english_verified ||
        !title->retail_menu_source_bytes ||
        title->retail_menu_source_size != NEXUS_TITLE_BIN_ENGLISH_BYTES) {
        return 0;
    }
    source = title->retail_menu_source_bytes;
    source_size = title->retail_menu_source_size;
    source_hash = fnv1a64(source, source_size);
    if (source_hash != NEXUS_TITLE_BIN_ENGLISH_FNV1A64) {
        return 0;
    }
    if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME) {
        sprites = k_new_game_sprites;
        sprite_count = sizeof(k_new_game_sprites) / sizeof(k_new_game_sprites[0]);
    } else if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE) {
        sprites = k_continue_sprites;
        sprite_count = sizeof(k_continue_sprites) / sizeof(k_continue_sprites[0]);
    } else {
        return 0;
    }
    if (source_size < NEXUS_TITLE_MENU_TEXT_PALETTE + 32U ||
        source_size < NEXUS_TITLE_MENU_ICON_PALETTE + 32U) {
        return 0;
    }
    load_capture_palette(source, NEXUS_TITLE_MENU_TEXT_PALETTE,
                         NEXUS_TITLE_MENU_TEXT_COLOR_BASE, framebuffer->palette);
    load_capture_palette(source, NEXUS_TITLE_MENU_ICON_PALETTE,
                         NEXUS_TITLE_MENU_ICON_COLOR_BASE, framebuffer->palette);
    for (index = 0; index < sprite_count; ++index) {
        if (!draw_sprite(source, source_size, &sprites[index], framebuffer)) {
            return 0;
        }
    }
    return 1;
}
