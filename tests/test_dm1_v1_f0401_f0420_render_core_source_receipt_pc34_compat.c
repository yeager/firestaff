#include "dm1_v1_f0401_f0420_render_core_source_receipt_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(void){DM1_V1_F0401F0420InputPc34 i;DM1_V1_F0401F0420ReceiptPc34 r;int ok=1;memset(&i,0,sizeof(i));if(dm1_v1_f0401_f0420_render_core_source_receipt_pc34(&i,&r)){fprintf(stderr,"FAIL missing source admitted\n");ok=0;}if(!r.suppressSyntheticRuntime||r.valid||!r.f0413CpseUnavailable){fprintf(stderr,"FAIL gate\n");ok=0;}if(!strstr(dm1_v1_f0401_f0420_source_evidence_pc34(),"F0401-F0412")||!strstr(dm1_v1_f0401_f0420_source_evidence_pc34(),"F0414-F0420")||dm1_v1_f0401_f0420_fnv1a_pc34(0,0U)){fprintf(stderr,"FAIL audit\n");ok=0;}if(!ok)return 1;puts("PASS dm1_v1_f0401_f0420_render_core_source_receipt_pc34_compat");return 0;}
