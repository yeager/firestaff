#include "nexus_v1_saturn_capture_campaign_import.h"

#include <stdio.h>
#include <string.h>

static uint64_t fnv(const uint8_t *p, size_t n) { uint64_t h = UINT64_C(1469598103934665603); size_t i; for (i=0;i<n;++i) { h^=p[i]; h*=UINT64_C(1099511628211); } return h; }
static size_t make_export(char *out, size_t cap, uint64_t hash)
{
    return (size_t)snprintf(out, cap,
        NEXUS_V1_SATURN_CAPTURE_CAMPAIGN_MAGIC "\nproducer=mednafen-saturn-debugger\ntrace_sha256=0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\nraw_trace_fnv1a64=%016llx\nraw_trace_size=5\nprs3=dgn:0000000000000011,descriptor:2,frame:3,command:4\nstructure1f=level:0,dgn:0000000000000011,descriptor:2,mesh:5,face:6\nslev=level:0,source:0000000000000022,entry:00000010,task:00000020,callback:00000030\nsal=level:0,selector:40,sal:0000000000000033,map:0000000000000044,offset:100,size:20\n", (unsigned long long)hash);
}
int main(void)
{
    static const uint8_t raw[] = { 1, 2, 3, 4, 5 };
    Nexus_V1_Prs3DgnPlacementAdapterReceipt prs3; Nexus_V1_Structure1FCorpusTraceTarget structure1f;
    Nexus_V1_SlevTaskBodyCaptureTarget slev; Nexus_V1_SalCapturePlanTarget sal;
    Nexus_V1_SaturnCaptureCampaignImportInput in; Nexus_V1_SaturnCaptureCampaignReceipt receipt; char text[1024]; size_t size; uint64_t h = fnv(raw, sizeof(raw));
    memset(&prs3,0,sizeof(prs3)); memset(&structure1f,0,sizeof(structure1f)); memset(&slev,0,sizeof(slev)); memset(&sal,0,sizeof(sal)); memset(&in,0,sizeof(in));
    prs3.valid=prs3.trace_bound=prs3.dgn_source_bound=prs3.descriptor_envelope_bound=prs3.command_coordinates_bound=prs3.dgn_placement_observed=prs3.no_draw_only=1; prs3.trace_fnv1a64=h; prs3.dgn_fnv1a64=0x11; prs3.descriptor_index=2; prs3.descriptor_fnv1a64=0x12; prs3.frame_sequence=3; prs3.command_sequence=4;
    structure1f.valid=structure1f.original_saturn_trace_required=structure1f.no_draw_only=structure1f.blocks_real_dgn_mesh_render=1; structure1f.level_index=0; structure1f.dgn_fnv1a64=0x11; structure1f.descriptor_index=2; structure1f.descriptor_fnv1a64=0x12; structure1f.mesh_index=5; structure1f.face_index=6;
    slev.valid=slev.source_order_required=slev.original_saturn_trace_required=slev.no_dispatch_only=1; slev.level_index=0; slev.source_fnv1a64=0x22; slev.entry_pc=0x10; slev.task_body_pc=0x20; slev.callback_or_write_pc=0x30; slev.raw_trace_fnv1a64=h; slev.raw_trace_byte_count=sizeof(raw);
    sal.valid=sal.source_order_required=sal.original_saturn_trace_required=sal.no_playback_only=1; sal.level_index=0; sal.raw_map_selector=0x40; sal.canonical_sal_fnv1a64=0x33; sal.canonical_map_fnv1a64=0x44; sal.sal_offset=0x100; sal.sal_size=0x20; sal.raw_trace_fnv1a64=h; sal.raw_trace_byte_count=sizeof(raw);
    size=make_export(text,sizeof(text),h); in.prs3=&prs3; in.structure1f=&structure1f; in.slev=&slev; in.sal=&sal; in.raw_trace=raw; in.raw_trace_size=sizeof(raw); in.trace_sha256="0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"; in.export_text=text; in.export_text_size=size;
    if (!nexus_v1_saturn_capture_campaign_import(&in,&receipt) || !receipt.valid || !receipt.evidence_only || receipt.decoder_permitted || receipt.geometry_semantics_permitted || receipt.script_semantics_permitted || receipt.playback_permitted) return 1;
    text[0]='X'; if (nexus_v1_saturn_capture_campaign_import(&in,&receipt) || receipt.valid) return 1; text[0]='F';
    prs3.trace_fnv1a64++; if (nexus_v1_saturn_capture_campaign_import(&in,&receipt)) return 1; prs3.trace_fnv1a64=h;
    structure1f.descriptor_index++; if (nexus_v1_saturn_capture_campaign_import(&in,&receipt)) return 1; structure1f.descriptor_index=2;
    slev.raw_trace_byte_count++; if (nexus_v1_saturn_capture_campaign_import(&in,&receipt)) return 1; slev.raw_trace_byte_count=sizeof(raw);
    sal.no_playback_only=0; if (nexus_v1_saturn_capture_campaign_import(&in,&receipt)) return 1;
    puts("saturn capture campaign import: PASS"); return 0;
}
