#ifndef DM2_V1_WEATHER_GDAT_H
#define DM2_V1_WEATHER_GDAT_H

#include <stddef.h>
#include <stdint.h>

#include "dm2_v1_asset_loader.h"

/* skproject/SKULLWIN/c_weather.cpp DM2_UPDATE_WEATHER writes one of these
 * environment-command IDs and resolves it through QUERY_GDAT_TEXT(0x17,
 * MapGraphicsStyle(), id).  These are command text, not image fields. */
#define DM2_V1_WEATHER_CLOUD_LIGHT_CMD  0x67u
#define DM2_V1_WEATHER_CLOUD_HEAVY_CMD  0x68u
#define DM2_V1_WEATHER_CLOUD_STORM_CMD  0x69u
#define DM2_V1_WEATHER_RAIN_LIGHT_CMD   0x6au
#define DM2_V1_WEATHER_RAIN_HEAVY_CMD   0x6bu
#define DM2_V1_WEATHER_RAIN_STORM_CMD   0x6cu

#define DM2_V1_WEATHER_COMMAND_MASK(command_) \
    (1u << ((unsigned int)(command_) - DM2_V1_WEATHER_CLOUD_LIGHT_CMD))

typedef struct {
    uint8_t command;
    const uint8_t *raw_text;
    uint32_t byte_count;
    uint32_t raw_hash;
    /* c_bkgrnd.cpp RETRIEVE_ENVIRONMENT_CMD_CD_FW consumes these exact
     * CMDSTR fields before QUERY_TEMP_PICST.  A command is not drawable
     * until CD is present and non-zero in its original, NUL-terminated
     * dtText payload. */
    int material_valid;
    uint16_t rect_number;
    uint8_t flip_mode;
    uint32_t material_hash;
} DM2_V1_WeatherCommandReceipt;

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint16_t misty_map;
    uint32_t command_mask;
    uint32_t material_mask;
    uint32_t receipt_hash;
    DM2_V1_WeatherCommandReceipt commands[6];
} DM2_V1_WeatherGdatReceipt;

int dm2_v1_weather_gdat_receipt(const DM2_V1_AssetLoader *loader, uint8_t graphicsset, DM2_V1_WeatherGdatReceipt *out);
int dm2_v1_weather_gdat_command_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t graphicsset,
    uint8_t command,
    DM2_V1_WeatherCommandReceipt *out);

/* Exact bounded form of skproject DM2_QUERY_CMDSTR_TEXT.  The original
 * searches every occurrence, accepts an optional '=' and '-', and folds
 * decimal digits into the running result.  `out_found` distinguishes an
 * absent key from the source's legitimate numeric zero result. */
int dm2_v1_weather_cmdstr_query(const uint8_t *text, size_t text_size,
                                 const char *name, int *out_found,
                                 int32_t *out_value);

/* c_weather.cpp thresholds: cloud conditions use 0x10/0x40/0x80 and rain
 * conditions use 0x40/0x80/0xc0.  A value below the first threshold emits
 * no command.  This only binds source bytes; CMDSTR decoding is deliberately
 * outside this boundary. */
uint8_t dm2_v1_weather_gdat_cloud_command_for_level(uint8_t level);
uint8_t dm2_v1_weather_gdat_rain_command_for_level(uint8_t level);
#endif
