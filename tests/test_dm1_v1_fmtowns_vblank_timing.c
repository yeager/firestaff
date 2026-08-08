#include "dm1_v1_vblank_timing.h"

#include <assert.h>
#include <stdio.h>

int main(void) {
    DM1_V1_VBlankTimingState state;

    DM1_V1_VBlankTiming_Init(&state);
    assert(state.vblankPeriodMs == DM1_V1_PAL_VBLANK_MS);
    assert(state.maxVBlankCount == DM1_V1_MAX_VBLANK_COUNT_ORIGINAL);

    DM1_V1_VBlankTiming_ConfigureFmTowns(&state);
    assert(state.vblankPeriodMs == DM1_V1_FMTOWNS_VBLANK_MS);
    assert(state.maxVBlankCount == DM1_V1_MAX_VBLANK_COUNT_LATER);

    /* FM Towns: 12 VBlanks at 16ms = 192ms per tick. */
    assert(DM1_V1_VBlankTiming_Update(&state, 192) == 1);
    assert(state.vblankCount == 12);
    assert(state.stopWaitingForInput == 1);

    DM1_V1_VBlankTiming_ResetForNewTick(&state);
    assert(state.vblankCount == 0);
    assert(state.stopWaitingForInput == 0);

    /* 160ms = 10 VBlanks at 16ms, not enough for a tick (need 12). */
    assert(DM1_V1_VBlankTiming_Update(&state, 160) == 0);
    assert(state.vblankCount == 10);

    /* 32ms more = 2 more VBlanks, total 12 — tick fires. */
    assert(DM1_V1_VBlankTiming_Update(&state, 32) == 1);
    assert(state.vblankCount == 12);

    /* PAL comparison: 10 VBlanks at 20ms = 200ms per tick. */
    DM1_V1_VBlankTiming_Init(&state);
    assert(DM1_V1_VBlankTiming_Update(&state, 200) == 1);
    assert(state.vblankCount == 10);

    /* RecordTurn uses dynamic tick interval. */
    DM1_V1_VBlankTiming_Init(&state);
    DM1_V1_VBlankTiming_ConfigureFmTowns(&state);
    DM1_V1_VBlankTiming_RecordTurn(&state);
    assert(state.turnCooldownMs == 16u * 12u);

    DM1_V1_VBlankTiming_Init(&state);
    DM1_V1_VBlankTiming_RecordTurn(&state);
    assert(state.turnCooldownMs == 20u * 10u);

    puts("PASS dm1_v1_fmtowns_vblank_timing");
    return 0;
}
