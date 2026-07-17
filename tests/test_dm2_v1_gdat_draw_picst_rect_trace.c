#include "dm2_v1_gdat_draw_picst_rect_trace.h"
#include <stdio.h>
int main(void){DM2_V1_GdatDrawPicstTraceReceipt t={1,1,1,1,1};DM2_V1_GdatDrawPicstRectTrace r;int ok=dm2_v1_gdat_draw_picst_rect_trace_build(&t,-1,2,3,4,5,6,7,8,&r)&&r.valid&&r.no_draw&&r.source_x==6&&r.source_y==8;ok&=!dm2_v1_gdat_draw_picst_rect_trace_build(&t,0,2,3,4,5,6,7,8,&r);printf("%s dm2_v1_gdat_draw_picst_rect_trace\n",ok?"PASS":"FAIL");return ok?0:1;}
