#include "csb_v1_f1046_f1065_save_platform_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failed,checked;
#define CHECK(x) do{++checked;if(!(x)){++failed;fprintf(stderr,"FAIL:%d:%s\n",__LINE__,#x);}}while(0)
int main(void){uint8_t b[8]={1},before[8];CSB_V1_SavePlatformRawPc34 raw={b,8,1,b,8,2,1};CSB_V1_SavePlatformReceiptPc34 r;memcpy(before,b,sizeof(b));CHECK(csb_v1_f1046_f1065_save_platform_audit_pc34(&raw,1052,&r)==1);CHECK(r.graphics_required&&r.runtime_execution_blocked&&r.source_evidence!=NULL);CHECK(csb_v1_f1046_f1065_save_platform_audit_pc34(&raw,1057,&r)==1);CHECK(r.save_required&&r.existing_runtime_owner_preserved);CHECK(csb_v1_f1046_f1065_save_platform_audit_pc34(&raw,1059,&r)==1);CHECK(memcmp(b,before,sizeof(b))==0);raw.save_identity=0;CHECK(csb_v1_f1046_f1065_save_platform_audit_pc34(&raw,1057,&r)==0);raw.save_identity=1;CHECK(csb_v1_f1046_f1065_save_platform_audit_pc34(&raw,1048,&r)==0);CHECK(r.platform_behavior_fail_closed&&r.source_evidence!=NULL);printf("csb_v1_f1046_f1065_save_platform: %d/%d assertions passed\n",checked-failed,checked);return failed!=0;}
