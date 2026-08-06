#include "theron_v1_mednafen_main_ram_consumer_trace.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *path = getenv("THERON_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE");
    Theron_V1MednafenMainRamConsumerTraceReceipt receipt;

    if (!path || !path[0]) {
        puts("SKIP: THERON_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE is not set");
        return 77;
    }
    if (!theron_v1_mednafen_main_ram_consumer_trace_parse_file(path, &receipt) ||
        receipt.status != THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_READY ||
        !receipt.source_trace_md5_verified || !receipt.source_header_verified ||
        !receipt.bank_coordinates_verified || receipt.target_2600_bytes_present ||
        receipt.semantic_publication_allowed || receipt.read_count == 0u) {
        fprintf(stderr, "FAIL: real Mednafen consumer trace was not accepted\n");
        return 1;
    }
    printf("PASS: md5=%s reads=%u first_physical=%x last_physical=%x "
           "first_reader=%x last_reader=%x target_2600=absent "
           "semantic_publication=blocked\n",
           receipt.source_trace_md5, receipt.read_count,
           receipt.first_physical_address, receipt.last_physical_address,
           receipt.first_reader_physical_pc, receipt.last_reader_physical_pc);
    return 0;
}
