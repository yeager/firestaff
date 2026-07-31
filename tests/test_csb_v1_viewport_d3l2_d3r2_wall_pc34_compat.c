#include "csb_v1_viewport_d3l2_d3r2_wall_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(void) {
 CSB_V1_ViewportD3L2D3R2WallPartyPositionPc34 p={10,10,0};
 CSB_V1_ViewportD3L2D3R2WallPositionPc34 l={8,7,14,3,-2,0}, r={12,7,15,3,2,0};
 CSB_V1_ViewportD3L2D3R2WallRenderResultPc34 out; uint8_t a[1]={0},b[1]={0},d[1]={0};
 int ok=csb_v1_viewport_d3l2_d3r2_wall_render_square_pc34(&p,&l,&r,a,b,1,d,1,1,&out)==0 && out.ok && !out.left_drawn && !out.right_drawn && out.left_copied_pixels==0 && out.right_copied_pixels==0 && out.relative_square_gate_ok && out.wall_band_clip_ok && strstr(csb_v1_viewport_d3l2_d3r2_wall_source_evidence_pc34(),"F0676");
 printf("CSB D3L2/D3R2 no-draw metadata: failures=%d\n",!ok); return !ok;
}
