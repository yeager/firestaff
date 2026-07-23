#include "csb_v1_f1026_f1045_platform_video_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failed,checked;
#define CHECK(x) do{++checked;if(!(x)){++failed;fprintf(stderr,"FAIL:%d:%s\n",__LINE__,#x);}}while(0)
int main(void){uint8_t b[8]={1},before[8];CSB_V1_PlatformVideoRawPc34 raw={b,8,1,b,8,2,b,8,3,1};CSB_V1_PlatformVideoReceiptPc34 r;int i;static const int ids[]={1026,1029,1030,1031,1032,1033};memcpy(before,b,sizeof(b));for(i=0;i<(int)(sizeof(ids)/sizeof(ids[0]));++i){CHECK(csb_v1_f1026_f1045_platform_video_audit_pc34(&raw,ids[i],&r)==1);CHECK(r.raw_material_admitted&&r.existing_runtime_owner_preserved&&r.runtime_execution_blocked&&r.source_evidence!=NULL);}CHECK(memcmp(b,before,sizeof(b))==0);raw.graphics_identity=0;CHECK(csb_v1_f1026_f1045_platform_video_audit_pc34(&raw,1032,&r)==0);raw.graphics_identity=2;CHECK(csb_v1_f1026_f1045_platform_video_audit_pc34(&raw,1034,&r)==0);CHECK(r.platform_behavior_fail_closed&&r.source_evidence!=NULL);printf("csb_v1_f1026_f1045_platform_video: %d/%d assertions passed\n",checked-failed,checked);return failed!=0;}
