#include "dm1_v1_f0341_f0360_render_action_source_receipt_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(void){DM1_V1_F0341F0360InputPc34 i;DM1_V1_F0341F0360ReceiptPc34 r;int ok=1;memset(&i,0,sizeof(i));if(dm1_v1_f0341_f0360_render_action_source_receipt_pc34(&i,&r)){fprintf(stderr,"FAIL missing source admitted\n");ok=0;}if(!r.suppressSyntheticRuntime||r.valid||!r.f0356CpseUnavailable){fprintf(stderr,"FAIL fail-closed gate\n");ok=0;}if(!strstr(dm1_v1_f0341_f0360_source_evidence_pc34(),"F0341-F0355")||!strstr(dm1_v1_f0341_f0360_source_evidence_pc34(),"F0357-F0360")||dm1_v1_f0341_f0360_fnv1a_pc34(0,0U)){fprintf(stderr,"FAIL audit\n");ok=0;}if(!ok)return 1;puts("PASS dm1_v1_f0341_f0360_render_action_source_receipt_pc34_compat");return 0;}
