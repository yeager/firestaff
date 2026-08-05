/* Regression: c_move.cpp:2861 must not be repurposed as collision logic. */
#include "dm2_v1_move_075f_1bc2.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_Move075f1bc2Receipt receipt;
    int result = dm2_v1_DM2_move_075f_1bc2_target_receipt(
        NULL, 0, 1, 1, 0, 0, &receipt);

    if (result != 0 || receipt.valid || !receipt.blocked ||
        !receipt.source_state_unbound ||
        receipt.block_reason != DM2_V1_MOVE_075F_1BC2_BLOCK_SOURCE_STATE_UNBOUND ||
        dm2_v1_DM2_move_075f_1bc2_source_evidence()[0] == '\0') {
        return 1;
    }
    puts("DM2_move_075f_1bc2 rejects unbound party/RNG state");
    return 0;
}
