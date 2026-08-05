/* Regression: c_move.cpp:2914 must not be repurposed as a move commit. */
#include "dm2_v1_move_2c1d_028c.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_Move2c1d028cReceipt receipt;
    int result = dm2_v1_DM2_move_2c1d_028c_commit_receipt(NULL, &receipt);

    if (result != 0 || receipt.valid || !receipt.blocked ||
        !receipt.source_state_unbound ||
        receipt.outcome != DM2_V1_MOVE_2C1D_028C_OUTCOME_BLOCKED ||
        receipt.block_reason != DM2_V1_MOVE_075F_1BC2_BLOCK_SOURCE_STATE_UNBOUND ||
        dm2_v1_DM2_move_2c1d_028c_source_evidence()[0] == '\0') {
        return 1;
    }
    puts("DM2_move_2c1d_028c rejects unbound party state");
    return 0;
}
