#include "csb_v1_atari_hud_bar_graphs.h"

#include <string.h>

enum { SCREEN_WIDTH = 320, SCREEN_HEIGHT = 200, BAR_TOP = 2, BAR_ROWS = 25 };

/* ReDMCSB DATA.C G0055/G0056, MEDIA009 Atari ST. Index order is the order
 * F0287 computes heights: health, stamina, mana. Each mask word covers one
 * 16-pixel planar group; the second word is the adjacent group at +8 bytes. */
static const uint16_t k_masks[4][3][2] = {
    {{0x0003u, 0xc000u}, {0x0780u, 0x0000u}, {0x000fu, 0x0000u}},
    {{0x1e00u, 0x0000u}, {0x003cu, 0x0000u}, {0x7800u, 0x0000u}},
    {{0x00f0u, 0x0000u}, {0x0001u, 0xe000u}, {0x03c0u, 0x0000u}},
    {{0x0007u, 0x8000u}, {0x0f00u, 0x0000u}, {0x001eu, 0x0000u}}
};

static const uint16_t k_byte_offsets[4][3] = {
    {16u, 24u, 24u}, {56u, 56u, 64u},
    {88u, 88u, 96u}, {120u, 128u, 128u}
};

static const uint8_t k_champion_colors[4] = {7u, 11u, 8u, 14u};

static int bar_height(uint16_t current, uint16_t maximum, unsigned *out)
{
    uint32_t scaled;
    if (!out || (current != 0u && maximum == 0u)) return 0;
    if (current == 0u) {
        *out = 0u;
        return 1;
    }
    scaled = (uint32_t)current * BAR_ROWS;
    /* F0287's 10-bit fixed point calculation rounds every nonzero
     * fractional remainder upward. Clipping at 25 is equivalent to its
     * 25 source scanlines even when a stat temporarily exceeds its maximum. */
    *out = (unsigned)((scaled + maximum - 1u) / maximum);
    if (*out > BAR_ROWS) *out = BAR_ROWS;
    return 1;
}

static void draw_mask_word(uint8_t *framebuffer, unsigned x_group,
                           uint16_t mask, unsigned y, uint8_t color)
{
    unsigned bit;
    for (bit = 0u; bit < 16u; ++bit) {
        if ((mask & (uint16_t)(0x8000u >> bit)) != 0u) {
            const unsigned x = x_group * 16u + bit;
            if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
                framebuffer[(size_t)y * SCREEN_WIDTH + x] = color;
        }
    }
}

int csb_v1_atari_hud_draw_bar_graphs(
    uint8_t *framebuffer, size_t framebuffer_size,
    const CSB_V1_AtariHudChampionBars champions[4])
{
    uint8_t candidate[SCREEN_WIDTH * SCREEN_HEIGHT];
    unsigned champion;

    if (!framebuffer || framebuffer_size < sizeof(candidate) || !champions)
        return 0;
    memcpy(candidate, framebuffer, sizeof(candidate));
    for (champion = 0u; champion < 4u; ++champion) {
        unsigned stat;
        if (!champions[champion].present ||
            !champions[champion].statistics_enabled) continue;
        for (stat = 0u; stat < 3u; ++stat) {
            unsigned height;
            unsigned mask_part;
            unsigned row;
            const unsigned x_group = k_byte_offsets[champion][stat] / 8u;
            if (!bar_height(champions[champion].current[stat],
                            champions[champion].maximum[stat], &height))
                return 0;
            for (row = 0u; row < BAR_ROWS; ++row) {
                const unsigned source_line = BAR_ROWS - 1u - row;
                const uint8_t color = height > source_line
                    ? k_champion_colors[champion] : 12u;
                for (mask_part = 0u; mask_part < 2u; ++mask_part) {
                    draw_mask_word(candidate, x_group + mask_part,
                                   k_masks[champion][stat][mask_part],
                                   BAR_TOP + row, color);
                }
            }
        }
    }
    memcpy(framebuffer, candidate, sizeof(candidate));
    return 1;
}
