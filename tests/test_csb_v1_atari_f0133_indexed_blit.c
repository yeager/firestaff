#include "csb_v1_atari_f0133_indexed_blit.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    uint8_t source[64];
    uint8_t destination[60];
    uint8_t palette[16];
    int index;
    int result;

    for (index = 0; index < 64; ++index) {
        source[index] = (uint8_t)((index / 16) + 1);
    }
    source[2u * 16u + 3u] = 10u;
    for (index = 0; index < 16; ++index) palette[index] = (uint8_t)index;
    palette[3] = 9;
    memset(destination, 0xee, sizeof(destination));

    /* last=4, first=2 copies units 2,3, then wraps to units 0,1. */
    result = csb_v1_atari_f0133_fill_indexed_box(
        source, 32, 2, 4, 2, destination, 20, 2, 1, 18, 2, 10, palette);
    if (result != 35 || destination[1u * 20u + 2u] != 9u ||
        destination[1u * 20u + 2u + 3u] != 0xeeu ||
        destination[1u * 20u + 18u] != 4u ||
        destination[2u * 20u] != 0xeeu ||
        destination[2u * 20u + 2u] != 1u ||
        destination[2u * 20u + 18u] != 2u) {
        fprintf(stderr, "F0133 indexed-unit wrap/clip/palette/transparency mismatch: %d\n",
                result);
        return 1;
    }
    if (csb_v1_atari_f0133_fill_indexed_box(
            source, 32, 2, 1, 2, destination, 20, 0, 0, 16, 1, 10,
            NULL) != -1 ||
        csb_v1_atari_f0133_fill_indexed_box(
            source, 31, 2, 4, 2, destination, 20, 0, 0, 16, 1, 10,
            NULL) != -1) {
        fprintf(stderr, "F0133 accepted an invalid source-unit range\n");
        return 2;
    }
    puts("PASS: ReDMCSB F0133 indexed unit wrap and transparent box fill");
    return 0;
}
