#include "firestaff_amg_decode.h"

#include <stdio.h>
#include <string.h>

static uint16_t rd16_be(const uint8_t* p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int FirestaffAmgSnd2_Decode(const uint8_t* data, size_t data_size,
                            FirestaffAmgSnd2* out) {
    uint16_t sample_count;
    size_t expected_min;
    size_t trailing;

    if (!data || !out || data_size < FIRESTAFF_AMG_SND2_HEADER_BYTES) {
        return -1;
    }
    memset(out, 0, sizeof(*out));

    sample_count = rd16_be(data);
    expected_min = FIRESTAFF_AMG_SND2_HEADER_BYTES + (size_t)sample_count;
    if (data_size < expected_min) {
        return -2;
    }

    trailing = data_size - expected_min;
    if (trailing > FIRESTAFF_AMG_SND2_MAX_TRAILING_BYTES) {
        return -3;
    }

    out->sample_count = sample_count;
    out->samples = (const int8_t*)(const void*)(data + FIRESTAFF_AMG_SND2_HEADER_BYTES);
    out->sample_bytes = (size_t)sample_count;
    out->trailing = data + expected_min;
    out->trailing_bytes = trailing;
    return 0;
}

uint32_t FirestaffAmgSnd2_RateHzForPeriod(uint32_t cpu_clock_hz,
                                          uint16_t period) {
    uint32_t divisor;
    /* dmweb Data Files SND2: sample_rate = CPUclock / (2 * period).
     * ReDMCSB SWSHSND.C:F0908 lines 19/139 pass G0744_i_Period to
     * audio.device's ioa_Period field. */
    if (cpu_clock_hz == 0 || period == 0) return 0;
    divisor = (uint32_t)period * 2u;
    return (cpu_clock_hz + (divisor / 2u)) / divisor;
}

uint32_t FirestaffAmgSnd2_RateHzForPlaybackSpeed(uint32_t cpu_clock_hz,
                                                 uint8_t playback_speed) {
    uint32_t period;
    if (cpu_clock_hz == 0 || playback_speed == 0) return 0;

    period = (72800u + ((uint32_t)playback_speed / 2u)) /
             (uint32_t)playback_speed;
    if (period == 0 || period > UINT16_MAX) return 0;
    return FirestaffAmgSnd2_RateHzForPeriod(cpu_clock_hz, (uint16_t)period);
}

#define ST_ASSERT(cond, msg) do {                                      \
    if (!(cond)) {                                                     \
        fprintf(stderr, "%s:%d: %s (%s)\n", __FILE__, __LINE__,       \
                msg, #cond);                                          \
        return 0;                                                      \
    }                                                                 \
} while (0)

static int test_valid_snd2(void) {
    static const uint8_t data[] = {
        0x00, 0x04,
        0x80, 0xFF, 0x00, 0x7F,
        0xAA, 0xBB
    };
    FirestaffAmgSnd2 snd;
    int rc = FirestaffAmgSnd2_Decode(data, sizeof(data), &snd);
    ST_ASSERT(rc == 0, "valid SND2 decodes");
    ST_ASSERT(snd.sample_count == 4, "sample count");
    ST_ASSERT(snd.sample_bytes == 4, "sample bytes");
    ST_ASSERT(snd.samples != NULL, "sample pointer");
    ST_ASSERT(snd.samples[0] == (int8_t)-128, "signed sample min");
    ST_ASSERT(snd.samples[1] == (int8_t)-1, "signed sample -1");
    ST_ASSERT(snd.samples[2] == 0, "signed sample zero");
    ST_ASSERT(snd.samples[3] == 127, "signed sample max");
    ST_ASSERT(snd.trailing_bytes == 2, "trailing bytes");
    ST_ASSERT(snd.trailing[0] == 0xAA && snd.trailing[1] == 0xBB, "trailing data");
    return 1;
}

static int test_zero_sample_snd2(void) {
    static const uint8_t data[] = {0x00, 0x00, 0x11, 0x22, 0x33};
    FirestaffAmgSnd2 snd;
    int rc = FirestaffAmgSnd2_Decode(data, sizeof(data), &snd);
    ST_ASSERT(rc == 0, "zero-sample SND2 with 3 trailing bytes decodes");
    ST_ASSERT(snd.sample_count == 0, "zero sample count");
    ST_ASSERT(snd.sample_bytes == 0, "zero sample bytes");
    ST_ASSERT(snd.trailing_bytes == 3, "three trailing bytes");
    return 1;
}

static int test_too_short(void) {
    static const uint8_t data[] = {0x00};
    FirestaffAmgSnd2 snd;
    int rc = FirestaffAmgSnd2_Decode(data, sizeof(data), &snd);
    ST_ASSERT(rc == -1, "too-short input rejected");
    return 1;
}

static int test_count_exceeds_data(void) {
    static const uint8_t data[] = {0x00, 0x04, 0x01, 0x02, 0x03};
    FirestaffAmgSnd2 snd;
    int rc = FirestaffAmgSnd2_Decode(data, sizeof(data), &snd);
    ST_ASSERT(rc == -2, "declared sample count must fit");
    return 1;
}

static int test_too_much_trailing(void) {
    static const uint8_t data[] = {0x00, 0x01, 0x00, 0x11, 0x22, 0x33, 0x44};
    FirestaffAmgSnd2 snd;
    int rc = FirestaffAmgSnd2_Decode(data, sizeof(data), &snd);
    ST_ASSERT(rc == -3, "more than 3 trailing bytes rejected");
    return 1;
}

static int test_rate_helpers(void) {
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPeriod(
                  FIRESTAFF_AMG_AMIGA_PAL_CLOCK_HZ, 334) == 10619,
              "PAL period 334 rate");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPeriod(
                  FIRESTAFF_AMG_AMIGA_NTSC_CLOCK_HZ, 334) == 10717,
              "NTSC period 334 rate");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPlaybackSpeed(
                  FIRESTAFF_AMG_AMIGA_PAL_CLOCK_HZ, 112) == 5457,
              "PAL speed 112 rate");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPlaybackSpeed(
                  FIRESTAFF_AMG_AMIGA_NTSC_CLOCK_HZ, 112) == 5507,
              "NTSC speed 112 rate");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPlaybackSpeed(
                  FIRESTAFF_AMG_AMIGA_PAL_CLOCK_HZ, 138) == 6718,
              "PAL speed 138 rate");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPlaybackSpeed(
                  FIRESTAFF_AMG_AMIGA_NTSC_CLOCK_HZ, 138) == 6779,
              "NTSC speed 138 rate");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPlaybackSpeed(
                  FIRESTAFF_AMG_AMIGA_PAL_CLOCK_HZ, 150) == 7313,
              "PAL speed 150 rate");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPlaybackSpeed(
                  FIRESTAFF_AMG_AMIGA_NTSC_CLOCK_HZ, 150) == 7381,
              "NTSC speed 150 rate");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPeriod(0, 334) == 0, "zero clock");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPeriod(
                  FIRESTAFF_AMG_AMIGA_PAL_CLOCK_HZ, 0) == 0,
              "zero period");
    ST_ASSERT(FirestaffAmgSnd2_RateHzForPlaybackSpeed(
                  FIRESTAFF_AMG_AMIGA_PAL_CLOCK_HZ, 0) == 0,
              "zero playback speed");
    return 1;
}

int FirestaffAmgSnd2_SelfTest(void) {
    int total = 0;
    int passed = 0;
#define RUN(test_fn) do { total++; if (test_fn()) passed++; } while (0)
    RUN(test_valid_snd2);
    RUN(test_zero_sample_snd2);
    RUN(test_too_short);
    RUN(test_count_exceeds_data);
    RUN(test_too_much_trailing);
    RUN(test_rate_helpers);
#undef RUN

    if (passed != total) {
        fprintf(stderr, "firestaff_amg_decode self-test: %d/%d passed\n",
                passed, total);
        return -1;
    }
    return 0;
}
