#include "nexus_v1_prs3_execution_capture_admission.h"
#include "firestaff_x68k_media_receipt.h"

#include <stdio.h>
#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t count)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0U; index < count; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int main(void)
{
    uint8_t menu_bpk[] = { 9U, 8U, 7U, 6U, 5U, 4U };
    uint8_t dm_bin[] = { 0xa0U, 0xb1U, 0xc2U, 0xd3U };
    uint8_t output[] = { 1U, 2U, 3U, 4U };
    uint8_t vdp1[] = { 5U, 6U };
    Nexus_V1_Prs3OriginalExecutionEvidenceReceipt execution = {0};
    Nexus_V1_Prs3ExecutionCaptureAuthentication auth = {0};
    Nexus_V1_Prs3ExecutionCaptureAdmissionInput input = {0};
    Nexus_V1_Prs3ExecutionCaptureAdmissionReceipt receipt;

    execution.valid = execution.evidence_only = 1;
    execution.entry_index = auth.entry_index = 3U;
    execution.stream_offset = auth.stream_offset = 1U;
    execution.stream_size = auth.stream_size = 4U;
    execution.menu_bpk_fnv1a64 = auth.menu_bpk_fnv1a64 =
        fnv1a64(menu_bpk, sizeof(menu_bpk));
    execution.dm_bin_fnv1a64 = auth.dm_bin_fnv1a64 =
        fnv1a64(dm_bin, sizeof(dm_bin));
    execution.payload_fnv1a64 = auth.stream_fnv1a64 =
        fnv1a64(menu_bpk + execution.stream_offset, execution.stream_size);
    execution.output_write_bytes = 4U;
    execution.output_fnv1a64 = fnv1a64(output, sizeof(output));
    execution.last_output_write_sequence = auth.last_output_write_sequence = 7U;
    execution.vdp1_command_sequence = auth.vdp1_command_sequence = 8U;
    auth.valid = 1;
    auth.output_bytes_fnv1a64 = execution.output_fnv1a64;
    auth.vdp1_capture_fnv1a64 = fnv1a64(vdp1, sizeof(vdp1));
    if (firestaff_x68k_media_receipt_sha256_hex(menu_bpk, sizeof(menu_bpk),
            auth.menu_bpk_sha256, sizeof(auth.menu_bpk_sha256)) ||
        firestaff_x68k_media_receipt_sha256_hex(dm_bin, sizeof(dm_bin),
            auth.dm_bin_sha256, sizeof(auth.dm_bin_sha256)) ||
        firestaff_x68k_media_receipt_sha256_hex(
            menu_bpk + execution.stream_offset, execution.stream_size,
            auth.stream_sha256, sizeof(auth.stream_sha256)) ||
        firestaff_x68k_media_receipt_sha256_hex(output, sizeof(output),
            auth.output_bytes_sha256, sizeof(auth.output_bytes_sha256)) ||
        firestaff_x68k_media_receipt_sha256_hex(vdp1, sizeof(vdp1),
            auth.vdp1_capture_sha256, sizeof(auth.vdp1_capture_sha256))) return 1;
    input.execution = &execution;
    input.menu_bpk_bytes = menu_bpk;
    input.menu_bpk_byte_count = sizeof(menu_bpk);
    input.dm_bin_bytes = dm_bin;
    input.dm_bin_byte_count = sizeof(dm_bin);
    input.output_bytes = output;
    input.output_byte_count = sizeof(output);
    input.vdp1_capture_bytes = vdp1;
    input.vdp1_capture_byte_count = sizeof(vdp1);
    input.authentication = &auth;
    if (!nexus_v1_prs3_execution_capture_admit(&input, &receipt) ||
        !receipt.valid || !receipt.stream_identity_bound ||
        !receipt.command_order_bound || !receipt.evidence_only ||
        !receipt.stream_bytes_bound || !receipt.original_assets_bound ||
        receipt.decoder_promoted || receipt.graphics_permitted) return 1;
    auth.entry_index++;
    if (nexus_v1_prs3_execution_capture_admit(&input, &receipt) || receipt.valid) return 1;
    auth.entry_index--;
    auth.vdp1_command_sequence--;
    if (nexus_v1_prs3_execution_capture_admit(&input, &receipt) || receipt.valid) return 1;
    auth.vdp1_command_sequence++;
    menu_bpk[2] ^= 0x01U;
    if (nexus_v1_prs3_execution_capture_admit(&input, &receipt) || receipt.valid) return 1;
    puts("prs3 execution capture admission: PASS");
    return 0;
}
