#include "dm2_v1_gdat_b073_input_receipt.h"
#include <string.h>
int dm2_v1_gdat_b073_input_receipt_build(const DM2_V1_GdatB073Input*i,DM2_V1_GdatB073InputReceipt*out){const uint8_t*p;size_t n;uint32_t h=2166136261u;if(!out)return 0;
    memset(out,0,sizeof(*out));/* SKULLWIN/c_querydb.cpp:2506-2545. */if(!i||!i->palette_identity||!i->light||!i->alpha_mask||!i->colors||!i->raw7_identity||!i->lookup_identity||!i->traversal_identity||(!i->cache_owned&&i->cache_allocation!=0)|| (i->cache_owned&&!i->cache_allocation))return 0;out->valid=out->no_draw=1;out->input=*i;p=(const uint8_t*)i;n=sizeof(*i);while(n--){h^=*p++;h*=16777619u;}out->identity_hash=h?h:1;return 1;}
