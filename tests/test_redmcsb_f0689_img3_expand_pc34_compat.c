#include "redmcsb_f0689_img3_expand_pc34_compat.h"

#include <stdint.h>

int main(void)
{
    const uint8_t source[] = {2, 0, 1, 0, 0x09, 0x80, 0, 0x12};
    uint8_t destination[1] = {0};
    uint16_t width, height;

    if (!redmcsb_f0689_img3_expand_even_pc34_compat(
            source, sizeof(source), destination, sizeof(destination), &width, &height) ||
        width != 2U || height != 1U || destination[0] != 0x98U) return 1;
    if (redmcsb_f0689_img3_expand_even_pc34_compat(
            source, sizeof(source), destination, SIZE_MAX / 2U + 1U, NULL, NULL))
        return 2;
    return 0;
}
