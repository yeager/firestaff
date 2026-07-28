#include "csb_v1_startup_img3_decode_pc34_compat.h"

#include <string.h>

int main(void)
{
    uint8_t pixels[16] = {0};
    uint8_t title[320 * 153];
    CSB_V1_StartupGraphicDecodeReceipt_PC34 receipt;

    /* Canonical PC3.4 CSB startup entries are big-endian IMG2 after the
     * archive/LZW boundary.  Command 0x3A emits four pixels of index A. */
    {
        const uint8_t img[] = {
            0x00, 0x04, 0x00, 0x01,
            0x3a
        };
        memset(pixels, 0, sizeof(pixels));
        if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img), 4u, 1u, pixels, sizeof(pixels)) ||
            pixels[0] != 10u || pixels[3] != 10u) return 1;
    }

    /* ReDMCSB IMAGE3.C format: little-endian width/height, six local
     * palette nibbles, then command nibbles.  0x8 is palette slot 0 with
     * a count; the following 2 means four indexed pixels. */
    {
        const uint8_t img[] = {
            0x04, 0x00, 0x01, 0x00,
            0x12, 0x34, 0x56,
            0x82
        };
        memset(&receipt, 0, sizeof(receipt));
        if (!csb_v1_startup_img3_decode_to_indexed_with_receipt_pc34_compat(
                img, sizeof(img), 4u, 1u, pixels, sizeof(pixels), &receipt) ||
            pixels[0] != 1u || pixels[3] != 1u || !receipt.valid ||
            receipt.stream_bytes_consumed != sizeof(img) ||
            receipt.emitted_planar_pixels != 4u ||
            !receipt.ended_at_record_boundary) return 2;
    }

    /* Literal colour uses kind 7 and the immediate following nibble. */
    {
        const uint8_t img[] = {
            0x03, 0x00, 0x01, 0x00,
            0x12, 0x34, 0x56,
            0xF9, 0x10
        };
        memset(pixels, 0, sizeof(pixels));
        if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img), 3u, 1u, pixels, sizeof(pixels)) ||
            pixels[0] != 9u || pixels[2] != 9u) return 3;
    }

    /* Kind 6 copies output from one preceding scanline. */
    {
        const uint8_t img[] = {
            0x02, 0x00, 0x02, 0x00,
            0x12, 0x34, 0x56,
            0x80, 0x66
        };
        memset(pixels, 0, sizeof(pixels));
        if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img), 2u, 2u, pixels, sizeof(pixels)) ||
            pixels[0] != 1u || pixels[1] != 1u ||
            pixels[2] != 1u || pixels[3] != 1u) return 4;
    }

    /* PC IMG2 uses its declared width as the row stride.  This covers
     * C002-like odd-width assets without Atari planar padding. */
    {
        const uint8_t img[] = { 0x00, 0x03, 0x00, 0x01, 0x2A };
        if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
                img, sizeof(img), 3u, 1u, pixels, sizeof(pixels)) ||
            pixels[0] != 10u || pixels[2] != 10u) return 5;
    }

    memset(title, 0, sizeof(title));
    if (csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 320u, 153u)) return 6;
    title[0] = 1u;
    title[80u * 320u] = 2u;
    if (csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 320u, 153u)) return 7;
    title[137u * 320u] = 15u;
    if (!csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 320u, 153u) ||
        csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 320u, 152u) ||
        csb_v1_startup_title_c001_regions_admit_pc34_compat(
            title, 319u, 153u)) return 8;
    return 0;
}
