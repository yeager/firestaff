#include "dm2_v1_gdat_wall_draw_picst_admission.h"
#include <stdio.h>
#include <string.h>
int main(void){DM2_V1_GdatWallM11CommandPlan p;DM2_V1_GdatWallDrawPicstAdmission r;uint8_t px=0;memset(&p,0,sizeof(p));p.valid=1;p.command_count=1;p.command_hash=1;p.commands[0].pixels=&px;p.commands[0].width=p.commands[0].height=1;p.commands[0].raw_hash=p.commands[0].decoded_hash=p.commands[0].palette_hash=p.commands[0].material_receipt_hash=p.commands[0].geometry_hash=1;int ok=dm2_v1_gdat_wall_draw_picst_admission_build(&p,0,&r)&&r.valid&&r.no_draw&&r.palette_entries==16; p.commands[0].palette_hash=0;ok&=!dm2_v1_gdat_wall_draw_picst_admission_build(&p,0,&r);printf("%s dm2_v1_gdat_wall_draw_picst_admission\n",ok?"PASS":"FAIL");return ok?0:1;}
