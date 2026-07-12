#ifndef DM2_V1_WEATHER_GDAT_H
#define DM2_V1_WEATHER_GDAT_H
#include <stdint.h>
#include "dm2_v1_asset_loader.h"
typedef struct { int valid; uint8_t graphicsset; uint16_t fog_palette; uint32_t image_mask; } DM2_V1_WeatherGdatReceipt;
int dm2_v1_weather_gdat_receipt(const DM2_V1_AssetLoader *loader, uint8_t graphicsset, DM2_V1_WeatherGdatReceipt *out);
#endif
