#include "nexus_v1_saturn_capture_campaign_import.h"

#include <stdio.h>
#include <string.h>

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t i;
    for (i = 0; i < size; ++i) { value ^= data[i]; value *= UINT64_C(1099511628211); }
    return value;
}

static int sha256_shape(const char *text)
{
    int i;
    if (!text || strlen(text) != 64U) return 0;
    for (i = 0; i < 64; ++i) {
        if (!((text[i] >= '0' && text[i] <= '9') ||
              (text[i] >= 'a' && text[i] <= 'f') ||
              (text[i] >= 'A' && text[i] <= 'F'))) return 0;
    }
    return 1;
}

int nexus_v1_saturn_capture_campaign_import(
    const Nexus_V1_SaturnCaptureCampaignImportInput *input,
    Nexus_V1_SaturnCaptureCampaignReceipt *out_receipt)
{
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *prs3;
    const Nexus_V1_Structure1FCorpusTraceTarget *structure1f;
    const Nexus_V1_SlevTaskBodyCaptureTarget *slev;
    const Nexus_V1_SalCapturePlanTarget *sal;
    Nexus_V1_SaturnCaptureCampaignReceipt receipt;
    char expected[2048];
    uint64_t trace_fnv;
    int length;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt)); receipt.evidence_only = 1;
    if (!input || !(prs3 = input->prs3) || !(structure1f = input->structure1f) ||
        !(slev = input->slev) || !(sal = input->sal) || !input->raw_trace ||
        !input->raw_trace_size || !sha256_shape(input->trace_sha256) ||
        !input->export_text || !input->export_text_size) { *out_receipt = receipt; return 0; }
    trace_fnv = fnv1a64(input->raw_trace, input->raw_trace_size);
    if (!prs3->valid || !prs3->trace_bound || !prs3->dgn_source_bound ||
        !prs3->descriptor_envelope_bound || !prs3->command_coordinates_bound ||
        !prs3->dgn_placement_observed || !prs3->trace_fnv1a64 || !prs3->dgn_fnv1a64 ||
        !prs3->descriptor_fnv1a64 || !prs3->frame_sequence || !prs3->command_sequence ||
        !prs3->no_draw_only || prs3->fallback_visuals_permitted ||
        !structure1f->valid || !structure1f->original_saturn_trace_required ||
        !structure1f->no_draw_only || structure1f->fallback_visuals_permitted ||
        !structure1f->blocks_real_dgn_mesh_render ||
        !slev->valid || !slev->source_order_required || !slev->original_saturn_trace_required ||
        !slev->no_dispatch_only || slev->fallback_script_permitted ||
        !sal->valid || !sal->source_order_required || !sal->original_saturn_trace_required ||
        !sal->no_playback_only || sal->fallback_script_permitted ||
        prs3->trace_fnv1a64 != trace_fnv || slev->raw_trace_fnv1a64 != trace_fnv ||
        sal->raw_trace_fnv1a64 != trace_fnv || slev->raw_trace_byte_count != input->raw_trace_size ||
        sal->raw_trace_byte_count != input->raw_trace_size ||
        structure1f->dgn_fnv1a64 != prs3->dgn_fnv1a64 ||
        structure1f->descriptor_index != prs3->descriptor_index ||
        structure1f->descriptor_fnv1a64 != prs3->descriptor_fnv1a64 ||
        slev->level_index != sal->level_index) { *out_receipt = receipt; return 0; }
    length = snprintf(expected, sizeof(expected),
        "%s\nproducer=mednafen-saturn-debugger\ntrace_sha256=%s\nraw_trace_fnv1a64=%016llx\nraw_trace_size=%llu\n"
        "prs3=dgn:%016llx,descriptor:%u,frame:%llu,command:%llu\n"
        "structure1f=level:%u,dgn:%016llx,descriptor:%u,mesh:%u,face:%u\n"
        "slev=level:%u,source:%016llx,entry:%08x,task:%08x,callback:%08x\n"
        "sal=level:%u,selector:%02x,sal:%016llx,map:%016llx,offset:%x,size:%x\n",
        NEXUS_V1_SATURN_CAPTURE_CAMPAIGN_MAGIC, input->trace_sha256,
        (unsigned long long)trace_fnv, (unsigned long long)input->raw_trace_size,
        (unsigned long long)prs3->dgn_fnv1a64, prs3->descriptor_index,
        (unsigned long long)prs3->frame_sequence, (unsigned long long)prs3->command_sequence,
        structure1f->level_index, (unsigned long long)structure1f->dgn_fnv1a64,
        structure1f->descriptor_index, structure1f->mesh_index, structure1f->face_index,
        slev->level_index, (unsigned long long)slev->source_fnv1a64, slev->entry_pc,
        slev->task_body_pc, slev->callback_or_write_pc, sal->level_index,
        sal->raw_map_selector, (unsigned long long)sal->canonical_sal_fnv1a64,
        (unsigned long long)sal->canonical_map_fnv1a64, sal->sal_offset, sal->sal_size);
    if (length < 0 || (size_t)length >= sizeof(expected) ||
        input->export_text_size != (size_t)length ||
        memcmp(input->export_text, expected, (size_t)length) != 0) {
        *out_receipt = receipt; return 0;
    }
    receipt.valid = receipt.prs3_evidence_bound = receipt.structure1f_evidence_bound =
        receipt.slev_evidence_bound = receipt.sal_evidence_bound = 1;
    receipt.raw_trace_fnv1a64 = trace_fnv; receipt.raw_trace_byte_count = input->raw_trace_size;
    receipt.dgn_fnv1a64 = prs3->dgn_fnv1a64; receipt.descriptor_index = prs3->descriptor_index;
    receipt.level_index = (uint32_t)slev->level_index; receipt.raw_map_selector = sal->raw_map_selector;
    *out_receipt = receipt; return 1;
}
