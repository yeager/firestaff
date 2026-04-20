#include <stdio.h>
#include <string.h>

#include "image_backend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_F0685_single_high_nibble_write(void) {
    unsigned char buf[2] = {0xAB, 0xCD};
    G2160_puc_Bitmap_Destination = buf;
    F0685_IMG3_LineColorFilling(0, 0x5, 1);
    return buf[0] == 0x5B && buf[1] == 0xCD;
}

static int test_F0685_single_low_nibble_write(void) {
    unsigned char buf[2] = {0xAB, 0xCD};
    G2160_puc_Bitmap_Destination = buf;
    F0685_IMG3_LineColorFilling(1, 0x4, 1);
    return buf[0] == 0xA4 && buf[1] == 0xCD;
}

static int test_F0685_fill_across_nibbles(void) {
    unsigned char buf[4] = {0x00, 0x00, 0x00, 0x00};
    G2160_puc_Bitmap_Destination = buf;
    F0685_IMG3_LineColorFilling(1, 0xA, 5);
    return buf[0] == 0x0A && buf[1] == 0xAA && buf[2] == 0xAA;
}

static int test_F0687_get_nibbles_in_sequence(void) {
    unsigned char src[2] = {0xAB, 0xCD};
    G2159_puc_Bitmap_Source = src;
    G2157_ = 0;
    return F0687_IMG3_GetNibble() == 0xA &&
           F0687_IMG3_GetNibble() == 0xB &&
           F0687_IMG3_GetNibble() == 0xC &&
           F0687_IMG3_GetNibble() == 0xD &&
           G2157_ == 4;
}

static int test_F0688_get_pixel_count_short_form(void) {
    unsigned char src[1] = {0x30};
    G2159_puc_Bitmap_Source = src;
    G2157_ = 0;
    return F0688_IMG3_GetPixelCount() == 5 && G2157_ == 1;
}

static int test_F0688_get_pixel_count_medium_form(void) {
    unsigned char src[2] = {0xF2, 0x0};
    G2159_puc_Bitmap_Source = src;
    G2157_ = 0;
    return F0688_IMG3_GetPixelCount() == 0x20 + 17 && G2157_ == 3;
}

static int test_F0688_get_pixel_count_long_form(void) {
    unsigned char src[4] = {0xFF, 0xF1, 0x23, 0x40};
    G2159_puc_Bitmap_Source = src;
    G2157_ = 0;
    return F0688_IMG3_GetPixelCount() == 0x1234 && G2157_ == 7;
}

static int test_F0686_copy_from_previous_line_even_to_even(void) {
    unsigned char buf[4] = {0x12, 0x34, 0x56, 0x78};
    G2160_puc_Bitmap_Destination = buf;
    F0686_IMG_CopyFromPreviousLine(4, 0, 4);
    return buf[0] == 0x12 && buf[1] == 0x34 && buf[2] == 0x12 && buf[3] == 0x34;
}

static int test_F0686_copy_from_previous_line_odd_alignment(void) {
    unsigned char buf[4] = {0x12, 0x34, 0x56, 0x78};
    G2160_puc_Bitmap_Destination = buf;
    F0686_IMG_CopyFromPreviousLine(1, 4, 4);
    return buf[0] == 0x15 && buf[1] == 0x67 && buf[2] == 0x86 && buf[3] == 0x78;
}

static int test_IMG3_Compat_ExpandOneCommandSameStride_palette_fill(void) {
    unsigned char src[2] = {0x91, 0x00};
    unsigned char dst[4] = {0x00, 0x00, 0x00, 0x00};
    unsigned char palette[6] = {1, 2, 3, 4, 5, 6};
    unsigned short dstPos = 0;
    G2159_puc_Bitmap_Source = src;
    G2160_puc_Bitmap_Destination = dst;
    G2157_ = 0;
    IMG3_Compat_ExpandOneCommandSameStride(palette, 4, &dstPos);
    return dst[0] == 0x22 && dst[1] == 0x20 && dstPos == 3 && G2157_ == 2;
}

static int test_IMG3_Compat_ExpandOneCommandSameStride_copy_previous(void) {
    unsigned char src[1] = {0x6};
    unsigned char dst[4] = {0x12, 0x34, 0x00, 0x00};
    unsigned char palette[6] = {1, 2, 3, 4, 5, 6};
    unsigned short dstPos = 4;
    G2159_puc_Bitmap_Source = src;
    G2160_puc_Bitmap_Destination = dst;
    G2157_ = 0;
    IMG3_Compat_ExpandOneCommandSameStride(palette, 4, &dstPos);
    return dst[0] == 0x12 && dst[1] == 0x34 && dst[2] == 0x10 && dst[3] == 0x00 && dstPos == 5 && G2157_ == 1;
}

static int test_IMG3_Compat_ExpandOneCommandPadded_palette_fill_wrap(void) {
    unsigned char src[2] = {0x92, 0x00};
    unsigned char dst[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char palette[6] = {1, 2, 3, 4, 5, 6};
    unsigned short dstPos = 0;
    unsigned short remaining = 3;
    G2159_puc_Bitmap_Source = src;
    G2160_puc_Bitmap_Destination = dst;
    G2157_ = 0;
    IMG3_Compat_ExpandOneCommandPadded(palette, 3, 4, &dstPos, &remaining);
    return dst[0] == 0x22 && dst[1] == 0x20 && dst[2] == 0x20 && dstPos == 5 && remaining == 2 && G2157_ == 2;
}

static int test_IMG3_Compat_ExpandOneCommandPadded_copy_wrap(void) {
    unsigned char src[2] = {0xE2, 0x00};
    unsigned char dst[6] = {0x12, 0x30, 0x45, 0x60, 0x00, 0x00};
    unsigned char palette[6] = {1, 2, 3, 4, 5, 6};
    unsigned short dstPos = 4;
    unsigned short remaining = 3;
    G2159_puc_Bitmap_Source = src;
    G2160_puc_Bitmap_Destination = dst;
    G2157_ = 0;
    IMG3_Compat_ExpandOneCommandPadded(palette, 3, 4, &dstPos, &remaining);
    return dst[0] == 0x12 && dst[1] == 0x30 && dst[2] == 0x12 && dst[3] == 0x30 && dst[4] == 0x10 && dstPos == 9 && remaining == 2 && G2157_ == 2;
}

static int test_IMG3_Compat_ExpandCommandsSameStride_two_commands(void) {
    unsigned char src[1] = {0x11};
    unsigned char dst[4] = {0x00, 0x00, 0x00, 0x00};
    unsigned char palette[6] = {1, 2, 3, 4, 5, 6};
    unsigned short dstPos = 0;
    G2159_puc_Bitmap_Source = src;
    G2160_puc_Bitmap_Destination = dst;
    G2157_ = 0;
    IMG3_Compat_ExpandCommandsSameStride(palette, 4, 2, &dstPos);
    return dst[0] == 0x22 && dst[1] == 0x00 && dstPos == 2 && G2157_ == 2;
}

static int test_IMG3_Compat_ExpandCommandsPadded_two_commands(void) {
    unsigned char src[1] = {0x11};
    unsigned char dst[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char palette[6] = {1, 2, 3, 4, 5, 6};
    unsigned short dstPos = 0;
    unsigned short remaining = 3;
    G2159_puc_Bitmap_Source = src;
    G2160_puc_Bitmap_Destination = dst;
    G2157_ = 0;
    IMG3_Compat_ExpandCommandsPadded(palette, 3, 4, 2, &dstPos, &remaining);
    return dst[0] == 0x22 && dst[1] == 0x00 && dstPos == 2 && remaining == 1 && G2157_ == 2;
}

static int test_IMG3_Compat_ExpandFromSource_same_stride(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char dst[2] = {0x00, 0x00};
    IMG3_Compat_ExpandFromSource(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x00;
}

static int test_IMG3_Compat_ExpandFromSource_padded_copy_second_row(void) {
    unsigned char src[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char dst[4] = {0x00, 0x00, 0x00, 0x00};
    IMG3_Compat_ExpandFromSource(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x20 && dst[2] == 0x22 && dst[3] == 0x20;
}

static int test_IMG3_Compat_ExpandFromSource_same_stride_literal_color(void) {
    unsigned char src[10] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0xFB, 0x00, 0x00};
    unsigned char dst[2] = {0x00, 0x00};
    IMG3_Compat_ExpandFromSource(src, dst);
    return dst[0] == 0xBB && dst[1] == 0x00;
}

static int test_IMG3_Compat_ExpandFromSource_same_stride_literal_color_with_count(void) {
    unsigned char src[10] = {0x04, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0xF0, 0x20, 0x00};
    unsigned char dst[2] = {0xFF, 0xFF};
    IMG3_Compat_ExpandFromSource(src, dst);
    return dst[0] == 0x00 && dst[1] == 0x00;
}

static int test_IMG3_Compat_ExpandFromSource_same_stride_copy_with_count(void) {
    unsigned char src[9] = {0x02, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE0};
    unsigned char dst[2] = {0x00, 0x00};
    IMG3_Compat_ExpandFromSource(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x22;
}

static int test_IMG3_Compat_ExpandFromSource_padded_literal_color_with_count(void) {
    unsigned char src[10] = {0x03, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0xFB, 0x10, 0x00};
    unsigned char dst[2] = {0x00, 0x00};
    IMG3_Compat_ExpandFromSource(src, dst);
    return dst[0] == 0xBB && dst[1] == 0xB0;
}

int main(void) {
    if (!test_F0685_single_high_nibble_write()) {
        fprintf(stderr, "test_F0685_single_high_nibble_write failed\n");
        return 1;
    }
    if (!test_F0685_single_low_nibble_write()) {
        fprintf(stderr, "test_F0685_single_low_nibble_write failed\n");
        return 1;
    }
    if (!test_F0685_fill_across_nibbles()) {
        fprintf(stderr, "test_F0685_fill_across_nibbles failed\n");
        return 1;
    }
    if (!test_F0687_get_nibbles_in_sequence()) {
        fprintf(stderr, "test_F0687_get_nibbles_in_sequence failed\n");
        return 1;
    }
    if (!test_F0688_get_pixel_count_short_form()) {
        fprintf(stderr, "test_F0688_get_pixel_count_short_form failed\n");
        return 1;
    }
    if (!test_F0688_get_pixel_count_medium_form()) {
        fprintf(stderr, "test_F0688_get_pixel_count_medium_form failed\n");
        return 1;
    }
    if (!test_F0688_get_pixel_count_long_form()) {
        fprintf(stderr, "test_F0688_get_pixel_count_long_form failed\n");
        return 1;
    }
    if (!test_F0686_copy_from_previous_line_even_to_even()) {
        fprintf(stderr, "test_F0686_copy_from_previous_line_even_to_even failed\n");
        return 1;
    }
    if (!test_F0686_copy_from_previous_line_odd_alignment()) {
        fprintf(stderr, "test_F0686_copy_from_previous_line_odd_alignment failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandOneCommandSameStride_palette_fill()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandOneCommandSameStride_palette_fill failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandOneCommandSameStride_copy_previous()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandOneCommandSameStride_copy_previous failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandOneCommandPadded_palette_fill_wrap()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandOneCommandPadded_palette_fill_wrap failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandOneCommandPadded_copy_wrap()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandOneCommandPadded_copy_wrap failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandCommandsSameStride_two_commands()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandCommandsSameStride_two_commands failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandCommandsPadded_two_commands()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandCommandsPadded_two_commands failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandFromSource_same_stride()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandFromSource_same_stride failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandFromSource_padded_copy_second_row()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandFromSource_padded_copy_second_row failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandFromSource_same_stride_literal_color()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandFromSource_same_stride_literal_color failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandFromSource_same_stride_literal_color_with_count()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandFromSource_same_stride_literal_color_with_count failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandFromSource_same_stride_copy_with_count()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandFromSource_same_stride_copy_with_count failed\n");
        return 1;
    }
    if (!test_IMG3_Compat_ExpandFromSource_padded_literal_color_with_count()) {
        fprintf(stderr, "test_IMG3_Compat_ExpandFromSource_padded_literal_color_with_count failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
