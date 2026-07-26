#include "redmcsb_f8156_f8157_c25_palette_pc34_compat.h"

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
    uint8_t before[32][3];
    RedmcsbF8156C25DacPc34Compat dac;
    WaitState wait_state = {0, 1};
    const RedmcsbF8157C25PaletteEntryPc34Compat entries[] = {
        {1, 0x3F, 0x12, 0x03}, {31, 0x10, 0x20, 0x30},
        {32, 0xAA, 0xBB, 0xCC}, {-1, 0, 0, 0}
    };
    const RedmcsbF8157C25PaletteEntryPc34Compat unterminated[] = {
        {2, 1, 2, 3}
    };

    memset(full, 0x11, sizeof(full));
    memset(&dac, 0xEE, sizeof(dac));
    expect_int("curtain off table update", redmcsb_f8157_set_multiple_colors_c25_pc34_compat(
                   full, entries, sizeof(entries) / sizeof(entries[0]), 0,
                   &dac, wait_vblank, &wait_state), 1);
    expect_int("index 1 red", full[1][0], 0x3F);
    expect_int("index 1 green", full[1][1], 0x12);
    expect_int("index 31 blue", full[31][2], 0x30);
    expect_int("out of range index unchanged", full[0][0], 0x11);
    expect_int("no VBlank under curtain off", wait_state.calls, 0);
    expect_int("DAC unchanged under curtain off", dac.rgb6[1][0], 0xEE);

    expect_int("curtain on publishes", redmcsb_f8157_set_multiple_colors_c25_pc34_compat(
                   full, entries, sizeof(entries) / sizeof(entries[0]), 1,
                   &dac, wait_vblank, &wait_state), 1);
    expect_int("one VBlank", wait_state.calls, 1);
    expect_int("DAC gets real RGB6 red", dac.rgb6[1][0], 0x3F);
    expect_int("DAC gets real RGB6 blue", dac.rgb6[31][2], 0x30);

    memcpy(before, full, sizeof(full));
    expect_int("unterminated table rejected", !redmcsb_f8157_set_multiple_colors_c25_pc34_compat(
                   full, unterminated, 1U, 0, &dac, wait_vblank, &wait_state), 1);
    expect_int("unterminated no table mutation", memcmp(before, full, sizeof(full)), 0);

    wait_state.allow = 0;
    expect_int("failed VBlank rejects DAC publish", !redmcsb_f8156_set_palette_c25_pc34_compat(
                   (const uint8_t (*)[REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34])full, &dac, wait_vblank, &wait_state), 1);
    expect_int("second VBlank call", wait_state.calls, 2);

    if (strstr(redmcsb_f8156_f8157_c25_palette_source_evidence_pc34(),
               "VIDEODRV.C:3302-3389") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8156/F8157 C25 RGB6 palette route");
    return 0;
}
