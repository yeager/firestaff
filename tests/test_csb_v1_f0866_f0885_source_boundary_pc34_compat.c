#include "csb_v1_f0866_f0885_source_boundary_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failed,checked;
#define CHECK(x) do{++checked;if(!(x)){++failed;fprintf(stderr,"FAIL:%d:%s\n",__LINE__,#x);}}while(0)
int main(void){uint8_t bytes[4]={1,2,3,4},before[4];CSB_V1_F0866_F0885_RawPc34 raw={bytes,4,1,bytes,4,2,1};CSB_V1_F0866_F0885_ReceiptPc34 r;int f;memcpy(before,bytes,sizeof(bytes));for(f=866;f<=885;++f){CHECK(csb_v1_f0866_f0885_source_boundary_pc34(&raw,f,&r)==0);CHECK(r.source_symbol_missing&&r.raw_material_rejected&&r.runtime_execution_blocked&&r.platform_behavior_fail_closed&&r.function_number==f&&r.source_evidence!=NULL);}CHECK(memcmp(bytes,before,sizeof(bytes))==0);CHECK(csb_v1_f0866_f0885_source_boundary_pc34(&raw,865,&r)==0);CHECK(r.source_evidence==NULL);printf("csb_v1_f0866_f0885_source_boundary: %d/%d assertions passed\n",checked-failed,checked);return failed!=0;}
