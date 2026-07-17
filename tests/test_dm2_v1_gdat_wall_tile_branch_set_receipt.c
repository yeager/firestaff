#include "dm2_v1_gdat_wall_tile_branch_set_receipt.h"
#include <stdio.h>
#include <string.h>
int main(void){DM2_V1_GdatWallTileOverlayReceipt a;DM2_V1_GdatWallTilePicstReceipt b;DM2_V1_GdatWallTileLoadableReceipt c;DM2_V1_GdatWallTileBranchSetReceipt r;memset(&a,0,sizeof(a));memset(&b,0,sizeof(b));memset(&c,0,sizeof(c));a.valid=a.no_draw=b.valid=b.no_draw=c.valid=c.no_draw=1;a.input.category=8;b.input.category=c.input.category=9;c.input.image_field=0x0f;a.identity_hash=1;b.identity_hash=2;c.identity_hash=3;int ok=dm2_v1_gdat_wall_tile_branch_set_receipt_build(&a,&b,&c,&r)&&r.valid&&r.no_draw;c.input.image_field=2;ok&=!dm2_v1_gdat_wall_tile_branch_set_receipt_build(&a,&b,&c,&r);printf("%s dm2_v1_gdat_wall_tile_branch_set_receipt\n",ok?"PASS":"FAIL");return ok?0:1;}
