#include "nexus_v1_prs3_original_execution_import.h"

#include <stdio.h>
#include <string.h>
static uint64_t fnv(const uint8_t *p, size_t n) { uint64_t h=UINT64_C(1469598103934665603); size_t i; for(i=0;i<n;++i){h^=p[i];h*=UINT64_C(1099511628211);} return h; }
static void fixture(Nexus_V1_Prs3Vdp1CaptureReceipt *t, Nexus_V1_Prs3Vdp1CaptureBindingReceipt *b, Nexus_V1_Prs3OriginalExecutionAuthentication *a, const uint8_t *raw, size_t size)
{
    memset(t,0,sizeof(*t)); memset(b,0,sizeof(*b)); memset(a,0,sizeof(*a));
    t->valid=t->complete_capture=t->complete_output_store_range_observed=t->output_store_predecessor_observed=t->vdp1_texture_consumption_observed=t->vdp1_command_consumption_observed=1; t->schema_version=10; t->menu_bpk_fnv1a64=1; t->dm_bin_fnv1a64=2; t->entry_index=3; t->stream_size=t->expected_output_bytes=t->input_read_bytes=t->output_write_bytes=t->output_store_bytes=t->vdp1_texture_source_bytes=4; t->payload_ram_address=t->first_input_read_address=0x100; t->last_input_read_address=0x103; t->output_ram_address=t->first_output_write_address=t->vdp1_texture_source_address=0x200; t->last_output_write_address=0x203; t->output_fnv1a64=t->output_store_fnv1a64=5; t->last_output_write_sequence=7; t->vdp1_command_sequence=8;
    b->valid=b->trace_valid=b->menu_bpk_matches=b->dm_bin_matches=b->entry_plan_matches=b->payload_span_matches=b->exact_vdp1_handoff_observed=b->vdp1_texture_consumption_observed=b->vdp1_command_consumption_observed=1;
    a->valid=a->mednafen_saturn_debugger_export=a->independently_authenticated_original_saturn_execution=1; a->trace_export_fnv1a64=fnv(raw,size); strcpy(a->trace_export_sha256,"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef");
}
int main(void)
{
    static const uint8_t raw[]="mednafen-export"; Nexus_V1_Prs3Vdp1CaptureReceipt t; Nexus_V1_Prs3Vdp1CaptureBindingReceipt b; Nexus_V1_Prs3OriginalExecutionAuthentication a; Nexus_V1_Prs3OriginalExecutionImportInput in; Nexus_V1_Prs3OriginalExecutionEvidenceReceipt r;
    fixture(&t,&b,&a,raw,sizeof(raw)-1U); memset(&in,0,sizeof(in)); in.trace=&t;in.binding=&b;in.authentication=&a;in.trace_export_bytes=raw;in.trace_export_size=sizeof(raw)-1U;
    if(!nexus_v1_prs3_original_execution_evidence_import(&in,&r)||!r.valid||!r.evidence_only||r.decoder_promoted||r.graphics_permitted) return 1;
    t.input_read_bytes=3;if(nexus_v1_prs3_original_execution_evidence_import(&in,&r))return 1;fixture(&t,&b,&a,raw,sizeof(raw)-1U);
    t.vdp1_command_sequence=7;if(nexus_v1_prs3_original_execution_evidence_import(&in,&r))return 1;fixture(&t,&b,&a,raw,sizeof(raw)-1U);
    t.output_store_fnv1a64=6;if(nexus_v1_prs3_original_execution_evidence_import(&in,&r))return 1;fixture(&t,&b,&a,raw,sizeof(raw)-1U);
    a.independently_authenticated_original_saturn_execution=0;if(nexus_v1_prs3_original_execution_evidence_import(&in,&r))return 1;
    puts("prs3 original execution import: PASS");return 0;
}
