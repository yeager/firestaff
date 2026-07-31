#include "csb_v1_viewport_d3l2_wall_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(void) {
 const CSB_V1_ViewportD3L2WallRouteSpec *s=csb_v1_viewport_d3l2_wall_route_spec_pc34(); int x,y,w,h;
 int ok=s && s->source_locked_contract_only && s->transparent_color==10 && s->preserves_c10_transparency &&
 csb_v1_viewport_d3l2_wall_resolve_zone_pc34(s,CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3L2,&x,&y,&w,&h)==0 && x==0 && y==25 &&
 csb_v1_viewport_d3l2_wall_resolve_zone_pc34(s,CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3R2,&x,&y,&w,&h)==0 && x==208 &&
 strstr(csb_v1_viewport_d3l2_wall_source_evidence_pc34(),"F0676");
 printf("CSB D3L2 wall metadata: failures=%d\n",!ok); return !ok;
}
