#include "nexus_v1_prs3_dgn_placement_adapter.h"

#include <string.h>

int nexus_v1_prs3_dgn_placement_adapter_admit(
    const Nexus_V1_Prs3DgnPlacementAdapterInput *input,
    Nexus_V1_Prs3DgnPlacementAdapterReceipt *out)
{
    const Nexus_V1_DgnActiveLevelRendererSourceReceipt *d;
    const Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *p;
    if (!out) return 0;
    memset(out, 0, sizeof(*out)); out->no_draw_only = 1; out->blocks_real_dgn_mesh_render = 1;
    if (!input || !(d = input->dgn) || !(p = input->prs3) ||
        !input->capture || !input->trace_fnv1a64 || !input->trace_size) return 0;
    out->trace_fnv1a64 = input->trace_fnv1a64; out->trace_size = input->trace_size; out->dgn_fnv1a64 = d->source_bytes_fnv1a64;
    out->prs3_entry_index = p->prs3_entry_index;
    out->trace_bound = p->trace_artifact_available && p->raw_trace_authenticated &&
        p->trace_binds_dm_bin && p->trace_binds_menu_bpk_entry5 &&
        p->trace_binds_lev00_structure2 && p->candidate_spans_bound &&
        p->prs3_header_span_fnv1a64 && p->prs3_bitmap_candidate_fnv1a64 &&
        p->prs3_bitmap_candidate_size && p->palt_candidate_fnv1a64 &&
        p->palt_candidate_size == 512U &&
        p->prs3_stream_offset <= UINT32_MAX - 4U &&
        p->prs3_stream_size >= 4U &&
        p->prs3_bitmap_candidate_offset == p->prs3_stream_offset + 4U &&
        p->prs3_bitmap_candidate_size == p->prs3_stream_size - 4U;
    if (out->trace_bound) {
        out->prs3_header_span_fnv1a64 = p->prs3_header_span_fnv1a64;
        out->prs3_bitmap_candidate_fnv1a64 =
            p->prs3_bitmap_candidate_fnv1a64;
        out->prs3_bitmap_candidate_offset = p->prs3_bitmap_candidate_offset;
        out->prs3_bitmap_candidate_size = p->prs3_bitmap_candidate_size;
        out->palt_candidate_fnv1a64 = p->palt_candidate_fnv1a64;
        out->palt_candidate_size = p->palt_candidate_size;
    }
    out->dgn_source_bound = d->valid && d->package_source_bound &&
        d->source_bytes_fnv1a64 && input->traced_dgn_fnv1a64 == d->source_bytes_fnv1a64 &&
        input->capture->valid && input->capture->complete_capture &&
        input->capture->schema_version >= 10U &&
        input->capture->dgn_placement_observed &&
        input->capture->dgn_fnv1a64 == d->source_bytes_fnv1a64 &&
        input->capture->dgn_descriptor_index == input->descriptor_index &&
        input->descriptor_fnv1a64 &&
        input->capture->dgn_descriptor_fnv1a64 == input->descriptor_fnv1a64 &&
        input->capture->dgn_frame_sequence &&
        input->capture->dgn_command_sequence == input->capture->vdp1_command_sequence &&
        d->vdp1_command_format_framed && d->vdp1_coordinate_words_framed;
    out->descriptor_envelope_bound = input->descriptor_envelope_valid;
    out->command_coordinates_bound = out->dgn_source_bound &&
        d->vdp1_command_framing.command.xa == input->traced_xa && d->vdp1_command_framing.command.ya == input->traced_ya &&
        d->vdp1_command_framing.command.xb == input->traced_xb && d->vdp1_command_framing.command.yb == input->traced_yb &&
        d->vdp1_command_framing.command.xc == input->traced_xc && d->vdp1_command_framing.command.yc == input->traced_yc &&
        d->vdp1_command_framing.command.xd == input->traced_xd && d->vdp1_command_framing.command.yd == input->traced_yd &&
        (uint16_t)input->traced_xa == (uint16_t)input->capture->dgn_command_xa && (uint16_t)input->traced_ya == (uint16_t)input->capture->dgn_command_ya &&
        (uint16_t)input->traced_xb == (uint16_t)input->capture->dgn_command_xb && (uint16_t)input->traced_yb == (uint16_t)input->capture->dgn_command_yb &&
        (uint16_t)input->traced_xc == (uint16_t)input->capture->dgn_command_xc && (uint16_t)input->traced_yc == (uint16_t)input->capture->dgn_command_yc &&
        (uint16_t)input->traced_xd == (uint16_t)input->capture->dgn_command_xd && (uint16_t)input->traced_yd == (uint16_t)input->capture->dgn_command_yd;
    if (out->dgn_source_bound) { out->descriptor_index = input->descriptor_index; out->descriptor_fnv1a64 = input->descriptor_fnv1a64; out->frame_sequence = input->capture->dgn_frame_sequence; out->command_sequence = input->capture->dgn_command_sequence; out->dgn_placement_observed = 1; }
    out->valid = out->trace_bound && out->dgn_source_bound && out->descriptor_envelope_bound && out->command_coordinates_bound;
    return out->valid;
}
