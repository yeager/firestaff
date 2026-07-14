#include "csb_v1_startup_img3_decode_pc34_compat.h"

int main(void)
{
    const uint8_t odd_width[] = {
        3, 0, 2, 0, 0x12, 0x34, 0x56, 0x01, 0x26, 0x79, 0x50
    };
    uint8_t pixels[6] = {0};
    uint8_t rejected[6] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

    if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
            odd_width, sizeof(odd_width), 3, 2, pixels, sizeof(pixels)) ||
        pixels[0] != 1 || pixels[1] != 2 || pixels[2] != 3 ||
        pixels[3] != 1 || pixels[4] != 9 || pixels[5] != 6) {
        return 1;
    }
    if (csb_v1_startup_img3_decode_to_indexed_pc34_compat(
            odd_width, sizeof(odd_width) - 1U, 3, 2, rejected,
            sizeof(rejected)) || rejected[0] != 0xaa || rejected[5] != 0xaa) {
        return 1;
    }
    return 0;
}
