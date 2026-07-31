/*
 * dm2_v1_boot.c — DM2 V1 Boot Profile Implementation
 *
 * Phase 1: Runtime profile split for Skullkeep/DM2.
 * Separates DM2 boot from DM1/CSB, including:
 *   - Asset discovery by known DM2 hashes, with legacy filename fallback
 *   - Menu launch routing
 *   - Save namespace (saves/dm2/)
 *   - Platform/version diagnostics
 *   - Deterministic config
 *
 * Source-lock anchors:
 *   SKULL.ASM T560  — DUNGEON_Load: header parsing, dungeon_seed
 *   SKULL.ASM T000  — DM2 title screen / startup entry
 *   SKULL.ASM T800  — outdoor/shop/NPC entry points
 *   SKULL.ASM T520  — party placement and start position
 *   SKULL.ASM T048  — platform detection and version label
 *   SKULL.ASM T200  — save namespace resolution
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "dm2_v1_gdat_wall_m11_command.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_boot_startup_view_model.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_weather_gdat.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_creature_animation_gdat.h"
#include "dm2_v1_game.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_shop.h"
#include "dm2_v1_sound.h"
#include "dm2_v1_startup_menu.h"
#include "dm2_v1_startup_presentation.h"
#include "dm2_v1_viewport_renderer.h"
#include "asset_find_by_hash.h"
#include <stdio.h>
#include <string.h>

static uint32_t dm2_v1_boot_packaged_capture_hash_step(uint32_t hash,
                                                        uint32_t value);
static int dm2_v1_boot_viewport_asset_address(int gdat_index,
                                              int *out_category,
                                              int *out_index,
                                              int *out_field);
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define DM2_GDAT_MAP_GRAPHICSSET_BOOT_WALL 0x01
#define DM2_GDAT_WALL_FIELD_CACHE_LIMIT 0x40
#define DM2_GDAT_DOOR_FRAME_FIELD_CACHE_LIMIT 0x20
#define DM2_GDAT_DOOR_PANEL_FIELD_CACHE_LIMIT 0x04
#define DM2_GDAT_DOOR_BUTTON_FIELD_CACHE_LIMIT 0x08
#define DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT 8
#define DM2_GDAT_WALL_BUTTON_CACHE_LIMIT 8
#define DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT 16
#define DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT 8
#define DM2_GDAT_OBJECT_ICON_FIELD_LIMIT 0x10
#define DM2_GDAT_TITLE_MENU_SCREEN_FIELD 4
#define DM2_GDAT_SCENE_MATERIAL_CACHE_LIMIT 16

/* ── Embedded MD5 (same implementation as asset_find_by_hash.c) ──────── */

typedef struct {
    unsigned int state[4];
    unsigned int count[2];
    unsigned char buffer[64];
} DM2_Md5Ctx;

static void dm2_md5_init(DM2_Md5Ctx *ctx);
static void dm2_md5_update(DM2_Md5Ctx *ctx, const unsigned char *input, unsigned int len);
static void dm2_md5_final(DM2_Md5Ctx *ctx, char outHex[33]);

typedef struct {
    uint8_t *bytes;
    size_t size;
    DM2_V1_AssetLoader loader;
    uint8_t *scene_material_pixels[DM2_GDAT_SCENE_MATERIAL_CACHE_LIMIT][2];
    int scene_material_w[DM2_GDAT_SCENE_MATERIAL_CACHE_LIMIT][2];
    int scene_material_h[DM2_GDAT_SCENE_MATERIAL_CACHE_LIMIT][2];
    uint8_t *wall_pixels[DM2_GDAT_WALL_FIELD_CACHE_LIMIT];
    int wall_w[DM2_GDAT_WALL_FIELD_CACHE_LIMIT];
    int wall_h[DM2_GDAT_WALL_FIELD_CACHE_LIMIT];
    int wall_keys[DM2_GDAT_WALL_FIELD_CACHE_LIMIT];
    uint8_t *door_frame_pixels[DM2_GDAT_DOOR_FRAME_FIELD_CACHE_LIMIT];
    int door_frame_w[DM2_GDAT_DOOR_FRAME_FIELD_CACHE_LIMIT];
    int door_frame_h[DM2_GDAT_DOOR_FRAME_FIELD_CACHE_LIMIT];
    uint8_t *door_panel_pixels[DM2_GDAT_DOOR_PANEL_FIELD_CACHE_LIMIT];
    int door_panel_w[DM2_GDAT_DOOR_PANEL_FIELD_CACHE_LIMIT];
    int door_panel_h[DM2_GDAT_DOOR_PANEL_FIELD_CACHE_LIMIT];
    int door_panel_keys[DM2_GDAT_DOOR_PANEL_FIELD_CACHE_LIMIT];
    uint8_t *door_button_pixels[DM2_GDAT_DOOR_BUTTON_FIELD_CACHE_LIMIT];
    int door_button_w[DM2_GDAT_DOOR_BUTTON_FIELD_CACHE_LIMIT];
    int door_button_h[DM2_GDAT_DOOR_BUTTON_FIELD_CACHE_LIMIT];
    int door_overlay_keys[DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT];
    uint8_t *door_overlay_pixels[DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT];
    int door_overlay_w[DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT];
    int door_overlay_h[DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT];
    int wall_button_keys[DM2_GDAT_WALL_BUTTON_CACHE_LIMIT];
    uint8_t *wall_button_pixels[DM2_GDAT_WALL_BUTTON_CACHE_LIMIT];
    int wall_button_w[DM2_GDAT_WALL_BUTTON_CACHE_LIMIT];
    int wall_button_h[DM2_GDAT_WALL_BUTTON_CACHE_LIMIT];
    int creature_keys[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    uint8_t *creature_pixels[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    int creature_w[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    int creature_h[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    int item_keys[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    uint8_t *item_pixels[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    int item_w[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    int item_h[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    int projectile_keys[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    uint8_t *projectile_pixels[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    int projectile_w[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    int projectile_h[DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT];
    uint8_t *teleporter_pixels;
    int teleporter_w;
    int teleporter_h;
    uint8_t *floor_gfx_map_chip_pixels[0x100];
    int floor_gfx_map_chip_w[0x100];
    int floor_gfx_map_chip_h[0x100];
    uint8_t *wall_gfx_map_chip_pixels[0x100];
    int wall_gfx_map_chip_w[0x100];
    int wall_gfx_map_chip_h[0x100];
    uint8_t *door_map_chip_pixels[0x100];
    int door_map_chip_w[0x100];
    int door_map_chip_h[0x100];
    /* skproject c_bkgrnd.cpp ENVIRONMENT_DRAW_DISTANT_ELEMENT resolves
     * GRAPHICSSET-specific ENVIRONMENT image fields (0x64..0x6c) through
     * QUERY_TEMP_PICST. Cache the decoded pixels keyed by the packed gdat
     * index so real weather overlays can be fetched per frame. */
    int weather_environment_keys[DM2_V1_WEATHER_COMMAND_COUNT];
    uint8_t *weather_environment_pixels[DM2_V1_WEATHER_COMMAND_COUNT];
    int weather_environment_w[DM2_V1_WEATHER_COMMAND_COUNT];
    int weather_environment_h[DM2_V1_WEATHER_COMMAND_COUNT];
    uint8_t *hud_portrait_pixels[DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT];
    int hud_portrait_w[DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT];
    int hud_portrait_h[DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT];
    uint8_t *hud_core_pixels[DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK + 1];
    int hud_core_w[DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK + 1];
    int hud_core_h[DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK + 1];
    uint8_t *dialogue_box_pixels;
    int dialogue_box_w;
    int dialogue_box_h;
    uint8_t *startup_title_pixels;
    int startup_title_w;
    int startup_title_h;
    int ccm_program_count;
    int ccm_program_field;
} DM2_V1_BootGraphicsDat;

int dm2_v1_boot_dialogue_box_draw_plan(
    const DM2_V1_BootProfile *profile,
    DM2_V1_DialogueBoxDrawPlan *out)
{
    const DM2_V1_BootGraphicsDat *gfx;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !profile->graphics_dat) return 0;
    gfx = (const DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_dialogue_box_draw_plan(&gfx->loader, out);
}

int dm2_v1_boot_leader_hand_image_field(
    const DM2_V1_BootProfile *profile,
    int gdat_category,
    int gdat_index,
    uint32_t object_index,
    uint32_t game_tick,
    int party_direction,
    uint8_t *out_image_field)
{
    const DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_GdatImageMetadata metadata;
    uint8_t palette16[16];
    uint32_t palette_hash = 0u;
    uint16_t selector = 0u;
    uint8_t image_field = 0u;

    if (out_image_field) *out_image_field = 0u;
    if (!profile || !profile->graphics_dat || !out_image_field ||
        gdat_category < DM2_GDAT_CATEGORY_WEAPONS ||
        gdat_category > DM2_GDAT_CATEGORY_MISCELLANEOUS ||
        gdat_index < 0 || gdat_index > 0xff) {
        return 0;
    }
    gfx = (const DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    /* _2405_014a starts at image 0x18; an absent dtWordValue(6) leaves that
     * source default intact. */
    (void)dm2_v1_asset_load_word_value(&gfx->loader, gdat_category,
                                       gdat_index, 6, &selector);
    if (!dm2_v1_viewport_select_carried_item_image_field(
            selector, object_index, game_tick, party_direction,
            &image_field) ||
        !dm2_v1_asset_load_image_metadata(&gfx->loader, gdat_category,
                                          gdat_index, image_field,
                                          &metadata) ||
        metadata.bits_per_pixel != 4u ||
        !dm2_v1_asset_load_image_local_palette(&gfx->loader, gdat_category,
                                               gdat_index, image_field,
                                               palette16, &palette_hash) ||
        palette_hash == 0u) {
        return 0;
    }
    *out_image_field = image_field;
    return 1;
}

static int dm2_v1_boot_runtime_raw_gdat_hud_probe(
    DM2_V1_BootProfile *profile,
    int *out_portrait_count,
    uint32_t *out_portrait_hash,
    uint32_t *out_portrait_byte_count,
    uint32_t *out_core_hash,
    uint32_t *out_core_byte_count,
    int *out_interface_count);
static int dm2_v1_boot_startup_raw_gdat_hash(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint32_t seed,
    uint32_t *out_hash,
    uint32_t *out_count);
static int dm2_v1_boot_runtime_decoded_gdat_hash_add(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint32_t *io_hash,
    uint32_t *io_pixel_count);
static int dm2_v1_boot_runtime_decoded_gdat_hud_probe(
    DM2_V1_BootProfile *profile,
    int *out_portrait_count,
    uint32_t *out_portrait_hash,
    uint32_t *out_portrait_pixel_count,
    uint32_t *out_core_hash,
    uint32_t *out_core_pixel_count,
    int *out_interface_count);

static int dm2_v1_boot_runtime_interface_palette_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_irgb_byte_count,
    uint32_t *out_pal16_byte_count,
    uint32_t *out_irgb_color_count,
    uint32_t *out_pal16_color_count);
static int dm2_v1_boot_runtime_graphicsset_word_values_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    uint32_t *out_hash,
    uint32_t *out_present_mask,
    uint32_t *out_query_count,
    uint32_t *out_scene_flags,
    uint32_t *out_scene_colorkey,
    uint32_t *out_ambient_light,
    uint32_t *out_highest_light_level,
    uint32_t *out_void_random_fall,
    uint32_t *out_animated_floor,
    uint32_t *out_scene_rain,
    uint32_t *out_misty_map,
    uint32_t *out_thunder_position,
    uint32_t *out_ambient_darkness);
static int dm2_v1_boot_runtime_wall_gfx_image_offsets_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_query_count,
    uint32_t *out_nonzero_count,
    uint32_t *out_present_mask);
static int dm2_v1_boot_startup_menu_raw_screen_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_byte_count);
static int dm2_v1_boot_runtime_interface_rect14_placement_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_placement_count,
    uint32_t *out_rotated_cell_mask,
    uint32_t *out_max_stretched_size);

/* ── MD5 implementation (same as asset_find_by_hash.c) ─────────────── */

#define DM2_F(x,y,z) (((x)&(y))|((~(x))&(z)))
#define DM2_G(x,y,z) (((x)&(z))|((y)&(~(z))))
#define DM2_H(x,y,z) ((x)^(y)^(z))
#define DM2_I(x,y,z) ((y)^((x)|(~(z))))
#define DM2_ROT(x,n) (((x)<<(n))|((x)>>(32-(n))))

static const unsigned char dm2_md5_padding[64] = {0x80};

static void dm2_md5_init(DM2_Md5Ctx *ctx) {
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    ctx->count[0] = ctx->count[1] = 0;
}

static void dm2_md5_body(DM2_Md5Ctx *ctx, const unsigned char *data) {
    unsigned int a = ctx->state[0], b = ctx->state[1];
    unsigned int c = ctx->state[2], d = ctx->state[3];
    unsigned int X[16];
    int i;
    for (i = 0; i < 16; i++) {
        X[i] = (unsigned int)data[i*4] |
               ((unsigned int)data[i*4+1] << 8) |
               ((unsigned int)data[i*4+2] << 16) |
               ((unsigned int)data[i*4+3] << 24);
    }
    /* The pre-existing body had two bugs: (1) the trailing `+ a` on each
     * line was adding the wrong variable (it should be `+ b`, `+ c`, `+ d`,
     * `+ a` respectively for each step, not always the variable being
     * updated), and (2) the F/G/H/I argument order was wrong on 3 of 4
     * lines per cycle (it should always be the OTHER three variables in
     * cyclic order, not always starting with the same variable). Together
     * these produced wrong hashes for any input. The new body uses the
     * STEP macro form from asset_find_by_hash.c which is verified to match
     * the RFC 1321 reference test vectors. Round 1 (F function) */
    #define DM2_STEP(f,a,b,c,d,x,s,ac) { \
        (a) += f((b),(c),(d)) + (x) + (unsigned int)(ac); \
        (a) = DM2_ROT((a),(s)); (a) += (b); }
    DM2_STEP(DM2_F,a,b,c,d,X[ 0], 7,0xd76aa478) DM2_STEP(DM2_F,d,a,b,c,X[ 1],12,0xe8c7b756)
    DM2_STEP(DM2_F,c,d,a,b,X[ 2],17,0x242070db) DM2_STEP(DM2_F,b,c,d,a,X[ 3],22,0xc1bdceee)
    DM2_STEP(DM2_F,a,b,c,d,X[ 4], 7,0xf57c0faf) DM2_STEP(DM2_F,d,a,b,c,X[ 5],12,0x4787c62a)
    DM2_STEP(DM2_F,c,d,a,b,X[ 6],17,0xa8304613) DM2_STEP(DM2_F,b,c,d,a,X[ 7],22,0xfd469501)
    DM2_STEP(DM2_F,a,b,c,d,X[ 8], 7,0x698098d8) DM2_STEP(DM2_F,d,a,b,c,X[ 9],12,0x8b44f7af)
    DM2_STEP(DM2_F,c,d,a,b,X[10],17,0xffff5bb1) DM2_STEP(DM2_F,b,c,d,a,X[11],22,0x895cd7be)
    DM2_STEP(DM2_F,a,b,c,d,X[12], 7,0x6b901122) DM2_STEP(DM2_F,d,a,b,c,X[13],12,0xfd987193)
    DM2_STEP(DM2_F,c,d,a,b,X[14],17,0xa679438e) DM2_STEP(DM2_F,b,c,d,a,X[15],22,0x49b40821)
    /* Round 2 (G function) */
    DM2_STEP(DM2_G,a,b,c,d,X[ 1], 5,0xf61e2562) DM2_STEP(DM2_G,d,a,b,c,X[ 6], 9,0xc040b340)
    DM2_STEP(DM2_G,c,d,a,b,X[11],14,0x265e5a51) DM2_STEP(DM2_G,b,c,d,a,X[ 0],20,0xe9b6c7aa)
    DM2_STEP(DM2_G,a,b,c,d,X[ 5], 5,0xd62f105d) DM2_STEP(DM2_G,d,a,b,c,X[10], 9,0x02441453)
    DM2_STEP(DM2_G,c,d,a,b,X[15],14,0xd8a1e681) DM2_STEP(DM2_G,b,c,d,a,X[ 4],20,0xe7d3fbc8)
    DM2_STEP(DM2_G,a,b,c,d,X[ 9], 5,0x21e1cde6) DM2_STEP(DM2_G,d,a,b,c,X[14], 9,0xc33707d6)
    DM2_STEP(DM2_G,c,d,a,b,X[ 3],14,0xf4d50d87) DM2_STEP(DM2_G,b,c,d,a,X[ 8],20,0x455a14ed)
    DM2_STEP(DM2_G,a,b,c,d,X[13], 5,0xa9e3e905) DM2_STEP(DM2_G,d,a,b,c,X[ 2], 9,0xfcefa3f8)
    DM2_STEP(DM2_G,c,d,a,b,X[ 7],14,0x676f02d9) DM2_STEP(DM2_G,b,c,d,a,X[12],20,0x8d2a4c8a)
    /* Round 3 (H function) */
    DM2_STEP(DM2_H,a,b,c,d,X[ 5], 4,0xfffa3942) DM2_STEP(DM2_H,d,a,b,c,X[ 8],11,0x8771f681)
    DM2_STEP(DM2_H,c,d,a,b,X[11],16,0x6d9d6122) DM2_STEP(DM2_H,b,c,d,a,X[14],23,0xfde5380c)
    DM2_STEP(DM2_H,a,b,c,d,X[ 1], 4,0xa4beea44) DM2_STEP(DM2_H,d,a,b,c,X[ 4],11,0x4bdecfa9)
    DM2_STEP(DM2_H,c,d,a,b,X[ 7],16,0xf6bb4b60) DM2_STEP(DM2_H,b,c,d,a,X[10],23,0xbebfbc70)
    DM2_STEP(DM2_H,a,b,c,d,X[13], 4,0x289b7ec6) DM2_STEP(DM2_H,d,a,b,c,X[ 0],11,0xeaa127fa)
    DM2_STEP(DM2_H,c,d,a,b,X[ 3],16,0xd4ef3085) DM2_STEP(DM2_H,b,c,d,a,X[ 6],23,0x04881d05)
    DM2_STEP(DM2_H,a,b,c,d,X[ 9], 4,0xd9d4d039) DM2_STEP(DM2_H,d,a,b,c,X[12],11,0xe6db99e5)
    DM2_STEP(DM2_H,c,d,a,b,X[15],16,0x1fa27cf8) DM2_STEP(DM2_H,b,c,d,a,X[ 2],23,0xc4ac5665)
    /* Round 4 (I function) */
    DM2_STEP(DM2_I,a,b,c,d,X[ 0], 6,0xf4292244) DM2_STEP(DM2_I,d,a,b,c,X[ 7],10,0x432aff97)
    DM2_STEP(DM2_I,c,d,a,b,X[14],15,0xab9423a7) DM2_STEP(DM2_I,b,c,d,a,X[ 5],21,0xfc93a039)
    DM2_STEP(DM2_I,a,b,c,d,X[12], 6,0x655b59c3) DM2_STEP(DM2_I,d,a,b,c,X[ 3],10,0x8f0ccc92)
    DM2_STEP(DM2_I,c,d,a,b,X[10],15,0xffeff47d) DM2_STEP(DM2_I,b,c,d,a,X[ 1],21,0x85845dd1)
    DM2_STEP(DM2_I,a,b,c,d,X[ 8], 6,0x6fa87e4f) DM2_STEP(DM2_I,d,a,b,c,X[15],10,0xfe2ce6e0)
    DM2_STEP(DM2_I,c,d,a,b,X[ 6],15,0xa3014314) DM2_STEP(DM2_I,b,c,d,a,X[13],21,0x4e0811a1)
    DM2_STEP(DM2_I,a,b,c,d,X[ 4], 6,0xf7537e82) DM2_STEP(DM2_I,d,a,b,c,X[11],10,0xbd3af235)
    DM2_STEP(DM2_I,c,d,a,b,X[ 2],15,0x2ad7d2bb) DM2_STEP(DM2_I,b,c,d,a,X[ 9],21,0xeb86d391)
    #undef DM2_STEP
    ctx->state[0] += a; ctx->state[1] += b;
    ctx->state[2] += c; ctx->state[3] += d;
}

static void dm2_md5_update(DM2_Md5Ctx *ctx, const unsigned char *input, unsigned int len) {
    unsigned int idx = (ctx->count[0] >> 3) & 0x3F;
    unsigned int partLen = 64 - idx;
    ctx->count[0] += len << 3;
    if (ctx->count[0] < (len << 3)) ctx->count[1]++;
    ctx->count[1] += len >> 29;
    if (len >= partLen) {
        memcpy(ctx->buffer + idx, input, partLen);
        dm2_md5_body(ctx, ctx->buffer);
        for (unsigned int i = partLen; i + 63 < len; i += 64)
            dm2_md5_body(ctx, input + i);
        /* Tail copy: the pre-existing code did memcpy(ctx->buffer + 0,
         * input + 0, len - 0) which (a) re-hashed the head bytes AND
         * (b) overflowed ctx->buffer (64 bytes) whenever len > 64 —
         * i.e. always for DM2's GRAPHICS.DAT (~8.6 MB). That triggered
         * __stack_chk_fail (SIGABRT) on any DM2 launch. Tail must
         * skip past the partLen head + the 64-byte chunks walked by
         * the for-loop. Source: md5 RFC 1321 reference update. */
        idx = 0;
        unsigned int consumed = partLen + ((len - partLen) & ~0x3Fu);
        memcpy(ctx->buffer + idx, input + consumed, len - consumed);
    } else {
        /* len < partLen: head-fill only, no tail beyond idx. */
        memcpy(ctx->buffer + idx, input, len);
    }
}

static void dm2_md5_final(DM2_Md5Ctx *ctx, char outHex[33]) {
    unsigned int bits[2];
    bits[0] = ctx->count[0]; bits[1] = ctx->count[1];
    unsigned int idx = (ctx->count[0] >> 3) & 0x3F;
    unsigned int padLen = (idx < 56) ? (56 - idx) : (120 - idx);
    dm2_md5_update(ctx, dm2_md5_padding, padLen);
    dm2_md5_update(ctx, (const unsigned char *)bits, 8);
    int i;
    for (i = 0; i < 16; i++) {
        unsigned int v = ctx->state[i >> 2] >> ((i & 3) << 3);
        sprintf(outHex + i*2, "%02x", v & 0xff);
    }
    outHex[32] = '\0';
}

static void dm2_md5_bytes_hex(const uint8_t *bytes,
                              size_t size,
                              char out_hex[33]) {
    DM2_Md5Ctx ctx;
    dm2_md5_init(&ctx);
    if (bytes && size > 0u) {
        while (size > 0u) {
            unsigned int chunk = size > 0x40000000u
                ? 0x40000000u
                : (unsigned int)size;
            dm2_md5_update(&ctx, bytes, chunk);
            bytes += chunk;
            size -= chunk;
        }
    }
    dm2_md5_final(&ctx, out_hex);
}

static int dm2_v1_boot_read_asset_bytes(const char *path,
                                        long max_size,
                                        uint8_t **out_bytes,
                                        size_t *out_size) {
    char materialized[512];
    const char *read_path;
    FILE *f;
    long fsize;
    uint8_t *bytes;
    size_t got;
    int remove_materialized = 0;
    static unsigned int materialize_serial = 0u;

    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (!path || path[0] == '\0' || !out_bytes || !out_size ||
        max_size <= 0) {
        return 0;
    }
    read_path = path;
    if (strstr(path, "::") != NULL) {
        snprintf(materialized, sizeof(materialized),
                 "/tmp/firestaff-dm2-asset-%ld-%u.dat",
                 (long)getpid(), materialize_serial++);
        if (!asset_extract_virtual_path(path, materialized)) {
            return 0;
        }
        read_path = materialized;
        remove_materialized = 1;
    }
    f = fopen(read_path, "rb");
    if (!f) {
        if (remove_materialized) remove(materialized);
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0 ||
        (fsize = ftell(f)) <= 0 || fsize > max_size ||
        fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        if (remove_materialized) remove(materialized);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)fsize);
    if (!bytes) {
        fclose(f);
        if (remove_materialized) remove(materialized);
        return 0;
    }
    got = fread(bytes, 1, (size_t)fsize, f);
    fclose(f);
    if (remove_materialized) remove(materialized);
    if (got != (size_t)fsize) {
        free(bytes);
        return 0;
    }
    *out_bytes = bytes;
    *out_size = got;
    return 1;
}

static void dm2_v1_boot_graphics_free(DM2_V1_BootGraphicsDat *gfx) {
    if (!gfx) return;
    for (int i = 0; i < DM2_GDAT_SCENE_MATERIAL_CACHE_LIMIT; ++i) {
        for (int field = 0; field < 2; ++field) {
            dm2_v1_asset_free_pixels(gfx->scene_material_pixels[i][field]);
        }
    }
    for (int i = 0; i < DM2_GDAT_WALL_FIELD_CACHE_LIMIT; ++i) {
        dm2_v1_asset_free_pixels(gfx->wall_pixels[i]);
    }
    for (int i = 0; i < DM2_GDAT_DOOR_FRAME_FIELD_CACHE_LIMIT; ++i) {
        dm2_v1_asset_free_pixels(gfx->door_frame_pixels[i]);
    }
    for (int i = 0; i < DM2_GDAT_DOOR_PANEL_FIELD_CACHE_LIMIT; ++i) {
        dm2_v1_asset_free_pixels(gfx->door_panel_pixels[i]);
    }
    for (int i = 0; i < DM2_GDAT_DOOR_BUTTON_FIELD_CACHE_LIMIT; ++i) {
        dm2_v1_asset_free_pixels(gfx->door_button_pixels[i]);
    }
    for (int i = 0; i < DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT; ++i) {
        dm2_v1_asset_free_pixels(gfx->door_overlay_pixels[i]);
    }
    for (int i = 0; i < DM2_GDAT_WALL_BUTTON_CACHE_LIMIT; ++i) {
        dm2_v1_asset_free_pixels(gfx->wall_button_pixels[i]);
    }
    for (int i = 0; i < DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT; ++i) {
        dm2_v1_asset_free_pixels(gfx->creature_pixels[i]);
        dm2_v1_asset_free_pixels(gfx->item_pixels[i]);
        dm2_v1_asset_free_pixels(gfx->projectile_pixels[i]);
    }
    dm2_v1_asset_free_pixels(gfx->teleporter_pixels);
    for (int i = 0; i < 0x100; ++i) {
        dm2_v1_asset_free_pixels(gfx->floor_gfx_map_chip_pixels[i]);
        dm2_v1_asset_free_pixels(gfx->wall_gfx_map_chip_pixels[i]);
        dm2_v1_asset_free_pixels(gfx->door_map_chip_pixels[i]);
    }
    for (int i = 0; i < (int)DM2_V1_WEATHER_COMMAND_COUNT; ++i) {
        dm2_v1_asset_free_pixels(gfx->weather_environment_pixels[i]);
    }
    for (int i = 0; i < DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT; ++i) {
        dm2_v1_asset_free_pixels(gfx->hud_portrait_pixels[i]);
    }
    for (int i = 0; i <= DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK; ++i) {
        dm2_v1_asset_free_pixels(gfx->hud_core_pixels[i]);
    }
    dm2_v1_asset_free_pixels(gfx->dialogue_box_pixels);
    dm2_v1_asset_free_pixels(gfx->startup_title_pixels);
    dm2_v1_asset_loader_free(&gfx->loader);
    dm2_v1_creature_reset_ai_table();
    dm2_v1_creature_reset_ccm_programs();
    free(gfx->bytes);
    memset(gfx, 0, sizeof(*gfx));
    free(gfx);
}

static DM2_V1_BootGraphicsDat *dm2_v1_boot_graphics_load(
    const char *graphics_path) {
    uint8_t *bytes = NULL;
    size_t size = 0u;
    DM2_V1_BootGraphicsDat *gfx;

    if (!graphics_path || graphics_path[0] == '\0') return NULL;
    if (!dm2_v1_boot_read_asset_bytes(graphics_path,
                                      16L * 1024L * 1024L,
                                      &bytes,
                                      &size)) {
        return NULL;
    }
    gfx = (DM2_V1_BootGraphicsDat *)calloc(1, sizeof(*gfx));
    if (!gfx) {
        free(bytes);
        return NULL;
    }
    gfx->bytes = bytes;
    if (dm2_v1_asset_loader_init(&gfx->loader, gfx->bytes, size) != 0 ||
        !dm2_v1_asset_loader_verify(&gfx->loader) ||
        !dm2_v1_asset_loader_validate_typed_graph(&gfx->loader)) {
        dm2_v1_boot_graphics_free(gfx);
        return NULL;
    }
    (void)dm2_v1_creature_load_ai_table_from_gdat(&gfx->loader);
    gfx->ccm_program_count =
        dm2_v1_creature_load_ccm_programs_from_gdat_auto(
            &gfx->loader, &gfx->ccm_program_field);
    gfx->size = size;
    return gfx;
}

/* ── Known DM2 hashes ──────────────────────────────────────────────── */

/*
 * DM2 PC English:
 *   GRAPHICS.DAT  (8.6 MB)  — MD5: 25247ede4dabb6a71e5dabdfbcd5907d
 *   DUNGEON.DAT   (39 KB)   — MD5: 6caccd7875009e82fe2e28e7f6d6adc0
 *
 * DM2 PC French:
 *   GRAPHICS.DAT             — MD5: b4d733576ea60c41737f79f212faf528
 *   (dungeon same as PC EN)   — MD5: 6caccd7875009e82fe2e28e7f6d6adc0
 *
 * DM2 PC German/English JewelCase:
 *   GRAPHICS.DAT             — MD5: e52ab5e01715042b16a4dcff02052e5d
 *   (dungeon same as PC EN)   — MD5: 6caccd7875009e82fe2e28e7f6d6adc0
 */
static const char *const g_dm2_graphics_hashes[] = {
    "25247ede4dabb6a71e5dabdfbcd5907d",  /* PC English */
    "b4d733576ea60c41737f79f212faf528",  /* PC French */
    "e52ab5e01715042b16a4dcff02052e5d",  /* PC German/English JewelCase */
    NULL
};

static const char *const g_dm2_dungeon_hashes [] = {
    "6caccd7875009e82fe2e28e7f6d6adc0",  /* PC English + all variants */
    NULL
};

/* PC SONGLIST.DAT, 63 bytes. SHA-256:
 * 401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72
 * The MD5 is used solely for the existing recursive asset-discovery API. */
static const char *const g_dm2_songlist_hashes[] = {
    "bd11f8ded337c4aea978d1304b91b8eb",
    NULL
};

/* ── Platform label table ────────────────────────────────────────────── */

static const char *const g_platform_labels[DM2_PLATFORM_COUNT] = {
    [DM2_PLATFORM_PC_EN]    = "PC English",
    [DM2_PLATFORM_PC_FR]    = "PC French",
    [DM2_PLATFORM_PC_JEWEL] = "PC German/English JewelCase",
};

/* ── Path separator ──────────────────────────────────────────────────── */

#if defined(_WIN32)
#define DM2_PATH_SEP '\\'
#else
#define DM2_PATH_SEP '/'
#endif

/* ── MD5 string comparison (case-insensitive) ─────────────────────── */

static int md5_matches(const char *found_hex, const char *expected_hex) {
    size_t i;
    if (!found_hex || !expected_hex) return 0;
    if (strlen(found_hex) != 32 || strlen(expected_hex) != 32) return 0;
    for (i = 0; i < 32; i++) {
        char a = found_hex[i];
        char b = expected_hex[i];
        if (a >= 'A' && a <= 'F') a = a - 'A' + 'a';
        if (b >= 'A' && b <= 'F') b = b - 'A' + 'a';
        if (a != b) return 0;
    }
    return 1;
}

/* ── File size helper ─────────────────────────────────────────────────── */

static size_t file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return (size_t)st.st_size;
}

/* ── MD5 hash string from path ───────────────────────────────────────── */

static int path_md5_hex(const char *path, char out_hex[33]) {
    uint8_t *bytes = NULL;
    size_t size = 0u;
    DM2_Md5Ctx ctx;
    FILE *f = fopen(path, "rb");
    if (!f) {
        if (!dm2_v1_boot_read_asset_bytes(path,
                                          32L * 1024L * 1024L,
                                          &bytes,
                                          &size)) {
            return 0;
        }
        dm2_md5_bytes_hex(bytes, size, out_hex);
        free(bytes);
        return 1;
    }
    dm2_md5_init(&ctx);
    unsigned char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        dm2_md5_update(&ctx, buf, (unsigned int)n);
    }
    fclose(f);
    dm2_md5_final(&ctx, out_hex);
    return 1;
}

static int dm2_try_hash_scan_root(const char *root,
                                  char graphics_path[512],
                                  size_t *graphics_size,
                                  char graphics_md5[33],
                                  char dungeon_path[512],
                                  size_t *dungeon_size,
                                  char dungeon_md5[33]) {
    const char *hashes[8];
    char paths[8][ASSET_PATH_MAX];
    int matched[8];
    int hash_count = 0;
    int graphics_index = -1;
    int dungeon_index = -1;
    int i;

    if (!root || !root[0]) return 0;
    for (i = 0; g_dm2_graphics_hashes[i] && hash_count < 7; ++i) {
        hashes[hash_count++] = g_dm2_graphics_hashes[i];
    }
    dungeon_index = hash_count;
    for (i = 0; g_dm2_dungeon_hashes[i] && hash_count < 7; ++i) {
        hashes[hash_count++] = g_dm2_dungeon_hashes[i];
    }
    hashes[hash_count] = NULL;
    memset(paths, 0, sizeof(paths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list(root, hashes, paths, matched,
                                   hash_count, 8) <= 0) {
        return 0;
    }
    for (i = 0; g_dm2_graphics_hashes[i]; ++i) {
        if (matched[i]) {
            graphics_index = i;
            break;
        }
    }
    if (graphics_index >= 0 && !graphics_path[0]) {
        strncpy(graphics_path, paths[graphics_index], 511);
        graphics_path[511] = '\0';
        if (graphics_size) *graphics_size = file_size(graphics_path);
        strncpy(graphics_md5, hashes[graphics_index], 32);
        graphics_md5[32] = '\0';
    }
    if (dungeon_index >= 0 && matched[dungeon_index] && !dungeon_path[0]) {
        strncpy(dungeon_path, paths[dungeon_index], 511);
        dungeon_path[511] = '\0';
        if (dungeon_size) *dungeon_size = file_size(dungeon_path);
        strncpy(dungeon_md5, hashes[dungeon_index], 32);
        dungeon_md5[32] = '\0';
    }
    return graphics_path[0] && dungeon_path[0];
}

static int dm2_scan_known_hash_assets(const char *base,
                                      char graphics_path[512],
                                      size_t *graphics_size,
                                      char graphics_md5[33],
                                      char dungeon_path[512],
                                      size_t *dungeon_size,
                                      char dungeon_md5[33]) {
    char subroot[512];
    const char *subdirs[] = {"dm2", "data", NULL};
    int i;

    if (!base || !base[0]) return 0;
    for (i = 0; subdirs[i]; ++i) {
        if (snprintf(subroot, sizeof(subroot), "%s%c%s",
                     base, DM2_PATH_SEP, subdirs[i]) >= (int)sizeof(subroot)) {
            continue;
        }
        if (dm2_try_hash_scan_root(subroot,
                                   graphics_path, graphics_size, graphics_md5,
                                   dungeon_path, dungeon_size, dungeon_md5)) {
            return 1;
        }
    }
    return dm2_try_hash_scan_root(base,
                                  graphics_path, graphics_size, graphics_md5,
                                  dungeon_path, dungeon_size, dungeon_md5);
}

static void copy_parent_dir(char dst[512], const char *path) {
    const char *slash;
    const char *backslash;
    size_t n;
    if (!dst) return;
    dst[0] = '\0';
    if (!path || !path[0]) {
        snprintf(dst, 512, ".");
        return;
    }
    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    if (!slash || (backslash && backslash > slash)) {
        slash = backslash;
    }
    if (!slash) {
        snprintf(dst, 512, ".");
        return;
    }
    n = (size_t)(slash - path);
    if (n >= 512) n = 511;
    memcpy(dst, path, n);
    dst[n] = '\0';
}

/* ── Scan and verify DM2 assets ─────────────────────────────────────── */

int dm2_v1_boot_scan_assets(DM2_V1_BootProfile *profile,
                            const char *data_dir) {
    char path[512];
    (void)path;
    const char *base = data_dir ? data_dir : ".";

    if (!profile) return -1;
    profile->graphics_path[0] = '\0';
    profile->dungeon_path[0] = '\0';
    profile->songlist_path[0] = '\0';
    profile->graphics_md5[0] = '\0';
    profile->dungeon_md5[0] = '\0';
    profile->songlist_md5[0] = '\0';
    profile->graphics_size = 0U;
    profile->dungeon_size = 0U;
    profile->songlist_size = 0U;
    memset(profile->songlist_map, 0, sizeof(profile->songlist_map));
    profile->songlist_verified = 0;
    profile->assets_verified = 0;
    profile->use_dm2_filenames = 0;

    /* Source-lock: SKULL.ASM T560 owns the DM2 data load. Firestaff
     * discovers user-supplied files by hash first so launch does not
     * depend on PC install names or directory layout. */
    (void)dm2_scan_known_hash_assets(base,
                                     profile->graphics_path,
                                     &profile->graphics_size,
                                     profile->graphics_md5,
                                     profile->dungeon_path,
                                     &profile->dungeon_size,
                                     profile->dungeon_md5);

    /* Determine if using DM2-specific filenames */
    profile->use_dm2_filenames =
        (strstr(profile->graphics_path, "DM2GRAPHICS") != NULL ||
         strstr(profile->graphics_path, "dm2graphics") != NULL ||
         strstr(profile->dungeon_path, "DM2DUNGEON") != NULL ||
         strstr(profile->dungeon_path, "dm2dungeon") != NULL) ? 1 : 0;

    /* Verify against known hashes */
    profile->assets_verified = 0;
    if (profile->graphics_md5[0] && profile->dungeon_md5[0]) {
        size_t i;
        for (i = 0; g_dm2_graphics_hashes[i]; i++) {
            if (md5_matches(profile->graphics_md5, g_dm2_graphics_hashes[i])) {
                size_t j;
                for (j = 0; g_dm2_dungeon_hashes[j]; j++) {
                    if (md5_matches(profile->dungeon_md5, g_dm2_dungeon_hashes[j])) {
                        profile->assets_verified = 1;
                        break;
                    }
                }
                break;
            }
        }
    }

    /* Detect platform */
    profile->platform = DM2_PLATFORM_PC_EN;
    if (profile->graphics_md5[0]) {
        if (md5_matches(profile->graphics_md5, "b4d733576ea60c41737f79f212faf528")) {
            profile->platform = DM2_PLATFORM_PC_FR;
        } else if (md5_matches(profile->graphics_md5, "e52ab5e01715042b16a4dcff02052e5d")) {
            profile->platform = DM2_PLATFORM_PC_JEWEL;
        }
    }
    strncpy(profile->platform_label,
            g_platform_labels[profile->platform],
            sizeof(profile->platform_label) - 1);
    profile->platform_label[sizeof(profile->platform_label) - 1] = '\0';

    /* Set version id */
    switch (profile->platform) {
        case DM2_PLATFORM_PC_EN:    strncpy(profile->version_id, "pc-en",   sizeof(profile->version_id) - 1); break;
        case DM2_PLATFORM_PC_FR:    strncpy(profile->version_id, "pc-fr",   sizeof(profile->version_id) - 1); break;
        case DM2_PLATFORM_PC_JEWEL: strncpy(profile->version_id, "pc-jewel",sizeof(profile->version_id) - 1); break;
        default:                    strncpy(profile->version_id, "unknown", sizeof(profile->version_id) - 1); break;
    }

    /* Build asset root from the actual resolved file location.  The
     * canonical user staging can be either <root>/dm2/ or an extracted
     * DOS install where the real files live in <root>/data/.  SKULL.ASM
     * T560 owns the DUNGEON.DAT load; this helper only normalizes the
     * Firestaff launch path before that parser runs. */
    if (profile->dungeon_path[0]) {
        copy_parent_dir(profile->asset_root, profile->dungeon_path);
    } else if (profile->graphics_path[0]) {
        copy_parent_dir(profile->asset_root, profile->graphics_path);
    } else {
        snprintf(profile->asset_root, sizeof(profile->asset_root),
                 "%s%cdm2", base, DM2_PATH_SEP);
    }

    /* PC music routing is an optional extra for launch, but it must never
     * borrow a filename-matched or generated table.  c_sound.cpp consumes
     * SONGLIST.DAT's first 44 map selectors; leave routing unavailable when
     * the authentic 63-byte file is absent or cannot be hash-admitted. */
    if (profile->assets_verified &&
        asset_find_by_md5(profile->asset_root, g_dm2_songlist_hashes[0],
                          profile->songlist_path,
                          (int)sizeof(profile->songlist_path), 4) &&
        path_md5_hex(profile->songlist_path, profile->songlist_md5) &&
        md5_matches(profile->songlist_md5, g_dm2_songlist_hashes[0])) {
        uint8_t *songlist_bytes = NULL;
        size_t songlist_size = 0u;
        if (dm2_v1_boot_read_asset_bytes(profile->songlist_path, 63L,
                                         &songlist_bytes, &songlist_size) &&
            songlist_size == 63u) {
            size_t i;
            int valid = 1;
            for (i = 0u; i < sizeof(profile->songlist_map); ++i) {
                uint8_t track = songlist_bytes[i];
                if (track != 0xffu && track >= 29u) {
                    valid = 0;
                    break;
                }
                profile->songlist_map[i] = track;
            }
            if (valid) {
                profile->songlist_size = songlist_size;
                profile->songlist_verified = 1;
            } else {
                memset(profile->songlist_map, 0,
                       sizeof(profile->songlist_map));
                profile->songlist_path[0] = '\0';
                profile->songlist_md5[0] = '\0';
            }
            free(songlist_bytes);
        }
    }

    /* Determine if we found both required files */
    if (profile->graphics_path[0] && profile->dungeon_path[0]) {
        return 0;  /* success */
    }
    return -1;  /* missing assets */
}

int dm2_v1_boot_songlist_track_for_map(const DM2_V1_BootProfile *profile,
                                       int map_index, int *out_track) {
    uint8_t track;
    if (out_track) *out_track = -1;
    if (!profile || !profile->songlist_verified || map_index < 0 ||
        map_index >= (int)sizeof(profile->songlist_map)) return 0;
    track = profile->songlist_map[map_index];
    if (track == 0xffu || track >= 29u) return 0;
    if (out_track) *out_track = (int)track;
    return 1;
}

/* ── Init defaults ────────────────────────────────────────────────────── */

void dm2_v1_boot_profile_init(DM2_V1_BootProfile *profile) {
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));

    strncpy(profile->game_id, "dm2", sizeof(profile->game_id) - 1);
    profile->platform = DM2_PLATFORM_PC_EN;
    strncpy(profile->platform_label, "PC English", sizeof(profile->platform_label) - 1);
    strncpy(profile->version_id, "pc-en", sizeof(profile->version_id) - 1);

    /* Deterministic defaults: V1 tick rate (18.2 Hz = 18 + 2/10) */
    profile->deterministic.tick_rate_hz      = 18;
    profile->deterministic.tick_rate_hz_frac = 2;  /* 18.2 Hz */
    profile->deterministic.tick_ms           = 55;  /* ~55ms per tick */
    profile->deterministic.dungeon_move_speed = 0x0080;  /* Q8: 0.5 squares/tick */
    profile->deterministic.outdoor_move_speed = 0x0100;  /* Q8: 1.0 squares/tick */
    profile->deterministic.max_champions      = 4;
    profile->deterministic.max_party_members  = 5;
    profile->deterministic.day_cycle_minutes  = 1440;
    profile->deterministic.day_cycle_ticks    = (1440u * 60u * 18u) / (60u * 60u * 1000u / 1000u);
    profile->deterministic.max_levels         = 28;  /* PC English */
    profile->deterministic.dungeon_seed       = 257; /* default fallback */
}

/* ── Probe availability ───────────────────────────────────────────────── */

int dm2_v1_boot_probe_available(const char *data_dir) {
    char gfxPath[ASSET_PATH_MAX];
    char dunPath[ASSET_PATH_MAX];
    const char *base = data_dir ? data_dir : ".";
    gfxPath[0] = '\0';
    dunPath[0] = '\0';
    if (asset_find_by_md5_list(base, g_dm2_graphics_hashes, gfxPath,
                               (int)sizeof(gfxPath), NULL, 8) &&
        asset_find_by_md5_list(base, g_dm2_dungeon_hashes, dunPath,
                               (int)sizeof(dunPath), NULL, 8)) {
        return 1;
    }
    return 0;
}

/* ── Save root ───────────────────────────────────────────────────────── */

void dm2_v1_boot_set_save_root(DM2_V1_BootProfile *profile,
                                const char *save_dir) {
    if (!profile) return;
    if (save_dir && save_dir[0]) {
        strncpy(profile->save_root, save_dir, sizeof(profile->save_root) - 1);
    } else {
        /* Default: <data_dir>/../saves/dm2/ */
        snprintf(profile->save_root, sizeof(profile->save_root),
                 "%s%c..%csaves%cdm2",
                 profile->asset_root[0] ? profile->asset_root : ".",
                 DM2_PATH_SEP, DM2_PATH_SEP, DM2_PATH_SEP);
    }
}

/* ── Deterministic config from dungeon header ────────────────────────── */

/*
 * DM2 DUNGEON.DAT header (offset bytes):
 *   0-1:  0x0000 (reserved)
 *   2-3:  0x4731 ("G1" magic)
 *   4-5:  0x002c (44) first level data offset
 *   6-7:  level_count = 28 (PC English)
 *   8-9:  dungeon_seed = 257 (word at offset 8)
 *   10-11: metadata
 *
 * Source: SKULL.ASM T560 DUNGEON_Load
 */
static uint16_t dm2_rd16_le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

void dm2_v1_boot_build_deterministic_config(DM2_V1_BootProfile *profile,
                                            const uint8_t *dungeon_header,
                                            int dungeon_size) {
    if (!profile || !dungeon_header || dungeon_size < 12) return;

    /* Read dungeon seed from header offset 8 */
    uint16_t seed = dm2_rd16_le(dungeon_header + 8);
    profile->deterministic.dungeon_seed = seed;

    /* Read level count from header offset 6 */
    uint16_t level_count = dm2_rd16_le(dungeon_header + 6);
    if (level_count > 0 && level_count <= DM2_V1_MAX_LEVELS + 2) {
        profile->deterministic.max_levels = level_count;
    }

    /* Outdoor movement is 2x dungeon speed in DM2 */
    profile->deterministic.outdoor_move_speed = 0x0200;  /* Q8: 2.0 squares/tick */

    /* Derive day cycle tick rate from VBlank tick */
    /* DM2 day/night cycle: full rotation in day_cycle_minutes minutes.
     * Each minute = 60 seconds = 18.2 ticks ≈ 1092 ticks.
     * 1440 minutes = 1440 * 1092 ≈ 1,572,480 ticks. */
    profile->deterministic.day_cycle_ticks =
        (profile->deterministic.day_cycle_minutes * 60u * 18u) / 60u;
}

/* ── Enter game ──────────────────────────────────────────────────────── */

/*
 * dm2_v1_boot_enter_game — transition from boot to game state.
 *
 * Sets profile->dm2_state (DM2_V1_GameState*) and
 * profile->dungeon_data (DM2_V1_DungeonData*) from verified assets.
 *
 * On success (return 0), the caller can use profile->dm2_state
 * directly or cast to DM2_V1_GameState for game loop dispatch.
 *
 * Source: SKULL.ASM T520 — party placement after load
 *         SKULL.ASM T560 — dungeon load completion
 *         SKULL.ASM T200 — game state init after boot
 */

int dm2_v1_boot_enter_game(DM2_V1_BootProfile *profile) {
    if (!profile || !profile->assets_verified) return -1;

    /* Allocate DM2 game state */
    DM2_V1_GameState *gs = (DM2_V1_GameState *)
        calloc(1, sizeof(DM2_V1_GameState));
    if (!gs) return -1;

    /* Allocate dungeon data */
    DM2_V1_DungeonData *dd = (DM2_V1_DungeonData *)
        calloc(1, sizeof(DM2_V1_DungeonData));
    if (!dd) {
        free(gs);
        return -1;
    }

    /* Init game state with boot profile data_dir */
    dm2_v1_init(gs, profile->asset_root);

    /* Load dungeon */
    if (profile->dungeon_path[0]) {
        uint8_t *dat = NULL;
        size_t dat_size = 0u;
        if (dm2_v1_boot_read_asset_bytes(profile->dungeon_path,
                                         10L * 1024L * 1024L,
                                         &dat,
                                         &dat_size)) {
            size_t n = dat_size < 64u ? dat_size : 64u;
            /* Build deterministic config from header */
            if (n >= 12) {
                dm2_v1_boot_build_deterministic_config(
                    profile, dat, (int)n);
            }
            if (dm2_v1_dungeon_load(dd, dat, (int)dat_size) != 0) {
                /* The real game may only leave the title/menu after
                 * its GDAT startup surface is ready. A malformed map is
                 * fatal, but record-graph completeness is a later world
                 * capability, not boot admission. */
                dm2_v1_dungeon_free(dd);
                free(dat);
                free(dd);
                free(gs);
                return -1;
            }
            free(dat);
        }
    }

    if (profile->graphics_path[0] != '\0') {
        profile->graphics_dat =
            dm2_v1_boot_graphics_load(profile->graphics_path);
    }
    if (profile->graphics_dat) {
        DM2_V1_BootGraphicsDat *gfx =
            (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
        /* c_sound.cpp consumes music through the already hash-admitted GDAT
         * handle.  Bind the same owner used by the title/HUD/viewport rather
         * than accepting loose HMP filenames from the data directory. */
        dm2_v1_sound_bind_gdat_loader(&gfx->loader, profile->assets_verified);
    } else {
        dm2_v1_sound_bind_gdat_loader(NULL, 0);
    }

    /* skproject/SKWIN DME.h File_header stores the new-game party pose in
     * the G1 header. The dungeon loader has already admitted that pose
     * against map 0 dimensions and origin; do not replace it with the old
     * synthetic Hall-of-Champions default. */
    if (dd->initial_party_pose_valid) {
        gs->party_x = dd->initial_party_x;
        gs->party_y = dd->initial_party_y;
        gs->party_dir = dd->initial_party_dir & 3;
    }
    gs->current_level = 0;
    gs->outdoor = 0;

    profile->dm2_state = gs;
    profile->dungeon_data = dd;

    return 0;
}

int dm2_v1_boot_load_new_dungeon(
    DM2_V1_BootProfile *profile,
    DM2_V1_BootNewDungeonReceipt *out_receipt)
{
    size_t file_size = 0u;
    uint8_t *bytes = NULL;
    char current_md5[33];
    DM2_V1_DungeonData candidate;
    DM2_V1_DungeonData previous;
    DM2_V1_GameState *game;
    DM2_V1_BootNewDungeonReceipt receipt;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->assets_verified || !profile->dm2_state ||
        !profile->dungeon_data || profile->dungeon_path[0] == '\0') {
        return 0;
    }
    /* The title scan admits an original DUNGEON.DAT by hash. GAME_LOAD must
     * not silently consume a replacement that appeared after that scan. */
    if (profile->dungeon_md5[0] != '\0' &&
        (!path_md5_hex(profile->dungeon_path, current_md5) ||
         !md5_matches(current_md5, profile->dungeon_md5))) {
        return 0;
    }
    if (!dm2_v1_boot_read_asset_bytes(profile->dungeon_path,
                                      10L * 1024L * 1024L,
                                      &bytes,
                                      &file_size)) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    if (dm2_v1_dungeon_load(&candidate, bytes, (int)file_size) != 0 ||
        candidate.square_bytes != 1 || candidate.level_count <= 0 ||
        !candidate.initial_party_pose_valid) {
        dm2_v1_dungeon_free(&candidate);
        free(bytes);
        return 0;
    }
    for (size_t i = 0; i < file_size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, bytes[i]);
    }
    free(bytes);
    if (hash == 0u) {
        dm2_v1_dungeon_free(&candidate);
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.source_party_reset_required = 1;
    receipt.source_leader_hand_reset_required = 1;
    receipt.synthetic_party_created = 0;
    receipt.map_count = candidate.level_count;
    receipt.dungeon_seed = candidate.raw_data && candidate.raw_size >= 10
        ? (int)dm2_rd16_le(candidate.raw_data + 8) : -1;
    receipt.raw_byte_count = (uint32_t)candidate.raw_size;
    receipt.raw_hash = hash;
    if (receipt.raw_byte_count == 0u) {
        dm2_v1_dungeon_free(&candidate);
        return 0;
    }

    /* SKWINSPX skcore.cpp::GAME_LOAD reaches LOAD_NEW_DUNGEON before the
     * mirror-selection flow creates any champion. LOAD_NEW_DUNGEON still
     * replaces the source-owned G1 start pose: retaining an earlier world's
     * pose would make the eventual entrance synthetic. c_loadlevel.cpp's G1
     * header start word is therefore part of this atomic admission, not an
     * optional presentation hint. Party, hand, gold and timers remain
     * untouched. */
    game = (DM2_V1_GameState *)profile->dm2_state;
    game->party_x = candidate.initial_party_x;
    game->party_y = candidate.initial_party_y;
    game->party_dir = candidate.initial_party_dir & 3;
    game->current_level = 0;
    game->outdoor = 0;
    dm2_v1_boot_build_deterministic_config(
        profile, candidate.raw_data, candidate.raw_size);

    /* c_savegame.cpp reloads the structure before later GAME_LOAD party and
     * timer work. Swap only a complete candidate; a failed parse leaves the
     * active world untouched. */
    previous = *(DM2_V1_DungeonData *)profile->dungeon_data;
    *(DM2_V1_DungeonData *)profile->dungeon_data = candidate;
    memset(&candidate, 0, sizeof(candidate));
    dm2_v1_dungeon_free(&previous);
    receipt.reloaded = 1;
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

const char *dm2_v1_boot_startup_prepare_result_name(
    DM2_V1_BootStartupPrepareResult result) {
    switch (result) {
    case DM2_V1_BOOT_STARTUP_PREPARE_OK:
        return "OK";
    case DM2_V1_BOOT_STARTUP_PREPARE_BAD_INPUT:
        return "BAD_INPUT";
    case DM2_V1_BOOT_STARTUP_PREPARE_OOM:
        return "OOM";
    case DM2_V1_BOOT_STARTUP_PREPARE_SCAN_FAILED:
        return "SCAN_FAILED";
    case DM2_V1_BOOT_STARTUP_PREPARE_UNVERIFIED_ASSETS:
        return "UNVERIFIED_ASSETS";
    case DM2_V1_BOOT_STARTUP_PREPARE_ENTER_GAME_FAILED:
        return "ENTER_GAME_FAILED";
    case DM2_V1_BOOT_STARTUP_PREPARE_RUNTIME_BIND_FAILED:
        return "RUNTIME_BIND_FAILED";
    default:
        return "UNKNOWN";
    }
}

int dm2_v1_boot_startup_host_facts_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    DM2_V1_StartupHostFacts *out_facts)
{
    if (!out_facts) {
        return 0;
    }
    memset(out_facts, 0, sizeof(*out_facts));
    out_facts->startup_menu_active = startup_menu_active ? 1 : 0;
    out_facts->save_root = startup_save_root ? startup_save_root : "";
    out_facts->fallback_save_root = profile ? profile->save_root : NULL;
    out_facts->resume_available = resume_available ? 1 : 0;
    out_facts->slot_mask = slot_mask;
    out_facts->selected_row = selected_row;
    out_facts->scan_save_root = profile ? profile->save_root : NULL;
    return 1;
}

int dm2_v1_boot_startup_launch_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    DM2_V1_StartupLaunchReceipt *out_receipt)
{
    DM2_V1_StartupHostFacts facts;
    if (!out_receipt) {
        return 0;
    }
    dm2_v1_startup_launch_receipt_clear(out_receipt);
    if (!dm2_v1_boot_startup_host_facts_from_runtime_state(
            profile,
            startup_menu_active,
            startup_save_root,
            resume_available,
            slot_mask,
            selected_row,
            &facts)) {
        return 0;
    }
    return dm2_v1_startup_launch_from_host_facts_with_receipt(
        &facts,
        out_receipt);
}

int dm2_v1_boot_startup_launch_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_StartupLaunchReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return dm2_v1_boot_startup_launch_from_runtime_state(
        snapshot->profile,
        snapshot->startup_menu_active,
        snapshot->startup_save_root,
        snapshot->resume_available,
        snapshot->slot_mask,
        snapshot->selected_row,
        out_receipt);
}

int dm2_v1_boot_startup_launch_from_launch_snapshot(
    const DM2_V1_BootStartupLaunch *launch,
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_StartupLaunchReceipt *out_receipt)
{
    DM2_V1_BootRuntimeStartupSnapshot boot_snapshot;
    if (!launch || !launch->profile || !snapshot) {
        if (out_receipt) {
            dm2_v1_startup_launch_receipt_clear(out_receipt);
        }
        return 0;
    }
    boot_snapshot = *snapshot;
    boot_snapshot.profile = launch->profile;
    return dm2_v1_boot_startup_launch_from_snapshot(&boot_snapshot,
                                                    out_receipt);
}

int dm2_v1_boot_startup_launch_from_launch(
    const DM2_V1_BootStartupLaunch *launch,
    DM2_V1_StartupLaunchReceipt *out_receipt)
{
    if (!launch || !launch->profile) {
        if (out_receipt) {
            dm2_v1_startup_launch_receipt_clear(out_receipt);
        }
        return 0;
    }
    return dm2_v1_boot_startup_launch_from_runtime_state(
        launch->profile,
        1,
        NULL,
        0,
        0u,
        0,
        out_receipt);
}

int dm2_v1_boot_startup_advance_idle_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int mouth_redraw,
    DM2_V1_StartupIdleReceipt *out_receipt)
{
    DM2_V1_StartupHostFacts facts;
    if (!out_receipt) {
        return 0;
    }
    if (!dm2_v1_boot_startup_host_facts_from_runtime_state(
            profile,
            startup_menu_active,
            startup_save_root,
            resume_available,
            slot_mask,
            selected_row,
            &facts)) {
        return 0;
    }
    return dm2_v1_startup_advance_idle_from_host_facts_with_receipt(
        &facts,
        mouth_redraw,
        out_receipt);
}

int dm2_v1_boot_startup_advance_idle_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    int mouth_redraw,
    DM2_V1_StartupIdleReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return dm2_v1_boot_startup_advance_idle_from_runtime_state(
        snapshot->profile,
        snapshot->startup_menu_active,
        snapshot->startup_save_root,
        snapshot->resume_available,
        snapshot->slot_mask,
        snapshot->selected_row,
        mouth_redraw,
        out_receipt);
}

int dm2_v1_boot_startup_execute_firestaff_input_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    int (*apply_session)(void *userdata, const DM2_V1_SessionState *session),
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt)
{
    DM2_V1_StartupHostFacts facts;
    if (!out_receipt) {
        return 0;
    }
    if (!dm2_v1_boot_startup_host_facts_from_runtime_state(
            profile,
            startup_menu_active,
            startup_save_root,
            resume_available,
            slot_mask,
            selected_row,
            &facts)) {
        return 0;
    }
    return dm2_v1_startup_execute_firestaff_input_from_host_facts_with_receipt(
        &facts,
        menu_input,
        apply_session,
        apply_userdata,
        out_execution,
        out_receipt);
}

int dm2_v1_boot_startup_execute_firestaff_input_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    int menu_input,
    int (*apply_session)(void *userdata, const DM2_V1_SessionState *session),
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return dm2_v1_boot_startup_execute_firestaff_input_from_runtime_state(
        snapshot->profile,
        snapshot->startup_menu_active,
        snapshot->startup_save_root,
        snapshot->resume_available,
        snapshot->slot_mask,
        snapshot->selected_row,
        menu_input,
        apply_session,
        apply_userdata,
        out_execution,
        out_receipt);
}

int dm2_v1_boot_startup_execute_pointer_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int x,
    int y,
    int (*apply_session)(void *userdata, const DM2_V1_SessionState *session),
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt)
{
    DM2_V1_StartupHostFacts facts;
    if (!out_receipt) {
        return 0;
    }
    if (!dm2_v1_boot_startup_host_facts_from_runtime_state(
            profile,
            startup_menu_active,
            startup_save_root,
            resume_available,
            slot_mask,
            selected_row,
            &facts)) {
        return 0;
    }
    return dm2_v1_startup_execute_pointer_from_host_facts_with_receipt(
        &facts,
        x,
        y,
        apply_session,
        apply_userdata,
        out_execution,
        out_receipt);
}

int dm2_v1_boot_startup_execute_pointer_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    int (*apply_session)(void *userdata, const DM2_V1_SessionState *session),
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return dm2_v1_boot_startup_execute_pointer_from_runtime_state(
        snapshot->profile,
        snapshot->startup_menu_active,
        snapshot->startup_save_root,
        snapshot->resume_available,
        snapshot->slot_mask,
        snapshot->selected_row,
        x,
        y,
        apply_session,
        apply_userdata,
        out_execution,
        out_receipt);
}

static int dm2_v1_boot_startup_rect_contains(
    const DM2_V1_InterfaceRect *rect,
    int x,
    int y)
{
    return rect && rect->w > 0 && rect->h > 0 &&
           x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

static int dm2_v1_boot_expand_hud_rect(const uint8_t *raw, size_t raw_size,
                                       uint16_t rect_id,
                                       DM2_V1_InterfaceRect *out);
static int dm2_v1_boot_query_compressed_rect(const uint8_t *raw,
                                             size_t raw_size,
                                             uint16_t rect_id,
                                             DM2_V1_InterfaceRect *out);
static int dm2_v1_boot_blit_anchor(int mode, int x0, int y0, int width,
                                   int height,
                                   DM2_V1_InterfaceRect *out);

static int dm2_v1_boot_startup_menu_event_rect(
    const uint8_t *raw,
    size_t raw_size,
    uint16_t rect_id,
    DM2_V1_InterfaceRect *out)
{
    DM2_V1_InterfaceRect current;
    DM2_V1_InterfaceRect next;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* skproject c_xrect.cpp QUERY_RECT keeps title-menu event rectangles in
     * the same compressed INTERFACE_GENERAL/0/dt04/0 table as HUD blit
     * chains. For input events, prefer the event chain itself over the HUD
     * drawable expansion path: 0xD7 is an anchor-backed event, while 0xD9 is
     * already a direct hit box. */
    if (!dm2_v1_boot_query_compressed_rect(raw, raw_size, rect_id,
                                           &current)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    if (current.x > 0 && current.x <= 8 && current.y != 0 &&
        dm2_v1_boot_query_compressed_rect(raw, raw_size,
                                          (uint16_t)current.y, &next) &&
        next.x == 9 &&
        dm2_v1_boot_blit_anchor(current.x, current.w, current.h,
                                next.w, next.h, out)) {
        return 1;
    }
    *out = current;
    if (out->w <= 0 || out->h <= 0) {
        memset(out, 0, sizeof(*out));
        if (dm2_v1_boot_expand_hud_rect(raw, raw_size, rect_id, out) &&
            out->w > 0 && out->h > 0) {
            return 1;
        }
        memset(out, 0, sizeof(*out));
        return 0;
    }
    return 1;
}

int dm2_v1_boot_startup_execute_original_pointer_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int x,
    int y,
    int (*apply_session)(void *userdata, const DM2_V1_SessionState *session),
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt)
{
    DM2_V1_StartupHostFacts facts;
    DM2_V1_StartupMenuPointerHitReceipt pointer_hit;
    DM2_V1_StartupAction action;

    if (!out_receipt || !startup_menu_active ||
        !dm2_v1_boot_startup_host_facts_from_runtime_state(
            profile, startup_menu_active, startup_save_root, resume_available,
            slot_mask, selected_row, &facts) ||
        !dm2_v1_boot_startup_menu_pointer_hit(
            (DM2_V1_BootProfile *)profile, x, y, &pointer_hit)) {
        return 0;
    }

    /* skproject SkWinCore.cpp HANDLE_UI_EVENT:32001-32021 maps 0xD7 to NEW
     * and 0xD9 to RESUME. Both rectangles came from INTERFACE_GENERAL raw4;
     * use the admitted startup save scan for RESUME rather than synthetic row
     * geometry. */
    memset(&action, 0, sizeof(action));
    action.row = -1;
    action.slot = -1;
    if (pointer_hit.target == DM2_V1_STARTUP_POINTER_TARGET_NEW_GAME) {
        action.kind = DM2_V1_STARTUP_ACTION_NEW_GAME;
    } else if (pointer_hit.target == DM2_V1_STARTUP_POINTER_TARGET_RESUME_GAME &&
               facts.resume_available) {
        action.kind = DM2_V1_STARTUP_ACTION_CONTINUE;
    } else {
        return 0;
    }
    return dm2_v1_startup_execute_action_from_host_facts_with_receipt(
        &action, &facts, apply_session, apply_userdata, out_execution,
        out_receipt);
}

int dm2_v1_boot_startup_presentation_build_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    DM2_V1_StartupHostFacts facts;
    if (!out_commands || max_commands <= 0 || !startup_menu_active) {
        return 0;
    }
    if (!dm2_v1_boot_startup_host_facts_from_runtime_state(
            profile,
            startup_menu_active,
            startup_save_root,
            resume_available,
            slot_mask,
            selected_row,
            &facts)) {
        return 0;
    }
    return dm2_v1_startup_presentation_build_from_host_facts(
        &facts,
        out_commands,
        max_commands);
}

int dm2_v1_boot_startup_presentation_build_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!snapshot) {
        return 0;
    }
    return dm2_v1_boot_startup_presentation_build_from_runtime_state(
        snapshot->profile,
        snapshot->startup_menu_active,
        snapshot->startup_save_root,
        snapshot->resume_available,
        snapshot->slot_mask,
        snapshot->selected_row,
        out_commands,
        max_commands);
}

int dm2_v1_boot_startup_view_model_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    void *out_commands,
    int max_commands,
    int *out_command_count,
    DM2_V1_StartupViewReceipt *out_view_receipt,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    int command_count = 0;
    if (out_command_count) {
        *out_command_count = 0;
    }
    if (out_view_receipt) {
        memset(out_view_receipt, 0, sizeof(*out_view_receipt));
    }
    if (!snapshot) {
        return 0;
    }
    if (!snapshot->startup_menu_active) {
        if (out_view_receipt) {
            DM2_V1_StartupRuntimeHandoffReceipt *handoff =
                &out_view_receipt->runtime_handoff;
            memset(out_view_receipt, 0, sizeof(*out_view_receipt));
            out_view_receipt->valid = 1;
            out_view_receipt->render.valid = 1;
            out_view_receipt->render.hud_runtime_ready = 1;
            out_view_receipt->render.hud_overlay_suppressed = 0;
            (void)dm2_v1_startup_runtime_handoff_receipt_from_state(
                handoff,
                0,
                1);
        }
        if (out_command_count) {
            *out_command_count = 0;
        }
        (void)dm2_v1_boot_startup_presentation_receipt_from_snapshot(
            snapshot,
            out_phase,
            out_phase_size,
            out_startup_active,
            out_animation,
            out_animation_size,
            out_animation_active,
            out_title_frame,
            out_title_frame_max,
            out_title_ready);
        return 1;
    }
    if (out_commands && max_commands > 0) {
        if (out_view_receipt) {
            DM2_V1_StartupHostFacts facts;
            if (dm2_v1_boot_startup_host_facts_from_runtime_state(
                    snapshot->profile,
                    snapshot->startup_menu_active,
                    snapshot->startup_save_root,
                    snapshot->resume_available,
                    snapshot->slot_mask,
                    snapshot->selected_row,
                    &facts)) {
                (void)dm2_v1_startup_presentation_view_receipt_from_host_facts(
                    &facts,
                    1,
                    (DM2_V1_StartupDrawCommand *)out_commands,
                    max_commands,
                    out_view_receipt);
                command_count = out_view_receipt->command_count;
            }
        } else {
            command_count = dm2_v1_boot_startup_presentation_build_from_snapshot(
                snapshot,
                (DM2_V1_StartupDrawCommand *)out_commands,
                max_commands);
        }
        if (out_command_count) {
            *out_command_count = command_count;
        }
    }
    if (out_view_receipt && out_view_receipt->valid) {
        if (out_phase && out_phase_size > 0) {
            snprintf(out_phase,
                     (size_t)out_phase_size,
                     "%s",
                     out_view_receipt->runtime_handoff.animation);
        }
        if (out_startup_active) {
            *out_startup_active =
                out_view_receipt->runtime_handoff.startup_menu_active;
        }
        if (out_animation && out_animation_size > 0) {
            snprintf(out_animation,
                     (size_t)out_animation_size,
                     "%s",
                     out_view_receipt->runtime_handoff.animation);
        }
        if (out_animation_active) {
            *out_animation_active =
                out_view_receipt->runtime_handoff.animation_active;
        }
        if (out_title_frame) {
            *out_title_frame = out_view_receipt->runtime_handoff.title_frame;
        }
        if (out_title_frame_max) {
            *out_title_frame_max =
                out_view_receipt->runtime_handoff.title_frame_max;
        }
        if (out_title_ready) {
            *out_title_ready = out_view_receipt->runtime_handoff.title_ready;
        }
    } else {
        (void)dm2_v1_boot_startup_presentation_receipt_from_snapshot(
            snapshot,
            out_phase,
            out_phase_size,
            out_startup_active,
            out_animation,
            out_animation_size,
            out_animation_active,
            out_title_frame,
            out_title_frame_max,
            out_title_ready);
    }
    return 1;
}

void dm2_v1_boot_startup_view_model_clear(
    DM2_V1_BootStartupViewModel *out_view_model)
{
    if (!out_view_model) {
        return;
    }
    memset(out_view_model, 0, sizeof(*out_view_model));
}

static void dm2_v1_boot_startup_full_start_receipt_clear(
    DM2_V1_BootStartupFullStartReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

static void dm2_v1_boot_startup_host_view_receipt_clear(
    DM2_V1_BootStartupHostViewReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

static int dm2_v1_boot_startup_static_menu_timing_ready(
    const DM2_V1_BootStartupFullStartReceipt *receipt)
{
    return receipt &&
           receipt->title_animation_tick == 0 &&
           receipt->title_frame == 0 &&
           receipt->title_frame_max == 0 &&
           receipt->title_frame_duration_ticks == 0 &&
           receipt->title_cycle_ticks == 1 &&
           receipt->title_cycle_position_tick == 0 &&
           receipt->title_frame_start_tick == 0 &&
           receipt->title_next_frame_tick == 1 &&
           receipt->title_frame_elapsed_ticks == 0 &&
           receipt->title_frame_remaining_ticks == 1 &&
           receipt->title_cycle_remaining_ticks == 1;
}

static uint32_t dm2_v1_boot_packaged_capture_hash_step(uint32_t hash,
                                                       uint32_t value)
{
    hash ^= value + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    return hash ? hash : 1u;
}

/* SkWinCore::EXTENDED_LOAD_SPELLS_DEFINITION scans the 254 custom SPELL_DEF
 * slots, admits a slot only when rune 1 is nonzero, and then reads words
 * 2..7 plus dtText/0x18. Keep the raw GDAT result inside boot receipt flow;
 * no Firestaff-local spell table is allowed to stand in for these records. */
static int dm2_v1_boot_extended_spells_definition_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_ExtendedSpellsDefinitionReceipt *out_receipt)
{
    DM2_V1_BootGraphicsDat *gfx;
    uint32_t hash = 0x4553504cu;
    uint32_t count = 0u;
    uint32_t index;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!profile || !profile->graphics_dat || !out_receipt) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    for (index = 0u; index < 254u; ++index) {
        uint16_t words[7];
        const uint8_t *name;
        size_t name_size = 0u;
        uint32_t field;

        if (!dm2_v1_asset_load_word_value(
                &gfx->loader, DM2_GDAT_CATEGORY_SPELL_DEF,
                (int)index, 1, &words[0]) || words[0] == 0u) {
            continue;
        }
        for (field = 2u; field <= 7u; ++field) {
            if (!dm2_v1_asset_load_word_value(
                    &gfx->loader, DM2_GDAT_CATEGORY_SPELL_DEF,
                    (int)index, (int)field, &words[field - 1u])) {
                return 0;
            }
        }
        name = dm2_v1_asset_load_text_sized(
            &gfx->loader, DM2_GDAT_CATEGORY_SPELL_DEF, (int)index,
            0x18, &name_size);
        if (!name || name_size == 0u) {
            return 0;
        }
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, index);
        for (field = 0u; field < 7u; ++field) {
            hash = dm2_v1_boot_packaged_capture_hash_step(hash, words[field]);
        }
        for (field = 0u; field < name_size; ++field) {
            hash = dm2_v1_boot_packaged_capture_hash_step(hash, name[field]);
        }
        hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                       (uint32_t)name_size);
        ++count;
    }
    if (count == 0u) {
        return 1;
    }
    out_receipt->loaded = 1;
    out_receipt->spell_count = count;
    out_receipt->gdat_hash = hash;
    return 1;
}

void dm2_v1_boot_startup_packaged_capture_proof_init(
    DM2_V1_BootStartupPackagedCaptureProof *proof)
{
    if (proof) {
        memset(proof, 0, sizeof(*proof));
    }
}

static int dm2_v1_boot_startup_fill_full_start_receipt(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupViewModel *view_model)
{
    DM2_V1_BootStartupFullStartReceipt *receipt;
    const DM2_V1_StartupRuntimeHandoffReceipt *handoff;
    const DM2_V1_StartupRenderReceipt *render;
    int hud_raw_gdat_interface_count = 0;

    if (!snapshot || !view_model ||
        !view_model->view_receipt.valid ||
        !view_model->view_receipt.runtime_handoff.valid) {
        return 0;
    }
    receipt = &view_model->full_start_receipt;
    handoff = &view_model->view_receipt.runtime_handoff;
    render = &view_model->view_receipt.render;
    dm2_v1_boot_startup_full_start_receipt_clear(receipt);
    receipt->valid = 1;
    receipt->startup_menu_active = handoff->startup_menu_active;
    receipt->title_animation_tick = handoff->title_animation_tick;
    receipt->title_frame = handoff->title_frame;
    receipt->title_frame_max = handoff->title_frame_max;
    receipt->title_frame_duration_ticks =
        handoff->title_frame_duration_ticks;
    receipt->title_ready = handoff->title_ready;
    receipt->title_gdat_category = render->title_gdat_category;
    receipt->title_gdat_index = render->title_gdat_index;
    receipt->title_gdat_field = render->title_gdat_field;
    receipt->title_backdrop_ready = render->title_backdrop_ready;
    receipt->hud_overlay_suppressed = render->hud_overlay_suppressed;
    receipt->hud_runtime_ready = handoff->hud_runtime_ready;
    receipt->runtime_menu_ready = handoff->runtime_menu_ready;
    receipt->runtime_action_ready = handoff->runtime_action_ready;
    receipt->first_hud_frame_ready = handoff->first_hud_frame_ready;
    receipt->full_start_graphics_ready = render->full_start_graphics_ready;
    receipt->title_gdat_asset_ready = view_model->title_gdat_asset_ready;
    receipt->title_gdat_asset_w = view_model->title_gdat_asset_w;
    receipt->title_gdat_asset_h = view_model->title_gdat_asset_h;
    receipt->title_gdat_asset_stride = view_model->title_gdat_asset_stride;
    receipt->menu_gdat_category = render->menu_gdat_category;
    receipt->menu_gdat_index = render->menu_gdat_index;
    receipt->menu_gdat_field = render->menu_gdat_field;
    if (receipt->startup_menu_active &&
        receipt->title_frame_duration_ticks > 0 &&
        receipt->title_frame_max > 0) {
        int duration = receipt->title_frame_duration_ticks;
        receipt->title_cycle_ticks =
            duration * (receipt->title_frame_max + 1);
        receipt->title_cycle_position_tick =
            receipt->title_animation_tick % receipt->title_cycle_ticks;
        receipt->title_frame =
            receipt->title_cycle_position_tick / duration;
        receipt->title_frame_start_tick = receipt->title_frame * duration;
        receipt->title_next_frame_tick = receipt->title_frame_start_tick +
                                         duration;
        receipt->title_frame_elapsed_ticks =
            receipt->title_cycle_position_tick - receipt->title_frame_start_tick;
        receipt->title_frame_remaining_ticks =
            receipt->title_next_frame_tick -
            receipt->title_cycle_position_tick;
        receipt->title_cycle_remaining_ticks =
            receipt->title_cycle_ticks -
            receipt->title_cycle_position_tick;
    } else if (receipt->startup_menu_active) {
        receipt->title_cycle_ticks = 1;
        receipt->title_cycle_position_tick = 0;
        receipt->title_frame_start_tick = 0;
        receipt->title_next_frame_tick = 1;
        receipt->title_frame_elapsed_ticks = 0;
        receipt->title_frame_remaining_ticks = 1;
        receipt->title_cycle_remaining_ticks = 1;
    }
    receipt->exact_title_timing_ready =
        dm2_v1_boot_startup_static_menu_timing_ready(receipt);
    receipt->menu_row_count = render->row_count;
    receipt->menu_text_count = render->menu_text_count;
    receipt->selectable_text_count = render->selectable_text_count;
    receipt->selected_highlight_count = render->selected_highlight_count;
    receipt->menu_panel_ready = 1;
    receipt->startup_menu_assets_ready =
        receipt->title_backdrop_ready &&
        receipt->menu_gdat_field == DM2_GDAT_TITLE_MENU_SCREEN_FIELD;
    receipt->full_start_real_asset_ready =
        receipt->full_start_graphics_ready &&
        receipt->title_gdat_asset_ready &&
        receipt->startup_menu_assets_ready &&
        receipt->hud_overlay_suppressed &&
        receipt->hud_runtime_ready;
    if (snapshot->profile &&
        !dm2_v1_boot_extended_spells_definition_receipt(
            (DM2_V1_BootProfile *)snapshot->profile,
            &receipt->extended_spells)) {
        return 0;
    }
    if (snapshot && snapshot->profile &&
        dm2_v1_boot_gdat_raw_asset_proof(
            (DM2_V1_BootProfile *)snapshot->profile,
            receipt->title_gdat_category,
            receipt->title_gdat_index,
            receipt->title_gdat_field,
            0x32545257u,
            &receipt->title_raw_gdat_hash,
            &receipt->title_raw_gdat_byte_count) &&
        dm2_v1_boot_gdat_raw_asset_proof(
            (DM2_V1_BootProfile *)snapshot->profile,
            receipt->menu_gdat_category,
            receipt->menu_gdat_index,
            receipt->menu_gdat_field,
            0x324d5257u,
            &receipt->menu_raw_gdat_hash,
            &receipt->menu_raw_gdat_byte_count)) {
        receipt->title_menu_raw_gdat_capture_ready =
            receipt->title_raw_gdat_hash != 0u &&
            receipt->title_raw_gdat_byte_count > 0u &&
            receipt->menu_raw_gdat_hash != 0u &&
            receipt->menu_raw_gdat_byte_count > 0u;
    }
    if (snapshot && snapshot->profile) {
        uint32_t title_decoded_hash = 0x32544443u;
        uint32_t title_decoded_pixels = 0u;
        uint32_t menu_decoded_hash = 0x324d4443u;
        uint32_t menu_decoded_pixels = 0u;
        if (dm2_v1_boot_runtime_decoded_gdat_hash_add(
                (DM2_V1_BootProfile *)snapshot->profile,
                receipt->title_gdat_category,
                receipt->title_gdat_index,
                receipt->title_gdat_field,
                &title_decoded_hash,
                &title_decoded_pixels) &&
            dm2_v1_boot_runtime_decoded_gdat_hash_add(
                (DM2_V1_BootProfile *)snapshot->profile,
                receipt->menu_gdat_category,
                receipt->menu_gdat_index,
                receipt->menu_gdat_field,
                &menu_decoded_hash,
                &menu_decoded_pixels)) {
            receipt->title_decoded_gdat_hash = title_decoded_hash;
            receipt->title_decoded_gdat_pixel_count = title_decoded_pixels;
            receipt->menu_decoded_gdat_hash = menu_decoded_hash;
            receipt->menu_decoded_gdat_pixel_count = menu_decoded_pixels;
            receipt->title_menu_decoded_gdat_capture_ready =
                receipt->title_decoded_gdat_hash != 0u &&
                receipt->title_decoded_gdat_pixel_count == 320u * 200u &&
                receipt->menu_decoded_gdat_hash != 0u &&
                receipt->menu_decoded_gdat_pixel_count == 320u * 200u;
        }
    }
    if (snapshot && snapshot->profile) {
        if (dm2_v1_boot_runtime_raw_gdat_hud_probe(
                (DM2_V1_BootProfile *)snapshot->profile,
                &receipt->hud_raw_gdat_portrait_count,
                &receipt->hud_raw_gdat_portrait_hash,
                &receipt->hud_raw_gdat_portrait_byte_count,
                &receipt->hud_raw_gdat_core_hash,
                &receipt->hud_raw_gdat_core_byte_count,
                &hud_raw_gdat_interface_count)) {
            receipt->hud_raw_gdat_capture_ready = 1;
        }
    }
    if (receipt->hud_raw_gdat_capture_ready) {
        receipt->full_start_real_asset_ready =
            receipt->full_start_real_asset_ready &&
            receipt->title_menu_raw_gdat_capture_ready &&
            receipt->title_menu_decoded_gdat_capture_ready &&
            receipt->hud_raw_gdat_portrait_count >= 4 &&
            receipt->hud_raw_gdat_portrait_hash != 0u &&
            receipt->hud_raw_gdat_portrait_byte_count > 0u &&
            receipt->hud_raw_gdat_core_hash != 0u &&
            receipt->hud_raw_gdat_core_byte_count > 0u &&
            hud_raw_gdat_interface_count >= 3;
    }
    /* skproject/SKWIN title/menu startup keeps title timing, GDAT title art,
     * HUD suppression, and runtime handoff as one boot boundary. M11 can use
     * this receipt directly instead of combining command counts and flags. */
    (void)snapshot;
    return 1;
}

int dm2_v1_boot_startup_packaged_capture_proof_from_host_view(
    const DM2_V1_BootStartupHostViewReceipt *host_view,
    DM2_V1_BootStartupPackagedCaptureProof *out_proof)
{
    uint32_t hash = 0x324d3255u;
    const DM2_V1_BootStartupFullStartReceipt *full_start;

    dm2_v1_boot_startup_packaged_capture_proof_init(out_proof);
    if (!host_view || !out_proof || !host_view->valid) {
        return 0;
    }
    full_start = &host_view->full_start;
    out_proof->host_view_valid = host_view->valid;
    out_proof->full_start_valid = full_start->valid;
    out_proof->draw_startup_menu = host_view->draw_startup_menu;
    out_proof->command_count = host_view->command_count;
    out_proof->selected_row = host_view->selected_row;
    out_proof->title_animation_tick = host_view->title_animation_tick;
    out_proof->title_frame = host_view->title_frame;
    out_proof->title_frame_max = host_view->title_frame_max;
    out_proof->title_frame_duration_ticks =
        host_view->title_frame_duration_ticks;
    out_proof->title_ready = host_view->title_ready;
    out_proof->title_cycle_ticks = host_view->title_cycle_ticks;
    out_proof->title_cycle_position_tick =
        host_view->title_cycle_position_tick;
    out_proof->title_frame_start_tick = host_view->title_frame_start_tick;
    out_proof->title_next_frame_tick = host_view->title_next_frame_tick;
    out_proof->title_frame_elapsed_ticks =
        host_view->title_frame_elapsed_ticks;
    out_proof->title_frame_remaining_ticks =
        host_view->title_frame_remaining_ticks;
    out_proof->title_cycle_remaining_ticks =
        host_view->title_cycle_remaining_ticks;
    out_proof->exact_title_timing_ready =
        host_view->exact_title_timing_ready;
    out_proof->title_gdat_category = full_start->title_gdat_category;
    out_proof->title_gdat_index = full_start->title_gdat_index;
    out_proof->title_gdat_field = full_start->title_gdat_field;
    out_proof->title_gdat_asset_ready =
        host_view->title_gdat_asset_ready;
    out_proof->title_gdat_asset_w = host_view->title_gdat_asset_w;
    out_proof->title_gdat_asset_h = host_view->title_gdat_asset_h;
    out_proof->title_gdat_asset_stride =
        host_view->title_gdat_asset_stride;
    out_proof->menu_gdat_category = full_start->menu_gdat_category;
    out_proof->menu_gdat_index = full_start->menu_gdat_index;
    out_proof->menu_gdat_field = full_start->menu_gdat_field;
    out_proof->title_menu_raw_gdat_capture_ready =
        full_start->title_menu_raw_gdat_capture_ready;
    out_proof->title_raw_gdat_hash = full_start->title_raw_gdat_hash;
    out_proof->title_raw_gdat_byte_count =
        full_start->title_raw_gdat_byte_count;
    out_proof->menu_raw_gdat_hash = full_start->menu_raw_gdat_hash;
    out_proof->menu_raw_gdat_byte_count =
        full_start->menu_raw_gdat_byte_count;
    out_proof->title_menu_decoded_gdat_capture_ready =
        full_start->title_menu_decoded_gdat_capture_ready;
    out_proof->title_decoded_gdat_hash =
        full_start->title_decoded_gdat_hash;
    out_proof->title_decoded_gdat_pixel_count =
        full_start->title_decoded_gdat_pixel_count;
    out_proof->menu_decoded_gdat_hash =
        full_start->menu_decoded_gdat_hash;
    out_proof->menu_decoded_gdat_pixel_count =
        full_start->menu_decoded_gdat_pixel_count;
    out_proof->menu_row_count = host_view->menu_row_count;
    out_proof->menu_text_count = host_view->menu_text_count;
    out_proof->selectable_text_count = host_view->selectable_text_count;
    out_proof->selected_highlight_count =
        host_view->selected_highlight_count;
    out_proof->menu_panel_ready = host_view->menu_panel_ready;
    out_proof->startup_menu_assets_ready =
        host_view->startup_menu_assets_ready;
    out_proof->hud_overlay_suppressed = host_view->hud_overlay_suppressed;
    out_proof->hud_runtime_ready = host_view->hud_runtime_ready;
    out_proof->hud_raw_gdat_capture_ready =
        full_start->hud_raw_gdat_capture_ready;
    out_proof->hud_raw_gdat_portrait_count =
        full_start->hud_raw_gdat_portrait_count;
    out_proof->hud_raw_gdat_portrait_hash =
        full_start->hud_raw_gdat_portrait_hash;
    out_proof->hud_raw_gdat_portrait_byte_count =
        full_start->hud_raw_gdat_portrait_byte_count;
    out_proof->hud_raw_gdat_core_hash =
        full_start->hud_raw_gdat_core_hash;
    out_proof->hud_raw_gdat_core_byte_count =
        full_start->hud_raw_gdat_core_byte_count;
    out_proof->extended_spells = host_view->extended_spells;
    out_proof->first_hud_frame_ready = host_view->first_hud_frame_ready;
    out_proof->status_scope = host_view->status_scope;
    out_proof->status = host_view->status;
    out_proof->title_capture_ready =
        host_view->exact_title_timing_ready &&
        full_start->title_backdrop_ready &&
        host_view->title_gdat_asset_ready &&
        host_view->title_gdat_asset_w > 0 &&
        host_view->title_gdat_asset_h > 0 &&
        host_view->title_gdat_asset_stride >=
            host_view->title_gdat_asset_w;
    out_proof->menu_capture_ready = host_view->startup_menu_assets_ready;
    out_proof->hud_handoff_capture_ready =
        host_view->startup_hud_handoff_ready;
    out_proof->runtime_handoff_capture_ready =
        host_view->runtime_handoff_ready;
    out_proof->m11_consumer_ready =
        host_view->m11_host_view_ready &&
        ((host_view->draw_startup_menu &&
          out_proof->exact_title_timing_ready &&
          out_proof->menu_capture_ready &&
          out_proof->hud_handoff_capture_ready) ||
         out_proof->runtime_handoff_capture_ready);

    /* skproject/SKWIN startup title capture is one package: exact title
     * timing, GDAT title bitmap, menu panel/text, and HUD/runtime handoff.
     * The hash gives M11/tests a stable receipt without reading loose fields. */
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->draw_startup_menu);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->command_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->selected_row);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_animation_tick);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_frame);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_frame_duration_ticks);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_cycle_position_tick);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_frame_start_tick);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_next_frame_tick);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_frame_elapsed_ticks);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_frame_remaining_ticks);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_gdat_category);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_gdat_index);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_gdat_field);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_gdat_asset_w);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_gdat_asset_h);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->menu_gdat_category);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->menu_gdat_field);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_menu_raw_gdat_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->title_raw_gdat_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->title_raw_gdat_byte_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->menu_raw_gdat_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->menu_raw_gdat_byte_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->title_menu_decoded_gdat_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->title_decoded_gdat_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->title_decoded_gdat_pixel_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->menu_decoded_gdat_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->menu_decoded_gdat_pixel_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->menu_row_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->menu_text_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->hud_overlay_suppressed);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->hud_runtime_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->hud_raw_gdat_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->hud_raw_gdat_portrait_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->hud_raw_gdat_portrait_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->hud_raw_gdat_core_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        (uint32_t)out_proof->extended_spells.loaded);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->extended_spells.spell_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
        out_proof->extended_spells.gdat_hash);
    out_proof->packaged_capture_hash = hash;
    out_proof->valid =
        out_proof->host_view_valid &&
        out_proof->full_start_valid &&
        out_proof->m11_consumer_ready &&
        (!full_start->full_start_real_asset_ready ||
         (out_proof->title_menu_raw_gdat_capture_ready &&
          out_proof->title_raw_gdat_hash != 0u &&
          out_proof->title_raw_gdat_byte_count > 0u &&
          out_proof->menu_raw_gdat_hash != 0u &&
          out_proof->menu_raw_gdat_byte_count > 0u)) &&
        out_proof->packaged_capture_hash != 0u;
    return out_proof->valid;
}

void dm2_v1_boot_startup_packaged_full_start_receipt_init(
    DM2_V1_BootStartupPackagedFullStartReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

int dm2_v1_boot_startup_packaged_full_start_receipt_from_host_view(
    const DM2_V1_BootStartupHostViewReceipt *host_view,
    DM2_V1_BootStartupPackagedFullStartReceipt *out_receipt)
{
    uint32_t hash = 0x32465354u;
    const DM2_V1_BootStartupFullStartReceipt *full_start;
    DM2_V1_BootStartupPackagedCaptureProof capture_proof;

    dm2_v1_boot_startup_packaged_full_start_receipt_init(out_receipt);
    if (!host_view || !out_receipt || !host_view->valid ||
        !host_view->full_start.valid ||
        !dm2_v1_boot_startup_packaged_capture_proof_from_host_view(
            host_view,
            &capture_proof)) {
        return 0;
    }
    full_start = &host_view->full_start;
    out_receipt->full_start = *full_start;
    out_receipt->capture_proof = capture_proof;
    out_receipt->full_start_valid = full_start->valid;
    out_receipt->capture_proof_valid = capture_proof.valid;
    out_receipt->full_start_graphics_ready =
        full_start->full_start_graphics_ready;
    out_receipt->full_start_real_asset_ready =
        full_start->full_start_real_asset_ready;
    out_receipt->exact_title_timing_ready =
        capture_proof.exact_title_timing_ready;
    out_receipt->title_capture_ready = capture_proof.title_capture_ready;
    out_receipt->menu_capture_ready = capture_proof.menu_capture_ready;
    out_receipt->hud_handoff_capture_ready =
        capture_proof.hud_handoff_capture_ready;
    out_receipt->runtime_handoff_capture_ready =
        capture_proof.runtime_handoff_capture_ready;
    out_receipt->m11_consumer_ready = capture_proof.m11_consumer_ready;
    out_receipt->title_ready = capture_proof.title_ready;
    out_receipt->runtime_menu_ready = full_start->runtime_menu_ready;
    out_receipt->runtime_action_ready = full_start->runtime_action_ready;
    out_receipt->first_hud_frame_ready = full_start->first_hud_frame_ready;
    out_receipt->startup_menu_active = full_start->startup_menu_active;
    out_receipt->draw_startup_menu = capture_proof.draw_startup_menu;
    out_receipt->command_count = capture_proof.command_count;
    out_receipt->selected_row = capture_proof.selected_row;
    out_receipt->title_animation_tick = capture_proof.title_animation_tick;
    out_receipt->title_frame = capture_proof.title_frame;
    out_receipt->title_frame_max = capture_proof.title_frame_max;
    out_receipt->title_frame_duration_ticks =
        capture_proof.title_frame_duration_ticks;
    out_receipt->title_cycle_ticks = capture_proof.title_cycle_ticks;
    out_receipt->title_cycle_position_tick =
        capture_proof.title_cycle_position_tick;
    out_receipt->title_frame_start_tick =
        capture_proof.title_frame_start_tick;
    out_receipt->title_next_frame_tick = capture_proof.title_next_frame_tick;
    out_receipt->title_frame_elapsed_ticks =
        capture_proof.title_frame_elapsed_ticks;
    out_receipt->title_frame_remaining_ticks =
        capture_proof.title_frame_remaining_ticks;
    out_receipt->title_cycle_remaining_ticks =
        capture_proof.title_cycle_remaining_ticks;
    out_receipt->title_gdat_category = capture_proof.title_gdat_category;
    out_receipt->title_gdat_index = capture_proof.title_gdat_index;
    out_receipt->title_gdat_field = capture_proof.title_gdat_field;
    out_receipt->title_gdat_asset_ready =
        capture_proof.title_gdat_asset_ready;
    out_receipt->title_gdat_asset_w = capture_proof.title_gdat_asset_w;
    out_receipt->title_gdat_asset_h = capture_proof.title_gdat_asset_h;
    out_receipt->title_gdat_asset_stride =
        capture_proof.title_gdat_asset_stride;
    out_receipt->menu_gdat_category = capture_proof.menu_gdat_category;
    out_receipt->menu_gdat_index = capture_proof.menu_gdat_index;
    out_receipt->menu_gdat_field = capture_proof.menu_gdat_field;
    out_receipt->title_menu_raw_gdat_capture_ready =
        capture_proof.title_menu_raw_gdat_capture_ready;
    out_receipt->title_raw_gdat_hash = capture_proof.title_raw_gdat_hash;
    out_receipt->title_raw_gdat_byte_count =
        capture_proof.title_raw_gdat_byte_count;
    out_receipt->menu_raw_gdat_hash = capture_proof.menu_raw_gdat_hash;
    out_receipt->menu_raw_gdat_byte_count =
        capture_proof.menu_raw_gdat_byte_count;
    out_receipt->title_menu_decoded_gdat_capture_ready =
        capture_proof.title_menu_decoded_gdat_capture_ready;
    out_receipt->title_decoded_gdat_hash =
        capture_proof.title_decoded_gdat_hash;
    out_receipt->title_decoded_gdat_pixel_count =
        capture_proof.title_decoded_gdat_pixel_count;
    out_receipt->menu_decoded_gdat_hash =
        capture_proof.menu_decoded_gdat_hash;
    out_receipt->menu_decoded_gdat_pixel_count =
        capture_proof.menu_decoded_gdat_pixel_count;
    out_receipt->menu_row_count = capture_proof.menu_row_count;
    out_receipt->menu_text_count = capture_proof.menu_text_count;
    out_receipt->selectable_text_count =
        capture_proof.selectable_text_count;
    out_receipt->selected_highlight_count =
        capture_proof.selected_highlight_count;
    out_receipt->menu_panel_ready = capture_proof.menu_panel_ready;
    out_receipt->startup_menu_assets_ready =
        capture_proof.startup_menu_assets_ready;
    out_receipt->hud_overlay_suppressed =
        capture_proof.hud_overlay_suppressed;
    out_receipt->hud_runtime_ready = capture_proof.hud_runtime_ready;
    out_receipt->hud_raw_gdat_capture_ready =
        capture_proof.hud_raw_gdat_capture_ready;
    out_receipt->hud_raw_gdat_portrait_count =
        capture_proof.hud_raw_gdat_portrait_count;
    out_receipt->hud_raw_gdat_portrait_hash =
        capture_proof.hud_raw_gdat_portrait_hash;
    out_receipt->hud_raw_gdat_portrait_byte_count =
        capture_proof.hud_raw_gdat_portrait_byte_count;
    out_receipt->hud_raw_gdat_core_hash =
        capture_proof.hud_raw_gdat_core_hash;
    out_receipt->hud_raw_gdat_core_byte_count =
        capture_proof.hud_raw_gdat_core_byte_count;
    out_receipt->extended_spells = capture_proof.extended_spells;
    out_receipt->status_scope = capture_proof.status_scope;
    out_receipt->status = capture_proof.status;

    /* skproject/SKWIN title startup is treated as one full-start package:
     * exact timing, real GDAT title/menu proof, HUD suppression, and host
     * consumer readiness. Host callers can consume this instead of pairing
     * full_start and capture_proof fields manually. */
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, capture_proof.packaged_capture_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->full_start_graphics_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->full_start_real_asset_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->title_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->title_menu_raw_gdat_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->title_raw_gdat_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->title_raw_gdat_byte_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_raw_gdat_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_raw_gdat_byte_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->title_menu_decoded_gdat_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->title_decoded_gdat_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->title_decoded_gdat_pixel_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_decoded_gdat_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_decoded_gdat_pixel_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->menu_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->hud_handoff_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->hud_raw_gdat_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->hud_raw_gdat_portrait_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->hud_raw_gdat_core_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->extended_spells.loaded);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->extended_spells.spell_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->extended_spells.gdat_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->first_hud_frame_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_action_ready);
    out_receipt->packaged_full_start_hash = hash;
    out_receipt->valid =
        out_receipt->full_start_valid &&
        out_receipt->capture_proof_valid &&
        out_receipt->exact_title_timing_ready &&
        (!out_receipt->full_start_real_asset_ready ||
         (out_receipt->title_menu_raw_gdat_capture_ready &&
          out_receipt->title_raw_gdat_hash != 0u &&
          out_receipt->title_raw_gdat_byte_count > 0u &&
          out_receipt->menu_raw_gdat_hash != 0u &&
          out_receipt->menu_raw_gdat_byte_count > 0u &&
          out_receipt->title_menu_decoded_gdat_capture_ready)) &&
        out_receipt->menu_capture_ready &&
        out_receipt->m11_consumer_ready &&
        out_receipt->packaged_full_start_hash != 0u;
    return out_receipt->valid;
}

static int dm2_v1_boot_startup_fill_host_view_receipt(
    DM2_V1_BootStartupViewModel *view_model)
{
    DM2_V1_BootStartupHostViewReceipt *receipt;
    const DM2_V1_BootStartupFullStartReceipt *full_start;

    if (!view_model || !view_model->full_start_receipt.valid) {
        return 0;
    }
    receipt = &view_model->host_view_receipt;
    full_start = &view_model->full_start_receipt;
    dm2_v1_boot_startup_host_view_receipt_clear(receipt);
    receipt->valid = 1;
    receipt->command_count = view_model->command_count;
    receipt->selected_row = view_model->view_receipt.menu_state.selected_row;
    receipt->render_commands_ready = view_model->command_count > 0;
    receipt->menu_state_ready =
        view_model->view_receipt.menu_state.row_count > 0 &&
        view_model->view_receipt.menu_state.selected_row >= 0 &&
        view_model->view_receipt.menu_state.selected_row <
            view_model->view_receipt.menu_state.row_count;
    receipt->row_selection_ready = receipt->menu_state_ready;
    receipt->resume_menu_ready =
        view_model->view_receipt.render.resume_menu_ready;
    receipt->save_slot_menu_ready =
        view_model->view_receipt.render.save_slot_menu_ready;
    receipt->new_game_menu_ready =
        view_model->view_receipt.render.new_game_menu_ready;
    receipt->title_timing_ready =
        full_start->startup_menu_active &&
        full_start->exact_title_timing_ready;
    receipt->title_asset_ready =
        full_start->full_start_real_asset_ready
            ? 1
            : full_start->title_backdrop_ready;
    receipt->title_menu_ready =
        receipt->title_timing_ready &&
        receipt->title_asset_ready &&
        receipt->new_game_menu_ready;
    receipt->title_animation_tick = full_start->title_animation_tick;
    receipt->draw_startup_menu =
        full_start->startup_menu_active &&
        full_start->full_start_graphics_ready &&
        full_start->hud_overlay_suppressed;
    receipt->title_frame = full_start->title_frame;
    receipt->title_frame_max = full_start->title_frame_max;
    receipt->title_frame_duration_ticks =
        full_start->title_frame_duration_ticks;
    receipt->title_ready = full_start->title_ready;
    receipt->title_gdat_asset_ready = full_start->title_gdat_asset_ready;
    receipt->title_gdat_asset_w = full_start->title_gdat_asset_w;
    receipt->title_gdat_asset_h = full_start->title_gdat_asset_h;
    receipt->title_gdat_asset_stride = full_start->title_gdat_asset_stride;
    receipt->full_start_real_asset_ready =
        full_start->full_start_real_asset_ready;
    receipt->title_cycle_ticks = full_start->title_cycle_ticks;
    receipt->title_cycle_position_tick =
        full_start->title_cycle_position_tick;
    receipt->title_frame_start_tick = full_start->title_frame_start_tick;
    receipt->title_next_frame_tick = full_start->title_next_frame_tick;
    receipt->title_frame_elapsed_ticks =
        full_start->title_frame_elapsed_ticks;
    receipt->title_frame_remaining_ticks =
        full_start->title_frame_remaining_ticks;
    receipt->title_cycle_remaining_ticks =
        full_start->title_cycle_remaining_ticks;
    receipt->exact_title_timing_ready = full_start->exact_title_timing_ready;
    receipt->menu_row_count = full_start->menu_row_count;
    receipt->menu_text_count = full_start->menu_text_count;
    receipt->selectable_text_count = full_start->selectable_text_count;
    receipt->selected_highlight_count = full_start->selected_highlight_count;
    receipt->menu_panel_ready = full_start->menu_panel_ready;
    receipt->startup_menu_assets_ready =
        full_start->startup_menu_assets_ready;
    receipt->hud_overlay_suppressed = full_start->hud_overlay_suppressed;
    receipt->hud_runtime_ready = full_start->hud_runtime_ready;
    receipt->runtime_menu_ready = full_start->runtime_menu_ready;
    receipt->runtime_action_ready = full_start->runtime_action_ready;
    receipt->first_hud_frame_ready = full_start->first_hud_frame_ready;
    receipt->extended_spells = full_start->extended_spells;
    receipt->startup_hud_handoff_ready =
        receipt->draw_startup_menu &&
        receipt->hud_overlay_suppressed &&
        receipt->hud_runtime_ready &&
        !receipt->first_hud_frame_ready;
    receipt->runtime_handoff_ready =
        full_start->title_ready &&
        receipt->runtime_action_ready &&
        receipt->first_hud_frame_ready;
    receipt->m11_host_view_ready =
        receipt->valid &&
        receipt->exact_title_timing_ready &&
        ((receipt->draw_startup_menu &&
          receipt->startup_menu_assets_ready &&
          receipt->startup_hud_handoff_ready) ||
         receipt->runtime_handoff_ready);
    receipt->status_scope =
        receipt->draw_startup_menu ? "STARTUP" : "RUNTIME";
    receipt->status =
        receipt->draw_startup_menu ? "DM2 STARTUP MENU" : "DM2 RUNTIME";
    receipt->log_line =
        receipt->draw_startup_menu ? "T0: DM2 STARTUP MENU"
                                   : "T0: DM2 RUNTIME";
    receipt->full_start = *full_start;
    receipt->capture_proof_valid =
        dm2_v1_boot_startup_packaged_capture_proof_from_host_view(
            receipt,
            &receipt->capture_proof);
    /* M11 host consumption contract: callers gate startup drawing and probe
     * state on this receipt, not on ad-hoc command-count/HUD flag checks. */
    return 1;
}

int dm2_v1_boot_startup_view_model_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupViewModel *out_view_model)
{
    int ok;
    if (!snapshot || !out_view_model) {
        return 0;
    }
    dm2_v1_boot_startup_view_model_clear(out_view_model);
    ok = dm2_v1_boot_startup_view_model_from_snapshot(
        snapshot,
        out_view_model->commands,
        (int)(sizeof(out_view_model->commands) /
              sizeof(out_view_model->commands[0])),
        &out_view_model->command_count,
        &out_view_model->view_receipt,
        out_view_model->phase,
        (int)sizeof(out_view_model->phase),
        &out_view_model->startup_active,
        out_view_model->animation,
        (int)sizeof(out_view_model->animation),
        &out_view_model->animation_active,
        &out_view_model->title_frame,
        &out_view_model->title_frame_max,
        &out_view_model->title_ready);
    if (ok && out_view_model->view_receipt.runtime_handoff.valid) {
        const DM2_V1_StartupRuntimeHandoffReceipt *handoff =
            &out_view_model->view_receipt.runtime_handoff;
        out_view_model->initialize_v2_runtime =
            handoff->initialize_v2_runtime;
        out_view_model->initialize_hud_runtime =
            handoff->initialize_hud_runtime;
        out_view_model->initialize_touch_runtime =
            handoff->initialize_touch_runtime;
        out_view_model->hud_runtime_ready = handoff->hud_runtime_ready;
        out_view_model->runtime_menu_ready = handoff->runtime_menu_ready;
        out_view_model->runtime_action_ready = handoff->runtime_action_ready;
        out_view_model->first_hud_frame_ready =
            handoff->first_hud_frame_ready;
        out_view_model->title_backdrop_ready =
            out_view_model->view_receipt.render.title_backdrop_ready;
        out_view_model->resume_menu_ready =
            out_view_model->view_receipt.render.resume_menu_ready;
        out_view_model->save_slot_menu_ready =
            out_view_model->view_receipt.render.save_slot_menu_ready;
        out_view_model->new_game_menu_ready =
            out_view_model->view_receipt.render.new_game_menu_ready;
        out_view_model->full_start_graphics_ready =
            out_view_model->view_receipt.render.full_start_graphics_ready;
        if (snapshot->profile &&
            out_view_model->view_receipt.render.title_backdrop_ready) {
            uint8_t *pixels = NULL;
            int w = 0;
            int h = 0;
            int stride = 0;
            const DM2_V1_StartupRenderReceipt *render =
                &out_view_model->view_receipt.render;
            /* skproject/SKWIN boot title/menu uses the real GDAT title
             * bitmap. The boot profile owns the GRAPHICS.DAT cache, so M11
             * receives an asset receipt instead of probing GDAT itself. */
            if (dm2_v1_boot_gdat_image_asset_fetch(
                    (DM2_V1_BootProfile *)snapshot->profile,
                    render->title_gdat_category,
                    render->title_gdat_index,
                    render->title_gdat_field,
                    &pixels,
                    &w,
                    &h,
                    &stride) == 0 &&
                pixels &&
                w == render->title_rect.w &&
                h == render->title_rect.h &&
                stride >= w) {
                out_view_model->title_gdat_asset_ready = 1;
                out_view_model->title_gdat_asset_w = w;
                out_view_model->title_gdat_asset_h = h;
                out_view_model->title_gdat_asset_stride = stride;
            }
            dm2_v1_boot_gdat_image_asset_free(pixels);
        }
        out_view_model->full_start_real_asset_ready =
            out_view_model->full_start_graphics_ready &&
            out_view_model->title_gdat_asset_ready;
        (void)dm2_v1_boot_startup_fill_full_start_receipt(
            snapshot,
            out_view_model);
        (void)dm2_v1_boot_startup_fill_host_view_receipt(
            out_view_model);
    }
    return ok;
}

static int dm2_v1_boot_startup_view_model_receipt_from_snapshot_tick(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    int title_animation_tick,
    DM2_V1_BootStartupViewModel *out_view_model)
{
    DM2_V1_StartupRuntimeHandoffReceipt handoff;

    if (!snapshot || !out_view_model ||
        !dm2_v1_boot_startup_view_model_receipt_from_snapshot(
            snapshot,
            out_view_model) ||
        !out_view_model->view_receipt.valid) {
        return 0;
    }
    memset(&handoff, 0, sizeof(handoff));
    if (!dm2_v1_startup_runtime_handoff_receipt_from_tick(
            &handoff,
            snapshot->startup_menu_active,
            1,
            title_animation_tick)) {
        return 0;
    }
    out_view_model->view_receipt.runtime_handoff = handoff;
    snprintf(out_view_model->phase,
             sizeof(out_view_model->phase),
             "%s",
             handoff.animation);
    out_view_model->startup_active = handoff.startup_menu_active;
    snprintf(out_view_model->animation,
             sizeof(out_view_model->animation),
             "%s",
             handoff.animation);
    out_view_model->animation_active = handoff.animation_active;
    out_view_model->title_frame = handoff.title_frame;
    out_view_model->title_frame_max = handoff.title_frame_max;
    out_view_model->title_ready = handoff.title_ready;
    out_view_model->initialize_v2_runtime = handoff.initialize_v2_runtime;
    out_view_model->initialize_hud_runtime = handoff.initialize_hud_runtime;
    out_view_model->initialize_touch_runtime = handoff.initialize_touch_runtime;
    out_view_model->hud_runtime_ready = handoff.hud_runtime_ready;
    out_view_model->runtime_menu_ready = handoff.runtime_menu_ready;
    out_view_model->runtime_action_ready = handoff.runtime_action_ready;
    out_view_model->first_hud_frame_ready = handoff.first_hud_frame_ready;
    (void)dm2_v1_boot_startup_fill_full_start_receipt(
        snapshot,
        out_view_model);
    (void)dm2_v1_boot_startup_fill_host_view_receipt(out_view_model);
    return out_view_model->full_start_receipt.valid &&
           out_view_model->host_view_receipt.valid;
}

int dm2_v1_boot_startup_host_view_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupHostViewReceipt *out_receipt)
{
    DM2_V1_BootStartupViewModel view_model;

    if (out_receipt) {
        dm2_v1_boot_startup_host_view_receipt_clear(out_receipt);
    }
    if (!snapshot || !out_receipt ||
        !dm2_v1_boot_startup_view_model_receipt_from_snapshot(
            snapshot,
            &view_model) ||
        !view_model.host_view_receipt.valid) {
        return 0;
    }
    *out_receipt = view_model.host_view_receipt;
    return 1;
}

int dm2_v1_boot_startup_host_view_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupHostViewReceipt *out_receipt)
{
    DM2_V1_BootRuntimeStartupSnapshot snapshot;
    DM2_V1_BootStartupViewModel view_model;

    if (out_receipt) {
        dm2_v1_boot_startup_host_view_receipt_clear(out_receipt);
    }
    if (!out_receipt) {
        return 0;
    }
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.profile = profile;
    snapshot.startup_menu_active = startup_menu_active;
    snapshot.startup_save_root = startup_save_root;
    snapshot.resume_available = resume_available;
    snapshot.slot_mask = slot_mask;
    snapshot.selected_row = selected_row;
    if (!dm2_v1_boot_startup_view_model_receipt_from_snapshot_tick(
            &snapshot,
            title_animation_tick,
            &view_model) ||
        !view_model.host_view_receipt.valid) {
        return 0;
    }
    *out_receipt = view_model.host_view_receipt;
    out_receipt->interface_rect14_host_ready =
        dm2_v1_boot_interface_rect14_host_receipt(
            (DM2_V1_BootProfile *)profile,
            &out_receipt->interface_rect14);
    {
        DM2_V1_ExtendedSpellGdatReceipt spells;
        if (dm2_v1_boot_extended_spell_gdat_receipt(
                (DM2_V1_BootProfile *)profile, &spells)) {
            out_receipt->extended_spell_gdat_ready = spells.valid;
            out_receipt->extended_spell_gdat_defined_count = spells.defined_count;
            out_receipt->extended_spell_gdat_word_hash = spells.word_hash;
        }
    }
    return 1;
}

int dm2_v1_boot_startup_packaged_full_start_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupPackagedFullStartReceipt *out_receipt)
{
    DM2_V1_BootStartupHostViewReceipt host_view;

    dm2_v1_boot_startup_packaged_full_start_receipt_init(out_receipt);
    if (!snapshot || !out_receipt ||
        !dm2_v1_boot_startup_host_view_receipt_from_snapshot(
            snapshot,
            &host_view)) {
        return 0;
    }
    return dm2_v1_boot_startup_packaged_full_start_receipt_from_host_view(
        &host_view,
        out_receipt);
}

int dm2_v1_boot_startup_packaged_full_start_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupPackagedFullStartReceipt *out_receipt)
{
    DM2_V1_BootStartupHostViewReceipt host_view;

    dm2_v1_boot_startup_packaged_full_start_receipt_init(out_receipt);
    if (!out_receipt ||
        !dm2_v1_boot_startup_host_view_receipt_from_runtime_state(
            profile,
            startup_menu_active,
            startup_save_root,
            resume_available,
            slot_mask,
            selected_row,
            title_animation_tick,
            &host_view)) {
        return 0;
    }
    return dm2_v1_boot_startup_packaged_full_start_receipt_from_host_view(
        &host_view,
        out_receipt);
}

void dm2_v1_boot_startup_packaged_consumer_receipt_init(
    DM2_V1_BootStartupPackagedConsumerReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

int dm2_v1_boot_startup_packaged_consumer_receipt_from_full_start(
    const DM2_V1_BootStartupPackagedFullStartReceipt *package,
    DM2_V1_BootStartupPackagedConsumerReceipt *out_receipt)
{
    int draw_ready;

    dm2_v1_boot_startup_packaged_consumer_receipt_init(out_receipt);
    if (!package || !out_receipt || !package->valid) {
        return 0;
    }

    draw_ready =
        package->draw_startup_menu &&
        package->menu_capture_ready &&
        package->hud_handoff_capture_ready &&
        package->command_count > 0;

    /* skproject/SKWIN boot hosts consume the title/menu/HUD startup state as
     * one receipt; keep draw/probe gates here instead of exposing callers to
     * loose timing, command-count, and asset-readiness fields. */
    out_receipt->packaged_full_start_valid = package->valid;
    out_receipt->packaged_full_start_hash = package->packaged_full_start_hash;
    out_receipt->startup_active = package->draw_startup_menu;
    out_receipt->startup_animation_active =
        package->hud_handoff_capture_ready;
    out_receipt->startup_title_frame = package->title_frame;
    out_receipt->startup_title_frame_max = package->title_frame_max;
    out_receipt->startup_title_ready = package->title_ready;
    out_receipt->startup_hud_runtime_ready = package->hud_runtime_ready;
    out_receipt->startup_hud_raw_gdat_capture_ready =
        package->hud_raw_gdat_capture_ready;
    out_receipt->startup_hud_raw_gdat_portrait_count =
        package->hud_raw_gdat_portrait_count;
    out_receipt->startup_hud_raw_gdat_portrait_hash =
        package->hud_raw_gdat_portrait_hash;
    out_receipt->startup_hud_raw_gdat_portrait_byte_count =
        package->hud_raw_gdat_portrait_byte_count;
    out_receipt->startup_hud_raw_gdat_core_hash =
        package->hud_raw_gdat_core_hash;
    out_receipt->startup_hud_raw_gdat_core_byte_count =
        package->hud_raw_gdat_core_byte_count;
    out_receipt->startup_title_menu_raw_gdat_capture_ready =
        package->title_menu_raw_gdat_capture_ready;
    out_receipt->startup_title_raw_gdat_hash = package->title_raw_gdat_hash;
    out_receipt->startup_title_raw_gdat_byte_count =
        package->title_raw_gdat_byte_count;
    out_receipt->startup_menu_raw_gdat_hash = package->menu_raw_gdat_hash;
    out_receipt->startup_menu_raw_gdat_byte_count =
        package->menu_raw_gdat_byte_count;
    out_receipt->startup_title_menu_decoded_gdat_capture_ready =
        package->title_menu_decoded_gdat_capture_ready;
    out_receipt->startup_title_decoded_gdat_hash =
        package->title_decoded_gdat_hash;
    out_receipt->startup_title_decoded_gdat_pixel_count =
        package->title_decoded_gdat_pixel_count;
    out_receipt->startup_menu_decoded_gdat_hash =
        package->menu_decoded_gdat_hash;
    out_receipt->startup_menu_decoded_gdat_pixel_count =
        package->menu_decoded_gdat_pixel_count;
    out_receipt->extended_spells = package->extended_spells;
    out_receipt->startup_draw_ready = draw_ready;
    out_receipt->startup_draw_command_count = package->command_count;
    out_receipt->startup_draw_menu_capture_ready =
        package->menu_capture_ready;
    out_receipt->startup_draw_hud_handoff_ready =
        package->hud_handoff_capture_ready;
    out_receipt->title_capture_ready = package->title_capture_ready;
    out_receipt->menu_capture_ready = package->menu_capture_ready;
    out_receipt->hud_handoff_capture_ready =
        package->hud_handoff_capture_ready;
    out_receipt->runtime_handoff_capture_ready =
        package->runtime_handoff_capture_ready;
    out_receipt->exact_title_timing_ready =
        package->exact_title_timing_ready;
    out_receipt->packaged_title_timing_consumed =
        package->exact_title_timing_ready &&
        (!package->full_start_real_asset_ready ||
         (package->title_menu_raw_gdat_capture_ready &&
          package->title_raw_gdat_hash != 0u &&
          package->title_raw_gdat_byte_count > 0u &&
          package->menu_raw_gdat_hash != 0u &&
          package->menu_raw_gdat_byte_count > 0u &&
          package->title_menu_decoded_gdat_capture_ready)) &&
        package->exact_title_timing_ready &&
        package->title_animation_tick == 0 &&
        package->title_frame_duration_ticks == 0 &&
        package->title_frame_max == 0 &&
        package->title_cycle_ticks == 1;
    out_receipt->packaged_first_hud_receipt_consumed =
        (package->hud_handoff_capture_ready &&
         (!package->full_start_real_asset_ready ||
          package->hud_raw_gdat_capture_ready)) ||
        package->runtime_handoff_capture_ready;
    out_receipt->m11_startup_receipt_ready =
        package->m11_consumer_ready &&
        out_receipt->packaged_title_timing_consumed &&
        out_receipt->packaged_first_hud_receipt_consumed &&
        (!package->full_start_real_asset_ready ||
         (out_receipt->startup_title_menu_raw_gdat_capture_ready &&
          out_receipt->startup_title_raw_gdat_hash != 0u &&
          out_receipt->startup_title_raw_gdat_byte_count > 0u &&
          out_receipt->startup_menu_raw_gdat_hash != 0u &&
          out_receipt->startup_menu_raw_gdat_byte_count > 0u));
    out_receipt->full_start_real_asset_ready =
        package->full_start_real_asset_ready;
    out_receipt->runtime_menu_ready = package->runtime_menu_ready;
    out_receipt->runtime_action_ready = package->runtime_action_ready;
    out_receipt->first_hud_frame_ready = package->first_hud_frame_ready;
    out_receipt->title_gdat_asset_ready = package->title_gdat_asset_ready;
    out_receipt->title_gdat_asset_w = package->title_gdat_asset_w;
    out_receipt->title_gdat_asset_h = package->title_gdat_asset_h;
    out_receipt->title_frame_duration_ticks =
        package->title_frame_duration_ticks;
    out_receipt->title_cycle_ticks = package->title_cycle_ticks;
    out_receipt->title_cycle_position_tick =
        package->title_cycle_position_tick;
    out_receipt->title_frame_start_tick = package->title_frame_start_tick;
    out_receipt->title_next_frame_tick = package->title_next_frame_tick;
    out_receipt->title_frame_elapsed_ticks =
        package->title_frame_elapsed_ticks;
    out_receipt->title_frame_remaining_ticks =
        package->title_frame_remaining_ticks;
    out_receipt->phase =
        package->draw_startup_menu ? "dm2-startup-menu" : "dm2-runtime";
    out_receipt->animation = out_receipt->phase;
    out_receipt->status_scope = package->status_scope;
    out_receipt->status = package->status;
    out_receipt->valid =
        out_receipt->packaged_full_start_valid &&
        out_receipt->m11_startup_receipt_ready &&
        out_receipt->packaged_full_start_hash != 0u;
    return out_receipt->valid;
}

int dm2_v1_boot_startup_packaged_consumer_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupPackagedConsumerReceipt *out_receipt)
{
    DM2_V1_BootStartupPackagedFullStartReceipt package;

    dm2_v1_boot_startup_packaged_consumer_receipt_init(out_receipt);
    if (!snapshot || !out_receipt ||
        !dm2_v1_boot_startup_packaged_full_start_receipt_from_snapshot(
            snapshot,
            &package)) {
        return 0;
    }
    return dm2_v1_boot_startup_packaged_consumer_receipt_from_full_start(
        &package,
        out_receipt);
}

int dm2_v1_boot_startup_packaged_consumer_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupPackagedConsumerReceipt *out_receipt)
{
    DM2_V1_BootStartupPackagedFullStartReceipt package;

    dm2_v1_boot_startup_packaged_consumer_receipt_init(out_receipt);
    if (!out_receipt ||
        !dm2_v1_boot_startup_packaged_full_start_receipt_from_runtime_state(
            profile,
            startup_menu_active,
            startup_save_root,
            resume_available,
            slot_mask,
            selected_row,
            title_animation_tick,
            &package)) {
        return 0;
    }
    return dm2_v1_boot_startup_packaged_consumer_receipt_from_full_start(
        &package,
        out_receipt);
}

void dm2_v1_boot_startup_host_frame_receipt_init(
    DM2_V1_BootStartupHostFrameReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

int dm2_v1_boot_startup_host_frame_receipt_from_consumer(
    const DM2_V1_BootStartupPackagedConsumerReceipt *consumer,
    DM2_V1_BootStartupHostFrameReceipt *out_receipt)
{
    dm2_v1_boot_startup_host_frame_receipt_init(out_receipt);
    if (!consumer || !out_receipt || !consumer->valid) {
        return 0;
    }

    /* skproject/SKWIN keeps boot-title drawing, startup menu selection, and
     * the first HUD/runtime handoff in one host frame decision. Preserve that
     * as a DM2-owned receipt so M11 does not re-derive draw/input/HUD gates
     * from individual timing and command-count fields. */
    out_receipt->consume_startup_package =
        consumer->m11_startup_receipt_ready;
    out_receipt->render_startup_title =
        consumer->startup_active &&
        consumer->startup_draw_ready &&
        consumer->packaged_title_timing_consumed &&
        (!consumer->full_start_real_asset_ready ||
         (consumer->startup_title_menu_raw_gdat_capture_ready &&
          consumer->startup_title_raw_gdat_hash != 0u &&
          consumer->startup_title_raw_gdat_byte_count > 0u &&
          consumer->startup_menu_raw_gdat_hash != 0u &&
          consumer->startup_menu_raw_gdat_byte_count > 0u &&
          consumer->startup_title_menu_decoded_gdat_capture_ready));
    out_receipt->render_startup_menu =
        consumer->startup_active &&
        consumer->startup_draw_ready &&
        consumer->startup_draw_menu_capture_ready;
    out_receipt->suppress_game_hud =
        consumer->startup_active &&
        consumer->startup_hud_runtime_ready &&
        (!consumer->full_start_real_asset_ready ||
         consumer->startup_hud_raw_gdat_capture_ready) &&
        !consumer->first_hud_frame_ready;
    out_receipt->enable_runtime_input =
        consumer->runtime_action_ready &&
        consumer->first_hud_frame_ready &&
        !consumer->startup_active;
    out_receipt->present_first_hud_frame =
        consumer->first_hud_frame_ready &&
        !out_receipt->suppress_game_hud;
    out_receipt->schedule_next_title_tick = 0;
    out_receipt->next_title_tick_delta =
        out_receipt->schedule_next_title_tick
            ? consumer->title_frame_remaining_ticks
            : 0;
    out_receipt->title_animation_tick =
        consumer->title_cycle_position_tick;
    out_receipt->title_frame = consumer->startup_title_frame;
    out_receipt->title_frame_max = consumer->startup_title_frame_max;
    out_receipt->title_frame_duration_ticks =
        consumer->title_frame_duration_ticks;
    out_receipt->title_frame_elapsed_ticks =
        consumer->title_frame_elapsed_ticks;
    out_receipt->title_frame_remaining_ticks =
        consumer->title_frame_remaining_ticks;
    out_receipt->title_next_frame_tick = consumer->title_next_frame_tick;
    out_receipt->startup_draw_command_count =
        consumer->startup_draw_command_count;
    out_receipt->startup_draw_ready = consumer->startup_draw_ready;
    out_receipt->startup_hud_runtime_ready =
        consumer->startup_hud_runtime_ready;
    out_receipt->startup_hud_raw_gdat_capture_ready =
        consumer->startup_hud_raw_gdat_capture_ready;
    out_receipt->startup_hud_raw_gdat_portrait_count =
        consumer->startup_hud_raw_gdat_portrait_count;
    out_receipt->startup_hud_raw_gdat_portrait_hash =
        consumer->startup_hud_raw_gdat_portrait_hash;
    out_receipt->startup_hud_raw_gdat_portrait_byte_count =
        consumer->startup_hud_raw_gdat_portrait_byte_count;
    out_receipt->startup_hud_raw_gdat_core_hash =
        consumer->startup_hud_raw_gdat_core_hash;
    out_receipt->startup_hud_raw_gdat_core_byte_count =
        consumer->startup_hud_raw_gdat_core_byte_count;
    out_receipt->startup_title_menu_raw_gdat_capture_ready =
        consumer->startup_title_menu_raw_gdat_capture_ready;
    out_receipt->startup_title_raw_gdat_hash =
        consumer->startup_title_raw_gdat_hash;
    out_receipt->startup_title_raw_gdat_byte_count =
        consumer->startup_title_raw_gdat_byte_count;
    out_receipt->startup_menu_raw_gdat_hash =
        consumer->startup_menu_raw_gdat_hash;
    out_receipt->startup_menu_raw_gdat_byte_count =
        consumer->startup_menu_raw_gdat_byte_count;
    out_receipt->startup_title_menu_decoded_gdat_capture_ready =
        consumer->startup_title_menu_decoded_gdat_capture_ready;
    out_receipt->startup_title_decoded_gdat_hash =
        consumer->startup_title_decoded_gdat_hash;
    out_receipt->startup_title_decoded_gdat_pixel_count =
        consumer->startup_title_decoded_gdat_pixel_count;
    out_receipt->startup_menu_decoded_gdat_hash =
        consumer->startup_menu_decoded_gdat_hash;
    out_receipt->startup_menu_decoded_gdat_pixel_count =
        consumer->startup_menu_decoded_gdat_pixel_count;
    out_receipt->extended_spells = consumer->extended_spells;
    out_receipt->runtime_menu_ready = consumer->runtime_menu_ready;
    out_receipt->runtime_action_ready = consumer->runtime_action_ready;
    out_receipt->first_hud_frame_ready = consumer->first_hud_frame_ready;
    out_receipt->packaged_full_start_hash =
        consumer->packaged_full_start_hash;
    out_receipt->phase = consumer->phase;
    out_receipt->animation = consumer->animation;
    out_receipt->status_scope = consumer->status_scope;
    out_receipt->status = consumer->status;
    out_receipt->valid =
        out_receipt->consume_startup_package &&
        ((out_receipt->render_startup_title &&
          out_receipt->render_startup_menu &&
          out_receipt->suppress_game_hud) ||
         (out_receipt->enable_runtime_input &&
          out_receipt->present_first_hud_frame)) &&
        out_receipt->packaged_full_start_hash != 0u;
    return out_receipt->valid;
}

int dm2_v1_boot_startup_host_frame_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupHostFrameReceipt *out_receipt)
{
    DM2_V1_BootStartupPackagedConsumerReceipt consumer;

    dm2_v1_boot_startup_host_frame_receipt_init(out_receipt);
    if (!snapshot || !out_receipt ||
        !dm2_v1_boot_startup_packaged_consumer_receipt_from_snapshot(
            snapshot,
            &consumer)) {
        return 0;
    }
    return dm2_v1_boot_startup_host_frame_receipt_from_consumer(
        &consumer,
        out_receipt);
}

int dm2_v1_boot_startup_host_frame_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupHostFrameReceipt *out_receipt)
{
    DM2_V1_BootStartupPackagedConsumerReceipt consumer;

    dm2_v1_boot_startup_host_frame_receipt_init(out_receipt);
    if (!out_receipt ||
        !dm2_v1_boot_startup_packaged_consumer_receipt_from_runtime_state(
            profile,
            startup_menu_active,
            startup_save_root,
            resume_available,
            slot_mask,
            selected_row,
            title_animation_tick,
            &consumer)) {
        return 0;
    }
    return dm2_v1_boot_startup_host_frame_receipt_from_consumer(
        &consumer,
        out_receipt);
}

void dm2_v1_boot_startup_render_ownership_receipt_init(
    DM2_V1_BootStartupRenderOwnershipReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

static int dm2_v1_boot_startup_render_ownership_from_view_model(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    const DM2_V1_BootStartupViewModel *view_model,
    DM2_V1_BootStartupRenderOwnershipReceipt *out_receipt)
{
    DM2_V1_BootStartupPackagedFullStartReceipt package;
    DM2_V1_BootStartupPackagedConsumerReceipt consumer;
    DM2_V1_BootStartupHostFrameReceipt host_frame;
    int i;

    dm2_v1_boot_startup_render_ownership_receipt_init(out_receipt);
    if (!snapshot || !view_model || !out_receipt ||
        !view_model->host_view_receipt.valid ||
        !dm2_v1_boot_startup_packaged_full_start_receipt_from_host_view(
            &view_model->host_view_receipt,
            &package) ||
        !dm2_v1_boot_startup_packaged_consumer_receipt_from_full_start(
            &package,
            &consumer) ||
        !dm2_v1_boot_startup_host_frame_receipt_from_consumer(
            &consumer,
            &host_frame)) {
        return 0;
    }

    /* skproject/SKWIN keeps startup rendering owned by the boot/title/menu
     * path: GDAT title blit, menu commands, HUD suppression, and next title
     * tick are consumed together before runtime HUD drawing can take over. */
    out_receipt->packaged_full_start_valid = package.valid;
    out_receipt->host_frame_valid = host_frame.valid;
    out_receipt->consume_host_frame_receipt =
        host_frame.consume_startup_package;
    out_receipt->execute_startup_draw_commands =
        host_frame.render_startup_title &&
        host_frame.render_startup_menu &&
        host_frame.suppress_game_hud;
    out_receipt->draw_command_count = view_model->command_count;
    out_receipt->title_gdat_command_count =
        package.title_gdat_category == DM2_GDAT_CATEGORY_TITLE &&
        package.title_gdat_field == DM2_GDAT_TITLE_MENU_SCREEN_FIELD ? 1 : 0;
    out_receipt->suppress_game_hud = host_frame.suppress_game_hud;
    out_receipt->startup_hud_raw_gdat_receipt_consumed =
        host_frame.startup_hud_raw_gdat_capture_ready;
    out_receipt->startup_title_menu_raw_gdat_receipt_consumed =
        host_frame.startup_title_menu_raw_gdat_capture_ready;
    out_receipt->startup_title_raw_gdat_hash =
        host_frame.startup_title_raw_gdat_hash;
    out_receipt->startup_title_raw_gdat_byte_count =
        host_frame.startup_title_raw_gdat_byte_count;
    out_receipt->startup_menu_raw_gdat_hash =
        host_frame.startup_menu_raw_gdat_hash;
    out_receipt->startup_menu_raw_gdat_byte_count =
        host_frame.startup_menu_raw_gdat_byte_count;
    out_receipt->startup_title_menu_decoded_gdat_receipt_consumed =
        host_frame.startup_title_menu_decoded_gdat_capture_ready;
    out_receipt->startup_title_decoded_gdat_hash =
        host_frame.startup_title_decoded_gdat_hash;
    out_receipt->startup_title_decoded_gdat_pixel_count =
        host_frame.startup_title_decoded_gdat_pixel_count;
    out_receipt->startup_menu_decoded_gdat_hash =
        host_frame.startup_menu_decoded_gdat_hash;
    out_receipt->startup_menu_decoded_gdat_pixel_count =
        host_frame.startup_menu_decoded_gdat_pixel_count;
    out_receipt->extended_spells = host_frame.extended_spells;
    out_receipt->extended_spells_definition_consumed =
        !out_receipt->extended_spells.loaded ||
        (out_receipt->extended_spells.spell_count > 0u &&
         out_receipt->extended_spells.gdat_hash != 0u);
    out_receipt->present_first_hud_frame =
        host_frame.present_first_hud_frame;
    out_receipt->schedule_next_title_tick =
        host_frame.schedule_next_title_tick;
    out_receipt->next_title_tick_delta =
        host_frame.next_title_tick_delta;
    out_receipt->title_animation_tick =
        host_frame.title_animation_tick;
    out_receipt->title_frame = host_frame.title_frame;
    out_receipt->title_frame_max = host_frame.title_frame_max;
    out_receipt->title_frame_duration_ticks =
        host_frame.title_frame_duration_ticks;
    out_receipt->title_frame_elapsed_ticks =
        host_frame.title_frame_elapsed_ticks;
    out_receipt->title_frame_remaining_ticks =
        host_frame.title_frame_remaining_ticks;
    out_receipt->title_next_frame_tick =
        host_frame.title_next_frame_tick;
    out_receipt->runtime_menu_ready = host_frame.runtime_menu_ready;
    out_receipt->runtime_action_ready = host_frame.runtime_action_ready;
    out_receipt->first_hud_frame_ready = host_frame.first_hud_frame_ready;
    out_receipt->packaged_full_start_hash =
        host_frame.packaged_full_start_hash;
    out_receipt->phase = host_frame.phase;
    out_receipt->animation = host_frame.animation;
    out_receipt->status_scope = host_frame.status_scope;
    out_receipt->status = host_frame.status;

    for (i = 0; i < view_model->command_count; ++i) {
        const DM2_V1_StartupDrawCommand *command =
            &view_model->commands[i];
        if (command->kind == DM2_V1_STARTUP_DRAW_NONE) {
            continue;
        }
        ++out_receipt->executed_command_count;
        if (command->kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE) {
            ++out_receipt->executed_gdat_image_count;
            if (command->gdat_category == DM2_GDAT_CATEGORY_TITLE &&
                command->frame_owner == DM2_V1_FRAME_OWNER_STARTUP_TITLE) {
                out_receipt->title_gdat_asset_ready =
                    package.title_gdat_asset_ready;
                out_receipt->title_gdat_asset_w =
                    package.title_gdat_asset_w;
                out_receipt->title_gdat_asset_h =
                    package.title_gdat_asset_h;
                out_receipt->title_gdat_asset_stride =
                    package.title_gdat_asset_stride;
                if (snapshot->profile) {
                    uint8_t *pixels = NULL;
                    int w = 0;
                    int h = 0;
                    int stride = 0;
                    out_receipt->title_gdat_asset_required = 1;
                    if (dm2_v1_boot_gdat_image_asset_fetch(
                            (DM2_V1_BootProfile *)snapshot->profile,
                            command->gdat_category,
                            command->gdat_index,
                            command->gdat_field,
                            &pixels,
                            &w,
                            &h,
                            &stride) == 0 &&
                        pixels &&
                        w == command->rect.w &&
                        h == command->rect.h &&
                        stride >= w) {
                        out_receipt->title_gdat_asset_consumed = 1;
                        out_receipt->title_gdat_asset_ready = 1;
                        out_receipt->title_gdat_asset_w = w;
                        out_receipt->title_gdat_asset_h = h;
                        out_receipt->title_gdat_asset_stride = stride;
                    }
                    dm2_v1_boot_gdat_image_asset_free(pixels);
                }
            } else if (command->gdat_category == DM2_GDAT_CATEGORY_TITLE &&
                       command->frame_owner == DM2_V1_FRAME_OWNER_STARTUP_MENU) {
                ++out_receipt->menu_gdat_command_count;
            }
        } else if (command->kind == DM2_V1_STARTUP_DRAW_FILL_RECT ||
                   command->kind == DM2_V1_STARTUP_DRAW_OUTLINE_RECT) {
            ++out_receipt->executed_rect_count;
        } else if (command->kind == DM2_V1_STARTUP_DRAW_TEXT) {
            ++out_receipt->executed_text_count;
        }
    }

    out_receipt->fallback_title_blit_used = 0;
    out_receipt->packaged_draw_commands_consumed =
        out_receipt->draw_command_count > 0 &&
        out_receipt->executed_command_count ==
            out_receipt->draw_command_count;
    out_receipt->title_timing_receipt_consumed =
        out_receipt->title_animation_tick == 0 &&
        out_receipt->title_frame_duration_ticks == 0 &&
        out_receipt->title_frame_max == 0 &&
        out_receipt->title_next_frame_tick >
            out_receipt->title_animation_tick &&
        out_receipt->title_frame_remaining_ticks > 0;
    out_receipt->real_gdat_title_asset_receipt_breadth =
        out_receipt->title_gdat_command_count == 1 &&
        out_receipt->menu_gdat_command_count == 1 &&
        ((out_receipt->title_gdat_asset_required &&
          out_receipt->title_gdat_asset_consumed &&
          out_receipt->title_gdat_asset_w > 0 &&
          out_receipt->title_gdat_asset_h > 0 &&
          out_receipt->title_gdat_asset_stride >=
              out_receipt->title_gdat_asset_w) ||
         (!out_receipt->title_gdat_asset_required &&
          package.title_gdat_asset_ready ==
              out_receipt->title_gdat_asset_ready)) &&
        (!package.full_start_real_asset_ready ||
         (out_receipt->startup_title_menu_raw_gdat_receipt_consumed &&
          out_receipt->startup_title_raw_gdat_hash != 0u &&
          out_receipt->startup_title_raw_gdat_byte_count > 0u &&
          out_receipt->startup_menu_raw_gdat_hash != 0u &&
          out_receipt->startup_menu_raw_gdat_byte_count > 0u));
    out_receipt->menu_hud_startup_receipt_breadth =
        package.menu_capture_ready &&
        package.hud_handoff_capture_ready &&
        (!package.full_start_real_asset_ready ||
         (out_receipt->startup_hud_raw_gdat_receipt_consumed &&
          out_receipt->startup_title_menu_raw_gdat_receipt_consumed &&
          out_receipt->startup_title_menu_decoded_gdat_receipt_consumed)) &&
        out_receipt->suppress_game_hud;
    out_receipt->final_m11_draw_caller_ready =
        out_receipt->consume_host_frame_receipt &&
        out_receipt->execute_startup_draw_commands &&
        out_receipt->packaged_draw_commands_consumed &&
        out_receipt->title_timing_receipt_consumed &&
        out_receipt->real_gdat_title_asset_receipt_breadth &&
        out_receipt->menu_hud_startup_receipt_breadth &&
        out_receipt->extended_spells_definition_consumed &&
        !out_receipt->fallback_title_blit_used;
    out_receipt->final_m11_draw_caller_consumes_ownership =
        out_receipt->final_m11_draw_caller_ready;

    out_receipt->valid =
        out_receipt->packaged_full_start_valid &&
        out_receipt->host_frame_valid &&
        out_receipt->consume_host_frame_receipt &&
        out_receipt->execute_startup_draw_commands &&
        out_receipt->draw_command_count > 0 &&
        out_receipt->packaged_draw_commands_consumed &&
        out_receipt->title_gdat_command_count == 1 &&
        out_receipt->suppress_game_hud &&
        out_receipt->extended_spells_definition_consumed &&
        (!package.full_start_real_asset_ready ||
         (out_receipt->startup_hud_raw_gdat_receipt_consumed &&
          out_receipt->startup_title_menu_raw_gdat_receipt_consumed &&
          out_receipt->startup_title_menu_decoded_gdat_receipt_consumed)) &&
        !out_receipt->fallback_title_blit_used &&
        (!out_receipt->title_gdat_asset_required ||
         out_receipt->title_gdat_asset_consumed) &&
        out_receipt->final_m11_draw_caller_ready &&
        out_receipt->packaged_full_start_hash != 0u;
    return out_receipt->valid;
}

int dm2_v1_boot_startup_render_ownership_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupRenderOwnershipReceipt *out_receipt)
{
    DM2_V1_BootStartupViewModel view_model;

    dm2_v1_boot_startup_render_ownership_receipt_init(out_receipt);
    if (!snapshot || !out_receipt ||
        !dm2_v1_boot_startup_view_model_receipt_from_snapshot(
            snapshot,
            &view_model)) {
        return 0;
    }
    return dm2_v1_boot_startup_render_ownership_from_view_model(
        snapshot,
        &view_model,
        out_receipt);
}

int dm2_v1_boot_startup_render_ownership_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupRenderOwnershipReceipt *out_receipt)
{
    DM2_V1_BootRuntimeStartupSnapshot snapshot;
    DM2_V1_BootStartupViewModel view_model;

    dm2_v1_boot_startup_render_ownership_receipt_init(out_receipt);
    if (!out_receipt) {
        return 0;
    }
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.profile = profile;
    snapshot.startup_menu_active = startup_menu_active;
    snapshot.startup_save_root = startup_save_root;
    snapshot.resume_available = resume_available;
    snapshot.slot_mask = slot_mask;
    snapshot.selected_row = selected_row;
    if (!dm2_v1_boot_startup_view_model_receipt_from_snapshot_tick(
            &snapshot,
            title_animation_tick,
            &view_model)) {
        return 0;
    }
    return dm2_v1_boot_startup_render_ownership_from_view_model(
        &snapshot,
        &view_model,
        out_receipt);
}

void dm2_v1_boot_startup_real_visual_capture_receipt_init(
    DM2_V1_BootStartupRealVisualCaptureReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

static uint8_t dm2_v1_boot_startup_style_color(DM2_V1_StartupStyle style)
{
    switch (style) {
        case DM2_V1_STARTUP_STYLE_PANEL: return 0x08u;
        case DM2_V1_STARTUP_STYLE_BORDER: return 0x0fu;
        case DM2_V1_STARTUP_STYLE_TITLE: return 0x0eu;
        case DM2_V1_STARTUP_STYLE_SELECTED_FILL: return 0x04u;
        case DM2_V1_STARTUP_STYLE_SELECTED_TEXT: return 0x0fu;
        case DM2_V1_STARTUP_STYLE_TEXT:
        default: return 0x0cu;
    }
}

static void dm2_v1_boot_startup_composite_rect(uint8_t *frame,
                                               int width,
                                               int height,
                                               const DM2_V1_StartupDrawCommand *command)
{
    int x0;
    int y0;
    int x1;
    int y1;
    int x;
    int y;
    uint8_t color;

    if (!frame || !command || width <= 0 || height <= 0) {
        return;
    }
    x0 = command->rect.x < 0 ? 0 : command->rect.x;
    y0 = command->rect.y < 0 ? 0 : command->rect.y;
    x1 = command->rect.x + command->rect.w;
    y1 = command->rect.y + command->rect.h;
    if (x1 > width) x1 = width;
    if (y1 > height) y1 = height;
    if (x0 >= x1 || y0 >= y1) {
        return;
    }
    color = dm2_v1_boot_startup_style_color(command->style);
    if (command->kind == DM2_V1_STARTUP_DRAW_FILL_RECT) {
        for (y = y0; y < y1; ++y) {
            memset(frame + (size_t)y * (size_t)width + (size_t)x0,
                   color,
                   (size_t)(x1 - x0));
        }
        return;
    }
    for (x = x0; x < x1; ++x) {
        frame[(size_t)y0 * (size_t)width + (size_t)x] = color;
        frame[(size_t)(y1 - 1) * (size_t)width + (size_t)x] = color;
    }
    for (y = y0; y < y1; ++y) {
        frame[(size_t)y * (size_t)width + (size_t)x0] = color;
        frame[(size_t)y * (size_t)width + (size_t)(x1 - 1)] = color;
    }
}

static void dm2_v1_boot_startup_composite_text_zone(
    uint8_t *frame,
    int width,
    int height,
    const DM2_V1_StartupDrawCommand *command)
{
    int cx;
    int cy;
    size_t i;
    uint8_t color;

    if (!frame || !command || width <= 0 || height <= 0) {
        return;
    }
    color = dm2_v1_boot_startup_style_color(command->style);
    for (i = 0; command->text[i] != '\0'; ++i) {
        if (command->text[i] == ' ') {
            continue;
        }
        for (cy = 0; cy < 7; ++cy) {
            int y = command->y + cy;
            if (y < 0 || y >= height) {
                continue;
            }
            for (cx = 0; cx < 5; ++cx) {
                int x = command->x + (int)i * 6 + cx;
                if (x >= 0 && x < width) {
                    frame[(size_t)y * (size_t)width + (size_t)x] = color;
                }
            }
        }
    }
}

static int dm2_v1_boot_startup_composite_capture(
    DM2_V1_BootProfile *profile,
    const DM2_V1_StartupDrawCommand *commands,
    int command_count,
    DM2_V1_BootStartupRealVisualCaptureReceipt *out_receipt)
{
    uint8_t *frame;
    uint32_t pixel_hash = 0x32464346u;
    int i;
    int ready;
    int suppress_synthetic_menu_overlay;

    if (!profile || !commands || command_count <= 0 || !out_receipt) {
        return 0;
    }
    frame = (uint8_t *)calloc(320u * 200u, 1u);
    if (!frame) {
        return 0;
    }

    /* skproject/SKWIN/SkWinCore.cpp SHOW_MENU_SCREEN first consumes TITLE
     * GDAT fields 1 and 4. When dt07/4 provides the complete raw menu
     * screen, Firestaff must not redraw the old synthetic panel/text overlay
     * on top of it. */
    suppress_synthetic_menu_overlay =
        out_receipt->menu_raw_screen_route_ready &&
        out_receipt->menu_raw_screen_consumed;
    for (i = 0; i < command_count; ++i) {
        const DM2_V1_StartupDrawCommand *command = &commands[i];
        if (command->kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE) {
            uint8_t *pixels = NULL;
            int w = 0;
            int h = 0;
            int stride = 0;
            int consumed_raw_menu_screen = 0;
            if (suppress_synthetic_menu_overlay &&
                command->gdat_category == DM2_GDAT_CATEGORY_TITLE &&
                command->gdat_index == 0 &&
                command->gdat_field == DM2_GDAT_TITLE_MENU_SCREEN_FIELD &&
                profile->graphics_dat) {
                DM2_V1_BootGraphicsDat *gfx =
                    (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
                size_t raw_size = 0;
                const uint8_t *raw =
                    dm2_v1_asset_load_typed_sized(
                        &gfx->loader,
                        DM2_GDAT_CATEGORY_TITLE,
                        0,
                        DM2_GDAT_ENTRY_TYPE_RAW7,
                        DM2_GDAT_TITLE_MENU_SCREEN_FIELD,
                        &raw_size);
                if (raw && raw_size == 320u * 200u) {
                    memcpy(frame, raw, 320u * 200u);
                    ++out_receipt->composite_gdat_blit_count;
                    consumed_raw_menu_screen = 1;
                }
            }
            if (consumed_raw_menu_screen) {
                continue;
            }
            if (dm2_v1_boot_gdat_image_asset_fetch(profile,
                                                   command->gdat_category,
                                                   command->gdat_index,
                                                   command->gdat_field,
                                                   &pixels,
                                                   &w,
                                                   &h,
                                                   &stride) == 0 &&
                pixels && w > 0 && h > 0 && stride >= w) {
                int y;
                int copy_w = command->rect.w < w ? command->rect.w : w;
                int copy_h = command->rect.h < h ? command->rect.h : h;
                if (copy_w > 320 - command->rect.x) copy_w = 320 - command->rect.x;
                if (copy_h > 200 - command->rect.y) copy_h = 200 - command->rect.y;
                for (y = 0; y < copy_h; ++y) {
                    int dst_y = command->rect.y + y;
                    if (dst_y >= 0 && dst_y < 200 && command->rect.x >= 0 &&
                        command->rect.x < 320 && copy_w > 0) {
                        memcpy(frame + (size_t)dst_y * 320u +
                                   (size_t)command->rect.x,
                               pixels + (size_t)y * (size_t)stride,
                               (size_t)copy_w);
                    }
                }
                ++out_receipt->composite_gdat_blit_count;
            }
            dm2_v1_boot_gdat_image_asset_free(pixels);
        } else if (command->kind == DM2_V1_STARTUP_DRAW_FILL_RECT ||
                   command->kind == DM2_V1_STARTUP_DRAW_OUTLINE_RECT) {
            if (suppress_synthetic_menu_overlay) {
                ++out_receipt->synthetic_menu_overlay_command_count;
                out_receipt->synthetic_menu_overlay_suppressed = 1;
                continue;
            }
            dm2_v1_boot_startup_composite_rect(frame, 320, 200, command);
            ++out_receipt->composite_rect_count;
        } else if (command->kind == DM2_V1_STARTUP_DRAW_TEXT) {
            if (suppress_synthetic_menu_overlay) {
                ++out_receipt->synthetic_menu_overlay_command_count;
                out_receipt->synthetic_menu_overlay_suppressed = 1;
                continue;
            }
            dm2_v1_boot_startup_composite_text_zone(frame, 320, 200, command);
            ++out_receipt->composite_text_zone_count;
        }
    }

    for (i = 0; i < 320 * 200; ++i) {
        pixel_hash = dm2_v1_boot_packaged_capture_hash_step(
            pixel_hash,
            frame[i]);
    }
    out_receipt->composite_pixel_hash = pixel_hash;
    out_receipt->composite_pixel_count = 64000u;
    out_receipt->hud_suppressed_capture_ready =
        out_receipt->hud_handoff_capture_ready &&
        out_receipt->suppress_game_hud &&
        !out_receipt->present_first_hud_frame;
    out_receipt->real_menu_screen_no_synthetic_overlay_ready =
        out_receipt->composite_rect_count == 0 &&
        out_receipt->composite_text_zone_count == 0;
    ready =
        out_receipt->composite_gdat_blit_count == 2 &&
        out_receipt->real_menu_screen_no_synthetic_overlay_ready &&
        out_receipt->composite_pixel_hash != 0u &&
        out_receipt->composite_pixel_count == 64000u &&
        out_receipt->hud_suppressed_capture_ready;
    out_receipt->full_visual_composite_capture_ready = ready ? 1 : 0;
    free(frame);
    return ready;
}

static int dm2_v1_boot_startup_real_visual_breadth_probe(
    DM2_V1_BootProfile *profile,
    const DM2_V1_BootRuntimeStartupSnapshot *base_snapshot,
    DM2_V1_BootStartupRealVisualCaptureReceipt *out_receipt)
{
    static const int k_title_ticks[] = {0, 12, 42};
    static const int k_selected_rows[] = {0, 1, 2};
    uint32_t title_hashes[3];
    uint32_t menu_hashes[3];
    int title_hash_count = 0;
    int menu_hash_count = 0;
    int i;

    if (!profile || !base_snapshot || !out_receipt) {
        return 0;
    }

    /* fe7299.cpp presents one static TITLE/0 dt07/4 surface while the HUD is
     * suppressed. Capture that surface once; menu navigation remains input
     * state and must not generate Firestaff-owned pixels. */
    out_receipt->sampled_title_pixel_hash = 0x32545348u;
    for (i = 0; i < (int)(sizeof(k_title_ticks) / sizeof(k_title_ticks[0])); ++i) {
        DM2_V1_BootStartupViewModel sample_view;
        DM2_V1_BootStartupPackagedFullStartReceipt sample_package;
        DM2_V1_BootRuntimeStartupSnapshot sample_snapshot = *base_snapshot;
        const int tick = k_title_ticks[i];
        uint8_t *pixels = NULL;
        int w = 0;
        int h = 0;
        int stride = 0;
        uint32_t pixel_hash = 0x32544954u;
        int hash_i;
        int seen_hash = 0;

        if (!dm2_v1_boot_startup_view_model_receipt_from_snapshot_tick(
                &sample_snapshot,
                tick,
                &sample_view) ||
            !dm2_v1_boot_startup_packaged_full_start_receipt_from_host_view(
                &sample_view.host_view_receipt,
                &sample_package) ||
            !sample_package.exact_title_timing_ready ||
            sample_package.title_frame < 0 ||
            sample_package.title_frame > 30) {
            continue;
        }
        ++out_receipt->sampled_title_timing_capture_count;
        out_receipt->sampled_title_frame_mask |=
            1 << sample_package.title_frame;
        if (dm2_v1_boot_gdat_image_asset_fetch(
                profile,
                sample_package.title_gdat_category,
                sample_package.title_gdat_index,
                sample_package.title_gdat_field,
                &pixels,
                &w,
                &h,
                &stride) == 0 &&
            pixels &&
            w == 320 &&
            h == 200 &&
            stride >= w) {
            int y;
            ++out_receipt->sampled_title_pixel_capture_count;
            for (y = 0; y < h; ++y) {
                int x;
                const uint8_t *src = pixels + (size_t)y * (size_t)stride;
                for (x = 0; x < w; ++x) {
                    pixel_hash = dm2_v1_boot_packaged_capture_hash_step(
                        pixel_hash,
                        src[x]);
                }
            }
            out_receipt->sampled_title_pixel_hash =
                dm2_v1_boot_packaged_capture_hash_step(
                    out_receipt->sampled_title_pixel_hash,
                    pixel_hash);
            for (hash_i = 0; hash_i < title_hash_count; ++hash_i) {
                if (title_hashes[hash_i] == pixel_hash) {
                    seen_hash = 1;
                    break;
                }
            }
            if (!seen_hash && title_hash_count < 3) {
                title_hashes[title_hash_count++] = pixel_hash;
            }
        }
        dm2_v1_boot_gdat_image_asset_free(pixels);
    }
    out_receipt->sampled_title_unique_pixel_hash_count =
        title_hash_count;

    out_receipt->sampled_menu_composite_hash = 0x324d5348u;
    for (i = 0; i < (int)(sizeof(k_selected_rows) / sizeof(k_selected_rows[0])); ++i) {
        DM2_V1_BootStartupViewModel sample_view;
        DM2_V1_BootStartupPackagedFullStartReceipt sample_package;
        DM2_V1_BootStartupRealVisualCaptureReceipt sample_capture;
        DM2_V1_BootRuntimeStartupSnapshot sample_snapshot = *base_snapshot;
        int hash_i;
        int seen_hash = 0;
        sample_snapshot.selected_row = k_selected_rows[i];
        sample_snapshot.resume_available = 1;
        if (sample_snapshot.slot_mask == 0u) {
            sample_snapshot.slot_mask = 1u;
        }
        if (!dm2_v1_boot_startup_view_model_receipt_from_snapshot_tick(
                &sample_snapshot,
                out_receipt->title_animation_tick,
                &sample_view) ||
            !dm2_v1_boot_startup_packaged_full_start_receipt_from_host_view(
                &sample_view.host_view_receipt,
                &sample_package) ||
            sample_package.menu_row_count < 3 ||
            sample_package.selected_row != k_selected_rows[i] ||
            !sample_package.menu_capture_ready) {
            continue;
        }
        ++out_receipt->sampled_menu_selection_capture_count;
        out_receipt->sampled_menu_selection_mask |=
            1 << sample_package.selected_row;

        dm2_v1_boot_startup_real_visual_capture_receipt_init(&sample_capture);
        sample_capture.menu_row_count = sample_package.menu_row_count;
        sample_capture.hud_handoff_capture_ready =
            sample_package.hud_handoff_capture_ready;
        sample_capture.suppress_game_hud = 1;
        sample_capture.present_first_hud_frame = 0;
        sample_capture.menu_raw_screen_route_ready =
            dm2_v1_boot_startup_menu_raw_screen_receipt(
                profile,
                &sample_capture.menu_raw_screen_hash,
                &sample_capture.menu_raw_screen_byte_count);
        sample_capture.menu_raw_screen_consumed =
            sample_capture.menu_raw_screen_route_ready;
        if (dm2_v1_boot_startup_composite_capture(
                profile,
                sample_view.commands,
                sample_view.command_count,
                &sample_capture) &&
                sample_capture.composite_gdat_blit_count == 2 &&
            sample_capture.composite_pixel_hash != 0u) {
            ++out_receipt->sampled_menu_composite_capture_count;
            out_receipt->sampled_menu_composite_hash =
                dm2_v1_boot_packaged_capture_hash_step(
                    out_receipt->sampled_menu_composite_hash,
                    sample_capture.composite_pixel_hash);
            for (hash_i = 0; hash_i < menu_hash_count; ++hash_i) {
                if (menu_hashes[hash_i] ==
                    sample_capture.composite_pixel_hash) {
                    seen_hash = 1;
                    break;
                }
            }
            if (!seen_hash && menu_hash_count < 3) {
                menu_hashes[menu_hash_count++] =
                    sample_capture.composite_pixel_hash;
            }
        }
    }
    out_receipt->sampled_menu_unique_composite_hash_count =
        menu_hash_count;

    {
        DM2_V1_BootStartupHostViewReceipt runtime_view;
        dm2_v1_boot_startup_host_view_receipt_clear(&runtime_view);
        if (dm2_v1_boot_startup_host_view_receipt_from_runtime_state(
                profile,
                0,
                base_snapshot->startup_save_root,
                base_snapshot->resume_available,
                base_snapshot->slot_mask,
                base_snapshot->selected_row,
                0,
                &runtime_view) &&
            runtime_view.runtime_handoff_ready &&
            runtime_view.first_hud_frame_ready &&
            runtime_view.runtime_action_ready &&
            !runtime_view.draw_startup_menu &&
            runtime_view.status &&
            strcmp(runtime_view.status, "DM2 RUNTIME") == 0) {
            out_receipt->sampled_runtime_hud_handoff_capture_ready = 1;
        }
    }

    out_receipt->real_gdat_capture_breadth_ready =
        out_receipt->sampled_title_timing_capture_count >= 3 &&
        out_receipt->sampled_title_pixel_capture_count >= 3 &&
        out_receipt->sampled_title_unique_pixel_hash_count >= 1 &&
        out_receipt->sampled_title_pixel_hash != 0u &&
        out_receipt->sampled_menu_selection_capture_count >= 3 &&
        out_receipt->sampled_menu_composite_capture_count >= 3 &&
        /* SHOW_MENU_SCREEN is a source-owned static menu surface. Selection
         * remains input state; requiring Firestaff to generate three visual
         * variants would force a synthetic overlay. */
        out_receipt->sampled_menu_unique_composite_hash_count >= 1 &&
        out_receipt->sampled_menu_composite_hash != 0u &&
        (out_receipt->sampled_menu_selection_mask & 0x7) == 0x7 &&
        out_receipt->sampled_runtime_hud_handoff_capture_ready;
    return out_receipt->real_gdat_capture_breadth_ready;
}

static int dm2_v1_boot_startup_raw_gdat_hash(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint32_t seed,
    uint32_t *out_hash,
    uint32_t *out_count)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0;
    size_t i;
    uint32_t hash = seed;

    if (out_hash) *out_hash = 0u;
    if (out_count) *out_count = 0u;
    if (!profile || !profile->graphics_dat || !out_hash || !out_count) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_sized(&gfx->loader,
                                  category,
                                  index,
                                  field,
                                  &raw_size);
    if (!raw || raw_size == 0 || raw_size > UINT32_MAX) {
        return 0;
    }
    /* skproject/SKWIN/SkWinCore.cpp SHOW_MENU_SCREEN lines 55187-55196
     * first queries the raw TITLE GDAT entries, then presents/decodes them.
     * Hashing the raw GDAT payload alongside decoded pixels proves both
     * layers came from GRAPHICS.DAT. */
    for (i = 0; i < raw_size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    }
    *out_hash = hash;
    *out_count = (uint32_t)raw_size;
    return 1;
}

static int dm2_v1_boot_startup_typed_raw_gdat_hash(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int type,
    int field,
    uint32_t seed,
    uint32_t *out_hash,
    uint32_t *out_count)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0;
    size_t i;
    uint32_t hash = seed;

    if (out_hash) *out_hash = 0u;
    if (out_count) *out_count = 0u;
    if (!profile || !profile->graphics_dat || !out_hash || !out_count) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(&gfx->loader,
                                        category,
                                        index,
                                        type,
                                        field,
                                        &raw_size);
    if (!raw || raw_size == 0 || raw_size > UINT32_MAX) {
        return 0;
    }
    /* skproject/SKWIN QUERY_GDAT_ENTRY_DATA_PTR(cls1, cls2, cls3, cls4)
     * keeps typed GDAT payloads separate: dtImage is decoded as a bitmap,
     * while dtRaw7/dtRaw8/dtPalIRGB/dtImageOffset remain data records. */
    for (i = 0; i < raw_size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    }
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)type);
    *out_hash = hash;
    *out_count = (uint32_t)raw_size;
    return 1;
}

int dm2_v1_boot_wall_gfx_ornate_animation_field(
    DM2_V1_BootProfile *profile, uint8_t wall_gfx_index, uint32_t tick,
    uint32_t delta, uint8_t *out_field, uint32_t *out_receipt_hash)
{
    DM2_V1_BootGraphicsDat *gfx;
    uint16_t frame = 0u;
    uint32_t receipt = 0u;

    if (out_field) *out_field = 0u;
    if (out_receipt_hash) *out_receipt_hash = 0u;
    if (!profile || !profile->graphics_dat || !out_field ||
        !out_receipt_hash) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_asset_query_ornate_animation_frame(
            &gfx->loader, DM2_GDAT_CATEGORY_WALL_GFX, wall_gfx_index,
            tick, delta, &frame, &receipt) || frame > 63u) {
        return 0;
    }
    /* SUMMARIZE_STONE_ROOM packs frame in bits 10.. then the viewport's
     * WALL_GFX resolver uses ((packed >> 8) & 0xff) + 1. */
    *out_field = (uint8_t)(frame * 4u + 1u);
    *out_receipt_hash = dm2_v1_boot_packaged_capture_hash_step(
        receipt, *out_field);
    return *out_receipt_hash != 0u;
}

int dm2_v1_boot_gdat_raw_asset_proof(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint32_t seed,
    uint32_t *out_hash,
    uint32_t *out_byte_count)
{
    if (out_hash) *out_hash = 0u;
    if (out_byte_count) *out_byte_count = 0u;
    if (category < 0 || index < 0 || field < 0 || !out_hash ||
        !out_byte_count) {
        return 0;
    }
    /* The sized loader bounds the returned range to this GDAT entry.  Do not
     * let M11 consume decoded pixels until that original byte interval has
     * been independently accepted. */
    return dm2_v1_boot_startup_raw_gdat_hash(profile,
                                             category,
                                             index,
                                             field,
                                             seed,
                                             out_hash,
                                             out_byte_count) &&
           *out_hash != 0u && *out_byte_count > 0u;
}

int dm2_v1_boot_gdat_typed_raw_asset_proof(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int type,
    int field,
    uint32_t seed,
    uint32_t *out_hash,
    uint32_t *out_byte_count)
{
    if (out_hash) *out_hash = 0u;
    if (out_byte_count) *out_byte_count = 0u;
    if (category < 0 || index < 0 || type < 0 || field < 0 || !out_hash ||
        !out_byte_count) {
        return 0;
    }
    return dm2_v1_boot_startup_typed_raw_gdat_hash(profile,
                                                   category,
                                                   index,
                                                   type,
                                                   field,
                                                   seed,
                                                   out_hash,
                                                   out_byte_count) &&
           *out_hash != 0u && *out_byte_count > 0u;
}

const DM2_V1_AssetLoader *dm2_v1_boot_asset_loader(
    const DM2_V1_BootProfile *profile)
{
    DM2_V1_BootGraphicsDat *gfx;
    if (!profile || !profile->graphics_dat) return NULL;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return &gfx->loader;
}

static int dm2_v1_boot_runtime_raw_gdat_hash_add(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint32_t *io_hash,
    uint32_t *io_count)
{
    uint32_t entry_hash = 0u;
    uint32_t entry_count = 0u;
    if (!io_hash || !io_count) {
        return 0;
    }
    if (!dm2_v1_boot_startup_raw_gdat_hash(profile,
                                           category,
                                           index,
                                           field,
                                           0x32485552u,
                                           &entry_hash,
                                           &entry_count)) {
        return 0;
    }
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash, entry_hash);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash, entry_count);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)category);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)index);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)field);
    *io_count += entry_count;
    return 1;
}

static int dm2_v1_boot_runtime_typed_raw_gdat_hash_add(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int type,
    int field,
    uint32_t *io_hash,
    uint32_t *io_count)
{
    uint32_t entry_hash = 0u;
    uint32_t entry_count = 0u;
    if (!io_hash || !io_count ||
        !dm2_v1_boot_startup_typed_raw_gdat_hash(profile,
                                                 category,
                                                 index,
                                                 type,
                                                 field,
                                                 0x32485452u,
                                                 &entry_hash,
                                                 &entry_count)) {
        return 0;
    }
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash, entry_hash);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash, entry_count);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)category);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)index);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)type);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)field);
    *io_count += entry_count;
    return 1;
}

static int dm2_v1_boot_runtime_raw_gdat_hud_probe(
    DM2_V1_BootProfile *profile,
    int *out_portrait_count,
    uint32_t *out_portrait_hash,
    uint32_t *out_portrait_byte_count,
    uint32_t *out_core_hash,
    uint32_t *out_core_byte_count,
    int *out_interface_count)
{
    struct TypedEntry { int category; int index; int type; int field; };
    static const struct TypedEntry k_interface_entries[] = {
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_PAL_IRGB, DM2_GDAT_INTERFACE_PALETTE_FIELD },
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_PAL_16, DM2_GDAT_INTERFACE_PALETTE_FIELD },
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_INTERFACE_RAW_LAYOUT_TABLE },
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_INTERFACE_RAW_ACTION_TABLE },
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_RAW6, 0x00 }
    };
    int portrait_index;
    int portrait_count = 0;
    int interface_count = 0;
    uint32_t portrait_hash = 0x32485052u;
    uint32_t portrait_bytes = 0u;
    uint32_t core_hash = 0x32484352u;
    uint32_t core_bytes = 0u;

    if (out_portrait_count) *out_portrait_count = 0;
    if (out_portrait_hash) *out_portrait_hash = 0u;
    if (out_portrait_byte_count) *out_portrait_byte_count = 0u;
    if (out_core_hash) *out_core_hash = 0u;
    if (out_core_byte_count) *out_core_byte_count = 0u;
    if (out_interface_count) *out_interface_count = 0;
    if (!profile || !out_portrait_count || !out_portrait_hash ||
        !out_portrait_byte_count || !out_core_hash || !out_core_byte_count ||
        !out_interface_count) {
        return 0;
    }

    /* skproject/SKWIN/SkWinCore.cpp DRAW_CHAMPION_PICTURE lines
     * 12866-12880 draws the right HUD portraits from
     * GDAT_CATEGORY_CHAMPIONS; DISPLAY_RIGHT_PANEL_SQUAD_HANDS wires that
     * panel into startup/runtime handoff. */
    for (portrait_index = 0;
         portrait_index < DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT;
         ++portrait_index) {
        if (dm2_v1_boot_runtime_raw_gdat_hash_add(
                profile,
                DM2_GDAT_CATEGORY_CHAMPIONS,
                portrait_index,
                DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD,
                &portrait_hash,
                &portrait_bytes)) {
            ++portrait_count;
        }
    }
    /* skproject/SKWIN INIT and QUERY_GDAT_ENTRY_DATA_PTR keep these as
     * typed GDAT records, not as INTERFACE_GENERAL image fields. */
    for (int i = 0; i < (int)(sizeof(k_interface_entries) /
                              sizeof(k_interface_entries[0])); ++i) {
        if (dm2_v1_boot_runtime_typed_raw_gdat_hash_add(
                profile,
                k_interface_entries[i].category,
                k_interface_entries[i].index,
                k_interface_entries[i].type,
                k_interface_entries[i].field,
                &core_hash,
                &core_bytes)) {
            ++interface_count;
        }
    }
    if (interface_count < 4 ||
        !dm2_v1_boot_runtime_raw_gdat_hash_add(profile,
                                               DM2_GDAT_CATEGORY_GRAPHICSSET,
                                               0,
                                               DM2_GDAT_GFXSET_FLOOR,
                                               &core_hash,
                                               &core_bytes) ||
        !dm2_v1_boot_runtime_raw_gdat_hash_add(profile,
                                               DM2_GDAT_CATEGORY_GRAPHICSSET,
                                               0,
                                               DM2_GDAT_GFXSET_CEIL,
                                               &core_hash,
                                               &core_bytes)) {
        return 0;
    }

    *out_portrait_count = portrait_count;
    *out_portrait_hash = portrait_hash;
    *out_portrait_byte_count = portrait_bytes;
    *out_core_hash = core_hash;
    *out_core_byte_count = core_bytes;
    *out_interface_count = interface_count;
    return portrait_count >= 4 &&
           portrait_hash != 0u &&
           portrait_bytes > 0u &&
           core_hash != 0u &&
           core_bytes > 0u;
}

static int dm2_v1_boot_runtime_decoded_gdat_hash_add(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint32_t *io_hash,
    uint32_t *io_pixel_count)
{
    uint8_t *pixels = NULL;
    int w = 0;
    int h = 0;
    int stride = 0;
    uint32_t hash;
    int y;

    if (!io_hash || !io_pixel_count ||
        dm2_v1_boot_gdat_image_asset_fetch(profile,
                                           category,
                                           index,
                                           field,
                                           &pixels,
                                           &w,
                                           &h,
                                           &stride) != 0 ||
        !pixels || w <= 0 || h <= 0 || stride < w) {
        dm2_v1_boot_gdat_image_asset_free(pixels);
        return 0;
    }
    hash = *io_hash;
    for (y = 0; y < h; ++y) {
        const uint8_t *src = pixels + (size_t)y * (size_t)stride;
        int x;
        for (x = 0; x < w; ++x) {
            hash = dm2_v1_boot_packaged_capture_hash_step(hash, src[x]);
        }
    }
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                      (uint32_t)category);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)index);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)field);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)w);
    *io_hash = dm2_v1_boot_packaged_capture_hash_step(*io_hash,
                                                      (uint32_t)h);
    *io_pixel_count += (uint32_t)(w * h);
    dm2_v1_boot_gdat_image_asset_free(pixels);
    return 1;
}

static int dm2_v1_boot_runtime_map_chip_category_hash_add(
    DM2_V1_BootProfile *profile,
    int category,
    int max_materialized,
    uint32_t *io_raw_hash,
    uint32_t *io_raw_byte_count,
    uint32_t *io_decoded_hash,
    uint32_t *io_decoded_pixel_count)
{
    DM2_V1_BootGraphicsDat *gfx;
    int entry_count;
    int materialized = 0;

    if (!profile || !profile->graphics_dat || !io_raw_hash ||
        !io_raw_byte_count || !io_decoded_hash || !io_decoded_pixel_count ||
        max_materialized <= 0) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    entry_count = dm2_v1_asset_category_entry_count(&gfx->loader, category);
    if (entry_count <= 0) return 0;

    for (int index = 0; index < entry_count && materialized < max_materialized;
         ++index) {
        uint32_t raw_hash = *io_raw_hash;
        uint32_t raw_bytes = *io_raw_byte_count;
        uint32_t decoded_hash = *io_decoded_hash;
        uint32_t decoded_pixels = *io_decoded_pixel_count;
        if (dm2_v1_boot_runtime_raw_gdat_hash_add(
                profile,
                category,
                index,
                DM2_GDAT_IMG_MAP_CHIP,
                &raw_hash,
                &raw_bytes) &&
            dm2_v1_boot_runtime_decoded_gdat_hash_add(
                profile,
                category,
                index,
                DM2_GDAT_IMG_MAP_CHIP,
                &decoded_hash,
                &decoded_pixels)) {
            *io_raw_hash = raw_hash;
            *io_raw_byte_count = raw_bytes;
            *io_decoded_hash = decoded_hash;
            *io_decoded_pixel_count = decoded_pixels;
            ++materialized;
        }
    }
    return materialized;
}

static int dm2_v1_boot_runtime_graphicsset_word_values_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    uint32_t *out_hash,
    uint32_t *out_present_mask,
    uint32_t *out_query_count,
    uint32_t *out_scene_flags,
    uint32_t *out_scene_colorkey,
    uint32_t *out_ambient_light,
    uint32_t *out_highest_light_level,
    uint32_t *out_void_random_fall,
    uint32_t *out_animated_floor,
    uint32_t *out_scene_rain,
    uint32_t *out_misty_map,
    uint32_t *out_thunder_position,
    uint32_t *out_ambient_darkness)
{
    static const uint8_t k_fields[] = {
        DM2_GDAT_GFXSET_SCENE_COLORKEY,
        DM2_GDAT_GFXSET_SCENE_FLAGS,
        DM2_GDAT_GFXSET_SCENE_RAIN,
        DM2_GDAT_GFXSET_AMBIANT_LIGHT,
        DM2_GDAT_GFXSET_HIGHEST_LIGHT_LEVEL,
        DM2_GDAT_GFXSET_MISTY_MAP,
        DM2_GDAT_GFXSET_VOID_RANDOM_FALL,
        DM2_GDAT_GFXSET_ANIMATED_FLOOR,
        DM2_GDAT_GFXSET_THUNDER_POSITION,
        DM2_GDAT_GFXSET_AMBIANT_DARKNESS,
        DM2_GDAT_GFXSET_TRIM_WALL_D1,
        DM2_GDAT_GFXSET_TRIM_WALL_D2
    };
    DM2_V1_BootGraphicsDat *gfx;
    uint32_t best_hash = 0u;
    uint32_t best_mask = 0u;
    uint32_t best_count = 0u;
    uint32_t best_scene_flags = 0u;
    uint32_t best_scene_colorkey = 0u;
    uint32_t best_ambient_light = 0u;
    uint32_t best_highest_light_level = 0u;
    uint32_t best_void_random_fall = 0u;
    uint32_t best_animated_floor = 0u;
    uint32_t best_scene_rain = 0u;
    uint32_t best_misty_map = 0u;
    uint32_t best_thunder_position = 0u;
    uint32_t best_ambient_darkness = 0u;
    int best_ready = 0;

    if (out_hash) *out_hash = 0u;
    if (out_present_mask) *out_present_mask = 0u;
    if (out_query_count) *out_query_count = 0u;
    if (out_scene_flags) *out_scene_flags = 0u;
    if (out_scene_colorkey) *out_scene_colorkey = 0u;
    if (out_ambient_light) *out_ambient_light = 0u;
    if (out_highest_light_level) *out_highest_light_level = 0u;
    if (out_void_random_fall) *out_void_random_fall = 0u;
    if (out_animated_floor) *out_animated_floor = 0u;
    if (out_scene_rain) *out_scene_rain = 0u;
    if (out_misty_map) *out_misty_map = 0u;
    if (out_thunder_position) *out_thunder_position = 0u;
    if (out_ambient_darkness) *out_ambient_darkness = 0u;
    if (!profile || !profile->graphics_dat || !out_hash ||
        !out_present_mask || !out_query_count || !out_scene_flags ||
        !out_scene_colorkey || !out_ambient_light ||
        !out_highest_light_level || !out_void_random_fall ||
        !out_animated_floor || !out_scene_rain || !out_misty_map ||
        !out_thunder_position || !out_ambient_darkness ||
        graphicsset_index < 0) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;

    /* SKWIN queries the active map graphics-set directly.  A control word
     * from another set is not a valid render handoff for this map. */
    for (int attempt = 0; attempt < 1; ++attempt) {
        int candidate = graphicsset_index;
        uint32_t hash = 0x32475756u;
        uint32_t mask = 0u;
        uint32_t count = 0u;
        uint32_t scene_flags = 0u;
        uint32_t scene_colorkey = 0u;
        uint32_t ambient_light = 0u;
        uint32_t highest_light_level = 0u;
        uint32_t void_random_fall = 0u;
        uint32_t animated_floor = 0u;
        uint32_t scene_rain = 0u;
        uint32_t misty_map = 0u;
        uint32_t thunder_position = 0u;
        uint32_t ambient_darkness = 0u;
        int ready;

        for (uint32_t i = 0u;
             i < (uint32_t)(sizeof(k_fields) / sizeof(k_fields[0])); ++i) {
            uint16_t value = 0u;
            if (!dm2_v1_asset_load_word_value(&gfx->loader,
                                              DM2_GDAT_CATEGORY_GRAPHICSSET,
                                              candidate,
                                              k_fields[i],
                                              &value)) {
                continue;
            }
            mask |= 1u << i;
            ++count;
            hash = dm2_v1_boot_packaged_capture_hash_step(hash, k_fields[i]);
            hash = dm2_v1_boot_packaged_capture_hash_step(hash, value);
            if (k_fields[i] == DM2_GDAT_GFXSET_SCENE_COLORKEY) {
                scene_colorkey = value;
            } else if (k_fields[i] == DM2_GDAT_GFXSET_SCENE_FLAGS) {
                scene_flags = value;
            } else if (k_fields[i] == DM2_GDAT_GFXSET_AMBIANT_LIGHT) {
                ambient_light = value;
            } else if (k_fields[i] == DM2_GDAT_GFXSET_HIGHEST_LIGHT_LEVEL) {
                highest_light_level = value;
            } else if (k_fields[i] == DM2_GDAT_GFXSET_VOID_RANDOM_FALL) {
                void_random_fall = value;
            } else if (k_fields[i] == DM2_GDAT_GFXSET_ANIMATED_FLOOR) {
                animated_floor = value;
            } else if (k_fields[i] == DM2_GDAT_GFXSET_SCENE_RAIN) {
                scene_rain = value;
            } else if (k_fields[i] == DM2_GDAT_GFXSET_MISTY_MAP) {
                misty_map = value;
            } else if (k_fields[i] == DM2_GDAT_GFXSET_THUNDER_POSITION) {
                thunder_position = value;
            } else if (k_fields[i] == DM2_GDAT_GFXSET_AMBIANT_DARKNESS) {
                ambient_darkness = value;
            }
        }
        hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                      (uint32_t)candidate);
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, mask);
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, count);
        /* SKProject map setup admits the selected GRAPHICSSET after its
         * SCENE_COLORKEY and SCENE_FLAGS reads.  The later light and weather
         * words are separate consumers: requiring them here would let an
         * unproven subsystem block an otherwise source-owned dungeon surface.
         * Keep their successfully read values in this receipt for those
         * consumers, but do not promote them to map-scene prerequisites. */
        ready = (mask & ((1u << 0) | (1u << 1))) ==
                ((1u << 0) | (1u << 1));
        if (ready || count > best_count) {
            best_hash = hash;
            best_mask = mask;
            best_count = count;
            best_scene_flags = scene_flags;
            best_scene_colorkey = scene_colorkey;
            best_ambient_light = ambient_light;
            best_highest_light_level = highest_light_level;
            best_void_random_fall = void_random_fall;
            best_animated_floor = animated_floor;
            best_scene_rain = scene_rain;
            best_misty_map = misty_map;
            best_thunder_position = thunder_position;
            best_ambient_darkness = ambient_darkness;
            best_ready = ready;
        }
        if (ready) {
            break;
        }
    }
    *out_hash = best_hash;
    *out_present_mask = best_mask;
    *out_query_count = best_count;
    *out_scene_flags = best_scene_flags;
    *out_scene_colorkey = best_scene_colorkey;
    *out_ambient_light = best_ambient_light;
    *out_highest_light_level = best_highest_light_level;
    *out_void_random_fall = best_void_random_fall;
    *out_animated_floor = best_animated_floor;
    *out_scene_rain = best_scene_rain;
    *out_misty_map = best_misty_map;
    *out_thunder_position = best_thunder_position;
    *out_ambient_darkness = best_ambient_darkness;
    return best_ready;
}

int dm2_v1_boot_graphicsset_scene_control(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    uint32_t *out_hash,
    uint32_t *out_present_mask,
    uint32_t *out_query_count,
    uint32_t *out_scene_flags,
    uint32_t *out_scene_colorkey,
    uint32_t *out_ambient_light,
    uint32_t *out_highest_light_level,
    uint32_t *out_void_random_fall,
    uint32_t *out_animated_floor,
    uint32_t *out_scene_rain,
    uint32_t *out_misty_map,
    uint32_t *out_thunder_position,
    uint32_t *out_ambient_darkness)
{
    return dm2_v1_boot_runtime_graphicsset_word_values_receipt(
        profile,
        graphicsset_index,
        out_hash,
        out_present_mask,
        out_query_count,
        out_scene_flags,
        out_scene_colorkey,
        out_ambient_light,
        out_highest_light_level,
        out_void_random_fall,
        out_animated_floor,
        out_scene_rain,
        out_misty_map,
        out_thunder_position,
        out_ambient_darkness);
}

int dm2_v1_boot_gdat_scene_m11_command_plan(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    DM2_V1_GdatSceneM11CommandPlan *out_plan)
{
    DM2_V1_BootGraphicsDat *gfx;

    if (out_plan) memset(out_plan, 0, sizeof(*out_plan));
    if (!profile || !profile->graphics_dat || !out_plan ||
        graphicsset_index < 0 || graphicsset_index > 0xff) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_gdat_scene_m11_command_plan_build(
        &gfx->loader, (uint8_t)graphicsset_index, out_plan);
}

int dm2_v1_boot_gdat_wall_m11_command_plan(
    DM2_V1_BootProfile *profile, int graphicsset_index,
    DM2_V1_GdatWallM11CommandPlan *out_plan)
{
    DM2_V1_BootGraphicsDat *gfx;
    if (out_plan) memset(out_plan, 0, sizeof(*out_plan));
    if (!profile || !profile->graphics_dat || !out_plan ||
        graphicsset_index < 0 || graphicsset_index > 0xff) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_gdat_wall_m11_command_plan_build(
        &gfx->loader, (uint8_t)graphicsset_index, out_plan);
}

int dm2_v1_boot_gdat_door_overlay_m11_command_plan(
    DM2_V1_BootProfile *profile, const DM2_V1_DoorRenderPlan *door_plan,
    DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan)
{
    DM2_V1_BootGraphicsDat *gfx;
    if (out_plan) memset(out_plan, 0, sizeof(*out_plan));
    if (!profile || !profile->graphics_dat || !door_plan || !out_plan) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_gdat_door_overlay_m11_command_plan_build(
        &gfx->loader, door_plan, out_plan);
}

int dm2_v1_boot_gdat_door_overlay_apply_light_palette(
    DM2_V1_BootProfile *profile,
    uint8_t c_light_parameter,
    uint32_t c_light_receipt_hash,
    DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    DM2_V1_GdatDoorOverlayM11CommandPlan candidate;
    DM2_V1_InterfaceActionTable table;
    int table_loaded = 0;
    int transformed = 0;

    if (!profile || !profile->graphics_dat || !plan || !plan->valid ||
        !plan->command_hash || !c_light_receipt_hash ||
        c_light_parameter > 64u) {
        return 0;
    }
    candidate = *plan;
    for (uint8_t i = 0u; i < candidate.command_count; ++i) {
        DM2_V1_GdatDoorOverlayM11Command *command = &candidate.commands[i];
        uint8_t darkness;
        uint32_t hash = 2166136261u;

        if (command->kind != DM2_V1_GDAT_DOOR_PANEL ||
            command->light_palette == 0u) {
            continue;
        }
        if (!dm2_v1_gdat_door_light_palette_darkness(
                c_light_parameter, command->light_palette, &darkness) ||
            (!table_loaded &&
             !dm2_v1_boot_interface_action_table(profile, &table)) ||
            !dm2_v1_interface_action_table_remap_palette(
                &table, command->palette16, 16u, darkness,
                command->color_key, -1)) {
            return 0;
        }
        table_loaded = 1;
        for (size_t p = 0u; p < sizeof(command->palette16); ++p) {
            hash = dm2_v1_boot_packaged_capture_hash_step(
                hash, command->palette16[p]);
        }
        command->palette_hash = hash ? hash : 1u;
        hash = dm2_v1_boot_packaged_capture_hash_step(
            2166136261u, c_light_receipt_hash);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, command->light_palette);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, c_light_parameter);
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, darkness);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, command->palette_hash);
        command->palette_darkness = darkness;
        command->palette_light_receipt_hash = c_light_receipt_hash;
        command->palette_transform_hash = hash ? hash : 1u;
        command->selection_hash = dm2_v1_boot_packaged_capture_hash_step(
            command->selection_hash, command->palette_transform_hash);
        ++transformed;
    }
    if (transformed > 0 &&
        !dm2_v1_gdat_door_overlay_m11_command_plan_refresh_hash(&candidate)) {
        return 0;
    }
    *plan = candidate;
    return 1;
}

static uint32_t dm2_v1_boot_hash_u8_span(const uint8_t *bytes, size_t size);

int dm2_v1_boot_gdat_scene_m11_apply_light_palette(
    DM2_V1_BootProfile *profile,
    int movement_active,
    uint8_t c_light_parameter,
    uint32_t c_light_receipt_hash,
    DM2_V1_GdatSceneM11CommandPlan *plan)
{
    DM2_V1_GdatSceneM11CommandPlan candidate;
    DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_InterfaceActionTable table;
    int table_loaded = 0;

    if (!profile || !profile->graphics_dat || !plan || !plan->valid ||
        plan->command_hash == 0u || !c_light_receipt_hash ||
        c_light_parameter > 64u) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    candidate = *plan;
    for (size_t i = 0u; i < 2u; ++i) {
        DM2_V1_GdatSceneM11Command *command = &candidate.commands[i];
        const uint8_t *translation;
        size_t translated_size = 0u;
        uint8_t translation_field;
        uint8_t darkness;
        uint32_t hash = 2166136261u;

        if (!dm2_v1_gdat_scene_m11_plane_translation_field(
                command->field, movement_active, &translation_field)) {
            return 0;
        }
        translation = dm2_v1_asset_load_typed_sized(
            &gfx->loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
            candidate.graphicsset, DM2_GDAT_ENTRY_TYPE_RAW7,
            translation_field, &translated_size);
        if (!dm2_v1_gdat_scene_m11_plane_palette_darkness(
                command->field, c_light_parameter, &darkness)) {
            return 0;
        }
        if (translation &&
            !dm2_v1_gdat_scene_m11_translate_palette(
                command->palette16, sizeof(command->palette16), translation,
                translated_size, &command->palette_translation_hash)) {
            return 0;
        }
        if (!table_loaded &&
            !dm2_v1_boot_interface_action_table(profile, &table)) {
            return 0;
        }
        if (!dm2_v1_interface_action_table_remap_palette(
                &table, command->palette16, sizeof(command->palette16),
                darkness, candidate.scene_colorkey, -1)) {
            return 0;
        }
        command->palette_translation_field = translation_field;
        table_loaded = 1;
        /* The viewport recomputes this hash with FNV-1a over the 16-byte
         * local palette, so match that exactly rather than the boot's
         * packaged-capture step. */
        {
            uint32_t pal_hash = 2166136261u;
            for (size_t p = 0u; p < sizeof(command->palette16); ++p) {
                pal_hash ^= command->palette16[p];
                pal_hash *= 16777619u;
            }
            command->palette_hash = pal_hash ? pal_hash : 1u;
        }
        hash = dm2_v1_boot_packaged_capture_hash_step(
            2166136261u, c_light_receipt_hash);
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, command->field);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, command->palette_translation_field);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, command->palette_translation_hash);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, c_light_parameter);
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, darkness);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, command->palette_hash);
        command->palette_darkness = darkness;
        command->palette_light_receipt_hash = c_light_receipt_hash;
        command->palette_transform_hash = hash ? hash : 1u;
    }
    if (!dm2_v1_gdat_scene_m11_command_plan_refresh_draw_order(&candidate)) {
        return 0;
    }
    *plan = candidate;
    return 1;
}

int dm2_v1_boot_gdat_hud_m11_command_plan(
    DM2_V1_BootProfile *profile,
    const DM2_V1_HudPartyState *party,
    DM2_V1_GdatHudM11CommandPlan *out_plan)
{
    DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_InterfaceRect source_portraits[4];
    DM2_V1_ViewportRect portrait_destinations[4];
    uint32_t table_hash = 0u;
    int slot;

    if (out_plan) memset(out_plan, 0, sizeof(*out_plan));
    if (!profile || !profile->graphics_dat || !party || !out_plan) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_gdat_hud_m11_command_plan_build_for_party(
            &gfx->loader, party, out_plan) ||
        !dm2_v1_boot_interface_hud_portrait_destinations(
            profile, source_portraits, &table_hash) || table_hash == 0u) {
        dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
        return 0;
    }
    for (slot = 0; slot < 4; ++slot) {
        if (source_portraits[slot].x < 0 || source_portraits[slot].y < 0 ||
            source_portraits[slot].w <= 0 || source_portraits[slot].h <= 0) {
            dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
            return 0;
        }
        /* SKProject _098d_1208 uses the 640-wide dt04 coordinates; M11 is
         * its matching 320-wide indexed surface. */
        portrait_destinations[slot] = (DM2_V1_ViewportRect){
            source_portraits[slot].x / 2, source_portraits[slot].y / 2,
            source_portraits[slot].w / 2, source_portraits[slot].h / 2 };
    }
    return dm2_v1_gdat_hud_m11_command_plan_bind_portrait_destinations(
        out_plan, party, portrait_destinations, table_hash);
}

int dm2_v1_boot_gdat_hud_static_m11_command_plan(
    DM2_V1_BootProfile *profile,
    int is_outdoor,
    DM2_V1_GdatHudM11CommandPlan *out_plan)
{
    DM2_V1_BootGraphicsDat *gfx;

    if (out_plan) memset(out_plan, 0, sizeof(*out_plan));
    if (!profile || !profile->graphics_dat || !out_plan) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_gdat_hud_m11_command_plan_build(
        &gfx->loader, is_outdoor, out_plan);
}

static uint32_t dm2_v1_boot_hash_u8_span(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes || size == 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, bytes[i]);
    }
    return hash ? hash : 1u;
}

static int dm2_v1_boot_startup_image_receipt(
    DM2_V1_BootGraphicsDat *gfx,
    int field,
    int *out_width,
    int *out_height,
    DM2_ImageFormat *out_format,
    uint32_t *out_raw_hash,
    uint32_t *out_pixel_hash)
{
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;

    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (out_format) *out_format = DM2_IMG_FMT_UNKNOWN;
    if (out_raw_hash) *out_raw_hash = 0u;
    if (out_pixel_hash) *out_pixel_hash = 0u;
    if (!gfx || !out_width || !out_height || !out_format ||
        !out_raw_hash || !out_pixel_hash) {
        return 0;
    }
    raw = dm2_v1_asset_load_sized(&gfx->loader, DM2_GDAT_CATEGORY_TITLE,
                                  0, field, &raw_size);
    pixels = dm2_v1_asset_load_image_field(&gfx->loader,
                                           DM2_GDAT_CATEGORY_TITLE,
                                           0, field,
                                           &width, &height, &format);
    if (!raw || raw_size == 0u || !pixels || width != 320 || height != 200 ||
        format == DM2_IMG_FMT_UNKNOWN) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    *out_width = width;
    *out_height = height;
    *out_format = format;
    *out_raw_hash = dm2_v1_boot_hash_u8_span(raw, raw_size);
    *out_pixel_hash = dm2_v1_boot_hash_u8_span(
        pixels, (size_t)width * (size_t)height);
    dm2_v1_asset_free_pixels(pixels);
    return *out_raw_hash != 0u && *out_pixel_hash != 0u;
}

int dm2_v1_boot_startup_menu_hud_gdat_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_BootStartupMenuHudGdatReceipt *out_receipt)
{
    DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_StartupMenuPointerLayout layout;
    DM2_V1_StartupMenuPointerHitReceipt new_hit;
    DM2_V1_StartupMenuPointerHitReceipt resume_hit;
    DM2_V1_InterfacePalette palette;
    DM2_V1_GdatHudM11CommandPlan hud;
    uint32_t hash = 2166136261u;
    int i;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !out_receipt || !profile->graphics_dat) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    out_receipt->graphics_dat_ready = 1;

    out_receipt->title_image_ready = dm2_v1_boot_startup_image_receipt(
        gfx, 1, &out_receipt->title_width, &out_receipt->title_height,
        &out_receipt->title_format, &out_receipt->title_raw_hash,
        &out_receipt->title_pixel_hash);
    out_receipt->menu_image_ready = dm2_v1_boot_startup_image_receipt(
        gfx, 4, &out_receipt->menu_width, &out_receipt->menu_height,
        &out_receipt->menu_format, &out_receipt->menu_raw_hash,
        &out_receipt->menu_pixel_hash);

    memset(&layout, 0, sizeof(layout));
    out_receipt->pointer_layout_ready =
        dm2_v1_boot_startup_menu_pointer_layout(profile, &layout) &&
        layout.valid && layout.table_hash != 0u &&
        layout.new_game.w > 0 && layout.new_game.h > 0 &&
        layout.resume_game.w > 0 && layout.resume_game.h > 0;
    out_receipt->pointer_table_hash = layout.table_hash;
    memset(&new_hit, 0, sizeof(new_hit));
    memset(&resume_hit, 0, sizeof(resume_hit));
    if (out_receipt->pointer_layout_ready) {
        out_receipt->new_game_click_ready =
            dm2_v1_boot_startup_menu_pointer_hit_from_layout(
                &layout,
                layout.new_game.x + layout.new_game.w / 2,
                layout.new_game.y + layout.new_game.h / 2,
                &new_hit) &&
            new_hit.target == DM2_V1_STARTUP_POINTER_TARGET_NEW_GAME;
        out_receipt->resume_click_surface_ready =
            dm2_v1_boot_startup_menu_pointer_hit_from_layout(
                &layout,
                layout.resume_game.x + layout.resume_game.w / 2,
                layout.resume_game.y + layout.resume_game.h / 2,
                &resume_hit) &&
            resume_hit.target ==
                DM2_V1_STARTUP_POINTER_TARGET_RESUME_GAME;
    }

    memset(&palette, 0, sizeof(palette));
    out_receipt->interface_palette_ready =
        dm2_v1_boot_interface_palette(profile, &palette) &&
        palette.hash != 0u;
    out_receipt->interface_palette_hash = palette.hash;

    memset(&hud, 0, sizeof(hud));
    out_receipt->hud_static_plan_ready =
        dm2_v1_boot_gdat_hud_static_m11_command_plan(profile, 0, &hud) &&
        hud.valid && hud.command_count == 9 && hud.command_hash != 0u;
    out_receipt->hud_static_command_count = hud.command_count;
    out_receipt->hud_static_plan_hash = hud.command_hash;
    out_receipt->hud_palette_ready = out_receipt->hud_static_plan_ready;
    for (i = 0; i < hud.command_count; ++i) {
        if (hud.commands[i].palette_hash == 0u) {
            out_receipt->hud_palette_ready = 0;
            break;
        }
    }
    dm2_v1_gdat_hud_m11_command_plan_free(&hud);

    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->title_raw_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->title_pixel_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_raw_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_pixel_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->pointer_table_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->interface_palette_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->hud_static_plan_hash);
    out_receipt->receipt_hash = hash ? hash : 1u;
    out_receipt->valid =
        out_receipt->graphics_dat_ready &&
        out_receipt->title_image_ready &&
        out_receipt->menu_image_ready &&
        out_receipt->pointer_layout_ready &&
        out_receipt->new_game_click_ready &&
        out_receipt->resume_click_surface_ready &&
        out_receipt->interface_palette_ready &&
        out_receipt->hud_static_plan_ready &&
        out_receipt->hud_palette_ready &&
        out_receipt->receipt_hash != 0u;
    if (!out_receipt->valid) {
        return 0;
    }
    return 1;
}

int dm2_v1_boot_weather_gdat_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    DM2_V1_WeatherGdatReceipt *out_receipt)
{
    DM2_V1_BootGraphicsDat *gfx;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->graphics_dat || !out_receipt ||
        graphicsset_index < 0 || graphicsset_index > 0xff) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_weather_gdat_receipt(
        &gfx->loader, (uint8_t)graphicsset_index, out_receipt);
}

int dm2_v1_boot_weather_gdat_destination_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    DM2_V1_BootWeatherDestinationReceipt *out_receipt)
{
    DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_WeatherGdatReceipt weather;
    const uint8_t *rect_table;
    size_t rect_table_size = 0u;
    unsigned int i;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->graphics_dat || graphicsset_index < 0 ||
        graphicsset_index > 0xff ||
        !dm2_v1_boot_weather_gdat_receipt(profile, graphicsset_index,
                                           &weather) ||
        !weather.valid) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    rect_table = dm2_v1_asset_load_typed_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &rect_table_size);
    if (!rect_table || rect_table_size == 0u) return 0;
    for (i = 0u; i < rect_table_size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, rect_table[i]);
    }
    for (i = 0u; i < 6u; ++i) {
        if (!dm2_v1_weather_gdat_destination_clip(
                rect_table, rect_table_size, &weather.commands[i],
                &out_receipt->clips[i])) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
        out_receipt->destination_mask |= (1u << i);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, weather.commands[i].material_hash);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, out_receipt->clips[i].table_hash);
    }
    out_receipt->graphicsset = (uint8_t)graphicsset_index;
    out_receipt->rect_table_hash = out_receipt->clips[0].table_hash;
    out_receipt->receipt_hash = hash;
    out_receipt->valid = out_receipt->rect_table_hash != 0u && hash != 0u;
    return out_receipt->valid;
}

int dm2_v1_boot_weather_renderer_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    const DM2_V1_WeatherRestoredStateReceipt *restored_state,
    const DM2_V1_DistantEnvironmentReceipt *slots,
    unsigned int slot_count,
    const DM2_V1_WeatherDrawContext *context,
    DM2_V1_WeatherRendererReceipt *out_receipt)
{
    DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_WeatherGdatReceipt weather;
    const uint8_t *rect_table;
    size_t rect_table_size = 0u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->graphics_dat || !restored_state || !slots ||
        slot_count == 0u || slot_count > DM2_V1_WEATHER_MAX_SLOTS ||
        !context || !out_receipt ||
        graphicsset_index < 0 || graphicsset_index > 0xff ||
        !dm2_v1_boot_weather_gdat_receipt(profile, graphicsset_index,
                                           &weather) || !weather.valid) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    rect_table = dm2_v1_asset_load_typed_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &rect_table_size);
    if (!rect_table || rect_table_size == 0u) return 0;
    return dm2_v1_weather_gdat_renderer_receipt(
        restored_state, &weather, slots, slot_count, context,
        rect_table, rect_table_size, out_receipt);
}

int dm2_v1_boot_dialogue_gdat_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    uint8_t shell_field,
    DM2_V1_DialogueGdatReceipt *out_receipt)
{
    DM2_V1_BootGraphicsDat *gfx;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->graphics_dat || !out_receipt ||
        graphicsset_index < 0 || graphicsset_index > 0xff) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_dialogue_gdat_receipt(&gfx->loader,
                                         (uint8_t)graphicsset_index,
                                         shell_field, out_receipt);
}

static int dm2_v1_boot_runtime_wall_gfx_image_offsets_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_query_count,
    uint32_t *out_nonzero_count,
    uint32_t *out_present_mask)
{
    static const int k_fields[] = { 0xf0, 0xf1, 0xf2, 0xfd };
    uint32_t hash = 0x32494f46u;
    uint32_t count = 0u;
    uint32_t nonzero = 0u;
    uint32_t mask = 0u;
    DM2_V1_BootGraphicsDat *gfx;

    if (out_hash) *out_hash = 0u;
    if (out_query_count) *out_query_count = 0u;
    if (out_nonzero_count) *out_nonzero_count = 0u;
    if (out_present_mask) *out_present_mask = 0u;
    if (!profile || !profile->graphics_dat || !out_hash ||
        !out_query_count || !out_nonzero_count || !out_present_mask) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;

    /* skproject/SKWIN QUERY_GDAT_PICT_OFFSET and wall/shop drawing paths
     * consume WALL_GFX dtImageOffset fields 0xF0/0xF1/0xF2/0xFD as render
     * placement data, stored in ENT1 data_index. */
    for (int index = 0; index < 256; ++index) {
        for (uint32_t i = 0u; i < (uint32_t)(sizeof(k_fields) / sizeof(k_fields[0]));
             ++i) {
            uint16_t value = 0u;
            if (!dm2_v1_asset_load_image_offset(&gfx->loader,
                                                DM2_GDAT_CATEGORY_WALL_GFX,
                                                index,
                                                k_fields[i],
                                                &value)) {
                continue;
            }
            mask |= 1u << i;
            ++count;
            if (value != 0u) ++nonzero;
            hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                          (uint32_t)index);
            hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                          (uint32_t)k_fields[i]);
            hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                          (uint32_t)value);
        }
    }

    hash = dm2_v1_boot_packaged_capture_hash_step(hash, mask);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, nonzero);
    *out_hash = hash;
    *out_query_count = count;
    *out_nonzero_count = nonzero;
    *out_present_mask = mask;
    return count > 0u &&
           nonzero > 0u &&
           mask != 0u &&
           hash != 0u;
}

static int dm2_v1_boot_startup_menu_raw_screen_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_byte_count)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0;
    uint32_t hash = 0x324d5253u;

    if (out_hash) *out_hash = 0u;
    if (out_byte_count) *out_byte_count = 0u;
    if (!profile || !profile->graphics_dat || !out_hash ||
        !out_byte_count) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(&gfx->loader,
                                        DM2_GDAT_CATEGORY_TITLE,
                                        0,
                                        DM2_GDAT_ENTRY_TYPE_RAW7,
                                        4,
                                        &raw_size);
    if (!raw || raw_size != 320u * 200u || raw_size > UINT32_MAX) {
        return 0;
    }

    /* skproject/SKWIN/SkWinCore.cpp SHOW_MENU_SCREEN first checks TITLE
     * category index 0 dt07/4 for a 64000-byte raw menu screen. Only if that
     * entry is absent does it fall back to TITLE image field 4. */
    for (size_t i = 0; i < raw_size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    }
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)raw_size);
    *out_hash = hash;
    *out_byte_count = (uint32_t)raw_size;
    return *out_hash != 0u && *out_byte_count == 320u * 200u;
}

static int dm2_v1_boot_runtime_interface_rect14_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_byte_count,
    uint32_t *out_row_count,
    uint32_t *out_stride,
    uint32_t *out_nonzero_5x5_count,
    uint32_t *out_image_field_count,
    uint32_t *out_stretch_field_count,
    uint32_t *out_flag_field_count)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0;
    uint32_t hash = 0x32495231u;
    uint32_t rows;
    uint32_t nonzero_5x5 = 0u;
    uint32_t image_fields = 0u;
    uint32_t stretch_fields = 0u;
    uint32_t flag_fields = 0u;

    if (out_hash) *out_hash = 0u;
    if (out_byte_count) *out_byte_count = 0u;
    if (out_row_count) *out_row_count = 0u;
    if (out_stride) *out_stride = 0u;
    if (out_nonzero_5x5_count) *out_nonzero_5x5_count = 0u;
    if (out_image_field_count) *out_image_field_count = 0u;
    if (out_stretch_field_count) *out_stretch_field_count = 0u;
    if (out_flag_field_count) *out_flag_field_count = 0u;
    if (!profile || !profile->graphics_dat || !out_hash ||
        !out_byte_count || !out_row_count || !out_stride ||
        !out_nonzero_5x5_count || !out_image_field_count ||
        !out_stretch_field_count || !out_flag_field_count) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
        0,
        DM2_GDAT_ENTRY_TYPE_RAW7,
        DM2_GDAT_INTERFACE_RAW_RECT14_TABLE,
        &raw_size);
    if (!raw || raw_size == 0 || raw_size > UINT32_MAX ||
        (raw_size % 14u) != 0u) {
        return 0;
    }

    rows = (uint32_t)(raw_size / 14u);
    /* skproject/SKWIN/SkWinCore.cpp LOAD_GDAT_INTERFACE_00_0A loads
     * INTERFACE_GENERAL dt07/0x0A as U8 (*)[14] into _4976_5a98. Later
     * creature placement code reads [0] as the 5x5 slot, [2..5] as
     * direction image fields, [6..9] as stretch sizes via
     * CALC_STRETCHED_SIZE, and [10..13] as per-direction flags. */
    for (uint32_t row = 0; row < rows; ++row) {
        const uint8_t *r = raw + (size_t)row * 14u;
        if (r[0] != 0u) ++nonzero_5x5;
        for (int i = 2; i <= 5; ++i) {
            if (r[i] != 0xffu) ++image_fields;
        }
        for (int i = 6; i <= 9; ++i) {
            if (r[i] != 0u) ++stretch_fields;
        }
        for (int i = 10; i <= 13; ++i) {
            if (r[i] != 0u) ++flag_fields;
        }
        for (int i = 0; i < 14; ++i) {
            hash = dm2_v1_boot_packaged_capture_hash_step(hash, r[i]);
        }
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, row);
    }
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, rows);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, 14u);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, stretch_fields);

    *out_hash = hash;
    *out_byte_count = (uint32_t)raw_size;
    *out_row_count = rows;
    *out_stride = 14u;
    *out_nonzero_5x5_count = nonzero_5x5;
    *out_image_field_count = image_fields;
    *out_stretch_field_count = stretch_fields;
    *out_flag_field_count = flag_fields;
    return rows > 0u && image_fields > 0u;
}

static int dm2_v1_boot_runtime_interface_rect14_placement_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_placement_count,
    uint32_t *out_rotated_cell_mask,
    uint32_t *out_max_stretched_size)
{
    static const int k_distance_stretch64[4] = { 64, 52, 40, 32 };
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0;
    uint32_t hash = 0x32523150u;
    uint32_t placement_count = 0u;
    uint32_t rotated_mask = 0u;
    uint32_t max_stretched = 0u;
    uint32_t rows;

    if (out_hash) *out_hash = 0u;
    if (out_placement_count) *out_placement_count = 0u;
    if (out_rotated_cell_mask) *out_rotated_cell_mask = 0u;
    if (out_max_stretched_size) *out_max_stretched_size = 0u;
    if (!profile || !profile->graphics_dat || !out_hash ||
        !out_placement_count || !out_rotated_cell_mask ||
        !out_max_stretched_size) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
        0,
        DM2_GDAT_ENTRY_TYPE_RAW7,
        DM2_GDAT_INTERFACE_RAW_RECT14_TABLE,
        &raw_size);
    if (!raw || raw_size == 0 || raw_size > UINT32_MAX ||
        (raw_size % 14u) != 0u) {
        return 0;
    }

    rows = (uint32_t)(raw_size / 14u);
    for (uint32_t row = 0; row < rows; ++row) {
        const uint8_t *r = raw + (size_t)row * 14u;
        if (r[0] > 24u) {
            continue;
        }
        for (int cell = 0; cell < 4; ++cell) {
            DM2_V1_InterfaceRect14Placement placement;
            if (!dm2_v1_viewport_interface_rect14_placement(
                    r,
                    cell,
                    k_distance_stretch64[cell],
                    &placement)) {
                continue;
            }
            ++placement_count;
            hash = dm2_v1_boot_packaged_capture_hash_step(hash, row);
            hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                          (uint32_t)cell);
            hash = dm2_v1_boot_packaged_capture_hash_step(
                hash, (uint32_t)(uint8_t)placement.lateral_offset);
            for (int dir = 0; dir < 4; ++dir) {
                uint32_t bit = (uint32_t)
                    (dm2_v1_viewport_rotate_5x5_pos(r[0], dir) & 31);
                rotated_mask |= 1u << bit;
                if (placement.stretched_size[dir] > max_stretched) {
                    max_stretched = placement.stretched_size[dir];
                }
                hash = dm2_v1_boot_packaged_capture_hash_step(
                    hash, placement.blit_rect_id[dir]);
                hash = dm2_v1_boot_packaged_capture_hash_step(
                    hash, placement.image_field[dir]);
                hash = dm2_v1_boot_packaged_capture_hash_step(
                    hash, placement.stretched_size[dir]);
                hash = dm2_v1_boot_packaged_capture_hash_step(
                    hash, placement.flags[dir]);
            }
        }
    }
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, placement_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, rotated_mask);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, max_stretched);

    *out_hash = hash;
    *out_placement_count = placement_count;
    *out_rotated_cell_mask = rotated_mask;
    *out_max_stretched_size = max_stretched;
    return placement_count >= 4u && rotated_mask != 0u && hash != 0u;
}

static int dm2_v1_boot_runtime_interface_action_table_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_byte_count,
    uint32_t *out_group_count,
    uint32_t *out_entry_count,
    uint32_t *out_tail_byte_count)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0;
    uint32_t hash = 0x32494132u;
    uint32_t group_count;
    uint32_t entry_count = 0u;
    size_t min_payload;

    if (out_hash) *out_hash = 0u;
    if (out_byte_count) *out_byte_count = 0u;
    if (out_group_count) *out_group_count = 0u;
    if (out_entry_count) *out_entry_count = 0u;
    if (out_tail_byte_count) *out_tail_byte_count = 0u;
    if (!profile || !profile->graphics_dat || !out_hash ||
        !out_byte_count || !out_group_count || !out_entry_count ||
        !out_tail_byte_count) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
        0,
        DM2_GDAT_ENTRY_TYPE_RAW7,
        DM2_GDAT_INTERFACE_RAW_ACTION_TABLE,
        &raw_size);
    if (!raw || raw_size < 2 || raw_size > UINT32_MAX) {
        return 0;
    }

    /* skproject/SKWIN/SkWinCore.cpp LOAD_GDAT_INTERFACE_00_02 reads the
     * first byte as group count, copies one length byte per group into
     * _4976_4bde[].b0, then binds two variable-length blocks (pv1/pv5)
     * before the trailing _4976_4be2 command table. */
    group_count = raw[0];
    if (group_count == 0u || raw_size < 1u + (size_t)group_count) {
        return 0;
    }
    for (uint32_t i = 0; i < group_count; ++i) {
        entry_count += raw[1u + i];
    }
    min_payload = 1u + (size_t)group_count + ((size_t)entry_count * 2u);
    if (raw_size < min_payload) {
        return 0;
    }
    for (size_t i = 0; i < raw_size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    }
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, group_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, entry_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash,
        (uint32_t)(raw_size - min_payload));

    *out_hash = hash;
    *out_byte_count = (uint32_t)raw_size;
    *out_group_count = group_count;
    *out_entry_count = entry_count;
    *out_tail_byte_count = (uint32_t)(raw_size - min_payload);
    return entry_count > 0u;
}

static int dm2_v1_boot_runtime_interface_font_table_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_byte_count,
    uint32_t *out_row_count,
    uint32_t *out_char_count,
    uint32_t *out_nonzero_byte_count,
    uint32_t *out_printable_char_count)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0;
    uint32_t hash = 0x32494654u;
    uint32_t nonzero = 0u;
    uint32_t printable = 0u;

    if (out_hash) *out_hash = 0u;
    if (out_byte_count) *out_byte_count = 0u;
    if (out_row_count) *out_row_count = 0u;
    if (out_char_count) *out_char_count = 0u;
    if (out_nonzero_byte_count) *out_nonzero_byte_count = 0u;
    if (out_printable_char_count) *out_printable_char_count = 0u;
    if (!profile || !profile->graphics_dat || !out_hash ||
        !out_byte_count || !out_row_count || !out_char_count ||
        !out_nonzero_byte_count || !out_printable_char_count) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
        0,
        DM2_GDAT_ENTRY_TYPE_RAW7,
        DM2_GDAT_INTERFACE_RAW_LAYOUT_TABLE,
        &raw_size);
    if (!raw || raw_size != 0x300u) {
        return 0;
    }

    /* skproject/SKWIN/SkWinCore.cpp _3929_0e16 loads
     * INTERFACE_GENERAL dt07/0x00 into _4976_5c0e with 0x300 bytes.
     * QUERY_FONT indexes it as (row << 7) + character for six font rows. */
    for (size_t i = 0; i < raw_size; ++i) {
        if (raw[i] != 0u) ++nonzero;
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    }
    for (uint32_t ch = 0x20u; ch <= 0x7eu; ++ch) {
        int has_pixel = 0;
        for (uint32_t row = 0u; row < 6u; ++row) {
            if (raw[(row << 7) + ch] != 0u) {
                has_pixel = 1;
                break;
            }
        }
        if (has_pixel) ++printable;
    }
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, 6u);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, 128u);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, printable);

    *out_hash = hash;
    *out_byte_count = (uint32_t)raw_size;
    *out_row_count = 6u;
    *out_char_count = 128u;
    *out_nonzero_byte_count = nonzero;
    *out_printable_char_count = printable;
    return nonzero > 0u && printable > 0u;
}

static int dm2_v1_boot_runtime_interface_palette_receipt(
    DM2_V1_BootProfile *profile,
    uint32_t *out_hash,
    uint32_t *out_irgb_byte_count,
    uint32_t *out_pal16_byte_count,
    uint32_t *out_irgb_color_count,
    uint32_t *out_pal16_color_count)
{
    DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_InterfacePalette palette;

    if (out_hash) *out_hash = 0u;
    if (out_irgb_byte_count) *out_irgb_byte_count = 0u;
    if (out_pal16_byte_count) *out_pal16_byte_count = 0u;
    if (out_irgb_color_count) *out_irgb_color_count = 0u;
    if (out_pal16_color_count) *out_pal16_color_count = 0u;
    if (!profile || !profile->graphics_dat || !out_hash ||
        !out_irgb_byte_count || !out_pal16_byte_count ||
        !out_irgb_color_count || !out_pal16_color_count) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_asset_load_interface_palette(
            &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
            DM2_GDAT_INTERFACE_PALETTE_FIELD, &palette)) {
        return 0;
    }
    *out_hash = palette.hash;
    *out_irgb_byte_count = 256u * 4u;
    *out_pal16_byte_count = 16u;
    *out_irgb_color_count = 256u;
    *out_pal16_color_count = 16u;
    return *out_hash != 0u &&
           *out_irgb_color_count > 0u &&
           *out_pal16_color_count > 0u;
}

int dm2_v1_boot_interface_palette(DM2_V1_BootProfile *profile,
                                  DM2_V1_InterfacePalette *out_palette)
{
    DM2_V1_BootGraphicsDat *gfx;
    if (!out_palette) return 0;
    memset(out_palette, 0, sizeof(*out_palette));
    if (!profile || !profile->graphics_dat) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_asset_load_interface_palette(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_INTERFACE_PALETTE_FIELD, out_palette);
}

static int dm2_v1_boot_parse_interface_action_table(
    const uint8_t *raw,
    size_t raw_size,
    DM2_V1_InterfaceActionTable *out_table)
{
    uint32_t hash = 0x49374154u; /* I7AT */
    size_t cursor;
    uint32_t group_count;
    uint32_t entry_count = 0u;
    uint32_t i;

    if (!out_table) return 0;
    memset(out_table, 0, sizeof(*out_table));
    if (!raw || raw_size == 0u || raw_size > UINT32_MAX) return 0;

    /* skproject/SKWIN/SkWinCore.cpp LOAD_GDAT_INTERFACE_00_02 reads
     * INTERFACE_GENERAL/0/dt07/2 as:
     *   group_count, length[group_count], primary[block], secondary[block], tail */
    group_count = raw[0];
    if (group_count == 0u ||
        group_count > DM2_V1_INTERFACE_ACTION_GROUP_MAX ||
        raw_size < 1u + group_count) {
        return 0;
    }

    cursor = 1u + (size_t)group_count;
    for (i = 0u; i < group_count; ++i) {
        uint8_t length = raw[1u + i];
        if (length == 0u) {
            memset(out_table, 0, sizeof(*out_table));
            return 0;
        }
        out_table->groups[i].length = length;
        entry_count += length;
    }

    if (entry_count == 0u ||
        raw_size < cursor + ((size_t)entry_count * 2u)) {
        memset(out_table, 0, sizeof(*out_table));
        return 0;
    }

    for (i = 0u; i < group_count; ++i) {
        out_table->groups[i].primary_offset = (uint32_t)cursor;
        cursor += out_table->groups[i].length;
        out_table->groups[i].secondary_offset = (uint32_t)cursor;
        cursor += out_table->groups[i].length;
    }

    for (i = 0u; i < raw_size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    }

    out_table->valid = 1;
    out_table->raw = raw;
    out_table->raw_size = (uint32_t)raw_size;
    out_table->hash = hash;
    out_table->group_count = group_count;
    out_table->entry_count = entry_count;
    out_table->tail_offset = (uint32_t)cursor;
    out_table->tail_size = (uint32_t)(raw_size - cursor);
    return 1;
}

int dm2_v1_boot_interface_action_table(
    DM2_V1_BootProfile *profile,
    DM2_V1_InterfaceActionTable *out_table)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0u;

    if (!out_table) return 0;
    memset(out_table, 0, sizeof(*out_table));
    if (!profile || !profile->graphics_dat) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
        0,
        DM2_GDAT_ENTRY_TYPE_RAW7,
        DM2_GDAT_INTERFACE_RAW_ACTION_TABLE,
        &raw_size);
    /* skproject/SKWIN/SkWinCore.cpp LOAD_GDAT_INTERFACE_00_02 binds the
     * two group spans and command tail directly from this original dt07/2
     * payload. Firestaff retains those offsets without inventing actions. */
    return dm2_v1_boot_parse_interface_action_table(raw, raw_size, out_table);
}

int dm2_v1_boot_extended_spell_gdat_receipt(DM2_V1_BootProfile *profile, DM2_V1_ExtendedSpellGdatReceipt *out)
{
    DM2_V1_BootGraphicsDat *gfx;
    int i;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !profile->graphics_dat) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    out->word_hash = 0x53504c57u;
    for (i = 0; i < 254; ++i) {
        uint16_t w;
        int f;
        if (!dm2_v1_asset_load_word_value(&gfx->loader, DM2_GDAT_CATEGORY_SPELL_DEF, i, 1, &w) || w == 0u) continue;
        ++out->defined_count;
        for (f = 1; f <= 7; ++f) {
            if (!dm2_v1_asset_load_word_value(&gfx->loader, DM2_GDAT_CATEGORY_SPELL_DEF, i, f, &w)) return 0;
            out->word_hash = dm2_v1_boot_packaged_capture_hash_step(out->word_hash, (uint8_t)w);
            out->word_hash = dm2_v1_boot_packaged_capture_hash_step(out->word_hash, (uint8_t)(w >> 8));
        }
    }
    out->valid = 1;
    return 1;
}

int dm2_v1_interface_action_table_remap_palette(
    const DM2_V1_InterfaceActionTable *table,
    uint8_t *palette,
    uint32_t palette_count,
    uint8_t darkness_0_to_64,
    int colorkey1,
    int colorkey2)
{
    uint32_t attenuation;

    if (!table || !table->valid || !table->raw || !palette ||
        palette_count == 0u || palette_count > 256u ||
        table->tail_size < 512u ||
        table->tail_offset > table->raw_size - 512u) {
        return 0;
    }
    attenuation = darkness_0_to_64 > 64u ? 0u :
        64u - (uint32_t)darkness_0_to_64;
    for (uint32_t i = 0; i < palette_count; ++i) {
        uint8_t color;
        uint32_t pair_offset;
        uint8_t group_index;
        uint8_t threshold_index;
        const DM2_V1_InterfaceActionGroup *group;
        uint32_t target;
        uint32_t selected = 0u;

        if ((int)i == colorkey1 || (int)i == colorkey2) continue;
        color = palette[i];
        pair_offset = table->tail_offset + (uint32_t)color * 2u;
        group_index = table->raw[pair_offset];
        threshold_index = table->raw[pair_offset + 1u];
        if (group_index >= table->group_count) return 0;
        group = &table->groups[group_index];
        if (group->length == 0u || threshold_index >= group->length ||
            group->primary_offset > table->raw_size - group->length ||
            group->secondary_offset > table->raw_size - group->length) {
            return 0;
        }
        /* skproject _0b36_037e (0B36:03E6-04EB): scale the selected pv1
         * threshold, then choose the closest source pv1 entry. */
        target = ((uint32_t)table->raw[group->primary_offset + threshold_index] *
                  attenuation) >> 6;
        for (; selected + 1u < group->length; ++selected) {
            uint32_t left = table->raw[group->primary_offset + selected];
            uint32_t right = table->raw[group->primary_offset + selected + 1u];
            if (left <= target && right >= target) {
                if (target - left > right - target) ++selected;
                break;
            }
        }
        palette[i] = table->raw[group->secondary_offset + selected];
    }
    return 1;
}

int dm2_v1_boot_interface_font_table(
    DM2_V1_BootProfile *profile,
    const uint8_t **out_rows,
    uint32_t *out_hash)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *rows;
    size_t byte_count = 0u;
    uint32_t hash = 2166136261u;

    if (out_rows) *out_rows = NULL;
    if (out_hash) *out_hash = 0u;
    if (!profile || !profile->graphics_dat || !out_rows || !out_hash) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    rows = dm2_v1_asset_load_typed_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_INTERFACE_RAW_LAYOUT_TABLE,
        &byte_count);
    if (!rows || byte_count != 0x300u) {
        return 0;
    }
    /* skproject/SKWIN/SkWinCore.cpp QUERY_FONT indexes exactly six rows of
     * 128 bytes: `(row << 7) + character`.  Keep the source payload whole;
     * no Firestaff font or inferred glyph shape is substituted. */
    for (size_t i = 0; i < byte_count; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, rows[i]);
    }
    *out_rows = rows;
    *out_hash = hash;
    return hash != 0u;
}

static int dm2_v1_boot_g1_wall_gfx_scalar_read(
    void *userdata,
    int entry_type,
    int category,
    int index,
    int field,
    uint16_t *out_value)
{
    const DM2_V1_BootGraphicsDat *gfx =
        (const DM2_V1_BootGraphicsDat *)userdata;

    if (!gfx || !out_value) return 0;
    if (entry_type == DM2_GDAT_ENTRY_TYPE_WORD_VALUE) {
        return dm2_v1_asset_load_word_value(&gfx->loader, category, index,
                                            field, out_value);
    }
    if (entry_type == DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET) {
        return dm2_v1_asset_load_image_offset(&gfx->loader, category, index,
                                               field, out_value);
    }
    return 0;
}

static int dm2_v1_boot_g1_raw_read(
    void *userdata,
    int entry_type,
    int category,
    int index,
    int field,
    const uint8_t **out_data,
    uint32_t *out_byte_count)
{
    const DM2_V1_BootGraphicsDat *gfx =
        (const DM2_V1_BootGraphicsDat *)userdata;
    size_t byte_count = 0u;
    const uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_byte_count) *out_byte_count = 0u;
    if (!gfx || !out_data || !out_byte_count) return 0;
    data = dm2_v1_asset_load_typed_sized(&gfx->loader, category, index,
                                          entry_type, field, &byte_count);
    if (!data || byte_count == 0u || byte_count > UINT32_MAX) return 0;
    *out_data = data;
    *out_byte_count = (uint32_t)byte_count;
    return 1;
}

static int dm2_v1_boot_g1_text_read(
    void *userdata,
    int category,
    int index,
    int field,
    const uint8_t **out_data,
    uint32_t *out_byte_count)
{
    const DM2_V1_BootGraphicsDat *gfx =
        (const DM2_V1_BootGraphicsDat *)userdata;
    size_t byte_count = 0u;
    const uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_byte_count) *out_byte_count = 0u;
    if (!gfx || !out_data || !out_byte_count) return 0;
    data = dm2_v1_asset_load_text_sized(&gfx->loader, category, index, field,
                                        &byte_count);
    if (!data || byte_count == 0u || byte_count > UINT32_MAX) return 0;
    *out_data = data;
    *out_byte_count = (uint32_t)byte_count;
    return 1;
}

static int dm2_v1_boot_g1_image_metadata_read(
    void *userdata,
    int category,
    int index,
    int field,
    int *out_width,
    int *out_height,
    int *out_format)
{
    const DM2_V1_BootGraphicsDat *gfx =
        (const DM2_V1_BootGraphicsDat *)userdata;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    uint8_t *pixels;
    int width = 0;
    int height = 0;

    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (out_format) *out_format = DM2_IMG_FMT_UNKNOWN;
    if (!gfx || !out_width || !out_height || !out_format) return 0;
    pixels = dm2_v1_asset_load_image_field(&gfx->loader, category, index,
                                            field, &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 ||
        (format != DM2_IMG_FMT_IMG3 && format != DM2_IMG_FMT_U4 &&
         format != DM2_IMG_FMT_U8 && format != DM2_IMG_FMT_IMG9)) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    dm2_v1_asset_free_pixels(pixels);
    *out_width = width;
    *out_height = height;
    *out_format = format;
    return 1;
}

static int dm2_v1_boot_g1_image_local_palette_read(
    void *userdata,
    int category,
    int index,
    int field,
    uint8_t out_palette16[16],
    uint32_t *out_hash)
{
    const DM2_V1_BootGraphicsDat *gfx =
        (const DM2_V1_BootGraphicsDat *)userdata;

    if (out_hash) *out_hash = 0u;
    if (out_palette16) memset(out_palette16, 0, 16u);
    if (!gfx || !out_palette16 || !out_hash) return 0;
    return dm2_v1_asset_load_image_local_palette(
        &gfx->loader, category, index, field, out_palette16, out_hash);
}

int dm2_v1_boot_g1_text_wall_gfx_materials(
    DM2_V1_BootProfile *profile,
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1TextWallGfxRuntimeReceipt *out)
{
    DM2_V1_BootGraphicsDat *gfx;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !profile->graphics_dat) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_dungeon_materialize_g1_text_wall_gfx_image_material_runtime(
        texts, dm2_v1_boot_g1_wall_gfx_scalar_read,
        dm2_v1_boot_g1_image_metadata_read,
        dm2_v1_boot_g1_image_local_palette_read, gfx, out)) return 0;
    for (i = 0; i < out->material_count; ++i) {
        DM2_V1_GdatGfxRawMaterialReceipt raw;
        DM2_V1_G1TextWallGfxMaterial *material = &out->materials[i];

        if (!material->front_image_ready || !material->local_palette_hash ||
            !dm2_v1_gdat_image_raw_material_receipt(
                &gfx->loader, DM2_GDAT_CATEGORY_WALL_GFX,
                material->wall_gfx_index, 1, &raw) || !raw.accepted ||
            raw.source_hash == 0u || raw.receipt_hash == 0u) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        material->raw_material_index = raw.raw_index;
        material->raw_material_bytes = raw.source_bytes;
        material->raw_material_byte_count = raw.source_byte_count;
        material->raw_material_hash = raw.source_hash;
        material->raw_material_receipt_hash = raw.receipt_hash;
    }
    return 1;
}

int dm2_v1_boot_g1_gdat_text_materials(
    DM2_V1_BootProfile *profile,
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatTextMessageRuntimeReceipt *out)
{
    DM2_V1_BootGraphicsDat *gfx;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !profile->graphics_dat) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_dungeon_materialize_g1_map5_gdat_text_messages(
        texts, dm2_v1_boot_g1_text_read, gfx, out);
}

int dm2_v1_boot_g1_actuator_wall_gfx_materials(
    DM2_V1_BootProfile *profile,
    int map,
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt *out)
{
    DM2_V1_BootGraphicsDat *gfx;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !profile->graphics_dat || !profile->dungeon_data) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_dungeon_materialize_g1_actuator_wall_gfx_image_material_runtime(
        (const DM2_V1_DungeonData *)profile->dungeon_data, map,
        dm2_v1_boot_g1_wall_gfx_scalar_read,
        dm2_v1_boot_g1_image_metadata_read,
        dm2_v1_boot_g1_image_local_palette_read, gfx, out)) return 0;
    for (i = 0; i < out->material_count; ++i) {
        DM2_V1_GdatGfxRawMaterialReceipt raw;
        DM2_V1_G1ActuatorWallGfxMaterial *material = &out->materials[i];

        if (!material->front_image_ready || !material->local_palette_hash ||
            !dm2_v1_gdat_image_raw_material_receipt(
                &gfx->loader, DM2_GDAT_CATEGORY_WALL_GFX,
                material->wall_gfx_index, 1, &raw) || !raw.accepted ||
            raw.source_hash == 0u || raw.receipt_hash == 0u) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        material->raw_material_index = raw.raw_index;
        material->raw_material_bytes = raw.source_bytes;
        material->raw_material_byte_count = raw.source_byte_count;
        material->raw_material_hash = raw.source_hash;
        material->raw_material_receipt_hash = raw.receipt_hash;
    }
    return 1;
}

int dm2_v1_boot_g1_creature_map_chip_materials(
    DM2_V1_BootProfile *profile,
    int map,
    DM2_V1_G1CreatureMapChipRuntimeReceipt *out)
{
    DM2_V1_BootGraphicsDat *gfx;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !profile->graphics_dat || !profile->dungeon_data) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    return dm2_v1_dungeon_materialize_g1_creature_map_chip_runtime(
        (const DM2_V1_DungeonData *)profile->dungeon_data, map,
        dm2_v1_boot_g1_raw_read, dm2_v1_boot_g1_image_metadata_read,
        dm2_v1_boot_g1_image_local_palette_read, gfx, out);
}

static uint32_t dm2_v1_boot_g1_indexed_pixel_hash(const uint8_t *pixels,
                                                   int width,
                                                   int height,
                                                   int stride)
{
    uint32_t hash = 2166136261u;

    if (!pixels || width <= 0 || height <= 0 || stride < width) return 0u;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            hash ^= pixels[y * stride + x];
            hash *= 16777619u;
        }
    }
    return hash ? hash : 1u;
}

static int dm2_v1_boot_bind_g1_weapon_map_chip_pixels(
    DM2_V1_BootProfile *profile,
    DM2_V1_G1WeaponMapChipRuntimeReceipt *receipt)
{
    for (int i = 0; i < receipt->material_count; ++i) {
        DM2_V1_G1WeaponMapChipMaterial *material = &receipt->materials[i];
        DM2_V1_BootViewportAssetEvidence evidence;
        const uint8_t *pixels = NULL;
        int width = 0, height = 0, stride = 0;
        int gdat_index = dm2_v1_viewport_item_graphic_index(
            0x10, material->item_type, 0xf9);

        /* skproject DRAW_MAP_CHIP reaches exactly WEAPONS/itemType/F9. The
         * decoded image must still be the raw-GDAT-proven virtual resource
         * selected by the DB5 receipt, not merely a matching-size bitmap. */
        if (gdat_index == 0 ||
            !dm2_v1_boot_viewport_asset_evidence(profile, gdat_index,
                                                  &evidence) ||
            evidence.category != 0x10 ||
            evidence.entry_index != material->item_type ||
            evidence.field != 0xf9 ||
            evidence.raw_byte_count != material->raw_byte_count ||
            evidence.decoded_w != material->image_width ||
            evidence.decoded_h != material->image_height ||
            dm2_v1_boot_viewport_asset_fetch(profile, gdat_index, &pixels,
                                             &width, &height, &stride) != 0 ||
            width != material->image_width || height != material->image_height) {
            return 0;
        }
        material->decoded_pixel_hash = dm2_v1_boot_g1_indexed_pixel_hash(
            pixels, width, height, stride);
        if (material->decoded_pixel_hash == 0u) return 0;
    }
    return 1;
}

static int dm2_v1_boot_bind_g1_container_map_chip_pixels(
    DM2_V1_BootProfile *profile,
    DM2_V1_G1ContainerMapChipRuntimeReceipt *receipt)
{
    for (int i = 0; i < receipt->material_count; ++i) {
        DM2_V1_G1ContainerMapChipMaterial *material = &receipt->materials[i];
        DM2_V1_BootViewportAssetEvidence evidence;
        const uint8_t *pixels = NULL;
        int width = 0, height = 0, stride = 0;
        int gdat_index = dm2_v1_viewport_item_graphic_index(
            0x14, material->container_type, 0xf9);

        if (gdat_index == 0 ||
            !dm2_v1_boot_viewport_asset_evidence(profile, gdat_index,
                                                  &evidence) ||
            evidence.category != 0x14 ||
            evidence.entry_index != material->container_type ||
            evidence.field != 0xf9 ||
            evidence.raw_byte_count != material->raw_byte_count ||
            evidence.decoded_w != material->image_width ||
            evidence.decoded_h != material->image_height ||
            dm2_v1_boot_viewport_asset_fetch(profile, gdat_index, &pixels,
                                             &width, &height, &stride) != 0 ||
            width != material->image_width || height != material->image_height) {
            return 0;
        }
        material->decoded_pixel_hash = dm2_v1_boot_g1_indexed_pixel_hash(
            pixels, width, height, stride);
        if (material->decoded_pixel_hash == 0u) return 0;
    }
    return 1;
}

int dm2_v1_boot_g1_weapon_map_chip_materials(
    DM2_V1_BootProfile *profile,
    int map,
    DM2_V1_G1WeaponMapChipRuntimeReceipt *out)
{
    DM2_V1_BootGraphicsDat *gfx;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !profile->graphics_dat || !profile->dungeon_data) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_dungeon_materialize_g1_weapon_map_chip_runtime(
        (const DM2_V1_DungeonData *)profile->dungeon_data, map,
        dm2_v1_boot_g1_raw_read, dm2_v1_boot_g1_image_metadata_read,
        dm2_v1_boot_g1_image_local_palette_read, gfx, out) ||
        !dm2_v1_boot_bind_g1_weapon_map_chip_pixels(profile, out)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    return 1;
}

int dm2_v1_boot_g1_container_map_chip_materials(
    DM2_V1_BootProfile *profile,
    int map,
    DM2_V1_G1ContainerMapChipRuntimeReceipt *out)
{
    DM2_V1_BootGraphicsDat *gfx;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !profile->graphics_dat || !profile->dungeon_data) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_dungeon_materialize_g1_container_map_chip_runtime(
        (const DM2_V1_DungeonData *)profile->dungeon_data, map,
        dm2_v1_boot_g1_raw_read, dm2_v1_boot_g1_image_metadata_read,
        dm2_v1_boot_g1_image_local_palette_read, gfx, out) ||
        !dm2_v1_boot_bind_g1_container_map_chip_pixels(profile, out)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    return 1;
}

static uint16_t dm2_v1_boot_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int dm2_v1_boot_rect_raw(const uint8_t *raw, size_t raw_size,
                                uint16_t rect_id, DM2_V1_InterfaceRect *out)
{
    uint16_t groups;
    size_t pos;
    if (!raw || raw_size < 4u || !out || dm2_v1_boot_le16(raw) != 0xfc0du) return 0;
    groups = dm2_v1_boot_le16(raw + 2);
    if (groups == 0u || 4u + (size_t)groups * 4u > raw_size) return 0;
    pos = 4u + (size_t)groups * 4u;
    for (uint16_t group = 0; group < groups; ++group) {
        uint16_t first = dm2_v1_boot_le16(raw + 4u + (size_t)group * 4u);
        uint16_t last = dm2_v1_boot_le16(raw + 6u + (size_t)group * 4u);
        size_t count = last >= first ? (size_t)(last - first + 1u) : 0u;
        if (count == 0u || pos + count * 8u > raw_size) return 0;
        if (rect_id >= first && rect_id <= last) {
            const uint8_t *row = raw + pos + (size_t)(rect_id - first) * 8u;
            out->x = (int16_t)dm2_v1_boot_le16(row);
            out->y = (int16_t)dm2_v1_boot_le16(row + 2);
            out->w = (int16_t)dm2_v1_boot_le16(row + 4);
            out->h = (int16_t)dm2_v1_boot_le16(row + 6);
            return 1;
        }
        pos += count * 8u;
    }
    return 0;
}

static int dm2_v1_boot_expand_hud_rect(const uint8_t *raw, size_t raw_size,
                                       uint16_t rect_id,
                                       DM2_V1_InterfaceRect *out)
{
    DM2_V1_InterfaceRect current, next;
    int anchor, x, y, w, h;
    if (!dm2_v1_boot_rect_raw(raw, raw_size, rect_id, &current) ||
        current.y == 0 || !dm2_v1_boot_rect_raw(raw, raw_size,
                                                  (uint16_t)current.y, &next) ||
        next.x != 9) return 0;
    anchor = current.x; x = current.w; y = current.h; w = next.w; h = next.h;
    for (int guard = 0; current.y != 0 && guard < 16; ++guard) {
        if (!dm2_v1_boot_rect_raw(raw, raw_size, (uint16_t)current.y, &next)) return 0;
        if (next.x == 1) { x += next.w; y += next.h; }
        else if (next.x == 9) {
            int dx, dy;
            switch (current.x) {
            case 1: dx = current.w; dy = current.h; break;
            case 4: dx = current.w; dy = current.h - (next.h - 1); break;
            case 7: dx = current.w - ((next.w + 1) >> 1); dy = current.h - (next.h - 1); break;
            default: return 0;
            }
            x += dx; y += dy;
        } else return 0;
        current = next;
    }
    if (current.y != 0 || anchor < 1 || anchor > 8 || w <= 0 || h <= 0) return 0;
    if (anchor == 1 || anchor == 4 || anchor == 8) out->x = x;
    else out->x = x - ((w + 1) >> 1);
    if (anchor == 1 || anchor == 2 || anchor == 5) out->y = y;
    else out->y = y - (h - 1);
    out->w = w; out->h = h;
    return 1;
}

/* skproject c_xrect.cpp::DM2_COMPRESS_RECTS and DM2_QUERY_RECT.  raw4 is
 * not a flat rectangle table: it is compressed into c_rinfo nodes before
 * QUERY_BLIT_RECT consumes it.  This immutable decoder reproduces that node
 * view without inventing a second table or allocating source-derived state. */
static int dm2_v1_boot_query_compressed_rect(const uint8_t *raw, size_t raw_size,
                                             uint16_t rect_id,
                                             DM2_V1_InterfaceRect *out)
{
    uint16_t groups;
    size_t pos;

    if (!raw || !out || raw_size < 4u || dm2_v1_boot_le16(raw) != 0xfc0du ||
        rect_id == 0u) return 0;
    groups = dm2_v1_boot_le16(raw + 2u);
    if (groups == 0u || 4u + (size_t)groups * 4u > raw_size) return 0;
    pos = 4u + (size_t)groups * 4u;
    for (uint16_t group = 0; group < groups; ++group) {
        uint16_t first = dm2_v1_boot_le16(raw + 4u + (size_t)group * 4u);
        uint16_t last = dm2_v1_boot_le16(raw + 6u + (size_t)group * 4u);
        size_t count;
        uint8_t mask = 0x1fu;
        uint16_t x0;
        uint16_t y0;
        const uint8_t *row;

        if (last < first) return 0;
        count = (size_t)(last - first) + 1u;
        if (count > (raw_size - pos) / 8u) return 0;
        if (rect_id < first || rect_id > last) {
            pos += count * 8u;
            continue;
        }
        x0 = dm2_v1_boot_le16(raw + pos);
        y0 = dm2_v1_boot_le16(raw + pos + 2u);
        for (size_t i = 0u; i < count; ++i) {
            const uint8_t *candidate = raw + pos + i * 8u;
            int16_t width = (int16_t)dm2_v1_boot_le16(candidate + 4u);
            int16_t height = (int16_t)dm2_v1_boot_le16(candidate + 6u);
            if (dm2_v1_boot_le16(candidate) != x0) mask &= (uint8_t)~0x02u;
            if (dm2_v1_boot_le16(candidate + 2u) != y0) mask &= (uint8_t)~0x01u;
            if (dm2_v1_boot_le16(candidate + 2u) > 0xffu) mask &= (uint8_t)~0x04u;
            if (width < 0 || width > 0xff || height < 0 || height > 0xff)
                mask &= (uint8_t)~0x10u;
            if (width < -128 || width > 127 || height < -128 || height > 127)
                mask &= (uint8_t)~0x08u;
        }
        if (mask & 0x03u) mask &= (uint8_t)~0x04u;
        row = raw + pos + (size_t)(rect_id - first) * 8u;
        out->x = (mask & 0x04u) ? (int)row[0] :
                 ((mask & 0x02u) ? (int)(uint8_t)x0 :
                  (int)(int16_t)dm2_v1_boot_le16(row));
        out->y = (mask & 0x04u) ? (int)row[2] :
                 ((mask & 0x01u) ? (int)(int16_t)y0 :
                  (int)(int16_t)dm2_v1_boot_le16(row + 2u));
        if (mask & 0x08u) {
            out->w = (int)(int8_t)row[4];
            out->h = (int)(int8_t)row[6];
        } else if (mask & 0x10u) {
            out->w = (int)row[4];
            out->h = (int)row[6];
        } else {
            out->w = (int)(int16_t)dm2_v1_boot_le16(row + 4u);
            out->h = (int)(int16_t)dm2_v1_boot_le16(row + 6u);
        }
        return 1;
    }
    return 0;
}

static int dm2_v1_boot_blit_anchor(int mode, int x0, int y0, int width,
                                   int height, DM2_V1_InterfaceRect *out)
{
    if (!out || width <= 0 || height <= 0 || mode < 0 || mode > 8) return 0;
    switch (mode) {
    case 0: out->x = x0 - (width + 1) / 2; out->y = y0 - (height + 1) / 2; break;
    case 1: out->x = x0; out->y = y0; break;
    case 2: out->x = x0 - width + 1; out->y = y0; break;
    case 3: out->x = x0 - width + 1; out->y = y0 - height + 1; break;
    case 4: out->x = x0; out->y = y0 - height + 1; break;
    case 5: out->x = x0 - (width + 1) / 2; out->y = y0; break;
    case 6: out->x = x0 - width + 1; out->y = y0 - (height + 1) / 2; break;
    case 7: out->x = x0 - (width + 1) / 2; out->y = y0 - height + 1; break;
    default: out->x = x0; out->y = y0 - (height + 1) / 2; break;
    }
    out->w = width;
    out->h = height;
    return 1;
}

/* Exact no-bitmap subset of c_xrect.cpp::DM2_QUERY_BLIT_RECT.  The dialog
 * labels call it with source font metrics; clipping remains the source's
 * unrestricted default rectangle. */
static int dm2_v1_boot_query_blit_text_rect(
    const uint8_t *raw, size_t raw_size, uint16_t rect_id,
    int text_width, int text_height, DM2_V1_InterfaceRect *out)
{
    DM2_V1_InterfaceRect current, next, clip;
    int mode, x0, y0, dx = 0, dy = 0;
    int clip_x = -10000, clip_y = -10000, clip_w = 20000, clip_h = 20000;

    if (!out || text_width <= 0 || text_height <= 0 ||
        !dm2_v1_boot_query_compressed_rect(raw, raw_size, rect_id, &current)) return 0;
    mode = current.x;
    if (mode == 9 || mode < 0 || mode > 18) return 0;
    if (mode > 8) { x0 = 0; y0 = 0; mode -= 10; }
    else { x0 = current.w; y0 = current.h; }

    for (int guard = 0; current.y != 0 && guard < 64; ++guard) {
        if (current.x < 10 || current.x > 18) {
            if (!dm2_v1_boot_query_compressed_rect(raw, raw_size,
                                                    (uint16_t)current.y, &next)) return 0;
            dx = next.w;
            dy = next.h;
            if (next.x != 1) {
                if (next.x == 9) {
                    if (current.x > 8 ||
                        !dm2_v1_boot_blit_anchor(current.x, current.w, current.h,
                                                   next.w, next.h, &clip)) return 0;
                    dx = clip.x;
                    dy = clip.y;
                    if (dx > clip_x) clip_x = dx;
                    if (clip_x + clip_w - 1 >= dx + next.w) clip_w = next.w - clip_x + dx;
                    if (clip_y < dy) clip_y = dy;
                    dy += next.h;
                    if (clip_y + clip_h - 1 >= dy) clip_h = dy - clip_y;
                } else if (next.x <= 8) {
                    /* Source sets a deferred anchor flag here.  The dialog
                     * labels only use the source-verified RECT_9 chain. */
                    return 0;
                } else return 0;
            } else {
                x0 += dx; y0 += dy; clip_x += dx; clip_y += dy;
            }
        } else {
            DM2_V1_InterfaceRect link;
            if (!dm2_v1_boot_query_compressed_rect(raw, raw_size,
                                                    (uint16_t)current.y, &link)) return 0;
            if (link.y == 0 || link.x < 0 || link.x > 8 ||
                !dm2_v1_boot_query_compressed_rect(raw, raw_size,
                                                    (uint16_t)link.y, &next)) return 0;
            dx = link.w; dy = link.h;
            switch (link.x) {
            case 0: dy -= (next.h + 1) / 2; /* fall through */
            case 5: dx -= (next.w + 1) / 2; break;
            case 1: break;
            case 3: dy -= next.h - 1; /* fall through */
            case 2: dx -= next.w - 1; break;
            case 6: dx -= next.w - 1; /* fall through */
            case 8: dy -= (next.h + 1) / 2; break;
            case 7: dx -= (next.w + 1) / 2; /* fall through */
            default: dy -= next.h - 1; break;
            }
            clip_x += dx; if (dx > clip_x) clip_x = dx;
            clip_w = (next.w + dx <= clip_x + clip_w - 1) ? next.w - clip_x + dx : next.w + dx;
            clip_y += dy; if (clip_y < dy) clip_y = dy;
            if (dy + next.h <= clip_y + clip_h - 1) clip_h = dy + next.h - clip_y;
            switch (current.x - 10) {
            case 0: dy += (next.h + 1) / 2; /* fall through */
            case 5: dx += (next.w + 1) / 2; break;
            case 1: break;
            case 3: dy += next.h - 1; /* fall through */
            case 2: dx += next.w - 1; break;
            case 6: dx += next.w - 1; /* fall through */
            case 8: dy += (next.h + 1) / 2; break;
            case 7: dx += (next.w + 1) / 2; /* fall through */
            default: dy += next.h - 1; break;
            }
            x0 += dx + current.w;
            y0 += dy + current.h;
        }
        current = next;
    }
    if (current.y != 0 ||
        !dm2_v1_boot_blit_anchor(mode, x0, y0, text_width, text_height, out)) return 0;
    dx = clip_x - out->x;
    if (dx > 0) { out->x = clip_x; out->w = text_width - dx < clip_w ? text_width - dx : clip_w; }
    else out->w = text_width < dx + clip_w ? text_width : dx + clip_w;
    dy = clip_y - out->y;
    if (dy > 0) { out->y = clip_y; out->h = text_height - dy < clip_h ? text_height - dy : clip_h; }
    else out->h = text_height < dy + clip_h ? text_height : dy + clip_h;
    return out->w > 0 && out->h > 0;
}

int dm2_v1_boot_query_expanded_rect_receipt(
    const DM2_V1_BootProfile *profile, uint16_t rect_id,
    DM2_V1_BootExpandedRectReceipt *out_receipt)
{
    const DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->graphics_dat || rect_id == 0u) return 0;
    gfx = (const DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw_size);
    if (!raw || raw_size < 4u ||
        !dm2_v1_boot_expand_hud_rect(raw, raw_size, rect_id,
                                     &out_receipt->rect) ||
        out_receipt->rect.w <= 0 || out_receipt->rect.h <= 0) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    for (size_t i = 0u; i < raw_size; ++i)
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    out_receipt->rect_id = rect_id;
    out_receipt->raw4_bytes = raw;
    out_receipt->raw4_byte_count = raw_size;
    out_receipt->raw4_hash = hash ? hash : 1u;
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, rect_id);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)out_receipt->rect.x);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)out_receipt->rect.y);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)out_receipt->rect.w);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)out_receipt->rect.h);
    out_receipt->receipt_hash = hash ? hash : 1u;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_boot_g1_static_object_material_receipt(
    const DM2_V1_BootProfile *profile,
    const DM2_V1_G1StaticObjectMaterialSelector *selector,
    uint16_t clip_rect_id, DM2_V1_G1StaticObjectMaterialReceipt *out_receipt)
{
    const DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_GdatGfxRawMaterialReceipt raw;
    DM2_V1_BootExpandedRectReceipt rect;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->graphics_dat || !selector || !selector->valid ||
        (selector->category != 0x10u && selector->category != 0x14u) ||
        (selector->category == 0x10u && selector->image_field != 0u) ||
        (selector->category == 0x14u && selector->image_field != 0u &&
         selector->image_field != 4u) || clip_rect_id == 0u) return 0;
    gfx = (const DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_gdat_image_raw_material_receipt(
            &gfx->loader, selector->category, selector->item_type,
            selector->image_field, &raw) || !raw.accepted || !raw.source_bytes ||
        !raw.source_byte_count || !raw.source_hash || !raw.receipt_hash ||
        !dm2_v1_asset_load_image_local_palette(
            &gfx->loader, selector->category, selector->item_type,
            selector->image_field, out_receipt->local_palette16,
            &out_receipt->local_palette_hash) || !out_receipt->local_palette_hash ||
        !dm2_v1_boot_query_expanded_rect_receipt(profile, clip_rect_id, &rect)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->selector = *selector;
    out_receipt->raw_gfx256_bytes = raw.source_bytes;
    out_receipt->raw_gfx256_byte_count = raw.source_byte_count;
    out_receipt->raw_gfx256_hash = raw.source_hash;
    out_receipt->raw_gfx256_receipt_hash = raw.receipt_hash;
    out_receipt->clip_rect_id = clip_rect_id;
    out_receipt->raw4_hash = rect.raw4_hash;
    out_receipt->raw4_receipt_hash = rect.receipt_hash;
    return 1;
}

int dm2_v1_boot_g1_flying_item_material_receipt(
    const DM2_V1_BootProfile *profile,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    DM2_V1_G1FlyingItemMaterialReceipt *out_receipt)
{
    const DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_GdatGfxRawMaterialReceipt raw;
    DM2_V1_BootExpandedRectReceipt rect;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* DRAW_FLYING_ITEM reaches QUERY_TEMP_PICST only with these fields. */
    if (!profile || !profile->graphics_dat || !source || !source->valid ||
        (source->category != 0x0du && source->category != 0x0eu) ||
        (source->image_field != 8u && source->image_field != 9u &&
         source->image_field != 10u && source->image_field != 12u) ||
        source->clip_rect_id == 0u) return 0;
    gfx = (const DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_gdat_image_raw_material_receipt(
            &gfx->loader, source->category, source->item_type,
            source->image_field, &raw) || !raw.accepted || !raw.source_bytes ||
        !raw.source_byte_count || !raw.source_hash || !raw.receipt_hash ||
        !dm2_v1_asset_load_image_local_palette(
            &gfx->loader, source->category, source->item_type,
            source->image_field, out_receipt->local_palette16,
            &out_receipt->local_palette_hash) || !out_receipt->local_palette_hash ||
        !dm2_v1_boot_query_expanded_rect_receipt(
            profile, source->clip_rect_id, &rect)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->source = *source;
    out_receipt->raw_gfx256_bytes = raw.source_bytes;
    out_receipt->raw_gfx256_byte_count = raw.source_byte_count;
    out_receipt->raw_gfx256_hash = raw.source_hash;
    out_receipt->raw_gfx256_receipt_hash = raw.receipt_hash;
    out_receipt->clip_rect_id = source->clip_rect_id;
    out_receipt->raw4_hash = rect.raw4_hash;
    out_receipt->raw4_receipt_hash = rect.receipt_hash;
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, source->identity_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw.source_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw.receipt_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, out_receipt->local_palette_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, rect.raw4_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, rect.receipt_hash);
    out_receipt->identity_hash = hash ? hash : 1u;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_boot_g1_static_weapon_selector(
    const DM2_V1_BootProfile *profile, const DM2_V1_G1DirectWeaponRoot *root,
    DM2_V1_G1StaticObjectMaterialSelector *out_selector)
{
    const DM2_V1_BootGraphicsDat *gfx;
    uint16_t offset = 0;
    if (!profile || !profile->graphics_dat || !root) return 0;
    gfx = (const DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    /* SKWIN/SkWinCore.cpp DRAW_ITEM (tt == 0) reads the record's dtImageOffset
     * only at the default item index 0xFE, and QUERY_GDAT_ENTRY_DATA_INDEX
     * returns 0 when the entry is absent — the draw continues without an
     * offset.  A proven-absent entry therefore binds offset 0 instead of
     * blocking the weapon; the per-type offset entries are inventory-icon
     * material that the floor DRAW_ITEM route never consumes. */
    if (!dm2_v1_asset_load_image_offset(&gfx->loader, 0x10, 0xfe, 0u, &offset))
        offset = 0;
    return dm2_v1_g1_static_object_material_selector(root, offset, out_selector);
}

int dm2_v1_boot_g1_static_container_selector(
    const DM2_V1_BootProfile *profile, const DM2_V1_G1DirectContainerRoot *root,
    DM2_V1_G1StaticObjectMaterialSelector *out_selector)
{
    const DM2_V1_BootGraphicsDat *gfx;
    uint16_t offset = 0;
    uint8_t field;
    if (!profile || !profile->graphics_dat || !root) return 0;
    gfx = (const DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    field = root->opened ? 4u : 0u;
    /* Same DRAW_ITEM dtImageOffset rule as the weapon selector: default item
     * index 0xFE, proven-absent binds offset 0. */
    if (!dm2_v1_asset_load_image_offset(&gfx->loader, 0x14, 0xfe, field,
                                        &offset))
        offset = 0;
    return dm2_v1_g1_static_container_material_selector(root, offset, out_selector);
}

static int dm2_v1_boot_dialogue_text_width(const uint8_t *text, size_t size)
{
    size_t count = 0u;
    if (!text || size == 0u) return 0;
    while (count < size && text[count] && count < 80u) ++count;
    return count > 0u ? (int)count * 3 : 0;
}

int dm2_v1_boot_interface_hud_layout(DM2_V1_BootProfile *profile,
                                     DM2_V1_InterfaceHudLayout *out_layout)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint32_t hash = 2166136261u;
    if (!out_layout) return 0;
    memset(out_layout, 0, sizeof(*out_layout));
    if (!profile || !profile->graphics_dat) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(&gfx->loader, 1, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw_size);
    if (!raw || raw_size < 4u) return 0;
    for (size_t i = 0; i < raw_size; ++i) hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    for (uint16_t slot = 0; slot < DM2_V1_INTERFACE_HUD_CHAMPION_COUNT; ++slot) {
        if (!dm2_v1_boot_expand_hud_rect(raw, raw_size, (uint16_t)(173u + slot), &out_layout->portrait[slot]) ||
            !dm2_v1_boot_expand_hud_rect(raw, raw_size, (uint16_t)(165u + slot), &out_layout->name[slot])) return 0;
        for (uint16_t stat = 0; stat < 3u; ++stat)
            if (!dm2_v1_boot_expand_hud_rect(raw, raw_size, (uint16_t)(185u + slot + stat * 4u), &out_layout->status[slot][stat])) return 0;
    }
    out_layout->table_hash = hash; out_layout->valid = 1; return 1;
}

int dm2_v1_boot_interface_hud_portrait_destinations(
    DM2_V1_BootProfile *profile,
    DM2_V1_InterfaceRect out_portraits[DM2_V1_INTERFACE_HUD_CHAMPION_COUNT],
    uint32_t *out_table_hash)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint32_t hash = 2166136261u;
    uint16_t slot;

    if (!out_portraits || !out_table_hash || !profile || !profile->graphics_dat) {
        return 0;
    }
    memset(out_portraits, 0,
           sizeof(DM2_V1_InterfaceRect) * DM2_V1_INTERFACE_HUD_CHAMPION_COUNT);
    *out_table_hash = 0u;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(&gfx->loader, 1, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw_size);
    if (!raw || raw_size < 4u) return 0;
    for (size_t i = 0; i < raw_size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    }
    for (slot = 0; slot < DM2_V1_INTERFACE_HUD_CHAMPION_COUNT; ++slot) {
        if (!dm2_v1_boot_expand_hud_rect(raw, raw_size,
                                         (uint16_t)(173u + slot),
                                         &out_portraits[slot])) {
            memset(out_portraits, 0, sizeof(DM2_V1_InterfaceRect) *
                   DM2_V1_INTERFACE_HUD_CHAMPION_COUNT);
            return 0;
        }
    }
    *out_table_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_boot_dialogue_box_host_command(
    DM2_V1_BootProfile *profile,
    DM2_V1_DialogueBoxHostCommand *out_command)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint32_t hash = 2166136261u;

    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!profile || !profile->graphics_dat ||
        !dm2_v1_boot_dialogue_box_draw_plan(profile, &out_command->draw)) {
        return 0;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw_size);
    if (!raw || raw_size < 4u ||
        !dm2_v1_boot_expand_hud_rect(
            raw, raw_size, out_command->draw.expanded_rect_index,
            &out_command->rect)) {
        memset(out_command, 0, sizeof(*out_command));
        return 0;
    }

    /* skproject/SKULLWIN/c_dialog.cpp:57-64 expands RECT_453, then blits
     * DIALOG_BOXES/0x81/0 at exactly that origin with its local palette. */
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_command->draw.plan_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_command->rect.x);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_command->rect.y);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_command->rect.w);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_command->rect.h);
    out_command->command_hash = hash ? hash : 1u;
    out_command->valid = 1;
    return 1;
}

int dm2_v1_boot_dialogue_open_panel_host_command(
    DM2_V1_BootProfile *profile,
    DM2_V1_DialogueOpenPanelHostCommand *out_command)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    const DM2_V1_InterfaceRect *rects[4];
    size_t raw_size = 0u;
    uint32_t hash = 2166136261u;

    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!profile || !profile->graphics_dat) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!dm2_v1_dialogue_open_panel_receipt(&gfx->loader, &out_command->draw))
        return 0;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw_size);
    if (!raw || raw_size < 4u ||
        !dm2_v1_boot_expand_hud_rect(raw, raw_size,
            out_command->draw.panel_rect_index, &out_command->panel_rect) ||
        !dm2_v1_boot_query_blit_text_rect(raw, raw_size,
            out_command->draw.version_rect_index,
            dm2_v1_boot_dialogue_text_width(out_command->draw.version_text,
                                             out_command->draw.version_text_size),
            6, &out_command->version_text_rect) ||
        !dm2_v1_boot_query_blit_text_rect(raw, raw_size,
            DM2_V1_DIALOGUE_OPEN_PANEL_PRIMARY_TEXT_RECT,
            dm2_v1_boot_dialogue_text_width(out_command->draw.text[0],
                                             out_command->draw.text_size[0]),
            6, &out_command->primary_text_rect) ||
        !dm2_v1_boot_query_blit_text_rect(raw, raw_size,
            DM2_V1_DIALOGUE_OPEN_PANEL_SECONDARY_TEXT_RECT,
            dm2_v1_boot_dialogue_text_width(out_command->draw.text[1],
                                             out_command->draw.text_size[1]),
            6, &out_command->secondary_text_rect)) {
        memset(out_command, 0, sizeof(*out_command));
        return 0;
    }

    /* skproject/SKULLWIN/c_dialog.cpp:375-415 consumes the source version
     * string, GDAT button labels, panel image/local palette, and raw4
     * rectangles together. */
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_command->draw.receipt_hash);
    rects[0] = &out_command->panel_rect;
    rects[1] = &out_command->version_text_rect;
    rects[2] = &out_command->primary_text_rect;
    rects[3] = &out_command->secondary_text_rect;
    for (unsigned int i = 0; i < 4u; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, (uint32_t)rects[i]->x);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, (uint32_t)rects[i]->y);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, (uint32_t)rects[i]->w);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, (uint32_t)rects[i]->h);
    }
    out_command->command_hash = hash ? hash : 1u;
    out_command->valid = 1;
    return 1;
}

int dm2_v1_boot_dialogue_save_pointer_receipt(
    DM2_V1_BootProfile *profile,
    uint16_t event_rect_index,
    uint16_t event_top_left_index,
    int pointer_y,
    DM2_V1_DialogueSavePointerReceipt *out_receipt)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *raw;
    size_t raw_size = 0u;
    DM2_V1_InterfaceRect top_left;
    int row;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->graphics_dat || event_rect_index == 0u ||
        event_top_left_index == 0u) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw_size);
    /* c_dialog.cpp:191-198 and c_savegame.cpp:775-780: expand the event
     * rect, query the source top-left with a 1x1 metric, then clamp the row
     * by the immutable c_gfx_str.cpp strxplus=7 line stride. */
    if (!raw || raw_size < 4u ||
        !dm2_v1_boot_expand_hud_rect(raw, raw_size, event_rect_index,
                                     &out_receipt->event_rect) ||
        !dm2_v1_boot_query_blit_text_rect(raw, raw_size, event_top_left_index,
                                          1, 1, &top_left)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    if (pointer_y < out_receipt->event_rect.y + top_left.y) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    row = (pointer_y - (out_receipt->event_rect.y + top_left.y)) / 7;
    if (row > 10) row = 10;
    out_receipt->event_rect_index = event_rect_index;
    out_receipt->event_top_left_index = event_top_left_index;
    out_receipt->top_left_x = top_left.x;
    out_receipt->top_left_y = top_left.y;
    out_receipt->row_stride = 7;
    out_receipt->selected_slot = row;
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, event_rect_index);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, event_top_left_index);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                   (uint32_t)out_receipt->event_rect.x);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                   (uint32_t)out_receipt->event_rect.y);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)top_left.x);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)top_left.y);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)pointer_y);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, (uint32_t)row);
    out_receipt->command_hash = hash ? hash : 1u;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_boot_startup_menu_pointer_layout(
    DM2_V1_BootProfile *profile,
    DM2_V1_StartupMenuPointerLayout *out_layout)
{
    DM2_V1_BootGraphicsDat *gfx;
    DM2_V1_BootGraphicsDat *owned_gfx = NULL;
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint32_t hash = 2166136261u;

    if (!out_layout) return 0;
    memset(out_layout, 0, sizeof(*out_layout));
    if (!profile) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (!gfx && profile->graphics_path[0] != '\0') {
        /* SHOW_MENU_SCREEN owns this route before GAME_LOAD publishes the
         * runtime graphics handle. Re-open only the verified source file
         * named by the boot profile; do not promote a fallback surface. */
        owned_gfx = dm2_v1_boot_graphics_load(profile->graphics_path);
        gfx = owned_gfx;
    }
    if (!gfx) return 0;
    raw = dm2_v1_asset_load_typed_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw_size);
    if (!raw || raw_size < 4u) {
        dm2_v1_boot_graphics_free(owned_gfx);
        return 0;
    }
    for (size_t i = 0; i < raw_size; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, raw[i]);
    }
    /* skproject _098d_1208 -> LOAD_RECTS_AND_COMPRESS loads raw4, then
     * HANDLE_UI_EVENT uses the title-menu event codes 0xD7 and 0xD9. */
    if (!dm2_v1_boot_startup_menu_event_rect(raw, raw_size, 0x00d7u,
                                             &out_layout->new_game) ||
        !dm2_v1_boot_startup_menu_event_rect(raw, raw_size, 0x00d9u,
                                             &out_layout->resume_game)) {
        dm2_v1_boot_graphics_free(owned_gfx);
        return 0;
    }
    out_layout->table_hash = hash;
    out_layout->valid = 1;
    dm2_v1_boot_graphics_free(owned_gfx);
    return 1;
}

int dm2_v1_boot_startup_menu_pointer_hit(
    DM2_V1_BootProfile *profile,
    int x,
    int y,
    DM2_V1_StartupMenuPointerHitReceipt *out_receipt)
{
    DM2_V1_StartupMenuPointerLayout layout;

    if (!dm2_v1_boot_startup_menu_pointer_layout(profile, &layout)) {
        return 0;
    }
    return dm2_v1_boot_startup_menu_pointer_hit_from_layout(
        &layout, x, y, out_receipt);
}

int dm2_v1_boot_startup_menu_pointer_hit_from_layout(
    const DM2_V1_StartupMenuPointerLayout *layout,
    int x,
    int y,
    DM2_V1_StartupMenuPointerHitReceipt *out_receipt)
{
    DM2_V1_StartupMenuPointerHitReceipt candidate;

    if (!out_receipt || !layout || !layout->valid ||
        layout->table_hash == 0u) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.table_hash = layout->table_hash;
    if (dm2_v1_boot_startup_rect_contains(&layout->new_game, x, y)) {
        candidate.target = DM2_V1_STARTUP_POINTER_TARGET_NEW_GAME;
        candidate.rect = layout->new_game;
    } else if (dm2_v1_boot_startup_rect_contains(&layout->resume_game, x, y)) {
        candidate.target = DM2_V1_STARTUP_POINTER_TARGET_RESUME_GAME;
        candidate.rect = layout->resume_game;
    } else {
        return 0;
    }
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_boot_interface_rect14_table(
    DM2_V1_BootProfile *profile,
    const uint8_t **out_rows,
    uint32_t *out_row_count,
    uint32_t *out_hash)
{
    DM2_V1_BootGraphicsDat *gfx;
    const uint8_t *rows;
    size_t byte_count = 0;
    uint32_t hash = 2166136261u;

    if (out_rows) *out_rows = NULL;
    if (out_row_count) *out_row_count = 0u;
    if (out_hash) *out_hash = 0u;
    if (!profile || !profile->graphics_dat || !out_rows ||
        !out_row_count || !out_hash) return 0;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    rows = dm2_v1_asset_load_typed_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_INTERFACE_RAW_RECT14_TABLE,
        &byte_count);
    if (!rows || byte_count == 0u || byte_count > UINT32_MAX ||
        (byte_count % 14u) != 0u) return 0;
    for (size_t i = 0; i < byte_count; ++i) {
        hash = dm2_v1_boot_packaged_capture_hash_step(hash, rows[i]);
    }
    *out_rows = rows;
    *out_row_count = (uint32_t)(byte_count / 14u);
    *out_hash = hash;
    return *out_row_count > 0u;
}

int dm2_v1_boot_interface_rect14_host_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_InterfaceRect14HostReceipt *out_receipt)
{
    const uint8_t *rows;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_boot_interface_rect14_table(profile, &rows,
                                             &out_receipt->row_count,
                                             &out_receipt->table_hash) ||
        !rows ||
        !dm2_v1_boot_runtime_interface_rect14_placement_receipt(
            profile,
            &out_receipt->placement_hash,
            &out_receipt->placement_count,
            &out_receipt->rotated_cell_mask,
            &out_receipt->max_stretched_size)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* skproject/SKWIN/SkWinCore.cpp LOAD_GDAT_INTERFACE_00_0A supplies the
     * 14-byte rows; the host receives only its bounded placement proof. */
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_boot_load_gdat_interface_00_0a_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_LoadGdatInterface000AReceipt *out_receipt)
{
    DM2_V1_InterfaceRect14HostReceipt host;
    uint32_t hash = 0x4c303041u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_boot_interface_rect14_host_receipt(profile, &host) ||
        !host.valid || host.row_count == 0u || host.table_hash == 0u ||
        host.placement_hash == 0u ||
        host.placement_count < host.row_count ||
        host.rotated_cell_mask == 0u ||
        host.max_stretched_size == 0u) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->host_receipt_consumed = 1;
    out_receipt->table_hash = host.table_hash;
    out_receipt->row_count = host.row_count;
    out_receipt->stride = 14u;
    out_receipt->byte_count = host.row_count * 14u;
    out_receipt->placement_hash = host.placement_hash;
    out_receipt->placement_count = host.placement_count;
    out_receipt->rotated_cell_mask = host.rotated_cell_mask;
    out_receipt->max_stretched_size = host.max_stretched_size;
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, host.table_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, host.row_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, host.placement_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash, host.placement_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                  host.rotated_cell_mask);
    hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                  host.max_stretched_size);
    out_receipt->receipt_hash = hash ? hash : 1u;
    return 1;
}

static int dm2_v1_boot_runtime_decoded_gdat_hud_probe(
    DM2_V1_BootProfile *profile,
    int *out_portrait_count,
    uint32_t *out_portrait_hash,
    uint32_t *out_portrait_pixel_count,
    uint32_t *out_core_hash,
    uint32_t *out_core_pixel_count,
    int *out_interface_count)
{
    struct TypedEntry { int category; int index; int type; int field; };
    static const struct TypedEntry k_interface_entries[] = {
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_PAL_IRGB, DM2_GDAT_INTERFACE_PALETTE_FIELD },
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_PAL_16, DM2_GDAT_INTERFACE_PALETTE_FIELD },
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_INTERFACE_RAW_LAYOUT_TABLE },
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_INTERFACE_RAW_ACTION_TABLE },
        { DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
          DM2_GDAT_ENTRY_TYPE_RAW6, 0x00 }
    };
    int portrait_index;
    int portrait_count = 0;
    int interface_count = 0;
    uint32_t portrait_hash = 0x32485044u;
    uint32_t portrait_pixels = 0u;
    uint32_t core_hash = 0x32484344u;
    uint32_t core_pixels = 0u;

    if (out_portrait_count) *out_portrait_count = 0;
    if (out_portrait_hash) *out_portrait_hash = 0u;
    if (out_portrait_pixel_count) *out_portrait_pixel_count = 0u;
    if (out_core_hash) *out_core_hash = 0u;
    if (out_core_pixel_count) *out_core_pixel_count = 0u;
    if (out_interface_count) *out_interface_count = 0;
    if (!profile || !out_portrait_count || !out_portrait_hash ||
        !out_portrait_pixel_count || !out_core_hash ||
        !out_core_pixel_count || !out_interface_count) {
        return 0;
    }

    /* skproject/SKWIN DRAW_CHAMPION_PICTURE consumes decoded CHAMPIONS
     * GDAT images in the right HUD. Hash decoded pixels too, so a startup
     * HUD bridge cannot pass with raw bytes alone. */
    for (portrait_index = 0;
         portrait_index < DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT;
         ++portrait_index) {
        if (dm2_v1_boot_runtime_decoded_gdat_hash_add(
                profile,
                DM2_GDAT_CATEGORY_CHAMPIONS,
                portrait_index,
                DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD,
                &portrait_hash,
                &portrait_pixels)) {
            ++portrait_count;
        }
    }
    for (int i = 0; i < (int)(sizeof(k_interface_entries) /
                              sizeof(k_interface_entries[0])); ++i) {
        if (dm2_v1_boot_runtime_typed_raw_gdat_hash_add(
                profile,
                k_interface_entries[i].category,
                k_interface_entries[i].index,
                k_interface_entries[i].type,
                k_interface_entries[i].field,
                &core_hash,
                &core_pixels)) {
            ++interface_count;
        }
    }
    if (interface_count < 4 ||
        !dm2_v1_boot_runtime_decoded_gdat_hash_add(
            profile,
            DM2_GDAT_CATEGORY_GRAPHICSSET,
            0,
            DM2_GDAT_GFXSET_FLOOR,
            &core_hash,
            &core_pixels) ||
        !dm2_v1_boot_runtime_decoded_gdat_hash_add(
            profile,
            DM2_GDAT_CATEGORY_GRAPHICSSET,
            0,
            DM2_GDAT_GFXSET_CEIL,
            &core_hash,
            &core_pixels)) {
        return 0;
    }

    *out_portrait_count = portrait_count;
    *out_portrait_hash = portrait_hash;
    *out_portrait_pixel_count = portrait_pixels;
    *out_core_hash = core_hash;
    *out_core_pixel_count = core_pixels;
    *out_interface_count = interface_count;
    return portrait_count >= 4 &&
           portrait_hash != 0u &&
           portrait_pixels > 0u &&
           core_hash != 0u &&
           core_pixels > 0u;
}

int dm2_v1_boot_startup_real_visual_capture_receipt_from_runtime_state(
    DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupRealVisualCaptureReceipt *out_receipt)
{
    DM2_V1_BootRuntimeStartupSnapshot snapshot;
    DM2_V1_BootStartupViewModel view_model;
    DM2_V1_BootStartupPackagedFullStartReceipt package;
    DM2_V1_BootStartupPackagedConsumerReceipt consumer;
    DM2_V1_BootStartupHostFrameReceipt host_frame;
    DM2_V1_BootStartupRenderOwnershipReceipt ownership;
    uint8_t *title_pixels = NULL;
    uint8_t *menu_pixels = NULL;
    int title_w = 0;
    int title_h = 0;
    int title_stride = 0;
    int menu_w = 0;
    int menu_h = 0;
    int menu_stride = 0;
    uint32_t hash = 0x32475643u;
    int i;

    dm2_v1_boot_startup_real_visual_capture_receipt_init(out_receipt);
    if (!profile || !out_receipt) {
        return 0;
    }

    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.profile = profile;
    snapshot.startup_menu_active = startup_menu_active;
    snapshot.startup_save_root = startup_save_root;
    snapshot.resume_available = resume_available;
    snapshot.slot_mask = slot_mask;
    snapshot.selected_row = selected_row;

    if (!dm2_v1_boot_startup_view_model_receipt_from_snapshot_tick(
            &snapshot,
            title_animation_tick,
            &view_model) ||
        !dm2_v1_boot_startup_packaged_full_start_receipt_from_host_view(
            &view_model.host_view_receipt,
            &package) ||
        !dm2_v1_boot_startup_packaged_consumer_receipt_from_full_start(
            &package,
            &consumer) ||
        !dm2_v1_boot_startup_host_frame_receipt_from_consumer(
            &consumer,
            &host_frame) ||
        !dm2_v1_boot_startup_render_ownership_from_view_model(
            &snapshot,
            &view_model,
            &ownership)) {
        return 0;
    }

    out_receipt->profile_ready = profile->assets_verified ? 1 : 0;
    out_receipt->graphics_dat_ready = profile->graphics_dat ? 1 : 0;
    out_receipt->packaged_full_start_valid = package.valid;
    out_receipt->packaged_consumer_valid = consumer.valid;
    out_receipt->host_frame_valid = host_frame.valid;
    out_receipt->render_ownership_valid = ownership.valid;
    out_receipt->real_visual_capture_consumes_package =
        package.valid &&
        consumer.packaged_full_start_valid &&
        consumer.packaged_full_start_hash == package.packaged_full_start_hash;
    out_receipt->real_visual_capture_consumes_host_frame =
        host_frame.valid &&
        host_frame.consume_startup_package &&
        host_frame.packaged_full_start_hash == package.packaged_full_start_hash;
    out_receipt->packaged_status_consumed =
        consumer.status_scope == package.status_scope &&
        consumer.status == package.status &&
        host_frame.status_scope == package.status_scope &&
        host_frame.status == package.status;
    out_receipt->packaged_startup_phase_consumed =
        consumer.phase &&
        host_frame.phase &&
        strcmp(consumer.phase, "dm2-startup-menu") == 0 &&
        strcmp(host_frame.phase, consumer.phase) == 0 &&
        host_frame.render_startup_title &&
        host_frame.render_startup_menu;
    out_receipt->packaged_hud_suppression_consumed =
        host_frame.suppress_game_hud &&
        !host_frame.present_first_hud_frame &&
        consumer.startup_hud_runtime_ready &&
        (!consumer.full_start_real_asset_ready ||
         consumer.startup_hud_raw_gdat_capture_ready) &&
        consumer.startup_draw_hud_handoff_ready;
    out_receipt->packaged_full_start_hash = package.packaged_full_start_hash;
    out_receipt->packaged_consumer_hash = consumer.packaged_full_start_hash;
    out_receipt->phase = host_frame.phase;
    out_receipt->animation = host_frame.animation;
    out_receipt->real_gdat_title_asset_required = 1;
    out_receipt->real_gdat_menu_asset_required = 1;
    out_receipt->title_gdat_category = package.title_gdat_category;
    out_receipt->title_gdat_index = package.title_gdat_index;
    out_receipt->title_gdat_field = package.title_gdat_field;
    out_receipt->menu_gdat_category =
        view_model.view_receipt.render.menu_gdat_category;
    out_receipt->menu_gdat_index =
        view_model.view_receipt.render.menu_gdat_index;
    out_receipt->menu_gdat_field =
        view_model.view_receipt.render.menu_gdat_field;
    out_receipt->skproject_title_query_ready =
        view_model.view_receipt.render.skproject_title_query_ready;
    out_receipt->skproject_menu_query_ready =
        view_model.view_receipt.render.skproject_menu_query_ready;
    out_receipt->skproject_title_category =
        view_model.view_receipt.render.skproject_title_category;
    out_receipt->skproject_title_index =
        view_model.view_receipt.render.skproject_title_index;
    out_receipt->skproject_credit_screen_field =
        view_model.view_receipt.render.skproject_credit_screen_field;
    out_receipt->skproject_menu_screen_field =
        view_model.view_receipt.render.skproject_menu_screen_field;
    out_receipt->menu_capture_ready = package.menu_capture_ready;
    out_receipt->menu_command_count = view_model.command_count;
    out_receipt->menu_row_count = package.menu_row_count;
    out_receipt->selected_highlight_count =
        package.selected_highlight_count;
    out_receipt->resume_menu_ready =
        view_model.view_receipt.render.resume_menu_ready;
    out_receipt->save_slot_menu_ready =
        view_model.view_receipt.render.save_slot_menu_ready;
    out_receipt->new_game_menu_ready =
        view_model.view_receipt.render.new_game_menu_ready;
    out_receipt->hud_handoff_capture_ready =
        package.hud_handoff_capture_ready;
    out_receipt->suppress_game_hud = ownership.suppress_game_hud;
    out_receipt->present_first_hud_frame = ownership.present_first_hud_frame;
    out_receipt->exact_title_timing_ready =
        package.exact_title_timing_ready;
    out_receipt->title_animation_tick = package.title_animation_tick;
    out_receipt->title_frame = package.title_frame;
    out_receipt->title_frame_remaining_ticks =
        package.title_frame_remaining_ticks;
    out_receipt->no_fallback_title_blit =
        ownership.fallback_title_blit_used ? 0 : 1;
    out_receipt->status_scope = package.status_scope;
    out_receipt->status = package.status;

    for (i = 0; i < view_model.command_count; ++i) {
        const DM2_V1_StartupDrawCommand *command = &view_model.commands[i];
        if (command->kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE) {
            ++out_receipt->menu_gdat_command_count;
        } else if (command->kind == DM2_V1_STARTUP_DRAW_FILL_RECT ||
                   command->kind == DM2_V1_STARTUP_DRAW_OUTLINE_RECT) {
            ++out_receipt->menu_rect_command_count;
        } else if (command->kind == DM2_V1_STARTUP_DRAW_TEXT) {
            ++out_receipt->menu_text_command_count;
        }
    }

    out_receipt->menu_raw_screen_route_ready =
        dm2_v1_boot_startup_menu_raw_screen_receipt(
            profile,
            &out_receipt->menu_raw_screen_hash,
            &out_receipt->menu_raw_screen_byte_count);
    if (dm2_v1_boot_gdat_raw_asset_proof(profile,
                                         package.title_gdat_category,
                                         package.title_gdat_index,
                                         package.title_gdat_field,
                                         0x32545257u,
                                         &out_receipt->title_raw_byte_hash,
                                         &out_receipt->title_raw_byte_count)) {
        if (out_receipt->menu_raw_screen_route_ready) {
            out_receipt->menu_raw_byte_hash =
                out_receipt->menu_raw_screen_hash;
            out_receipt->menu_raw_byte_count =
                out_receipt->menu_raw_screen_byte_count;
        } else {
            (void)dm2_v1_boot_gdat_raw_asset_proof(
                profile,
                out_receipt->menu_gdat_category,
                out_receipt->menu_gdat_index,
                out_receipt->menu_gdat_field,
                0x324d5257u,
                &out_receipt->menu_raw_byte_hash,
                &out_receipt->menu_raw_byte_count);
        }
        out_receipt->raw_gdat_capture_ready =
            out_receipt->title_raw_byte_hash != 0u &&
            out_receipt->title_raw_byte_count > 0u &&
            out_receipt->menu_raw_byte_hash != 0u &&
            out_receipt->menu_raw_byte_count > 0u;
    }

    if (dm2_v1_boot_gdat_image_asset_fetch(profile,
                                           package.title_gdat_category,
                                           package.title_gdat_index,
                                           package.title_gdat_field,
                                           &title_pixels,
                                           &title_w,
                                           &title_h,
                                           &title_stride) == 0 &&
        title_pixels &&
        title_w == 320 &&
        title_h == 200 &&
        title_stride >= title_w) {
        size_t row;
        uint32_t pixel_hash = 0x32544954u;
        out_receipt->real_gdat_title_asset_consumed = 1;
        out_receipt->title_gdat_asset_w = title_w;
        out_receipt->title_gdat_asset_h = title_h;
        out_receipt->title_gdat_asset_stride = title_stride;
        out_receipt->title_pixel_count =
            (uint32_t)(title_w * title_h);
        for (row = 0; row < (size_t)title_h; ++row) {
            const uint8_t *src =
                title_pixels + row * (size_t)title_stride;
            int x;
            for (x = 0; x < title_w; ++x) {
                pixel_hash = dm2_v1_boot_packaged_capture_hash_step(
                    pixel_hash,
                    src[x]);
            }
        }
        out_receipt->title_pixel_hash = pixel_hash;
    }
    dm2_v1_boot_gdat_image_asset_free(title_pixels);

    if (out_receipt->menu_raw_screen_route_ready) {
        out_receipt->real_gdat_menu_asset_consumed = 1;
        out_receipt->menu_raw_screen_consumed = 1;
        out_receipt->menu_gdat_asset_w = 320;
        out_receipt->menu_gdat_asset_h = 200;
        out_receipt->menu_gdat_asset_stride = 320;
        out_receipt->menu_pixel_count = 320u * 200u;
        out_receipt->menu_pixel_hash =
            out_receipt->menu_raw_screen_hash;
    } else if (dm2_v1_boot_gdat_image_asset_fetch(profile,
                                                  out_receipt->menu_gdat_category,
                                                  out_receipt->menu_gdat_index,
                                                  out_receipt->menu_gdat_field,
                                                  &menu_pixels,
                                                  &menu_w,
                                                  &menu_h,
                                                  &menu_stride) == 0 &&
               menu_pixels &&
               menu_w == 320 &&
               menu_h == 200 &&
               menu_stride >= menu_w) {
        size_t row;
        uint32_t pixel_hash = 0x324d454eu;
        out_receipt->real_gdat_menu_asset_consumed = 1;
        out_receipt->menu_image_field_fallback_used = 1;
        out_receipt->menu_gdat_asset_w = menu_w;
        out_receipt->menu_gdat_asset_h = menu_h;
        out_receipt->menu_gdat_asset_stride = menu_stride;
        out_receipt->menu_pixel_count = (uint32_t)(menu_w * menu_h);
        for (row = 0; row < (size_t)menu_h; ++row) {
            const uint8_t *src = menu_pixels + row * (size_t)menu_stride;
            int x;
            for (x = 0; x < menu_w; ++x) {
                pixel_hash = dm2_v1_boot_packaged_capture_hash_step(
                    pixel_hash,
                    src[x]);
            }
        }
        out_receipt->menu_pixel_hash = pixel_hash;
    }
    dm2_v1_boot_gdat_image_asset_free(menu_pixels);

    out_receipt->title_capture_ready =
        out_receipt->real_gdat_title_asset_consumed &&
        out_receipt->title_pixel_hash != 0u &&
        out_receipt->title_pixel_count == 64000u;
    out_receipt->full_title_frame_capture_ready =
        out_receipt->title_capture_ready &&
        out_receipt->title_gdat_asset_w == 320 &&
        out_receipt->title_gdat_asset_h == 200 &&
        out_receipt->title_gdat_asset_stride >= 320;
    out_receipt->menu_gdat_capture_ready =
        out_receipt->real_gdat_menu_asset_consumed &&
        out_receipt->menu_pixel_hash != 0u &&
        out_receipt->menu_pixel_count == 64000u &&
        out_receipt->menu_gdat_asset_w == 320 &&
        out_receipt->menu_gdat_asset_h == 200 &&
        out_receipt->menu_gdat_asset_stride >= 320;
    out_receipt->menu_title_composite_capture_ready =
        out_receipt->full_title_frame_capture_ready &&
        out_receipt->menu_gdat_capture_ready &&
        out_receipt->menu_capture_ready &&
        /* skproject SHOW_MENU_SCREEN may consume the complete menu through
         * its verified raw screen instead of emitting Firestaff text/rect
         * commands.  That route has one title image command; accept it only
         * when the raw screen is actually consumed and the later composite
         * receipt proves no synthetic overlay. */
        ((out_receipt->menu_raw_screen_route_ready &&
          out_receipt->menu_raw_screen_consumed &&
          !out_receipt->menu_image_field_fallback_used &&
          out_receipt->menu_gdat_command_count >= 1) ||
         /* This verified PC GDAT uses the decoded IMAGE field when no raw
          * SHOW_MENU_SCREEN record is present.  It is still an original
          * 320x200 menu surface; the composite gate below remains responsible
          * for rejecting any generated text/rect overlay. */
         (!out_receipt->menu_raw_screen_route_ready &&
          out_receipt->menu_image_field_fallback_used &&
          out_receipt->menu_gdat_command_count >= 1));
    out_receipt->exact_selected_highlight_ready =
        out_receipt->selected_highlight_count == 1;
    (void)dm2_v1_boot_startup_composite_capture(profile,
                                                view_model.commands,
                                                view_model.command_count,
                                                out_receipt);
    out_receipt->startup_title_menu_hud_breadth_ready =
        out_receipt->menu_title_composite_capture_ready &&
        out_receipt->full_visual_composite_capture_ready &&
        (!resume_available || out_receipt->resume_menu_ready) &&
        (slot_mask == 0u || out_receipt->save_slot_menu_ready) &&
        out_receipt->new_game_menu_ready &&
        out_receipt->hud_handoff_capture_ready &&
        out_receipt->suppress_game_hud &&
        !out_receipt->present_first_hud_frame;
    out_receipt->title_menu_hud_visual_proof_ready =
        out_receipt->menu_title_composite_capture_ready &&
        out_receipt->full_visual_composite_capture_ready &&
        out_receipt->skproject_title_query_ready &&
        out_receipt->skproject_menu_query_ready &&
        (!out_receipt->menu_raw_screen_route_ready ||
         (out_receipt->menu_raw_screen_consumed &&
          !out_receipt->menu_image_field_fallback_used &&
          out_receipt->real_menu_screen_no_synthetic_overlay_ready)) &&
        out_receipt->hud_suppressed_capture_ready &&
        out_receipt->no_fallback_title_blit;
    (void)dm2_v1_boot_startup_real_visual_breadth_probe(profile,
                                                        &snapshot,
                                                        out_receipt);
    /* SKWINSPX skcore.cpp::SHOW_MENU_SCREEN presents TITLE/0/4 before
     * GAME_LOAD; TITLE/0/1 belongs only to SHOW_CREDITS. There is no source
     * party at this point,
     * so startup proves the HUD handoff/suppression only. Runtime HUD pixels
     * are verified by the post-GAME_LOAD capture path; do not fabricate a
     * four-portrait frame merely to satisfy this title-menu receipt. */
    out_receipt->real_visual_status_consumer_ready =
        out_receipt->real_visual_capture_consumes_package &&
        out_receipt->real_visual_capture_consumes_host_frame &&
        out_receipt->packaged_status_consumed &&
        out_receipt->packaged_startup_phase_consumed &&
        out_receipt->packaged_hud_suppression_consumed &&
        out_receipt->title_menu_hud_visual_proof_ready &&
        out_receipt->real_gdat_capture_breadth_ready;

    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, package.packaged_full_start_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, consumer.packaged_full_start_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, ownership.packaged_full_start_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->title_raw_byte_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_raw_byte_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->title_pixel_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_pixel_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_raw_screen_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_raw_screen_consumed);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_image_field_fallback_used);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->composite_pixel_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->composite_gdat_blit_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->synthetic_menu_overlay_suppressed);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->synthetic_menu_overlay_command_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->real_menu_screen_no_synthetic_overlay_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->hud_suppressed_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->skproject_title_query_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->skproject_menu_screen_field);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_command_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->menu_text_command_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->startup_title_menu_hud_breadth_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->hud_handoff_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->suppress_game_hud);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->real_visual_status_consumer_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->sampled_title_frame_mask);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->sampled_title_unique_pixel_hash_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->sampled_title_pixel_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->sampled_menu_selection_mask);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->sampled_menu_unique_composite_hash_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->sampled_menu_composite_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->sampled_runtime_hud_handoff_capture_ready);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud_direction_mask);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud_sample_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud_min_asset_portrait_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud_min_asset_floor_ceiling_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud_min_asset_wall_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud_raw_portrait_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud_raw_portrait_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud_raw_core_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud_decoded_portrait_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud_decoded_portrait_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud_decoded_core_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud_frame_hash);
    out_receipt->packaged_visual_capture_hash = hash;

    /* skproject/SKWIN startup draws the real title GDAT surface, menu
     * commands, and HUD handoff as one package. This receipt proves that
     * Firestaff consumed real GRAPHICS.DAT pixels instead of accepting the
     * synthetic ownership path alone. */
    out_receipt->valid =
        out_receipt->profile_ready &&
        out_receipt->graphics_dat_ready &&
        out_receipt->packaged_full_start_valid &&
        out_receipt->packaged_consumer_valid &&
        out_receipt->host_frame_valid &&
        out_receipt->render_ownership_valid &&
        out_receipt->real_visual_capture_consumes_package &&
        out_receipt->real_visual_capture_consumes_host_frame &&
        out_receipt->real_visual_status_consumer_ready &&
        out_receipt->real_gdat_capture_breadth_ready &&
        out_receipt->raw_gdat_capture_ready &&
        out_receipt->full_title_frame_capture_ready &&
        out_receipt->menu_title_composite_capture_ready &&
        out_receipt->full_visual_composite_capture_ready &&
        out_receipt->hud_suppressed_capture_ready &&
        out_receipt->title_menu_hud_visual_proof_ready &&
        out_receipt->exact_title_timing_ready &&
        out_receipt->packaged_visual_capture_hash != 0u;
    return out_receipt->valid;
}

int dm2_v1_boot_startup_presentation_receipt_from_runtime_state(
    int startup_menu_active,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    return dm2_v1_startup_presentation_receipt(
        startup_menu_active,
        out_phase,
        out_phase_size,
        out_startup_active,
        out_animation,
        out_animation_size,
        out_animation_active,
        out_title_frame,
        out_title_frame_max,
        out_title_ready);
}

int dm2_v1_boot_startup_presentation_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    if (!snapshot) {
        return 0;
    }
    return dm2_v1_boot_startup_presentation_receipt_from_runtime_state(
        snapshot->startup_menu_active,
        out_phase,
        out_phase_size,
        out_startup_active,
        out_animation,
        out_animation_size,
        out_animation_active,
        out_title_frame,
        out_title_frame_max,
        out_title_ready);
}

int dm2_v1_boot_startup_execute_draw_commands(
    const DM2_V1_StartupDrawCommand *commands,
    int command_count,
    const void *executor)
{
    return dm2_v1_startup_execute_draw_commands(
        commands,
        command_count,
        (const DM2_V1_StartupDrawExecutor *)executor);
}

int dm2_v1_boot_startup_execute_save_path_with_host_receipt(
    const char *save_path,
    int (*apply_session)(void *userdata,
                         const DM2_V1_SessionState *session),
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    void *out_direct_resume_receipt)
{
    return dm2_v1_startup_execute_save_path_with_host_receipt(
        save_path,
        apply_session,
        apply_userdata,
        out_execution,
        (DM2_V1_StartupDirectResumeReceipt *)out_direct_resume_receipt);
}

int dm2_v1_boot_startup_execute_launch_save_path_with_host_receipt(
    DM2_V1_BootStartupLaunch *launch,
    const char *save_path,
    int (*apply_session)(void *userdata,
                         const DM2_V1_SessionState *session),
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    void *out_direct_resume_receipt)
{
    DM2_V1_StartupDirectResumeReceipt *receipt =
        (DM2_V1_StartupDirectResumeReceipt *)out_direct_resume_receipt;
    if (!launch || !launch->profile || !receipt) {
        return 0;
    }
    if (!dm2_v1_boot_startup_execute_save_path_with_host_receipt(
            save_path,
            apply_session,
            apply_userdata,
            out_execution,
            receipt)) {
        return 0;
    }
    if (receipt->save_root_valid) {
        dm2_v1_boot_set_save_root(launch->profile, receipt->save_root);
    }
    return 1;
}

static void dm2_v1_boot_runtime_receipt_clear(
    DM2_V1_BootRuntimeReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
        receipt->operation_result = -1;
    }
}

int dm2_v1_boot_runtime_capture(DM2_V1_BootProfile *profile,
                                DM2_V1_BootRuntimeReceipt *out_receipt)
{
    DM2_V1_GameState *game;
    dm2_v1_boot_runtime_receipt_clear(out_receipt);
    if (!profile || !profile->dm2_state || !out_receipt) {
        return 0;
    }
    game = (DM2_V1_GameState *)profile->dm2_state;
    out_receipt->runtime_ready = 1;
    out_receipt->current_level = game->current_level;
    out_receipt->party_x = dm2_v1_runtime_get_party_x();
    out_receipt->party_y = dm2_v1_runtime_get_party_y();
    out_receipt->party_dir = dm2_v1_runtime_get_party_dir();
    out_receipt->tick_count = dm2_v1_runtime_get_tick_count();
    out_receipt->leader_hand_object = dm2_v1_runtime_get_leader_hand_object();
    return 1;
}

int dm2_v1_boot_runtime_tick(DM2_V1_BootProfile *profile,
                             DM2_V1_BootRuntimeReceipt *out_receipt)
{
    if (!profile || !profile->dm2_state) {
        dm2_v1_boot_runtime_receipt_clear(out_receipt);
        return 0;
    }
    dm2_v1_runtime_tick();
    if (!dm2_v1_boot_runtime_capture(profile, out_receipt)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->operation_result = 0;
    }
    return 1;
}

int dm2_v1_boot_runtime_turn(DM2_V1_BootProfile *profile,
                             int delta,
                             DM2_V1_BootRuntimeReceipt *out_receipt)
{
    int result;
    if (!profile || !profile->dm2_state) {
        dm2_v1_boot_runtime_receipt_clear(out_receipt);
        return 0;
    }
    result = dm2_v1_runtime_turn(delta);
    (void)dm2_v1_boot_runtime_capture(profile, out_receipt);
    if (out_receipt) {
        out_receipt->operation_result = result;
    }
    return result == 0;
}

int dm2_v1_boot_runtime_move(DM2_V1_BootProfile *profile,
                             int direction,
                             DM2_V1_BootRuntimeReceipt *out_receipt)
{
    int result;
    if (!profile || !profile->dm2_state) {
        dm2_v1_boot_runtime_receipt_clear(out_receipt);
        return 0;
    }
    result = dm2_v1_runtime_move(direction);
    (void)dm2_v1_boot_runtime_capture(profile, out_receipt);
    if (out_receipt) {
        out_receipt->operation_result = result;
    }
    return 1;
}

static void dm2_v1_boot_runtime_action_receipt_clear(
    DM2_V1_BootRuntimeActionReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
        receipt->action_kind = DM2_V1_BOOT_ACTION_NO_TARGET;
        receipt->target_square = -1;
        receipt->status_scope = "ACTION";
        receipt->status = "DM2 NO TARGET";
    }
}

int dm2_v1_boot_runtime_action_front_cell(
    DM2_V1_BootProfile *profile,
    int direction,
    DM2_V1_BootRuntimeActionReceipt *out_receipt)
{
    DM2_V1_GameState *game;
    DM2_V1_BootRuntimeReceipt runtime;
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int dir = direction & 3;
    int level;
    int fx;
    int fy;
    int square;

    dm2_v1_boot_runtime_action_receipt_clear(out_receipt);
    if (!profile || !profile->dm2_state || !out_receipt) {
        return 0;
    }
    game = (DM2_V1_GameState *)profile->dm2_state;
    if (!dm2_v1_boot_runtime_capture(profile, &runtime)) {
        return 0;
    }
    level = game->current_level;
    fx = runtime.party_x + dx[dir];
    fy = runtime.party_y + dy[dir];
    square = dm2_v1_runtime_get_square_type(level, fx, fy);

    out_receipt->runtime = runtime;
    out_receipt->target_level = level;
    out_receipt->target_x = fx;
    out_receipt->target_y = fy;
    out_receipt->target_square = square;
    out_receipt->status_scope = "ACTION";

    if (dm2_v1_runtime_enter_shop(level, fx, fy) == 0) {
        out_receipt->action_kind = DM2_V1_BOOT_ACTION_SHOP;
        out_receipt->status = "DM2 SHOP";
        out_receipt->reset_shop_selection = 1;
    } else if (square >= 0) {
        if (square == 4 &&
            dm2_v1_runtime_door_action(level, fx, fy, dir, 0) == 0) {
            out_receipt->action_kind = DM2_V1_BOOT_ACTION_DOOR;
            out_receipt->status = "DM2 DOOR";
        } else if (dm2_v1_runtime_npc_interact(level, fx, fy) == 0) {
            int npc_id = dm2_v1_runtime_get_last_npc_id();
            int npc_line_index = dm2_v1_runtime_get_last_npc_dialog_line();
            out_receipt->action_kind = DM2_V1_BOOT_ACTION_NPC;
            out_receipt->status = "DM2 INTERACT";
            out_receipt->inspect_title = dm2_v1_npc_get_name(npc_id);
            out_receipt->inspect_text =
                dm2_v1_npc_get_dialog(npc_id, npc_line_index);
        } else if (dm2_v1_runtime_invoke_square_actuators(level, fx, fy) > 0) {
            out_receipt->action_kind = DM2_V1_BOOT_ACTION_ACTUATOR;
            out_receipt->status = "DM2 ACTUATOR";
        } else {
            out_receipt->action_kind = DM2_V1_BOOT_ACTION_NO_ACTION;
            out_receipt->status = "DM2 NO ACTION";
        }
    } else {
        out_receipt->action_kind = DM2_V1_BOOT_ACTION_NO_TARGET;
        out_receipt->status = "DM2 NO TARGET";
    }
    (void)dm2_v1_boot_runtime_capture(profile, &out_receipt->runtime);
    return 1;
}

static void dm2_v1_boot_runtime_inventory_receipt_clear(
    DM2_V1_BootRuntimeInventoryReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
        receipt->champion_index = -1;
        receipt->champion_slot = -1;
        receipt->status_scope = "INVENTORY";
        receipt->status = "DM2 NO OBJECT";
    }
}

int dm2_v1_boot_runtime_swap_inventory_slot(
    DM2_V1_BootProfile *profile,
    int champion_index,
    int champion_slot,
    DM2_V1_BootRuntimeInventoryReceipt *out_receipt)
{
    uint32_t slot_object;
    uint32_t leader_object;
    dm2_v1_boot_runtime_inventory_receipt_clear(out_receipt);
    if (!profile || !profile->dm2_state || !out_receipt ||
        champion_index < 0 || champion_index >= 4 ||
        champion_slot < 0 || champion_slot >= 30) {
        return 0;
    }
    slot_object = dm2_v1_runtime_get_champion_inventory_object(
        (uint8_t)champion_index,
        (uint8_t)champion_slot);
    leader_object = dm2_v1_runtime_get_leader_hand_object();
    out_receipt->champion_index = champion_index;
    out_receipt->champion_slot = champion_slot;
    out_receipt->slot_object_before = slot_object;
    out_receipt->leader_hand_before = leader_object;
    if (slot_object == 0u && leader_object == 0u) {
        return 0;
    }
    if (dm2_v1_runtime_set_champion_inventory_object(
            (uint8_t)champion_index,
            (uint8_t)champion_slot,
            leader_object) != 0) {
        return 0;
    }
    dm2_v1_runtime_set_leader_hand_object(slot_object);
    out_receipt->slot_object_after = leader_object;
    out_receipt->leader_hand_after = slot_object;
    if (out_receipt->leader_hand_after != 0u && leader_object != 0u) {
        out_receipt->status = "DM2 SWAP";
    } else if (out_receipt->leader_hand_after != 0u) {
        out_receipt->status = "DM2 PICKUP";
    } else {
        out_receipt->status = "DM2 PLACE";
    }
    (void)dm2_v1_boot_runtime_capture(profile, &out_receipt->runtime);
    return 1;
}

static void dm2_v1_boot_runtime_render_receipt_clear(
    DM2_V1_BootRuntimeRenderReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
        receipt->render_result = -1;
    }
}

static uint32_t dm2_v1_boot_runtime_hud_hash_frame(const uint8_t *framebuffer,
                                                   int stride,
                                                   int width,
                                                   int height)
{
    uint32_t hash = 0x32485544u;
    int y;
    if (!framebuffer || stride <= 0 || width <= 0 || height <= 0) {
        return 0u;
    }
    for (y = 0; y < height; ++y) {
        int x;
        const uint8_t *row = framebuffer + (size_t)y * (size_t)stride;
        for (x = 0; x < width; ++x) {
            hash = dm2_v1_boot_packaged_capture_hash_step(hash, row[x]);
        }
    }
    return hash;
}

int dm2_v1_boot_runtime_render_frame(
    DM2_V1_BootProfile *profile,
    uint8_t *framebuffer,
    int fb_stride,
    int view_w,
    int view_h,
    DM2_V1_BootRuntimeRenderCallback v2_render,
    void *v2_userdata,
    DM2_V1_BootRuntimeRenderReceipt *out_receipt)
{
    DM2_V1_BootRuntimeReceipt runtime;
    DM2_V1_RuntimeFrameOwnershipReceipt frame_ownership;
    DM2_V1_ViewportM11FrameReceipt m11_frame;
    DM2_V1_RuntimeRawSaveHandoffReceipt raw_sksave_handoff;
    int rendered = -1;
    dm2_v1_boot_runtime_render_receipt_clear(out_receipt);
    if (!profile || !profile->dm2_state || !framebuffer) {
        return 0;
    }
    if (!dm2_v1_boot_runtime_capture(profile, &runtime)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->runtime = runtime;
        out_receipt->startup_title_ready = 1;
        out_receipt->startup_profile_verified =
            profile->assets_verified ? 1 : 0;
        out_receipt->startup_hud_runtime_ready = 1;
    }
    if (v2_render) {
        if (out_receipt) {
            out_receipt->v2_attempted = 1;
        }
        rendered = v2_render(runtime.party_dir,
                             runtime.party_x,
                             runtime.party_y,
                             framebuffer,
                             fb_stride,
                             view_w,
                             view_h,
                             v2_userdata);
        if (out_receipt && rendered == 0) {
            out_receipt->v2_succeeded = 1;
        }
    }
    if (rendered != 0) {
        if (out_receipt) {
            out_receipt->v1_attempted = 1;
        }
        rendered = dm2_v1_runtime_render_frame(runtime.party_dir,
                                               runtime.party_x,
                                               runtime.party_y,
                                               framebuffer,
                                               fb_stride,
                                               view_w,
                                               view_h);
        if (out_receipt && rendered == 0) {
            out_receipt->v1_succeeded = 1;
        }
    }
    /* Keep the ownership decision for the exact frame this API presented.
     * The later raw/decoded HUD probes render independent sample frames and
     * must not replace this frame's no-fallback/no-block decision. */
    memset(&frame_ownership, 0, sizeof(frame_ownership));
    memset(&m11_frame, 0, sizeof(m11_frame));
    memset(&raw_sksave_handoff, 0, sizeof(raw_sksave_handoff));
    (void)dm2_v1_runtime_last_frame_ownership(&frame_ownership);
    (void)dm2_v1_runtime_last_m11_frame_receipt(&m11_frame);
    (void)dm2_v1_runtime_last_raw_sksave_handoff_receipt(
        &raw_sksave_handoff);
    if (out_receipt) {
        out_receipt->render_result = rendered;
        out_receipt->startup_render_ready =
            rendered == 0 &&
            out_receipt->runtime.runtime_ready &&
            out_receipt->startup_title_ready &&
            out_receipt->startup_profile_verified &&
            out_receipt->startup_hud_runtime_ready;
        out_receipt->runtime_hud_asset_portrait_count =
            dm2_v1_runtime_last_asset_hud_portrait_count();
        out_receipt->runtime_hud_fallback_portrait_count =
            dm2_v1_runtime_last_fallback_hud_portrait_count();
        /* An empty original squad owns no portrait draw.  The viewport
         * blocks any occupied slot without a source HeroType, so zero
         * fallback draws is the correct no-invention receipt here. */
        out_receipt->runtime_hud_no_fallback_portraits =
            out_receipt->runtime_hud_fallback_portrait_count == 0;
        out_receipt->runtime_hud_raw_gdat_capture_ready =
            dm2_v1_boot_runtime_raw_gdat_hud_probe(
                profile,
                &out_receipt->runtime_hud_raw_portrait_count,
                &out_receipt->runtime_hud_raw_portrait_hash,
                &out_receipt->runtime_hud_raw_portrait_byte_count,
                &out_receipt->runtime_hud_raw_core_hash,
                &out_receipt->runtime_hud_raw_core_byte_count,
                &out_receipt->runtime_hud_raw_interface_count);
        out_receipt->runtime_hud_decoded_gdat_capture_ready =
            dm2_v1_boot_runtime_decoded_gdat_hud_probe(
                profile,
                &out_receipt->runtime_hud_decoded_portrait_count,
                &out_receipt->runtime_hud_decoded_portrait_hash,
                &out_receipt->runtime_hud_decoded_portrait_pixel_count,
                &out_receipt->runtime_hud_decoded_core_hash,
                &out_receipt->runtime_hud_decoded_core_pixel_count,
                &out_receipt->runtime_hud_decoded_interface_count);
        out_receipt->runtime_hud_frame_hash =
            dm2_v1_boot_runtime_hud_hash_frame(framebuffer,
                                               fb_stride,
                                               view_w,
                                               view_h);
        out_receipt->runtime_hud_frame_pixel_count =
            (uint32_t)(view_w * view_h);
        out_receipt->runtime_hud_real_asset_ready =
            profile->graphics_dat &&
            out_receipt->runtime_hud_no_fallback_portraits &&
            out_receipt->runtime_hud_raw_gdat_capture_ready &&
            out_receipt->runtime_hud_decoded_gdat_capture_ready &&
            out_receipt->runtime_hud_raw_interface_count >= 4 &&
            out_receipt->runtime_hud_decoded_interface_count >= 4 &&
            out_receipt->runtime_hud_frame_hash != 0u;
        out_receipt->runtime_hud_capture_ready =
            out_receipt->startup_render_ready &&
            out_receipt->runtime_hud_real_asset_ready;
        out_receipt->runtime_render_asset_floor_ceiling_count =
            dm2_v1_runtime_last_asset_floor_ceiling_count();
        out_receipt->runtime_raw_sksave_handoff_consumed =
            raw_sksave_handoff.valid &&
            raw_sksave_handoff.first_frame_consumed;
        out_receipt->runtime_raw_sksave_prefix_hash =
            raw_sksave_handoff.valid ? raw_sksave_handoff.prefix_hash : 0u;
        out_receipt->runtime_raw_sksave_map_data_hash =
            raw_sksave_handoff.valid ? raw_sksave_handoff.map_data_hash : 0u;
        out_receipt->runtime_raw_sksave_dungeon_byte_count =
            raw_sksave_handoff.valid &&
            raw_sksave_handoff.dungeon_byte_count <= UINT32_MAX
                ? (uint32_t)raw_sksave_handoff.dungeon_byte_count : 0u;
        if (raw_sksave_handoff.valid) {
            int type;
            for (type = 0; type < DM2_RAW_SKSAVE_DB_POOL_COUNT; ++type) {
                out_receipt->runtime_raw_sksave_db_record_count +=
                    raw_sksave_handoff.db_record_counts[type];
            }
        }
        out_receipt->runtime_render_fallback_floor_ceiling_count =
            dm2_v1_runtime_last_fallback_floor_ceiling_count();
        out_receipt->runtime_render_asset_wall_count =
            dm2_v1_runtime_last_asset_wall_count();
        out_receipt->runtime_render_fallback_wall_count =
            dm2_v1_runtime_last_fallback_wall_count();
        out_receipt->runtime_render_asset_door_panel_count =
            dm2_v1_runtime_last_asset_door_panel_count();
        out_receipt->runtime_render_asset_door_overlay_count =
            dm2_v1_runtime_last_asset_door_overlay_count();
        out_receipt->runtime_render_asset_door_frame_count =
            dm2_v1_runtime_last_asset_door_frame_count();
        out_receipt->runtime_render_asset_door_button_count =
            dm2_v1_runtime_last_asset_door_button_count();
        out_receipt->runtime_render_fallback_door_count =
            dm2_v1_runtime_last_fallback_door_count();
        out_receipt->runtime_render_asset_creature_count =
            dm2_v1_runtime_last_asset_creature_count();
        out_receipt->runtime_render_fallback_creature_count =
            dm2_v1_runtime_last_fallback_creature_count();
        out_receipt->runtime_render_asset_item_count =
            dm2_v1_runtime_last_asset_item_count();
        out_receipt->runtime_render_fallback_item_count =
            dm2_v1_runtime_last_fallback_item_count();
        out_receipt->runtime_render_asset_creature_possession_item_count =
            dm2_v1_runtime_last_asset_creature_possession_item_count();
        out_receipt->runtime_render_fallback_creature_possession_item_count =
            dm2_v1_runtime_last_fallback_creature_possession_item_count();
        out_receipt->runtime_render_asset_carried_item_count =
            dm2_v1_runtime_last_asset_carried_item_count();
        out_receipt->runtime_render_fallback_carried_item_count =
            dm2_v1_runtime_last_fallback_carried_item_count();
        out_receipt->runtime_render_asset_projectile_count =
            dm2_v1_runtime_last_asset_projectile_count();
        out_receipt->runtime_render_fallback_projectile_count =
            dm2_v1_runtime_last_fallback_projectile_count();
        /* Indoor frames must consume floor, ceiling, and at least one wall.
         * Outdoor frames consume sky and ground planes instead; no wall pass
         * is expected. */
        out_receipt->runtime_render_no_core_fallbacks =
            out_receipt->runtime_render_asset_floor_ceiling_count >= 2 &&
            out_receipt->runtime_render_fallback_floor_ceiling_count == 0 &&
            (frame_ownership.is_outdoor ||
             out_receipt->runtime_render_asset_wall_count > 0) &&
            out_receipt->runtime_render_fallback_wall_count == 0 &&
            out_receipt->runtime_render_fallback_door_count == 0 &&
            out_receipt->runtime_render_fallback_creature_count == 0 &&
            out_receipt->runtime_render_fallback_item_count == 0 &&
            out_receipt->runtime_render_fallback_creature_possession_item_count == 0 &&
            out_receipt->runtime_render_fallback_carried_item_count == 0 &&
            out_receipt->runtime_render_fallback_projectile_count == 0;
        out_receipt->runtime_render_real_asset_ready =
            out_receipt->runtime_hud_capture_ready &&
            out_receipt->runtime_render_no_core_fallbacks;
        out_receipt->runtime_m11_frame_receipt_consumed =
            m11_frame.valid && m11_frame.m11_consume_frame;
        out_receipt->runtime_m11_frame_map_load_token =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.map_load_token : 0u;
        out_receipt->runtime_m11_frame_scene_control_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.scene_control_hash : 0u;
        out_receipt->runtime_m11_frame_scene_light_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.scene_light_hash : 0u;
        out_receipt->runtime_m11_frame_presentation_state_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.presentation_state_hash : 0u;
        out_receipt->runtime_m11_frame_scene_ambient_light =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.scene_ambient_light : 0u;
        out_receipt->runtime_m11_frame_weather_graphicsset_bound =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.weather_graphicsset_bound : 0;
        out_receipt->runtime_m11_frame_weather_graphicsset =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.weather_graphicsset : 0u;
        out_receipt->runtime_m11_frame_weather_source_receipt_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.weather_source_receipt_hash : 0u;
        out_receipt->runtime_m11_frame_weather_destination_receipt_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.weather_destination_receipt_hash : 0u;
        out_receipt->runtime_m11_frame_floor_material_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.floor_material_hash : 0u;
        out_receipt->runtime_m11_frame_ceiling_material_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.ceiling_material_hash : 0u;
        out_receipt->runtime_m11_frame_wall_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.wall_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_wall_material_plan_command_count =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.wall_material_plan_command_count : 0;
        out_receipt->runtime_m11_frame_door_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.door_material_plan_required : 0;
        out_receipt->runtime_m11_frame_door_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.door_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_door_material_plan_command_count =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.door_material_plan_command_count : 0;
        out_receipt->runtime_m11_frame_door_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.door_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_hud_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.hud_material_plan_required : 0;
        out_receipt->runtime_m11_frame_hud_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.hud_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_hud_scene_control_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.hud_scene_control_hash : 0u;
        out_receipt->runtime_m11_frame_hud_material_plan_command_count =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.hud_material_plan_command_count : 0;
        out_receipt->runtime_m11_frame_hud_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.hud_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_creature_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.creature_material_plan_required : 0;
        out_receipt->runtime_m11_frame_creature_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.creature_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_creature_material_plan_command_count =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.creature_material_plan_command_count : 0;
        out_receipt->runtime_m11_frame_creature_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.creature_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_projectile_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.projectile_material_plan_required : 0;
        out_receipt->runtime_m11_frame_projectile_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.projectile_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_projectile_material_plan_command_count =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.projectile_material_plan_command_count : 0;
        out_receipt->runtime_m11_frame_projectile_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.projectile_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_item_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.item_material_plan_required : 0;
        out_receipt->runtime_m11_frame_item_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.item_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_item_scene_control_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.item_scene_control_hash : 0u;
        out_receipt->runtime_m11_frame_item_material_plan_command_count =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.item_material_plan_command_count : 0;
        out_receipt->runtime_m11_frame_item_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.item_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_weather_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.weather_material_plan_required : 0;
        out_receipt->runtime_m11_frame_weather_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.weather_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_weather_material_plan_command_count =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.weather_material_plan_command_count : 0;
        out_receipt->runtime_m11_frame_weather_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.weather_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_teleporter_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.teleporter_material_plan_required : 0;
        out_receipt->runtime_m11_frame_teleporter_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.teleporter_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_teleporter_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.teleporter_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_floor_gfx_map_chip_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.floor_gfx_map_chip_material_plan_required : 0;
        out_receipt->runtime_m11_frame_floor_gfx_map_chip_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.floor_gfx_map_chip_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_floor_gfx_map_chip_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.floor_gfx_map_chip_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_wall_gfx_map_chip_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.wall_gfx_map_chip_material_plan_required : 0;
        out_receipt->runtime_m11_frame_wall_gfx_map_chip_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.wall_gfx_map_chip_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_wall_gfx_map_chip_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.wall_gfx_map_chip_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_door_map_chip_material_plan_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.door_map_chip_material_plan_required : 0;
        out_receipt->runtime_m11_frame_door_map_chip_material_plan_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.door_map_chip_material_plan_hash : 0u;
        out_receipt->runtime_m11_frame_door_map_chip_material_plan_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.door_map_chip_material_plan_consumed : 0;
        out_receipt->runtime_m11_frame_palette_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.palette_hash : 0u;
        out_receipt->runtime_m11_frame_interface_action_palette_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.interface_action_palette_hash : 0u;
        out_receipt->runtime_m11_frame_interface_action_palette_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.interface_action_palette_consumed : 0;
        out_receipt->runtime_m11_frame_interface_rect14_required =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.interface_rect14_required : 0;
        out_receipt->runtime_m11_frame_interface_rect14_consumed =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.interface_rect14_consumed : 0;
        out_receipt->runtime_m11_frame_interface_rect14_table_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.interface_rect14_table_hash : 0u;
        out_receipt->runtime_m11_frame_interface_rect14_placement_hash =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.interface_rect14_placement_hash : 0u;
        out_receipt->runtime_m11_frame_interface_rect14_row_count =
            out_receipt->runtime_m11_frame_receipt_consumed ?
            m11_frame.interface_rect14_row_count : 0u;
    }
    return rendered == 0;
}

void dm2_v1_boot_runtime_hud_capture_receipt_init(
    DM2_V1_BootRuntimeHudCaptureReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

int dm2_v1_boot_runtime_hud_capture_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_BootRuntimeHudCaptureReceipt *out_receipt)
{
    uint8_t framebuffer[320 * 200];
    uint32_t frame_hashes[4];
    uint32_t combined_hash = 0x32485543u;
    DM2_V1_BootRuntimeReceipt initial_runtime;
    int dir;
    int frame_hash_count = 0;

    dm2_v1_boot_runtime_hud_capture_receipt_init(out_receipt);
    if (!profile || !profile->dm2_state || !out_receipt) {
        return 0;
    }

    out_receipt->profile_ready = profile->assets_verified ? 1 : 0;
    out_receipt->graphics_dat_ready = profile->graphics_dat ? 1 : 0;
    out_receipt->runtime_ready = 1;
    out_receipt->min_asset_portrait_count = 9999;
    out_receipt->min_asset_floor_ceiling_count = 9999;
    out_receipt->min_asset_wall_count = 9999;
    if (!dm2_v1_boot_runtime_capture(profile, &initial_runtime)) {
        return 0;
    }

    /* skproject/SKWIN T560 renders the same right-side runtime HUD from
     * party state while the dungeon view changes by direction. Sampling all
     * directions proves Firestaff keeps HUD portraits GDAT-backed through the
     * real runtime frame path, not only through startup handoff receipts. */
    for (dir = 0; dir < 4; ++dir) {
        DM2_V1_BootRuntimeRenderReceipt frame_receipt;
        DM2_V1_BootRuntimeReceipt turn_receipt;
        int seen_hash = 0;
        int hash_i;
        memset(framebuffer, 0, sizeof(framebuffer));
        if (!dm2_v1_boot_runtime_render_frame(profile,
                                              framebuffer,
                                              320,
                                              320,
                                              200,
                                              NULL,
                                              NULL,
                                              &frame_receipt)) {
            continue;
        }
        ++out_receipt->render_sample_count;
        out_receipt->sampled_direction_mask |= 1 << (dir & 3);
        out_receipt->runtime_direction_mask |=
            1 << (frame_receipt.runtime.party_dir & 3);
        if (frame_receipt.render_result == 0 &&
            frame_receipt.runtime_hud_capture_ready) {
            ++out_receipt->render_success_count;
        }
        for (hash_i = 0; hash_i < frame_hash_count; ++hash_i) {
            if (frame_hashes[hash_i] ==
                frame_receipt.runtime_hud_frame_hash) {
                seen_hash = 1;
                break;
            }
        }
        if (!seen_hash && frame_hash_count < 4) {
            frame_hashes[frame_hash_count++] =
                frame_receipt.runtime_hud_frame_hash;
        }
        out_receipt->total_asset_portrait_count +=
            frame_receipt.runtime_hud_asset_portrait_count;
        out_receipt->total_fallback_portrait_count +=
            frame_receipt.runtime_hud_fallback_portrait_count;
        out_receipt->total_asset_floor_ceiling_count +=
            frame_receipt.runtime_render_asset_floor_ceiling_count;
        out_receipt->total_fallback_floor_ceiling_count +=
            frame_receipt.runtime_render_fallback_floor_ceiling_count;
        out_receipt->total_asset_wall_count +=
            frame_receipt.runtime_render_asset_wall_count;
        out_receipt->total_fallback_wall_count +=
            frame_receipt.runtime_render_fallback_wall_count;
        out_receipt->total_asset_door_panel_count +=
            frame_receipt.runtime_render_asset_door_panel_count;
        out_receipt->total_asset_door_overlay_count +=
            frame_receipt.runtime_render_asset_door_overlay_count;
        out_receipt->total_asset_door_frame_count +=
            frame_receipt.runtime_render_asset_door_frame_count;
        out_receipt->total_asset_door_button_count +=
            frame_receipt.runtime_render_asset_door_button_count;
        out_receipt->total_fallback_door_count +=
            frame_receipt.runtime_render_fallback_door_count;
        out_receipt->total_asset_creature_count +=
            frame_receipt.runtime_render_asset_creature_count;
        out_receipt->total_fallback_creature_count +=
            frame_receipt.runtime_render_fallback_creature_count;
        out_receipt->total_asset_item_count +=
            frame_receipt.runtime_render_asset_item_count;
        out_receipt->total_fallback_item_count +=
            frame_receipt.runtime_render_fallback_item_count;
        out_receipt->total_asset_creature_possession_item_count +=
            frame_receipt
                .runtime_render_asset_creature_possession_item_count;
        out_receipt->total_fallback_creature_possession_item_count +=
            frame_receipt
                .runtime_render_fallback_creature_possession_item_count;
        out_receipt->total_asset_carried_item_count +=
            frame_receipt.runtime_render_asset_carried_item_count;
        out_receipt->total_fallback_carried_item_count +=
            frame_receipt.runtime_render_fallback_carried_item_count;
        out_receipt->total_asset_projectile_count +=
            frame_receipt.runtime_render_asset_projectile_count;
        out_receipt->total_fallback_projectile_count +=
            frame_receipt.runtime_render_fallback_projectile_count;
        if (frame_receipt.runtime_hud_asset_portrait_count <
            out_receipt->min_asset_portrait_count) {
            out_receipt->min_asset_portrait_count =
                frame_receipt.runtime_hud_asset_portrait_count;
        }
        if (frame_receipt.runtime_render_asset_floor_ceiling_count <
            out_receipt->min_asset_floor_ceiling_count) {
            out_receipt->min_asset_floor_ceiling_count =
                frame_receipt.runtime_render_asset_floor_ceiling_count;
        }
        if (frame_receipt.runtime_render_asset_wall_count <
            out_receipt->min_asset_wall_count) {
            out_receipt->min_asset_wall_count =
                frame_receipt.runtime_render_asset_wall_count;
        }
        if (frame_receipt.runtime_hud_asset_portrait_count >
            out_receipt->max_asset_portrait_count) {
            out_receipt->max_asset_portrait_count =
                frame_receipt.runtime_hud_asset_portrait_count;
        }
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            frame_receipt.runtime_hud_frame_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_hud_asset_portrait_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_asset_floor_ceiling_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_asset_wall_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_asset_door_panel_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_asset_door_frame_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_fallback_door_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_asset_item_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_fallback_item_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_asset_carried_item_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_fallback_carried_item_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_asset_creature_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime_render_asset_projectile_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            (uint32_t)frame_receipt.runtime.party_dir);
        out_receipt->combined_pixel_count +=
            frame_receipt.runtime_hud_frame_pixel_count;
        if (out_receipt->render_sample_count == 1) {
            out_receipt->first_frame = frame_receipt;
        }
        memset(&turn_receipt, 0, sizeof(turn_receipt));
        if (dm2_v1_boot_runtime_turn(profile, 1, &turn_receipt) &&
            turn_receipt.runtime_ready &&
            turn_receipt.operation_result == 0) {
            ++out_receipt->runtime_turn_count;
        }
    }
    dm2_v1_runtime_set_position(initial_runtime.current_level,
                                initial_runtime.party_x,
                                initial_runtime.party_y,
                                initial_runtime.party_dir);
    if (out_receipt->min_asset_portrait_count == 9999) {
        out_receipt->min_asset_portrait_count = 0;
    }
    if (out_receipt->min_asset_floor_ceiling_count == 9999) {
        out_receipt->min_asset_floor_ceiling_count = 0;
    }
    if (out_receipt->min_asset_wall_count == 9999) {
        out_receipt->min_asset_wall_count = 0;
    }
    out_receipt->unique_frame_hash_count = frame_hash_count;
    out_receipt->combined_frame_hash = combined_hash;
    /* A raw G1 boot has no original squad record yet.  Its static HUD must
     * therefore prove zero portrait fallback rather than inventing one. */
    out_receipt->no_fallback_portraits =
        out_receipt->total_fallback_portrait_count == 0;
    out_receipt->no_core_render_fallbacks =
        out_receipt->total_asset_floor_ceiling_count > 0 &&
        out_receipt->total_fallback_floor_ceiling_count == 0 &&
        out_receipt->total_asset_wall_count > 0 &&
        out_receipt->total_fallback_wall_count == 0 &&
        out_receipt->total_fallback_door_count == 0 &&
        out_receipt->total_fallback_creature_count == 0 &&
        out_receipt->total_fallback_creature_possession_item_count == 0 &&
        out_receipt->total_fallback_projectile_count == 0;
    out_receipt->first_runtime_hud_ready =
        out_receipt->first_frame.runtime_hud_capture_ready;
    out_receipt->real_gdat_core_render_ready =
        out_receipt->graphics_dat_ready &&
        out_receipt->min_asset_floor_ceiling_count >= 2 &&
        out_receipt->min_asset_wall_count > 0 &&
        out_receipt->no_core_render_fallbacks;
    out_receipt->raw_gdat_runtime_hud_capture_ready =
        dm2_v1_boot_runtime_raw_gdat_hud_probe(
            profile,
            &out_receipt->raw_gdat_runtime_portrait_count,
            &out_receipt->raw_gdat_runtime_portrait_hash,
            &out_receipt->raw_gdat_runtime_portrait_byte_count,
            &out_receipt->raw_gdat_runtime_core_hash,
            &out_receipt->raw_gdat_runtime_core_byte_count,
            &out_receipt->raw_gdat_runtime_interface_count);
    out_receipt->decoded_gdat_runtime_hud_capture_ready =
        dm2_v1_boot_runtime_decoded_gdat_hud_probe(
            profile,
            &out_receipt->decoded_gdat_runtime_portrait_count,
            &out_receipt->decoded_gdat_runtime_portrait_hash,
            &out_receipt->decoded_gdat_runtime_portrait_pixel_count,
            &out_receipt->decoded_gdat_runtime_core_hash,
            &out_receipt->decoded_gdat_runtime_core_pixel_count,
            &out_receipt->decoded_gdat_runtime_interface_count);
    out_receipt->real_gdat_portrait_ready =
        out_receipt->graphics_dat_ready &&
        out_receipt->no_fallback_portraits &&
        out_receipt->raw_gdat_runtime_portrait_count >= 4 &&
        out_receipt->decoded_gdat_runtime_portrait_count >= 4;
    /* skproject/SKWIN/SkWinCore.cpp DRAW_MAP_CHIP calls
     * QUERY_DUNGEON_MAP_CHIP_PICT(GDAT_CATEGORY_TELEPORTERS, 0, ...)
     * for teleporter dungeon tiles.  This receipt proves the runtime
     * dungeon asset owner can materialize that real map-chip atlas, even
     * when the current startup view does not contain a visible teleporter. */
    out_receipt->teleporter_map_chip_raw_hash = 0x32485452u;
    out_receipt->teleporter_map_chip_decoded_hash = 0x32485444u;
    if (dm2_v1_boot_runtime_raw_gdat_hash_add(
            profile,
            DM2_GDAT_CATEGORY_TELEPORTERS,
            0,
            DM2_GDAT_IMG_MAP_CHIP,
            &out_receipt->teleporter_map_chip_raw_hash,
            &out_receipt->teleporter_map_chip_raw_byte_count) &&
        dm2_v1_boot_runtime_decoded_gdat_hash_add(
            profile,
            DM2_GDAT_CATEGORY_TELEPORTERS,
            0,
            DM2_GDAT_IMG_MAP_CHIP,
            &out_receipt->teleporter_map_chip_decoded_hash,
            &out_receipt->teleporter_map_chip_decoded_pixel_count)) {
        out_receipt->teleporter_map_chip_ready = 1;
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->teleporter_map_chip_raw_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->teleporter_map_chip_decoded_hash);
    }
    /* skproject/SKWIN/SkWinCore.cpp DRAW_MAP_CHIP first pulls the base
     * magic-map tile from GDAT_CATEGORY_GRAPHICSSET, then overlays
     * GDAT_CATEGORY_WALL_GFX and GDAT_CATEGORY_FLOOR_GFX map chips via
     * QUERY_DUNGEON_MAP_CHIP_PICT before the teleporter/object routes. */
    out_receipt->dungeon_map_chip_raw_hash = 0x324d4352u;
    out_receipt->dungeon_map_chip_decoded_hash = 0x324d4344u;
    out_receipt->dungeon_map_chip_graphicsset_raw_hash = 0x32474752u;
    out_receipt->dungeon_map_chip_graphicsset_decoded_hash = 0x32474744u;
    out_receipt->dungeon_map_chip_wall_raw_hash = 0x32574752u;
    out_receipt->dungeon_map_chip_wall_decoded_hash = 0x32574744u;
    out_receipt->dungeon_map_chip_floor_raw_hash = 0x32464752u;
    out_receipt->dungeon_map_chip_floor_decoded_hash = 0x32464744u;
    out_receipt->dungeon_map_chip_graphicsset_count =
        dm2_v1_boot_runtime_map_chip_category_hash_add(
            profile,
            DM2_GDAT_CATEGORY_GRAPHICSSET,
            4,
            &out_receipt->dungeon_map_chip_raw_hash,
            &out_receipt->dungeon_map_chip_raw_byte_count,
            &out_receipt->dungeon_map_chip_decoded_hash,
            &out_receipt->dungeon_map_chip_decoded_pixel_count);
    dm2_v1_boot_runtime_map_chip_category_hash_add(
        profile,
        DM2_GDAT_CATEGORY_GRAPHICSSET,
        4,
        &out_receipt->dungeon_map_chip_graphicsset_raw_hash,
        &out_receipt->dungeon_map_chip_graphicsset_raw_byte_count,
        &out_receipt->dungeon_map_chip_graphicsset_decoded_hash,
        &out_receipt->dungeon_map_chip_graphicsset_decoded_pixel_count);
    out_receipt->dungeon_map_chip_wall_count =
        dm2_v1_boot_runtime_map_chip_category_hash_add(
            profile,
            DM2_GDAT_CATEGORY_WALL_GFX,
            8,
            &out_receipt->dungeon_map_chip_raw_hash,
            &out_receipt->dungeon_map_chip_raw_byte_count,
            &out_receipt->dungeon_map_chip_decoded_hash,
            &out_receipt->dungeon_map_chip_decoded_pixel_count);
    dm2_v1_boot_runtime_map_chip_category_hash_add(
        profile,
        DM2_GDAT_CATEGORY_WALL_GFX,
        8,
        &out_receipt->dungeon_map_chip_wall_raw_hash,
        &out_receipt->dungeon_map_chip_wall_raw_byte_count,
        &out_receipt->dungeon_map_chip_wall_decoded_hash,
        &out_receipt->dungeon_map_chip_wall_decoded_pixel_count);
    out_receipt->dungeon_map_chip_floor_count =
        dm2_v1_boot_runtime_map_chip_category_hash_add(
            profile,
            DM2_GDAT_CATEGORY_FLOOR_GFX,
            8,
            &out_receipt->dungeon_map_chip_raw_hash,
            &out_receipt->dungeon_map_chip_raw_byte_count,
            &out_receipt->dungeon_map_chip_decoded_hash,
            &out_receipt->dungeon_map_chip_decoded_pixel_count);
    dm2_v1_boot_runtime_map_chip_category_hash_add(
        profile,
        DM2_GDAT_CATEGORY_FLOOR_GFX,
        8,
        &out_receipt->dungeon_map_chip_floor_raw_hash,
        &out_receipt->dungeon_map_chip_floor_raw_byte_count,
        &out_receipt->dungeon_map_chip_floor_decoded_hash,
        &out_receipt->dungeon_map_chip_floor_decoded_pixel_count);
    out_receipt->dungeon_map_chip_graphicsset_ready =
        out_receipt->dungeon_map_chip_graphicsset_count > 0 &&
        out_receipt->dungeon_map_chip_graphicsset_raw_hash != 0u &&
        out_receipt->dungeon_map_chip_graphicsset_raw_byte_count > 0u &&
        out_receipt->dungeon_map_chip_graphicsset_decoded_hash != 0u &&
        out_receipt->dungeon_map_chip_graphicsset_decoded_pixel_count > 0u;
    out_receipt->dungeon_map_chip_wall_ready =
        out_receipt->dungeon_map_chip_wall_count > 0 &&
        out_receipt->dungeon_map_chip_wall_raw_hash != 0u &&
        out_receipt->dungeon_map_chip_wall_raw_byte_count > 0u &&
        out_receipt->dungeon_map_chip_wall_decoded_hash != 0u &&
        out_receipt->dungeon_map_chip_wall_decoded_pixel_count > 0u;
    out_receipt->dungeon_map_chip_floor_ready =
        out_receipt->dungeon_map_chip_floor_count > 0 &&
        out_receipt->dungeon_map_chip_floor_raw_hash != 0u &&
        out_receipt->dungeon_map_chip_floor_raw_byte_count > 0u &&
        out_receipt->dungeon_map_chip_floor_decoded_hash != 0u &&
        out_receipt->dungeon_map_chip_floor_decoded_pixel_count > 0u;
    out_receipt->dungeon_map_chip_ready =
        out_receipt->dungeon_map_chip_graphicsset_ready &&
        out_receipt->dungeon_map_chip_wall_ready &&
        out_receipt->dungeon_map_chip_floor_ready &&
        out_receipt->dungeon_map_chip_raw_hash != 0u &&
        out_receipt->dungeon_map_chip_raw_byte_count > 0u &&
        out_receipt->dungeon_map_chip_decoded_hash != 0u &&
        out_receipt->dungeon_map_chip_decoded_pixel_count > 0u;
    if (out_receipt->dungeon_map_chip_ready) {
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->dungeon_map_chip_raw_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->dungeon_map_chip_decoded_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->dungeon_map_chip_graphicsset_raw_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->dungeon_map_chip_wall_raw_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->dungeon_map_chip_floor_raw_hash);
    }
    out_receipt->graphicsset_word_values_ready =
        dm2_v1_boot_runtime_graphicsset_word_values_receipt(
            profile,
            DM2_GDAT_MAP_GRAPHICSSET_BOOT_WALL,
            &out_receipt->graphicsset_word_values_hash,
            &out_receipt->graphicsset_word_values_present_mask,
            &out_receipt->graphicsset_word_values_query_count,
            &out_receipt->graphicsset_scene_flags,
            &out_receipt->graphicsset_scene_colorkey,
            &out_receipt->graphicsset_ambient_light,
            &out_receipt->graphicsset_highest_light_level,
            &out_receipt->graphicsset_void_random_fall,
            &out_receipt->graphicsset_animated_floor,
            &out_receipt->graphicsset_scene_rain,
            &out_receipt->graphicsset_misty_map,
            &out_receipt->graphicsset_thunder_position,
            &out_receipt->graphicsset_ambient_darkness);
    if (out_receipt->graphicsset_word_values_ready) {
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->graphicsset_word_values_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->graphicsset_word_values_present_mask);
    }
    out_receipt->wall_gfx_image_offsets_ready =
        dm2_v1_boot_runtime_wall_gfx_image_offsets_receipt(
            profile,
            &out_receipt->wall_gfx_image_offsets_hash,
            &out_receipt->wall_gfx_image_offsets_query_count,
            &out_receipt->wall_gfx_image_offsets_nonzero_count,
            &out_receipt->wall_gfx_image_offsets_present_mask);
    if (out_receipt->wall_gfx_image_offsets_ready) {
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->wall_gfx_image_offsets_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->wall_gfx_image_offsets_present_mask);
    }
    out_receipt->interface_rect14_ready =
        dm2_v1_boot_runtime_interface_rect14_receipt(
            profile,
            &out_receipt->interface_rect14_hash,
            &out_receipt->interface_rect14_byte_count,
            &out_receipt->interface_rect14_row_count,
            &out_receipt->interface_rect14_stride,
            &out_receipt->interface_rect14_nonzero_5x5_count,
            &out_receipt->interface_rect14_image_field_count,
            &out_receipt->interface_rect14_stretch_field_count,
            &out_receipt->interface_rect14_flag_field_count);
    out_receipt->interface_rect14_placement_plan_ready =
        dm2_v1_boot_runtime_interface_rect14_placement_receipt(
            profile,
            &out_receipt->interface_rect14_placement_hash,
            &out_receipt->interface_rect14_placement_count,
            &out_receipt->interface_rect14_rotated_cell_mask,
            &out_receipt->interface_rect14_max_stretched_size);
    out_receipt->interface_action_table_ready =
        dm2_v1_boot_runtime_interface_action_table_receipt(
            profile,
            &out_receipt->interface_action_table_hash,
            &out_receipt->interface_action_table_byte_count,
            &out_receipt->interface_action_group_count,
            &out_receipt->interface_action_entry_count,
            &out_receipt->interface_action_tail_byte_count);
    out_receipt->interface_font_table_ready =
        dm2_v1_boot_runtime_interface_font_table_receipt(
            profile,
            &out_receipt->interface_font_table_hash,
            &out_receipt->interface_font_table_byte_count,
            &out_receipt->interface_font_table_row_count,
            &out_receipt->interface_font_table_char_count,
            &out_receipt->interface_font_table_nonzero_byte_count,
            &out_receipt->interface_font_table_printable_char_count);
    out_receipt->interface_palette_ready =
        dm2_v1_boot_runtime_interface_palette_receipt(
            profile,
            &out_receipt->interface_palette_hash,
            &out_receipt->interface_palette_irgb_byte_count,
            &out_receipt->interface_palette_pal16_byte_count,
            &out_receipt->interface_palette_irgb_color_count,
            &out_receipt->interface_palette_pal16_color_count);
    if (out_receipt->interface_rect14_ready) {
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_rect14_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_rect14_row_count);
    }
    if (out_receipt->interface_rect14_placement_plan_ready) {
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_rect14_placement_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_rect14_placement_count);
    }
    if (out_receipt->interface_action_table_ready) {
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_action_table_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_action_entry_count);
    }
    if (out_receipt->interface_font_table_ready) {
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_font_table_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_font_table_printable_char_count);
    }
    if (out_receipt->interface_palette_ready) {
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_palette_hash);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_palette_irgb_color_count);
        combined_hash = dm2_v1_boot_packaged_capture_hash_step(
            combined_hash,
            out_receipt->interface_palette_pal16_color_count);
    }
    out_receipt->combined_frame_hash = combined_hash;
    out_receipt->real_gdat_runtime_hud_breadth_ready =
        out_receipt->render_sample_count == 4 &&
        out_receipt->render_success_count == 4 &&
        out_receipt->sampled_direction_mask == 0x0f &&
        out_receipt->runtime_direction_mask == 0x0f &&
        out_receipt->runtime_turn_count == 4 &&
        out_receipt->unique_frame_hash_count > 0 &&
        out_receipt->real_gdat_portrait_ready &&
        out_receipt->real_gdat_core_render_ready &&
        out_receipt->raw_gdat_runtime_hud_capture_ready &&
        out_receipt->decoded_gdat_runtime_hud_capture_ready &&
        out_receipt->raw_gdat_runtime_interface_count >= 4 &&
        out_receipt->decoded_gdat_runtime_interface_count >= 4 &&
        out_receipt->teleporter_map_chip_ready &&
        out_receipt->dungeon_map_chip_ready &&
        out_receipt->wall_gfx_image_offsets_ready &&
        out_receipt->interface_action_table_ready &&
        out_receipt->interface_font_table_ready &&
        out_receipt->interface_palette_ready &&
        out_receipt->combined_frame_hash != 0u &&
        out_receipt->combined_pixel_count == 4u * 320u * 200u;
    out_receipt->valid =
        out_receipt->profile_ready &&
        out_receipt->runtime_ready &&
        out_receipt->real_gdat_runtime_hud_breadth_ready;
    return out_receipt->valid;
}

void dm2_v1_boot_creature_atlas_capture_receipt_init(
    DM2_V1_BootCreatureAtlasCaptureReceipt *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->min_frame_count = 9999;
}

int dm2_v1_boot_creature_atlas_capture_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_BootCreatureAtlasCaptureReceipt *out_receipt)
{
    uint32_t hash = 0x32434154u;

    dm2_v1_boot_creature_atlas_capture_receipt_init(out_receipt);
    if (!profile || !out_receipt) return 0;
    out_receipt->profile_ready =
        profile->assets_verified && profile->graphics_dat != NULL;
    out_receipt->graphics_dat_ready = profile->graphics_dat != NULL;
    if (!out_receipt->profile_ready) return 0;

    /* skproject SKWIN/SkWinCore.cpp QUERY_DUNGEON_MAP_CHIP_PICT feeds
     * creature category 0x0f map-chip images into DRAW_MAP_CHIP.  This
     * receipt materializes a bounded atlas sample through the same boot
     * viewport asset provider used by runtime, proving raw GDAT bytes and
     * decoded atlas pixels before complete DM2 support is reported. */
    for (int creature = 0; creature < 64; ++creature) {
        DM2_V1_BootViewportAssetEvidence ev;
        int gdat_index = dm2_v1_viewport_creature_graphic_index(creature, 0);
        int frame_count;
        int parity_rows = 0;
        if (gdat_index == 0 ||
            !dm2_v1_boot_viewport_asset_evidence(profile, gdat_index, &ev) ||
            ev.category != DM2_GDAT_CATEGORY_CREATURES ||
            ev.field != DM2_GDAT_IMG_MAP_CHIP) {
            continue;
        }
        frame_count =
            dm2_v1_viewport_map_chip_frame_count(ev.decoded_w, ev.decoded_h);
        if (frame_count <= 0) continue;
        for (int requested = 0; requested < frame_count && requested < 8;
             ++requested) {
            for (int creature_dir = 0; creature_dir < 4; ++creature_dir) {
                for (int party_dir = 0; party_dir < 4; ++party_dir) {
                    int selected =
                        dm2_v1_viewport_creature_frame_for_direction(
                            requested, creature_dir, party_dir, frame_count);
                    int expected;
                    if (frame_count <= 3) {
                        expected = dm2_v1_viewport_map_chip_frame_index(
                            requested, frame_count);
                    } else {
                        int base = requested & ~1;
                        int rel;
                        if (base + 1 >= frame_count) base = 0;
                        rel = ((party_dir & 3) - (creature_dir & 3)) & 3;
                        expected = dm2_v1_viewport_map_chip_frame_index(
                            base + (rel & 1), frame_count);
                    }
                    if (selected != expected) {
                        continue;
                    }
                    ++parity_rows;
                    out_receipt->frame_parity_hash =
                        dm2_v1_boot_packaged_capture_hash_step(
                            out_receipt->frame_parity_hash
                                ? out_receipt->frame_parity_hash
                                : 0x32434650u,
                            (uint32_t)((creature << 16) |
                                       (requested << 8) |
                                       (creature_dir << 4) |
                                       party_dir));
                    out_receipt->frame_parity_hash =
                        dm2_v1_boot_packaged_capture_hash_step(
                            out_receipt->frame_parity_hash,
                            (uint32_t)selected);
                }
            }
        }
        if (parity_rows <= 0) continue;
        ++out_receipt->sampled_creature_index_count;
        ++out_receipt->materialized_creature_index_count;
        out_receipt->frame_parity_matrix_count += parity_rows;
        if (creature < 32) {
            out_receipt->sampled_creature_mask_low |= (uint32_t)1u << creature;
        } else {
            out_receipt->sampled_creature_mask_high |=
                (uint32_t)1u << (creature - 32);
        }
        if (frame_count < out_receipt->min_frame_count) {
            out_receipt->min_frame_count = frame_count;
        }
        if (frame_count > out_receipt->max_frame_count) {
            out_receipt->max_frame_count = frame_count;
        }
        out_receipt->raw_gdat_byte_count += ev.raw_byte_count;
        out_receipt->decoded_gdat_pixel_count += ev.decoded_pixel_count;
        out_receipt->raw_gdat_hash =
            dm2_v1_boot_packaged_capture_hash_step(
                out_receipt->raw_gdat_hash ? out_receipt->raw_gdat_hash
                                           : 0x32435257u,
                ev.raw_hash);
        out_receipt->decoded_gdat_hash =
            dm2_v1_boot_packaged_capture_hash_step(
                out_receipt->decoded_gdat_hash
                    ? out_receipt->decoded_gdat_hash
                    : 0x32434445u,
                ev.decoded_hash);
        hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                      (uint32_t)creature);
        hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                      (uint32_t)frame_count);
        hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                      ev.raw_hash);
        hash = dm2_v1_boot_packaged_capture_hash_step(hash,
                                                      ev.decoded_hash);
        hash = dm2_v1_boot_packaged_capture_hash_step(
            hash, out_receipt->frame_parity_hash);
        if (out_receipt->materialized_creature_index_count >= 4) {
            break;
        }
    }
    /* skproject/SKWIN/SkWinCore.cpp GET_CREATURE_ANIMATION_FRAME consumes
     * creature action/sequence tables for runtime frame choice.
     * skproject/SKWINSPX/src/v4/skcrture.cpp:
     * GET_CREATURE_COMMAND_ANIMATION_V5 reads dtRaw8/0xfb, while
     * GET_ANIM_SEQUENCE_INFO_V5 and GET_CREATURE_ANIMATION_IMAGE_ID_V5 read
     * dtRaw7/0xfc and dtRaw7/0xfd.  Keep this as boot/render asset evidence:
     * it proves the real GDAT tables are present for creature rendering
     * without executing creature AI or combat. */
    for (int creature = 0; creature < 64; ++creature) {
        uint32_t before_hash = out_receipt->animation_table_hash;
        uint32_t before_count = out_receipt->animation_table_byte_count;
        int table_count = 0;
        if (dm2_v1_boot_runtime_typed_raw_gdat_hash_add(
                profile,
                DM2_GDAT_CATEGORY_CREATURES,
                creature,
                DM2_GDAT_ENTRY_TYPE_RAW8,
                DM2_GDAT_CREATURE_ANIM_ATTRIBUTION,
                &out_receipt->animation_table_hash,
                &out_receipt->animation_table_byte_count)) {
            ++out_receipt->animation_attribution_count;
            ++table_count;
        }
        if (dm2_v1_boot_runtime_typed_raw_gdat_hash_add(
                profile,
                DM2_GDAT_CATEGORY_CREATURES,
                creature,
                DM2_GDAT_ENTRY_TYPE_RAW7,
                DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE,
                &out_receipt->animation_table_hash,
                &out_receipt->animation_table_byte_count)) {
            ++out_receipt->animation_info_sequence_count;
            ++table_count;
        }
        if (dm2_v1_boot_runtime_typed_raw_gdat_hash_add(
                profile,
                DM2_GDAT_CATEGORY_CREATURES,
                creature,
                DM2_GDAT_ENTRY_TYPE_RAW7,
                DM2_GDAT_CREATURE_ANIM_FRAME_SEQUENCE,
                &out_receipt->animation_table_hash,
                &out_receipt->animation_table_byte_count)) {
            ++out_receipt->animation_frame_sequence_count;
            ++table_count;
        }
        if (table_count > 0) {
            out_receipt->animation_table_hash =
                dm2_v1_boot_packaged_capture_hash_step(
                    out_receipt->animation_table_hash,
                    (uint32_t)((creature << 8) | table_count));
        } else {
            out_receipt->animation_table_hash = before_hash;
            out_receipt->animation_table_byte_count = before_count;
        }
        if (table_count == 3) {
            ++out_receipt->animation_complete_creature_index_count;
            if (creature < 32) {
                out_receipt->animation_complete_creature_mask_low |=
                    (uint32_t)1u << creature;
            } else {
                out_receipt->animation_complete_creature_mask_high |=
                    (uint32_t)1u << (creature - 32);
            }
            out_receipt->animation_complete_creature_hash =
                dm2_v1_boot_packaged_capture_hash_step(
                    out_receipt->animation_complete_creature_hash
                        ? out_receipt->animation_complete_creature_hash
                        : 0x32434149u,
                    (uint32_t)creature);
        }
    }
    out_receipt->animation_table_ready =
        out_receipt->animation_attribution_count > 0 &&
        out_receipt->animation_info_sequence_count > 0 &&
        out_receipt->animation_frame_sequence_count > 0 &&
        out_receipt->animation_complete_creature_index_count > 0 &&
        out_receipt->animation_complete_creature_hash != 0u &&
        out_receipt->animation_table_hash != 0u &&
        out_receipt->animation_table_byte_count > 0u;
    if (out_receipt->min_frame_count == 9999) {
        out_receipt->min_frame_count = 0;
    }
    out_receipt->atlas_material_hash = hash;
    out_receipt->valid =
        out_receipt->graphics_dat_ready &&
        out_receipt->materialized_creature_index_count >= 4 &&
        out_receipt->frame_parity_matrix_count >=
            out_receipt->materialized_creature_index_count * 16 &&
        out_receipt->min_frame_count > 0 &&
        out_receipt->raw_gdat_hash != 0u &&
        out_receipt->raw_gdat_byte_count > 0u &&
        out_receipt->decoded_gdat_hash != 0u &&
        out_receipt->decoded_gdat_pixel_count > 0u &&
        out_receipt->animation_table_ready &&
        out_receipt->frame_parity_hash != 0u &&
        out_receipt->atlas_material_hash != 0u;
    return out_receipt->valid;
}

void dm2_v1_boot_complete_support_receipt_init(
    DM2_V1_CompleteSupportReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    dm2_v1_boot_startup_real_visual_capture_receipt_init(
        &receipt->startup_visual);
    dm2_v1_boot_runtime_hud_capture_receipt_init(&receipt->runtime_hud);
    dm2_v1_boot_creature_atlas_capture_receipt_init(&receipt->creature_atlas);
    receipt->status_scope = "DM2";
    receipt->status = "invalid";
}

int dm2_v1_boot_complete_support_receipt_from_runtime_state(
    DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_CompleteSupportReceipt *out_receipt)
{
    uint32_t hash = 0x32414353u;
    const char *save_corpus_root = NULL;
    DM2_SKSaveCorpusReceipt save_corpus;
    DM2_OriginalSaveStateCorpusReceipt original_save_state_corpus;
    int startup_visual_ready;
    int runtime_hud_ready;
    int creature_atlas_ready;

    dm2_v1_boot_complete_support_receipt_init(out_receipt);
    if (!profile || !out_receipt) {
        return 0;
    }
    startup_visual_ready =
        dm2_v1_boot_startup_real_visual_capture_receipt_from_runtime_state(
            profile,
            startup_menu_active,
            startup_save_root,
            resume_available,
            slot_mask,
            selected_row,
            title_animation_tick,
            &out_receipt->startup_visual);
    runtime_hud_ready =
        dm2_v1_boot_runtime_hud_capture_receipt(
            profile,
            &out_receipt->runtime_hud);
    creature_atlas_ready =
        dm2_v1_boot_creature_atlas_capture_receipt(
            profile,
            &out_receipt->creature_atlas);
    memset(&save_corpus, 0, sizeof(save_corpus));
    memset(&original_save_state_corpus, 0, sizeof(original_save_state_corpus));
    save_corpus_root = (startup_save_root && startup_save_root[0])
        ? startup_save_root
        : profile->save_root;
    out_receipt->save_corpus_scan_complete =
        dm2_v1_sksave_corpus_scan(save_corpus_root, &save_corpus) ? 1 : 0;
    out_receipt->save_corpus_valid_candidate_count =
        (int)save_corpus.has_last_session +
        (int)save_corpus.has_last_session_backup +
        (int)save_corpus.valid_slot_count;
    out_receipt->save_corpus_importable_candidate_count =
        (int)save_corpus.importable_candidate_count;
    out_receipt->save_corpus_rejected_candidate_count =
        (int)save_corpus.import_rejected_candidate_count +
        (int)save_corpus.invalid_candidate_count;
    out_receipt->save_corpus_original_candidate_count =
        (int)save_corpus.original_envelope_candidate_count +
        (int)save_corpus.original_raw_candidate_count;
    out_receipt->save_corpus_valid_slot_mask =
        (unsigned int)save_corpus.valid_slot_mask;
    out_receipt->save_corpus_hash = 0x32534353u;
    out_receipt->save_corpus_hash = dm2_v1_boot_packaged_capture_hash_step(
        out_receipt->save_corpus_hash,
        (uint32_t)out_receipt->save_corpus_scan_complete);
    out_receipt->save_corpus_hash = dm2_v1_boot_packaged_capture_hash_step(
        out_receipt->save_corpus_hash,
        (uint32_t)out_receipt->save_corpus_valid_candidate_count);
    out_receipt->save_corpus_hash = dm2_v1_boot_packaged_capture_hash_step(
        out_receipt->save_corpus_hash,
        (uint32_t)out_receipt->save_corpus_importable_candidate_count);
    out_receipt->save_corpus_hash = dm2_v1_boot_packaged_capture_hash_step(
        out_receipt->save_corpus_hash,
        (uint32_t)out_receipt->save_corpus_rejected_candidate_count);
    out_receipt->save_corpus_hash = dm2_v1_boot_packaged_capture_hash_step(
        out_receipt->save_corpus_hash,
        (uint32_t)out_receipt->save_corpus_original_candidate_count);
    out_receipt->save_corpus_hash = dm2_v1_boot_packaged_capture_hash_step(
        out_receipt->save_corpus_hash,
        out_receipt->save_corpus_valid_slot_mask);
    out_receipt->save_corpus_original_state_scan_complete =
        dm2_v1_original_save_state_corpus_probe(
            save_corpus_root, &original_save_state_corpus) ? 1 : 0;
    out_receipt->save_corpus_original_state_list_complete =
        original_save_state_corpus.original_candidate_list_complete;
    out_receipt->save_corpus_original_state_candidate_count =
        original_save_state_corpus.original_candidate_count;
    out_receipt->save_corpus_original_state_parsed_candidate_count =
        original_save_state_corpus.parsed_candidate_count;
    out_receipt->save_corpus_original_state_rejected_candidate_count =
        original_save_state_corpus.rejected_candidate_count;
    out_receipt->save_corpus_original_state_hash =
        original_save_state_corpus.corpus_hash;

    out_receipt->skproject_gdat_queries_ready =
        out_receipt->startup_visual.skproject_title_query_ready &&
        out_receipt->startup_visual.skproject_menu_query_ready;
    out_receipt->startup_title_menu_complete =
        startup_visual_ready &&
        out_receipt->startup_visual.valid &&
        out_receipt->startup_visual.title_menu_hud_visual_proof_ready &&
        out_receipt->startup_visual.full_title_frame_capture_ready &&
        out_receipt->startup_visual.menu_gdat_capture_ready &&
        out_receipt->startup_visual.menu_title_composite_capture_ready &&
        out_receipt->startup_visual.exact_title_timing_ready;
    out_receipt->startup_hud_handoff_complete =
        out_receipt->startup_visual.hud_handoff_capture_ready &&
        out_receipt->startup_visual.hud_suppressed_capture_ready &&
        out_receipt->startup_visual.suppress_game_hud &&
        !out_receipt->startup_visual.present_first_hud_frame;
    out_receipt->runtime_gdat_hud_complete =
        runtime_hud_ready &&
        out_receipt->runtime_hud.valid &&
        out_receipt->runtime_hud.first_runtime_hud_ready &&
        out_receipt->runtime_hud.real_gdat_portrait_ready &&
        out_receipt->runtime_hud.no_fallback_portraits &&
        out_receipt->runtime_hud.raw_gdat_runtime_portrait_count >= 4 &&
        out_receipt->runtime_hud.decoded_gdat_runtime_portrait_count >= 4;
    out_receipt->runtime_gdat_dungeon_complete =
        out_receipt->runtime_hud.real_gdat_core_render_ready &&
        out_receipt->runtime_hud.no_core_render_fallbacks &&
        out_receipt->runtime_hud.min_asset_floor_ceiling_count >= 2 &&
        out_receipt->runtime_hud.min_asset_wall_count > 0 &&
        out_receipt->runtime_hud.teleporter_map_chip_ready &&
        out_receipt->runtime_hud.dungeon_map_chip_ready &&
        out_receipt->runtime_hud.dungeon_map_chip_graphicsset_ready &&
        out_receipt->runtime_hud.dungeon_map_chip_wall_ready &&
        out_receipt->runtime_hud.dungeon_map_chip_floor_ready &&
        out_receipt->runtime_hud.wall_gfx_image_offsets_ready &&
        out_receipt->runtime_hud.interface_action_table_ready &&
        out_receipt->runtime_hud.interface_font_table_ready &&
        out_receipt->runtime_hud.interface_palette_ready &&
        out_receipt->runtime_hud.total_fallback_door_count == 0 &&
        out_receipt->runtime_hud.total_fallback_creature_count == 0 &&
        out_receipt->runtime_hud.total_fallback_item_count == 0 &&
        out_receipt->runtime_hud.total_fallback_creature_possession_item_count == 0 &&
        out_receipt->runtime_hud.total_fallback_carried_item_count == 0 &&
        out_receipt->runtime_hud.total_fallback_projectile_count == 0;
    out_receipt->runtime_gdat_map_chip_categories_complete =
        out_receipt->runtime_hud.dungeon_map_chip_graphicsset_ready &&
        out_receipt->runtime_hud.dungeon_map_chip_wall_ready &&
        out_receipt->runtime_hud.dungeon_map_chip_floor_ready &&
        out_receipt->runtime_hud.dungeon_map_chip_graphicsset_raw_hash != 0u &&
        out_receipt->runtime_hud.dungeon_map_chip_wall_raw_hash != 0u &&
        out_receipt->runtime_hud.dungeon_map_chip_floor_raw_hash != 0u &&
        out_receipt->runtime_hud.dungeon_map_chip_graphicsset_decoded_hash != 0u &&
        out_receipt->runtime_hud.dungeon_map_chip_wall_decoded_hash != 0u &&
        out_receipt->runtime_hud.dungeon_map_chip_floor_decoded_hash != 0u &&
        (!out_receipt->runtime_hud.graphicsset_word_values_ready ||
         (out_receipt->runtime_hud.graphicsset_word_values_hash != 0u &&
          out_receipt->runtime_hud.graphicsset_word_values_query_count >= 2u &&
          (out_receipt->runtime_hud.graphicsset_word_values_present_mask &
           DM2_V1_GRAPHICSSET_SCENE_ADMISSION_PRESENT_MASK) ==
              DM2_V1_GRAPHICSSET_SCENE_ADMISSION_PRESENT_MASK)) &&
        out_receipt->runtime_hud.wall_gfx_image_offsets_ready &&
        out_receipt->runtime_hud.wall_gfx_image_offsets_hash != 0u &&
        out_receipt->runtime_hud.wall_gfx_image_offsets_query_count > 0u &&
        out_receipt->runtime_hud.wall_gfx_image_offsets_nonzero_count > 0u &&
        out_receipt->runtime_hud.wall_gfx_image_offsets_present_mask != 0u;
    out_receipt->runtime_gdat_interface_placement_complete =
        out_receipt->runtime_hud.wall_gfx_image_offsets_ready &&
        out_receipt->runtime_hud.wall_gfx_image_offsets_hash != 0u &&
        (!out_receipt->runtime_hud.interface_rect14_ready ||
         (out_receipt->runtime_hud.interface_rect14_stride == 14u &&
          out_receipt->runtime_hud.interface_rect14_row_count > 0u &&
          out_receipt->runtime_hud.interface_rect14_hash != 0u &&
          out_receipt->runtime_hud.interface_rect14_byte_count ==
              out_receipt->runtime_hud.interface_rect14_row_count * 14u &&
          out_receipt->runtime_hud.interface_rect14_image_field_count > 0u &&
          out_receipt->runtime_hud.interface_rect14_stretch_field_count > 0u &&
          out_receipt->runtime_hud.interface_rect14_placement_plan_ready &&
          out_receipt->runtime_hud.interface_rect14_placement_hash != 0u &&
          out_receipt->runtime_hud.interface_rect14_placement_count >=
              out_receipt->runtime_hud.interface_rect14_row_count &&
          out_receipt->runtime_hud.interface_rect14_rotated_cell_mask != 0u &&
          out_receipt->runtime_hud.interface_rect14_max_stretched_size > 0u)) &&
        out_receipt->runtime_hud.interface_font_table_ready &&
        out_receipt->runtime_hud.interface_font_table_byte_count == 0x300u &&
        out_receipt->runtime_hud.interface_font_table_row_count == 6u &&
        out_receipt->runtime_hud.interface_font_table_char_count == 128u &&
        out_receipt->runtime_hud.interface_font_table_printable_char_count > 0u &&
        out_receipt->runtime_hud.interface_palette_ready &&
        out_receipt->runtime_hud.interface_palette_hash != 0u &&
        out_receipt->runtime_hud.interface_palette_irgb_color_count > 0u &&
        out_receipt->runtime_hud.interface_palette_pal16_color_count > 0u;
    out_receipt->runtime_creature_atlas_complete =
        creature_atlas_ready &&
        out_receipt->creature_atlas.valid &&
        out_receipt->creature_atlas.materialized_creature_index_count >= 4 &&
        out_receipt->creature_atlas.frame_parity_matrix_count >=
            out_receipt->creature_atlas.materialized_creature_index_count * 16 &&
        out_receipt->creature_atlas.min_frame_count > 0 &&
        out_receipt->creature_atlas.raw_gdat_hash != 0u &&
        out_receipt->creature_atlas.decoded_gdat_hash != 0u &&
        out_receipt->creature_atlas.animation_table_ready &&
        out_receipt->creature_atlas.frame_parity_hash != 0u;
    out_receipt->runtime_gdat_direction_breadth_complete =
        out_receipt->runtime_hud.render_sample_count == 4 &&
        out_receipt->runtime_hud.render_success_count == 4 &&
        out_receipt->runtime_hud.sampled_direction_mask == 0x0f &&
        out_receipt->runtime_hud.runtime_direction_mask == 0x0f &&
        out_receipt->runtime_hud.runtime_turn_count == 4 &&
        out_receipt->runtime_hud.unique_frame_hash_count > 0;
    out_receipt->no_fallback_title_or_runtime_visuals =
        out_receipt->startup_visual.no_fallback_title_blit &&
        out_receipt->runtime_hud.no_fallback_portraits &&
        out_receipt->runtime_hud.no_core_render_fallbacks;
    out_receipt->raw_gdat_capture_complete =
        out_receipt->startup_visual.raw_gdat_capture_ready &&
        out_receipt->startup_visual.runtime_hud_raw_gdat_capture_ready &&
        out_receipt->runtime_hud.raw_gdat_runtime_hud_capture_ready &&
        out_receipt->runtime_hud.raw_gdat_runtime_interface_count >= 4 &&
        out_receipt->runtime_hud.raw_gdat_runtime_portrait_hash != 0u &&
        out_receipt->runtime_hud.raw_gdat_runtime_core_hash != 0u;
    out_receipt->decoded_gdat_capture_complete =
        out_receipt->startup_visual.runtime_hud_decoded_gdat_capture_ready &&
        out_receipt->runtime_hud.decoded_gdat_runtime_hud_capture_ready &&
        out_receipt->runtime_hud.decoded_gdat_runtime_interface_count >= 4 &&
        out_receipt->runtime_hud.decoded_gdat_runtime_portrait_hash != 0u &&
        out_receipt->runtime_hud.decoded_gdat_runtime_core_hash != 0u;

    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->startup_visual.packaged_visual_capture_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.combined_frame_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud.render_sample_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud.min_asset_portrait_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud.min_asset_floor_ceiling_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud.min_asset_wall_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud.total_asset_item_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud.total_fallback_item_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud.total_asset_carried_item_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->runtime_hud.total_fallback_carried_item_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.raw_gdat_runtime_portrait_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.decoded_gdat_runtime_portrait_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.dungeon_map_chip_graphicsset_raw_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.dungeon_map_chip_wall_raw_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.dungeon_map_chip_floor_raw_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.graphicsset_word_values_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.graphicsset_word_values_present_mask);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.wall_gfx_image_offsets_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.wall_gfx_image_offsets_present_mask);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.interface_rect14_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.interface_rect14_row_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.interface_rect14_placement_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.interface_rect14_placement_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.interface_palette_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.interface_palette_irgb_color_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->runtime_hud.interface_palette_pal16_color_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->creature_atlas.atlas_material_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->creature_atlas.frame_parity_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->save_corpus_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->save_corpus_original_state_scan_complete);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->save_corpus_original_state_list_complete);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->save_corpus_original_state_candidate_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->save_corpus_original_state_parsed_candidate_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, (uint32_t)out_receipt->save_corpus_original_state_rejected_candidate_count);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->save_corpus_original_state_hash);
    out_receipt->complete_support_hash = hash;

    /* skproject/SKWIN T520/T560 consumes GDAT title/menu, HUD, and dungeon
     * draw paths as one runtime contract. Firestaff only reports complete
     * DM2 support when startup and live dungeon rendering are backed by real
     * GDAT material, no legacy visual fallback remains, and at least one
     * original SKSave candidate has been parsed into source-owned state. */
    out_receipt->complete_support_ready =
        out_receipt->skproject_gdat_queries_ready &&
        out_receipt->startup_title_menu_complete &&
        out_receipt->startup_hud_handoff_complete &&
        out_receipt->runtime_gdat_hud_complete &&
        out_receipt->runtime_gdat_dungeon_complete &&
        out_receipt->runtime_gdat_map_chip_categories_complete &&
        out_receipt->runtime_gdat_interface_placement_complete &&
        out_receipt->runtime_creature_atlas_complete &&
        out_receipt->runtime_gdat_direction_breadth_complete &&
        out_receipt->no_fallback_title_or_runtime_visuals &&
        out_receipt->raw_gdat_capture_complete &&
        out_receipt->decoded_gdat_capture_complete &&
        out_receipt->save_corpus_scan_complete &&
        out_receipt->save_corpus_original_state_scan_complete &&
        out_receipt->save_corpus_original_state_list_complete &&
        out_receipt->save_corpus_original_state_candidate_count > 0 &&
        out_receipt->save_corpus_original_state_parsed_candidate_count > 0 &&
        out_receipt->save_corpus_original_state_rejected_candidate_count == 0 &&
        out_receipt->save_corpus_original_state_hash != 0u &&
        out_receipt->complete_support_hash != 0u;
    out_receipt->valid = out_receipt->complete_support_ready;
    out_receipt->status_scope = "DM2";
    out_receipt->status = out_receipt->complete_support_ready
        ? "complete-support-ready"
        : (out_receipt->skproject_gdat_queries_ready &&
           out_receipt->startup_title_menu_complete &&
           out_receipt->startup_hud_handoff_complete &&
           out_receipt->runtime_gdat_hud_complete &&
           out_receipt->runtime_gdat_dungeon_complete &&
           out_receipt->runtime_gdat_map_chip_categories_complete &&
           out_receipt->runtime_gdat_interface_placement_complete &&
           out_receipt->runtime_creature_atlas_complete &&
           out_receipt->runtime_gdat_direction_breadth_complete &&
           out_receipt->no_fallback_title_or_runtime_visuals &&
           out_receipt->raw_gdat_capture_complete &&
           out_receipt->decoded_gdat_capture_complete)
        ? "incomplete-save-corpus"
        : out_receipt->runtime_gdat_dungeon_complete
        ? "incomplete-startup-gdat"
        : "incomplete-runtime-gdat";
    return 1;
}

static const char *dm2_v1_boot_startup_prepare_host_status(
    DM2_V1_BootStartupPrepareResult result)
{
    switch (result) {
    case DM2_V1_BOOT_STARTUP_PREPARE_BAD_INPUT:
        return "DM2 BAD INPUT";
    case DM2_V1_BOOT_STARTUP_PREPARE_OOM:
        return "DM2 OOM";
    case DM2_V1_BOOT_STARTUP_PREPARE_SCAN_FAILED:
        return "DM2 ASSETS MISSING";
    case DM2_V1_BOOT_STARTUP_PREPARE_UNVERIFIED_ASSETS:
        return "DM2 ASSETS UNVERIFIED";
    case DM2_V1_BOOT_STARTUP_PREPARE_ENTER_GAME_FAILED:
        return "DM2 ENTER GAME FAILED";
    case DM2_V1_BOOT_STARTUP_PREPARE_RUNTIME_BIND_FAILED:
        return "DM2 RUNTIME BIND FAILED";
    default:
        return dm2_v1_boot_startup_prepare_result_name(result);
    }
}

static void dm2_v1_boot_startup_set_failure_status(
    DM2_V1_BootStartupPrepareResult result,
    DM2_V1_BootStartupLaunch *launch)
{
    if (!launch) {
        return;
    }
    launch->failure_status_scope = "BOOT";
    launch->failure_status = dm2_v1_boot_startup_prepare_host_status(result);
}

int dm2_v1_boot_startup_prepare_failure_host_receipt(
    const DM2_V1_BootStartupLaunch *launch,
    DM2_V1_StartupHostReceipt *out_receipt)
{
    DM2_V1_BootStartupPrepareResult result =
        DM2_V1_BOOT_STARTUP_PREPARE_BAD_INPUT;
    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->input_result = DM2_V1_STARTUP_HOST_INPUT_IGNORED;
    out_receipt->status_scope = "BOOT";
    out_receipt->status = dm2_v1_boot_startup_prepare_host_status(result);
    if (launch) {
        result = launch->prepare_result;
        out_receipt->status_scope =
            launch->failure_status_scope ? launch->failure_status_scope
                                         : "BOOT";
        out_receipt->status =
            launch->failure_status
                ? launch->failure_status
                : dm2_v1_boot_startup_prepare_host_status(result);
    }
    return 1;
}

int dm2_v1_boot_startup_launch_alloc(
    const char *data_dir,
    DM2_V1_BootStartupLaunch *out_launch) {
    DM2_V1_BootProfile *profile;
    if (!out_launch) return 0;
    memset(out_launch, 0, sizeof(*out_launch));
    if (!data_dir || data_dir[0] == '\0') {
        out_launch->prepare_result = DM2_V1_BOOT_STARTUP_PREPARE_BAD_INPUT;
        dm2_v1_boot_startup_set_failure_status(out_launch->prepare_result,
                                               out_launch);
        return 0;
    }
    profile = (DM2_V1_BootProfile *)calloc(1, sizeof(*profile));
    if (!profile) {
        out_launch->prepare_result = DM2_V1_BOOT_STARTUP_PREPARE_OOM;
        dm2_v1_boot_startup_set_failure_status(out_launch->prepare_result,
                                               out_launch);
        return 0;
    }
    dm2_v1_boot_profile_init(profile);
    if (dm2_v1_boot_scan_assets(profile, data_dir) != 0) {
        out_launch->prepare_result = DM2_V1_BOOT_STARTUP_PREPARE_SCAN_FAILED;
        dm2_v1_boot_startup_set_failure_status(out_launch->prepare_result,
                                               out_launch);
        free(profile);
        return 0;
    }
    if (!profile->assets_verified) {
        out_launch->prepare_result =
            DM2_V1_BOOT_STARTUP_PREPARE_UNVERIFIED_ASSETS;
        dm2_v1_boot_startup_set_failure_status(out_launch->prepare_result,
                                               out_launch);
        dm2_v1_boot_cleanup(profile);
        free(profile);
        return 0;
    }
    dm2_v1_boot_set_save_root(profile, NULL);
    dm2_v1_boot_print_summary(profile);
    if (dm2_v1_boot_enter_game(profile) != 0) {
        out_launch->prepare_result =
            DM2_V1_BOOT_STARTUP_PREPARE_ENTER_GAME_FAILED;
        dm2_v1_boot_startup_set_failure_status(out_launch->prepare_result,
                                               out_launch);
        dm2_v1_boot_cleanup(profile);
        free(profile);
        return 0;
    }
    /* SKULL.ASM T520/T560: the verified boot profile owns the V1 game
     * state, so bind the runtime singleton here before M11 applies resume
     * or startup-session state. */
    if (!dm2_v1_runtime_bind_boot_profile(profile)) {
        out_launch->prepare_result =
            DM2_V1_BOOT_STARTUP_PREPARE_RUNTIME_BIND_FAILED;
        dm2_v1_boot_startup_set_failure_status(out_launch->prepare_result,
                                               out_launch);
        dm2_v1_boot_cleanup(profile);
        free(profile);
        return 0;
    }
    out_launch->profile = profile;
    out_launch->prepare_result = DM2_V1_BOOT_STARTUP_PREPARE_OK;
    out_launch->runtime_bound = 1;
    return 1;
}

int dm2_v1_boot_startup_launch_detach_runtime(
    DM2_V1_BootStartupLaunch *launch,
    DM2_V1_BootStartupRuntimeReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!launch || !launch->profile) {
        return 0;
    }
    out_receipt->profile = launch->profile;
    out_receipt->dm2_state = launch->profile->dm2_state;
    snprintf(out_receipt->boot_asset_md5,
             sizeof(out_receipt->boot_asset_md5),
             "%s",
             launch->profile->graphics_md5);
    snprintf(out_receipt->dungeon_path,
             sizeof(out_receipt->dungeon_path),
             "%s",
             launch->profile->dungeon_path[0]
                 ? launch->profile->dungeon_path
                 : "DUNGEON.DAT");
    snprintf(out_receipt->title,
             sizeof(out_receipt->title),
             "%s",
             "DUNGEON MASTER II: SKULLKEEP");
    snprintf(out_receipt->source_id,
             sizeof(out_receipt->source_id),
             "%s",
             "dm2");
    out_receipt->initialize_v2_runtime = 1;
    out_receipt->initialize_hud_runtime = 1;
    out_receipt->initialize_touch_runtime = 1;
    launch->profile = NULL;
    return 1;
}

void dm2_v1_boot_startup_launch_cleanup(
    DM2_V1_BootStartupLaunch *launch) {
    if (!launch) return;
    if (launch->profile) {
        dm2_v1_boot_cleanup(launch->profile);
        free(launch->profile);
    }
    memset(launch, 0, sizeof(*launch));
}

int dm2_v1_boot_hud_core_asset_address(int field,
                                       int *out_category,
                                       int *out_index,
                                       int *out_field)
{
    if (!out_category || !out_index || !out_field ||
        field < 0 || field > DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK) {
        return 0;
    }

    *out_category = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    *out_index = 0;
    *out_field = field;

    /* skproject/SKWINSPX loads the base interface tables with
     * LOAD_GDAT_INTERFACE_00_02, then draws HUD pieces through
     * QUERY_GDAT_IMAGE_ENTRY_BUFF(1, subcat, field).  PC DM2 stores these
     * core HUD images under subcategories 2-7 rather than index 0. */
    if (field == DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR) {
        *out_index = 6;
        *out_field = 0x00;
    } else if (field == DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_STRIP) {
        *out_index = 5;
        *out_field = 0x0b;
    } else if (field == DM2_V1_VIEWPORT_GFX_HUD_CORE_GOLD_BOX) {
        *out_index = 2;
        *out_field = 0x03;
    } else if (field == DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL) {
        *out_index = 4;
        *out_field = 0x01;
    } else if (field >= DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_ICON_BASE &&
               field < DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_ICON_BASE +
                           DM2_V1_HUD_ACTION_ICON_COUNT) {
        *out_index = 3;
        *out_field =
            0x02 + (field - DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_ICON_BASE);
    }
    return 1;
}

int dm2_v1_boot_viewport_asset_fetch(void *user,
                                     int gdat_index,
                                     const uint8_t **out_pixels,
                                     int *out_w,
                                     int *out_h,
                                     int *out_stride) {
    DM2_V1_BootProfile *profile = (DM2_V1_BootProfile *)user;
    DM2_V1_BootGraphicsDat *gfx;
    uint8_t **cache_pixels;
    int *cache_w;
    int *cache_h;
    DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;
    int category;
    int index;
    int field;
    int wall_button_key = 0;

    if (out_pixels) *out_pixels = NULL;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (out_stride) *out_stride = 0;
    if (!profile || !profile->graphics_dat) return -1;
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;

    if (dm2_v1_viewport_scene_material_graphic_address(
            gdat_index, &index, &field)) {
        if (index >= DM2_GDAT_SCENE_MATERIAL_CACHE_LIMIT) return -1;
        cache_pixels = &gfx->scene_material_pixels[index][field];
        cache_w = &gfx->scene_material_w[index][field];
        cache_h = &gfx->scene_material_h[index][field];
        category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    } else if (gdat_index == dm2_v1_viewport_dialogue_box_graphic_index()) {
        cache_pixels = &gfx->dialogue_box_pixels;
        cache_w = &gfx->dialogue_box_w;
        cache_h = &gfx->dialogue_box_h;
        category = DM2_GDAT_CATEGORY_DIALOG_BOXES;
        index = DM2_V1_DIALOGUE_BOX_INDEX;
        field = DM2_V1_DIALOGUE_BOX_FIELD;
    } else if (gdat_index == DM2_V1_VIEWPORT_GFX_TELEPORTER_MAP_CHIP) {
        cache_pixels = &gfx->teleporter_pixels;
        cache_w = &gfx->teleporter_w;
        cache_h = &gfx->teleporter_h;
        category = DM2_GDAT_CATEGORY_TELEPORTERS;
        index = 0;
        field = DM2_GDAT_IMG_MAP_CHIP;
    } else if (dm2_v1_viewport_floor_gfx_map_chip_graphic_address(
                   gdat_index, &index)) {
        cache_pixels = &gfx->floor_gfx_map_chip_pixels[index];
        cache_w = &gfx->floor_gfx_map_chip_w[index];
        cache_h = &gfx->floor_gfx_map_chip_h[index];
        category = DM2_GDAT_CATEGORY_FLOOR_GFX;
        field = DM2_GDAT_IMG_MAP_CHIP;
    } else if (dm2_v1_viewport_wall_gfx_map_chip_graphic_address(
                   gdat_index, &index)) {
        cache_pixels = &gfx->wall_gfx_map_chip_pixels[index];
        cache_w = &gfx->wall_gfx_map_chip_w[index];
        cache_h = &gfx->wall_gfx_map_chip_h[index];
        category = DM2_GDAT_CATEGORY_WALL_GFX;
        field = DM2_GDAT_IMG_MAP_CHIP;
    } else if (dm2_v1_viewport_door_map_chip_graphic_address(
                   gdat_index, &index)) {
        cache_pixels = &gfx->door_map_chip_pixels[index];
        cache_w = &gfx->door_map_chip_w[index];
        cache_h = &gfx->door_map_chip_h[index];
        category = DM2_GDAT_CATEGORY_DOORS;
        field = DM2_GDAT_IMG_MAP_CHIP;
    } else if (gdat_index == DM2_V1_VIEWPORT_GFX_CEILING ||
               gdat_index == DM2_V1_VIEWPORT_GFX_FLOOR) {
        int material_field = gdat_index == DM2_V1_VIEWPORT_GFX_CEILING ?
            DM2_GDAT_GFXSET_CEIL : DM2_GDAT_GFXSET_FLOOR;
        cache_pixels = &gfx->scene_material_pixels[0][material_field];
        cache_w = &gfx->scene_material_w[0][material_field];
        cache_h = &gfx->scene_material_h[0][material_field];
        category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        index = 0;
        field = material_field;
    } else if (dm2_v1_viewport_wall_graphic_address(
                   gdat_index, &index, &field)) {
        if (field < 0 || field >= DM2_GDAT_WALL_FIELD_CACHE_LIMIT) {
            return -1;
        }
        if (gfx->wall_pixels[field] && gfx->wall_keys[field] != gdat_index) {
            dm2_v1_asset_free_pixels(gfx->wall_pixels[field]);
            gfx->wall_pixels[field] = NULL;
            gfx->wall_w[field] = 0;
            gfx->wall_h[field] = 0;
        }
        cache_pixels = &gfx->wall_pixels[field];
        cache_w = &gfx->wall_w[field];
        cache_h = &gfx->wall_h[field];
        category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        gfx->wall_keys[field] = gdat_index;
    } else if (dm2_v1_viewport_weather_environment_graphic_address(
                   gdat_index, &index, &field)) {
        int slot = -1;
        if (index < 0 || index >= DM2_GDAT_SCENE_MATERIAL_CACHE_LIMIT ||
            field < (int)DM2_V1_WEATHER_BOLT_CMD_BASE ||
            field > (int)DM2_V1_WEATHER_RAIN_STORM_CMD) {
            return -1;
        }
        for (int i = 0; i < (int)DM2_V1_WEATHER_COMMAND_COUNT; ++i) {
            if (gfx->weather_environment_pixels[i] &&
                gfx->weather_environment_keys[i] == gdat_index) {
                slot = i;
                break;
            }
            if (slot < 0 && !gfx->weather_environment_pixels[i]) {
                slot = i;
            }
        }
        if (slot < 0) {
            slot = (index + field) % (int)DM2_V1_WEATHER_COMMAND_COUNT;
            dm2_v1_asset_free_pixels(gfx->weather_environment_pixels[slot]);
            gfx->weather_environment_pixels[slot] = NULL;
            gfx->weather_environment_w[slot] = 0;
            gfx->weather_environment_h[slot] = 0;
        }
        cache_pixels = &gfx->weather_environment_pixels[slot];
        cache_w = &gfx->weather_environment_w[slot];
        cache_h = &gfx->weather_environment_h[slot];
        gfx->weather_environment_keys[slot] = gdat_index;
        category = DM2_GDAT_CATEGORY_ENVIRONMENT;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE &&
               (((DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff) >=
                   DM2_GDAT_CATEGORY_SPELL_MISSILES &&
               (((DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff) <=
                   DM2_GDAT_CATEGORY_MISCELLANEOUS) {
        int packed = DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index;
        int slot = -1;
        category = (packed >> DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff;
        index = (packed >> DM2_V1_VIEWPORT_GFX_PROJECTILE_INDEX_SHIFT) & 0xff;
        field = DM2_GDAT_IMG_MAP_CHIP;
        for (int i = 0; i < DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT; ++i) {
            if (gfx->projectile_pixels[i] &&
                gfx->projectile_keys[i] == gdat_index) {
                slot = i;
                break;
            }
            if (slot < 0 && !gfx->projectile_pixels[i]) {
                slot = i;
            }
        }
        if (slot < 0) {
            slot = (category + index) % DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT;
            dm2_v1_asset_free_pixels(gfx->projectile_pixels[slot]);
            gfx->projectile_pixels[slot] = NULL;
            gfx->projectile_w[slot] = 0;
            gfx->projectile_h[slot] = 0;
        }
        cache_pixels = &gfx->projectile_pixels[slot];
        cache_w = &gfx->projectile_w[slot];
        cache_h = &gfx->projectile_h[slot];
        gfx->projectile_keys[slot] = gdat_index;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE &&
               (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) >=
                   DM2_GDAT_CATEGORY_WEAPONS &&
               (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) <=
                   DM2_GDAT_CATEGORY_MISCELLANEOUS) {
        int packed = DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index;
        int slot = -1;
        category = (packed >> DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff;
        index = (packed >> DM2_V1_VIEWPORT_GFX_ITEM_INDEX_SHIFT) & 0xff;
        field = packed & DM2_V1_VIEWPORT_GFX_ITEM_FIELD_MASK;
        if (category < DM2_GDAT_CATEGORY_WEAPONS ||
            category > DM2_GDAT_CATEGORY_MISCELLANEOUS) {
            return -1;
        }
        for (int i = 0; i < DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT; ++i) {
            if (gfx->item_pixels[i] && gfx->item_keys[i] == gdat_index) {
                slot = i;
                break;
            }
            if (slot < 0 && !gfx->item_pixels[i]) {
                slot = i;
            }
        }
        if (slot < 0) {
            slot = (category + index) % DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT;
            dm2_v1_asset_free_pixels(gfx->item_pixels[slot]);
            gfx->item_pixels[slot] = NULL;
            gfx->item_w[slot] = 0;
            gfx->item_h[slot] = 0;
        }
        cache_pixels = &gfx->item_pixels[slot];
        cache_w = &gfx->item_w[slot];
        cache_h = &gfx->item_h[slot];
        gfx->item_keys[slot] = gdat_index;
    } else if ((gdat_index <= DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE &&
                DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE - gdat_index <
                    (0x100 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT)) ||
               (gdat_index <= DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE &&
                DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE - gdat_index <
                    (0x100 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT))) {
        int direct_field =
            gdat_index <= DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE;
        int packed = (direct_field
            ? DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE
            : DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE) - gdat_index;
        int slot = -1;
        if (packed < 0 ||
            packed >= (0x100 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT)) {
            return -1;
        }
        category = DM2_GDAT_CATEGORY_CREATURES;
        index = (packed >> DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT) & 0xff;
        field = direct_field
            ? (packed & DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_MASK)
            : DM2_GDAT_IMG_MAP_CHIP;
        for (int i = 0; i < DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT; ++i) {
            if (gfx->creature_pixels[i] &&
                gfx->creature_keys[i] == gdat_index) {
                slot = i;
                break;
            }
            if (slot < 0 && !gfx->creature_pixels[i]) {
                slot = i;
            }
        }
        if (slot < 0) {
            slot = index % DM2_GDAT_VIEWPORT_SPRITE_CACHE_LIMIT;
            dm2_v1_asset_free_pixels(gfx->creature_pixels[slot]);
            gfx->creature_pixels[slot] = NULL;
            gfx->creature_w[slot] = 0;
            gfx->creature_h[slot] = 0;
        }
        cache_pixels = &gfx->creature_pixels[slot];
        cache_w = &gfx->creature_w[slot];
        cache_h = &gfx->creature_h[slot];
        gfx->creature_keys[slot] = gdat_index;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE - gdat_index <=
                   DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK) {
        int logical_field;
        field = DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE - gdat_index;
        logical_field = field;
        if (!dm2_v1_boot_hud_core_asset_address(logical_field,
                                                &category,
                                                &index,
                                                &field)) {
            return -1;
        }
        cache_pixels = &gfx->hud_core_pixels[logical_field];
        cache_w = &gfx->hud_core_w[logical_field];
        cache_h = &gfx->hud_core_h[logical_field];
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE - gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT)) {
        int packed = DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE - gdat_index;
        index = (packed >> DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT) & 0xff;
        field = packed & DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_MASK;
        if (index < 0 || index >= DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT) {
            return -1;
        }
        cache_pixels = &gfx->hud_portrait_pixels[index];
        cache_w = &gfx->hud_portrait_w[index];
        cache_h = &gfx->hud_portrait_h[index];
        /* skproject SKWIN/SkWinCore.cpp DRAW_CHAMPION_PICTURE
         * (_2e62_061d) draws runtime HUD portraits from
         * GDAT_CATEGORY_CHAMPIONS, hero type, image field 0. */
        category = DM2_GDAT_CATEGORY_CHAMPIONS;
    } else if (gdat_index <=
                   DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE &&
               gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE) {
        int packed =
            DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE - gdat_index;
        /* skproject SKWIN/SkWinCore.cpp lines 46405-46457 load door panels
         * from GDAT_CATEGORY_DOORS using the map-local door graphic index. */
        field = packed & DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_MASK;
        if (packed < 0 ||
            packed >= (0x100 << DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT) ||
            field >= DM2_GDAT_DOOR_PANEL_FIELD_CACHE_LIMIT) {
            return -1;
        }
        index = (packed >> DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT) & 0xff;
        if (gfx->door_panel_pixels[field] &&
            gfx->door_panel_keys[field] != gdat_index) {
            dm2_v1_asset_free_pixels(gfx->door_panel_pixels[field]);
            gfx->door_panel_pixels[field] = NULL;
            gfx->door_panel_w[field] = 0;
            gfx->door_panel_h[field] = 0;
        }
        cache_pixels = &gfx->door_panel_pixels[field];
        cache_w = &gfx->door_panel_w[field];
        cache_h = &gfx->door_panel_h[field];
        gfx->door_panel_keys[field] = gdat_index;
        category = DM2_GDAT_CATEGORY_DOORS;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE &&
               gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE) {
        int packed = DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE - gdat_index;
        int slot = -1;
        if (packed < 0 ||
            packed >= (0x100 << DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT)) {
            return -1;
        }
        index = (packed >> DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT) & 0xff;
        field = packed & DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_FIELD_MASK;
        for (int i = 0; i < DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT; ++i) {
            if (gfx->door_overlay_pixels[i] &&
                gfx->door_overlay_keys[i] == gdat_index) {
                slot = i;
                break;
            }
            if (slot < 0 && !gfx->door_overlay_pixels[i]) {
                slot = i;
            }
        }
        if (slot < 0) {
            slot = (index + field) % DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT;
            dm2_v1_asset_free_pixels(gfx->door_overlay_pixels[slot]);
            gfx->door_overlay_pixels[slot] = NULL;
            gfx->door_overlay_w[slot] = 0;
            gfx->door_overlay_h[slot] = 0;
        }
        cache_pixels = &gfx->door_overlay_pixels[slot];
        cache_w = &gfx->door_overlay_w[slot];
        cache_h = &gfx->door_overlay_h[slot];
        gfx->door_overlay_keys[slot] = gdat_index;
        category = DM2_GDAT_CATEGORY_DOOR_GFX;
    } else if (gdat_index <=
                   DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE -
                       gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT)) {
        int packed =
            DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE - gdat_index;
        int slot = -1;
        if (packed < 0 ||
            packed >= (0x100 << DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT)) {
            return -1;
        }
        index = (packed >> DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT) & 0xff;
        field = packed & DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_FIELD_MASK;
        for (int i = 0; i < DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT; ++i) {
            if (gfx->door_overlay_pixels[i] &&
                gfx->door_overlay_keys[i] == gdat_index) {
                slot = i;
                break;
            }
            if (slot < 0 && !gfx->door_overlay_pixels[i]) {
                slot = i;
            }
        }
        if (slot < 0) {
            slot = (index + field) % DM2_GDAT_DOOR_OVERLAY_CACHE_LIMIT;
            dm2_v1_asset_free_pixels(gfx->door_overlay_pixels[slot]);
            gfx->door_overlay_pixels[slot] = NULL;
            gfx->door_overlay_w[slot] = 0;
            gfx->door_overlay_h[slot] = 0;
        }
        cache_pixels = &gfx->door_overlay_pixels[slot];
        cache_w = &gfx->door_overlay_w[slot];
        cache_h = &gfx->door_overlay_h[slot];
        gfx->door_overlay_keys[slot] = gdat_index;
        category = DM2_GDAT_CATEGORY_DOORS;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE) {
        int packed = DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE - gdat_index;
        int slot = -1;
        if (packed < 0 ||
            packed >= (0x100 << DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT)) {
            return -1;
        }
        index = (packed >> DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT) & 0xff;
        field = packed & DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_MASK;
        wall_button_key = gdat_index;
        for (int i = 0; i < DM2_GDAT_WALL_BUTTON_CACHE_LIMIT; ++i) {
            if (gfx->wall_button_pixels[i] && gfx->wall_button_keys[i] == wall_button_key) {
                slot = i;
                break;
            }
            if (slot < 0 && !gfx->wall_button_pixels[i]) {
                slot = i;
            }
        }
        if (slot < 0) {
            slot = (index + field) % DM2_GDAT_WALL_BUTTON_CACHE_LIMIT;
            dm2_v1_asset_free_pixels(gfx->wall_button_pixels[slot]);
            gfx->wall_button_pixels[slot] = NULL;
            gfx->wall_button_w[slot] = 0;
            gfx->wall_button_h[slot] = 0;
        }
        cache_pixels = &gfx->wall_button_pixels[slot];
        cache_w = &gfx->wall_button_w[slot];
        cache_h = &gfx->wall_button_h[slot];
        gfx->wall_button_keys[slot] = wall_button_key;
        category = DM2_GDAT_CATEGORY_WALL_GFX;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_RELEASED) {
        field = DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE - gdat_index;
        if (field < 0 ||
            field >= DM2_GDAT_DOOR_BUTTON_FIELD_CACHE_LIMIT) {
            return -1;
        }
        cache_pixels = &gfx->door_button_pixels[field];
        cache_w = &gfx->door_button_w[field];
        cache_h = &gfx->door_button_h[field];
        category = DM2_GDAT_CATEGORY_DOOR_BUTTONS;
        index = 0;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT) {
        field = DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - gdat_index;
        if (field < 0 ||
            field >= DM2_GDAT_DOOR_PANEL_FIELD_CACHE_LIMIT) {
            return -1;
        }
        cache_pixels = &gfx->door_panel_pixels[field];
        cache_w = &gfx->door_panel_w[field];
        cache_h = &gfx->door_panel_h[field];
        if (gfx->door_panel_pixels[field] &&
            gfx->door_panel_keys[field] != gdat_index) {
            dm2_v1_asset_free_pixels(gfx->door_panel_pixels[field]);
            gfx->door_panel_pixels[field] = NULL;
            gfx->door_panel_w[field] = 0;
            gfx->door_panel_h[field] = 0;
        }
        gfx->door_panel_keys[field] = gdat_index;
        category = DM2_GDAT_CATEGORY_DOORS;
        index = 0;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT) {
        field = DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE - gdat_index;
        if (field < 0 ||
            field >= DM2_GDAT_DOOR_FRAME_FIELD_CACHE_LIMIT) {
            return -1;
        }
        cache_pixels = &gfx->door_frame_pixels[field];
        cache_w = &gfx->door_frame_w[field];
        cache_h = &gfx->door_frame_h[field];
        category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        index = DM2_GDAT_MAP_GRAPHICSSET_BOOT_WALL;
    } else {
        return -1;
    }

    if (!*cache_pixels) {
        /* skproject SKWIN/defines.h lines ~484-490 names GRAPHICSSET
         * floor/ceiling/front-frame fields; SkWinCore.cpp lines
         * ~47373-47474 and ~48011-48026 draw those through GDAT image
         * queries during dungeon tile rendering. */
        *cache_pixels = dm2_v1_asset_load_image_field(&gfx->loader,
                                                      category,
                                                      index,
                                                      field,
                                                      cache_w,
                                                      cache_h,
                                                      &fmt);
    }
    if (!*cache_pixels || *cache_w <= 0 || *cache_h <= 0) return -1;
    if (out_pixels) *out_pixels = *cache_pixels;
    if (out_w) *out_w = *cache_w;
    if (out_h) *out_h = *cache_h;
    if (out_stride) *out_stride = *cache_w;
    (void)fmt;
    return 0;
}

int dm2_v1_boot_viewport_asset_palette_fetch(void *user,
                                             int gdat_index,
                                             uint8_t out_palette16[16],
                                             uint32_t *out_hash)
{
    DM2_V1_BootProfile *profile = (DM2_V1_BootProfile *)user;
    DM2_V1_BootGraphicsDat *gfx;
    int category;
    int index;
    int field;

    if (out_palette16) memset(out_palette16, 0, 16u);
    if (out_hash) *out_hash = 0u;
    if (!profile || !profile->graphics_dat || !out_palette16 ||
        !dm2_v1_boot_viewport_asset_address(
            gdat_index, &category, &index, &field)) {
        return -1;
    }
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    if (category == DM2_GDAT_CATEGORY_ENVIRONMENT) {
        /* Weather ENVIRONMENT images may be 4bpp IMG3 or 8bpp IMG9.  The
         * 8bpp path has no 16-color local palette; SUMMARY_IMAGE installs a
         * 256-entry identity translation instead. */
        if (!dm2_v1_weather_environment_asset_palette_fetch(
                &gfx->loader, (uint8_t)index, (uint8_t)field,
                out_palette16, out_hash)) {
            return -1;
        }
        return 0;
    }
    if (!dm2_v1_asset_load_image_local_palette(
            &gfx->loader, category, index, field, out_palette16, out_hash)) {
        return -1;
    }
    return 0;
}

static int dm2_v1_boot_viewport_asset_address(int gdat_index,
                                              int *out_category,
                                              int *out_index,
                                              int *out_field)
{
    int packed;
    if (!out_category || !out_index || !out_field) return 0;
    if (dm2_v1_viewport_scene_material_graphic_address(
            gdat_index, out_index, out_field)) {
        *out_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    } else if (dm2_v1_viewport_weather_environment_graphic_address(
                   gdat_index, out_index, out_field)) {
        *out_category = DM2_GDAT_CATEGORY_ENVIRONMENT;
    } else if (gdat_index == dm2_v1_viewport_dialogue_box_graphic_index()) {
        *out_category = DM2_GDAT_CATEGORY_DIALOG_BOXES;
        *out_index = DM2_V1_DIALOGUE_BOX_INDEX;
        *out_field = DM2_V1_DIALOGUE_BOX_FIELD;
    } else if (gdat_index == DM2_V1_VIEWPORT_GFX_TELEPORTER_MAP_CHIP) {
        *out_category = DM2_GDAT_CATEGORY_TELEPORTERS;
        *out_index = 0;
        *out_field = DM2_GDAT_IMG_MAP_CHIP;
    } else if (dm2_v1_viewport_floor_gfx_map_chip_graphic_address(
                   gdat_index, out_index)) {
        *out_category = DM2_GDAT_CATEGORY_FLOOR_GFX;
        *out_field = DM2_GDAT_IMG_MAP_CHIP;
    } else if (dm2_v1_viewport_wall_gfx_map_chip_graphic_address(
                   gdat_index, out_index)) {
        *out_category = DM2_GDAT_CATEGORY_WALL_GFX;
        *out_field = DM2_GDAT_IMG_MAP_CHIP;
    } else if (dm2_v1_viewport_door_map_chip_graphic_address(
                   gdat_index, out_index)) {
        *out_category = DM2_GDAT_CATEGORY_DOORS;
        *out_field = DM2_GDAT_IMG_MAP_CHIP;
    } else if (gdat_index == DM2_V1_VIEWPORT_GFX_FLOOR) {
        *out_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        *out_index = 0;
        *out_field = DM2_GDAT_GFXSET_FLOOR;
    } else if (gdat_index == DM2_V1_VIEWPORT_GFX_CEILING) {
        *out_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        *out_index = 0;
        *out_field = DM2_GDAT_GFXSET_CEIL;
    } else if (dm2_v1_viewport_wall_graphic_address(
                   gdat_index, out_index, out_field)) {
        *out_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE &&
               (((DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff) >=
                   DM2_GDAT_CATEGORY_SPELL_MISSILES &&
               (((DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff) <=
                   DM2_GDAT_CATEGORY_MISCELLANEOUS) {
        packed = DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index;
        *out_category =
            (packed >> DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff;
        *out_index =
            (packed >> DM2_V1_VIEWPORT_GFX_PROJECTILE_INDEX_SHIFT) & 0xff;
        *out_field = DM2_GDAT_IMG_MAP_CHIP;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE &&
               (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) >=
                   DM2_GDAT_CATEGORY_WEAPONS &&
               (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) <=
                   DM2_GDAT_CATEGORY_MISCELLANEOUS) {
        packed = DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index;
        *out_category =
            (packed >> DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff;
        *out_index =
            (packed >> DM2_V1_VIEWPORT_GFX_ITEM_INDEX_SHIFT) & 0xff;
        *out_field = packed & DM2_V1_VIEWPORT_GFX_ITEM_FIELD_MASK;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE &&
               gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE) {
        packed = DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE - gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOORS;
        *out_index = (packed >> DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT) & 0xff;
        *out_field = packed & DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_MASK;
    } else if ((gdat_index <= DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE &&
                DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE - gdat_index <
                    (0x100 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT)) ||
               (gdat_index <= DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE &&
                DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE - gdat_index <
                    (0x100 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT))) {
        int direct_field =
            gdat_index <= DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE;
        packed = (direct_field
            ? DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE
            : DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE) - gdat_index;
        *out_category = DM2_GDAT_CATEGORY_CREATURES;
        *out_index = (packed >> DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT) & 0xff;
        *out_field = direct_field
            ? (packed & DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_MASK)
            : DM2_GDAT_IMG_MAP_CHIP;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE - gdat_index <=
                   DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK) {
        return dm2_v1_boot_hud_core_asset_address(
            DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE - gdat_index,
            out_category,
            out_index,
            out_field);
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE - gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT)) {
        packed = DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE - gdat_index;
        *out_category = DM2_GDAT_CATEGORY_CHAMPIONS;
        *out_index =
            (packed >> DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT) & 0xff;
        *out_field = packed & DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_MASK;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE &&
               gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE) {
        packed = DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE - gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOOR_GFX;
        *out_index =
            (packed >> DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT) & 0xff;
        *out_field = packed & DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_FIELD_MASK;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE - gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT)) {
        packed = DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE -
                 gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOORS;
        *out_index =
            (packed >> DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT) & 0xff;
        *out_field = packed & DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_FIELD_MASK;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE) {
        packed = DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE - gdat_index;
        *out_category = DM2_GDAT_CATEGORY_WALL_GFX;
        *out_index =
            (packed >> DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT) & 0xff;
        *out_field = packed & DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_MASK;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_RELEASED) {
        *out_category = DM2_GDAT_CATEGORY_DOOR_BUTTONS;
        *out_index = 0;
        *out_field = DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE - gdat_index;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE -
                            DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT) {
        *out_category = DM2_GDAT_CATEGORY_DOORS;
        *out_index = 0;
        *out_field = DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - gdat_index;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE -
                            DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT) {
        *out_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        *out_index = DM2_GDAT_MAP_GRAPHICSSET_BOOT_WALL;
        *out_field = DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE - gdat_index;
    } else {
        return 0;
    }
    return *out_field >= 0;
}

int dm2_v1_boot_viewport_asset_evidence(
    DM2_V1_BootProfile *profile,
    int gdat_index,
    DM2_V1_BootViewportAssetEvidence *out_evidence)
{
    const uint8_t *pixels = NULL;
    int w = 0;
    int h = 0;
    int stride = 0;
    uint32_t hash = 0x32445644u;
    int x;
    int y;
    if (!out_evidence) return 0;
    memset(out_evidence, 0, sizeof(*out_evidence));
    out_evidence->gdat_index = gdat_index;
    if (!dm2_v1_boot_viewport_asset_address(gdat_index,
                                            &out_evidence->category,
                                            &out_evidence->entry_index,
                                            &out_evidence->field) ||
        !dm2_v1_boot_gdat_raw_asset_proof(profile,
                                           out_evidence->category,
                                           out_evidence->entry_index,
                                           out_evidence->field,
                                           0x32445652u,
                                           &out_evidence->raw_hash,
                                           &out_evidence->raw_byte_count) ||
        dm2_v1_boot_viewport_asset_fetch(profile, gdat_index, &pixels,
                                         &w, &h, &stride) != 0 ||
        !pixels || w <= 0 || h <= 0 || stride < w) {
        return 0;
    }
    out_evidence->decoded_w = w;
    out_evidence->decoded_h = h;
    out_evidence->decoded_stride = stride;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            hash = dm2_v1_boot_packaged_capture_hash_step(
                hash, pixels[(size_t)y * (size_t)stride + (size_t)x]);
        }
    }
    out_evidence->decoded_hash = hash;
    out_evidence->decoded_pixel_count = (uint32_t)((size_t)w * (size_t)h);
    return out_evidence->raw_hash != 0u && out_evidence->raw_byte_count > 0u &&
           out_evidence->decoded_hash != 0u &&
           out_evidence->decoded_pixel_count > 0u;
}

int dm2_v1_boot_dynamic_creature_material_receipt(
    DM2_V1_BootProfile *profile,
    int creature_type,
    uint16_t command,
    uint16_t previous_frame,
    int direction,
    DM2_V1_BootDynamicCreatureMaterialReceipt *out_receipt)
{
    DM2_V1_BootDynamicCreatureMaterialReceipt candidate;
    DM2_V1_BootGraphicsDat *gfx;
    const DM2_AIDefinition *ai;
    DM2_V1_CreatureAnimationGdatReceipt animation;
    uint8_t palette16[16];
    const uint8_t *raw_material;
    size_t raw_material_size = 0u;
    int gdat_index;
    uint32_t palette_hash = 0u;

    if (!out_receipt || !profile || !profile->graphics_dat ||
        creature_type < 0 || creature_type >= DM2_AI_TABLE_SIZE) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    memset(&animation, 0, sizeof(animation));
    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    ai = dm2_v1_creature_ai_spec(creature_type);
    if (!ai || (ai->w0AIFlags & DM2_AIFLAG_STATIC) != 0u ||
        !dm2_v1_creature_animation_gdat_select_dynamic_v5(
            &gfx->loader, creature_type, command, previous_frame,
            ai->w0AIFlags, direction, &animation) ||
        !animation.valid || !animation.dynamic) {
        return 0;
    }
    gdat_index = dm2_v1_viewport_creature_field_graphic_index(
        creature_type, animation.image_id);
    if (gdat_index == 0 ||
        !dm2_v1_boot_viewport_asset_evidence(
            profile, gdat_index, &candidate.image) ||
        candidate.image.category != DM2_GDAT_CATEGORY_CREATURES ||
        candidate.image.entry_index != creature_type ||
        candidate.image.field != animation.image_id ||
        dm2_v1_boot_viewport_asset_palette_fetch(
            profile, gdat_index, palette16, &palette_hash) != 0 ||
        palette_hash == 0u) {
        return 0;
    }
    raw_material = dm2_v1_asset_load_sized(
        &gfx->loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        animation.image_id, &raw_material_size);
    if (!raw_material || raw_material_size == 0u ||
        raw_material_size > UINT32_MAX) {
        return 0;
    }
    for (uint16_t raw_index = 0u; raw_index < gfx->loader.raw_data_count;
         ++raw_index) {
        if (gfx->loader.raw_sizes[raw_index] == raw_material_size &&
            gfx->loader.data + gfx->loader.raw_offsets[raw_index] ==
                raw_material) {
            DM2_V1_GdatGfxRawMaterialReceipt raw_receipt;

            if (!dm2_v1_gdat_allocate_gfx256_raw_material_receipt(
                    &gfx->loader, raw_index, &raw_receipt) ||
                raw_receipt.source_bytes != raw_material ||
                raw_receipt.source_byte_count != raw_material_size ||
                !raw_receipt.source_hash || !raw_receipt.receipt_hash) {
                return 0;
            }
            candidate.raw_material_index = raw_receipt.raw_index;
            candidate.raw_material_bytes = raw_receipt.source_bytes;
            candidate.raw_material_byte_count = (uint32_t)raw_receipt.source_byte_count;
            candidate.raw_material_hash = raw_receipt.source_hash;
            candidate.raw_material_receipt_hash = raw_receipt.receipt_hash;
            break;
        }
    }
    if (!candidate.raw_material_bytes || candidate.raw_material_byte_count !=
            candidate.image.raw_byte_count || !candidate.raw_material_hash ||
        !candidate.raw_material_receipt_hash) {
        return 0;
    }
    candidate.creature_type = creature_type;
    candidate.command = command;
    candidate.previous_frame = previous_frame;
    candidate.selected_frame = animation.selected_frame;
    candidate.sequence_offset = animation.sequence_offset;
    candidate.direction = animation.direction;
    candidate.image_field = animation.image_id;
    candidate.animation_table_hash = animation.table_hash;
    candidate.palette_hash = palette_hash;
    candidate.material_hash = dm2_v1_boot_packaged_capture_hash_step(
        animation.table_hash, candidate.image.raw_hash);
    candidate.material_hash = dm2_v1_boot_packaged_capture_hash_step(
        candidate.material_hash, candidate.image.decoded_hash);
    candidate.material_hash = dm2_v1_boot_packaged_capture_hash_step(
        candidate.material_hash, palette_hash);
    candidate.material_hash = dm2_v1_boot_packaged_capture_hash_step(
        candidate.material_hash, candidate.raw_material_receipt_hash);
    candidate.valid = candidate.material_hash != 0u;
    if (!candidate.valid) return 0;
    *out_receipt = candidate;
    return 1;
}

static int dm2_v1_boot_object_pool_to_gdat_category(uint8_t pool)
{
    switch (pool) {
        case 5: return DM2_GDAT_CATEGORY_WEAPONS;
        case 6: return DM2_GDAT_CATEGORY_CLOTHES;
        case 7: return DM2_GDAT_CATEGORY_SCROLLS;
        case 10: return DM2_GDAT_CATEGORY_MISCELLANEOUS;
        default: return -1;
    }
}

int dm2_v1_boot_object_icon_asset_fetch(
    DM2_V1_BootProfile *profile,
    uint32_t object_id,
    uint8_t **out_pixels,
    int *out_w,
    int *out_h,
    int *out_stride)
{
    DM2_V1_BootGraphicsDat *gfx;
    uint8_t pool;
    uint32_t index;
    int category;
    int field;
    DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;

    if (out_pixels) *out_pixels = NULL;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (out_stride) *out_stride = 0;
    if (!profile || !profile->graphics_dat || !out_pixels) return -1;
    if (!dm2_db_decode_handle(object_id, &pool, &index)) return -1;
    if (index > 0xffu) return -1;
    category = dm2_v1_boot_object_pool_to_gdat_category(pool);
    if (category < 0) return -1;

    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    for (field = 0; field < DM2_GDAT_OBJECT_ICON_FIELD_LIMIT; ++field) {
        uint8_t *pixels = dm2_v1_asset_load_image_field(&gfx->loader,
                                                        category,
                                                        (int)index,
                                                        field,
                                                        out_w,
                                                        out_h,
                                                        &fmt);
        if (pixels && out_w && out_h && *out_w > 0 && *out_h > 0) {
            *out_pixels = pixels;
            if (out_stride) *out_stride = *out_w;
            (void)fmt;
            return 0;
        }
        dm2_v1_asset_free_pixels(pixels);
    }
    return -1;
}

void dm2_v1_boot_object_icon_asset_free(uint8_t *pixels)
{
    dm2_v1_asset_free_pixels(pixels);
}

int dm2_v1_boot_gdat_image_asset_fetch(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint8_t **out_pixels,
    int *out_w,
    int *out_h,
    int *out_stride)
{
    DM2_V1_BootGraphicsDat *gfx;
    DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;
    uint8_t *pixels;
    size_t bytes;

    if (out_pixels) *out_pixels = NULL;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (out_stride) *out_stride = 0;
    if (!profile || !profile->graphics_dat || !out_pixels) return -1;

    gfx = (DM2_V1_BootGraphicsDat *)profile->graphics_dat;
    /* skproject/SKWIN/SkWinCore.cpp SHOW_MENU_SCREEN lines 55187-55196
     * selects TITLE/0 dt07/4 as the complete 320x200 menu surface before
     * considering the decoded image entry. Keep that source choice inside
     * the boot-owned GDAT API so M11 cannot silently substitute an image
     * field or synthesize a menu when the original raw screen is present. */
    if (category == DM2_GDAT_CATEGORY_TITLE && index == 0 &&
        field == DM2_GDAT_TITLE_MENU_SCREEN_FIELD) {
        const uint8_t *raw;
        size_t raw_size = 0u;

        raw = dm2_v1_asset_load_typed_sized(
            &gfx->loader, category, index, DM2_GDAT_ENTRY_TYPE_RAW7, field,
            &raw_size);
        if (raw && raw_size == 320u * 200u) {
            pixels = (uint8_t *)malloc(raw_size);
            if (!pixels) return -1;
            memcpy(pixels, raw, raw_size);
            *out_pixels = pixels;
            if (out_w) *out_w = 320;
            if (out_h) *out_h = 200;
            if (out_stride) *out_stride = 320;
            return 0;
        }
    }
    if (category == DM2_GDAT_CATEGORY_TITLE && index == 0 && field == 1) {
        if (!gfx->startup_title_pixels) {
            gfx->startup_title_pixels =
                dm2_v1_asset_load_image_field(&gfx->loader,
                                              category,
                                              index,
                                              field,
                                              &gfx->startup_title_w,
                                              &gfx->startup_title_h,
                                              &fmt);
            if (!gfx->startup_title_pixels ||
                gfx->startup_title_w <= 0 ||
                gfx->startup_title_h <= 0) {
                dm2_v1_asset_free_pixels(gfx->startup_title_pixels);
                gfx->startup_title_pixels = NULL;
                gfx->startup_title_w = 0;
                gfx->startup_title_h = 0;
                return -1;
            }
        }
        bytes = (size_t)gfx->startup_title_w * (size_t)gfx->startup_title_h;
        pixels = (uint8_t *)malloc(bytes ? bytes : 1U);
        if (!pixels) return -1;
        memcpy(pixels, gfx->startup_title_pixels, bytes);
        *out_pixels = pixels;
        if (out_w) *out_w = gfx->startup_title_w;
        if (out_h) *out_h = gfx->startup_title_h;
        if (out_stride) *out_stride = gfx->startup_title_w;
        return 0;
    }

    pixels = dm2_v1_asset_load_image_field(&gfx->loader,
                                           category,
                                           index,
                                           field,
                                           out_w,
                                           out_h,
                                           &fmt);
    if (!pixels || !out_w || !out_h || *out_w <= 0 || *out_h <= 0) {
        dm2_v1_asset_free_pixels(pixels);
        return -1;
    }
    *out_pixels = pixels;
    if (out_stride) *out_stride = *out_w;
    (void)fmt;
    return 0;
}

void dm2_v1_boot_gdat_image_asset_free(uint8_t *pixels)
{
    dm2_v1_asset_free_pixels(pixels);
}

/* ── Cleanup ─────────────────────────────────────────────────────────── */

void dm2_v1_boot_cleanup(DM2_V1_BootProfile *profile) {
    if (!profile) return;
    /* The sound singleton borrows the loader stored in graphics_dat. */
    dm2_v1_sound_bind_gdat_loader(NULL, 0);
    if (profile->graphics_dat) {
        dm2_v1_boot_graphics_free(
            (DM2_V1_BootGraphicsDat *)profile->graphics_dat);
        profile->graphics_dat = NULL;
    }
    if (profile->dungeon_data) {
        DM2_V1_DungeonData *dd = (DM2_V1_DungeonData *)profile->dungeon_data;
        dm2_v1_dungeon_free(dd);
        free(dd);
        profile->dungeon_data = NULL;
    }
    if (profile->dm2_state) {
        free(profile->dm2_state);
        profile->dm2_state = NULL;
    }
    profile->graphics_path[0] = '\0';
    profile->dungeon_path[0] = '\0';
}

/* ── Diagnostics ─────────────────────────────────────────────────────── */

size_t dm2_v1_diagnostic_report(const DM2_V1_BootProfile *profile,
                                 char *buf, size_t buf_size) {
    if (!profile || !buf || buf_size == 0) return 0;
    int n = snprintf(buf, buf_size,
        "=== DM2 V1 Boot Profile ===\n"
        "Game:         %s\n"
        "Platform:     %s (%s)\n"
        "Asset root:   %s\n"
        "GRAPHICS:     %s\n"
        "  size:       %zu bytes\n"
        "  MD5:        %.32s%s\n"
        "DUNGEON:      %s\n"
        "  size:       %zu bytes\n"
        "  MD5:        %.32s%s\n"
        "Filenames:    %s\n"
        "Hash verified:%s\n"
        "Save root:    %s\n"
        "\n"
        "=== Deterministic Config ===\n"
        "Tick rate:    %u.%u Hz (~%u ms/tick)\n"
        "Dungeon move: 0x%04x Q8 (%.2f sq/tick)\n"
        "Outdoor move: 0x%04x Q8 (%.2f sq/tick)\n"
        "Max levels:   %u\n"
        "Dungeon seed: %u\n"
        "Day cycle:    %u min (%u ticks)\n"
        "Max champions:%u\n",
        profile->game_id,
        profile->platform_label,
        profile->version_id,
        profile->asset_root,
        profile->graphics_path[0] ? profile->graphics_path : "(not found)",
        profile->graphics_size,
        profile->graphics_md5,
        profile->assets_verified ? "" : "  ← UNVERIFIED",
        profile->dungeon_path[0] ? profile->dungeon_path : "(not found)",
        profile->dungeon_size,
        profile->dungeon_md5,
        profile->assets_verified ? "" : "  ← UNVERIFIED",
        profile->use_dm2_filenames ? "DM2GRAPHICS.DAT/DM2DUNGEON.DAT" : "GRAPHICS.DAT/DUNGEON.DAT",
        profile->assets_verified ? "YES" : "NO",
        profile->save_root,
        profile->deterministic.tick_rate_hz,
        profile->deterministic.tick_rate_hz_frac,
        profile->deterministic.tick_ms,
        profile->deterministic.dungeon_move_speed,
        (double)profile->deterministic.dungeon_move_speed / 256.0,
        profile->deterministic.outdoor_move_speed,
        (double)profile->deterministic.outdoor_move_speed / 256.0,
        profile->deterministic.max_levels,
        profile->deterministic.dungeon_seed,
        profile->deterministic.day_cycle_minutes,
        profile->deterministic.day_cycle_ticks,
        profile->deterministic.max_champions
    );
    if ((size_t)n >= buf_size) return buf_size;
    return (size_t)n;
}

void dm2_v1_boot_print_summary(const DM2_V1_BootProfile *profile) {
    if (!profile) {
        printf("DM2: no profile\n");
        return;
    }
    printf("DM2: %-20s  seed=%-5u  levels=%-2u  tick=%ums  "
           "move=0x%04x/0x%04x\n",
           profile->platform_label,
           profile->deterministic.dungeon_seed,
           profile->deterministic.max_levels,
           profile->deterministic.tick_ms,
           profile->deterministic.dungeon_move_speed,
           profile->deterministic.outdoor_move_speed);
}

const char *dm2_v1_boot_source_evidence(void) {
    return
        "DM2 V1 Boot Profile — Phase 1 implementation\n"
        "Source: SKULL.ASM T560  — DUNGEON_Load: header parsing, dungeon_seed\n"
        "Source: SKULL.ASM T000  — DM2 title screen / startup entry\n"
        "Source: SKULL.ASM T800  — outdoor/shop/NPC entry points\n"
        "Source: SKULL.ASM T520  — party placement and start position\n"
        "Source: SKULL.ASM T048  — platform detection and version label\n"
        "Source: SKULL.ASM T200  — save namespace resolution\n"
        "Asset hashes: PC EN=25247ede4dabb6a71e5dabdfbcd5907d/6caccd7875009e82fe2e28e7f6d6adc0\n"
        "              PC FR=b4d733576ea60c41737f79f212faf528\n"
        "              PC Jewel=e52ab5e01715042b16a4dcff02052e5d\n";
}
