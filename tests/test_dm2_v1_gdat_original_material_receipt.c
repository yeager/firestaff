#include "dm2_v1_gdat_original_material_receipt.h"
#include <stdio.h>
int main(void){DM2_V1_GdatPaletteM11ConsumerReceipt p={1,1,0,16,1,2,3,4};DM2_V1_GdatOriginalMaterialReceipt r;uint8_t b[4]={1,2,3,4};uint32_t h=2166136261u;int i,ok;for(i=0;i<4;i++){h^=b[i];h*=16777619u;}ok=dm2_v1_gdat_original_material_receipt_build(&p,b,2,2,2,4,h,&r)&&r.valid&&r.no_draw&&r.bytes==b;++b[0];ok&=!dm2_v1_gdat_original_material_receipt_build(&p,b,2,2,2,4,h,&r);printf("%s dm2_v1_gdat_original_material_receipt\n",ok?"PASS":"FAIL");return ok?0:1;}
