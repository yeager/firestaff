#include <stdio.h>

#include "theron_v1_track02_loader_output_record_admission.h"

int main(void)
{
    Theron_V1Track02LoaderOutputRecordAdmissionReceipt receipt;

    if (theron_v1_track02_loader_output_record_admit(
            NULL, NULL, NULL, NULL, NULL, &receipt) || receipt.valid ||
        receipt.envelope_header_fields_proven || receipt.level_boundary_proven ||
        receipt.header_level_identifier_semantics_allowed || receipt.bitmap_boundary_proven ||
        receipt.object_continuation_boundary_proven || receipt.render_allowed ||
        receipt.fallback_visuals_allowed) return 1;
    puts("test_theron_v1_track02_loader_output_record_admission: PASS (no local corpus)");
    return 0;
}
