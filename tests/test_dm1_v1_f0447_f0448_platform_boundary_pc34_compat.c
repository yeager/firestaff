#include "dm1_v1_f0447_f0448_platform_boundary_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

int main(void)
{
    DM1_V1_F0447F0448PlatformRequestPc34 request;
    DM1_V1_F0447F0448PlatformReceiptPc34 receipt;

    memset(&request, 0, sizeof(request));
    request.requested_platform = DM1_V1_F0447_F0448_SOURCE_PLATFORM_PC34;
    request.original_source_branch_verified = 1;
    request.no_platform_emulation = 1;
    request.no_synthetic_memory_manager = 1;

    CHECK(!dm1_v1_f0447_hang_if_false_boundary_pc34(&request, &receipt));
    CHECK(!receipt.source_body_applicable && !receipt.execution_permitted &&
          receipt.fail_closed && receipt.hang_suppressed &&
          !receipt.memory_manager_suppressed && receipt.suppress_platform_emulation);

    CHECK(!dm1_v1_f0448_initialize_memory_manager_boundary_pc34(&request, &receipt));
    CHECK(!receipt.source_body_applicable && !receipt.execution_permitted &&
          receipt.fail_closed && !receipt.hang_suppressed &&
          receipt.memory_manager_suppressed && receipt.suppress_platform_emulation);

    request.no_platform_emulation = 0;
    CHECK(!dm1_v1_f0447_hang_if_false_boundary_pc34(&request, &receipt));
    CHECK(!receipt.fail_closed && !receipt.hang_suppressed && !receipt.source_evidence);

    request.no_platform_emulation = 1;
    request.requested_platform = DM1_V1_F0447_F0448_SOURCE_PLATFORM_ATARI_ST;
    CHECK(!dm1_v1_f0448_initialize_memory_manager_boundary_pc34(&request, &receipt));
    CHECK(!receipt.fail_closed && !receipt.memory_manager_suppressed);

    CHECK(strstr(dm1_v1_f0447_f0448_platform_boundary_source_evidence_pc34(),
                 "Neither source body is PC34") != NULL);
    printf("test_dm1_v1_f0447_f0448_platform_boundary_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
