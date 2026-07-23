#include "csb_v1_f1106_f1125_media_palette_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failed,checked;
#define CHECK(x) do{++checked;if(!(x)){++failed;fprintf(stderr,"FAIL:%d:%s\n",__LINE__,#x);}}while(0)
int main(void){uint8_t b[8]={1},before[8];CSB_V1_MediaPaletteRawPc34 raw={b,8,1,b,8,2,1};CSB_V1_MediaPaletteReceiptPc34 r;memcpy(before,b,sizeof(b));CHECK(csb_v1_f1106_f1125_media_palette_audit_pc34(&raw,1122,&r)==1);CHECK(r.raw_material_admitted&&r.palette_required&&r.runtime_execution_blocked&&r.source_evidence!=NULL);CHECK(memcmp(b,before,sizeof(b))==0);raw.palette_identity=0;CHECK(csb_v1_f1106_f1125_media_palette_audit_pc34(&raw,1122,&r)==0);raw.palette_identity=2;CHECK(csb_v1_f1106_f1125_media_palette_audit_pc34(&raw,1106,&r)==0);CHECK(r.platform_behavior_fail_closed&&r.source_evidence!=NULL);CHECK(csb_v1_f1106_f1125_media_palette_audit_pc34(&raw,1123,&r)==0);printf("csb_v1_f1106_f1125_media_palette: %d/%d assertions passed\n",checked-failed,checked);return failed!=0;}
