/* Read-only validator for externally captured Nexus PRS3 V3 traces.
 *
 * Usage:
 *   firestaff_nexus_v1_prs3_v3_capture_validator TRACE MENU.BPK DM.BIN
 *   firestaff_nexus_v1_prs3_v3_capture_validator TRACE MENU.BPK DM.BIN \
 *       OUTPUT.BIN VDP1-COMMAND.BIN PALETTE.BIN
 *   firestaff_nexus_v1_prs3_v3_capture_validator TRACE MENU.BPK DM.BIN \
 *       OUTPUT.BIN VDP1-COMMAND.BIN PALETTE.BIN PROVENANCE.TXT PRODUCER
 *   firestaff_nexus_v1_prs3_v3_capture_validator TRACE MENU.BPK DM.BIN \
 *       OUTPUT.BIN VDP1-COMMAND.BIN PALETTE.BIN PROVENANCE.TXT PRODUCER ATTESTATION.TXT
 *
 * The program deliberately does not emit decoded bytes or render a surface.
 * It only reports whether an externally recorded trace is internally complete
 * and bound to the canonical Track 1 MENU.BPK/DM.BIN files. */

#include "nexus_v1_prs3_capture_trace_schema.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    Nexus_V1_Prs3Vdp1CaptureFileReceipt receipt;
    Nexus_V1_Prs3Vdp1RawSidecarReceipt sidecars;
    Nexus_V1_Prs3Vdp1ProvenanceReceipt provenance;
    Nexus_V1_Prs3Vdp1ProducerAttestationReceipt attestation;
    int accepted;

    if (argc != 4 && argc != 7 && argc != 9 && argc != 10) {
        fprintf(stderr, "usage: %s TRACE MENU.BPK DM.BIN [OUTPUT VDP1-COMMAND PALETTE [PROVENANCE PRODUCER [ATTESTATION]]]\n",
                argv[0]);
        return 2;
    }
    if (argc >= 7) {
        accepted = nexus_v1_prs3_vdp1_capture_validate_raw_sidecars(
            argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], &sidecars);
        receipt = sidecars.trace_file;
        printf("output_sidecar_bound=%d\n", sidecars.output_sidecar_bound);
        printf("vdp1_command_sidecar_bound=%d\n",
               sidecars.vdp1_command_sidecar_bound);
        printf("palette_sidecar_bound=%d\n", sidecars.palette_sidecar_bound);
        printf("raw_sidecars_bound=%d\n", sidecars.raw_sidecars_bound);
        printf("capture_producer_authenticated=%d\n",
               sidecars.capture_producer_authenticated);
        if (argc >= 9) {
            int provenance_accepted = nexus_v1_prs3_vdp1_capture_validate_provenance(
                argv[7], argv[1], argv[4], argv[5], argv[6], argv[8],
                &sidecars, &provenance);
            printf("provenance_ledger_parsed=%d\n", provenance.ledger_parsed);
            printf("provenance_complete=%d\n", provenance.provenance_complete);
            printf("producer_binary_bound=%d\n", provenance.producer_binary_bound);
            printf("provenance_capture_producer_authenticated=%d\n",
                   provenance.capture_producer_authenticated);
            printf("provenance_runtime_import_permitted=%d\n",
                   provenance.runtime_import_permitted);
            accepted = accepted && provenance_accepted;
        }
        if (argc == 10) {
            int attestation_accepted = nexus_v1_prs3_vdp1_capture_validate_producer_attestation(
                argv[9], argv[1], argv[4], argv[5], argv[6], argv[8],
                &sidecars, &provenance, &attestation);
            printf("attestation_file_read=%d\n", attestation.attestation_file_read);
            printf("attestation_parsed=%d\n", attestation.attestation_parsed);
            printf("producer_workflow_complete=%d\n", attestation.workflow_complete);
            printf("independent_authentication_required=%d\n",
                   attestation.independent_authentication_required);
            printf("attestation_capture_producer_authenticated=%d\n",
                   attestation.capture_producer_authenticated);
            accepted = accepted && attestation_accepted;
        }
    } else {
        accepted = nexus_v1_prs3_vdp1_capture_validate_files(
            argv[1], argv[2], argv[3], &receipt);
    }
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
