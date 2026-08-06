#ifndef THERON_V1_STARTUP_MEDIA_H
#define THERON_V1_STARTUP_MEDIA_H

#include "theron_v1_track02.h"
#include "theron_v1_startup_media_identity.h"
#include "theron_v1_world.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THERON_STARTUP_MEDIA_ROSTER_CAPACITY 8u
#define THERON_STARTUP_MEDIA_PROMPT_CAPACITY 40u

typedef struct {
    int track02_variant;
    char track02_md5[33];
    size_t track02_size;
    int startup_media_ready;
    int startup_bitmap_decode_status;
    int startup_bitmap_sample_count;
    unsigned int startup_bitmap_route_mask;
    size_t startup_bitmap_nonzero_pixel_count;
    uint32_t startup_bitmap_checksum;
    int startup_bitmap_title_route_ready;
    int startup_bitmap_stage_route_ready;
    int startup_bitmap_soul_room_route_ready;
    int startup_bitmap_forcefield_route_ready;
    int startup_bitmap_atlas_ready;
    int startup_bitmap_atlas_route_count;
    unsigned int startup_bitmap_atlas_route_mask;
    size_t startup_bitmap_atlas_tile_count;
    size_t startup_bitmap_atlas_nonzero_pixel_count;
    uint32_t startup_bitmap_atlas_checksum;
    Theron_Track02StartupBitmapAtlas startup_bitmap_atlas;
    Theron_RuntimeMediaIdentity runtime_media_identity;
    int startup_bitmap_title_sample_count;
    int startup_bitmap_stage_sample_count;
    int startup_bitmap_soul_room_sample_count;
    int startup_bitmap_forcefield_sample_count;
    size_t startup_bitmap_title_nonzero_pixel_count;
    size_t startup_bitmap_stage_nonzero_pixel_count;
    size_t startup_bitmap_soul_room_nonzero_pixel_count;
    size_t startup_bitmap_forcefield_nonzero_pixel_count;
    uint32_t startup_bitmap_title_checksum;
    uint32_t startup_bitmap_stage_checksum;
    uint32_t startup_bitmap_soul_room_checksum;
    uint32_t startup_bitmap_forcefield_checksum;
    size_t startup_bitmap_title_atlas_tile_count;
    size_t startup_bitmap_stage_atlas_tile_count;
    size_t startup_bitmap_soul_room_atlas_tile_count;
    size_t startup_bitmap_forcefield_atlas_tile_count;
    uint16_t startup_bitmap_title_atlas_width;
    uint16_t startup_bitmap_stage_atlas_width;
    uint16_t startup_bitmap_soul_room_atlas_width;
    uint16_t startup_bitmap_forcefield_atlas_width;
    size_t startup_bitmap_title_first_raw_offset;
    size_t startup_bitmap_title_last_raw_offset;
    size_t startup_bitmap_title_first_user_data_offset;
    size_t startup_bitmap_title_last_user_data_offset;
    size_t startup_bitmap_stage_first_raw_offset;
    size_t startup_bitmap_stage_last_raw_offset;
    size_t startup_bitmap_stage_first_user_data_offset;
    size_t startup_bitmap_stage_last_user_data_offset;
    size_t startup_bitmap_soul_room_first_raw_offset;
    size_t startup_bitmap_soul_room_last_raw_offset;
    size_t startup_bitmap_soul_room_first_user_data_offset;
    size_t startup_bitmap_soul_room_last_user_data_offset;
    size_t startup_bitmap_forcefield_first_raw_offset;
    size_t startup_bitmap_forcefield_last_raw_offset;
    size_t startup_bitmap_forcefield_first_user_data_offset;
    size_t startup_bitmap_forcefield_last_user_data_offset;
    unsigned int startup_bitmap_wide_route_mask;
    int startup_bitmap_wide_route_count;
    size_t startup_bitmap_wide_atlas_tile_count;
    unsigned int startup_bitmap_raw_route_mask;
    int startup_bitmap_raw_route_count;
    size_t startup_bitmap_raw_atlas_tile_count;
    unsigned int startup_bitmap_iso_route_mask;
    int startup_bitmap_iso_route_count;
    size_t startup_bitmap_iso_atlas_tile_count;
    int startup_roster_name_status;
    int startup_roster_name_count;
    char startup_roster_names[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                              [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY];
    char startup_roster_titles[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                               [THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY];
    int startup_text_prompt_status;
    int startup_text_prompt_count;
    char startup_text_prompt[THERON_STARTUP_MEDIA_PROMPT_CAPACITY];
    int startup_font_tiles_ready;
    Theron_Track02FontTileReceipt startup_font_tile_receipt;
} Theron_StartupMedia;

typedef struct {
    int track02_variant;
    char track02_md5[33];
    size_t track02_size;
    int startup_media_ready;
    int startup_bitmap_decode_status;
    int startup_bitmap_sample_count;
    unsigned int startup_bitmap_route_mask;
    size_t startup_bitmap_nonzero_pixel_count;
    uint32_t startup_bitmap_checksum;
    int startup_bitmap_title_route_ready;
    int startup_bitmap_stage_route_ready;
    int startup_bitmap_soul_room_route_ready;
    int startup_bitmap_forcefield_route_ready;
    int startup_bitmap_atlas_ready;
    int startup_bitmap_atlas_route_count;
    unsigned int startup_bitmap_atlas_route_mask;
    size_t startup_bitmap_atlas_tile_count;
    size_t startup_bitmap_atlas_nonzero_pixel_count;
    uint32_t startup_bitmap_atlas_checksum;
    Theron_Track02StartupBitmapAtlas startup_bitmap_atlas;
    Theron_RuntimeMediaIdentity runtime_media_identity;
    int startup_bitmap_title_sample_count;
    int startup_bitmap_stage_sample_count;
    int startup_bitmap_soul_room_sample_count;
    int startup_bitmap_forcefield_sample_count;
    size_t startup_bitmap_title_nonzero_pixel_count;
    size_t startup_bitmap_stage_nonzero_pixel_count;
    size_t startup_bitmap_soul_room_nonzero_pixel_count;
    size_t startup_bitmap_forcefield_nonzero_pixel_count;
    uint32_t startup_bitmap_title_checksum;
    uint32_t startup_bitmap_stage_checksum;
    uint32_t startup_bitmap_soul_room_checksum;
    uint32_t startup_bitmap_forcefield_checksum;
    size_t startup_bitmap_title_atlas_tile_count;
    size_t startup_bitmap_stage_atlas_tile_count;
    size_t startup_bitmap_soul_room_atlas_tile_count;
    size_t startup_bitmap_forcefield_atlas_tile_count;
    uint16_t startup_bitmap_title_atlas_width;
    uint16_t startup_bitmap_stage_atlas_width;
    uint16_t startup_bitmap_soul_room_atlas_width;
    uint16_t startup_bitmap_forcefield_atlas_width;
    size_t startup_bitmap_title_first_raw_offset;
    size_t startup_bitmap_title_last_raw_offset;
    size_t startup_bitmap_title_first_user_data_offset;
    size_t startup_bitmap_title_last_user_data_offset;
    size_t startup_bitmap_stage_first_raw_offset;
    size_t startup_bitmap_stage_last_raw_offset;
    size_t startup_bitmap_stage_first_user_data_offset;
    size_t startup_bitmap_stage_last_user_data_offset;
    size_t startup_bitmap_soul_room_first_raw_offset;
    size_t startup_bitmap_soul_room_last_raw_offset;
    size_t startup_bitmap_soul_room_first_user_data_offset;
    size_t startup_bitmap_soul_room_last_user_data_offset;
    size_t startup_bitmap_forcefield_first_raw_offset;
    size_t startup_bitmap_forcefield_last_raw_offset;
    size_t startup_bitmap_forcefield_first_user_data_offset;
    size_t startup_bitmap_forcefield_last_user_data_offset;
    unsigned int startup_bitmap_wide_route_mask;
    int startup_bitmap_wide_route_count;
    size_t startup_bitmap_wide_atlas_tile_count;
    unsigned int startup_bitmap_raw_route_mask;
    int startup_bitmap_raw_route_count;
    size_t startup_bitmap_raw_atlas_tile_count;
    unsigned int startup_bitmap_iso_route_mask;
    int startup_bitmap_iso_route_count;
    size_t startup_bitmap_iso_atlas_tile_count;
    int startup_roster_name_status;
    int startup_roster_name_count;
    char startup_roster_names[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                              [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY];
    char startup_roster_titles[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                               [THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY];
    int startup_text_prompt_status;
    int startup_text_prompt_count;
    char startup_text_prompt[THERON_STARTUP_MEDIA_PROMPT_CAPACITY];
    int startup_font_tiles_ready;
    Theron_Track02FontTileReceipt startup_font_tile_receipt;
} Theron_StartupMediaStateReceipt;

void theron_v1_startup_media_init(Theron_StartupMedia *media);
void theron_v1_startup_media_state_receipt_init(
    Theron_StartupMediaStateReceipt *receipt);

void theron_v1_startup_media_capture_track02(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_StartupMedia *out_media);

void theron_v1_startup_media_capture_track02_state_receipt(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_StartupMediaStateReceipt *out_receipt);

int theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
    const Theron_StartupMediaStateReceipt *receipt);

/* A source-backed indexed startup route.  `pixels` contains only the 4 bpp
 * indices extracted from the hash-recognised Track 02 atlas.  Track 02 has
 * no loader-bound title/stage/Soul Room/forcefield palette window yet, so a
 * successful raw receipt deliberately still has `palette_binding_verified`
 * and `rgba_output_allowed` clear.  (Restored after the df88dbda4 clobber.)
 */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    char track02_md5[33];
    unsigned int route_bit;
    size_t tile_count;
    uint16_t width;
    uint16_t height;
    size_t first_raw_offset;
    size_t last_raw_offset;
    size_t first_user_data_offset;
    size_t nonzero_pixel_count;
    uint32_t checksum;
    int raw_source_verified;
    int palette_binding_verified;
    int rgba_output_allowed;
    uint8_t palette_rgb8[16][3];
    uint8_t pixels[THERON_TRACK02_STARTUP_BITMAP_ATLAS_PIXELS];
} Theron_StartupRawBitmapRouteReceipt;

/* Consume one raw bitmap route from a complete Track 02 startup media state
 * receipt.  This is an indexed-byte consumer, not a palette guesser: it
 * never assigns an RGB value or a menu role beyond the already catalogued
 * route bit. */
int theron_v1_startup_media_consume_raw_bitmap_route(
    const Theron_StartupMediaStateReceipt *receipt,
    unsigned int route_bit,
    Theron_StartupRawBitmapRouteReceipt *out_receipt);

/* Bind only a complete, hash-profiled Track 02 media receipt to a world.
 * The operation clears the prior media cache before replacing it, so a stage
 * or forcefield transition cannot retain surfaces from another Track 02. */
int theron_v1_startup_media_bind_runtime_receipt(
    Theron_V1_World *world,
    const Theron_StartupMediaStateReceipt *receipt);

/* Candidate-only palette inspection entry point.  The hash-gated raw window
 * remains available for diagnostics, but no palette is promoted into runtime
 * media until a captured HuC6260 write, consumer and destination are bound.
 * Therefore this function currently returns 0 for authenticated candidates. */
int theron_v1_startup_media_bind_runtime_palette(
    Theron_V1_World *world,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_STARTUP_MEDIA_H */
