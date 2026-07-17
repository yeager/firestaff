#include "dm2_v1_gdat_original_palette_receipt.h"
#include <stdio.h>
int main(void){DM2_V1_GdatDrawTempPaletteSurfaceReceipt s={1,1,1,2,3,{0}};DM2_V1_GdatOriginalPaletteReceipt r;uint8_t p[16]={1};uint32_t h=2166136261u;int i,ok;for(i=0;i<16;i++){h^=p[i];h*=16777619u;}ok=dm2_v1_gdat_original_palette_receipt_build(&s,p,16,h,&r)&&r.valid&&r.no_draw&&r.bytes==p;++p[0];ok&=!dm2_v1_gdat_original_palette_receipt_build(&s,p,16,h,&r);printf("%s dm2_v1_gdat_original_palette_receipt\n",ok?"PASS":"FAIL");return ok?0:1;}
