/*
 * test_csb_v1_amg_sound.c
 *
 * Data-free contract test for CSB Utility Disk sound-effect AMG
 * sound-effect container. Real local examples include:
 *   SWIPE.AMG    sampleByteCount=995,  controlWord=10
 *   TELE2.AMG    sampleByteCount=1506, controlWord=11
 *   MAGEXPLO.AMG sampleByteCount=3107, controlWord=9
 *   DRAGON.AMG   sampleByteCount=4003, controlWord=6
 *   EXPLOS1.AMG  sampleByteCount=3970, controlWord=95
 */

#include "csb_v1_amg_sound.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(int ok, const char* name)
{
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

static void test_valid_sound(void)
{
    uint8_t data[] = {
        0x00, 0x06, 0x00, 0x0a,
        0x00, 0x7f, 0x80, 0xff, 0x20, 0xe0
    };
    CsbV1AmgSoundInfo info;
    memset(&info, 0, sizeof(info));

    check(csb_v1_amg_sound_parse(data, sizeof(data), &info) == 0,
          "valid AMG sound parses");
    check(info.sampleByteCount == 6u, "sample byte count is big-endian");
    check(info.controlWord == 10u, "control word is big-endian");
    check(info.sampleOffset == 4u, "sample offset is 4");
    check(info.minSample == (int8_t)0x80, "min signed sample is tracked");
    check(info.maxSample == (int8_t)0x7f, "max signed sample is tracked");
}

static void test_rejections(void)
{
    uint8_t naked_like[] = {
        0x00, 0x06, 0x00, 0x12,
        0x4d, 0xd4, 0x4e, 0x4e, 0xc3, 0xd8, 0x74, 0x88
    };
    uint8_t zero_control[] = { 0x00, 0x01, 0x00, 0x00, 0x7f };
    uint8_t zero_samples[] = { 0x00, 0x00, 0x00, 0x01 };
    CsbV1AmgSoundInfo info;

    check(csb_v1_amg_sound_parse(NULL, sizeof(naked_like), &info) == -1,
          "NULL data rejected");
    check(csb_v1_amg_sound_parse(naked_like, sizeof(naked_like), NULL) == -1,
          "NULL output rejected");
    check(csb_v1_amg_sound_parse(naked_like, sizeof(naked_like), &info) == -2,
          "NAKED.AMG-like non sound-effect AMG container rejected");
    check(csb_v1_amg_sound_parse(zero_control, sizeof(zero_control), &info) == -3,
          "zero control word rejected");
    check(csb_v1_amg_sound_parse(zero_samples, sizeof(zero_samples), &info) == -1,
          "too-small zero-sample file rejected");
}

static void test_sample_conversion(void)
{
    check(csb_v1_amg_sound_sample_to_s16(0x00) == 0,
          "sample 0x00 -> 0");
    check(csb_v1_amg_sound_sample_to_s16(0x7f) == 32512,
          "sample 0x7f -> positive peak");
    check(csb_v1_amg_sound_sample_to_s16(0x80) == -32768,
          "sample 0x80 -> negative peak");
    check(csb_v1_amg_sound_sample_to_s16(0xff) == -256,
          "sample 0xff -> -256");
}

int main(void)
{
    test_valid_sound();
    test_rejections();
    test_sample_conversion();
    if (failures) {
        printf("test_csb_v1_amg_sound: FAIL %d\n", failures);
        return 1;
    }
    puts("test_csb_v1_amg_sound: PASS");
    return 0;
}
