#include "csb_v1_f0906_f0925_swoosh_primitive_raw_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failed,checked;
#define CHECK(x) do{++checked;if(!(x)){++failed;fprintf(stderr,"FAIL:%d:%s\n",__LINE__,#x);}}while(0)
static CSB_V1_SwooshPrimitiveRawPc34 raw(uint8_t*b){CSB_V1_SwooshPrimitiveRawPc34 r;memset(&r,0,sizeof(r));r.package=r.graphics=r.sound=r.utility=b;r.package_size=r.graphics_size=r.sound_size=r.utility_size=8;r.package_identity=1;r.graphics_identity=2;r.sound_identity=3;r.utility_identity=4;r.authenticated_pc34=1;return r;}
int main(void){uint8_t b[8]={1},before[8];CSB_V1_SwooshPrimitiveRawPc34 r=raw(b);CSB_V1_SwooshPrimitiveReceiptPc34 q;int i;static const int ids[]={906,907,908,909,910,913,914,915,917,918,919,920,922,924,925};memcpy(before,b,sizeof(b));for(i=0;i<(int)(sizeof(ids)/sizeof(ids[0]));++i){CHECK(csb_v1_f0906_f0925_swoosh_primitive_audit_pc34(&r,ids[i],&q)==1);CHECK(q.read_only_query&&q.runtime_execution_blocked&&q.source_evidence!=NULL);}CHECK(memcmp(b,before,sizeof(b))==0);r.sound_identity=0;CHECK(csb_v1_f0906_f0925_swoosh_primitive_audit_pc34(&r,909,&q)==0);r=raw(b);CHECK(csb_v1_f0906_f0925_swoosh_primitive_audit_pc34(&r,911,&q)==0);CHECK(csb_v1_f0906_f0925_swoosh_primitive_audit_pc34(&r,916,&q)==0);printf("csb_v1_f0906_f0925_swoosh_primitive_raw: %d/%d assertions passed\n",checked-failed,checked);return failed!=0;}
