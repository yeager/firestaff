#include "dm1_v1_f0201_f0220_action_source_receipt_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(void){DM1_V1_F0201F0220InputPc34 in;DM1_V1_F0201F0220ReceiptPc34 r;int ok=1;memset(&in,0,sizeof(in));if(dm1_v1_f0201_f0220_action_source_receipt_pc34(&in,&r)){fprintf(stderr,"FAIL missing data admitted\n");ok=0;}if(!r.suppressSyntheticRuntime||r.valid||!r.f0210F0211CopyProtectionUnavailable){fprintf(stderr,"FAIL fail-closed receipt\n");ok=0;}if(!strstr(dm1_v1_f0201_f0220_source_evidence_pc34(),"F0209")||!strstr(dm1_v1_f0201_f0220_source_evidence_pc34(),"F0210/F0211")||dm1_v1_f0201_f0220_fnv1a_pc34(0,0U)){fprintf(stderr,"FAIL source audit\n");ok=0;}if(!ok)return 1;puts("PASS dm1_v1_f0201_f0220_action_source_receipt_pc34_compat");return 0;}
