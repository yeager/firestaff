#include "redmcsb_f0550_video_fill_screen_box_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static unsigned int be16(const unsigned char *p)
{
    return ((unsigned int)p[0] << 8) | p[1];
}

static void test_word_box_solid_planar_fill(void)
{
    unsigned char bitmap[16];
    const int16_t box[4] = {0, 1, 0, 0};

    memset(bitmap, 0xff, sizeof(bitmap));
    CHECK(F0550_VIDEO_FillScreenBox_PC34(bitmap, sizeof(bitmap), 8, 2, box,
                                         false, 0x0005));

    CHECK(be16(bitmap + 0) == 0xffffU);
    CHECK(be16(bitmap + 2) == 0x3fffU);
    CHECK(be16(bitmap + 4) == 0xffffU);
    CHECK(be16(bitmap + 6) == 0x3fffU);
    CHECK(be16(bitmap + 8) == 0xffffU);
}

static void test_byte_box_and_shade_fill(void)
{
    unsigned char bitmap[16];
    const unsigned char box[4] = {0, 1, 0, 1};

    memset(bitmap, 0, sizeof(bitmap));
    CHECK(F0550_VIDEO_FillScreenBox(bitmap, sizeof(bitmap), 8, 2, box, true,
                                    0x8001));

    CHECK(be16(bitmap + 0) == 0x4000U);
    CHECK(be16(bitmap + 2) == 0x0000U);
    CHECK(be16(bitmap + 8) == 0x8000U);
    CHECK(be16(bitmap + 10) == 0x0000U);
}

static void test_invalid_inputs_fail_closed(void)
{
    unsigned char bitmap[8];
    const int16_t bad_box[4] = {3, 1, 0, 0};
    const int16_t good_box[4] = {0, 0, 0, 0};

    memset(bitmap, 0xaa, sizeof(bitmap));
    CHECK(!F0550_VIDEO_FillScreenBox_PC34(bitmap, sizeof(bitmap), 8, 1,
                                          bad_box, false, 1));
    CHECK(bitmap[0] == 0xaaU);
    CHECK(!F0550_VIDEO_FillScreenBox_PC34(NULL, sizeof(bitmap), 8, 1,
                                          good_box, false, 1));
    CHECK(!F0550_VIDEO_FillScreenBox_PC34(bitmap, sizeof(bitmap), 7, 1,
                                          good_box, false, 1));
}

static void test_source_evidence(void)
{
    const char *evidence =
        redmcsb_f0550_video_fill_screen_box_pc34_compat_source_evidence();

    CHECK(evidence != NULL);
    CHECK(strstr(evidence, "F0550_VIDEO_FillScreenBox") != NULL);
    CHECK(strstr(evidence, "AMIGA.H:351") != NULL);
}

int main(void)
{
    test_word_box_solid_planar_fill();
    test_byte_box_and_shade_fill();
    test_invalid_inputs_fail_closed();
    test_source_evidence();
    return 0;
}
