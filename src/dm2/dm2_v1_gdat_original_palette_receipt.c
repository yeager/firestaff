#include "dm2_v1_gdat_original_palette_receipt.h"
#include <string.h>
static uint32_t hash(const uint8_t*p,uint16_t n){uint32_t h=2166136261u;while(n--){h^=*p++;h*=16777619u;}return h;}
int dm2_v1_gdat_original_palette_receipt_build(const DM2_V1_GdatDrawTempPaletteSurfaceReceipt*s,const uint8_t*b,uint16_t n,uint32_t expected,DM2_V1_GdatOriginalPaletteReceipt*out){uint32_t h;if(!out)return 0;
    memset(out,0,sizeof(*out));/* c_querydb.cpp:2506-2539 reads the supplied palette only. */if(!s||!s->valid||!s->no_draw||!s->identity_hash||!b||(n!=16&&n!=256)||!expected||(h=hash(b,n))!=expected)return 0;out->valid=out->no_draw=1;out->bytes=b;out->byte_count=n;out->byte_hash=h;out->palette_surface_hash=s->identity_hash;out->identity_hash=(h^s->identity_hash)*16777619u;return out->identity_hash!=0;}
