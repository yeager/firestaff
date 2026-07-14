#include "redmcsb_f0678_f0679_d2_side_pc34_compat.h"
#include <stddef.h>
int redmcsb_f0678_f0679_draw_d2_side_pc34_compat(int side, int16_t element, int flipped, const redmcsb_f0678_f0679_runtime_pc34_compat *r) {
    int zone, normal_wall, flipped_wall, field_aspect;
    if (r==NULL || side<0 || side>1) return 0;
    zone=side==REDMCSB_F0678_D2L2?707:708;
    normal_wall=side==REDMCSB_F0678_D2L2?6:5;
    flipped_wall=side==REDMCSB_F0678_D2L2?5:6;
    field_aspect=side==REDMCSB_F0678_D2L2?5:6;
    if(element==REDMCSB_F0678_ELEMENT_WALL) { if(r->draw_wall==NULL)return 0; r->draw_wall(r->context,side,flipped?flipped_wall:normal_wall,zone,flipped!=0); return 1; }
    if(element==REDMCSB_F0678_ELEMENT_TELEPORTER) { if(r->draw_teleporter_field==NULL)return 0; r->draw_teleporter_field(r->context,side,field_aspect,zone); }
    return 1;
}
const char *redmcsb_f0678_f0679_d2_side_source_evidence_pc34(void) { return "ReDMCSB DUNVIEW.C F0678_DrawD2L2 (6837-6865); F0679_DrawD2R2 (6868-6896), PC I34E/I34M"; }
