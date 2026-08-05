#include "theron_v1_mednafen_main_ram_trace.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *path = getenv("THERON_MEDNAFEN_MAIN_RAM_TRACE");
    Theron_V1MednafenMainRamTraceReceipt receipt;

    if (!path || !path[0]) {
        puts("SKIP: THERON_MEDNAFEN_MAIN_RAM_TRACE is not set");
        return 77;
    }
    if (!theron_v1_mednafen_main_ram_trace_parse_file(path, &receipt) ||
        receipt.status != THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_READY ||
        !receipt.source_trace_md5_verified || !receipt.source_header_verified ||
        !receipt.transfer_coordinates_verified || receipt.target_2600_bytes_present ||
        receipt.semantic_publication_allowed || receipt.first_length != 0x80u) {
        fprintf(stderr, "FAIL: Main-RAM loader trace was not accepted\n");
        return 1;
    }
    printf("PASS: md5=%s transfers=%u rts=%u post_rts=%u "
           "pc=%x physical=%x source=%x destination=%x length=%x "
           "target_2600=absent semantic_publication=blocked\n",
           receipt.source_trace_md5, receipt.block_transfer_count,
           receipt.rts_count, receipt.post_rts_count, receipt.first_logical_pc,
           receipt.first_physical_pc, receipt.first_source,
           receipt.first_destination, receipt.first_length);
    return 0;
}
