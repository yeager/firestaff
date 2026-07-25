#include "dm2_v1_gdat_wall_tile_loadable_receipt.h"
#include <string.h>
int dm2_v1_gdat_wall_tile_loadable_receipt_build(const DM2_V1_GdatWallTileLoadableInput*i,DM2_V1_GdatWallTileLoadableReceipt*out){const uint8_t*p;size_t n;uint32_t h=2166136261u;if(!out)return 0;
    memset(out,0,sizeof(*out));/* SKULLWIN/c_gui_vp.cpp:6651-6692. */if(!i||i->category!=9||!i->selector||i->image_field!=0x0f||!i->loadable||!i->source_identity||i->scale_x!=0x40||i->scale_y!=0x40||i->flip>1||i->query1==0xffff||i->query2==0xffff||i->query3==0xffff||!i->alpha)return 0;out->valid=out->no_draw=1;out->input=*i;out->alpha_forced=i->active==0;if(out->alpha_forced&&i->alpha!=0xfffe)return 0;p=(const uint8_t*)i;n=sizeof(*i);while(n--){h^=*p++;h*=16777619u;}out->identity_hash=h?h:1;return 1;}
