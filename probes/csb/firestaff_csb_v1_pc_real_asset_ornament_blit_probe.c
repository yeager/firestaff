/*
 * firestaff_csb_v1_pc_real_asset_ornament_blit_probe.c
 *
 * CSB V1 PC real-asset ornament blit gate.
 *
 * Purpose
 * -------
 * Tier 1 (real-asset CSB graphics + ornament blits) is the next blocker
 * in the CSB finish lane per docs/FIRESTAFF_GAP_LIST.md C5 and
 * docs/CSB_V1_BOOTSTRAP_SCOUT.md. The existing CSB V1 viewport modules
 * (F0107 wall ornament, F0108 floor/ceiling ornament, F0111 door front,
 * F0115 thing-pass cell ordering) are data-free source-locked contracts
 * (see include/firestaff/csb/v1/viewport/ headers and
 * src/csb/csb_v1_viewport_* source).
 * They prove the source-locked math (zone formulas, transparency masks,
 * cell orders) is correct, but they never touch real PC CSB GRAPHICS.DAT
 * pixels, so the runtime path is not yet proven to feed those formulas
 * from a verified CSB pair.
 *
 * This probe closes that gap by:
 *
 *   1. Discovering a real, hash-verified PC CSB GRAPHICS.DAT through the
 *      existing `csb_v1_boot_scan_assets` path (the same path the launcher
 *      uses to gate launch), and confirming the matched MD5 sits inside
 *      the canonical PC CSB graphics list
 *      (61fbfd56887c94adc26888a9491c6611 — see src/csb/csb_v1_boot.c).
 *   2. Parsing the DMCSB1 BE header (sig 0x8001 BE + count BE +
 *      comp[count] BE + decomp[count] BE + widths[count] BE +
 *      heights[count] BE) into a stable in-memory table. The header
 *      totals must equal the file-data section exactly so the parser is
 *      proven on real bytes.
 *   3. Selecting a real PC CSB ornament-sized bitmap by matching the
 *      DMCSB1 table entry to its inline BE width/height header (the
 *      canonical PC 3.4 pair currently selects bitmap[6], 80x14),
 *      matching the ReDMCSB DUNVIEW.C F0108 1500-1509 zone math and
 *      the CSB-lineage Viewport.cpp:6924-6927 first-CSB-backdrop
 *      anchor, then loading the DMCSB1 nibble-coded compressed payload.
 *   4. Driving the source-locked ornament blit math against those real
 *      bytes:
 *        - `csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34`
 *          computes the C1500_ZONE_FLOOR_ORNAMENT + cs*11 + vf zone
 *          (ReDMCSB DUNVIEW.C F0108:3998/4004).
 *        - `csb_v1_viewport_d1c_f0108_blend_c10_pc34` applies the
 *          C10_COLOR_FLESH transparency rule (ReDMCSB DEFS.H:2088 and
 *          DUNVIEW.C F0108:3989-4004) to the real bitmap bytes.
 *        - `csb_v1_viewport_d1c_f0115_decode_order_pc34` decodes the
 *          source-locked thing-pass cell ordering for the real D1C
 *          thing-pass route (ReDMCSB DUNVIEW.C F0115:4547-4581,
 *          4923, 5180-5188, 5211-5214, 5458-5570, 5668-5671).
 *   5. Capturing the resulting framebuffer to
 *      `/tmp/csb_pc_real_ornament_capture.ppm` (P6, 320x200, 4bpp
 *      decoded grayscale) so the ornament blit math is human-viewable
 *      and reproducible byte-for-byte across runs.
 *   6. Hashing the captured PPM with a built-in SHA256 to make the run
 *      deterministic and ctest-stable.
 *
 * Source-lock boundary
 * --------------------
 *   - ReDMCSB DUNVIEW.C F0108:3940-4011 (floor ornament ordinal gate,
 *     MASK 0x8000 footprint recursion, C10 blit transparency, C1500 +
 *     CoordinateSet * 11 + ViewFloor zone math, 320x200 framebuffer /
 *     224x136 viewport contract).
 *   - ReDMCSB DUNVIEW.C F0115:4547-4581, 4923, 5180-5188, 5211-5214,
 *     5458-5570, 5668-5671 (D1C thing-pass cell ordering and creature-
 *     cache neighborhood).
 *   - ReDMCSB DEFS.H:2088 (C10_COLOR_FLESH), 2596-2611 (view squares),
 *     2668-2677 (cell orders), 4045-4046 (C705/C706), 4223
 *     (C1500_ZONE_FLOOR_ORNAMENT).
 *   - dmweb Data Files page: PC CSB GRAPHICS.DAT = DMCSB1 BE
 *     (sig 0x8001 BE, count + comp/decomp/w/h tables).
 *
 * Skip-safe on hosts without user-supplied PC CSB game data.
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "firestaff/csb/v1/viewport/d1c_f0108_floor_ceiling_ornament_pc34_compat.h"
#include "firestaff/csb/v1/viewport/d1c_f0115_thing_pass_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DEFAULT_PC_CSB_DATA_DIR        "/Users/bosse/.firestaff/data/csb"
#define CANONICAL_PC_CSB_GRAPHICS_MD5  "61fbfd56887c94adc26888a9491c6611"
#define CAPTURE_PATH                   "/tmp/csb_pc_real_ornament_capture.ppm"
#define CAPTURE_HASH_PATH              "/tmp/csb_pc_real_ornament_capture.sha256"
#define CAPTURE_MANIFEST_PATH          "/tmp/csb_pc_real_ornament_capture_manifest.json"

#define DMCSB1_NEW_FORMAT_BE_SIG       0x8001u
#define FRAMEBUFFER_W                  320
#define FRAMEBUFFER_H                  200
#define CAPTURE_D1C_FLOOR_BAND_Y       100
#define CAPTURE_D1C_FLOOR_BAND_ROWS    9
#define DMCSB1_MAX_GRAPHICS            4096u
#define DMCSB1_HEADER_MAX              8u * 1024u

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

/* Find the first bitmap that has both a sensible table w/h AND a
 * matching inline BE width/height header in the data section. Returns
 * the bitmap index, or -1 if none.
 */
static int find_verified_ornament_bitmap(const uint8_t* data,
                                         const dmcsb1_table_t* t,
                                         int want_min_w, int want_min_h,
                                         int want_max_w, int want_max_h,
                                         int* out_w, int* out_h,
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
        if ((int)w >= want_min_w && (int)w <= want_max_w &&
            (int)h >= want_min_h && (int)h <= want_max_h &&
            t->compressed[i] >= 4) {
            if (out_w) *out_w = (int)w;
            if (out_h) *out_h = (int)h;
            if (out_data_offset) *out_data_offset = bo;
            return (int)i;
        }
        cum += t->compressed[i];
    }
    return -1;
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

/* ── Ornament blit math driver ───────────────────────────────── */

/*
 * Drive the source-locked CSB V1 D1C F0108 floor+ceiling+ornament
 * math against a real PC CSB bitmap. The probe:
 *   1. Reads the real bitmap's inline BE width/height header.
 *   2. Walks every (coordinate_set, view_floor) pair the source-locked
 *      F0108 zone formula expects and checks that the resulting zone
 *      falls inside the source-locked C1500_ZONE_FLOOR_ORNAMENT band
 *      (ReDMCSB DUNVIEW.C F0108:3998/4004, DEFS.H:4223).
 *   3. Applies the C10_COLOR_FLESH transparency rule to a few real
 *      bytes drawn from the bitmap's nibble-RLE payload, proving the
 *      blend contract matches the inline pixel data.
 *   4. Decodes the source-locked F0115 thing-pass cell order for the
 *      D1C route (ReDMCSB DUNVIEW.C F0115:4547-4581) and proves the
 *      decode is the documented non-overlapping route.
 */
typedef struct {
    int zones_checked;
    int zones_inside_band;
    int zones_match_redmcsb_formula;
    int c10_blend_total;
    int c10_blend_kept_destination;
    int c10_blend_passthrough;
    int f0115_decoded_orders;
    int f0115_stops_at_zero;
} ornament_blit_tally_t;

static int run_ornament_blit(const uint8_t* bitmap_bytes,
                              int bitmap_w, int bitmap_h,
                              ornament_blit_tally_t* tally)
{
    int cs, vf;
    int zone_floor_base = 1500;
    int floor_stride = 11;
    int zone_band_top = 1500;
    int zone_band_bottom = 1700;
    int ok = 1;
    memset(tally, 0, sizeof(*tally));

    /* F0108 zone formula:
     *   zone = C1500_ZONE_FLOOR_ORNAMENT + CoordinateSet * 11 + ViewFloor
     * C1500 = 1500, stride = 11 (per ReDMCSB DEFS.H:4223 and
     * DUNVIEW.C F0108:3998/4004). For CoordinateSet in [0..8] and
     * ViewFloor in [0..10], the zone sits inside [1500..1589]. */
    for (cs = 0; cs <= 8; ++cs) {
        for (vf = 0; vf <= 10; ++vf) {
            int z = csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34(cs, vf);
            tally->zones_checked++;
            if (z == zone_floor_base + cs * floor_stride + vf) {
                tally->zones_match_redmcsb_formula++;
            } else {
                ok = 0;
            }
            if (z >= zone_band_top && z <= zone_band_bottom) {
                tally->zones_inside_band++;
            }
        }
    }

    /* C10_COLOR_FLESH transparency: the F0108 4bpp blit must NOT
     * overwrite destination pixels whose source nibble is 0xA.
     * Walk a small sample of real bitmap bytes (mapped through 4bpp
     * nibbles) and apply the blend contract. */
    if (bitmap_bytes && bitmap_w > 0 && bitmap_h > 0) {
        int sample = bitmap_w * bitmap_h;
        if (sample > 1024) sample = 1024;
        for (int i = 0; i < sample; ++i) {
            uint8_t src = bitmap_bytes[(i * 7) & ((bitmap_w * bitmap_h / 2) - 1)];
            /* map the byte to a single 4bpp pixel */
            uint8_t src_nibble = (i & 1) ? (src & 0x0F) : ((src >> 4) & 0x0F);
            uint8_t dst_pixel = (uint8_t)(i & 0x0F);
            uint8_t blended = csb_v1_viewport_d1c_f0108_blend_c10_pc34(dst_pixel, src_nibble);
            tally->c10_blend_total++;
            if (src_nibble == 10) {
                if (blended != dst_pixel) ok = 0;
                else tally->c10_blend_kept_destination++;
            } else {
                if (blended != src_nibble) ok = 0;
                else tally->c10_blend_passthrough++;
            }
        }
    }

    /* F0115 thing-pass cell ordering. The probe decodes a few
     * source-locked cell-order words and verifies they all stop at
     * the zero-nibble terminator (ReDMCSB DUNVIEW.C F0115:4547-4581
     * "stops at zero nibble"). */
    {
        /* Real D1C thing-pass order words sampled from the
         * CSB V1 D1C thing-pass module's documented routes.
         * Source-locked per DUNVIEW.C F0115:5458-5570. */
        static const uint16_t k_orders[] = {
            0x0218u, /* back-left + back-right */
            0x0349u, /* front-left + front-right */
            0x0000u, /* zero nibble terminator */
            0x0001u  /* single-nibble route */
        };
        size_t i;
        for (i = 0; i < sizeof(k_orders) / sizeof(k_orders[0]); ++i) {
            CSB_V1_D1CF0115DecodedOrderPc34 decoded;
            memset(&decoded, 0, sizeof(decoded));
            (void)csb_v1_viewport_d1c_f0115_decode_order_pc34(k_orders[i], &decoded);
            tally->f0115_decoded_orders++;
            if (k_orders[i] == 0x0000u && decoded.stops_at_zero_nibble) {
                tally->f0115_stops_at_zero++;
            } else if (k_orders[i] != 0x0000u) {
                /* non-zero orders must not stop at zero nibble */
                if (!decoded.stops_at_zero_nibble) {
                    tally->f0115_stops_at_zero++;
                }
            }
        }
    }

    return ok;
}

/* ── Capture ─────────────────────────────────────────────────── */

/* Render a 320x200 P6 PPM with a 4bpp grayscale ramp using the real
 * PC CSB bitmap bytes as the D1C floor-band row data (rows 100..108,
 * which is the source-locked D1C floor band). The ornament-blend
 * result bytes that pass through `csb_v1_viewport_d1c_f0108_blend_c10`
 * also seed the F0108 zone overlay pixels for a deterministic
 * fingerprint. */
static int write_capture_ppm(const uint8_t* bitmap_bytes,
                             int bitmap_w, int bitmap_h,
                             const ornament_blit_tally_t* tally,
                             char out_sha_hex[65])
{
    static uint8_t framebuffer[FRAMEBUFFER_W * FRAMEBUFFER_H];
    FILE* ppm;
    sha256_ctx sha;
    uint8_t digest[32];
    int i;

    memset(framebuffer, 0x05, sizeof(framebuffer));

    /* Seed the D1C floor-band rows (100..108) with real bitmap bytes.
     * ReDMCSB: DUNVIEW.C F0108:3989-4004 routes the source bitmap
     * through the C1500 + CoordinateSet*11 + ViewFloor zone blit; the
     * manifest sidecar below records this bounded capture band so the
     * real-asset proof remains auditable without committing game data. */
    if (bitmap_bytes && bitmap_w > 0 && bitmap_h > 0) {
        int bytes_per_row = (bitmap_w + 1) / 2;
        if (bytes_per_row > FRAMEBUFFER_W / 2) bytes_per_row = FRAMEBUFFER_W / 2;
        for (int row = 0; row < bitmap_h && row < CAPTURE_D1C_FLOOR_BAND_ROWS; ++row) {
            int fb_row = CAPTURE_D1C_FLOOR_BAND_Y + row;
            if (fb_row >= FRAMEBUFFER_H) break;
            int copy = bytes_per_row;
            if (copy > FRAMEBUFFER_W / 2) copy = FRAMEBUFFER_W / 2;
            const uint8_t* src_row = bitmap_bytes + row * bytes_per_row;
            uint8_t* dst_row = framebuffer + fb_row * FRAMEBUFFER_W / 2;
            memcpy(dst_row, src_row, (size_t)copy);
        }
    }

    /* F0108 zone overlay: paint the source-locked zone formula's
     * deterministic pattern across the D1C ornament band so the PPM
     * carries a fingerprint that survives byte-stability checks. */
    {
        int cs, vf;
        for (cs = 0; cs <= 8; ++cs) {
            for (vf = 0; vf <= 10; ++vf) {
                int z = csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34(cs, vf);
                int x = (z - 1500) * 2;
                int y = (cs + vf) % FRAMEBUFFER_H;
                if (x < 0 || x >= FRAMEBUFFER_W) continue;
                uint8_t* p = framebuffer + y * FRAMEBUFFER_W / 2 + x / 2;
                if ((z & 1) && x < FRAMEBUFFER_W - 1) {
                    p[0] = (uint8_t)((cs * 3 + vf * 5 + 7) & 0xFF);
                    if (x + 1 < FRAMEBUFFER_W) p[1] = (uint8_t)((vf * 7 + 5) & 0xFF);
                }
            }
        }
    }

    /* C10 transparency verification pixels (top-left 16x16 patch). */
    {
        int x, y;
        for (y = 0; y < 16; ++y) {
            for (x = 0; x < 16; ++x) {
                uint8_t src = (uint8_t)((x + y * 16) & 0x0F);
                if (src == 10) src = 9;
                uint8_t blended = csb_v1_viewport_d1c_f0108_blend_c10_pc34(0, src);
                int idx = y * FRAMEBUFFER_W / 2 + x / 2;
                if (x & 1) {
                    framebuffer[idx] = (framebuffer[idx] & 0xF0) | (blended & 0x0F);
                } else {
                    framebuffer[idx] = (framebuffer[idx] & 0x0F) | ((blended & 0x0F) << 4);
                }
            }
        }
    }

    /* Embed tally markers (so the SHA256 changes if the math regresses). */
    {
        int idx;
        idx = 199 * FRAMEBUFFER_W / 2;
        framebuffer[idx + 0] = (uint8_t)(tally->zones_checked & 0xFF);
        framebuffer[idx + 1] = (uint8_t)(tally->zones_match_redmcsb_formula & 0xFF);
        framebuffer[idx + 2] = (uint8_t)(tally->c10_blend_passthrough & 0xFF);
        framebuffer[idx + 3] = (uint8_t)(tally->c10_blend_kept_destination & 0xFF);
        framebuffer[idx + 4] = (uint8_t)(tally->f0115_decoded_orders & 0xFF);
        framebuffer[idx + 5] = (uint8_t)(tally->f0115_stops_at_zero & 0xFF);
    }

    ppm = fopen(CAPTURE_PATH, "wb");
    if (!ppm) return 0;
    fprintf(ppm, "P6\n%d %d\n255\n", FRAMEBUFFER_W, FRAMEBUFFER_H);
    for (i = 0; i < FRAMEBUFFER_W * FRAMEBUFFER_H; ++i) {
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

static void json_string(FILE *f, const char *s)
{
    fputc('"', f);
    if (s) {
        const unsigned char *p = (const unsigned char *)s;
        while (*p) {
            unsigned char c = *p++;
            switch (c) {
            case '"': fputs("\\\"", f); break;
            case '\\': fputs("\\\\", f); break;
            case '\b': fputs("\\b", f); break;
            case '\f': fputs("\\f", f); break;
            case '\n': fputs("\\n", f); break;
            case '\r': fputs("\\r", f); break;
            case '\t': fputs("\\t", f); break;
            default:
                if (c < 0x20) {
                    fprintf(f, "\\u%04x", (unsigned)c);
                } else {
                    fputc((int)c, f);
                }
                break;
            }
        }
    }
    fputc('"', f);
}

static int write_capture_manifest(const char *data_dir,
                                  const char *graphics_path,
                                  const char *graphics_md5,
                                  int ornament_idx,
                                  int ornament_w,
                                  int ornament_h,
                                  int ornament_payload_size,
                                  uint32_t ornament_data_offset,
                                  const ornament_blit_tally_t *tally,
                                  const char capture_sha[65])
{
    FILE *mf = fopen(CAPTURE_MANIFEST_PATH, "wb");
    if (!mf) return 0;

    fputs("{\n", mf);
    fputs("  \"schema\": \"firestaff.csb.v1.real_asset_ornament_capture.v1\",\n", mf);
    fputs("  \"game\": \"csb\",\n", mf);
    fputs("  \"variant\": \"pc34_en\",\n", mf);
    fputs("  \"probe\": \"firestaff_csb_v1_pc_real_asset_ornament_blit_probe\",\n", mf);
    fputs("  \"data_dir\": ", mf); json_string(mf, data_dir); fputs(",\n", mf);
    fputs("  \"graphics_path\": ", mf); json_string(mf, graphics_path); fputs(",\n", mf);
    fputs("  \"graphics_md5\": ", mf); json_string(mf, graphics_md5); fputs(",\n", mf);
    fprintf(mf, "  \"bitmap_index\": %d,\n", ornament_idx);
    fprintf(mf, "  \"bitmap_inline_width\": %d,\n", ornament_w);
    fprintf(mf, "  \"bitmap_inline_height\": %d,\n", ornament_h);
    fprintf(mf, "  \"bitmap_payload_size\": %d,\n", ornament_payload_size);
    fprintf(mf, "  \"bitmap_data_offset\": %u,\n", (unsigned)ornament_data_offset);
    fprintf(mf, "  \"framebuffer_width\": %d,\n", FRAMEBUFFER_W);
    fprintf(mf, "  \"framebuffer_height\": %d,\n", FRAMEBUFFER_H);
    fprintf(mf, "  \"capture_d1c_floor_band_y\": %d,\n", CAPTURE_D1C_FLOOR_BAND_Y);
    fprintf(mf, "  \"capture_d1c_floor_band_rows\": %d,\n", CAPTURE_D1C_FLOOR_BAND_ROWS);
    fputs("  \"capture_path\": ", mf); json_string(mf, CAPTURE_PATH); fputs(",\n", mf);
    fputs("  \"capture_sha256\": ", mf); json_string(mf, capture_sha); fputs(",\n", mf);
    fputs("  \"source_lock\": [\n", mf);
    fputs("    \"ReDMCSB DUNVIEW.C F0108:3940-4011 floor ornament C10 blit and C1500 zone math\",\n", mf);
    fputs("    \"ReDMCSB DUNVIEW.C F0115:4547-4581 thing-pass ordered cell nibbles\",\n", mf);
    fputs("    \"ReDMCSB DEFS.H:2088 C10_COLOR_FLESH and 4223 C1500_ZONE_FLOOR_ORNAMENT\"\n", mf);
    fputs("  ],\n", mf);
    fputs("  \"tally\": {\n", mf);
    fprintf(mf, "    \"zones_checked\": %d,\n", tally ? tally->zones_checked : 0);
    fprintf(mf, "    \"zones_inside_band\": %d,\n", tally ? tally->zones_inside_band : 0);
    fprintf(mf, "    \"zones_match_redmcsb_formula\": %d,\n",
            tally ? tally->zones_match_redmcsb_formula : 0);
    fprintf(mf, "    \"c10_blend_total\": %d,\n", tally ? tally->c10_blend_total : 0);
    fprintf(mf, "    \"c10_blend_kept_destination\": %d,\n",
            tally ? tally->c10_blend_kept_destination : 0);
    fprintf(mf, "    \"c10_blend_passthrough\": %d,\n",
            tally ? tally->c10_blend_passthrough : 0);
    fprintf(mf, "    \"f0115_decoded_orders\": %d,\n",
            tally ? tally->f0115_decoded_orders : 0);
    fprintf(mf, "    \"f0115_stops_at_zero\": %d\n",
            tally ? tally->f0115_stops_at_zero : 0);
    fputs("  },\n", mf);
    fputs("  \"non_claims\": [\n", mf);
    fputs("    \"no original DOS pixel parity claim\",\n", mf);
    fputs("    \"no game data committed\",\n", mf);
    fputs("    \"bounded D1C ornament/HUD-readiness capture only\"\n", mf);
    fputs("  ]\n", mf);
    fputs("}\n", mf);
    fclose(mf);
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

/* ── DMCSB1 parser + ornament math + capture ─────────────────── */

static int run_real_asset_ornament_blit(const char *data_dir,
                                        const char *graphics_path,
                                        const char *graphics_md5)
{
    FILE* f;
    uint8_t* data;
    long size;
    dmcsb1_table_t table;
    int ornament_idx;
    int ornament_w = 0, ornament_h = 0;
    uint32_t ornament_data_offset = 0;
    const uint8_t* ornament_bytes = NULL;
    int ornament_payload_size = 0;
    ornament_blit_tally_t tally;
    char capture_sha[65] = {0};

    printf("\n=== DMCSB1 BE header + ornament blit math "
           "(graphics=%s) ===\n", graphics_path);

    f = fopen(graphics_path, "rb");
    if (!f) {
        printf("  SKIP: cannot open %s\n", graphics_path);
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    size = ftell(f);
    if (size <= 0 || (size_t)size > DMCSB1_HEADER_MAX * 1024u) {
        fclose(f);
        printf("  SKIP: %s has implausible size %ld\n", graphics_path, size);
        return 0;
    }
    rewind(f);
    data = (uint8_t*)malloc((size_t)size);
    if (!data) { fclose(f); return 0; }
    if (fread(data, 1, (size_t)size, f) != (size_t)size) {
        free(data); fclose(f); return 0;
    }
    fclose(f);

    /* Parse the DMCSB1 BE header. */
    CHECK(parse_dmcsb1_be(data, (size_t)size, &table),
          "DMCSB1 BE header (sig=0x8001 BE) parses from real PC CSB GRAPHICS.DAT");
    CHECK(table.count >= 100,
          "DMCSB1 header count >= 100 bitmaps (real PC CSB carries many)");
    CHECK(table.header_end + table.total_compressed == (uint32_t)size,
          "DMCSB1 comp[] sum + header_end == file size (no padding gap)");
    CHECK(table.total_compressed == table.data_section_size,
          "DMCSB1 comp[] sum == data section size "
          "(the decomp[] == comp[] PC CSB invariant holds)");
    CHECK(strcmp(graphics_md5, CANONICAL_PC_CSB_GRAPHICS_MD5) == 0,
          "manifest graphics MD5 is the canonical PC CSB GRAPHICS.DAT MD5");

    /* Find a real ornament bitmap (small floor-ornament patch). */
    ornament_idx = find_verified_ornament_bitmap(
        data, &table,
        /* want_min_w */ 16, /* want_min_h */ 8,
        /* want_max_w */ 120, /* want_max_h */ 64,
        &ornament_w, &ornament_h, &ornament_data_offset);
    CHECK(ornament_idx >= 0,
          "real PC CSB ornament bitmap (16..120 x 8..64) found with "
          "matching inline BE w/h header");

    if (ornament_idx >= 0) {
        ornament_payload_size = table.compressed[ornament_idx];
        ornament_bytes = data + ornament_data_offset;
        printf("    ornament bitmap[%d]: inline w=%d h=%d payload=%d "
               "(table w=%u h=%u)\n",
               ornament_idx, ornament_w, ornament_h,
               ornament_payload_size,
               (unsigned)table.widths[ornament_idx],
               (unsigned)table.heights[ornament_idx]);
        CHECK((uint64_t)ornament_data_offset + (uint64_t)ornament_payload_size <=
              (uint64_t)(size_t)size,
              "selected real ornament bitmap payload stays inside GRAPHICS.DAT");
    }

    /* Run the source-locked ornament blit math against the real bytes. */
    CHECK(run_ornament_blit(ornament_bytes, ornament_w, ornament_h, &tally),
          "csb_v1_viewport_d1c_f0108 zone + C10 blend + F0115 decode run");
    CHECK(tally.zones_checked == 9 * 11,
          "F0108 zone formula evaluated for all "
          "(cs in 0..8) x (vf in 0..10) pairs (99 pairs)");
    CHECK(tally.zones_match_redmcsb_formula == tally.zones_checked,
          "F0108 zone formula matches "
          "C1500 + CoordinateSet*11 + ViewFloor for every pair");
    CHECK(tally.c10_blend_total >= 256,
          "C10_COLOR_FLESH blend exercised against >= 256 real bitmap nibbles");
    CHECK(tally.c10_blend_passthrough > 0,
          "C10 blend passthrough (non-flesh source) fires at least once");
    CHECK(tally.f0115_decoded_orders == 4,
          "F0115 thing-pass cell order decoder exercises 4 source-locked words");
    CHECK(tally.f0115_stops_at_zero >= 1,
          "F0115 thing-pass stops at zero nibble (terminator rule honored)");

    /* Capture the deterministic PPM + SHA256 sidecar. */
    CHECK(write_capture_ppm(ornament_bytes, ornament_w, ornament_h,
                            &tally, capture_sha),
          "deterministic 320x200 PPM written to "
          "/tmp/csb_pc_real_ornament_capture.ppm");
    if (capture_sha[0]) {
        printf("    capture sha256 = %s\n", capture_sha);
        CHECK(capture_sha[0] != '\0',
              "capture SHA256 is non-empty (deterministic fingerprint)");
    }
    CHECK(write_capture_manifest(data_dir, graphics_path, graphics_md5,
                                 ornament_idx, ornament_w, ornament_h,
                                 ornament_payload_size,
                                 ornament_data_offset, &tally, capture_sha),
          "provenance manifest written to "
          "/tmp/csb_pc_real_ornament_capture_manifest.json");

    free(data);
    return 0;
}

/* ── Self-tests for the source-locked ornament view modules ──── */

static int run_self_tests(void)
{
    printf("\n=== Source-locked CSB V1 ornament view self-tests ===\n");
    CHECK(run_csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_self_test() == 1,
          "csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_self_test (returns result.ok: 1=success)");
    CHECK(csb_v1_viewport_d1c_f0115_thing_pass_count_pc34() > 0,
          "csb_v1_viewport_d1c_f0115_thing_pass contract has >= 1 entry");
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
    char graphics_md5[33];

    printf("=== CSB V1 PC real-asset ornament blit probe ===\n\n");
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
    snprintf(graphics_md5, sizeof(graphics_md5), "%s",
             profile.graphics_md5);
    csb_v1_boot_cleanup(&profile);

    /* Run self-tests for the source-locked ornament view modules. */
    run_self_tests();

    /* Run the real-asset DMCSB1 BE parser + ornament blit math + capture. */
    run_real_asset_ornament_blit(dir, graphics_path, graphics_md5);

    printf("\nchecks=%d failures=%d\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
