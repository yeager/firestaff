#include "dm2_v1_anim_bootstrap.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(int condition, const char *message)
{
    if (!condition) {
        printf("FAIL %s\n", message);
        ++failures;
    }
}

int main(void)
{
    DM2_V1_AnimBootstrapReceipt boot;
    DM2_V1_AnimDecodeImg1Receipt decode;
    uint8_t pixels[16];
    const uint8_t img_fill[] = {
        0x00, 0x04, 0x00, 0x02,
        0x33, 0x37
    };
    const uint8_t img_literal[] = {
        0x00, 0x04, 0x00, 0x01,
        0x90, 0x03, 0xab, 0xcd
    };
    const uint8_t img_previous_row[] = {
        0x00, 0x04, 0x00, 0x02,
        0x33,
        0xbe, 0x02
    };

    memset(pixels, 0, sizeof(pixels));
    check(dm2_v1_anim_setpixel_seq_4bpp(pixels, sizeof(pixels), 0, 0x0au) &&
              pixels[0] == 0xa0u,
          "ANIM_SETPIXEL_SEQ_4BPP writes high nibble for even pixel");
    check(dm2_v1_anim_setpixel_seq_4bpp(pixels, sizeof(pixels), 1, 0x05u) &&
              pixels[0] == 0xa5u,
          "ANIM_SETPIXEL_SEQ_4BPP writes low nibble for odd pixel");
    check(dm2_v1_anim_fill_seq_4bpp(pixels, sizeof(pixels), 2, 0x03u, 4) &&
              pixels[1] == 0x33u && pixels[2] == 0x33u,
          "ANIM_FILL_SEQ_4BPP fills sequential nibbles");
    check(!dm2_v1_anim_fill_seq_4bpp(pixels, sizeof(pixels), 0, 0, 0),
          "ANIM_FILL_SEQ_4BPP rejects zero count like source assert");

    check(dm2_v1_anim_bootstrap_swoosh(&boot) &&
              boot.valid && boot.argc == 4 &&
              strcmp(boot.command_line, "ANIM swoosh +pm +sb") == 0 &&
              strcmp(boot.argv[1], "swoosh") == 0 &&
              boot.source_line == 2034,
          "ANIM_BOOTSTRAP_SWOOSH owns exact skproject argv");
    check(dm2_v1_anim_bootstrap_title(&boot) &&
              boot.valid && boot.argc == 7 &&
              strcmp(boot.command_line, "ANIM title +ah +as +ab +pm +sb") == 0 &&
              strcmp(boot.argv[2], "+ah") == 0 &&
              strcmp(boot.argv[6], "+sb") == 0 &&
              boot.source_line == 2045,
          "ANIM_BOOTSTRAP_TITLE owns exact skproject argv");

    memset(pixels, 0, sizeof(pixels));
    check(dm2_v1_anim_decode_img1(img_fill, sizeof(img_fill), pixels,
                                  sizeof(pixels), &decode) &&
              decode.valid && decode.width == 4 && decode.height == 2 &&
              decode.even_width == 4 && decode.decoded_pixels == 8 &&
              decode.fill_run_count == 2 &&
              pixels[0] == 0x33u && pixels[1] == 0x33u &&
              pixels[2] == 0x77u && pixels[3] == 0x77u,
          "ANIM_DECODE_IMG1 decodes short fill runs");

    memset(pixels, 0, sizeof(pixels));
    check(dm2_v1_anim_decode_img1(img_literal, sizeof(img_literal), pixels,
                                  sizeof(pixels), &decode) &&
              decode.literal_run_count == 1 &&
              pixels[0] == 0xabu && pixels[1] == 0xcdu,
          "ANIM_DECODE_IMG1 decodes literal 4bpp runs");

    memset(pixels, 0, sizeof(pixels));
    check(dm2_v1_anim_decode_img1(img_previous_row,
                                  sizeof(img_previous_row),
                                  pixels,
                                  sizeof(pixels),
                                  &decode) &&
              decode.previous_row_run_count == 1 &&
              pixels[0] == 0x33u && pixels[1] == 0x33u &&
              pixels[2] == 0x33u && pixels[3] == 0x3eu,
          "ANIM_DECODE_IMG1 copies previous-row runs and trailing color");
    check(!dm2_v1_anim_decode_img1(img_fill, 5, pixels, sizeof(pixels), NULL),
          "ANIM_DECODE_IMG1 rejects truncated stream");

    if (failures != 0) {
        printf("dm2_v1_anim_bootstrap: %d failures\n", failures);
        return 1;
    }
    puts("dm2_v1_anim_bootstrap: all assertions passed");
    return 0;
}
