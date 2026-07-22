#include "nexus_v1_prs3_execution_capture_admission.h"
#include "firestaff_x68k_media_receipt.h"
#include <string.h>

static uint64_t fnv(const uint8_t *p,size_t n){uint64_t h=UINT64_C(1469598103934665603);size_t i;for(i=0;i<n;++i){h^=p[i];h*=UINT64_C(1099511628211);}return h;}
static int sha256_matches(const uint8_t *bytes, size_t byte_count,
                          const char expected[65])
{
    char actual[65];

    return bytes && byte_count && expected &&
        firestaff_x68k_media_receipt_sha256_hex(
            bytes, byte_count, actual, sizeof(actual)) == 0 &&
        strcmp(actual, expected) == 0;
}

int nexus_v1_prs3_execution_capture_admit(
    const Nexus_V1_Prs3ExecutionCaptureAdmissionInput *in,
    Nexus_V1_Prs3ExecutionCaptureAdmissionReceipt *out)
{
    Nexus_V1_Prs3ExecutionCaptureAdmissionReceipt r;
    const Nexus_V1_Prs3OriginalExecutionEvidenceReceipt *e;
    const Nexus_V1_Prs3ExecutionCaptureAuthentication *x;
    const uint8_t *stream;

    memset(&r, 0, sizeof(r));
    r.evidence_only = 1;
    if (!out) return 0;
    if (!in || !(e = in->execution) || !(x = in->authentication) ||
        !e->valid || !e->evidence_only || e->decoder_promoted ||
        e->graphics_permitted || !e->stream_size || !e->payload_fnv1a64 ||
        !e->last_output_write_sequence ||
        e->vdp1_command_sequence <= e->last_output_write_sequence ||
        !in->menu_bpk_bytes || !in->menu_bpk_byte_count ||
        !in->dm_bin_bytes || !in->dm_bin_byte_count ||
        !in->output_bytes || !in->output_byte_count ||
        !in->vdp1_capture_bytes || !in->vdp1_capture_byte_count || !x->valid ||
        x->menu_bpk_fnv1a64 != e->menu_bpk_fnv1a64 ||
        x->dm_bin_fnv1a64 != e->dm_bin_fnv1a64 ||
        x->entry_index != e->entry_index || x->stream_offset != e->stream_offset ||
        x->stream_size != e->stream_size || x->stream_fnv1a64 != e->payload_fnv1a64 ||
        x->last_output_write_sequence != e->last_output_write_sequence ||
        x->vdp1_command_sequence != e->vdp1_command_sequence ||
        e->stream_offset > in->menu_bpk_byte_count ||
        e->stream_size > in->menu_bpk_byte_count - e->stream_offset ||
        in->output_byte_count != e->output_write_bytes) {
        *out = r;
        return 0;
    }
    stream = in->menu_bpk_bytes + e->stream_offset;
    if (fnv(in->menu_bpk_bytes, in->menu_bpk_byte_count) != e->menu_bpk_fnv1a64 ||
        fnv(in->dm_bin_bytes, in->dm_bin_byte_count) != e->dm_bin_fnv1a64 ||
        fnv(stream, e->stream_size) != e->payload_fnv1a64 ||
        fnv(in->output_bytes, in->output_byte_count) != e->output_fnv1a64 ||
        fnv(in->output_bytes, in->output_byte_count) != x->output_bytes_fnv1a64 ||
        fnv(in->vdp1_capture_bytes, in->vdp1_capture_byte_count) !=
            x->vdp1_capture_fnv1a64 ||
        !sha256_matches(in->menu_bpk_bytes, in->menu_bpk_byte_count,
                        x->menu_bpk_sha256) ||
        !sha256_matches(in->dm_bin_bytes, in->dm_bin_byte_count,
                        x->dm_bin_sha256) ||
        !sha256_matches(stream, e->stream_size, x->stream_sha256) ||
        !sha256_matches(in->output_bytes, in->output_byte_count,
                        x->output_bytes_sha256) ||
        !sha256_matches(in->vdp1_capture_bytes, in->vdp1_capture_byte_count,
                        x->vdp1_capture_sha256)) {
        *out = r;
        return 0;
    }
    r.valid = 1;
    r.entry_index = e->entry_index;
    r.stream_offset = e->stream_offset;
    r.stream_size = e->stream_size;
    r.stream_fnv1a64 = e->payload_fnv1a64;
    r.output_fnv1a64 = e->output_fnv1a64;
    r.vdp1_capture_fnv1a64 = x->vdp1_capture_fnv1a64;
    r.full_output_range_bound = r.vdp1_capture_bound = r.command_order_bound = 1;
    r.stream_identity_bound = r.stream_bytes_bound = r.original_assets_bound = 1;
    *out = r;
    return 1;
}
