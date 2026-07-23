#include "dm1_v1_f0481_f0500_graphics_cache_source_receipt_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(void){DM1_V1_F0481F0500InputPc34 i;DM1_V1_F0481F0500ReceiptPc34 r;int ok=1;memset(&i,0,sizeof(i));if(dm1_v1_f0481_f0500_graphics_cache_source_receipt_pc34(&i,&r)){fprintf(stderr,"FAIL missing source admitted\n");ok=0;}if(!r.suppressSyntheticGraphics||r.valid||!r.f0496ExistingOwner||!r.f0500AmigaUnavailable){fprintf(stderr,"FAIL gate\n");ok=0;}if(!strstr(dm1_v1_f0481_f0500_source_evidence_pc34(),"F0481-F0494")||!strstr(dm1_v1_f0481_f0500_source_evidence_pc34(),"F0495-F0497")||dm1_v1_f0481_f0500_fnv1a_pc34(0,0U)){fprintf(stderr,"FAIL audit\n");ok=0;}if(!ok)return 1;puts("PASS dm1_v1_f0481_f0500_graphics_cache_source_receipt_pc34_compat");return 0;}
