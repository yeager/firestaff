#include "dm2_v1_gdat_wall_tile_receipt.h"
#include <string.h>
static const uint8_t branch[23]={0,2,2,1,3,3,1,3,3,2,2,1,3,3,3,3,1,1,1,1,1,0,0};
static const int8_t orientation[23]={0,-1,1,0,-1,1,0,-1,1,-2,2,0,-1,1,-2,2,0,-1,1,-2,2,-3,3};
static uint32_t mix(uint32_t h,uint32_t x){h^=x;return h*16777619u;}
int dm2_v1_gdat_wall_tile_receipt_build(uint8_t cell,const DM2_V1_GdatWallM11CommandPlan*w,const DM2_V1_Dm2ViewportM11CompositionReceipt*c,DM2_V1_GdatWallTileReceipt*out){uint32_t h=2166136261u;if(!out)return 0;
    memset(out,0,sizeof(*out));/* c_gui_vp.cpp:6703-6741; dm2data.cpp:602-605,266-273. */if(!w||!c||cell>=23||!w->valid||!w->command_count||!w->command_hash||!c->valid||!c->no_draw||c->wall_command_hash!=w->command_hash||!c->identity_hash)return 0;out->valid=out->no_draw=1;out->cell=cell;out->table_branch=branch[cell];out->delegate_count=branch[cell]==3?2:(branch[cell]?1:0);out->wall_orientation=orientation[cell];out->wall_hash=w->command_hash;out->composition_hash=c->identity_hash;h=mix(h,cell);h=mix(h,out->table_branch);h=mix(h,(uint8_t)out->wall_orientation);h=mix(h,out->wall_hash);h=mix(h,out->composition_hash);out->identity_hash=h?h:1;return 1;}
