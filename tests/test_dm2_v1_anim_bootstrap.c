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
    DM2_V1_AnimFileReceipt file_receipt;
    uint8_t pixels[16];
    uint8_t file_bytes[40000];
    uint8_t file_read[40000];
    char text[32];
    void *heap_block;
    const char *tmp_path = "/tmp/firestaff_dm2_anim_bootstrap_test.bin";
    FILE *tmp;
    int handle;
    size_t i;
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

    for (i = 0; i < sizeof(file_bytes); ++i) {
        file_bytes[i] = (uint8_t)(i & 0xffu);
    }
    tmp = fopen(tmp_path, "wb");
    check(tmp != NULL, "test fixture file opens for writing");
    if (tmp) {
        check(fwrite(file_bytes, 1, sizeof(file_bytes), tmp) ==
                  sizeof(file_bytes),
              "test fixture file writes all bytes");
        fclose(tmp);
    }
    memset(file_read, 0, sizeof(file_read));
    handle = dm2_v1_anim_file_open(tmp_path, &file_receipt);
    check(handle > 0 && file_receipt.valid &&
              file_receipt.source_line == 915,
          "ANIM_FILE_OPEN returns a source-mapped handle");
    check(dm2_v1_anim_get_file_size(handle, &file_receipt) ==
              sizeof(file_bytes) &&
              file_receipt.file_size == sizeof(file_bytes) &&
              file_receipt.source_line == 924,
          "ANIM_GET_FILE_SIZE preserves the file position and size");
    check(dm2_v1_anim_read_huge_file(handle,
                                     (uint32_t)sizeof(file_read),
                                     file_read,
                                     &file_receipt) &&
              memcmp(file_bytes, file_read, sizeof(file_bytes)) == 0 &&
              file_receipt.chunk_count == 2 &&
              file_receipt.source_line == 939,
          "ANIM_READ_HUGE_FILE reads in 0x8000-sized chunks");
    dm2_v1_anim_file_close(handle, &file_receipt);
    check(file_receipt.valid && file_receipt.source_line == 970,
          "ANIM_FILE_CLOSE closes the source-mapped handle");
    remove(tmp_path);

    check(dm2_v1_anim_strcpy(text, "anim", &file_receipt) == text &&
              strcmp(text, "anim") == 0 &&
              file_receipt.source_line == 984,
          "ANIM_STRCPY returns the destination like strcpy");
    check(dm2_v1_anim_toupper('q', &file_receipt) == 'Q' &&
              dm2_v1_anim_toupper(-1, &file_receipt) == -1 &&
              file_receipt.source_line == 1308,
          "ANIM_TOUPPER keeps EOF and uppercases lowercase ASCII");
    heap_block = dm2_v1_anim_farmalloc(64, &file_receipt);
    check(heap_block != NULL && file_receipt.valid &&
              file_receipt.requested_bytes == 64 &&
              file_receipt.source_line == 934,
          "ANIM_farmalloc returns malloc-backed memory with source receipt");
    dm2_v1_anim_farfree(heap_block, &file_receipt);
    check(file_receipt.valid && file_receipt.source_line == 1054,
          "ANIM_farfree releases malloc-backed memory");
    check(dm2_v1_anim_farcoreleft(&file_receipt) == 1024u * 1024u &&
              file_receipt.file_size == 1024u * 1024u &&
              file_receipt.source_line == 1060,
          "ANIM_farcoreleft exposes skproject Win32 memory pool sentinel");

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

    memset(pixels, 0, sizeof(pixels));
    check(dm2_v1_anim_blit_to_memory_row_4to4bpp(
              (const uint8_t *)"\x12\x34\x56",
              3,
              1,
              pixels,
              sizeof(pixels),
              2,
              4,
              &file_receipt) &&
              pixels[1] == 0x23u && pixels[2] == 0x45u &&
              file_receipt.source_line == 1399,
          "ANIM_BLIT_TO_MEMORY_ROW_4TO4BPP copies unaligned nibbles");

    if (failures != 0) {
        printf("dm2_v1_anim_bootstrap: %d failures\n", failures);
        return 1;
    }
    puts("dm2_v1_anim_bootstrap: all assertions passed");
    return 0;
}
