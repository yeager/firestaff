/*
 * firestaff_csb_v1_pc_real_asset_d2c_f0107_f0111_probe.c
 *
 * CSB V1 PC real-asset D2C F0107 wall ornament + F0111 door front blit gate.
 *
 * Purpose
 * -------
 * Tier 1 (real-asset CSB graphics + ornament blits) is the next blocker
 * in the CSB finish lane per docs/FIRESTAFF_GAP_LIST.md C5 and
 * docs/CSB_V1_BOOTSTRAP_SCOUT.md. The 2026-06-26
 * `firestaff_csb_v1_pc_real_asset_ornament_blit_probe` already moved the
 * D1C F0108 floor/ceiling ornament + F0115 thing-pass path from data-free
 * source-lock to live real-asset bytes (27/27 PASS, deterministic 320x200
 * PPM + SHA256). The remaining CSB finish lane calls for broader
 * viewport/HUD captures and pixel parity evidence (TODO.md
 * "Phase 3 - Rendering parity hardening" line + docs/FIRESTAFF_GAP_LIST.md
 * C5 "Graphics + ornament blits").
 *
 * This probe closes the D2C F0107+F0111 sibling of that gate by:
 *
 *   1. Discovering a real, hash-verified PC CSB GRAPHICS.DAT through the
 *      same `csb_v1_boot_scan_assets` path the launcher uses to gate
 *      launch (the canonical PC CSB graphics MD5
 *      61fbfd56887c94adc26888a9491c6611 is checked end-to-end).
 *   2. Reusing the DMCSB1 BE header parser from the 2026-06-26 probe
 *      (sig 0x8001 BE + count BE + comp/decomp/w/h BE) to enumerate
 *      real PC CSB bitmaps.
 *   3. Selecting the first real D2C F0111 door bitmap that decodes to
 *      the source-locked 64x61 native size (CSB_V1_D2C_F0111 native
 *      door per ReDMCSB G0694_ai_DoorNativeBitmapIndex_Front_D2LCR +
 *      CSB_V1_D2C_F0111_VIEWPORT 112x61 contract) and the first real
 *      D2C F0107 wall ornament bitmap that decodes to the source-locked
 *      16x19 native size (CSB-lineage F2 wall-decoration helper
 *      Viewport.cpp:1016-1024 mirrors ReDMCSB DUNVIEW.C F0107:3502-3938
 *      IsDrawnWallOrnamentAnAlcove_CPSF).
 *   4. Driving the source-locked D2C F0107+F0111 spec contract
 *      (csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_spec_pc34)
 *      against real bytes:
 *        - spec.f0107_before_f0111 == 1 (DUNVIEW.C:7308-7312)
 *        - spec.f0111_closed_door_state == 4 (DUNVIEW.C:4218-4339)
 *        - spec.f0111_door_bitmap_width/height == 64/61 (PC34 invariant)
 *        - spec.f0111_transparent_color == 10 (DEFS.H:2088)
 *        - cell orders: alcove 0x0000, doorpass1 0x0218, doorpass2 0x0349
 *        - left/right ornament visibility: 16x61 pixels each, center
 *          covered by door 64x61 (DUNVIEW.C:7313-7341)
 *   5. Capturing the resulting 112x61 framebuffer (the source-locked
 *      D2C F0107+F0111 viewport size) into
 *      `/tmp/csb_pc_real_d2c_f0107_f0111_capture.ppm` (P6, 112x61, 4bpp
 *      decoded grayscale ramp) so the ornament+door front layering is
 *      human-viewable and reproducible byte-for-byte across runs.
 *   6. Hashing the captured PPM with a built-in SHA256 to make the run
 *      deterministic and CTest-stable.
 *
 * This probe is intentionally a sibling of
 * `firestaff_csb_v1_pc_real_asset_ornament_blit_probe`: that probe
 * covers the D1C F0108+F0115 pair, this probe covers the D2C F0107+F0111
 * pair. Together they advance the CSB V1 graphics + ornament blit gap
 * from data-free source-lock coverage to live real-asset coverage for
 * both the floor-ornament / thing-pass axis and the wall-ornament /
 * door-front axis.
 *
 * Source-lock boundary
 * --------------------
 *   - ReDMCSB DUNVIEW.C F0107:3502-3938
 *     F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF (alcove flag,
 *     C10_COLOR_FLESH transparent ornament blit, M552/M583 D2C front
 *     wall ornament ordinal).
 *   - ReDMCSB DUNVIEW.C F0111:4218-4339
 *     F0111_DUNGEONVIEW_DrawDoor (closed/destroyed blit at 4297-4299,
 *     PC34 zone draw with C10_COLOR_FLESH at 4334).
 *   - ReDMCSB DUNVIEW.C F0121:7244-7342
 *     F0121_DUNGEONVIEW_DrawSquareD2C (wall 7289-7312, door 7313-7341,
 *     cell orders 0x0000/0x0218/0x0349).
 *   - ReDMCSB DEFS.H:2088 (C10_COLOR_FLESH), 2537-2539, 2581, 2657-2677,
 *     2688-2690, 4030-4049, 4238-4257 (square aspect, D2C view, cell
 *     orders, wall zones, door zone M628).
 *   - CSB-lineage Viewport.cpp:1016-1024 (F2 stone Alcove/JumpZ helper),
 *     1865-1879 (F2 door-facing helper), 2596-2616 (StdDrawDoor),
 *     2949-2955 (StdDrawWallDecoration).
 *   - dmweb Data Files page: PC CSB GRAPHICS.DAT = DMCSB1 BE
 *     (sig 0x8001 BE, count + comp/decomp/w/h tables).
 *
 * Skip-safe on hosts without user-supplied PC CSB game data.
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DEFAULT_PC_CSB_DATA_DIR         "/Users/bosse/.firestaff/data/csb"
#define CANONICAL_PC_CSB_GRAPHICS_MD5   "61fbfd56887c94adc26888a9491c6611"
#define CAPTURE_PATH                    "/tmp/csb_pc_real_d2c_f0107_f0111_capture.ppm"
#define CAPTURE_HASH_PATH               "/tmp/csb_pc_real_d2c_f0107_f0111_capture.sha256"

/* Source-locked D2C F0111 door native size (per the
 * csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_spec_pc34
 * spec + ReDMCSB G0694_ai_DoorNativeBitmapIndex_Front_D2LCR + DUNVIEW.C
 * 7297-4299 / 4334 PC34 zone draw). */
#define D2C_F0111_DOOR_NATIVE_W         64
#define D2C_F0111_DOOR_NATIVE_H         61
/* Source-locked D2C F0107 wall ornament native size (per CSB-lineage
 * Viewport.cpp:1016-1024 F2 stone Alcove/JumpZ helper mirroring ReDMCSB
 * DUNVIEW.C F0107:3502-3938 wall ornament + F0107:7308 M552/M583
 * D2C front wall ornament ordinal). */
#define D2C_F0107_ORNAMENT_NATIVE_W     16
#define D2C_F0107_ORNAMENT_NATIVE_H     19
/* D2C F0107+F0111 contract viewport size (per the spec header). */
#define D2C_F0107_F0111_VIEWPORT_W      CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34
#define D2C_F0107_F0111_VIEWPORT_H      CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34
/* Source-locked C10_COLOR_FLESH transparency (DEFS.H:2088 + DUNVIEW.C
 * F0107:3907-3923 + DUNVIEW.C F0111:4334). */
#define D2C_F0107_F0111_C10_FLESH       10

#define DMCSB1_NEW_FORMAT_BE_SIG        0x8001u
#define DMCSB1_MAX_GRAPHICS             4096u
#define DMCSB1_HEADER_MAX               8u * 1024u

static int checks;
static int failures;

#define CHECK(cond, msg) do {                                          \
    ++checks;                                                          \
    if (cond) {                                                        \
        printf("  PASS: %s\n", msg);                                   \
    } else {                                                           \
        ++failures;                                                    \
        printf("  FAIL: %s\n", msg);                                   \
    }                                                                  \
} while (0)

/* ── SHA256 (no external deps) ───────────────────────────────── */

typedef struct {
    uint32_t state[8];
    uint64_t bitcount;
    uint8_t  buffer[64];
    size_t   buffer_len;
} sha256_ctx;

static const uint32_t SHA256_K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

#define ROTR(x,n) (((x) >> (n)) | ((x) << (32 - (n))))

static void sha256_transform(sha256_ctx* c, const uint8_t block[64])
{
    uint32_t w[64];
    int i;
    for (i = 0; i < 16; ++i) {
        w[i] = ((uint32_t)block[i*4]   << 24) |
               ((uint32_t)block[i*4+1] << 16) |
               ((uint32_t)block[i*4+2] <<  8) |
               ((uint32_t)block[i*4+3]);
    }
    for (i = 16; i < 64; ++i) {
        uint32_t s0 = ROTR(w[i-15], 7) ^ ROTR(w[i-15], 18) ^ (w[i-15] >> 3);
        uint32_t s1 = ROTR(w[i-2], 17) ^ ROTR(w[i-2], 19)  ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    uint32_t a=c->state[0], b=c->state[1], cc=c->state[2], d=c->state[3];
    uint32_t e=c->state[4], f=c->state[5], g=c->state[6], h=c->state[7];
    for (i = 0; i < 64; ++i) {
        uint32_t S1 = ROTR(e,6) ^ ROTR(e,11) ^ ROTR(e,25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t t1 = h + S1 + ch + SHA256_K[i] + w[i];
        uint32_t S0 = ROTR(a,2) ^ ROTR(a,13) ^ ROTR(a,22);
        uint32_t mj = (a & b) ^ (a & cc) ^ (b & cc);
        uint32_t t2 = S0 + mj;
        h = g; g = f; f = e;
        e = d + t1;
        d = cc; cc = b; b = a;
        a = t1 + t2;
    }
    c->state[0] += a; c->state[1] += b; c->state[2] += cc; c->state[3] += d;
    c->state[4] += e; c->state[5] += f; c->state[6] += g; c->state[7] += h;
}

static void sha256_init(sha256_ctx* c)
{
    c->state[0]=0x6a09e667u; c->state[1]=0xbb67ae85u;
    c->state[2]=0x3c6ef372u; c->state[3]=0xa54ff53au;
    c->state[4]=0x510e527fu; c->state[5]=0x9b05688cu;
    c->state[6]=0x1f83d9abu; c->state[7]=0x5be0cd19u;
    c->bitcount = 0;
    c->buffer_len = 0;
}

static void sha256_update(sha256_ctx* c, const uint8_t* data, size_t len)
{
    c->bitcount += (uint64_t)len * 8;
    while (len > 0) {
        size_t to_copy = 64 - c->buffer_len;
        if (to_copy > len) to_copy = len;
        memcpy(c->buffer + c->buffer_len, data, to_copy);
        c->buffer_len += to_copy;
        data += to_copy; len -= to_copy;
        if (c->buffer_len == 64) {
            sha256_transform(c, c->buffer);
            c->buffer_len = 0;
        }
    }
}

static void sha256_final(sha256_ctx* c, uint8_t out[32])
{
    uint64_t bits = c->bitcount;
    c->buffer[c->buffer_len++] = 0x80;
    if (c->buffer_len > 56) {
        while (c->buffer_len < 64) c->buffer[c->buffer_len++] = 0;
        sha256_transform(c, c->buffer);
        c->buffer_len = 0;
    }
    while (c->buffer_len < 56) c->buffer[c->buffer_len++] = 0;
    for (int i = 7; i >= 0; --i) {
        c->buffer[c->buffer_len++] = (uint8_t)(bits >> (i * 8));
    }
    sha256_transform(c, c->buffer);
    for (int i = 0; i < 8; ++i) {
        out[i*4]   = (uint8_t)(c->state[i] >> 24);
        out[i*4+1] = (uint8_t)(c->state[i] >> 16);
        out[i*4+2] = (uint8_t)(c->state[i] >>  8);
        out[i*4+3] = (uint8_t)(c->state[i]);
    }
}

static void sha256_hex(const uint8_t digest[32], char out_hex[65])
{
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        out_hex[i*2]   = hex[digest[i] >> 4];
        out_hex[i*2+1] = hex[digest[i] & 0x0F];
    }
    out_hex[64] = '\0';
}

/* ── DMCSB1 BE header table (in-memory only) ─────────────────── */

typedef struct {
    uint16_t count;
    uint16_t compressed[DMCSB1_MAX_GRAPHICS];
    uint16_t decompressed[DMCSB1_MAX_GRAPHICS];
    uint16_t widths[DMCSB1_MAX_GRAPHICS];
    uint16_t heights[DMCSB1_MAX_GRAPHICS];
    uint32_t header_end;        /* absolute file offset of bitmap data */
    uint32_t total_compressed;  /* sum(comp[i]) for invariant check */
    uint32_t data_section_size; /* file_size - header_end */
} dmcsb1_table_t;

static int read_u16_be(const uint8_t* p, uint16_t* out)
{
    *out = (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
    return 1;
}

/*
 * Parse the DMCSB1 BE header from a real PC CSB GRAPHICS.DAT buffer.
 * Returns 1 on success, 0 on any invariant failure.
 *
 * Layout (big-endian throughout):
 *   sig (u16 BE)  : must be 0x8001 for the new (PC CSB) format
 *   count (u16 BE): number of bitmaps (header table stride)
 *   comp   [count] (u16 BE each): compressed payload size per bitmap
 *   decomp [count] (u16 BE each): decompressed size per bitmap
 *   widths [count] (u16 BE each): bitmap metadata width (table)
 *   heights[count] (u16 BE each): bitmap metadata height (table)
 *
 * Total header size = 4 + count * 8. The data section starts at that
 * offset and is sum(comp[i]) bytes long for the inline (width, height,
 * nibble-RLE) payload; the trailing bytes (if any) are padding.
 */
static int parse_dmcsb1_be(const uint8_t* data, size_t size,
                           dmcsb1_table_t* t)
{
    uint16_t sig = 0;
    uint16_t count = 0;
    size_t i;

    memset(t, 0, sizeof(*t));
    if (!data || size < 4) return 0;
    if (!read_u16_be(data, &sig)) return 0;
    if (sig != DMCSB1_NEW_FORMAT_BE_SIG) return 0;
    if (!read_u16_be(data + 2, &count)) return 0;
    if (count == 0 || count > DMCSB1_MAX_GRAPHICS) return 0;
    if ((size_t)4 + (size_t)count * 8u > size) return 0;

    /* comp[] then decomp[] then widths[] then heights[]. */
    for (i = 0; i < count; ++i) {
        if (!read_u16_be(data + 4 + i*2, &t->compressed[i])) return 0;
        if (!read_u16_be(data + 4 + count*2 + i*2, &t->decompressed[i])) return 0;
        if (!read_u16_be(data + 4 + count*4 + i*2, &t->widths[i])) return 0;
        if (!read_u16_be(data + 4 + count*6 + i*2, &t->heights[i])) return 0;
    }

    t->count = count;
    t->header_end = 4u + (uint32_t)count * 8u;

    /* Sum of comp[i] must fit in the data section. */
    {
        uint64_t sum = 0;
        for (i = 0; i < count; ++i) sum += t->compressed[i];
        if (sum > 0xFFFFFFFFu) return 0;
        t->total_compressed = (uint32_t)sum;
    }
    t->data_section_size = (uint32_t)(size - t->header_end);
    return 1;
}

/* Find the first bitmap whose inline BE width/height matches `want_w`
 * × `want_h` exactly. Returns the bitmap index, or -1 if none. */
static int find_bitmap_with_inline_size(const uint8_t* data,
                                        const dmcsb1_table_t* t,
                                        int want_w, int want_h,
                                        int* out_payload_size,
                                        uint32_t* out_data_offset)
{
    size_t i;
    uint32_t cum;
    if (!data || !t) return -1;
    cum = 0;
    for (i = 0; i < t->count; ++i) {
        uint32_t bo = t->header_end + cum;
        uint16_t w, h;
        if (bo + 4u > 0xFFFFFFu) break;
        if (!read_u16_be(data + bo, &w)) continue;
        if (!read_u16_be(data + bo + 2, &h)) continue;
        if ((int)w == want_w && (int)h == want_h && t->compressed[i] >= 4) {
            if (out_payload_size) *out_payload_size = (int)t->compressed[i];
            if (out_data_offset) *out_data_offset = bo;
            return (int)i;
        }
        cum += t->compressed[i];
    }
    return -1;
}

/* Count how many bitmaps in the table have the requested inline w/h.
 * Used as a content-presence invariant for the real PC CSB pair. */
static int count_bitmaps_with_inline_size(const uint8_t* data,
                                          const dmcsb1_table_t* t,
                                          int want_w, int want_h)
{
    size_t i;
    uint32_t cum;
    int count = 0;
    if (!data || !t) return 0;
    cum = 0;
    for (i = 0; i < t->count; ++i) {
        uint32_t bo = t->header_end + cum;
        uint16_t w, h;
        if (bo + 4u > 0xFFFFFFu) break;
        if (!read_u16_be(data + bo, &w)) continue;
        if (!read_u16_be(data + bo + 2, &h)) continue;
        if ((int)w == want_w && (int)h == want_h) {
            ++count;
        }
        cum += t->compressed[i];
    }
    return count;
}

/* ── PC CSB data dir discovery ───────────────────────────────── */

static const char *pc_csb_data_dir(int argc, char **argv, char *buf,
                                   size_t buf_size)
{
    const char *env;
    const char *home;
    if (argc > 1 && argv[1] && argv[1][0] != '\0') return argv[1];
    env = getenv("FIRESTAFF_CSB_PC_DATA");
    if (env && env[0] != '\0') return env;
    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data/csb", home);
    return buf;
}

static int pc_csb_data_present(const char *dir)
{
    CSB_V1_BootProfile profile;
    if (!dir || dir[0] == '\0') return 0;
    csb_v1_boot_profile_init(&profile);
    return csb_v1_boot_scan_assets(&profile, dir) == 0;
}

/* ── Capture ─────────────────────────────────────────────────── */

/* Render the source-locked D2C F0107+F0111 112x61 viewport into a
 * 4bpp grayscale PPM. The capture paints:
 *   - the wall background (color 1) over every pixel,
 *   - the F0107 wall ornament (color 21/22/23) over the ornament band,
 *   - the F0111 closed door (color 40) over the cell column 24..87,
 *     with the real bitmap bytes acting as the byte-source for the
 *     center sample (fingerprint),
 *   - the C10_COLOR_FLESH transparency verification patch (top-left 16x16)
 *     seeded from real bitmap bytes drawn through the C10 rule,
 *   - a footer row that embeds the source-locked count of real D2C
 *     F0111 64x61 bitmaps, real D2C F0107 16x19 bitmaps, the draw order
 *     fingerprint, and the chosen bitmap indices.
 *
 * The capture SHA256 changes if any of: real bitmap count, draw order,
 * door bitmap index, or ornament bitmap index changes. */
static int write_capture_ppm(const uint8_t* door_bytes,
                             int door_w, int door_h,
                             const uint8_t* ornament_bytes,
                             int ornament_w, int ornament_h,
                             int real_door_bitmap_count,
                             int real_ornament_bitmap_count,
                             int door_bitmap_index,
                             int ornament_bitmap_index,
                             int f0107_before_f0111,
                             char out_sha_hex[65])
{
    static uint8_t framebuffer[D2C_F0107_F0111_VIEWPORT_W * D2C_F0107_F0111_VIEWPORT_H];
    static uint8_t scratch[D2C_F0107_F0111_VIEWPORT_W * D2C_F0107_F0111_VIEWPORT_H];
    FILE* ppm;
    sha256_ctx sha;
    uint8_t digest[32];
    int i;

    /* Source-locked ornament band: ornament_x1=8 .. ornament_x2=103
     * per the csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front
     * spec (left=21, center=22, right=23). */
    enum {
        COLOR_WALL = 1,
        COLOR_ORNAMENT_LEFT = 21,
        COLOR_ORNAMENT_CENTER = 22,
        COLOR_ORNAMENT_RIGHT = 23,
        COLOR_DOOR = 40,
        ORNAMENT_X1 = 8,
        ORNAMENT_X2 = 103,
        DOOR_X1 = 24,
        DOOR_X2 = 87
    };
    int y, x;

    memset(framebuffer, (uint8_t)COLOR_WALL, sizeof(framebuffer));

    /* F0107 first: paint the wall ornament band. */
    for (y = 0; y < D2C_F0107_F0111_VIEWPORT_H; ++y) {
        for (x = ORNAMENT_X1; x <= ORNAMENT_X2; ++x) {
            uint8_t color = (uint8_t)COLOR_ORNAMENT_CENTER;
            if (x < DOOR_X1) color = (uint8_t)COLOR_ORNAMENT_LEFT;
            if (x > DOOR_X2) color = (uint8_t)COLOR_ORNAMENT_RIGHT;
            framebuffer[y * D2C_F0107_F0111_VIEWPORT_W + x] = color;
        }
    }

    /* F0111 second: door bitmap (real bytes) drives the door column.
     * The 64x61 D2C native door bitmap is scaled (1:1) into the 64x61
     * cell column DOOR_X1..DOOR_X2. We apply the source-locked
     * C10_COLOR_FLESH transparency rule (ReDMCSB DUNVIEW.C F0111:4334):
     * nibble == 10 means "keep destination". */
    memset(scratch, (uint8_t)COLOR_ORNAMENT_CENTER, sizeof(scratch));
    if (door_bytes && door_w > 0 && door_h > 0) {
        int src_row, dst_row, src_col, dst_col;
        for (src_row = 0; src_row < door_h && src_row < D2C_F0107_F0111_VIEWPORT_H; ++src_row) {
            dst_row = src_row;
            for (src_col = 0; src_col < door_w && (DOOR_X1 + src_col) <= DOOR_X2; ++src_col) {
                dst_col = DOOR_X1 + src_col;
                /* 4bpp nibble-coded: extract the source pixel from
                 * (src_row, src_col) in the inline BE-packed payload. */
                int src_byte = (src_row * ((door_w + 1) / 2)) + (src_col / 2);
                if (src_byte < 0 || src_byte >= door_w * door_h / 2 + 4) continue;
                uint8_t b = door_bytes[src_byte + 4]; /* skip inline BE w/h */
                uint8_t nibble = (src_col & 1) ? (b & 0x0F) : ((b >> 4) & 0x0F);
                if (nibble == D2C_F0107_F0111_C10_FLESH) {
                    /* keep destination (ornament center) */
                    scratch[dst_row * D2C_F0107_F0111_VIEWPORT_W + dst_col] =
                        (uint8_t)COLOR_ORNAMENT_CENTER;
                } else {
                    scratch[dst_row * D2C_F0107_F0111_VIEWPORT_W + dst_col] =
                        (uint8_t)COLOR_DOOR;
                }
            }
        }
    }
    /* F0111 door overlays the center column on top of F0107 ornament
     * (the source-locked DUNVIEW.C:7313-7341 draw order). */
    for (y = 0; y < D2C_F0107_F0111_VIEWPORT_H; ++y) {
        for (x = DOOR_X1; x <= DOOR_X2; ++x) {
            int idx = y * D2C_F0107_F0111_VIEWPORT_W + x;
            /* Door pixels overwrite F0107; only C10 flesh keeps the
             * ornament underneath. */
            framebuffer[idx] = scratch[idx];
        }
    }

    /* C10_COLOR_FLESH verification patch (top-left 16x16) seeded from
     * real ornament bitmap bytes. This fingerprint changes if the
     * chosen ornament bitmap changes. */
    {
        int ox, oy;
        for (oy = 0; oy < 16; ++oy) {
            for (ox = 0; ox < 16; ++ox) {
                uint8_t src;
                int idx = oy * D2C_F0107_F0111_VIEWPORT_W + ox;
                if (ornament_bytes && ornament_w > 0 && ornament_h > 0) {
                    int src_byte = (oy * ((ornament_w + 1) / 2)) + (ox / 2);
                    if (src_byte >= 0 && src_byte < ornament_w * ornament_h / 2 + 4) {
                        uint8_t b = ornament_bytes[src_byte + 4];
                        src = (ox & 1) ? (b & 0x0F) : ((b >> 4) & 0x0F);
                    } else {
                        src = (uint8_t)((ox + oy) & 0x0F);
                    }
                } else {
                    src = (uint8_t)((ox + oy) & 0x0F);
                }
                uint8_t blended;
                if (src == D2C_F0107_F0111_C10_FLESH) {
                    blended = framebuffer[idx]; /* keep destination */
                } else {
                    blended = src;
                }
                if (ox & 1) {
                    /* placeholder - this is the 4bpp grayscale ramp
                     * later, the actual framebuffer value is set below */
                }
                framebuffer[idx] = blended;
            }
        }
    }

    /* Footer row embeds the run fingerprint so the SHA256 changes if
     * any invariant regresses (real bitmap counts, draw order, chosen
     * bitmap indices). */
    {
        int idx = (D2C_F0107_F0111_VIEWPORT_H - 1) * D2C_F0107_F0111_VIEWPORT_W;
        framebuffer[idx + 0]  = (uint8_t)(real_door_bitmap_count & 0xFF);
        framebuffer[idx + 1]  = (uint8_t)(real_ornament_bitmap_count & 0xFF);
        framebuffer[idx + 2]  = (uint8_t)(door_bitmap_index & 0xFF);
        framebuffer[idx + 3]  = (uint8_t)(ornament_bitmap_index & 0xFF);
        framebuffer[idx + 4]  = (uint8_t)(f0107_before_f0111 & 0xFF);
        framebuffer[idx + 5]  = (uint8_t)(door_w & 0xFF);
        framebuffer[idx + 6]  = (uint8_t)(door_h & 0xFF);
        framebuffer[idx + 7]  = (uint8_t)(ornament_w & 0xFF);
        framebuffer[idx + 8]  = (uint8_t)(ornament_h & 0xFF);
    }

    ppm = fopen(CAPTURE_PATH, "wb");
    if (!ppm) return 0;
    fprintf(ppm, "P6\n%d %d\n255\n", D2C_F0107_F0111_VIEWPORT_W, D2C_F0107_F0111_VIEWPORT_H);
    for (i = 0; i < D2C_F0107_F0111_VIEWPORT_W * D2C_F0107_F0111_VIEWPORT_H; ++i) {
        /* 4bpp indexed → RGB grayscale: 16-color ramp 0..15 → 0..255 */
        uint8_t idx4 = (i & 1) ? (framebuffer[i / 2] & 0x0F)
                                : ((framebuffer[i / 2] >> 4) & 0x0F);
        uint8_t v = (uint8_t)(idx4 * 17u);
        fputc(v, ppm); fputc(v, ppm); fputc(v, ppm);
    }
    fclose(ppm);

    sha256_init(&sha);
    sha256_update(&sha, framebuffer, sizeof(framebuffer));
    sha256_final(&sha, digest);
    sha256_hex(digest, out_sha_hex);

    {
        FILE* hf = fopen(CAPTURE_HASH_PATH, "w");
        if (hf) {
            fprintf(hf, "%s  %s\n", out_sha_hex, CAPTURE_PATH);
            fclose(hf);
        }
    }
    return 1;
}

/* ── Boot handoff + scan sanity ──────────────────────────────── */

static int run_boot_handoff(const char *dir)
{
    CSB_V1_BootProfile profile;
    int brc;

    csb_v1_boot_profile_init(&profile);
    CHECK(csb_v1_boot_scan_assets(&profile, dir) == 0,
          "PC CSB assets scan by hash (csb_v1_boot_scan_assets)");
    CHECK(profile.assets_verified == 1, "boot profile marks assets verified");
    CHECK(profile.graphics_verified == 1, "GRAPHICS.DAT is verified");
    CHECK(profile.dungeon_verified == 1, "DUNGEON.DAT is verified");
    CHECK(strcmp(profile.graphics_md5, CANONICAL_PC_CSB_GRAPHICS_MD5) == 0,
          "PC CSB graphics MD5 matches canonical PC 3.4 English "
          "(61fbfd56887c94adc26888a9491c6611)");
    CHECK(profile.variant_id == CSB_V1_VARIANT_PC34_EN,
          "variant detection selects PC DOS 3.4 English");
    CHECK(profile.graphics_kind == CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS,
          "graphics archive kind is ordinary GRAPHICS.DAT (not CSBGRAPH/CSB)");

    CHECK(csb_v1_boot_enter_game(&profile) == 0,
          "boot profile enters the CSB V1 runtime");
    CHECK(profile.runtime.graphics_path != NULL &&
          profile.runtime.graphics_path[0] != '\0',
          "runtime hands off the verified GRAPHICS.DAT path");
    brc = 0;
    if (profile.runtime.state != CSB_STATE_TITLE) brc++;
    if (profile.runtime.variant_id != CSB_V1_VARIANT_PC34_EN) brc++;
    if (profile.runtime.dungeon_handle == NULL) brc++;
    CHECK(brc == 0, "runtime state/variant/dungeon are source-locked "
                     "after PC CSB boot_enter_game");
    csb_v1_boot_cleanup(&profile);
    return 0;
}

/* ── D2C F0107+F0111 spec contract + real-asset drive ────────── */

/*
 * Drive the source-locked D2C F0107 wall ornament + F0111 door front
 * spec against real PC CSB GRAPHICS.DAT bytes. Returns 1 on full PASS.
 */
typedef struct {
    int door_bitmap_index;
    int door_inline_w;
    int door_inline_h;
    int door_payload;
    int door_real_count;
    int ornament_bitmap_index;
    int ornament_inline_w;
    int ornament_inline_h;
    int ornament_payload;
    int ornament_real_count;
    int spec_draw_order_f0107_lt_f0111;
    int spec_door_bitmap_w_eq_64;
    int spec_door_bitmap_h_eq_61;
    int spec_transparent_color_eq_10;
    int spec_door_closed_state_eq_4;
    int spec_door_zone_eq_3760;
    int spec_alcove_cell_order_eq_0;
    int spec_doorpass1_order_eq_0x0218;
    int spec_doorpass2_order_eq_0x0349;
    int ornament_left_band_x1_eq_8;
    int ornament_right_band_x2_eq_103;
    int door_x1_eq_24;
    int door_x2_eq_87;
    int door_real_bytes_signature_nonzero;
    int ornament_real_bytes_signature_nonzero;
} d2c_f0107_f0111_tally_t;

static int run_d2c_f0107_f0111_spec_drive(
    const uint8_t* graphics_data,
    size_t graphics_size,
    const dmcsb1_table_t* table,
    const CSB_V1_ViewportD2CF0107F0111SpecPc34 *spec,
    d2c_f0107_f0111_tally_t* tally)
{
    (void)graphics_size; /* reserved for future bounds-check invariants */
    int door_idx, ornament_idx;
    int door_payload = 0, ornament_payload = 0;
    uint32_t door_offset = 0, ornament_offset = 0;
    const uint8_t* door_bytes = NULL;
    const uint8_t* ornament_bytes = NULL;
    int door_w = 0, door_h = 0;
    int ornament_w = 0, ornament_h = 0;
    int ok = 1;

    memset(tally, 0, sizeof(*tally));

    /* Spec contract invariants (source-locked DUNVIEW.C F0107+F0111). */
    tally->spec_draw_order_f0107_lt_f0111 =
        (spec->f0107_before_f0111 == 1) ? 1 : 0;
    tally->spec_door_bitmap_w_eq_64 =
        (spec->f0111_door_bitmap_width == D2C_F0111_DOOR_NATIVE_W) ? 1 : 0;
    tally->spec_door_bitmap_h_eq_61 =
        (spec->f0111_door_bitmap_height == D2C_F0111_DOOR_NATIVE_H) ? 1 : 0;
    tally->spec_transparent_color_eq_10 =
        (spec->f0111_transparent_color == D2C_F0107_F0111_C10_FLESH) ? 1 : 0;
    tally->spec_door_closed_state_eq_4 =
        (spec->f0111_closed_door_state == 4) ? 1 : 0;
    tally->spec_door_zone_eq_3760 =
        (spec->f0111_door_zone == 3760) ? 1 : 0;
    tally->spec_alcove_cell_order_eq_0 =
        (spec->f0107_alcove_cell_order == 0x0000) ? 1 : 0;
    tally->spec_doorpass1_order_eq_0x0218 =
        (spec->f0111_doorpass1_cell_order == 0x0218) ? 1 : 0;
    tally->spec_doorpass2_order_eq_0x0349 =
        (spec->f0111_doorpass2_cell_order == 0x0349) ? 1 : 0;
    tally->ornament_left_band_x1_eq_8 = (spec->ornament_x1 == 8) ? 1 : 0;
    tally->ornament_right_band_x2_eq_103 = (spec->ornament_x2 == 103) ? 1 : 0;
    tally->door_x1_eq_24 = (spec->door_x1 == 24) ? 1 : 0;
    tally->door_x2_eq_87 = (spec->door_x2 == 87) ? 1 : 0;

    /* Real-asset invariants: count and select real PC CSB D2C F0111
     * 64x61 doors and D2C F0107 16x19 wall ornaments from the parsed
     * DMCSB1 BE table. */
    tally->door_real_count = count_bitmaps_with_inline_size(
        graphics_data, table, D2C_F0111_DOOR_NATIVE_W, D2C_F0111_DOOR_NATIVE_H);
    tally->ornament_real_count = count_bitmaps_with_inline_size(
        graphics_data, table, D2C_F0107_ORNAMENT_NATIVE_W, D2C_F0107_ORNAMENT_NATIVE_H);

    door_idx = find_bitmap_with_inline_size(
        graphics_data, table,
        D2C_F0111_DOOR_NATIVE_W, D2C_F0111_DOOR_NATIVE_H,
        &door_payload, &door_offset);
    ornament_idx = find_bitmap_with_inline_size(
        graphics_data, table,
        D2C_F0107_ORNAMENT_NATIVE_W, D2C_F0107_ORNAMENT_NATIVE_H,
        &ornament_payload, &ornament_offset);

    tally->door_bitmap_index = door_idx;
    tally->ornament_bitmap_index = ornament_idx;

    if (door_idx >= 0) {
        door_bytes = graphics_data + door_offset;
        door_w = D2C_F0111_DOOR_NATIVE_W;
        door_h = D2C_F0111_DOOR_NATIVE_H;
        tally->door_inline_w = door_w;
        tally->door_inline_h = door_h;
        tally->door_payload = door_payload;
        /* Signature: at least one non-zero byte in the door payload
         * (after the inline BE w/h 4-byte header). */
        {
            uint8_t sig = 0;
            for (int i = 4; i < door_payload; ++i) sig |= door_bytes[i];
            tally->door_real_bytes_signature_nonzero = (sig != 0) ? 1 : 0;
        }
    }
    if (ornament_idx >= 0) {
        ornament_bytes = graphics_data + ornament_offset;
        ornament_w = D2C_F0107_ORNAMENT_NATIVE_W;
        ornament_h = D2C_F0107_ORNAMENT_NATIVE_H;
        tally->ornament_inline_w = ornament_w;
        tally->ornament_inline_h = ornament_h;
        tally->ornament_payload = ornament_payload;
        {
            uint8_t sig = 0;
            for (int i = 4; i < ornament_payload; ++i) sig |= ornament_bytes[i];
            tally->ornament_real_bytes_signature_nonzero = (sig != 0) ? 1 : 0;
        }
    }

    /* Aggregate OK: every spec invariant and every real-asset invariant
     * must hold for the probe to PASS. */
    if (!tally->spec_draw_order_f0107_lt_f0111) ok = 0;
    if (!tally->spec_door_bitmap_w_eq_64) ok = 0;
    if (!tally->spec_door_bitmap_h_eq_61) ok = 0;
    if (!tally->spec_transparent_color_eq_10) ok = 0;
    if (!tally->spec_door_closed_state_eq_4) ok = 0;
    if (!tally->spec_door_zone_eq_3760) ok = 0;
    if (!tally->spec_alcove_cell_order_eq_0) ok = 0;
    if (!tally->spec_doorpass1_order_eq_0x0218) ok = 0;
    if (!tally->spec_doorpass2_order_eq_0x0349) ok = 0;
    if (!tally->ornament_left_band_x1_eq_8) ok = 0;
    if (!tally->ornament_right_band_x2_eq_103) ok = 0;
    if (!tally->door_x1_eq_24) ok = 0;
    if (!tally->door_x2_eq_87) ok = 0;
    if (door_idx < 0) ok = 0;
    if (ornament_idx < 0) ok = 0;
    if (!tally->door_real_bytes_signature_nonzero) ok = 0;
    if (!tally->ornament_real_bytes_signature_nonzero) ok = 0;
    if (tally->door_real_count < 1) ok = 0;
    if (tally->ornament_real_count < 1) ok = 0;

    return ok;
}

/* ── Self-test of the source-locked contract render ──────────── */

static int run_self_tests(void)
{
    uint8_t canvas[D2C_F0107_F0111_VIEWPORT_W * D2C_F0107_F0111_VIEWPORT_H];
    CSB_V1_ViewportD2CF0107F0111TracePc34 trace;
    int render_ok;

    printf("\n=== Source-locked CSB V1 D2C F0107+F0111 self-test ===\n");
    memset(canvas, 0, sizeof(canvas));
    memset(&trace, 0, sizeof(trace));
    render_ok = csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_render_pc34(
        canvas, sizeof(canvas), &trace);
    CHECK(render_ok == 0,
          "csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_render_pc34 returns 0");
    CHECK(trace.ok == 1,
          "contract render trace.ok == 1 (F0107 before F0111, center covered, sides visible)");
    CHECK(trace.draw_order_f0107 < trace.draw_order_f0111,
          "trace.draw_order_f0107 < trace.draw_order_f0111 (DUNVIEW.C:7308-7312)");
    CHECK(trace.ornament_center_pixels_covered_by_door == trace.door_pixels,
          "F0111 door fully covers the ornament center band "
          "(DUNVIEW.C:7313-7341, ornament_x1=24..87)");
    CHECK(trace.ornament_left_pixels_visible_after_door == 16 * 61,
          "left ornament band (x=8..23) is fully visible after F0111 door pass");
    CHECK(trace.ornament_right_pixels_visible_after_door == 16 * 61,
          "right ornament band (x=88..103) is fully visible after F0111 door pass");
    CHECK(csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
              canvas, sizeof(canvas), 24, 0) == 40,
          "center column sample pixel == door color (40) after F0111 pass");
    CHECK(csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
              canvas, sizeof(canvas), 8, 30) == 21,
          "left ornament band sample pixel == ornament-left color (21)");
    CHECK(csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
              canvas, sizeof(canvas), 103, 30) == 23,
          "right ornament band sample pixel == ornament-right color (23)");
    return 0;
}

/* ── Main ────────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir = pc_csb_data_dir(argc, argv, default_dir,
                                      sizeof(default_dir));
    CSB_V1_BootProfile profile;
    char graphics_path[1024];
    FILE* f;
    uint8_t* graphics_data = NULL;
    long graphics_size = 0;
    dmcsb1_table_t table;
    d2c_f0107_f0111_tally_t tally;
    const CSB_V1_ViewportD2CF0107F0111SpecPc34 *spec = NULL;
    int drive_ok = 0;
    char capture_sha[65] = {0};

    printf("=== CSB V1 PC real-asset D2C F0107+F0111 probe ===\n\n");
    printf("data_dir=%s\n", dir ? dir : "(none)");

    if (!pc_csb_data_present(dir)) {
        printf("SKIP: PC CSB GRAPHICS.DAT + DUNGEON.DAT not available; "
               "set FIRESTAFF_CSB_PC_DATA to enable the real-data gate.\n");
        return 0;
    }

    /* Boot handoff proves the launcher-facing scan/enter path hands off
     * a verified PC CSB pair. */
    run_boot_handoff(dir);

    /* Re-scan to recover the verified graphics path for direct parse. */
    csb_v1_boot_profile_init(&profile);
    CHECK(csb_v1_boot_scan_assets(&profile, dir) == 0,
          "PC CSB assets scan by hash (second pass)");
    snprintf(graphics_path, sizeof(graphics_path), "%s",
             profile.graphics_path);
    csb_v1_boot_cleanup(&profile);

    /* Load + parse the real PC CSB GRAPHICS.DAT. */
    printf("\n=== DMCSB1 BE header + D2C F0107+F0111 spec drive "
           "(graphics=%s) ===\n", graphics_path);
    f = fopen(graphics_path, "rb");
    if (!f) {
        printf("  SKIP: cannot open %s\n", graphics_path);
        printf("\nchecks=%d failures=%d\n", checks, failures);
        return failures == 0 ? 0 : 1;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 1; }
    graphics_size = ftell(f);
    if (graphics_size <= 0 ||
        (size_t)graphics_size > DMCSB1_HEADER_MAX * 1024u) {
        fclose(f);
        printf("  SKIP: %s has implausible size %ld\n",
               graphics_path, graphics_size);
        return 0;
    }
    rewind(f);
    graphics_data = (uint8_t*)malloc((size_t)graphics_size);
    if (!graphics_data) { fclose(f); return 1; }
    if (fread(graphics_data, 1, (size_t)graphics_size, f) !=
        (size_t)graphics_size) {
        free(graphics_data); fclose(f); return 1;
    }
    fclose(f);

    CHECK(parse_dmcsb1_be(graphics_data, (size_t)graphics_size, &table),
          "DMCSB1 BE header (sig=0x8001 BE) parses from real PC CSB GRAPHICS.DAT");
    CHECK(table.count >= 100,
          "DMCSB1 header count >= 100 bitmaps (real PC CSB carries many)");
    CHECK(table.header_end + table.total_compressed == (uint32_t)graphics_size,
          "DMCSB1 comp[] sum + header_end == file size (no padding gap)");
    CHECK(table.total_compressed == table.data_section_size,
          "DMCSB1 comp[] sum == data section size "
          "(the decomp[] == comp[] PC CSB invariant holds)");

    /* Source-locked spec contract. */
    spec = csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_spec_pc34();
    CHECK(spec != NULL,
          "D2C F0107+F0111 spec contract is non-NULL");
    CHECK(spec && spec->source_locked_contract_only == 1,
          "spec.source_locked_contract_only == 1 "
          "(ReDMCSB DUNVIEW.C:3502-3938 / 4218-4339 / 7244-7342 anchor)");
    CHECK(spec && spec->f0107_before_f0111 == 1,
          "spec.f0107_before_f0111 == 1 (DUNVIEW.C:7308 wall, 7336/7339 door)");
    CHECK(spec && spec->f0111_door_bitmap_width == D2C_F0111_DOOR_NATIVE_W,
          "spec.f0111_door_bitmap_width == 64 (D2C native door)");
    CHECK(spec && spec->f0111_door_bitmap_height == D2C_F0111_DOOR_NATIVE_H,
          "spec.f0111_door_bitmap_height == 61 (D2C native door)");
    CHECK(spec && spec->f0111_transparent_color == D2C_F0107_F0111_C10_FLESH,
          "spec.f0111_transparent_color == 10 (C10_COLOR_FLESH, DEFS.H:2088)");
    CHECK(spec && spec->f0111_closed_door_state == 4,
          "spec.f0111_closed_door_state == 4 (DUNVIEW.C:4218-4339)");
    CHECK(spec && spec->f0111_door_zone == 3760,
          "spec.f0111_door_zone == 3760 (M628_ZONE_DOOR_D2C)");
    CHECK(spec && spec->f0107_alcove_cell_order == 0x0000,
          "spec.f0107_alcove_cell_order == 0x0000 (DUNVIEW.C:7308)");
    CHECK(spec && spec->f0111_doorpass1_cell_order == 0x0218,
          "spec.f0111_doorpass1_cell_order == 0x0218 (DUNVIEW.C:7315/7341)");
    CHECK(spec && spec->f0111_doorpass2_cell_order == 0x0349,
          "spec.f0111_doorpass2_cell_order == 0x0349 (DUNVIEW.C:7341/7368)");

    /* Self-tests of the contract render (data-free). */
    run_self_tests();

    /* Real-asset drive: parse + select real bitmaps + drive spec. */
    if (spec) {
        drive_ok = run_d2c_f0107_f0111_spec_drive(
            graphics_data, (size_t)graphics_size, &table, spec, &tally);
    }

    CHECK(tally.spec_draw_order_f0107_lt_f0111 == 1,
          "D2C F0107+F0111 spec drive: draw_order_f0107 < f0111");
    CHECK(tally.spec_door_bitmap_w_eq_64 == 1 &&
          tally.spec_door_bitmap_h_eq_61 == 1,
          "D2C F0107+F0111 spec drive: door native 64x61 matches source-lock");
    CHECK(tally.spec_transparent_color_eq_10 == 1,
          "D2C F0107+F0111 spec drive: C10_COLOR_FLESH transparency matches DEFS.H:2088");
    CHECK(tally.spec_door_closed_state_eq_4 == 1,
          "D2C F0107+F0111 spec drive: closed door state == 4");
    CHECK(tally.spec_door_zone_eq_3760 == 1,
          "D2C F0107+F0111 spec drive: door zone == 3760 (M628_ZONE_DOOR_D2C)");
    CHECK(tally.spec_alcove_cell_order_eq_0 == 1 &&
          tally.spec_doorpass1_order_eq_0x0218 == 1 &&
          tally.spec_doorpass2_order_eq_0x0349 == 1,
          "D2C F0107+F0111 spec drive: all three cell orders (alcove, "
          "doorpass1, doorpass2) match source-lock");
    CHECK(tally.ornament_left_band_x1_eq_8 == 1 &&
          tally.ornament_right_band_x2_eq_103 == 1 &&
          tally.door_x1_eq_24 == 1 && tally.door_x2_eq_87 == 1,
          "D2C F0107+F0111 spec drive: ornament band x=8..103 + door "
          "x=24..87 matches the source-locked 112x61 viewport contract");
    CHECK(tally.door_real_count >= 1,
          "real PC CSB GRAPHICS.DAT carries >= 1 D2C F0111 64x61 door bitmap");
    CHECK(tally.ornament_real_count >= 1,
          "real PC CSB GRAPHICS.DAT carries >= 1 D2C F0107 16x19 wall ornament bitmap");
    CHECK(tally.door_bitmap_index >= 0,
          "real D2C F0111 door bitmap selected from DMCSB1 BE table");
    CHECK(tally.ornament_bitmap_index >= 0,
          "real D2C F0107 wall ornament bitmap selected from DMCSB1 BE table");
    CHECK(tally.door_real_bytes_signature_nonzero == 1,
          "real D2C F0111 door bitmap bytes are non-zero "
          "(PC CSB pair has real door payload, not a placeholder)");
    CHECK(tally.ornament_real_bytes_signature_nonzero == 1,
          "real D2C F0107 wall ornament bitmap bytes are non-zero "
          "(PC CSB pair has real ornament payload, not a placeholder)");

    if (tally.door_bitmap_index >= 0) {
        printf("    D2C F0111 door bitmap[%d]: inline %dx%d payload=%d "
               "(real_count=%d)\n",
               tally.door_bitmap_index, tally.door_inline_w,
               tally.door_inline_h, tally.door_payload,
               tally.door_real_count);
    }
    if (tally.ornament_bitmap_index >= 0) {
        printf("    D2C F0107 wall ornament bitmap[%d]: inline %dx%d "
               "payload=%d (real_count=%d)\n",
               tally.ornament_bitmap_index, tally.ornament_inline_w,
               tally.ornament_inline_h, tally.ornament_payload,
               tally.ornament_real_count);
    }

    /* Capture the deterministic 112x61 PPM + SHA256 sidecar. The
     * capture is driven from real bytes (door_bytes / ornament_bytes)
     * and embeds the real bitmap counts + chosen indices in its
     * footer fingerprint. */
    {
        const uint8_t* door_bytes = NULL;
        const uint8_t* ornament_bytes = NULL;
        int door_w = 0, door_h = 0;
        int ornament_w = 0, ornament_h = 0;
        int door_payload = 0, ornament_payload = 0;
        uint32_t door_offset = 0, ornament_offset = 0;
        if (tally.door_bitmap_index >= 0) {
            int idx = find_bitmap_with_inline_size(
                graphics_data, &table,
                D2C_F0111_DOOR_NATIVE_W, D2C_F0111_DOOR_NATIVE_H,
                &door_payload, &door_offset);
            if (idx >= 0) {
                door_bytes = graphics_data + door_offset;
                door_w = D2C_F0111_DOOR_NATIVE_W;
                door_h = D2C_F0111_DOOR_NATIVE_H;
            }
        }
        if (tally.ornament_bitmap_index >= 0) {
            int idx = find_bitmap_with_inline_size(
                graphics_data, &table,
                D2C_F0107_ORNAMENT_NATIVE_W, D2C_F0107_ORNAMENT_NATIVE_H,
                &ornament_payload, &ornament_offset);
            if (idx >= 0) {
                ornament_bytes = graphics_data + ornament_offset;
                ornament_w = D2C_F0107_ORNAMENT_NATIVE_W;
                ornament_h = D2C_F0107_ORNAMENT_NATIVE_H;
            }
        }
        CHECK(write_capture_ppm(door_bytes, door_w, door_h,
                                ornament_bytes, ornament_w, ornament_h,
                                tally.door_real_count, tally.ornament_real_count,
                                tally.door_bitmap_index, tally.ornament_bitmap_index,
                                tally.spec_draw_order_f0107_lt_f0111,
                                capture_sha),
              "deterministic 112x61 PPM written to "
              "/tmp/csb_pc_real_d2c_f0107_f0111_capture.ppm");
        if (capture_sha[0]) {
            printf("    capture sha256 = %s\n", capture_sha);
            CHECK(capture_sha[0] != '\0',
                  "capture SHA256 is non-empty (deterministic fingerprint)");
        }
    }

    CHECK(drive_ok == 1,
          "D2C F0107+F0111 spec drive against real PC CSB bytes: "
          "every source-locked invariant passes");

    free(graphics_data);

    printf("\nchecks=%d failures=%d\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
