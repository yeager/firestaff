#include "csb_v1_f0846_f0865_unmapped_boundary_pc34_compat.h"

#include <stdio.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

int main(void)
{
    CSB_V1_F0846F0865UnmappedBoundaryReceiptPc34 receipt;
    uint16_t function_id;

    for (function_id = 846u; function_id <= 865u; ++function_id) {
        check(csb_v1_f0846_f0865_unmapped_admit_pc34(function_id, &receipt) == 0,
              "unmapped range cannot admit a runtime route");
        check(receipt.function_id == function_id && receipt.source_callable_absent &&
                  receipt.authentic_package_required && receipt.runtime_execution_blocked &&
                  receipt.no_synthetic_ui_graphics_timing &&
                  receipt.source_evidence != NULL,
              "rejection records an explicit fail-closed source boundary");
    }
    check(csb_v1_f0846_f0865_unmapped_admit_pc34(845u, &receipt) == 0 &&
              receipt.function_id == 0u,
          "outside range does not fabricate an audit receipt");

    printf("csb_v1_f0846_f0865_unmapped_boundary: %s\n",
           failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
