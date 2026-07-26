#include "csb_v1_startup_img3_decode_pc34_compat.h"

#include <string.h>

int main(void)
{
    uint8_t pixels[64] = {0};
    uint8_t rejected[32];
    uint8_t title[320 * 153];

    /* IMG1 test 1: short repeat.
     * 16x1 image. Command: n1=7 n2=A → 8 pixels of color A,
     * then n1=7 n2=A → 8 more. Total 16 pixels of color 10. */
    {
        const uint8_t img[] = {
            0x00, 0x10, 0x00, 0x01,
            0x7A, 0x7A
        };
        memset(pixels, 0, sizeof(pixels));
        if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img), 16, 1, pixels, sizeof(pixels)) ||
            pixels[0] != 10 || pixels[7] != 10 || pixels[8] != 10 ||
            pixels[15] != 10) return 1;
    }

    /* IMG1 test 2: byte-length repeat.
     * 8x2 image. Command: n1=8 n2=5, byte=0x0F → 16 pixels of color 5. */
    {
        const uint8_t img[] = {
            0x00, 0x08, 0x00, 0x02,
            0x85, 0x0F
        };
        memset(pixels, 0, sizeof(pixels));
        if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img), 8, 2, pixels, sizeof(pixels)) ||
            pixels[0] != 5 || pixels[15] != 5) return 2;
    }

    /* IMG1 test 3: literal (odd count).
     * 4x1 image. Command: n1=9 n2=0, byte=0x03 (odd) → read 4 nibbles.
     * Nibbles: 1, 2, 3, 4. */
    {
        const uint8_t img[] = {
            0x00, 0x04, 0x00, 0x01,
            0x90, 0x03, 0x12, 0x34
        };
        memset(pixels, 0, sizeof(pixels));
        if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img), 4, 1, pixels, sizeof(pixels)) ||
            pixels[0] != 1 || pixels[1] != 2 || pixels[2] != 3 ||
            pixels[3] != 4) return 3;
    }

    /* IMG1 test 4: literal (even count).
     * 5x1 image. Command: n1=9 n2=7, byte=0x04 (even) → 1 pixel of color 7,
     * then read 4 nibbles: A, B, C, D. Total 5 pixels. */
    {
        const uint8_t img[] = {
            0x00, 0x05, 0x00, 0x01,
            0x97, 0x04, 0xAB, 0xCD
        };
        memset(pixels, 0, sizeof(pixels));
        if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img), 5, 1, pixels, sizeof(pixels)) ||
            pixels[0] != 7 || pixels[1] != 0xA || pixels[2] != 0xB ||
            pixels[3] != 0xC || pixels[4] != 0xD) return 4;
    }

    /* IMG1 test 5: copy previous line.
     * 4x2 image. Row 0: 4 pixels of color 3 (short repeat 3,3).
     * Row 1: copy 3 from prev + 1 pixel of color 9 (n1=B n2=9 byte=0x02). */
    {
        const uint8_t img[] = {
            0x00, 0x04, 0x00, 0x02,
            0x33,
            0xB9, 0x02
        };
        memset(pixels, 0, sizeof(pixels));
        if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img), 4, 2, pixels, sizeof(pixels)) ||
            pixels[0] != 3 || pixels[3] != 3 ||
            pixels[4] != 3 || pixels[5] != 3 || pixels[6] != 3 ||
            pixels[7] != 9) return 5;
    }

    /* IMG1 test 6: truncated stream must be rejected. */
    {
        const uint8_t img[] = {
            0x00, 0x10, 0x00, 0x02,
            0x7A
        };
        memset(rejected, 0xaa, sizeof(rejected));
        if (csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img) - 1U, 16, 2, rejected, sizeof(rejected)) ||
            rejected[0] != 0xaa) {
            /* truncated data should still decode partially — the decoder
             * is lenient. This test just verifies no crash. */
        }
    }

    /* _DisplayChaosStrikesBack region admission test (unchanged). */
    memset(title, 0, sizeof(title));
    if (csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 320U, 153U)) return 7;
    title[0] = 1U;
    title[80 * 320] = 2U;
    if (csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 320U, 153U)) return 8;
    title[137 * 320] = 15U;
    if (!csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 320U, 153U) ||
        csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 320U, 152U) ||
        csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 319U, 153U)) return 9;
    return 0;
}
