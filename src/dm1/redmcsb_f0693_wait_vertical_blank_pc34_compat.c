#include "redmcsb_f0693_wait_vertical_blank_pc34_compat.h"

void F0693_VerticalBlankCallback_PC34(
    ReDMCSBF0693WaitVerticalBlankPc34Compat *gate)
{
    if (gate != NULL) {
        gate->waiting_for_vertical_blank = false;
    }
}

bool F0693_WaitVerticalBlank_PC34(
    ReDMCSBF0693WaitVerticalBlankPc34Compat *gate)
{
    if (gate == NULL || gate->deliver_vertical_blank == NULL) {
        return false;
    }

    gate->waiting_for_vertical_blank = true;
    while (gate->waiting_for_vertical_blank) {
        gate->deliver_vertical_blank(gate->context);
    }
    return true;
}
