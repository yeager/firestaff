#include "dm2_v1_gdat_b073_input_receipt.h"
#include <stdio.h>
int main(void){DM2_V1_GdatB073Input i={1,2,3,4,0xfffe,16,0,20,0};DM2_V1_GdatB073InputReceipt r;int ok=dm2_v1_gdat_b073_input_receipt_build(&i,&r)&&r.valid&&r.no_draw;i.lookup_identity=0;ok&=!dm2_v1_gdat_b073_input_receipt_build(&i,&r);i.lookup_identity=3;i.cache_owned=1;i.cache_allocation=0x100;ok&=dm2_v1_gdat_b073_input_receipt_build(&i,&r);printf("%s dm2_v1_gdat_b073_input_receipt\n",ok?"PASS":"FAIL");return ok?0:1;}
