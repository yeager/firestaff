#include "dm2_v1_weather_gdat.h"
#include <string.h>
int dm2_v1_weather_gdat_receipt(const DM2_V1_AssetLoader *l, uint8_t gs, DM2_V1_WeatherGdatReceipt *out) {
    static const uint8_t fields[] = {0x64,0x67,0x6a,0x6d,0x71}; uint16_t fog;
    if (!out) return 0; memset(out,0,sizeof(*out));
    if (!l || !dm2_v1_asset_load_word_value(l,DM2_GDAT_CATEGORY_GRAPHICSSET,gs,0x69,&fog)) return 0;
    for (unsigned i=0;i<sizeof(fields);++i) { size_t n=0; const uint8_t *p=dm2_v1_asset_load_typed_sized(l,DM2_GDAT_CATEGORY_ENVIRONMENT,0,DM2_GDAT_ENTRY_TYPE_IMAGE,fields[i],&n); if(!p||!n)return 0; out->image_mask|=1u<<i; }
    out->valid=1;out->graphicsset=gs;out->fog_palette=fog;return 1;
}
int dm2_v1_weather_gdat_image_handle(const DM2_V1_AssetLoader *l, uint8_t field, DM2_V1_WeatherImageHandle *out) {
    size_t n=0; const uint8_t *p; if(!out)return 0; memset(out,0,sizeof(*out));
    if(field!=0x64&&field!=0x67&&field!=0x6a&&field!=0x6d&&field!=0x71)return 0;
    p=dm2_v1_asset_load_typed_sized(l,DM2_GDAT_CATEGORY_ENVIRONMENT,0,DM2_GDAT_ENTRY_TYPE_IMAGE,field,&n);
    if(!p||n==0||n>UINT32_MAX)return 0; out->valid=1;out->field=field;out->data=p;out->byte_count=(uint32_t)n;return 1;
}
int dm2_v1_weather_gdat_image_metadata(const DM2_V1_AssetLoader *l, uint8_t field, DM2_V1_WeatherImageMetadata *out) {
    uint8_t *pixels; int w=0,h=0; DM2_ImageFormat fmt;
    if(!out)return 0;memset(out,0,sizeof(*out));
    if(field!=0x64&&field!=0x67&&field!=0x6a&&field!=0x6d&&field!=0x71)return 0;
    pixels=dm2_v1_asset_load_image_field(l,DM2_GDAT_CATEGORY_ENVIRONMENT,0,field,&w,&h,&fmt);
    if(!pixels||w<=0||h<=0){dm2_v1_asset_free_pixels(pixels);return 0;}
    dm2_v1_asset_free_pixels(pixels);out->valid=1;out->field=field;out->width=w;out->height=h;out->format=fmt;return 1;
}
