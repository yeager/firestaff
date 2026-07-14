#include "redmcsb_f8169_blacken_pixels_c25_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    uint16_t first[5];
    size_t count;
    uint16_t last;
    uint8_t last_value;
} Trace;

static int failures;

static void trace_pixel(void *context, uint16_t pixel_index, uint8_t aperture_value)
{
    Trace *trace = (Trace *)context;
    if (trace->count < 5U) trace->first[trace->count] = pixel_index;
    ++trace->count;
    trace->last = pixel_index;
    trace->last_value = aperture_value;
}

static void expect_uint(const char *name, unsigned actual, unsigned expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %u, expected %u)\n", name, actual, expected);
        ++failures;
    }
}

static void expect_true(const char *name, int actual)
{
    if (!actual) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

int main(void)
{
    uint8_t aperture[REDMCSB_F8169_SCREEN_PIXELS_PC34 + 2U];
    Trace trace;

    memset(aperture, 0xEE, sizeof(aperture));
    memset(&trace, 0, sizeof(trace));
    expect_true("F8169 LFSR blackening",
                redmcsb_f8169_blacken_all_pixels_c25_pc34_compat(
                    aperture, sizeof(aperture), 0x10U, trace_pixel, &trace));
    expect_uint("all screen pixels written", (unsigned)trace.count,
                REDMCSB_F8169_SCREEN_PIXELS_PC34);
    expect_uint("LFSR first", trace.first[0], 1U);
    expect_uint("LFSR second", trace.first[1], 0xB400U);
    expect_uint("LFSR third", trace.first[2], 0x5A00U);
    expect_uint("LFSR fourth", trace.first[3], 0x2D00U);
    expect_uint("LFSR fifth", trace.first[4], 0x1680U);
    expect_uint("final pixel is zero", trace.last, 0U);
    expect_uint("black uses viewport bank", trace.last_value, 0x10U);
    expect_uint("screen first pixel", aperture[0], 0x10U);
    expect_uint("screen last pixel", aperture[63999], 0x10U);
    expect_uint("outside screen retained", aperture[64000], 0xEEU);

    aperture[0] = 0xAAU;
    expect_true("bad offset rejected",
                !redmcsb_f8169_blacken_all_pixels_c25_pc34_compat(
                    aperture, sizeof(aperture), 0x11U, NULL, NULL));
    expect_uint("rejected offset leaves aperture", aperture[0], 0xAAU);

    if (strstr(redmcsb_f8169_blacken_pixels_source_evidence_pc34(),
               "VIDEODRV.C:3833-3848") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8169 PC 3.4 C25 LFSR aperture blackening");
    return 0;
}
