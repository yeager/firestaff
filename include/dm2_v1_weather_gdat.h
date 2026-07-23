#ifndef DM2_V1_WEATHER_GDAT_H
#define DM2_V1_WEATHER_GDAT_H

#include <stddef.h>
#include <stdint.h>

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_graphics_data_open.h"
#include "dm2_v1_weather.h"

/* These are retained here while the weather receipt owns the historic
 * QUERY_TEMP_PICST contract. They are metadata identities only. */
#ifndef DM2_V1_GDAT_IMAGE_METADATA_DEFINED
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t bits_per_pixel;
    int16_t query_offset_x;
    int16_t query_offset_y;
    int graphicsset_offset_present;
    int image_offset_present;
    uint32_t metadata_hash;
} DM2_V1_GdatImageMetadata;
#define DM2_V1_GDAT_IMAGE_METADATA_DEFINED 1
#endif

#ifndef DM2_V1_WEATHER_RESTORED_STATE_RECEIPT_DEFINED
typedef struct {
    int valid;
    uint8_t weather;
    uint8_t intensity;
    uint16_t time_of_day;
    uint32_t weather_seed;
    uint32_t state_hash;
} DM2_V1_WeatherRestoredStateReceipt;
#define DM2_V1_WEATHER_RESTORED_STATE_RECEIPT_DEFINED 1
#endif

const uint8_t *dm2_v1_asset_load_text_sized(
    const DM2_V1_AssetLoader *loader, int category, int index, int field,
    size_t *out_size);
int dm2_v1_asset_load_image_metadata(
    const DM2_V1_AssetLoader *loader, int category, int index, int field,
    DM2_V1_GdatImageMetadata *out_metadata);
int dm2_v1_asset_load_image_local_palette(
    const DM2_V1_AssetLoader *loader, int category, int index, int field,
    uint8_t out_palette16[16], uint32_t *out_hash);
/* skproject QUERY_GDAT_SUMMARY_IMAGE installs a 256-entry identity translation
 * for non-4bpp images; this returns the deterministic hash of that table. */
uint32_t dm2_v1_weather_environment_identity_palette_hash(void);
int dm2_v1_weather_environment_asset_palette_fetch(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset, uint8_t command,
    uint8_t out_palette16[16], uint32_t *out_hash);

/* c_weather.cpp DM2_UPDATE_WEATHER slot commands: lightning bolt
 * 100+RAND16(3) (0x64..0x66), cloud 0x67..0x69, rain 0x6a..0x6c.  The real
 * DM2 GRAPHICS.DAT carries ENVIRONMENT dtText/dtImage for the whole
 * 0x64..0x6c range (bolt text is "cd6002" with no FW key; the source writes
 * the slot's cmFW byte with RANDDIR after a successful retrieve). */
#define DM2_V1_WEATHER_BOLT_CMD_BASE    0x64u
#define DM2_V1_WEATHER_BOLT_CMD_LAST    0x66u
#define DM2_V1_WEATHER_CLOUD_LIGHT_CMD  0x67u
#define DM2_V1_WEATHER_CLOUD_HEAVY_CMD  0x68u
#define DM2_V1_WEATHER_CLOUD_STORM_CMD  0x69u
#define DM2_V1_WEATHER_RAIN_LIGHT_CMD   0x6au
#define DM2_V1_WEATHER_RAIN_HEAVY_CMD   0x6bu
#define DM2_V1_WEATHER_RAIN_STORM_CMD   0x6cu
#define DM2_V1_WEATHER_COMMAND_COUNT    9u
#define DM2_V1_WEATHER_MAX_SLOTS        3u
#define DM2_V1_WEATHER_COMMAND_MASK(command_) \
    (1u << ((unsigned int)(command_) - DM2_V1_WEATHER_BOLT_CMD_BASE))

/* QUERY_GDAT_TEXT's decoded command buffer in
 * RETRIEVE_ENVIRONMENT_CMD_CD_FW is 128 bytes (SkWinCore.cpp 3DF7:075F
 * bp80[128]); longer command text cannot drive a source slot. */
#define DM2_V1_WEATHER_COMMAND_TEXT_MAX 128u

#define DM2_V1_DISTANT_ENVIRONMENT_BYTES 10u
typedef struct {
    int valid;
    uint8_t command;
    uint8_t slot_index;
    uint8_t raw[DM2_V1_DISTANT_ENVIRONMENT_BYTES];
    uint32_t raw_hash;
} DM2_V1_DistantEnvironmentReceipt;

typedef struct {
    int proven;
    uint32_t timer_hash;
    uint32_t distant_environment_hash;
    uint32_t transaction_hash;
} DM2_V1_WeatherTimerTransactionReceipt;

typedef struct {
    uint8_t command;
    const uint8_t *raw_text;
    uint32_t byte_count;
    uint32_t raw_hash;
    uint16_t text_raw_index;
    /* SkWinCore::QUERY_GDAT_TEXT (2636:0377): when the GDAT header word
     * dtWordValue(0,0,0) bit 3 is set, each text byte decodes as
     * (b ^ 0xFF) - i.  The CMDSTR parse below consumes this decoded form,
     * exactly like the source; empty when the GDAT text is not encoded. */
    uint8_t decoded_text[DM2_V1_WEATHER_COMMAND_TEXT_MAX];
    uint32_t decoded_text_size;
    uint32_t decoded_text_hash;
    int material_valid;
    uint16_t rect_number;
    uint8_t flip_mode;
    int image_present;
    uint8_t image_field;
    int query_metadata_valid;
    DM2_V1_GdatImageMetadata query_metadata;
    int local_palette_valid;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    int decoded_pixels_valid;
    const uint8_t *image_source_bytes;
    uint32_t image_source_byte_count;
    uint32_t image_raw_hash;
    uint16_t image_raw_index;
    uint32_t image_material_receipt_hash;
    uint16_t decoded_width;
    uint16_t decoded_height;
    DM2_ImageFormat decoded_format;
    uint32_t decoded_pixel_count;
    uint32_t decoded_pixels_hash;
    uint32_t material_hash;
    /* SkWinCore::QUERY_GDAT_SUMMARY_IMAGE (0B36:0520) palette-translation
     * receipt.  QUERY_GDAT_IMAGE_LOCALPAL (3e74:521A, DM2_EXTENDED_MODE==1)
     * returns NULL whenever the realized image is not 4bpp, and the summary
     * then installs the 256-entry identity translation (ref->b58[i] = i,
     * ref->w56 = 256); each decoded pixel byte indexes the global screen
     * palette directly.  A 4bpp IMG3/U4 keeps its 16-entry local palette
     * (w56 = 16).  palette_translation_hash is the admitted translation's
     * identity: local_palette_hash for 16, the identity-table hash for
     * 256. */
    int global_palette_identity_valid;
    uint16_t palette_translation_count;
    uint32_t palette_translation_hash;
    uint32_t global_palette_identity_hash;
} DM2_V1_WeatherCommandReceipt;

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint16_t misty_map;
    uint32_t command_mask;
    uint32_t material_mask;
    uint32_t receipt_hash;
    DM2_V1_WeatherCommandReceipt commands[DM2_V1_WEATHER_COMMAND_COUNT];
} DM2_V1_WeatherGdatReceipt;

typedef struct {
    uint8_t command;
    uint8_t slot_index;
    uint16_t rect_number;
    uint8_t flip_mode;
    int16_t source_offset_x;
    int16_t source_offset_y;
    uint8_t source_scale_x;
    uint8_t source_scale_y;
    uint16_t image_width;
    uint16_t image_height;
    int16_t query_offset_x;
    int16_t query_offset_y;
    uint32_t material_hash;
} DM2_V1_WeatherGdatOverlayCommand;

typedef struct {
    int valid;
    uint8_t cloud_level;
    uint8_t rain_level;
    uint32_t required_mask;
    uint32_t material_mask;
    uint32_t plan_hash;
    unsigned int command_count;
    DM2_V1_WeatherGdatOverlayCommand commands[2];
} DM2_V1_WeatherOverlayPlan;

typedef struct {
    uint8_t direction;
    int16_t map_x;
    int16_t map_y;
    int16_t map_offset_x;
    int16_t map_offset_y;
    int16_t map_level;
    uint16_t scene_flags;
    uint16_t game_tick;
    uint8_t player_direction;
    int player_moving;
    int16_t movement_offset_x;
    int16_t movement_offset_y;
    int16_t moving_horizon_offset_y;
    int16_t moving_other_offset_y;
} DM2_V1_WeatherDrawContext;

typedef struct {
    int valid;
    uint8_t command;
    uint16_t rect_number;
    uint8_t image_field;
    uint8_t mirror_flip;
    uint8_t scale_x;
    uint8_t scale_y;
    int16_t draw_offset_x;
    int16_t draw_offset_y;
    int source_bounds_valid;
    int16_t source_left;
    int16_t source_top;
    int16_t source_right;
    int16_t source_bottom;
    /* QUERY_TEMP_PICST must consume the exact decoded ENVIRONMENT plane and
     * QUERY_GDAT_IMAGE_LOCALPAL result admitted with the command.  Retain
     * both identities so the final viewport fetch cannot substitute another
     * same-sized image or palette. */
    uint32_t decoded_pixels_hash;
    uint32_t decoded_pixel_count;
    uint32_t local_palette_hash;
    uint32_t material_hash;
    /* QUERY_GDAT_SUMMARY_IMAGE palette translation admitted with the
     * command: 16-entry local palette for 4bpp IMG3/U4, or the 256-entry
     * global-palette identity (b58[i] = i, w56 = 256) for 8bpp IMG9. */
    uint16_t palette_translation_count;
    uint32_t palette_translation_hash;
} DM2_V1_WeatherDrawPlan;

typedef struct {
    int valid;
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    uint32_t table_hash;
} DM2_V1_WeatherDestinationClip;

typedef struct {
    int valid;
    DM2_V1_WeatherRestoredStateReceipt restored_state;
    unsigned int command_count;
    uint32_t distant_environment_hash;
    uint32_t renderer_hash;
    DM2_V1_WeatherDrawPlan draws[DM2_V1_WEATHER_MAX_SLOTS];
    DM2_V1_WeatherDestinationClip clips[DM2_V1_WEATHER_MAX_SLOTS];
} DM2_V1_WeatherRendererReceipt;

/* Final M11 admission for a source-owned outdoor weather render.  This
 * records immutable GRAPHICS.DAT dtText/dtImage identities and the live
 * DM2_SET_TIMER_WEATHER owner; it owns no decoded buffer or fallback draw. */
typedef struct {
    int valid;
    uint8_t graphicsset;
    uint32_t timer_owner_hash;
    uint32_t weather_receipt_hash;
    uint32_t renderer_hash;
    uint32_t raw_material_hash;
    uint32_t raw_material_byte_count;
    uint32_t command_mask;
    unsigned int command_count;
    uint32_t receipt_hash;
} DM2_V1_OutdoorWeatherM11Receipt;

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint32_t graphics_data_open_hash;
    uint32_t weather_receipt_hash;
    uint32_t command_mask;
    uint32_t material_mask;
    uint32_t command_text_hash;
    uint32_t renderer_hash;
    int source_text_ready;
    int material_ready;
    int renderer_ready;
    int palette_required;
    int blit_authorized;
    int no_fallback_blit;
    uint32_t admission_hash;
} DM2_V1_WeatherRuntimeAdmissionReceipt;

/* UPDATE_WEATHER selects these ENVIRONMENT fields.  They are source GDAT
 * addresses, not locally generated cloud or ground textures. */
#define DM2_V1_ENVIRONMENT_SKY_CLOUDS_MEDIUM 0x40u
#define DM2_V1_ENVIRONMENT_WET_GROUND_MEDIUM 0x80u

typedef struct {
    uint8_t environment_field;
    int32_t command_cd;
    uint8_t command_fw;
    uint32_t text_hash;
    uint32_t image_hash;
    uint32_t image_byte_count;
} DM2_V1_EnvironmentWeatherMaterialReceipt;

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint32_t map_load_token;
    unsigned int material_count;
    uint32_t receipt_hash;
    DM2_V1_EnvironmentWeatherMaterialReceipt materials[2];
} DM2_V1_EnvironmentWeatherReceipt;

typedef struct {
    int valid;
    uint8_t rain_intensity;
    uint16_t weather_turn;
    uint16_t party_turn;
    uint16_t turn_delta;
    uint8_t image_field;
    uint8_t mirror_phase;
    uint32_t receipt_hash;
} DM2_V1_RainfallParamReceipt;

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint32_t map_load_token;
    uint32_t scene_light_hash;
    uint32_t c_light_hash;
    uint32_t weather_receipt_hash;
    uint32_t weather_renderer_hash;
    uint32_t weather_admission_hash;
    uint32_t environment_receipt_hash;
    uint32_t rainfall_receipt_hash;
    uint32_t command_mask;
    uint32_t material_mask;
    unsigned int renderer_command_count;
    unsigned int environment_material_count;
    int distant_environment_display_bound;
    int draw_rain_bound;
    int no_synthetic_weather_fallback;
    uint32_t source_symbol_hash;
    uint32_t receipt_hash;
} DM2_V1_SceneWeatherLightRuntimeReceipt;

int dm2_v1_weather_gdat_receipt(const DM2_V1_AssetLoader *loader,
                                uint8_t graphicsset,
                                DM2_V1_WeatherGdatReceipt *out);
int dm2_v1_weather_gdat_command_receipt(const DM2_V1_AssetLoader *loader,
                                        uint8_t graphicsset,
                                        uint8_t command,
                                        DM2_V1_WeatherCommandReceipt *out);
int dm2_v1_weather_cmdstr_query(const uint8_t *text, size_t text_size,
                                 const char *name, int *out_found,
                                 int32_t *out_value);
uint8_t dm2_v1_weather_gdat_cloud_command_for_level(uint8_t level);
uint8_t dm2_v1_weather_gdat_rain_command_for_level(uint8_t level);

/* Builds the c_weather.cpp command sequence for the supplied live source
 * levels.  It never creates an image request from CD/FW alone: every selected
 * command must already carry a verified GDAT dtText receipt and material
 * fields.  `valid == 0` is therefore a no-draw result, not a fallback. */
int dm2_v1_weather_gdat_overlay_plan(
    const DM2_V1_WeatherGdatReceipt *receipt,
    uint8_t cloud_level,
    uint8_t rain_level,
    DM2_V1_WeatherOverlayPlan *out);

/* Mirrors skproject ENVIRONMENT_DRAW_DISTANT_ELEMENT (SKWIN/SkWinCore.cpp
 * 32CB:56BC).  It proves the command's real GDAT image/QUERY_TEMP_PICST
 * metadata before exposing a rectangle/transform plan. */
int dm2_v1_weather_gdat_draw_plan(
    const DM2_V1_WeatherCommandReceipt *command,
    const DM2_V1_WeatherDrawContext *context,
    DM2_V1_WeatherDrawPlan *out);

/* Realizes the exact live DME.h::DistantEnvironment register image used by
 * ENVIRONMENT_DRAW_DISTANT_ELEMENT. cmFW, cmCD, w4/w6, and b8/b9 must agree
 * with the selected GDAT command; disagreement is no-draw. */
int dm2_v1_weather_gdat_draw_plan_from_distant_environment(
    const DM2_V1_WeatherCommandReceipt *command,
    const DM2_V1_DistantEnvironmentReceipt *slot,
    const DM2_V1_WeatherDrawContext *context,
    DM2_V1_WeatherDrawPlan *out);

/* Bounded QUERY_BLIT_RECT route for a verified weather command. The supplied
 * bytes must be the original INTERFACE_GENERAL/0/dt04/0 table. Unsupported
 * compressed-rectangle forms are deliberately rejected. */
int dm2_v1_weather_gdat_destination_clip(
    const uint8_t *rect_table,
    size_t rect_table_size,
    const DM2_V1_WeatherCommandReceipt *command,
    DM2_V1_WeatherDestinationClip *out);

/* Binds already-restored runtime weather to the exact c_weather.cpp
 * DistantEnvironment command slots.  Each slot must name its own source
 * command byte, retain material provenance, and resolve through the original
 * dt04 rectangle table.  Missing or malformed slots leave no fallback draw. */
int dm2_v1_weather_gdat_renderer_receipt(
    const DM2_V1_WeatherRestoredStateReceipt *restored_state,
    const DM2_V1_WeatherGdatReceipt *weather,
    const DM2_V1_DistantEnvironmentReceipt *slots, unsigned int slot_count,
    const DM2_V1_WeatherDrawContext *context, const uint8_t *rect_table,
    size_t rect_table_size, DM2_V1_WeatherRendererReceipt *out);
int dm2_v1_weather_gdat_outdoor_m11_receipt(
    const DM2_V1_WeatherGdatReceipt *weather,
    const DM2_V1_WeatherRendererReceipt *renderer,
    const DM2_V1_SetTimerWeatherReceipt *timer_owner,
    DM2_V1_OutdoorWeatherM11Receipt *out);
int dm2_v1_weather_runtime_admission_receipt(
    const DM2_V1_GraphicsDataOpenReceipt *graphics_open,
    const DM2_V1_WeatherGdatReceipt *weather,
    const DM2_V1_WeatherRendererReceipt *renderer,
    DM2_V1_WeatherRuntimeAdmissionReceipt *out);
int dm2_v1_weather_distant_environment_receipt(
    const DM2_V1_WeatherGdatReceipt *weather, uint8_t command,
    uint8_t slot_index, const uint8_t raw[DM2_V1_DISTANT_ENVIRONMENT_BYTES],
    DM2_V1_DistantEnvironmentReceipt *out);
int dm2_v1_weather_timer_transaction_receipt(
    const DM2_V1_WeatherGdatReceipt *weather, const uint8_t *timer_bytes,
    size_t timer_size,
    const uint8_t distant_environment[DM2_V1_DISTANT_ENVIRONMENT_BYTES],
    DM2_V1_WeatherTimerTransactionReceipt *out);
int dm2_v1_weather_query_rainfall_param_receipt(
    uint8_t rain_intensity, uint16_t weather_turn, uint16_t party_turn,
    DM2_V1_RainfallParamReceipt *out);
int dm2_v1_scene_weather_light_runtime_receipt(
    const DM2_V1_GdatSceneLightM11Receipt *scene_light_receipt,
    const DM2_V1_CLightM11Receipt *c_light_receipt,
    const DM2_V1_WeatherGdatReceipt *weather,
    const DM2_V1_WeatherRendererReceipt *renderer,
    const DM2_V1_WeatherRuntimeAdmissionReceipt *admission,
    const DM2_V1_EnvironmentWeatherReceipt *environment,
    const DM2_V1_RainfallParamReceipt *rainfall,
    DM2_V1_SceneWeatherLightRuntimeReceipt *out);

/* Each requested environmental layer must retain both its QUERY_GDAT_TEXT
 * command data and matching dtImage entry. Missing selected data rejects the
 * full receipt; disabled weather remains an explicit, material-free state. */
int dm2_v1_weather_gdat_environment_receipt(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    uint32_t map_load_token, int outdoor_scene, int weather_enabled,
    uint8_t cloud_field, uint8_t wet_ground_field, int draw_clouds,
    int draw_wet_ground, DM2_V1_EnvironmentWeatherReceipt *out);

#endif
