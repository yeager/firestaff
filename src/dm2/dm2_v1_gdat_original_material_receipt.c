#include "dm2_v1_gdat_original_material_receipt.h"
#include <string.h>
static uint32_t hash(const uint8_t*p,uint32_t n){uint32_t h=2166136261u;while(n--){h^=*p++;h*=16777619u;}return h;}
int dm2_v1_gdat_original_material_receipt_build(const DM2_V1_GdatPaletteM11ConsumerReceipt*p,const uint8_t*b,uint16_t w,uint16_t h,uint16_t stride,uint32_t count,uint32_t expected,DM2_V1_GdatOriginalMaterialReceipt*out){uint32_t v;if(!out)return 0;
    memset(out,0,sizeof(*out));/* QUERY_TEMP_PICST receives already-decoded image storage; no decoder is admitted here. */if(!p||!p->valid||!p->no_draw||!p->identity_hash||!b||!w||!h||stride<w||count!=(uint32_t)stride*h||!expected||(v=hash(b,count))!=expected)return 0;out->valid=out->no_draw=1;out->bytes=b;out->width=w;out->height=h;out->stride=stride;out->byte_count=count;out->byte_hash=v;out->palette_consumer_hash=p->identity_hash;out->identity_hash=(v^p->identity_hash)*16777619u;return out->identity_hash!=0;}
