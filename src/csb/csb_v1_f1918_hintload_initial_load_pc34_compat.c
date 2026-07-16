#include "csb_v1_f1918_hintload_initial_load_pc34_compat.h"

int csb_v1_f1918_load_game_cpsx_pc34(
    CSB_V1_F1918_ReadExactPc34 read, void *context,
    CSB_V1_F1918_LoadReceiptPc34 *receipt)
{
    return redmcsb_f1918_load_initial_save_parts_pc34(read, context, receipt);
}

int csb_v1_f1919_post_f1918_load_game_cpsx_pc34(
    const CSB_V1_F1918_LoadReceiptPc34 *receipt)
{
    unsigned int part;

    if (receipt == 0 || receipt->header_valid == 0 ||
        receipt->parts_loaded != REDMCSB_F1918_PC34_PART_COUNT) {
        return 0;
    }
    for (part = 0U; part < REDMCSB_F1918_PC34_PART_COUNT; ++part) {
        if (receipt->parts[part].bytes == 0 ||
            receipt->parts[part].byte_count == 0U ||
            (receipt->parts[part].byte_count & 1U) != 0U) {
            return 0;
        }
    }
    return 1;
}

const char *csb_v1_f1918_hintload_initial_load_source_evidence_pc34(void)
{
    return "ReDMCSB HINTHTC.C F1918_LoadGame_CPSX and "
           "F1919_Post_F1918_LoadGame_CPSX; HINTLOAD.C F1910/F1913/F1914; "
           "CEDTINC6.C F7055/F7061";
}
