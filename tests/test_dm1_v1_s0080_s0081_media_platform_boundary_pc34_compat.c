#include "dm1_v1_s0080_s0081_media_platform_boundary_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;
#define CHECK(expression) do { ++assertions; if (!(expression)) { ++failures; \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); } } while (0)

int main(void)
{
    DM1_V1_S0080S0081MediaPlatformRequestPc34 request;
    DM1_V1_S0080S0081MediaPlatformReceiptPc34 receipt;
    memset(&request, 0, sizeof(request));
    request.raw_media_fingerprint = 1u;
    request.original_pc34_media_verified = 1;
    request.no_dma_emulation = 1;
    request.no_floppy_emulation = 1;
    CHECK(!dm1_v1_s0080_check_dma_transfer_completion_boundary_pc34(&request, &receipt));
    CHECK(receipt.fail_closed && receipt.dma_completion_suppressed &&
          !receipt.floppy_power_suppressed && !receipt.execution_permitted);
    CHECK(!dm1_v1_s0081_turn_off_floppy_drive_boundary_pc34(&request, &receipt));
    CHECK(receipt.fail_closed && !receipt.dma_completion_suppressed &&
          receipt.floppy_power_suppressed && !receipt.source_body_applicable);
    request.no_dma_emulation = 0;
    CHECK(!dm1_v1_s0080_check_dma_transfer_completion_boundary_pc34(&request, &receipt));
    CHECK(!receipt.fail_closed && !receipt.source_evidence);
    CHECK(strstr(dm1_v1_s0080_s0081_media_platform_source_evidence_pc34(),
                 "No PC34 source body") != NULL);
    printf("test_dm1_v1_s0080_s0081_media_platform_boundary_pc34_compat: %d assertions, %d failures\n", assertions, failures);
    return failures == 0 ? 0 : 1;
}
