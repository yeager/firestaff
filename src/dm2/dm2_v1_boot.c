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
#include "dm2_v1_boot_startup_view_model.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_game.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_shop.h"
#include "dm2_v1_startup_menu.h"
#include "dm2_v1_startup_presentation.h"
#include "dm2_v1_viewport_renderer.h"
#include "asset_find_by_hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define DM2_GDAT_GFXSET_FLOOR 0x00
#define DM2_GDAT_GFXSET_CEIL  0x01
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
#define DM2_GDAT_IMG_MAP_CHIP 0xF9

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
    uint8_t *ceiling_pixels;
    int ceiling_w;
    int ceiling_h;
    uint8_t *floor_pixels;
    int floor_w;
    int floor_h;
    uint8_t *wall_pixels[DM2_GDAT_WALL_FIELD_CACHE_LIMIT];
    int wall_w[DM2_GDAT_WALL_FIELD_CACHE_LIMIT];
    int wall_h[DM2_GDAT_WALL_FIELD_CACHE_LIMIT];
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
    uint8_t *hud_portrait_pixels[DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT];
    int hud_portrait_w[DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT];
    int hud_portrait_h[DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT];
    uint8_t *startup_title_pixels;
    int startup_title_w;
    int startup_title_h;
    int ccm_program_count;
    int ccm_program_field;
} DM2_V1_BootGraphicsDat;

static int dm2_v1_boot_runtime_raw_gdat_hud_probe(
    DM2_V1_BootProfile *profile,
    int *out_portrait_count,
    uint32_t *out_portrait_hash,
    uint32_t *out_portrait_byte_count,
    uint32_t *out_core_hash,
    uint32_t *out_core_byte_count);
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
    uint32_t *out_core_pixel_count);

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

static void dm2_v1_boot_graphics_free(DM2_V1_BootGraphicsDat *gfx) {
    if (!gfx) return;
    dm2_v1_asset_free_pixels(gfx->ceiling_pixels);
    dm2_v1_asset_free_pixels(gfx->floor_pixels);
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
    for (int i = 0; i < DM2_GDAT_HUD_PORTRAIT_CACHE_LIMIT; ++i) {
        dm2_v1_asset_free_pixels(gfx->hud_portrait_pixels[i]);
    }
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
    FILE *f;
    long fsize;
    size_t got;
    DM2_V1_BootGraphicsDat *gfx;

    if (!graphics_path || graphics_path[0] == '\0') return NULL;
    f = fopen(graphics_path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    fsize = ftell(f);
    if (fsize <= 0 || fsize > 16L * 1024L * 1024L) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    gfx = (DM2_V1_BootGraphicsDat *)calloc(1, sizeof(*gfx));
    if (!gfx) {
        fclose(f);
        return NULL;
    }
    gfx->bytes = (uint8_t *)malloc((size_t)fsize);
    if (!gfx->bytes) {
        fclose(f);
        dm2_v1_boot_graphics_free(gfx);
        return NULL;
    }
    got = fread(gfx->bytes, 1, (size_t)fsize, f);
    fclose(f);
    if (got != (size_t)fsize ||
        dm2_v1_asset_loader_init(&gfx->loader, gfx->bytes, got) != 0 ||
        !dm2_v1_asset_loader_verify(&gfx->loader)) {
        dm2_v1_boot_graphics_free(gfx);
        return NULL;
    }
    (void)dm2_v1_creature_load_ai_table_from_gdat(&gfx->loader);
    gfx->ccm_program_count =
        dm2_v1_creature_load_ccm_programs_from_gdat_auto(
            &gfx->loader, &gfx->ccm_program_field);
    gfx->size = got;
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
    DM2_Md5Ctx ctx;
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
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

/* ── Resolve asset path for a single file ───────────────────────────── */

static int resolve_asset_path(const char *base_dir,
                              const char *subdir,
                              const char *file_candidates[],
                              char resolved_path[512],
                              size_t *out_size,
                              char out_md5[33]) {
    char path[512];
    size_t i;
    for (i = 0; file_candidates[i]; i++) {
        /* Try base_dir/subdir/filename */
        if (subdir && subdir[0]) {
            snprintf(path, sizeof(path), "%s%c%s%c%s",
                     base_dir, DM2_PATH_SEP, subdir, DM2_PATH_SEP, file_candidates[i]);
        } else {
            snprintf(path, sizeof(path), "%s%c%s",
                     base_dir, DM2_PATH_SEP, file_candidates[i]);
        }
        if (file_size(path) > 0) {
            strncpy(resolved_path, path, 511);
            resolved_path[511] = '\0';
            if (out_size) *out_size = file_size(path);
            if (out_md5 && path_md5_hex(path, out_md5)) {
                /* MD5 computed */
            } else if (out_md5) {
                out_md5[0] = '\0';
            }
            return 1;
        }
    }
    return 0;
}

static int resolve_dm2_asset_path(const char *base,
                                  const char *file_candidates[],
                                  char resolved_path[512],
                                  size_t *out_size,
                                  char out_md5[33]) {
    const char *subdirs[] = {
        "dm2",
        "data",
        "",
        NULL
    };
    size_t i;
    for (i = 0; subdirs[i]; ++i) {
        if (resolve_asset_path(base, subdirs[i], file_candidates,
                               resolved_path, out_size, out_md5)) {
            return 1;
        }
    }
    return 0;
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
    profile->graphics_md5[0] = '\0';
    profile->dungeon_md5[0] = '\0';
    profile->graphics_size = 0U;
    profile->dungeon_size = 0U;
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

    /* Legacy fallback for incomplete/synthetic developer fixtures. */
    if (!profile->graphics_path[0]) {
        const char *gfx_candidates[] = {
            "DM2GRAPHICS.DAT",
            "GRAPHICS.DAT",
            "dm2graphics.dat",
            "graphics.dat",
            NULL
        };
        resolve_dm2_asset_path(base, gfx_candidates,
                               profile->graphics_path,
                               &profile->graphics_size,
                               profile->graphics_md5);
    }

    if (!profile->dungeon_path[0]) {
        const char *dun_candidates[] = {
            "DM2DUNGEON.DAT",
            "DUNGEON.DAT",
            "dm2dungeon.dat",
            "dungeon.dat",
            NULL
        };
        resolve_dm2_asset_path(base, dun_candidates,
                               profile->dungeon_path,
                               &profile->dungeon_size,
                               profile->dungeon_md5);
    }

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

    /* Determine if we found both required files */
    if (profile->graphics_path[0] && profile->dungeon_path[0]) {
        return 0;  /* success */
    }
    return -1;  /* missing assets */
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
    char path[512];
    char gfxPath[ASSET_PATH_MAX];
    char dunPath[ASSET_PATH_MAX];
    const char *base = data_dir ? data_dir : ".";
    struct stat st;
    gfxPath[0] = '\0';
    dunPath[0] = '\0';
    if (asset_find_by_md5_list(base, g_dm2_graphics_hashes, gfxPath,
                               (int)sizeof(gfxPath), NULL, 8) &&
        asset_find_by_md5_list(base, g_dm2_dungeon_hashes, dunPath,
                               (int)sizeof(dunPath), NULL, 8)) {
        return 1;
    }
    /* Legacy quick check for incomplete/synthetic developer fixtures:
     * look for DM2DUNGEON.DAT or DUNGEON.DAT in dm2/, data/, or root.
     * Extracted DOS installs use data/dungeon.dat. */
    int hasGfx = 0;
    int hasDungeon = 0;
    snprintf(path, sizeof(path), "%s%cdm2%cDM2DUNGEON.DAT", base, DM2_PATH_SEP, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 1000) hasDungeon = 1;
    snprintf(path, sizeof(path), "%s%cdm2%cDUNGEON.DAT", base, DM2_PATH_SEP, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 1000) hasDungeon = 1;
    snprintf(path, sizeof(path), "%s%cdm2%cGRAPHICS.DAT", base, DM2_PATH_SEP, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 100000) hasGfx = 1;
    snprintf(path, sizeof(path), "%s%cdata%cdungeon.dat", base, DM2_PATH_SEP, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 1000) hasDungeon = 1;
    snprintf(path, sizeof(path), "%s%cdata%cgraphics.dat", base, DM2_PATH_SEP, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 100000) hasGfx = 1;
    snprintf(path, sizeof(path), "%s%cDUNGEON.DAT", base, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 1000) hasDungeon = 1;
    snprintf(path, sizeof(path), "%s%cGRAPHICS.DAT", base, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 100000) hasGfx = 1;
    return hasGfx && hasDungeon;
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
        FILE *f = fopen(profile->dungeon_path, "rb");
        if (f) {
            uint8_t header[64];
            size_t n = fread(header, 1, sizeof(header), f);
            /* Build deterministic config from header */
            if (n >= 12) {
                dm2_v1_boot_build_deterministic_config(
                    profile, header, (int)n);
            }
            /* Re-read full file for dungeon loader */
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);
            if (fsize > 0 && fsize < 10*1024*1024) {
                uint8_t *dat = (uint8_t *)malloc((size_t)fsize);
                if (dat) {
                    size_t got = fread(dat, 1, (size_t)fsize, f);
                    (void)dm2_v1_dungeon_load(dd, dat, (int)got);
                    free(dat);
                }
            }
            fclose(f);
        }
    }

    if (profile->graphics_path[0] != '\0') {
        profile->graphics_dat =
            dm2_v1_boot_graphics_load(profile->graphics_path);
    }

    /* Set default start position (Hall of Champions, north-facing)
     * Source: SKULL.ASM T520 — party_placement
     * PC English DM2 start: mapX=15, mapY=15, facing North */
    gs->party_x = 15;
    gs->party_y = 15;
    gs->party_dir = 0;  /* North */
    gs->current_level = 0;
    gs->outdoor = 0;

    profile->dm2_state = gs;
    profile->dungeon_data = dd;

    return 0;
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
        0,
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

static uint32_t dm2_v1_boot_packaged_capture_hash_step(uint32_t hash,
                                                       uint32_t value)
{
    hash ^= value + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    return hash ? hash : 1u;
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
    receipt->title_cycle_ticks =
        (receipt->title_frame_max + 1) *
        receipt->title_frame_duration_ticks;
    if (receipt->startup_menu_active &&
        receipt->title_frame_duration_ticks > 0 &&
        receipt->title_cycle_ticks > 0) {
        const int cycle_tick =
            receipt->title_animation_tick % receipt->title_cycle_ticks;
        receipt->title_cycle_position_tick = cycle_tick;
        receipt->title_frame_start_tick =
            (cycle_tick / receipt->title_frame_duration_ticks) *
            receipt->title_frame_duration_ticks;
        receipt->title_next_frame_tick =
            receipt->title_frame_start_tick +
            receipt->title_frame_duration_ticks;
        receipt->title_frame_elapsed_ticks =
            cycle_tick - receipt->title_frame_start_tick;
        receipt->title_frame_remaining_ticks =
            receipt->title_next_frame_tick - cycle_tick;
        receipt->title_cycle_remaining_ticks =
            receipt->title_cycle_ticks - cycle_tick;
        receipt->exact_title_timing_ready =
            receipt->title_frame_start_tick <= cycle_tick &&
            cycle_tick < receipt->title_next_frame_tick &&
            receipt->title_frame_elapsed_ticks >= 0 &&
            receipt->title_frame_remaining_ticks > 0 &&
            receipt->title_next_frame_tick <= receipt->title_cycle_ticks &&
            receipt->title_frame ==
                receipt->title_frame_start_tick /
                    receipt->title_frame_duration_ticks;
    } else {
        receipt->exact_title_timing_ready = receipt->title_ready ? 1 : 0;
    }
    receipt->menu_row_count = render->row_count;
    receipt->menu_text_count = render->menu_text_count;
    receipt->selectable_text_count = render->selectable_text_count;
    receipt->selected_highlight_count = render->selected_highlight_count;
    receipt->menu_panel_ready =
        render->panel_rect.x == 78 &&
        render->panel_rect.y == 50 &&
        render->panel_rect.w == 164 &&
        render->panel_rect.h == 122;
    receipt->startup_menu_assets_ready =
        receipt->title_backdrop_ready &&
        receipt->menu_panel_ready &&
        receipt->menu_text_count >= receipt->menu_row_count &&
        receipt->selectable_text_count == receipt->menu_row_count &&
        receipt->selected_highlight_count == 1;
    receipt->full_start_real_asset_ready =
        receipt->full_start_graphics_ready &&
        receipt->title_gdat_asset_ready &&
        receipt->startup_menu_assets_ready &&
        receipt->hud_overlay_suppressed &&
        receipt->hud_runtime_ready;
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
    if (snapshot && snapshot->profile &&
        dm2_v1_boot_runtime_raw_gdat_hud_probe(
            (DM2_V1_BootProfile *)snapshot->profile,
            &receipt->hud_raw_gdat_portrait_count,
            &receipt->hud_raw_gdat_portrait_hash,
            &receipt->hud_raw_gdat_portrait_byte_count,
            &receipt->hud_raw_gdat_core_hash,
            &receipt->hud_raw_gdat_core_byte_count)) {
        receipt->hud_raw_gdat_capture_ready = 1;
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
            receipt->hud_raw_gdat_core_byte_count > 0u;
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
    out_proof->menu_capture_ready =
        host_view->startup_menu_assets_ready &&
        host_view->menu_row_count > 0 &&
        host_view->menu_text_count >= host_view->menu_row_count &&
        host_view->selectable_text_count == host_view->menu_row_count &&
        host_view->selected_highlight_count == 1;
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
        full_start->title_frame_max >= 0 &&
        full_start->title_frame_duration_ticks > 0;
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
        package->title_frame_duration_ticks > 0 &&
        package->title_cycle_ticks > 0 &&
        package->title_next_frame_tick >
            package->title_frame_start_tick &&
        package->title_frame_remaining_ticks > 0;
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
    out_receipt->schedule_next_title_tick =
        out_receipt->render_startup_title &&
        consumer->title_frame_remaining_ticks > 0;
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
          out_receipt->suppress_game_hud &&
          out_receipt->schedule_next_title_tick) ||
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
            if (command->gdat_category == DM2_GDAT_CATEGORY_TITLE) {
                ++out_receipt->title_gdat_command_count;
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
        out_receipt->schedule_next_title_tick &&
        out_receipt->title_frame_duration_ticks > 0 &&
        out_receipt->title_frame_remaining_ticks > 0 &&
        out_receipt->title_next_frame_tick >
            out_receipt->title_animation_tick;
    out_receipt->real_gdat_title_asset_receipt_breadth =
        out_receipt->title_gdat_command_count == 2 &&
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
        out_receipt->executed_rect_count >= 2 &&
        out_receipt->executed_text_count >= package.menu_row_count &&
        out_receipt->suppress_game_hud;
    out_receipt->final_m11_draw_caller_ready =
        out_receipt->consume_host_frame_receipt &&
        out_receipt->execute_startup_draw_commands &&
        out_receipt->packaged_draw_commands_consumed &&
        out_receipt->title_timing_receipt_consumed &&
        out_receipt->real_gdat_title_asset_receipt_breadth &&
        out_receipt->menu_hud_startup_receipt_breadth &&
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
        out_receipt->title_gdat_command_count == 2 &&
        out_receipt->executed_text_count > 0 &&
        out_receipt->suppress_game_hud &&
        (!package.full_start_real_asset_ready ||
         (out_receipt->startup_hud_raw_gdat_receipt_consumed &&
          out_receipt->startup_title_menu_raw_gdat_receipt_consumed &&
          out_receipt->startup_title_menu_decoded_gdat_receipt_consumed)) &&
        out_receipt->schedule_next_title_tick &&
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

    if (!profile || !commands || command_count <= 0 || !out_receipt) {
        return 0;
    }
    frame = (uint8_t *)calloc(320u * 200u, 1u);
    if (!frame) {
        return 0;
    }

    /* skproject/SKWIN/SkWinCore.cpp SHOW_MENU_SCREEN first consumes TITLE
     * GDAT fields 1 and 4, then draws the startup menu while the game HUD is
     * suppressed until runtime handoff. */
    for (i = 0; i < command_count; ++i) {
        const DM2_V1_StartupDrawCommand *command = &commands[i];
        if (command->kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE) {
            uint8_t *pixels = NULL;
            int w = 0;
            int h = 0;
            int stride = 0;
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
            dm2_v1_boot_startup_composite_rect(frame, 320, 200, command);
            ++out_receipt->composite_rect_count;
        } else if (command->kind == DM2_V1_STARTUP_DRAW_TEXT) {
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
    ready =
        out_receipt->composite_gdat_blit_count == 2 &&
        out_receipt->composite_rect_count >= 2 &&
        out_receipt->composite_text_zone_count >= out_receipt->menu_row_count &&
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
    static const int k_title_ticks[] = {0, 13, 47};
    static const int k_selected_rows[] = {0, 1, 2};
    uint32_t title_hashes[3];
    uint32_t menu_hashes[3];
    int title_hash_count = 0;
    int menu_hash_count = 0;
    int i;

    if (!profile || !base_snapshot || !out_receipt) {
        return 0;
    }

    /* skproject/SKWIN title/menu startup is ticked and re-entered while the
     * HUD is suppressed. Sample separate title ticks and menu selections so
     * the real-data receipt proves more than the single current host frame. */
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
        (out_receipt->sampled_title_frame_mask & (1 << 0)) &&
        (out_receipt->sampled_title_frame_mask & (1 << 2)) &&
        (out_receipt->sampled_title_frame_mask & (1 << 7)) &&
        out_receipt->sampled_menu_selection_capture_count >= 3 &&
        out_receipt->sampled_menu_composite_capture_count >= 3 &&
        out_receipt->sampled_menu_unique_composite_hash_count >= 3 &&
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

static int dm2_v1_boot_runtime_raw_gdat_hud_probe(
    DM2_V1_BootProfile *profile,
    int *out_portrait_count,
    uint32_t *out_portrait_hash,
    uint32_t *out_portrait_byte_count,
    uint32_t *out_core_hash,
    uint32_t *out_core_byte_count)
{
    int portrait_index;
    int portrait_count = 0;
    uint32_t portrait_hash = 0x32485052u;
    uint32_t portrait_bytes = 0u;
    uint32_t core_hash = 0x32484352u;
    uint32_t core_bytes = 0u;

    if (out_portrait_count) *out_portrait_count = 0;
    if (out_portrait_hash) *out_portrait_hash = 0u;
    if (out_portrait_byte_count) *out_portrait_byte_count = 0u;
    if (out_core_hash) *out_core_hash = 0u;
    if (out_core_byte_count) *out_core_byte_count = 0u;
    if (!profile || !out_portrait_count || !out_portrait_hash ||
        !out_portrait_byte_count || !out_core_hash || !out_core_byte_count) {
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
    if (!dm2_v1_boot_runtime_raw_gdat_hash_add(profile,
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

static int dm2_v1_boot_runtime_decoded_gdat_hud_probe(
    DM2_V1_BootProfile *profile,
    int *out_portrait_count,
    uint32_t *out_portrait_hash,
    uint32_t *out_portrait_pixel_count,
    uint32_t *out_core_hash,
    uint32_t *out_core_pixel_count)
{
    int portrait_index;
    int portrait_count = 0;
    uint32_t portrait_hash = 0x32485044u;
    uint32_t portrait_pixels = 0u;
    uint32_t core_hash = 0x32484344u;
    uint32_t core_pixels = 0u;

    if (out_portrait_count) *out_portrait_count = 0;
    if (out_portrait_hash) *out_portrait_hash = 0u;
    if (out_portrait_pixel_count) *out_portrait_pixel_count = 0u;
    if (out_core_hash) *out_core_hash = 0u;
    if (out_core_pixel_count) *out_core_pixel_count = 0u;
    if (!profile || !out_portrait_count || !out_portrait_hash ||
        !out_portrait_pixel_count || !out_core_hash ||
        !out_core_pixel_count) {
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
    if (!dm2_v1_boot_runtime_decoded_gdat_hash_add(
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
    DM2_V1_BootRuntimeHudCaptureReceipt runtime_hud;
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

    if (dm2_v1_boot_gdat_raw_asset_proof(profile,
                                         package.title_gdat_category,
                                         package.title_gdat_index,
                                         package.title_gdat_field,
                                         0x32545257u,
                                         &out_receipt->title_raw_byte_hash,
                                         &out_receipt->title_raw_byte_count) &&
        dm2_v1_boot_gdat_raw_asset_proof(profile,
                                         out_receipt->menu_gdat_category,
                                         out_receipt->menu_gdat_index,
                                         out_receipt->menu_gdat_field,
                                         0x324d5257u,
                                         &out_receipt->menu_raw_byte_hash,
                                         &out_receipt->menu_raw_byte_count)) {
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

    if (dm2_v1_boot_gdat_image_asset_fetch(profile,
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
        out_receipt->menu_gdat_command_count == 2 &&
        out_receipt->menu_rect_command_count >= 2 &&
        out_receipt->menu_text_command_count >= out_receipt->menu_row_count &&
        out_receipt->selected_highlight_count >= 1;
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
        out_receipt->exact_selected_highlight_ready &&
        out_receipt->hud_handoff_capture_ready &&
        out_receipt->suppress_game_hud &&
        !out_receipt->present_first_hud_frame;
    out_receipt->title_menu_hud_visual_proof_ready =
        out_receipt->menu_title_composite_capture_ready &&
        out_receipt->full_visual_composite_capture_ready &&
        out_receipt->skproject_title_query_ready &&
        out_receipt->skproject_menu_query_ready &&
        out_receipt->hud_suppressed_capture_ready &&
        out_receipt->no_fallback_title_blit;
    (void)dm2_v1_boot_startup_real_visual_breadth_probe(profile,
                                                        &snapshot,
                                                        out_receipt);
    dm2_v1_boot_runtime_hud_capture_receipt_init(&runtime_hud);
    if (dm2_v1_boot_runtime_hud_capture_receipt(profile, &runtime_hud) &&
        runtime_hud.valid &&
        runtime_hud.real_gdat_runtime_hud_breadth_ready) {
        out_receipt->runtime_hud_capture_consumed = 1;
        out_receipt->runtime_hud_real_gdat_ready = 1;
        out_receipt->runtime_hud_direction_mask =
            runtime_hud.runtime_direction_mask;
        out_receipt->runtime_hud_sample_count =
            runtime_hud.render_sample_count;
        out_receipt->runtime_hud_unique_frame_hash_count =
            runtime_hud.unique_frame_hash_count;
        out_receipt->runtime_hud_min_asset_portrait_count =
            runtime_hud.min_asset_portrait_count;
        out_receipt->runtime_hud_total_fallback_portrait_count =
            runtime_hud.total_fallback_portrait_count;
        out_receipt->runtime_hud_min_asset_floor_ceiling_count =
            runtime_hud.min_asset_floor_ceiling_count;
        out_receipt->runtime_hud_total_fallback_floor_ceiling_count =
            runtime_hud.total_fallback_floor_ceiling_count;
        out_receipt->runtime_hud_min_asset_wall_count =
            runtime_hud.min_asset_wall_count;
        out_receipt->runtime_hud_total_fallback_wall_count =
            runtime_hud.total_fallback_wall_count;
        out_receipt->runtime_hud_raw_gdat_capture_ready =
            runtime_hud.raw_gdat_runtime_hud_capture_ready;
        out_receipt->runtime_hud_raw_portrait_count =
            runtime_hud.raw_gdat_runtime_portrait_count;
        out_receipt->runtime_hud_raw_portrait_hash =
            runtime_hud.raw_gdat_runtime_portrait_hash;
        out_receipt->runtime_hud_raw_portrait_byte_count =
            runtime_hud.raw_gdat_runtime_portrait_byte_count;
        out_receipt->runtime_hud_raw_core_hash =
            runtime_hud.raw_gdat_runtime_core_hash;
        out_receipt->runtime_hud_raw_core_byte_count =
            runtime_hud.raw_gdat_runtime_core_byte_count;
        out_receipt->runtime_hud_decoded_gdat_capture_ready =
            runtime_hud.decoded_gdat_runtime_hud_capture_ready;
        out_receipt->runtime_hud_decoded_portrait_count =
            runtime_hud.decoded_gdat_runtime_portrait_count;
        out_receipt->runtime_hud_decoded_portrait_hash =
            runtime_hud.decoded_gdat_runtime_portrait_hash;
        out_receipt->runtime_hud_decoded_portrait_pixel_count =
            runtime_hud.decoded_gdat_runtime_portrait_pixel_count;
        out_receipt->runtime_hud_decoded_core_hash =
            runtime_hud.decoded_gdat_runtime_core_hash;
        out_receipt->runtime_hud_decoded_core_pixel_count =
            runtime_hud.decoded_gdat_runtime_core_pixel_count;
        out_receipt->runtime_hud_frame_hash =
            runtime_hud.combined_frame_hash;
        out_receipt->runtime_hud_pixel_count =
            runtime_hud.combined_pixel_count;
    }
    out_receipt->real_visual_status_consumer_ready =
        out_receipt->real_visual_capture_consumes_package &&
        out_receipt->real_visual_capture_consumes_host_frame &&
        out_receipt->packaged_status_consumed &&
        out_receipt->packaged_startup_phase_consumed &&
        out_receipt->packaged_hud_suppression_consumed &&
        out_receipt->title_menu_hud_visual_proof_ready &&
        out_receipt->real_gdat_capture_breadth_ready &&
        out_receipt->runtime_hud_capture_consumed;

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
        hash, out_receipt->composite_pixel_hash);
    hash = dm2_v1_boot_packaged_capture_hash_step(
        hash, out_receipt->composite_gdat_blit_count);
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
        out_receipt->runtime_hud_capture_consumed &&
        out_receipt->runtime_hud_real_gdat_ready &&
        out_receipt->runtime_hud_direction_mask == 0x0f &&
        out_receipt->runtime_hud_sample_count == 4 &&
        out_receipt->runtime_hud_unique_frame_hash_count > 0 &&
        out_receipt->runtime_hud_min_asset_portrait_count >= 4 &&
        out_receipt->runtime_hud_total_fallback_portrait_count == 0 &&
        out_receipt->runtime_hud_min_asset_floor_ceiling_count >= 2 &&
        out_receipt->runtime_hud_total_fallback_floor_ceiling_count == 0 &&
        out_receipt->runtime_hud_min_asset_wall_count > 0 &&
        out_receipt->runtime_hud_total_fallback_wall_count == 0 &&
        out_receipt->runtime_hud_raw_gdat_capture_ready &&
        out_receipt->runtime_hud_raw_portrait_count >= 4 &&
        out_receipt->runtime_hud_raw_portrait_hash != 0u &&
        out_receipt->runtime_hud_raw_portrait_byte_count > 0u &&
        out_receipt->runtime_hud_raw_core_hash != 0u &&
        out_receipt->runtime_hud_raw_core_byte_count > 0u &&
        out_receipt->runtime_hud_decoded_gdat_capture_ready &&
        out_receipt->runtime_hud_decoded_portrait_count >= 4 &&
        out_receipt->runtime_hud_decoded_portrait_hash != 0u &&
        out_receipt->runtime_hud_decoded_portrait_pixel_count > 0u &&
        out_receipt->runtime_hud_decoded_core_hash != 0u &&
        out_receipt->runtime_hud_decoded_core_pixel_count > 0u &&
        out_receipt->runtime_hud_frame_hash != 0u &&
        out_receipt->runtime_hud_pixel_count == 4u * 320u * 200u &&
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
            if (!out_receipt->inspect_title) {
                out_receipt->inspect_title = "DM2 NPC";
            }
            if (!out_receipt->inspect_text) {
                out_receipt->inspect_text = "WELCOME, TRAVELER.";
            }
        } else if (dm2_v1_runtime_invoke_square_actuators(level, fx, fy) > 0 ||
                   dm2_v1_runtime_invoke_actuator(
                       level,
                       fx,
                       fy,
                       DM2_ACTUATOR_PUSH_BUTTON_WALL_SWITCH,
                       0u) == 0) {
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
        out_receipt->runtime_hud_no_fallback_portraits =
            out_receipt->runtime_hud_asset_portrait_count > 0 &&
            out_receipt->runtime_hud_fallback_portrait_count == 0;
        out_receipt->runtime_hud_raw_gdat_capture_ready =
            dm2_v1_boot_runtime_raw_gdat_hud_probe(
                profile,
                &out_receipt->runtime_hud_raw_portrait_count,
                &out_receipt->runtime_hud_raw_portrait_hash,
                &out_receipt->runtime_hud_raw_portrait_byte_count,
                &out_receipt->runtime_hud_raw_core_hash,
                &out_receipt->runtime_hud_raw_core_byte_count);
        out_receipt->runtime_hud_decoded_gdat_capture_ready =
            dm2_v1_boot_runtime_decoded_gdat_hud_probe(
                profile,
                &out_receipt->runtime_hud_decoded_portrait_count,
                &out_receipt->runtime_hud_decoded_portrait_hash,
                &out_receipt->runtime_hud_decoded_portrait_pixel_count,
                &out_receipt->runtime_hud_decoded_core_hash,
                &out_receipt->runtime_hud_decoded_core_pixel_count);
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
            out_receipt->runtime_hud_frame_hash != 0u;
        out_receipt->runtime_hud_capture_ready =
            out_receipt->startup_render_ready &&
            out_receipt->runtime_hud_real_asset_ready;
        out_receipt->runtime_render_asset_floor_ceiling_count =
            dm2_v1_runtime_last_asset_floor_ceiling_count();
        out_receipt->runtime_render_fallback_floor_ceiling_count =
            dm2_v1_runtime_last_fallback_floor_ceiling_count();
        out_receipt->runtime_render_asset_wall_count =
            dm2_v1_runtime_last_asset_wall_count();
        out_receipt->runtime_render_fallback_wall_count =
            dm2_v1_runtime_last_fallback_wall_count();
        out_receipt->runtime_render_no_core_fallbacks =
            out_receipt->runtime_render_asset_floor_ceiling_count >= 2 &&
            out_receipt->runtime_render_fallback_floor_ceiling_count == 0 &&
            out_receipt->runtime_render_asset_wall_count > 0 &&
            out_receipt->runtime_render_fallback_wall_count == 0;
        out_receipt->runtime_render_real_asset_ready =
            out_receipt->runtime_hud_capture_ready &&
            out_receipt->runtime_render_no_core_fallbacks;
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
    out_receipt->no_fallback_portraits =
        out_receipt->total_asset_portrait_count > 0 &&
        out_receipt->total_fallback_portrait_count == 0;
    out_receipt->no_core_render_fallbacks =
        out_receipt->total_asset_floor_ceiling_count > 0 &&
        out_receipt->total_fallback_floor_ceiling_count == 0 &&
        out_receipt->total_asset_wall_count > 0 &&
        out_receipt->total_fallback_wall_count == 0;
    out_receipt->first_runtime_hud_ready =
        out_receipt->first_frame.runtime_hud_capture_ready;
    out_receipt->real_gdat_portrait_ready =
        out_receipt->graphics_dat_ready &&
        out_receipt->min_asset_portrait_count >= 4 &&
        out_receipt->no_fallback_portraits;
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
            &out_receipt->raw_gdat_runtime_core_byte_count);
    out_receipt->decoded_gdat_runtime_hud_capture_ready =
        dm2_v1_boot_runtime_decoded_gdat_hud_probe(
            profile,
            &out_receipt->decoded_gdat_runtime_portrait_count,
            &out_receipt->decoded_gdat_runtime_portrait_hash,
            &out_receipt->decoded_gdat_runtime_portrait_pixel_count,
            &out_receipt->decoded_gdat_runtime_core_hash,
            &out_receipt->decoded_gdat_runtime_core_pixel_count);
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
        out_receipt->combined_frame_hash != 0u &&
        out_receipt->combined_pixel_count == 4u * 320u * 200u;
    out_receipt->valid =
        out_receipt->profile_ready &&
        out_receipt->runtime_ready &&
        out_receipt->real_gdat_runtime_hud_breadth_ready;
    return out_receipt->valid;
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

    if (gdat_index == DM2_V1_VIEWPORT_GFX_CEILING) {
        cache_pixels = &gfx->ceiling_pixels;
        cache_w = &gfx->ceiling_w;
        cache_h = &gfx->ceiling_h;
        category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        index = 0;
        field = DM2_GDAT_GFXSET_CEIL;
    } else if (gdat_index == DM2_V1_VIEWPORT_GFX_FLOOR) {
        cache_pixels = &gfx->floor_pixels;
        cache_w = &gfx->floor_w;
        cache_h = &gfx->floor_h;
        category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        index = 0;
        field = DM2_GDAT_GFXSET_FLOOR;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST &&
               gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE) {
        field = DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE - gdat_index;
        if (field < 0 || field >= DM2_GDAT_WALL_FIELD_CACHE_LIMIT) {
            return -1;
        }
        cache_pixels = &gfx->wall_pixels[field];
        cache_w = &gfx->wall_w[field];
        cache_h = &gfx->wall_h[field];
        category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        index = DM2_GDAT_MAP_GRAPHICSSET_BOOT_WALL;
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
        field = DM2_GDAT_IMG_MAP_CHIP;
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
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE - gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT)) {
        int packed = DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE - gdat_index;
        int slot = -1;
        if (packed < 0 ||
            packed >= (0x100 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT)) {
            return -1;
        }
        category = DM2_GDAT_CATEGORY_CREATURES;
        index = (packed >> DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT) & 0xff;
        field = DM2_GDAT_IMG_MAP_CHIP;
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
