#include "dm1_v1_f0461_f0480_core_render_source_receipt_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(void){DM1_V1_F0461F0480InputPc34 i;DM1_V1_F0461F0480ReceiptPc34 r;int ok=1;memset(&i,0,sizeof(i));if(dm1_v1_f0461_f0480_core_render_source_receipt_pc34(&i,&r)){fprintf(stderr,"FAIL missing source admitted\n");ok=0;}if(!r.suppressSyntheticRendering||r.valid||!r.f0464CpseUnavailable){fprintf(stderr,"FAIL gate\n");ok=0;}if(!strstr(dm1_v1_f0461_f0480_source_evidence_pc34(),"F0461-F0463")||!strstr(dm1_v1_f0461_f0480_source_evidence_pc34(),"F0466-F0480")||dm1_v1_f0461_f0480_fnv1a_pc34(0,0U)){fprintf(stderr,"FAIL audit\n");ok=0;}if(!ok)return 1;puts("PASS dm1_v1_f0461_f0480_core_render_source_receipt_pc34_compat");return 0;}
