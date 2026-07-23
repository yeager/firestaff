#include "csb_v1_f0786_f0805_panel_layout_raw_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failed,checked;
#define CHECK(x) do{++checked;if(!(x)){++failed;fprintf(stderr,"FAIL:%d:%s\n",__LINE__,#x);}}while(0)
static CSB_V1_PanelLayoutRawPc34 raw(uint8_t*b){CSB_V1_PanelLayoutRawPc34 r;memset(&r,0,sizeof(r));r.graphics=r.zone=r.dungeon=r.sound=r.package=b;r.graphics_size=r.zone_size=r.dungeon_size=r.sound_size=r.package_size=8;r.graphics_identity=1;r.zone_identity=2;r.dungeon_identity=3;r.sound_identity=4;r.package_identity=5;r.authenticated_pc34=1;return r;}
int main(void){uint8_t b[8]={1},before[8];CSB_V1_PanelLayoutRawPc34 r=raw(b);CSB_V1_PanelLayoutReceiptPc34 q;int i;static const int ids[]={786,787,788,789,790,791,792,797,798,799,802,803,804,805};memcpy(before,b,sizeof(b));for(i=0;i<(int)(sizeof(ids)/sizeof(ids[0]));++i){CHECK(csb_v1_f0786_f0805_panel_layout_audit_pc34(&r,ids[i],&q)==1);CHECK(q.read_only_query&&q.runtime_execution_blocked&&q.source_evidence!=NULL);}CHECK(memcmp(b,before,sizeof(b))==0);r.sound_identity=0;CHECK(csb_v1_f0786_f0805_panel_layout_audit_pc34(&r,799,&q)==0);r=raw(b);CHECK(csb_v1_f0786_f0805_panel_layout_audit_pc34(&r,793,&q)==0);CHECK(csb_v1_f0786_f0805_panel_layout_audit_pc34(&r,800,&q)==0);printf("csb_v1_f0786_f0805_panel_layout_raw: %d/%d assertions passed\n",checked-failed,checked);return failed!=0;}
