#include "csb_v1_f1086_f1105_input_boundary_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failed,checked;
#define CHECK(x) do{++checked;if(!(x)){++failed;fprintf(stderr,"FAIL:%d:%s\n",__LINE__,#x);}}while(0)
int main(void){uint8_t b[8]={1},before[8];CSB_V1_InputBoundaryRawPc34 raw={b,8,1,b,8,2,1};CSB_V1_InputBoundaryReceiptPc34 r;int f;memcpy(before,b,sizeof(b));for(f=1097;f<=1102;++f){CHECK(csb_v1_f1086_f1105_input_boundary_audit_pc34(&raw,f,&r)==1);CHECK(r.raw_material_admitted&&r.input_required&&r.runtime_execution_blocked&&r.source_evidence!=NULL);}CHECK(memcmp(b,before,sizeof(b))==0);raw.input_identity=0;CHECK(csb_v1_f1086_f1105_input_boundary_audit_pc34(&raw,1097,&r)==0);raw.input_identity=2;CHECK(csb_v1_f1086_f1105_input_boundary_audit_pc34(&raw,1086,&r)==0);CHECK(r.platform_behavior_fail_closed&&r.source_evidence!=NULL);CHECK(csb_v1_f1086_f1105_input_boundary_audit_pc34(&raw,1103,&r)==0);printf("csb_v1_f1086_f1105_input_boundary: %d/%d assertions passed\n",checked-failed,checked);return failed!=0;}
