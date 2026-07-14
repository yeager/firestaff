#include "dm1_v2_startup_title_filter_handoff_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"

#include <string.h>

int dm1_v2_startup_title_filter_handoff_pc34(
    const char *game_id,
    int presentation_mode,
    int source_timing_consumed,
    int special_palette_valid,
    DM1_V2_StartupTitleFilterHandoffReceiptPc34 *out_receipt)
{
    DM1_V2_StartupTitleFilterHandoffReceiptPc34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!game_id || strcmp(game_id, "dm1") != 0 ||
        presentation_mode != DM1_V2_PM_V20_FILTERED ||
        !source_timing_consumed || !special_palette_valid) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.handled = 1;
    receipt.filters_active = 1;
    receipt.source_palette_preserved = 1;
    receipt.source_timing_consumed = 1;
    *out_receipt = receipt;
    return 1;
}
