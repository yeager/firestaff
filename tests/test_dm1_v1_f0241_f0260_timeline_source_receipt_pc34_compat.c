#include "dm1_v1_f0241_f0260_timeline_source_receipt_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(void){DM1_V1_F0241F0260InputPc34 i;DM1_V1_F0241F0260ReceiptPc34 r;int ok=1;memset(&i,0,sizeof(i));if(dm1_v1_f0241_f0260_timeline_source_receipt_pc34(&i,&r)){fprintf(stderr,"FAIL missing source admitted\n");ok=0;}if(!r.suppressSyntheticRuntime||r.valid||!r.f0256CpseUnavailable){fprintf(stderr,"FAIL receipt gate\n");ok=0;}if(!strstr(dm1_v1_f0241_f0260_source_evidence_pc34(),"F0241-F0260")||!strstr(dm1_v1_f0241_f0260_source_evidence_pc34(),"F0256")||dm1_v1_f0241_f0260_fnv1a_pc34(0,0U)){fprintf(stderr,"FAIL audit\n");ok=0;}if(!ok)return 1;puts("PASS dm1_v1_f0241_f0260_timeline_source_receipt_pc34_compat");return 0;}
