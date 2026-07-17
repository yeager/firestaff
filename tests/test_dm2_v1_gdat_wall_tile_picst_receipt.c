#include "dm2_v1_gdat_wall_tile_picst_receipt.h"
#include <stdio.h>
#include <string.h>
int main(void){DM2_V1_GdatWallTilePicstInput i={9,2,3,0x40,0x40,0,1,2,3,0xfffe,0,7};DM2_V1_GdatWallTilePicstReceipt r;int ok=dm2_v1_gdat_wall_tile_picst_receipt_build(&i,&r)&&r.valid&&r.no_draw&&r.alpha_forced;++i.selector;ok&=dm2_v1_gdat_wall_tile_picst_receipt_build(&i,&r)&&r.identity_hash!=0;i.category=8;ok&=!dm2_v1_gdat_wall_tile_picst_receipt_build(&i,&r);i.category=9;i.active=1;i.alpha=0xfffe;ok&=dm2_v1_gdat_wall_tile_picst_receipt_build(&i,&r)&&!r.alpha_forced;printf("%s dm2_v1_gdat_wall_tile_picst_receipt\n",ok?"PASS":"FAIL");return ok?0:1;}
