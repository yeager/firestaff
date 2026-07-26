#include "redmcsb_f8159_palette_curtain_c25_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

typedef struct {
    int calls;
    int allow;
} WaitState;

static bool wait_vblank(void *context)
{
    WaitState *state = (WaitState *)context;
    ++state->calls;
    return state->allow != 0;
}

static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %d, expected %d)\n", name, actual, expected);
        ++failures;
    }
}

int main(void)
{
    uint8_t full[32][3];
    RedmcsbF8156C25DacPc34Compat dac;
    uint8_t curtain = 7U;
    WaitState wait_state = {0, 1};
    size_t color;

    for (color = 0U; color < 32U; ++color) {
        full[color][0] = (uint8_t)color;
        full[color][1] = (uint8_t)(color + 1U);
        full[color][2] = (uint8_t)(color + 2U);
    }
    memset(&dac, 0xA5, sizeof(dac));

    expect_int("black curtain", redmcsb_f8159_palette_set_curtain_c25_pc34_compat(
                   (const uint8_t (*)[REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34])full, &dac, &curtain, REDMCSB_F8159_BLACK_PALETTE_PC34,
                   wait_vblank, &wait_state), 1);
    expect_int("black wait", wait_state.calls, 1);
    expect_int("black state", curtain, REDMCSB_F8159_BLACK_PALETTE_PC34);
    expect_int("black DAC first", dac.rgb6[0][0], 0);
    expect_int("black DAC last", dac.rgb6[31][2], 0);

    expect_int("normal curtain", redmcsb_f8159_palette_set_curtain_c25_pc34_compat(
                   (const uint8_t (*)[REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34])full, &dac, &curtain, REDMCSB_F8159_NORMAL_PALETTE_PC34,
                   wait_vblank, &wait_state), 1);
    expect_int("normal wait", wait_state.calls, 2);
    expect_int("normal state", curtain, REDMCSB_F8159_NORMAL_PALETTE_PC34);
    expect_int("normal real red", dac.rgb6[31][0], 31);
    expect_int("normal real blue", dac.rgb6[31][2], 33);

    expect_int("unknown source state", redmcsb_f8159_palette_set_curtain_c25_pc34_compat(
                   (const uint8_t (*)[REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34])full, &dac, &curtain, 7U, wait_vblank, &wait_state), 1);
    expect_int("unknown skips VBlank", wait_state.calls, 2);
    expect_int("unknown state stored", curtain, 7);

    wait_state.allow = 0;
    curtain = 9U;
    expect_int("failed VBlank", !redmcsb_f8159_palette_set_curtain_c25_pc34_compat(
                   (const uint8_t (*)[REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34])full, &dac, &curtain, REDMCSB_F8159_BLACK_PALETTE_PC34,
                   wait_vblank, &wait_state), 1);
    expect_int("failed state not stored", curtain, 9);

    if (strstr(redmcsb_f8159_palette_curtain_source_evidence_pc34(),
               "VIDEODRV.C:3487-3541") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8159 C25 RGB6 palette curtain");
    return 0;
}
