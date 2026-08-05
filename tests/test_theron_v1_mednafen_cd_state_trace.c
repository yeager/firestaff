#include "theron_v1_mednafen_cd_state_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *path = getenv("THERON_MEDNAFEN_CD_TRACE");
    Theron_V1MednafenCdStateTraceReceipt receipt;

    if (!path || !path[0]) {
        puts("SKIP: THERON_MEDNAFEN_CD_TRACE is not set");
        return 77;
    }
    if (!theron_v1_mednafen_cd_state_trace_parse_file(path, &receipt) ||
        receipt.status != THERON_V1_MEDNAFEN_CD_STATE_TRACE_READY ||
        !receipt.source_trace_md5_verified || !receipt.source_header_verified ||
        !receipt.raw_mode1_2352_verified ||
        !receipt.command_sector_binding_verified ||
        receipt.semantic_publication_allowed ||
        receipt.scsi_command_count == 0u || receipt.raw_sector_count == 0u ||
        receipt.raw_sector_count != receipt.requested_sector_count ||
        receipt.raw_sector_count != receipt.sector_binding_count) {
        fprintf(stderr, "FAIL: real Mednafen CD state trace was not accepted\n");
        return 1;
    }
    printf("PASS: md5=%s commands=%u requested=%u raw_sectors=%u bindings=%u "
           "lba=%u..%u irq=%u destinations=%u semantic_publication=blocked\n",
           receipt.source_trace_md5, receipt.scsi_command_count,
           receipt.requested_sector_count, receipt.raw_sector_count,
           receipt.sector_binding_count, receipt.first_lba, receipt.last_lba,
           receipt.cd_irq_count, receipt.destination_candidate_count);
    return 0;
}
