#include "redmcsb_f0676_f0677_d3_side_pc34_compat.h"
#include <stddef.h>
enum { D3L2_CORRIDOR=0x3421, D3R2_CORRIDOR=0x4312, D3L2_SIDE=0x0321, D3R2_SIDE=0x0412, D3L2_PASS1=0x0218, D3R2_PASS1=0x0128, D3L2_PASS2=0x0349, D3R2_PASS2=0x0439 };
int redmcsb_f0676_f0677_draw_d3_side_pc34_compat(int side, const redmcsb_f0676_square_pc34_compat *s, int16_t d, int16_t x, int16_t y, int flipped, const redmcsb_f0676_runtime_pc34_compat *r) {
    int corridor, side_order, pass1, pass2, flip;
    if (s==NULL || r==NULL || side<0 || side>1 || r->floor_ornament==NULL || r->things==NULL) return 0;
    corridor=side==REDMCSB_F0676_D3L2?D3L2_CORRIDOR:D3R2_CORRIDOR; side_order=side==0?D3L2_SIDE:D3R2_SIDE; pass1=side==0?D3L2_PASS1:D3R2_PASS1; pass2=side==0?D3L2_PASS2:D3R2_PASS2; flip=side==REDMCSB_F0677_D3R2;
    switch(s->element) {
    case REDMCSB_F0676_ELEMENT_STAIRS_FRONT: if(r->stairs==NULL) return 0; r->stairs(r->context,side,s->stairs_up); break;
    case REDMCSB_F0676_ELEMENT_WALL: if(r->wall==NULL || r->wall_ornament==NULL) return 0; r->wall(r->context,side,flipped&&flip); r->wall_ornament(r->context,side,s->wall_ornament); return 1;
    case REDMCSB_F0676_ELEMENT_DOOR_SIDE: case REDMCSB_F0676_ELEMENT_STAIRS_SIDE: r->floor_ornament(r->context,side,s->floor_ornament); r->things(r->context,side,s->first_thing,d,x,y,side_order); return 1;
    case REDMCSB_F0676_ELEMENT_DOOR_FRONT: if(r->door==NULL) return 0; r->floor_ornament(r->context,side,s->floor_ornament); r->things(r->context,side,s->first_thing,d,x,y,pass1); r->door(r->context,side,s->door_thing,s->door_state); r->things(r->context,side,s->first_thing,d,x,y,pass2); return 1;
    case REDMCSB_F0676_ELEMENT_PIT: if(!s->pit_or_teleporter_visible) { if(r->pit==NULL) return 0; r->pit(r->context,side,flip); } break;
    case REDMCSB_F0676_ELEMENT_TELEPORTER: case REDMCSB_F0676_ELEMENT_CORRIDOR: break;
    default: return 1;
    }
    r->floor_ornament(r->context,side,s->floor_ornament); r->things(r->context,side,s->first_thing,d,x,y,corridor);
    if(s->element==REDMCSB_F0676_ELEMENT_TELEPORTER) { if(r->field==NULL) return 0; r->field(r->context,side); } return 1;
}
const char *redmcsb_f0676_f0677_d3_side_source_evidence_pc34(void) { return "ReDMCSB DUNVIEW.C F0676_DrawD3L2 (6226-6291); F0677_DrawD3R2 (6293-6358), PC I34E/I34M"; }
