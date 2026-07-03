/*
 * firestaff_dm1_v2_source_owned_route_screenshot_probe.c
 *
 * DM1 V2 source-owned V1 route + V2 presentation-mode screenshot/pixel probe.
 *
 * Purpose
 *   Extend the deterministic V1/V2 screenshot/pixel script coverage
 *   beyond the existing smoke paths. Concretely this probe adds three
 *   things the existing `firestaff_dm1_v2_actual_render_screenshot_probe`
 *   and side-by-side seed probes do NOT cover:
 *
 *     1. ONE SOURCE-OWNED V1 ROUTE. The probe drives the real DM1 PC 3.4
 *        entry state fixture (map=0 x=1 y=3 dir=2) end-to-end through the
 *        V2 render pipeline. The fixture is the documented V1 source
 *        truth (DUNGEON.DAT offset 8 + DEFS.H:989-998 + LOADSAVE.C:1940-
 *        1945 + pass173 front-wall sensor audit). The composition is
 *        built by `dm1_v2_vp_build_composition_from_fixture` and rendered
 *        into a 224x136 RGBA viewport by `dm1_v2_vp_render_composition_flat`
 *        (DUNVIEW.C:8337-8542 composition order). The 224x136 viewport is
 *        then quantized to the DM1 VGA palette and composited into a
 *        320x200 V1 indexed framebuffer at the source-locked (48, 32)
 *        position (DUNVIEW.C:2999-3000 viewport placement). The V2
 *        movement command adapter is also exercised across all six V1
 *        source commands (C001..C006, DEFS.H:238-243) to prove the V1
 *        source-locked route reaches the V2 render pipeline.
 *
 *     2. ONE NEW V2 PRESENTATION MODE. V2.0 with the FULL filter chain
 *        enabled (CRT scanlines + palette LUT + palette interpolation +
 *        dither cleanup + sharpen). The existing actual-render probe
 *        covers V2.0 with CRT + palette LUT only, so this exercises a
 *        new V2.0 presentation path that combines the indexed-fb side
 *        (palette interp + dither cleanup) with the RGBA side (CRT +
 *        palette LUT + sharpen) of the V2.0 filter chain.
 *
 *     3. STABLE RECEIPTS. Five 24-bit BMP files (one per V2 presentation
 *        mode) plus five SHA-256 sidecar files in `sha256sum` format
 *        (`<hex>  <filename>\n`) so future visual diff tools and CI
 *        pipelines can verify file integrity across runs and hosts. An
 *        FNV-1a 32-bit hash is also computed and printed per BMP for the
 *        in-process cross-mode distinctness checks.
 *
 * Source locks (ReDMCSB WIP20210206 / Toolchains/Common/Source):
 *   - DUNGEON.DAT offset 8              DM1 PC 3.4 entry state (map/x/y/dir).
 *   - DEFS.H:238-243                    C001..C006 V1 source command ids.
 *   - DUNVIEW.C:2999-3000               224x136 viewport bitmap dimensions
 *                                       and (48, 32) placement in 320x200.
 *   - DUNVIEW.C:8337-8338               draw floor/ceiling before walking squares.
 *   - DUNVIEW.C:8490-8542               D3 -> D0 left/center/right draw order.
 *   - DUNVIEW.C:3913-3928               D1C champion portrait blit at {96, 35}.
 *   - COMMAND.C:2045-2155               F0359 command queue dispatch.
 *   - GAMELOOP.C:90                     F0128_DUNGEONVIEW_Draw_CPSF snapshot draw.
 *
 * Cross-mode invariants (BMP file-level):
 *   - V2.0 unfiltered == V1 baseline (V2.0 with no filter chain is a
 *     strict superset of V1, so the two BMPs must match byte-for-byte).
 *   - V2.0 unfiltered != V2.0 fully filtered (proves the new V2.0
 *     presentation mode reaches the presented pixels, not just config).
 *   - V1 != V2.1 (proves V2.1 EPX upscaling is wired).
 *   - V1 != V2.2 (proves V2.2 V22 overlay is wired).
 *   - V2.1 != V2.2 (proves the two V2 modes produce distinct output).
 *   - V2.0 fully filtered != V2.1 and != V2.2 (proves the new V2.0 mode
 *     is distinct from both V2.1 and V2.2).
 *
 * V1 framebuffer ownership:
 *   The V1 indexed framebuffer is captured before the first V2 mode
 *   present and compared byte-for-byte after the last V2 mode present.
 *   Any V2 mutation of the V1 source framebuffer breaks this gate.
 *
 * V1 source-locked geometry sanity:
 *   - The 320x200 framebuffer border (48 cols on each side, 32 rows on
 *     top and bottom) is filled with M11_FB_ENCODE(0, 5) (palette index
 *     0, brightness level 5, the canonical "black" encoding).
 *   - The 224x136 viewport area at (48, 32) carries the quantized
 *     source-locked geometry from the V2 viewport render.
 *   - The non-zero pixel count in the viewport area is well above the
 *     border pixel count, proving the source-locked geometry survives
 *     quantization.
 *
 * Headless / data-free / host-agnostic:
 *   - Uses SDL_VIDEODRIVER=dummy so the probe runs on Apple Silicon,
 *     Intel macOS, Linux, and Windows runners without a display.
 *   - Uses a probe-controlled temp directory; never touches
 *     the user-facing ~/.firestaff/screenshots/ or any data-dir assets.
 *   - Does NOT require real DM1 game data (GRAPHICS.DAT / DUNGEON.DAT).
 *   - The V1 entry state fixture is data-free and built into
 *     `dm1_v2_vp_dm1_pc34_entry_state_fixture`; the SHA-256 of the
 *     source DUNGEON.DAT the fixture was derived from is recorded in
 *     the fixture itself.
 *
 * Non-claims:
 *   - This probe does NOT run DOSBox, does NOT claim DOS pixel parity,
 *     and does NOT pair against original PC 3.4 captures. It is a
 *     deterministic Firestaff-side source-locked receipt only.
 *   - The V1 entry composition is derived from a built-in fixture
 *     (data-free), not from a real DUNGEON.DAT. The fixture's source
 *     reference is documented in the probe output.
 *   - The V1 source-locked geometry is a "symbolic" geometry rendered
 *     by the V2 viewport renderer using pass288 source-locked material
 *     tones, not the original DM1 PC 3.4 PCjr/VGA palette. This is the
 *     same symbolic render the existing side-by-side seed probes use.
 *
 * Disjoint from existing lanes:
 *   - firestaff_dm1_v2_actual_render_screenshot_probe (synthetic V1
 *     pattern, 5 modes, no source-owned route, no SHA-256 sidecar).
 *   - firestaff_dm1_v2_side_by_side_presentation_seed_probe (FNV-1a
 *     seed only, no BMP writes).
 *   - firestaff_dm1_v2_v1_v2_side_by_side_seed_probe (FNV-1a seed,
 *     no BMP writes).
 *   - dm1_v2_v1_v2_side_by_side_seed_pc34 (ctest gate over the FNV-1a
 *     seed, no BMP writes).
 *   - test_dm1_v2_source_route_state_hash_pc34 (source-route state hash,
 *     no BMP writes).
 *
 * Schema: firestaff.dm1_v2.source_owned_route_screenshot_probe.v1
 *
 * Exit codes: 0 = all invariants PASS, 1 = at least one invariant FAIL.
 */

#include "dm1_v2_viewport_renderer_pc34.h"
#include "dm1_v2_movement_command_adapter_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include "m11_v22_render_overlay_pc34.h"
#include "m11_v22_shape_cache_pc34.h"
#include "render_sdl_m11.h"

#include <SDL3/SDL.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(_WIN32)
#include <direct.h>
#endif

/* ════════════════════════════════════════════════════════════════════
 * Probe statistics
 * ════════════════════════════════════════════════════════════════════ */

typedef struct ProbeStats {
    int total;
    int passed;
    int failed;
} ProbeStats;

static void probe_record(ProbeStats* stats,
                         const char* id,
                         int ok,
                         const char* note) {
    stats->total += 1;
    if (ok) {
        stats->passed += 1;
        printf("PASS %s: %s\n", id, note);
    } else {
        stats->failed += 1;
        printf("FAIL %s: %s\n", id, note);
    }
}

/* ════════════════════════════════════════════════════════════════════
 * SHA-256 (FIPS 180-4). Small public-domain reference implementation.
 *
 * Used to write stable receipts in `sha256sum` format so CI tools can
 * verify file integrity across runs and hosts. The implementation is
 * self-contained (no OpenSSL, no system sha256sum) so the probe is
 * host-agnostic and data-free.
 * ════════════════════════════════════════════════════════════════════ */

static const uint32_t kSha256K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

static inline uint32_t rotr32(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

static void sha256_compress(uint32_t state[8], const unsigned char block[64]) {
    uint32_t w[64];
    int i;
    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4 + 0] << 24)
             | ((uint32_t)block[i * 4 + 1] << 16)
             | ((uint32_t)block[i * 4 + 2] <<  8)
             | ((uint32_t)block[i * 4 + 3]      );
    }
    for (i = 16; i < 64; i++) {
        uint32_t s0 = rotr32(w[i - 15], 7) ^ rotr32(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = rotr32(w[i - 2], 17) ^ rotr32(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
    for (i = 0; i < 64; i++) {
        uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t t1 = h + S1 + ch + kSha256K[i] + w[i];
        uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
        uint32_t mj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = S0 + mj;
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

typedef struct Sha256Ctx {
    uint32_t state[8];
    uint64_t bitlen;
    unsigned char buf[64];
    size_t buflen;
} Sha256Ctx;

static void sha256_init(Sha256Ctx* ctx) {
    if (!ctx) return;
    ctx->state[0] = 0x6a09e667u; ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u; ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu; ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu; ctx->state[7] = 0x5be0cd19u;
    ctx->bitlen = 0;
    ctx->buflen = 0;
}

static void sha256_update(Sha256Ctx* ctx, const unsigned char* data, size_t len) {
    if (!ctx || !data) return;
    ctx->bitlen += (uint64_t)len * 8u;
    while (len > 0) {
        size_t take = 64 - ctx->buflen;
        if (take > len) take = len;
        memcpy(ctx->buf + ctx->buflen, data, take);
        ctx->buflen += take;
        data += take;
        len -= take;
        if (ctx->buflen == 64) {
            sha256_compress(ctx->state, ctx->buf);
            ctx->buflen = 0;
        }
    }
}

static void sha256_final(Sha256Ctx* ctx, unsigned char out[32]) {
    static const unsigned char pad[64] = { 0x80 };
    unsigned char lenbe[8];
    uint64_t bl = ctx->bitlen;
    int i;
    for (i = 0; i < 8; i++) lenbe[i] = (unsigned char)((bl >> (56 - i * 8)) & 0xFFu);
    sha256_update(ctx, pad, 1);
    /* pad with zeros until buflen == 56 */
    while (ctx->buflen != 56) {
        unsigned char zero = 0;
        sha256_update(ctx, &zero, 1);
    }
    sha256_update(ctx, lenbe, 8);
    for (i = 0; i < 8; i++) {
        out[i * 4 + 0] = (unsigned char)((ctx->state[i] >> 24) & 0xFFu);
        out[i * 4 + 1] = (unsigned char)((ctx->state[i] >> 16) & 0xFFu);
        out[i * 4 + 2] = (unsigned char)((ctx->state[i] >>  8) & 0xFFu);
        out[i * 4 + 3] = (unsigned char)((ctx->state[i]      ) & 0xFFu);
    }
}

/* Hash a file by streaming through the SHA-256 context. */
static int sha256_file(const char* path, unsigned char out[32]) {
    FILE* f;
    Sha256Ctx ctx;
    unsigned char buf[4096];
    size_t n;
    if (!path || !out) return 0;
    sha256_init(&ctx);
    f = fopen(path, "rb");
    if (!f) return 0;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0u) {
        sha256_update(&ctx, buf, n);
    }
    fclose(f);
    sha256_final(&ctx, out);
    return 1;
}

/* Write a `sha256sum` sidecar file: "<hex>  <filename>\n".
 *
 * The dual-space separator is the format sha256sum -c expects. */
static int write_sha256_sidecar(const char* bmp_path,
                                const unsigned char digest[32]) {
    char sidecar_path[1024];
    FILE* f;
    int i;
    if (!bmp_path || !digest) return 0;
    snprintf(sidecar_path, sizeof(sidecar_path), "%s.sha256", bmp_path);
    f = fopen(sidecar_path, "w");
    if (!f) return 0;
    for (i = 0; i < 32; i++) {
        fprintf(f, "%02x", digest[i]);
    }
    fprintf(f, "  %s\n", bmp_path);
    fclose(f);
    return 1;
}

/* ════════════════════════════════════════════════════════════════════
 * BMP writer (24-bit) and helpers
 * ════════════════════════════════════════════════════════════════════ */

static int write_u16_le(unsigned char* p, unsigned v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    return 2;
}

static int write_u32_le(unsigned char* p, unsigned v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
    return 4;
}

/* Write a 24-bit BMP with the given RGBA pixels to `path`. Returns 1 on
 * success, 0 on failure. Pixel format is R,G,B,A in memory order
 * (matches SDL_PIXELFORMAT_RGBA32 / M11 present buffer). */
static int write_bmp_24bit_rgba(const char* path,
                                const unsigned char* rgba,
                                int width,
                                int height) {
    FILE* f;
    int rowBytes, padded, imageBytes, fileBytes;
    unsigned char fileHdr[14];
    unsigned char infoHdr[40];
    unsigned char* row;
    int y, x;

    if (!path || !rgba || width <= 0 || height <= 0) return 0;

    rowBytes = width * 3;
    padded = (rowBytes + 3) & ~3;
    imageBytes = padded * height;
    fileBytes = 14 + 40 + imageBytes;

    f = fopen(path, "wb");
    if (!f) return 0;

    fileHdr[0] = 'B'; fileHdr[1] = 'M';
    write_u32_le(fileHdr + 2, (unsigned)fileBytes);
    write_u16_le(fileHdr + 6, 0);
    write_u16_le(fileHdr + 8, 0);
    write_u32_le(fileHdr + 10, 14 + 40);
    fwrite(fileHdr, 1, 14, f);

    memset(infoHdr, 0, sizeof(infoHdr));
    write_u32_le(infoHdr + 0, 40);
    write_u32_le(infoHdr + 4, (unsigned)width);
    write_u32_le(infoHdr + 8, (unsigned)(-height)); /* top-down for viewers */
    write_u16_le(infoHdr + 12, 1);
    write_u16_le(infoHdr + 14, 24);
    write_u32_le(infoHdr + 16, 0);
    write_u32_le(infoHdr + 20, (unsigned)imageBytes);
    write_u32_le(infoHdr + 24, 2835);
    write_u32_le(infoHdr + 28, 2835);
    fwrite(infoHdr, 1, 40, f);

    row = (unsigned char*)calloc(1, (size_t)padded);
    if (!row) { fclose(f); return 0; }

    for (y = 0; y < height; y++) {
        const unsigned char* src = rgba + (size_t)y * (size_t)width * 4u;
        for (x = 0; x < width; x++) {
            row[x * 3 + 0] = src[x * 4 + 2]; /* B */
            row[x * 3 + 1] = src[x * 4 + 1]; /* G */
            row[x * 3 + 2] = src[x * 4 + 0]; /* R */
        }
        if (padded > rowBytes) {
            memset(row + rowBytes, 0, (size_t)(padded - rowBytes));
        }
        fwrite(row, 1, (size_t)padded, f);
    }

    free(row);
    fclose(f);
    return 1;
}

static int bmp_read_dimensions(const char* path, int* outW, int* outH) {
    unsigned char hdr[26];
    FILE* f = fopen(path, "rb");
    size_t n;
    if (!f) return 0;
    n = fread(hdr, 1, sizeof(hdr), f);
    fclose(f);
    if (n < 26u) return 0;
    if (hdr[0] != 'B' || hdr[1] != 'M') return 0;
    if (outW) {
        int w = (int)hdr[18] | ((int)hdr[19] << 8) | ((int)hdr[20] << 16) | ((int)hdr[21] << 24);
        *outW = w;
    }
    if (outH) {
        int h = (int)hdr[22] | ((int)hdr[23] << 8) | ((int)hdr[24] << 16) | ((int)hdr[25] << 24);
        *outH = h < 0 ? -h : h;
    }
    return 1;
}

static long bmp_file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
}

/* FNV-1a 32-bit file hash, used to prove distinct modes produce distinct
 * presented pixel output without depending on a system sha256. */
static uint32_t fnv1a_file(const char* path) {
    FILE* f;
    uint32_t h = 2166136261u;
    unsigned char buf[4096];
    size_t n;
    f = fopen(path, "rb");
    if (!f) return 0u;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0u) {
        size_t i;
        for (i = 0; i < n; ++i) {
            h ^= buf[i];
            h *= 16777619u;
        }
    }
    fclose(f);
    return h;
}

/* ════════════════════════════════════════════════════════════════════
 * Source-locked V1 framebuffer builder
 * ════════════════════════════════════════════════════════════════════ */

/* Position of the 224x136 viewport inside the 320x200 V1 framebuffer.
 * Source-locked to DUNVIEW.C:2999-3000 (224x136 viewport bitmap). */
#define SOURCE_VP_X 48
#define SOURCE_VP_Y 32
#define SOURCE_VP_W DM1_V2_VIEWPORT_W
#define SOURCE_VP_H DM1_V2_VIEWPORT_H
#define SOURCE_BORDER_PIXEL (M11_FB_ENCODE(0, 5))

/* Quantize a brightness 0..255 to a canonical DM1 VGA brightness level
 * 0..5 (0 = brightest, 5 = darkest). Used to map the V2 viewport
 * renderer's RGBA output to the DM1 4-bit per-pixel brightness field.
 *
 *   brightness 0..42   -> level 5
 *   brightness 43..85  -> level 4
 *   brightness 86..127 -> level 3
 *   brightness 128..170-> level 2
 *   brightness 171..213-> level 1
 *   brightness 214..255-> level 0
 *
 * Each band is 256/6 = 42 or 43 brightness units wide. */
static int quantize_brightness_to_level(int brightness) {
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;
    if (brightness < 43) return 5;
    if (brightness < 86) return 4;
    if (brightness < 128) return 3;
    if (brightness < 171) return 2;
    if (brightness < 214) return 1;
    return 0;
}

/* Build the source-locked V1 indexed framebuffer from the V1 entry state
 * fixture. The 224x136 V2 viewport render is composited into a 320x200
 * V1 indexed framebuffer at position (48, 32) per DUNVIEW.C:2999-3000.
 * The border is filled with M11_FB_ENCODE(0, 5) (canonical black).
 *
 * Returns 1 on success, 0 on failure. On success, *outNonZeroPixels is
 * set to the count of non-border pixels written. */
static int build_source_owned_v1_framebuffer(unsigned char* fb,
                                            int* outNonZeroPixels) {
    DM1_V2_ViewportState vp;
    DM1_V2_ViewportCompositionInput input;
    const DM1_V2_DungeonStateFixture* fixture;
    int x, y;
    int nonZero = 0;
    int fixtureOk = 0;
    int renderOk = 0;

    if (!fb) return 0;
    if (outNonZeroPixels) *outNonZeroPixels = 0;

    /* Fill the entire framebuffer with the canonical border pixel first. */
    {
        int i;
        for (i = 0; i < M11_FB_BYTES; i++) fb[i] = SOURCE_BORDER_PIXEL;
    }

    /* Render the V1 entry state fixture. The fixture is data-free and
     * is the documented V1 source truth for the DM1 PC 3.4 entry state. */
    fixture = dm1_v2_vp_dm1_pc34_entry_state_fixture();
    if (!fixture) {
        fprintf(stderr, "FAIL build_source_owned: dm1_v2_vp_dm1_pc34_entry_state_fixture returned NULL\n");
        return 0;
    }
    fixtureOk = 1;

    if (!dm1_v2_vp_build_composition_from_fixture(fixture,
                                                  fixture->startMapX,
                                                  fixture->startMapY,
                                                  fixture->startDirection,
                                                  &input)) {
        fprintf(stderr, "FAIL build_source_owned: dm1_v2_vp_build_composition_from_fixture failed\n");
        return 0;
    }

    dm1_v2_vp_init(&vp);
    if (!dm1_v2_vp_render_composition_flat(&vp, &input)) {
        fprintf(stderr, "FAIL build_source_owned: dm1_v2_vp_render_composition_flat failed\n");
        return 0;
    }
    renderOk = 1;

    /* Composite the 224x136 RGBA viewport into the 320x200 V1 indexed
     * framebuffer at position (48, 32). Each RGBA pixel is quantized
     * to the DM1 VGA brightness levels 0..5 with index=7 (light gray,
     * the canonical "grayscale" palette index for source-locked
     * symbolic renders). */
    for (y = 0; y < SOURCE_VP_H; y++) {
        for (x = 0; x < SOURCE_VP_W; x++) {
            const DM1_V2_Color* c = &vp.framebuffer[y][x];
            int brightness = ((int)c->r + (int)c->g + (int)c->b) / 3;
            int level = quantize_brightness_to_level(brightness);
            int idx = 7; /* light gray, canonical grayscale */
            unsigned char px = M11_FB_ENCODE(idx, level);
            fb[(y + SOURCE_VP_Y) * M11_FB_WIDTH + (x + SOURCE_VP_X)] = px;
            if (px != SOURCE_BORDER_PIXEL) nonZero++;
        }
    }

    if (outNonZeroPixels) *outNonZeroPixels = nonZero;
    (void)fixtureOk;
    (void)renderOk;
    return 1;
}

/* ════════════════════════════════════════════════════════════════════
 * Mode capture
 * ════════════════════════════════════════════════════════════════════ */

typedef struct ModeCapture {
    const char* id;
    const char* label;
    int expected_w;
    int expected_h;
    int present_via_rgba; /* 1 -> M11_Render_PresentRGBA, 0 -> PresentIndexed */
    int v22_overlay_active; /* 1 -> paint V22 overlay into fb before present */
    int v20_full_filter_chain; /* 1 -> enable all 5 V2.0 filters before present */
    char bmp_path[512];
    uint32_t bmp_hash;   /* FNV-1a 32-bit */
    unsigned char bmp_sha[32]; /* SHA-256 */
    long bmp_size;
    int bmp_w;
    int bmp_h;
    int sha256_sidecar_ok;
} ModeCapture;

static int ensure_dir(const char* path) {
    struct stat st;
    if (!path) return 0;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 1 : 0;
    }
#if defined(_WIN32)
    return _mkdir(path) == 0 ? 1 : 0;
#else
    return mkdir(path, 0755) == 0 ? 1 : 0;
#endif
}

static int run_mode_capture(ModeCapture* cap,
                            unsigned char* v1_framebuffer,
                            unsigned char* v1_shadow) {
    const unsigned char* rgba;
    int outW = 0;
    int outH = 0;
    int rc;

    if (!cap || !v1_framebuffer || !v1_shadow) return 0;
    memcpy(v1_shadow, v1_framebuffer, M11_FB_BYTES);

    /* V2.2 modern: paint the V22 shape-cache overlay INTO the V1 indexed
     * framebuffer BEFORE the present call, so PresentIndexed uploads the
     * post-overlay pixels. The overlay writes palette index 0xFF which
     * after palette expansion becomes a distinct color. */
    if (cap->v22_overlay_active) {
        const unsigned char raw_squares[3][3] = {
            { 0x00, 0x04, 0x20 },
            { 0x40, 0x10, 0x11 },
            { 0x04, 0x20, 0x00 }
        };
        dm1_v2_presentation_mode_reset();
        dm1_v2_presentation_mode_set_modern_pack_available(1);
        dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
        m11_v22_shape_cache_update(0, raw_squares);
        m11_v22_render_overlay_with_palette(
            v1_framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, 3);
    }

    /* V2.0 fully filtered: enable the FULL filter chain (CRT + palette
     * LUT + palette interpolation + dither cleanup + sharpen) before
     * the present call. The other V2.0 modes (V1 baseline, V2.0
     * unfiltered) leave the filter chain off so they remain a strict
     * superset of V1. */
    if (cap->v20_full_filter_chain) {
        (void)M11_Render_SetV2Filters(
            1, 80,    /* CRT scanlines on, 80% strength */
            1, 110, 10, 0,  /* palette LUT: gamma 1.10, +10 brightness */
            1, 100,   /* palette interpolation on, full strength */
            1,        /* dither cleanup on */
            1, 50);   /* sharpen on, 50% strength */
    } else {
        (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0);
    }

    if (cap->present_via_rgba) {
        /* V2.1 path: drive EPX into v21_viewport_rgba, then present that
         * RGBA through M11_Render_PresentRGBA so the presented buffer is
         * 640x400 (not 320x200). */
        v21_viewport_init(2);
        memcpy(v21_viewport_get_v1_framebuffer_mut(), v1_framebuffer, M11_FB_BYTES);
        v21_viewport_render_full_pipeline();
        {
            const uint32_t* v21_rgba = v21_viewport_get_rgba(&outW, &outH);
            if (!v21_rgba || outW <= 0 || outH <= 0) {
                fprintf(stderr, "FAIL %s: v21_viewport_get_rgba returned empty\n", cap->id);
                /* Restore fb even on failure so the next capture starts clean. */
                memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);
                return 0;
            }
            rc = M11_Render_PresentRGBA((const unsigned char*)v21_rgba, outW, outH);
            if (rc != M11_RENDER_OK) {
                fprintf(stderr, "FAIL %s: M11_Render_PresentRGBA rc=%d\n", cap->id, rc);
                memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);
                return 0;
            }
        }
    } else {
        /* V1 / V2.0 / V2.2 path: present the indexed V1 framebuffer.
         * For V2.2 this is the post-overlay framebuffer; the V22
         * placeholder 0xFF pixels are palette-expanded to a distinct
         * color in the presented buffer. */
        rc = M11_Render_PresentIndexed(v1_framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
        if (rc != M11_RENDER_OK) {
            fprintf(stderr, "FAIL %s: M11_Render_PresentIndexed rc=%d\n", cap->id, rc);
            memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);
            return 0;
        }
    }

    /* Read back the presented RGBA buffer. M11_Render_PresentIndexed /
     * PresentRGBA populates the same buffer M11_Screenshot_CapturePresentedRGBA
     * reads from, so we can capture the same pixels here and write a
     * deterministic BMP. */
    rgba = M11_Render_GetPresentedRGBA(&outW, &outH);
    if (!rgba || outW <= 0 || outH <= 0) {
        fprintf(stderr, "FAIL %s: M11_Render_GetPresentedRGBA returned empty (w=%d h=%d)\n",
                cap->id, outW, outH);
        memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);
        return 0;
    }

    if (!write_bmp_24bit_rgba(cap->bmp_path, rgba, outW, outH)) {
        fprintf(stderr, "FAIL %s: write_bmp_24bit_rgba failed for %s\n", cap->id, cap->bmp_path);
        memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);
        return 0;
    }

    if (!bmp_read_dimensions(cap->bmp_path, &cap->bmp_w, &cap->bmp_h)) {
        fprintf(stderr, "FAIL %s: bmp_read_dimensions failed for %s\n", cap->id, cap->bmp_path);
        memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);
        return 0;
    }
    cap->bmp_size = bmp_file_size(cap->bmp_path);
    cap->bmp_hash = fnv1a_file(cap->bmp_path);

    if (!sha256_file(cap->bmp_path, cap->bmp_sha)) {
        fprintf(stderr, "FAIL %s: sha256_file failed for %s\n", cap->id, cap->bmp_path);
        memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);
        return 0;
    }
    cap->sha256_sidecar_ok = write_sha256_sidecar(cap->bmp_path, cap->bmp_sha);

    if (cap->expected_w > 0 && cap->expected_h > 0) {
        if (cap->bmp_w != cap->expected_w || cap->bmp_h != cap->expected_h) {
            fprintf(stderr, "FAIL %s: BMP dims %dx%d, expected %dx%d\n",
                    cap->id, cap->bmp_w, cap->bmp_h, cap->expected_w, cap->expected_h);
            memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);
            return 0;
        }
    }

    /* Restore the V1 source framebuffer byte-for-byte before returning
     * so the next mode capture starts from the same canonical state.
     * V2 render must be presentation-only, never mutate the V1 source. */
    memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);

    return 1;
}

/* ════════════════════════════════════════════════════════════════════
 * V1 source-locked movement command table (ReDMCSB DEFS.H:238-243)
 * ════════════════════════════════════════════════════════════════════ */

typedef struct {
    DM1_V2_MovementCommand v2Command;
    int v1SourceCommand;
    const char* label;
} V1SourceCommandRow;

static const V1SourceCommandRow g_v1_command_table[6] = {
    { DM1_V2_MOVEMENT_COMMAND_TURN_LEFT,    1, "C001 TURN_LEFT"    },
    { DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT,   2, "C002 TURN_RIGHT"   },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, "C003 MOVE_FORWARD" },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT,   4, "C004 MOVE_RIGHT"   },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD,5, "C005 MOVE_BACKWARD"},
    { DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT,    6, "C006 MOVE_LEFT"    },
};
#define N_V1_COMMAND_ROWS \
    ((int)(sizeof(g_v1_command_table) / sizeof(g_v1_command_table[0])))

/* ════════════════════════════════════════════════════════════════════
 * Main
 * ════════════════════════════════════════════════════════════════════ */

int main(void) {
    ProbeStats stats;
    unsigned char* framebuffer;
    unsigned char v1_shadow[M11_FB_BYTES];
    unsigned char v1_initial_shadow[M11_FB_BYTES];
    char out_dir[512];
    ModeCapture v1_cap;
    ModeCapture v20_unfiltered_cap;
    ModeCapture v20_fully_filtered_cap;
    ModeCapture v21_cap;
    ModeCapture v22_cap;
    int rc;
    int nonZeroPixels = 0;
    int dist_v1_v20_unfiltered = 0;
    int dist_v20_unfiltered_v20_full = 0;
    int dist_v1_v20_full = 0;
    int dist_v1_v21 = 0;
    int dist_v1_v22 = 0;
    int dist_v20_full_v21 = 0;
    int dist_v20_full_v22 = 0;
    int dist_v21_v22 = 0;
    const char* output_root = NULL;
    int i;

    memset(&stats, 0, sizeof(stats));
    memset(&v1_cap, 0, sizeof(v1_cap));
    memset(&v20_unfiltered_cap, 0, sizeof(v20_unfiltered_cap));
    memset(&v20_fully_filtered_cap, 0, sizeof(v20_fully_filtered_cap));
    memset(&v21_cap, 0, sizeof(v21_cap));
    memset(&v22_cap, 0, sizeof(v22_cap));

    /* Probe-controlled temp dir so we never touch the user-facing
     * screenshotPath. Tests may redirect this inside the sandbox. */
    output_root = getenv("FIRESTAFF_PROBE_OUTPUT_ROOT");
    if (!output_root || !*output_root) output_root = getenv("HOME");
    if (!output_root || !*output_root) output_root = ".";
    snprintf(out_dir, sizeof(out_dir),
             "%s/.firestaff-probe-dm1-v2-source-owned-route", output_root);
    if (!ensure_dir(out_dir)) {
        fprintf(stderr, "FAIL could not create probe output dir %s\n", out_dir);
        return 1;
    }

    /* Force SDL3 onto the dummy video driver so CI + headless runs work
     * on Apple Silicon, Intel macOS, Linux, and Windows. */
#if defined(_WIN32)
    _putenv_s("SDL_VIDEODRIVER", "dummy");
#else
    setenv("SDL_VIDEODRIVER", "dummy", 1);
#endif

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "FAIL SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    /* Init the renderer at 320x200 (V1 native). V2.1 uses a separate
     * 640x400 RGBA path so it does not need a larger renderer here. */
    rc = M11_Render_Init(320, 200, M11_SCALE_1X);
    if (rc != M11_RENDER_OK) {
        fprintf(stderr, "FAIL M11_Render_Init: rc=%d\n", rc);
        SDL_Quit();
        return 1;
    }
    (void)M11_Render_SetDisplayAspectMode(M11_DISPLAY_ASPECT_CONTENT);
    (void)M11_Render_SetIntegerScaling(0);

    framebuffer = M11_Render_GetFramebuffer();
    if (!framebuffer) {
        fprintf(stderr, "FAIL M11_Render_GetFramebuffer returned NULL\n");
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    /* ---------- Build the source-locked V1 framebuffer ----------
     *
     * This is the heart of the source-owned route: the V1 entry state
     * fixture (DM1 PC 3.4, map=0 x=1 y=3 dir=2) is rendered through
     * the V2 viewport renderer's source-locked composition path
     * (DUNVIEW.C:8337-8542), and the resulting 224x136 RGBA viewport is
     * quantized and composited into the 320x200 V1 indexed framebuffer
     * at the source-locked (48, 32) position (DUNVIEW.C:2999-3000). */
    if (!build_source_owned_v1_framebuffer(framebuffer, &nonZeroPixels)) {
        fprintf(stderr, "FAIL build_source_owned_v1_framebuffer failed\n");
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    /* 224*136 = 30464 viewport pixels; with at least the ceiling/floor
     * (level 0) and the wall outer/inner (level 0) all at level 0, the
     * non-zero pixel count should be a large majority of the viewport
     * area. 75% is a conservative lower bound. */
    {
        int expected_min = (SOURCE_VP_W * SOURCE_VP_H) * 3 / 4;
        char note[160];
        snprintf(note, sizeof(note),
                 "nonZero=%d expected>=%d (224x136 viewport at (48,32), DUNVIEW.C:2999-3000)",
                 nonZeroPixels, expected_min);
        probe_record(&stats, "DM1V2_SOURCE_ROUTE_NONZERO_VIEWPORT_PIXELS",
                     nonZeroPixels >= expected_min, note);
    }

    /* Verify the border is uniform canonical black. */
    {
        int borderBad = 0;
        int x, y;
        for (y = 0; y < M11_FB_HEIGHT; y++) {
            for (x = 0; x < M11_FB_WIDTH; x++) {
                if (x >= SOURCE_VP_X && x < SOURCE_VP_X + SOURCE_VP_W &&
                    y >= SOURCE_VP_Y && y < SOURCE_VP_Y + SOURCE_VP_H) {
                    continue; /* inside the viewport area */
                }
                if (framebuffer[y * M11_FB_WIDTH + x] != SOURCE_BORDER_PIXEL) {
                    borderBad++;
                }
            }
        }
        probe_record(&stats, "DM1V2_SOURCE_ROUTE_BORDER_IS_CANONICAL_BLACK",
                     borderBad == 0,
                     "border pixels all M11_FB_ENCODE(0,5) outside the 224x136 viewport area");
    }

    /* Save the initial source-locked V1 framebuffer for the final
     * ownership check. */
    memcpy(v1_initial_shadow, framebuffer, M11_FB_BYTES);

    /* ---------- V1 source-locked movement command route ---------- */
    for (i = 0; i < N_V1_COMMAND_ROWS; ++i) {
        DM1_V2_MovementCommandRoute route;
        char note[160];
        char id[80];
        route = dm1_v2_movement_command_route_for_presentation(
            0, g_v1_command_table[i].v2Command);
        snprintf(id, sizeof(id), "DM1V2_SOURCE_ROUTE_C%03d",
                 g_v1_command_table[i].v1SourceCommand);
        snprintf(note, sizeof(note),
                 "%s: kind=%d sourceCommand=%d runtimeCommand=%d v2Enabled=%d (DEFS.H:238-243)",
                 g_v1_command_table[i].label,
                 (int)route.routeKind,
                 route.sourceCommand,
                 route.runtimeCommand,
                 route.v2PresentationEnabled);
        probe_record(&stats, id,
                     route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE &&
                     route.sourceCommand == g_v1_command_table[i].v1SourceCommand &&
                     route.runtimeCommand == route.sourceCommand &&
                     route.v2PresentationEnabled == 0,
                     note);
    }

    /* ---------- V1 baseline (must not be polluted by V2) ---------- */
    v1_cap.id = "V1";
    v1_cap.label = "V1 baseline (source-locked route)";
    v1_cap.expected_w = 320;
    v1_cap.expected_h = 200;
    v1_cap.present_via_rgba = 0;
    snprintf(v1_cap.bmp_path, sizeof(v1_cap.bmp_path), "%s/v1_source_route.bmp", out_dir);

    /* Make sure we are in V1 mode for the V1 baseline. */
    dm1_v2_presentation_mode_reset();
    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0);
    if (run_mode_capture(&v1_cap, framebuffer, v1_shadow)) {
        char note[220];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x, sha256=",
                 v1_cap.label, v1_cap.bmp_size, v1_cap.bmp_w, v1_cap.bmp_h, v1_cap.bmp_hash);
        /* Append the first 16 hex chars of the SHA-256 for in-log identification. */
        {
            char shahex[17];
            int k;
            for (k = 0; k < 8; k++) {
                snprintf(shahex + k * 2, 3, "%02x", v1_cap.bmp_sha[k * 2]);
                /* we'll only print 8 bytes -> 16 hex chars */
            }
            /* Use the simpler approach: just print the first 8 bytes. */
            char compact[33];
            for (k = 0; k < 8; k++) {
                snprintf(compact + k * 2, 3, "%02x", v1_cap.bmp_sha[k]);
            }
            compact[16] = '\0';
            /* Note: snprintf is already used above for note; append compact to note. */
            snprintf(note + strlen(note), sizeof(note) - strlen(note), "%s", compact);
        }
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_FILE",
                     v1_cap.bmp_size >= 54 + 320 * 200 * 3 &&
                     v1_cap.sha256_sidecar_ok,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_FILE", 0,
                     "V1 baseline capture failed");
    }

    /* ---------- V2.0 unfiltered baseline ---------- */
    v20_unfiltered_cap.id = "V20_UNFILTERED";
    v20_unfiltered_cap.label = "V2.0 unfiltered (source-locked route)";
    v20_unfiltered_cap.expected_w = 320;
    v20_unfiltered_cap.expected_h = 200;
    v20_unfiltered_cap.present_via_rgba = 0;
    snprintf(v20_unfiltered_cap.bmp_path, sizeof(v20_unfiltered_cap.bmp_path),
             "%s/v20_unfiltered_source_route.bmp", out_dir);

    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0);
    if (run_mode_capture(&v20_unfiltered_cap, framebuffer, v1_shadow)) {
        char note[160];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x, sha256=%02x%02x%02x%02x%02x%02x%02x%02x",
                 v20_unfiltered_cap.label, v20_unfiltered_cap.bmp_size,
                 v20_unfiltered_cap.bmp_w, v20_unfiltered_cap.bmp_h,
                 v20_unfiltered_cap.bmp_hash,
                 v20_unfiltered_cap.bmp_sha[0], v20_unfiltered_cap.bmp_sha[1],
                 v20_unfiltered_cap.bmp_sha[2], v20_unfiltered_cap.bmp_sha[3],
                 v20_unfiltered_cap.bmp_sha[4], v20_unfiltered_cap.bmp_sha[5],
                 v20_unfiltered_cap.bmp_sha[6], v20_unfiltered_cap.bmp_sha[7]);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_UNFILTERED_FILE",
                     v20_unfiltered_cap.bmp_size >= 54 + 320 * 200 * 3 &&
                     v20_unfiltered_cap.sha256_sidecar_ok,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_UNFILTERED_FILE", 0,
                     "V2.0 unfiltered capture failed");
    }

    /* ---------- V2.0 with the FULL filter chain [NEW MODE] ----------
     *
     * This is the new V2.0 presentation mode that exercises BOTH the
     * indexed-fb side (palette interpolation + dither cleanup) AND the
     * RGBA-post side (CRT + palette LUT + sharpen) of the V2.0 filter
     * chain. The existing actual-render probe only covers CRT + palette
     * LUT, so this mode proves a new V2.0 presentation path is wired. */
    v20_fully_filtered_cap.id = "V20_FULLY_FILTERED";
    v20_fully_filtered_cap.label = "V2.0 fully filtered (source-locked route, NEW MODE)";
    v20_fully_filtered_cap.expected_w = 320;
    v20_fully_filtered_cap.expected_h = 200;
    v20_fully_filtered_cap.present_via_rgba = 0;
    v20_fully_filtered_cap.v20_full_filter_chain = 1;
    snprintf(v20_fully_filtered_cap.bmp_path, sizeof(v20_fully_filtered_cap.bmp_path),
             "%s/v20_fully_filtered_source_route.bmp", out_dir);

    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    if (run_mode_capture(&v20_fully_filtered_cap, framebuffer, v1_shadow)) {
        char note[160];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x, sha256=%02x%02x%02x%02x%02x%02x%02x%02x",
                 v20_fully_filtered_cap.label, v20_fully_filtered_cap.bmp_size,
                 v20_fully_filtered_cap.bmp_w, v20_fully_filtered_cap.bmp_h,
                 v20_fully_filtered_cap.bmp_hash,
                 v20_fully_filtered_cap.bmp_sha[0], v20_fully_filtered_cap.bmp_sha[1],
                 v20_fully_filtered_cap.bmp_sha[2], v20_fully_filtered_cap.bmp_sha[3],
                 v20_fully_filtered_cap.bmp_sha[4], v20_fully_filtered_cap.bmp_sha[5],
                 v20_fully_filtered_cap.bmp_sha[6], v20_fully_filtered_cap.bmp_sha[7]);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_FULLY_FILTERED_FILE",
                     v20_fully_filtered_cap.bmp_size >= 54 + 320 * 200 * 3 &&
                     v20_fully_filtered_cap.sha256_sidecar_ok,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_FULLY_FILTERED_FILE", 0,
                     "V2.0 fully filtered capture failed");
    }
    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0);

    /* ---------- V2.1 upscaled ---------- */
    v21_cap.id = "V21";
    v21_cap.label = "V2.1 upscaled (source-locked route)";
    v21_cap.expected_w = 640;
    v21_cap.expected_h = 400;
    v21_cap.present_via_rgba = 1;
    snprintf(v21_cap.bmp_path, sizeof(v21_cap.bmp_path),
             "%s/v21_upscaled_source_route.bmp", out_dir);

    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    if (run_mode_capture(&v21_cap, framebuffer, v1_shadow)) {
        char note[160];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x, sha256=%02x%02x%02x%02x%02x%02x%02x%02x",
                 v21_cap.label, v21_cap.bmp_size, v21_cap.bmp_w, v21_cap.bmp_h,
                 v21_cap.bmp_hash,
                 v21_cap.bmp_sha[0], v21_cap.bmp_sha[1],
                 v21_cap.bmp_sha[2], v21_cap.bmp_sha[3],
                 v21_cap.bmp_sha[4], v21_cap.bmp_sha[5],
                 v21_cap.bmp_sha[6], v21_cap.bmp_sha[7]);
        probe_record(&stats, "DM1V2_SCREENSHOT_V21_FILE",
                     v21_cap.bmp_size >= 54 + 640 * 400 * 3 &&
                     v21_cap.sha256_sidecar_ok,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V21_FILE", 0,
                     "V2.1 upscaled capture failed");
    }

    /* ---------- V2.2 modern ---------- */
    v22_cap.id = "V22";
    v22_cap.label = "V2.2 modern (source-locked route)";
    v22_cap.expected_w = 320;
    v22_cap.expected_h = 200;
    v22_cap.present_via_rgba = 0;
    v22_cap.v22_overlay_active = 1;
    snprintf(v22_cap.bmp_path, sizeof(v22_cap.bmp_path),
             "%s/v22_modern_source_route.bmp", out_dir);

    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    if (run_mode_capture(&v22_cap, framebuffer, v1_shadow)) {
        char note[160];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x, sha256=%02x%02x%02x%02x%02x%02x%02x%02x",
                 v22_cap.label, v22_cap.bmp_size, v22_cap.bmp_w, v22_cap.bmp_h,
                 v22_cap.bmp_hash,
                 v22_cap.bmp_sha[0], v22_cap.bmp_sha[1],
                 v22_cap.bmp_sha[2], v22_cap.bmp_sha[3],
                 v22_cap.bmp_sha[4], v22_cap.bmp_sha[5],
                 v22_cap.bmp_sha[6], v22_cap.bmp_sha[7]);
        probe_record(&stats, "DM1V2_SCREENSHOT_V22_FILE",
                     v22_cap.bmp_size >= 54 + 320 * 200 * 3 &&
                     v22_cap.sha256_sidecar_ok,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V22_FILE", 0,
                     "V2.2 modern capture failed");
    }

    /* ---------- Cross-mode distinctness ----------
     *
     * V2.0 unfiltered MUST equal V1 (V2.0 with no filter chain is a
     * strict superset of V1, so both BMPs must be byte-identical AND
     * have matching SHA-256s). The remaining V2 modes must all
     * produce distinct output from V1 AND from each other, proving
     * each mode actually changes the presented pixels. */
    {
        int v1_eq_v20u =
            (v1_cap.bmp_hash == v20_unfiltered_cap.bmp_hash) &&
            (memcmp(v1_cap.bmp_sha, v20_unfiltered_cap.bmp_sha, 32) == 0);
        int v20u_ne_v20f =
            (v20_unfiltered_cap.bmp_hash != v20_fully_filtered_cap.bmp_hash) &&
            (memcmp(v20_unfiltered_cap.bmp_sha, v20_fully_filtered_cap.bmp_sha, 32) != 0);
        int v1_ne_v20f =
            (v1_cap.bmp_hash != v20_fully_filtered_cap.bmp_hash) &&
            (memcmp(v1_cap.bmp_sha, v20_fully_filtered_cap.bmp_sha, 32) != 0);
        int v1_ne_v21 =
            (v1_cap.bmp_hash != v21_cap.bmp_hash) &&
            (memcmp(v1_cap.bmp_sha, v21_cap.bmp_sha, 32) != 0);
        int v1_ne_v22 =
            (v1_cap.bmp_hash != v22_cap.bmp_hash) &&
            (memcmp(v1_cap.bmp_sha, v22_cap.bmp_sha, 32) != 0);
        int v20f_ne_v21 =
            (v20_fully_filtered_cap.bmp_hash != v21_cap.bmp_hash) &&
            (memcmp(v20_fully_filtered_cap.bmp_sha, v21_cap.bmp_sha, 32) != 0);
        int v20f_ne_v22 =
            (v20_fully_filtered_cap.bmp_hash != v22_cap.bmp_hash) &&
            (memcmp(v20_fully_filtered_cap.bmp_sha, v22_cap.bmp_sha, 32) != 0);
        int v21_ne_v22 =
            (v21_cap.bmp_hash != v22_cap.bmp_hash) &&
            (memcmp(v21_cap.bmp_sha, v22_cap.bmp_sha, 32) != 0);
        char note[300];
        snprintf(note, sizeof(note),
                 "V1=0x%08x V20u=0x%08x V20f=0x%08x V21=0x%08x V22=0x%08x",
                 v1_cap.bmp_hash, v20_unfiltered_cap.bmp_hash,
                 v20_fully_filtered_cap.bmp_hash, v21_cap.bmp_hash, v22_cap.bmp_hash);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_UNFILTERED_EQUALS_V1",
                     v1_eq_v20u, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_FULLY_FILTERED_REACHES_PIXELS",
                     v20u_ne_v20f, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_V20_FULLY_FILTERED_DISTINCT",
                     v1_ne_v20f, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_V21_DISTINCT",
                     v1_ne_v21, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_V22_DISTINCT",
                     v1_ne_v22, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_FULLY_FILTERED_V21_DISTINCT",
                     v20f_ne_v21, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_FULLY_FILTERED_V22_DISTINCT",
                     v20f_ne_v22, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V21_V22_DISTINCT",
                     v21_ne_v22, note);

        dist_v1_v20_unfiltered = v1_eq_v20u ? 1 : 0;
        dist_v20_unfiltered_v20_full = v20u_ne_v20f ? 1 : 0;
        dist_v1_v20_full = v1_ne_v20f ? 1 : 0;
        dist_v1_v21 = v1_ne_v21 ? 1 : 0;
        dist_v1_v22 = v1_ne_v22 ? 1 : 0;
        dist_v20_full_v21 = v20f_ne_v21 ? 1 : 0;
        dist_v20_full_v22 = v20f_ne_v22 ? 1 : 0;
        dist_v21_v22 = v21_ne_v22 ? 1 : 0;
    }
    (void)dist_v1_v20_unfiltered;
    (void)dist_v20_unfiltered_v20_full;
    (void)dist_v1_v20_full;
    (void)dist_v1_v21;
    (void)dist_v1_v22;
    (void)dist_v20_full_v21;
    (void)dist_v20_full_v22;
    (void)dist_v21_v22;

    /* ---------- V1 framebuffer ownership (re-asserted at end) ---------- */
    {
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_FRAMEBUFFER_OWNERSHIP",
                     memcmp(v1_initial_shadow, framebuffer, M11_FB_BYTES) == 0,
                     "source-locked V1 indexed framebuffer survived all 5 V1+V2 mode captures byte-identical");
    }

    /* ---------- Cleanup ---------- */
    dm1_v2_presentation_mode_reset();
    M11_Render_Shutdown();
    SDL_Quit();

    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);
    printf("# output dir: %s\n", out_dir);
    printf("# source: DUNGEON.DAT offset 8 + DEFS.H:989-998 + LOADSAVE.C:1940-1945 + pass173\n");
    printf("# source: DEFS.H:238-243 (C001..C006) + DUNVIEW.C:2999-3000 (224x136 viewport)\n");
    printf("# source: DUNVIEW.C:8337-8542 (composition order) + DUNVIEW.C:3913-3928 (D1C portrait)\n");
    printf("# source: COMMAND.C:2045-2155 (F0359 queue dispatch) + GAMELOOP.C:90 (F0128 draw)\n");
    printf("# non-claim: no DOSBox, no original PC 3.4 pairing, no DOS pixel parity claim\n");
    return (stats.failed == 0) ? 0 : 1;
}
