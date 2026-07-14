/* Read-only validator for externally captured Nexus PRS3 V3 traces.
 *
 * Usage:
 *   firestaff_nexus_v1_prs3_v3_capture_validator TRACE MENU.BPK DM.BIN
 *
 * The program deliberately does not emit decoded bytes or render a surface.
 * It only reports whether an externally recorded trace is internally complete
 * and bound to the canonical Track 1 MENU.BPK/DM.BIN files. */

#include "nexus_v1_prs3_capture_trace_schema.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    Nexus_V1_Prs3Vdp1CaptureFileReceipt receipt;
    int accepted;

    if (argc != 4) {
        fprintf(stderr, "usage: %s TRACE MENU.BPK DM.BIN\n", argv[0]);
        return 2;
    }
    accepted = nexus_v1_prs3_vdp1_capture_validate_files(
        argv[1], argv[2], argv[3], &receipt);
    printf("trace_file_read=%d\n", receipt.trace_file_read);
    printf("menu_bpk_original_hash_verified=%d\n",
           receipt.menu_bpk_original_hash_verified);
    printf("dm_bin_original_hash_verified=%d\n",
           receipt.dm_bin_original_hash_verified);
    printf("v3_trace_parsed=%d\n", receipt.v3_trace_parsed);
    printf("source_bound_capture=%d\n", receipt.source_bound_capture);
    printf("vdp1_command_consumption_observed=%d\n",
           receipt.binding.vdp1_command_consumption_observed);
    printf("palette_consumption_observed=%d\n",
           receipt.binding.palette_consumption_observed);
    printf("runtime_import_permitted=%d\n", receipt.runtime_import_permitted);
    printf("decoder_promoted=%d\n", receipt.decoder_promoted);
    printf("fallback_visuals_permitted=%d\n",
           receipt.fallback_visuals_permitted);
    return accepted ? 0 : 1;
}
