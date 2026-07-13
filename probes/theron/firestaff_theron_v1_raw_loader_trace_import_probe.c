#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>

int main(void)
{
    static const char capture[] =
        "source=mednafen-pce-instrumented\n"
        "boot_pc=e98a physical_pc=0000e98a instruction=LDA $22A4 cd_1800=90\n"
        "post_e98a_controller_transfer_source_pc=e98e source_physical_pc=0000e98e instruction=JSR $EA27 next_pc=ea27 next_physical_pc=0000ea27\n"
        "dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 record_cl=e0 record_dl=04 record_ch=00 variant=us_bin record=0004e0\n"
        "dynamic_cd_read_destination_span pc=4093 destination=3800 bytes=32 fnv1a=5a4f90c1\n"
        "dynamic_cd_read_controller_state pc=e74c f5_after_cd_read=00 f5_at_irq2_entry=00 status_1802=00 status_1803=00 f2_before_merge=00 f2_at_branch=00\n"
        "dynamic_huc6260_palette_store pc=4a00 physical_pc=00004a00 opcode=8d address=0402 accumulator=01\n"
        "dynamic_huc6260_palette_word index=000 word=001\n";
    const char *path = "/tmp/firestaff-tqr-mednafen-trace.txt";
    Theron_V1RawLoaderTraceReceipt receipt;
    FILE *file = fopen(path, "wb");

    if (!file || fputs(capture, file) < 0 || fclose(file) != 0) {
        if (file) fclose(file);
        remove(path);
        return 1;
    }
    if (!theron_v1_raw_loader_trace_import_mednafen_capture_file(
            path, THERON_TRACK02_MD5_US_BIN, &receipt) || !receipt.valid ||
        receipt.palette_descriptor_relation_verified ||
        !receipt.dynamic_cd_read_registers_verified ||
        !receipt.dynamic_cd_read_destination_span_verified ||
        receipt.palette_word_count != 1u ||
        theron_v1_raw_loader_trace_import_mednafen_capture_file(
            path, THERON_TRACK02_MD5_JP_BIN, &receipt)) {
        remove(path);
        return 1;
    }
    remove(path);
    return 0;
}
