#include "dm1_v1_f0281_f0300_champion_source_receipt_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(void){DM1_V1_F0281F0300InputPc34 i;DM1_V1_F0281F0300ReceiptPc34 r;int ok=1;memset(&i,0,sizeof(i));if(dm1_v1_f0281_f0300_champion_source_receipt_pc34(&i,&r)){fprintf(stderr,"FAIL missing source admitted\n");ok=0;}if(!r.suppressSyntheticUi||r.valid){fprintf(stderr,"FAIL synthetic UI not suppressed\n");ok=0;}if(!strstr(dm1_v1_f0281_f0300_source_evidence_pc34(),"F0281")||!strstr(dm1_v1_f0281_f0300_source_evidence_pc34(),"F0300")||dm1_v1_f0281_f0300_fnv1a_pc34(0,0U)){fprintf(stderr,"FAIL audit\n");ok=0;}if(!ok)return 1;puts("PASS dm1_v1_f0281_f0300_champion_source_receipt_pc34_compat");return 0;}
