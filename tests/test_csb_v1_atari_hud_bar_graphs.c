#include "csb_v1_atari_hud_bar_graphs.h"

#include <stdio.h>
#include <string.h>

enum { WIDTH = 320, HEIGHT = 200 };

static int expect_pixel(const uint8_t *pixels, int x, int y,
                        uint8_t expected, const char *name)
{
    const uint8_t actual = pixels[y * WIDTH + x];
    if (actual == expected) return 1;
    fprintf(stderr, "%s: pixel(%d,%d)=%u expected %u\n",
            name, x, y, actual, expected);
    return 0;
}

int main(void)
{
    uint8_t pixels[WIDTH * HEIGHT];
    CSB_V1_AtariHudChampionBars champions[4];

    memset(pixels, 3, sizeof(pixels));
    memset(champions, 0, sizeof(champions));
    champions[0].present = 1;
    champions[0].statistics_enabled = 1;
    champions[0].current[0] = 50u;
    champions[0].maximum[0] = 100u;
    champions[0].current[1] = 100u; /* stamina: F0287's second computed height */
    champions[0].maximum[1] = 100u;
    champions[0].current[2] = 0u; /* mana: F0287's third computed height */
    champions[0].maximum[2] = 100u;
    if (!csb_v1_atari_hud_draw_bar_graphs(pixels, sizeof(pixels), champions))
        return 1;
    /* G0056 health offset 16 and G0055 masks place the bar at x=46..49. */
    if (!expect_pixel(pixels, 46, 2, 12u, "health empty top") ||
        !expect_pixel(pixels, 46, 13, 12u, "health empty mid") ||
        !expect_pixel(pixels, 46, 14, 7u, "health filled boundary") ||
        !expect_pixel(pixels, 46, 26, 7u, "health filled bottom") ||
        !expect_pixel(pixels, 53, 2, 7u, "stamina full") ||
        !expect_pixel(pixels, 53, 26, 7u, "stamina full bottom") ||
        !expect_pixel(pixels, 60, 2, 12u, "mana empty") ||
        !expect_pixel(pixels, 60, 26, 12u, "mana zero") ||
        !expect_pixel(pixels, 40, 2, 3u, "outside G0055 remains untouched"))
        return 2;

    memset(pixels, 3, sizeof(pixels));
    champions[0].statistics_enabled = 0;
    if (!csb_v1_atari_hud_draw_bar_graphs(pixels, sizeof(pixels), champions) ||
        pixels[46 + WIDTH * 2] != 3u) return 3;

    champions[0].statistics_enabled = 1;
    champions[0].current[0] = 1u;
    champions[0].maximum[0] = 0u;
    memset(pixels, 3, sizeof(pixels));
    if (csb_v1_atari_hud_draw_bar_graphs(pixels, sizeof(pixels), champions) ||
        pixels[46 + WIDTH * 2] != 3u) return 4;
    puts("PASS: Atari F0287 bar graph masks, heights, colors and admission");
    return 0;
}
