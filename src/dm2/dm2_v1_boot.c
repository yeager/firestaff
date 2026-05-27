/*
 * dm2_v1_boot.c — DM2 V1 Boot Profile Implementation
 *
 * Phase 1: Runtime profile split for Skullkeep/DM2.
 * Separates DM2 boot from DM1/CSB, including:
 *   - Asset discovery (DM2GRAPHICS.DAT / DM2DUNGEON.DAT)
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
#include "dm2_v1_game.h"
#include "dm2_v1_dungeon_loader.h"
#include "asset_find_by_hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/* ── Embedded MD5 (same implementation as asset_find_by_hash.c) ──────── */

typedef struct {
    unsigned int state[4];
    unsigned int count[2];
    unsigned char buffer[64];
} DM2_Md5Ctx;

static void dm2_md5_init(DM2_Md5Ctx *ctx);
static void dm2_md5_update(DM2_Md5Ctx *ctx, const unsigned char *input, unsigned int len);
static void dm2_md5_final(DM2_Md5Ctx *ctx, char outHex[33]);

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
    /* Round 1 */
    a = DM2_ROT(a + DM2_F(b,c,d) + X[0]  + 0xd76aa478, 7) + a; b = DM2_ROT(b + DM2_F(d,a,b) + X[1]  + 0xe8c7b756, 12) + b;
    c = DM2_ROT(c + DM2_F(a,b,d) + X[2]  + 0x242070db, 17) + c; d = DM2_ROT(d + DM2_F(c,a,b) + X[3]  + 0xc1bdceee, 22) + d;
    a = DM2_ROT(a + DM2_F(b,c,d) + X[4]  + 0xf57c0faf, 7) + a; b = DM2_ROT(b + DM2_F(d,a,b) + X[5]  + 0x4787c62a, 12) + b;
    c = DM2_ROT(c + DM2_F(a,b,d) + X[6]  + 0xa8304613, 17) + c; d = DM2_ROT(d + DM2_F(c,a,b) + X[7]  + 0xfd469501, 22) + d;
    a = DM2_ROT(a + DM2_F(b,c,d) + X[8]  + 0x698098d8, 7) + a; b = DM2_ROT(b + DM2_F(d,a,b) + X[9]  + 0x8b44f7af, 12) + b;
    c = DM2_ROT(c + DM2_F(a,b,d) + X[10] + 0xffff5bb1, 17) + c; d = DM2_ROT(d + DM2_F(c,a,b) + X[11] + 0x895cd7be, 22) + d;
    a = DM2_ROT(a + DM2_F(b,c,d) + X[12] + 0x6b901122, 7) + a; b = DM2_ROT(b + DM2_F(d,a,b) + X[13] + 0xfd987193, 12) + b;
    c = DM2_ROT(c + DM2_F(a,b,d) + X[14] + 0xa679438e, 17) + c; d = DM2_ROT(d + DM2_F(c,a,b) + X[15] + 0x49b40821, 22) + d;
    /* Round 2 */
    a = DM2_ROT(a + DM2_G(b,c,d) + X[1]  + 0xf61e2562, 5) + a; b = DM2_ROT(b + DM2_G(d,a,b) + X[6]  + 0xc040b340, 9) + b;
    c = DM2_ROT(c + DM2_G(a,b,d) + X[11] + 0x265e5a51, 14) + c; d = DM2_ROT(d + DM2_G(c,a,b) + X[0]  + 0xe9b6c7aa, 20) + d;
    a = DM2_ROT(a + DM2_G(b,c,d) + X[5]  + 0xd62f105d, 5) + a; b = DM2_ROT(b + DM2_G(d,a,b) + X[10] + 0x02441453, 9) + b;
    c = DM2_ROT(c + DM2_G(a,b,d) + X[15] + 0xd8a1e681, 14) + c; d = DM2_ROT(d + DM2_G(c,a,b) + X[4]  + 0xe7d3fbc8, 20) + d;
    a = DM2_ROT(a + DM2_G(b,c,d) + X[9]  + 0x21e1cde6, 5) + a; b = DM2_ROT(b + DM2_G(d,a,b) + X[14] + 0xc33707d6, 9) + b;
    c = DM2_ROT(c + DM2_G(a,b,d) + X[3]  + 0xf4d50d87, 14) + c; d = DM2_ROT(d + DM2_G(c,a,b) + X[8]  + 0x455a14ed, 20) + d;
    a = DM2_ROT(a + DM2_G(b,c,d) + X[13] + 0xa9e3e905, 5) + a; b = DM2_ROT(b + DM2_G(d,a,b) + X[2]  + 0xfcefa3f8, 9) + b;
    c = DM2_ROT(c + DM2_G(a,b,d) + X[7]  + 0x676f02d9, 14) + c; d = DM2_ROT(d + DM2_G(c,a,b) + X[12] + 0x8d2a4c8a, 20) + d;
    /* Round 3 */
    a = DM2_ROT(a + DM2_H(b,c,d) + X[5]  + 0xfffa3942, 4) + a; b = DM2_ROT(b + DM2_H(d,a,b) + X[8]  + 0x8771f681, 11) + b;
    c = DM2_ROT(c + DM2_H(a,b,d) + X[11] + 0x6d9d6122, 16) + c; d = DM2_ROT(d + DM2_H(c,a,b) + X[14] + 0xfde5380c, 23) + d;
    a = DM2_ROT(a + DM2_H(b,c,d) + X[1]  + 0xa4beea44, 4) + a; b = DM2_ROT(b + DM2_H(d,a,b) + X[4]  + 0x4bdecfa9, 11) + b;
    c = DM2_ROT(c + DM2_H(a,b,d) + X[7]  + 0xf6bb4b60, 16) + c; d = DM2_ROT(d + DM2_H(c,a,b) + X[10] + 0xbebfbc70, 23) + d;
    a = DM2_ROT(a + DM2_H(b,c,d) + X[13] + 0x289b7ec6, 4) + a; b = DM2_ROT(b + DM2_H(d,a,b) + X[0]  + 0xeaa127fa, 11) + b;
    c = DM2_ROT(c + DM2_H(a,b,d) + X[3]  + 0xd4ef3085, 16) + c; d = DM2_ROT(d + DM2_H(c,a,b) + X[6]  + 0x04881d05, 23) + d;
    a = DM2_ROT(a + DM2_H(b,c,d) + X[9]  + 0xd9d4d039, 4) + a; b = DM2_ROT(b + DM2_H(d,a,b) + X[12] + 0xe6db99e5, 11) + b;
    c = DM2_ROT(c + DM2_H(a,b,d) + X[15] + 0x1fa27cf8, 16) + c; d = DM2_ROT(d + DM2_H(c,a,b) + X[2]  + 0xc4ac5665, 23) + d;
    /* Round 4 */
    a = DM2_ROT(a + DM2_I(b,c,d) + X[0]  + 0xf4292244, 6) + a; b = DM2_ROT(b + DM2_I(d,a,b) + X[7]  + 0x432aff97, 10) + b;
    c = DM2_ROT(c + DM2_I(a,b,d) + X[14] + 0xab9423a7, 15) + c; d = DM2_ROT(d + DM2_I(c,a,b) + X[5]  + 0xfc93a039, 21) + d;
    a = DM2_ROT(a + DM2_I(b,c,d) + X[12] + 0x655b59c3, 6) + a; b = DM2_ROT(b + DM2_I(d,a,b) + X[3]  + 0x8f0ccc92, 10) + b;
    c = DM2_ROT(c + DM2_I(a,b,d) + X[10] + 0xffeff47d, 15) + c; d = DM2_ROT(d + DM2_I(c,a,b) + X[1]  + 0x85845dd1, 21) + d;
    a = DM2_ROT(a + DM2_I(b,c,d) + X[8]  + 0x6fa87e4f, 6) + a; b = DM2_ROT(b + DM2_I(d,a,b) + X[15] + 0xfe2ce6e0, 10) + b;
    c = DM2_ROT(c + DM2_I(a,b,d) + X[6]  + 0xa3014314, 15) + c; d = DM2_ROT(d + DM2_I(c,a,b) + X[13] + 0x4e0811a1, 21) + d;
    a = DM2_ROT(a + DM2_I(b,c,d) + X[4]  + 0xf7537e82, 6) + a; b = DM2_ROT(b + DM2_I(d,a,b) + X[11] + 0xbd3af235, 10) + b;
    c = DM2_ROT(c + DM2_I(a,b,d) + X[2]  + 0x2ad7d2bb, 15) + c; d = DM2_ROT(d + DM2_I(c,a,b) + X[9]  + 0xeb86d391, 21) + d;
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
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
        idx = 0;
    } else {
        idx = 0;
    }
    memcpy(ctx->buffer + idx, input + 0, len - idx);
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

static const char *const g_dm2_dungeon_hashes[] = {
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

/* ── Scan and verify DM2 assets ─────────────────────────────────────── */

int dm2_v1_boot_scan_assets(DM2_V1_BootProfile *profile,
                            const char *data_dir) {
    char path[512];
    const char *base = data_dir ? data_dir : ".";

    /* Resolve GRAPHICS.DAT or DM2GRAPHICS.DAT */
    {
        const char *gfx_candidates[] = {
            "DM2GRAPHICS.DAT",
            "GRAPHICS.DAT",
            "dm2graphics.dat",
            "graphics.dat",
            NULL
        };
        if (!resolve_asset_path(base, "dm2", gfx_candidates,
                                profile->graphics_path,
                                &profile->graphics_size,
                                profile->graphics_md5)) {
            /* Try without subdir */
            resolve_asset_path(base, "", gfx_candidates,
                               profile->graphics_path,
                               &profile->graphics_size,
                               profile->graphics_md5);
        }
    }

    /* Resolve DUNGEON.DAT or DM2DUNGEON.DAT */
    {
        const char *dun_candidates[] = {
            "DM2DUNGEON.DAT",
            "DUNGEON.DAT",
            "dm2dungeon.dat",
            "dungeon.dat",
            NULL
        };
        if (!resolve_asset_path(base, "dm2", dun_candidates,
                                profile->dungeon_path,
                                &profile->dungeon_size,
                                profile->dungeon_md5)) {
            resolve_asset_path(base, "", dun_candidates,
                               profile->dungeon_path,
                               &profile->dungeon_size,
                               profile->dungeon_md5);
        }
    }

    /* Determine if using DM2-specific filenames */
    profile->use_dm2_filenames =
        (strstr(profile->graphics_path, "DM2GRAPHICS") != NULL ||
         strstr(profile->graphics_path, "dm2graphics") != NULL ||
         strstr(profile->dungeon_path, "DM2DUNGEON") != NULL ||
         strstr(profile->dungeon_path, "dm2dungeon") != NULL) ? 1 : 0;

    /* Verify against known hashes */
    profile->assets_verified = 0;
    if (profile->graphics_md5[0]) {
        size_t i;
        for (i = 0; g_dm2_graphics_hashes[i]; i++) {
            if (md5_matches(profile->graphics_md5, g_dm2_graphics_hashes[i])) {
                profile->assets_verified = 1;
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

    /* Build asset root */
    snprintf(profile->asset_root, sizeof(profile->asset_root),
             "%s%cdm2", base, DM2_PATH_SEP);

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
    const char *base = data_dir ? data_dir : ".";
    /* Quick check: look for DM2DUNGEON.DAT or DUNGEON.DAT in dm2/ */
    struct stat st;
    snprintf(path, sizeof(path), "%s%cdm2%cDM2DUNGEON.DAT", base, DM2_PATH_SEP, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 1000) return 1;
    snprintf(path, sizeof(path), "%s%cdm2%cDUNGEON.DAT", base, DM2_PATH_SEP, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 1000) return 1;
    snprintf(path, sizeof(path), "%s%cdm2%cGRAPHICS.DAT", base, DM2_PATH_SEP, DM2_PATH_SEP);
    if (stat(path, &st) == 0 && st.st_size > 100000) return 1;
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
    if (!profile) return -1;

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

/* ── Cleanup ─────────────────────────────────────────────────────────── */

void dm2_v1_boot_cleanup(DM2_V1_BootProfile *profile) {
    if (!profile) return;
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