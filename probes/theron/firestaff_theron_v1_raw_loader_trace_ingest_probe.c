#include "theron_v1_raw_loader_trace.h"

#include <string.h>

int main(void)
{
    static const char valid[] =
        "source=mednafen-pce-instrumented\n"
        "boot_pc=e98a physical_pc=0000e98a instruction=LDA $22A4 cd_1800=90\n"
        "post_e98a_controller_transfer_source_pc=e98e source_physical_pc=0000e98e instruction=JSR $EA27 next_pc=ea27 next_physical_pc=0000ea27\n"
        "dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 variant=us_bin record=0004e0\n"
        "dynamic_cd_read_controller_state pc=e74c f5_after_cd_read=00 f5_at_irq2_entry=00 status_1802=00 status_1803=00 f2_before_merge=00 f2_at_branch=00\n"
        "dynamic_huc6260_palette_store pc=4a00 physical_pc=00004a00 opcode=8d address=0402 accumulator=01\n"
        "dynamic_huc6260_palette_store pc=4a03 physical_pc=00004a03 opcode=8d address=0404 accumulator=7f\n";
    Theron_V1RawLoaderTraceReceipt receipt;

    return theron_v1_raw_loader_trace_ingest_mednafen_capture(
               valid, THERON_TRACK02_MD5_US_BIN, &receipt) &&
           receipt.valid && receipt.dynamic_cd_read_verified &&
           receipt.palette_store_observed_after_dynamic_read &&
           !receipt.palette_descriptor_relation_verified &&
           receipt.dynamic_cd_read_record == 0x0004e0u &&
           receipt.palette_store_count == 2u &&
           receipt.palette_register_mask == 0x05u &&
           receipt.first_palette_store_pc == 0x4a00u &&
           !theron_v1_raw_loader_trace_ingest_mednafen_capture(
               valid, THERON_TRACK02_MD5_JP_BIN, &receipt) &&
           !theron_v1_raw_loader_trace_ingest_mednafen_capture(
               "source=mednafen-pce-instrumented\n", THERON_TRACK02_MD5_US_BIN,
               &receipt) ? 0 : 1;
}
