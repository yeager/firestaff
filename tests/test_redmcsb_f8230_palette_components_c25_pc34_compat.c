#include "redmcsb_f8230_palette_components_c25_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int calls;
    int allow;
} WaitState;

static int failures;

static bool wait_vblank(void *context)
{
    WaitState *state = (WaitState *)context;
    ++state->calls;
    return state->allow != 0;
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
    uint8_t palette[32][3];
    RedmcsbF8156C25DacPc34Compat dac;
    WaitState wait_state = {0, 1};

    memset(palette, 0, sizeof(palette));
    memset(&dac, 0xAA, sizeof(dac));
    expect_true("hidden logical update",
                redmcsb_f8230_set_single_color_components_c25_pc34_compat(
                    palette, &dac, 0U, 5, 1U, 15U, 0U, wait_vblank,
                    &wait_state));
    expect_uint("red expand", palette[5][0], 7U);
    expect_uint("green expand", palette[5][1], 63U);
    expect_uint("blue expand", palette[5][2], 3U);
    expect_uint("hidden update skips VBlank", (unsigned)wait_state.calls, 0U);
    expect_uint("hidden update skips DAC", dac.rgb6[5][0], 0xAAU);

    expect_true("visible logical update publishes",
                redmcsb_f8230_set_single_color_components_c25_pc34_compat(
                    palette, &dac, 1U, 6, 2U, 3U, 4U, wait_vblank,
                    &wait_state));
    expect_uint("visible update waits", (unsigned)wait_state.calls, 1U);
    expect_uint("published earlier palette row", dac.rgb6[5][1], 63U);
    expect_uint("published red", dac.rgb6[6][0], 11U);
    expect_uint("published green", dac.rgb6[6][1], 15U);
    expect_uint("published blue", dac.rgb6[6][2], 19U);

    wait_state.allow = 0;
    expect_true("failed VBlank reports failure",
                !redmcsb_f8230_set_single_color_components_c25_pc34_compat(
                    palette, &dac, 1U, 7, 5U, 6U, 7U, wait_vblank,
                    &wait_state));
    expect_uint("failed publish retains logical red", palette[7][0], 23U);
    expect_uint("failed publish leaves DAC", dac.rgb6[7][0], 0U);

    expect_true("component range rejected",
                !redmcsb_f8230_set_single_color_components_c25_pc34_compat(
                    palette, &dac, 0U, 7, 16U, 0U, 0U, wait_vblank,
                    &wait_state));
    if (strstr(redmcsb_f8230_palette_components_source_evidence_pc34(),
               "VIDEODRV.C:3456-3471") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8230 PC 3.4 C25 palette components");
    return 0;
}
