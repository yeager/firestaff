#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool dm1_v1_viewport_d0l2_d0r2_f0108_flip_row_pc34(
    const uint8_t *source, uint8_t *destination, size_t width, size_t rows);
bool dm1_v1_viewport_d1l2_d1r2_f0108_flip_row_pc34(
    const uint8_t *source, uint8_t *destination, size_t width, size_t rows);

typedef bool (*FlipRows)(const uint8_t *, uint8_t *, size_t, size_t);

static int run_lane(const char *lane, FlipRows flip)
{
    const uint8_t expected[] = { 4u, 3u, 2u, 1u, 8u, 7u, 6u, 5u };
    uint8_t pixels[] = { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u };
    uint8_t copied[sizeof(pixels)] = { 0 };

    if (!flip(pixels, pixels, 4u, 2u) || memcmp(pixels, expected, sizeof(pixels)) != 0) {
        fprintf(stderr, "%s in-place F0099 row flip failed\n", lane);
        return 1;
    }
    if (!flip(expected, copied, 4u, 2u) ||
        copied[0] != 1u || copied[3] != 4u || copied[4] != 5u || copied[7] != 8u) {
        fprintf(stderr, "%s distinct-buffer F0099 row flip changed\n", lane);
        return 1;
    }
    return 0;
}

int main(void)
{
    return run_lane("D0L2/D0R2", dm1_v1_viewport_d0l2_d0r2_f0108_flip_row_pc34) |
        run_lane("D1L2/D1R2", dm1_v1_viewport_d1l2_d1r2_f0108_flip_row_pc34);
}
