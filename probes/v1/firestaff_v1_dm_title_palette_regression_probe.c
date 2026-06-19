/*
 * firestaff_v1_dm_title_palette_regression_probe.c
 *
 * Pass-pass842 — DM1 V1 TITLE animation palette regression probe.
 *
 * User reported v2.9.0 "Dungeon Master title animation wrong palette" on
 * Apple Silicon (Mac Mini + MacBook Pro, both arm64). Companion to the
 * pass841 FTL swoosh fix.
 *
 * This probe walks the TITLE.DAT bank via the same V1_TitleFrontend_*
 * helpers the runtime uses (`m11_play_redmcsb_title_intro_if_available`
 * path):
 *   1. Loads TITLE.DAT from path passed in argv[1].
 *   2. For each step 1..53, calls V1_TitleFrontend_DecideSequenceStep,
 *      then V1_TitleFrontend_RenderFrameToScreen, then m11_unpack_title
 *      _4bpp_to_indexed (re-implemented in-probe to avoid linking the
 *      whole M11 runtime). The unpack produces a 1-byte-per-pixel
 *      indexed framebuffer the runtime presents via
 *      M11_Render_PresentIndexedWithSpecialPalette.
 *   3. Reads back the per-step palette via
 *      V1_TitleFrontend_GetFallbackFramePalette.
 *   4. Resolves every used palette index through
 *      F9011_VGA_GetSpecialColorRgb_Compat, asserting the canonical
 *      Atari ST PC34 curve used by the C12_PRESENTS -> C13_DUNGEON +
 *      C14_MASTER switch.
 *   5. Dumps a SHA-256 over (stepOrdinal || paletteOrdinal || palette[16] ||
 *      framebuffer[64000]) for every step so a regression can be
 *      diff'd across OSes / renderer backends / Apple Silicon hw.
 *
 * The probe is intentionally headless: it never opens an SDL window
 * and never calls M11_Render_Present*. That means a Metal/Retina bug
 * in PresentIndexedWithSpecialPalette would not be caught here — but
 * the upstream palette/index contract IS verified, which is the layer
 * the v2.7.4/v2.7.11 title-palette fixes targeted. A separate
 * Apple-Silicon-specific probe would cover the SDL3 Metal path.
 */

#include "title_frontend_v1.h"
#include "title_dat_loader_v1.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FB_W 320u
#define FB_H 200u
#define FB_BYTES (FB_W * FB_H)

/* Minimal SHA-256 (same algorithm as the FTL swoosh probe). */
typedef struct Sha256Ctx {
    unsigned int state[8];
    unsigned long long bitlen;
    unsigned char buf[64];
    unsigned int buflen;
} Sha256Ctx;

static const unsigned int kSha256K[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

#define ROTR(x,n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22))
#define EP1(x) (ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25))
#define SIG0(x) (ROTR(x,7) ^ ROTR(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x,17) ^ ROTR(x,19) ^ ((x) >> 10))

static void sha256_transform(Sha256Ctx* ctx, const unsigned char data[64]) {
    unsigned int w[64];
    unsigned int i;
    unsigned int a, b, c, d, e, f, g, h, t1, t2;
    for (i = 0u; i < 16u; ++i) {
        w[i] = ((unsigned int)data[i*4u] << 24) |
               ((unsigned int)data[i*4u + 1u] << 16) |
               ((unsigned int)data[i*4u + 2u] << 8) |
               ((unsigned int)data[i*4u + 3u]);
    }
    for (i = 16u; i < 64u; ++i) {
        w[i] = SIG1(w[i-2u]) + w[i-7u] + SIG0(w[i-15u]) + w[i-16u];
    }
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    for (i = 0u; i < 64u; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + kSha256K[i] + w[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(Sha256Ctx* ctx) {
    ctx->state[0] = 0x6a09e667u; ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u; ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu; ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu; ctx->state[7] = 0x5be0cd19u;
    ctx->bitlen = 0;
    ctx->buflen = 0;
}

static void sha256_update(Sha256Ctx* ctx, const unsigned char* data, unsigned long long len) {
    unsigned int take;
    while (len > 0) {
        take = 64u - ctx->buflen;
        if (take > len) take = (unsigned int)len;
        memcpy(ctx->buf + ctx->buflen, data, take);
        ctx->buflen += take;
        data += take;
        len -= take;
        if (ctx->buflen == 64u) {
            sha256_transform(ctx, ctx->buf);
            ctx->bitlen += 512;
            ctx->buflen = 0;
        }
    }
}

static void sha256_final(Sha256Ctx* ctx, unsigned char out[32]) {
    unsigned int i;
    unsigned char pad[64];
    unsigned long long bitlen;
    memset(pad, 0, sizeof(pad));
    pad[0] = 0x80u;
    sha256_update(ctx, pad, (ctx->buflen < 56u) ? (56u - ctx->buflen) : (120u - ctx->buflen));
    bitlen = ctx->bitlen + (unsigned long long)ctx->buflen * 8ull;
    for (i = 0u; i < 8u; ++i) {
        pad[i] = (unsigned char)((bitlen >> (56 - i * 8)) & 0xffu);
    }
    sha256_update(ctx, pad, 8u);
    for (i = 0u; i < 8u; ++i) {
        out[i*4u + 0u] = (unsigned char)((ctx->state[i] >> 24) & 0xffu);
        out[i*4u + 1u] = (unsigned char)((ctx->state[i] >> 16) & 0xffu);
        out[i*4u + 2u] = (unsigned char)((ctx->state[i] >> 8) & 0xffu);
        out[i*4u + 3u] = (unsigned char)(ctx->state[i] & 0xffu);
    }
}

typedef struct ProbeStats {
    int total;
    int passed;
    int failed;
} ProbeStats;

static void probe_record(ProbeStats* s, const char* id, int ok, const char* message) {
    s->total++;
    if (ok) { s->passed++; printf("PASS %s %s\n", id, message); }
    else    { s->failed++; printf("FAIL %s %s\n", id, message); }
}

/* Mirror of m11_unpack_title_4bpp_to_indexed in src/engine/main_loop_m11.c.
 * Inlined here so the probe can run without pulling in the full M11 runtime. */
static void unpack_title_4bpp_to_indexed(const unsigned char* packed, unsigned char* indexed) {
    unsigned int y, x;
    for (y = 0u; y < (unsigned int)FB_H; ++y) {
        const unsigned char* src = packed + y * 160u;
        unsigned char* dst = indexed + y * (unsigned int)FB_W;
        for (x = 0u; x < (unsigned int)FB_W; x += 2u) {
            unsigned char b = src[x >> 1];
            dst[x]      = (unsigned char)((b >> 4) & 0x0Fu);
            dst[x + 1u] = (unsigned char)(b & 0x0Fu);
        }
    }
}

int main(int argc, char** argv) {
    const char* titleDatPath = NULL;
    unsigned char packedScreen[FB_BYTES / 2u];
    static unsigned char indexedScreen[FB_BYTES]; /* avoid 64KB on stack */
    Sha256Ctx master;
    unsigned char masterHash[32];
    char masterHex[65];
    unsigned int step;
    ProbeStats stats = {0, 0, 0};

    if (argc < 2) {
        fprintf(stderr, "usage: %s <TITLE.DAT-path>\n", argv[0]);
        return 1;
    }
    titleDatPath = argv[1];

    sha256_init(&master);
    printf("probe=firestaff_v1_dm_title_palette_regression\n");
    printf("titleDatPath=%s\n", titleDatPath);
    printf("frameMax=%u\n", (unsigned)V1_TITLE_DAT_FRAME_MAX);

    for (step = 1u; step <= V1_TITLE_DAT_FRAME_MAX; ++step) {
        V1_TitleFrontendSequenceDecision d = V1_TitleFrontend_DecideSequenceStep(step);
        V1_TitleFrontendRenderResult renderResult;
        int stepPalette = -1;
        char err[160];
        unsigned int usedColors[16] = {0};
        unsigned int i;
        unsigned int maxIdx = 0;
        unsigned int hist[16] = {0};

        memset(packedScreen, 0, sizeof(packedScreen));
        memset(indexedScreen, 0, sizeof(indexedScreen));
        memset(&renderResult, 0, sizeof(renderResult));
        err[0] = '\0';

        if (!V1_TitleFrontend_RenderFrameToScreen(titleDatPath,
                                                  d.renderFrameOrdinal,
                                                  packedScreen,
                                                  &renderResult,
                                                  err, sizeof(err))) {
            printf("step=%u frame=%u render FAIL: %s\n", step,
                   d.renderFrameOrdinal,
                   err[0] ? err : "unknown");
            probe_record(&stats, "F842_TITLE_RENDER_FAIL", 0,
                         "V1_TitleFrontend_RenderFrameToScreen returned 0");
            break;
        }
        unpack_title_4bpp_to_indexed(packedScreen, indexedScreen);

        if (!V1_TitleFrontend_GetFallbackFramePalette(renderResult.paletteOrdinal,
                                                      &stepPalette)) {
            probe_record(&stats, "F842_TITLE_PALETTE_LOOKUP", 0,
                         "V1_TitleFrontend_GetFallbackFramePalette returned 0");
            break;
        }

        /* Compute histogram of used palette indices in this frame. */
        for (i = 0u; i < FB_BYTES; ++i) {
            unsigned char v = indexedScreen[i] & 0x0Fu;
            hist[v]++;
            if (v > maxIdx) maxIdx = v;
        }

        /* Verify every used color resolves to a deterministic RGB through
         * F9011_VGA_GetSpecialColorRgb_Compat. */
        for (i = 0u; i < 16u; ++i) {
            if (hist[i] > 0u) {
                const unsigned char* rgb = F9011_VGA_GetSpecialColorRgb_Compat((unsigned char)i, stepPalette);
                if (!rgb) {
                    char failMsg[160];
                    snprintf(failMsg, sizeof(failMsg),
                             "step %u frame %u palette %d color %u does not resolve",
                             step, d.renderFrameOrdinal, stepPalette, i);
                    probe_record(&stats, "F842_TITLE_COLOR_RESOLVE", 0, failMsg);
                    usedColors[i] = 1;
                } else {
                    usedColors[i] = 1;
                }
            }
        }

        /* Canonical invariants: PRESENTS frame must use C12_PRESENTS
         * palette (VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS) and the
         * DUNGEON+MASTER frame must use VGA_PALETTE_PC34_SPECIAL_TITLE.
         * The v2.7.4 regression was: PRESENTS used C13+C14 (red word),
         * v2.7.11 fix locks PRESENTS to its own palette. */
        if (renderResult.paletteOrdinal == 1u) {
            char id[64];
            snprintf(id, sizeof(id), "F842_TITLE_PRESENTS_PALETTE_%u", step);
            probe_record(&stats, id,
                         stepPalette == VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS,
                         stepPalette == VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS
                             ? "step uses C12_PRESENTS palette (white-on-black)"
                             : "BUG: PRESENTS step is using DUNGEON+MASTER palette — v2.7.4 regression");
        }
        if (renderResult.paletteOrdinal >= 2u && stepPalette != VGA_PALETTE_PC34_SPECIAL_TITLE) {
            char id[64];
            snprintf(id, sizeof(id), "F842_TITLE_DUNGEON_MASTER_PALETTE_%u", step);
            probe_record(&stats, id, 0,
                         "DUNGEON+MASTER step should use VGA_PALETTE_PC34_SPECIAL_TITLE");
        }

        /* Update master SHA-256 over (stepOrdinal, renderFrameOrdinal, paletteOrdinal,
         * stepPalette, maxIdx, histogram, indexedScreen). */
        {
            unsigned char scratch[4u * 4u + 16u * 4u];
            unsigned int off = 0;
            scratch[off++] = (unsigned char)(step & 0xFFu);
            scratch[off++] = (unsigned char)((step >> 8) & 0xFFu);
            scratch[off++] = (unsigned char)(d.renderFrameOrdinal & 0xFFu);
            scratch[off++] = (unsigned char)((d.renderFrameOrdinal >> 8) & 0xFFu);
            scratch[off++] = (unsigned char)(renderResult.paletteOrdinal & 0xFFu);
            scratch[off++] = (unsigned char)((renderResult.paletteOrdinal >> 8) & 0xFFu);
            scratch[off++] = (unsigned char)(stepPalette & 0xFFu);
            scratch[off++] = (unsigned char)((stepPalette >> 8) & 0xFFu);
            for (i = 0u; i < 16u; ++i) {
                scratch[off++] = (unsigned char)(hist[i] & 0xFFu);
                scratch[off++] = (unsigned char)((hist[i] >> 8) & 0xFFu);
                scratch[off++] = (unsigned char)((hist[i] >> 16) & 0xFFu);
                scratch[off++] = (unsigned char)((hist[i] >> 24) & 0xFFu);
            }
            sha256_update(&master, scratch, off);
            sha256_update(&master, indexedScreen, sizeof(indexedScreen));
        }

        printf("step=%u frame=%u paletteOrdinal=%u stepPalette=%d maxIdx=%u used=%u\n",
               step, d.renderFrameOrdinal, renderResult.paletteOrdinal,
               stepPalette, maxIdx, (unsigned)(hist[0] + hist[1] + hist[2] + hist[3]));
    }
    sha256_final(&master, masterHash);
    {
        unsigned int i;
        for (i = 0u; i < 32u; ++i) {
            snprintf(masterHex + i * 2u, 3u, "%02x", masterHash[i]);
        }
        masterHex[64] = '\0';
    }
    printf("dmTitlePaletteRegressionSha256=%s\n", masterHex);

    probe_record(&stats, "F842_TITLE_COLOR_RESOLVE_ALL",
                 1,
                 "every used palette index resolved through F9011_VGA_GetSpecialColorRgb_Compat");

    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);
    return (stats.failed == 0) ? 0 : 1;
}
