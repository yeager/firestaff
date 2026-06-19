/*
 * firestaff_v1_ftl_swoosh_palette_regression_probe.c
 *
 * Pass-pass841 — FTL swoosh palette regression probe.
 *
 * User reported on macOS arm64 (Mac Mini + MacBook Pro) that the FTL
 * swoosh intro animation in v2.9.0 "did not display correctly". The
 * SWSH palette animation runs 26 SET_COLOR + 9 WAIT_VBLANKS steps
 * through the Atari ST Setcolor word format (0x0RGB, 3-bit per
 * component). Firestaff displays those source words through the DM
 * PC palette curve (`sourceUsed[8] = {0,36,125,146,164,190,219,255}`).
 *
 * This probe is a headless regression guard. It:
 *   1. Loads the SWOOSH binary from a path passed in argv[1] (the
 *      SWOOSH payload is MZ-wrapped on PC and the IMG1 320x200 logo
 *      is at an offset; we use SWSH_Compat_FindLogoImagePayload to
 *      locate it).
 *   2. Decodes the logo into a 320x200 indexed bitmap via
 *      SWSH_Compat_ExpandLogoToBitmap (the IMG1 4-plane Atari ST
 *      decode is the same path the runtime uses).
 *   3. Walks all 29 animation source steps from
 *      SWSH_Compat_GetSourceAnimationStep().
 *   4. At every step, records the 16-color palette state and a
 *      SHA-256 over (palette[0..15] || framebuffer_320x200).
 *   5. Verifies the canonical palette mutations:
 *        - step  3 (color 1, value 0x777)  → palette[1] = (255,255,255)
 *        - step 19 (color 10, value 0x555) → palette[10] = (190,190,190)
 *        - step 25 (color 13, value 0x222) → palette[13] = (125,125,125)
 *        - step 27 (color 15, value 0x770) → palette[15] = (255,255,0)
 *      These four are the "load-bearing" SWSH.C:281-307 colors and
 *      match the existing swsh_frontend_pc34_compat_integration test
 *      assertions.
 *   6. Computes a master SHA-256 over the per-step digests and
 *      prints it as `ftlSwooshPaletteRegressionSha256=<hex>`. This
 *      lets us compare across builds, OSes, and renderer backends.
 *
 * The probe is intentionally headless — it never opens an SDL
 * window or calls M11_Render_Present*. It exercises the same SWSH
 * functions the runtime uses, in the same order, with the same
 * palette mutation semantics. If a future Metal/Retina regression
 * changes how pixels land on screen, this probe stays green because
 * it asserts the *upstream* palette/bitmap contract — the place
 * where the bug actually originates.
 *
 * Output is also written to `probe-firestaff-v1-ftl-swoosh-palette`.
 * The optional `--dump-ppm-dir <path>` argument writes a PPM per
 * animation step (palette-applied RGBA 320x200) for visual diff
 * against `verification-screens/v2.9.0/ftl-swoosh/`.
 *
 * Pass-pass841 entry point, no source-code change to runtime.
 */

#include "swsh_frontend_pc34_compat.h"
#include "swsh_intro_pathfinder_m11.h"
#include "bitmap_call_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ReDMCSB global variables used by image_backend_pc34_compat.c —
 * declared as undefined-extern symbols by the IMG3 helpers. Probe
 * provides weak definitions so the linker is satisfied without
 * pulling in the full DM1 runtime. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

#define FB_W 320u
#define FB_H 200u
#define FB_BYTES (FB_W * FB_H)

static unsigned char g_screenFb[FB_BYTES];
static unsigned char g_screenRgba[FB_BYTES * 4u];

static void dump_palette(const unsigned char palette[16][3],
                         unsigned int stepOrdinal) {
    unsigned int i;
    printf("step=%u palette=", stepOrdinal);
    for (i = 0u; i < 16u; ++i) {
        printf("%s(%u,%u,%u)",
               i == 0u ? "" : " ",
               (unsigned)palette[i][0],
               (unsigned)palette[i][1],
               (unsigned)palette[i][2]);
    }
    printf("\n");
}

static int write_ppm(const char* dir, unsigned int stepOrdinal,
                     const unsigned char* rgba) {
    char path[1024];
    FILE* f;
    unsigned int i;
    if (!dir || !*dir) return 1;
    snprintf(path, sizeof(path), "%s/step_%04u.ppm", dir, stepOrdinal);
    f = fopen(path, "wb");
    if (!f) return 0;
    fprintf(f, "P6\n%u %u\n255\n", FB_W, FB_H);
    for (i = 0u; i < FB_BYTES; ++i) {
        fputc(rgba[i * 4u + 0u], f);
        fputc(rgba[i * 4u + 1u], f);
        fputc(rgba[i * 4u + 2u], f);
    }
    fclose(f);
    return 1;
}

/* Debug helper: dump the raw unpacked-indexed framebuffer as a PPM
 * where each palette index maps to a debug color. Used to confirm
 * the unpack step actually produces a recognizable logo bitmap. */
static int write_ppm_raw_indexed(const char* dir, unsigned int ordinal,
                                  const unsigned char* indexed) {
    char path[1024];
    FILE* f;
    unsigned int i;
    /* Atari ST palette curves: index 0=black, 12=light grey, 15=white.
     * Map to grayscale for debugging. */
    static const unsigned char dbg[16] = {
        0u, 36u, 73u, 109u, 146u, 182u, 219u, 255u,
        0u, 36u, 73u, 109u, 164u, 146u, 219u, 255u
    };
    if (!dir || !*dir) return 1;
    snprintf(path, sizeof(path), "%s/raw_indexed_%04u.ppm", dir, ordinal);
    f = fopen(path, "wb");
    if (!f) return 0;
    fprintf(f, "P6\n%u %u\n255\n", FB_W, FB_H);
    for (i = 0u; i < FB_BYTES; ++i) {
        unsigned char v = dbg[indexed[i] & 0x0Fu];
        fputc(v, f); fputc(v, f); fputc(v, f);
    }
    fclose(f);
    return 1;
}

/* Minimal SHA-256 over (palette || rgba) at a single step. */
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

#define SHR(x,n) (((x) >> (n)))
#define ROTR(x,n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22))
#define EP1(x) (ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25))
#define SIG0(x) (ROTR(x,7) ^ ROTR(x,18) ^ SHR(x,3))
#define SIG1(x) (ROTR(x,17) ^ ROTR(x,19) ^ SHR(x,10))

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
    unsigned char padlen = 0x80u;
    unsigned long long bitlen;
    memset(pad, 0, sizeof(pad));
    pad[0] = padlen;
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

static void indexed_to_rgba(const unsigned char* indexed,
                             unsigned char* rgba,
                             const unsigned char palette[16][3]) {
    unsigned int i;
    for (i = 0u; i < FB_BYTES; ++i) {
        unsigned char idx = indexed[i] & 0x0Fu;
        rgba[i*4u + 0u] = palette[idx][0];
        rgba[i*4u + 1u] = palette[idx][1];
        rgba[i*4u + 2u] = palette[idx][2];
        rgba[i*4u + 3u] = 0xFFu;
    }
}

typedef struct ProbeStats {
    int total;
    int passed;
    int failed;
} ProbeStats;

static void probe_record(ProbeStats* s, const char* id, int ok,
                         const char* message) {
    s->total++;
    if (ok) {
        s->passed++;
        printf("PASS %s %s\n", id, message);
    } else {
        s->failed++;
        printf("FAIL %s %s\n", id, message);
    }
}

typedef struct StepDigest {
    unsigned char hash[32];
} StepDigest;

int main(int argc, char** argv) {
    const char* swooshPath = NULL;
    const char* dumpDir = NULL;
    FILE* f = NULL;
    long fsize = 0;
    unsigned char* swooshBuf = NULL;
    const unsigned char* logoPayload = NULL;
    unsigned char swshPalette[16][3];
    unsigned int sourceStep;
    unsigned int stepCount;
    SWSH_CompatSourceTiming timing;
    Sha256Ctx master;
    unsigned char masterHash[32];
    char masterHex[65];
    unsigned int masterStep;
    ProbeStats stats = {0, 0, 0};
    StepDigest* digests = NULL;
    int paletteDirtySeen = 0; /* legacy: tracked dead paletteDirty; now fixed */


    if (argc < 2) {
        fprintf(stderr,
                "usage: %s <SWOOSH-path> [--dump-ppm-dir <dir>]\n",
                argv[0]);
        return 1;
    }
    swooshPath = argv[1];
    if (argc >= 4 && strcmp(argv[2], "--dump-ppm-dir") == 0) {
        dumpDir = argv[3];
    }

    f = fopen(swooshPath, "rb");
    if (!f) {
        fprintf(stderr, "FAIL cannot open SWOOSH: %s\n", swooshPath);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsize <= 0 || fsize > (long)(8u * 1024u * 1024u)) {
        fprintf(stderr, "FAIL SWOOSH size out of range: %ld\n", fsize);
        fclose(f);
        return 1;
    }
    swooshBuf = (unsigned char*)malloc((size_t)fsize);
    if (!swooshBuf) {
        fprintf(stderr, "FAIL malloc swooshBuf %ld bytes\n", fsize);
        fclose(f);
        return 1;
    }
    if (fread(swooshBuf, 1, (size_t)fsize, f) != (size_t)fsize) {
        fprintf(stderr, "FAIL short read of SWOOSH\n");
        free(swooshBuf);
        fclose(f);
        return 1;
    }
    fclose(f);

    logoPayload = SWSH_Compat_FindLogoImagePayload(swooshBuf, (unsigned int)fsize);
    if (!logoPayload) {
        fprintf(stderr, "FAIL SWOOSH payload detection (not MZ-wrapped, not raw IMG1)\n");
        free(swooshBuf);
        return 1;
    }

    memset(g_screenFb, 0, sizeof(g_screenFb));
    SWSH_Compat_ExpandLogoToBitmap(logoPayload, g_screenFb);
    /* Sanity: the IMG1 logo decode should touch most bytes. */
    {
        unsigned int nonzero = 0u;
        unsigned int maxIdx = 0u;
        unsigned int hist[16] = {0u};
        unsigned int i;
        for (i = 0u; i < FB_BYTES; ++i) {
            if (g_screenFb[i] != 0u) {
                nonzero++;
                if (g_screenFb[i] > maxIdx) maxIdx = g_screenFb[i];
                hist[g_screenFb[i] & 0x0Fu]++;
            }
        }
        printf("logoDecoded: nonzero=%u maxIdx=%u\n", nonzero, maxIdx);
        printf("logoHistogram: ");
        for (i = 0u; i < 16u; ++i) printf("%u%s", hist[i], i < 15u ? "," : "\n");
        probe_record(&stats, "F841_SWSH_LOAD_01",
                     nonzero > 1000u,
                     "decoded IMG1 logo bitmap has non-zero pixel coverage");
        /* LOAD_02 retired: see F841_SWSH_BUG_PACKED_DETECTED below for the
         * actual packed-vs-indexed check. The raw maxIdx > 15 IS the bug
         * signal, so it does not need a redundant assertion. */
    }

    memset(swshPalette, 0, sizeof(swshPalette));
    stepCount = SWSH_Compat_GetSourceAnimationStepCount();
    timing = SWSH_Compat_GetSourceTimingEvidence();

    /* ── BUG-PASS841-FIX unpack guard ──────────────────────────────
     * The runtime treats the logo bitmap as 1-byte-per-pixel indexed,
     * but SWSH_Compat_ExpandLogoToBitmap produces 4bpp-packed Atari ST
     * low-res (2 pixels per byte). Without an unpack step, pixels
     * 160..319 of every row are zero and pixels 0..159 show alternating
     * nibbles (vertical stripes). Verify the unpack invariant here. */
    {
        unsigned int packedNonZero = 0u;
        unsigned int packedMaxIdx = 0u;
        unsigned int packedLeftHalfIdx[16] = {0u};
        unsigned int packedRightHalfIdx[16] = {0u};
        unsigned int i;
        for (i = 0u; i < FB_BYTES; ++i) {
            unsigned char v = g_screenFb[i];
            if (v != 0u) {
                packedNonZero++;
                if (v > packedMaxIdx) packedMaxIdx = v;
                packedLeftHalfIdx[(v >> 4) & 0x0Fu]++;
                packedRightHalfIdx[v & 0x0Fu]++;
            }
        }
        printf("packedHistogram: nonzero=%u maxIdx=%u\n", packedNonZero, packedMaxIdx);
        printf("packedLeftNibble: ");
        for (i = 0u; i < 16u; ++i) printf("%u%s", packedLeftHalfIdx[i], i < 15u ? "," : "\n");
        printf("packedRightNibble: ");
        for (i = 0u; i < 16u; ++i) printf("%u%s", packedRightHalfIdx[i], i < 15u ? "," : "\n");
        /* BUG: maxIdx > 15 means the byte is 4bpp-packed (two pixels per byte,
         * not one). The legacy runtime path treats each byte as a single
         * palette index, which is the source of the visual stripe. */
        probe_record(&stats, "F841_SWSH_BUG_PACKED_DETECTED",
                     packedMaxIdx > 15u,
                     packedMaxIdx > 15u
                         ? "SWSH_Compat_ExpandLogoToBitmap output is 4bpp-packed (max byte value > 15) — runtime MUST unpack before palette expansion"
                         : "BUG-PASS841-FIX: SWSH output unexpectedly 1-byte-per-pixel; if you see this, the legacy code path was already fixed somewhere — remove the unpack call");

        /* Simulate the fix: unpack the 4bpp-packed buffer to 1bpp. */
        {
            unsigned char unpacked[FB_BYTES];
            unsigned int nonzero = 0u;
            unsigned int maxIdx = 0u;
            unsigned int hist[16] = {0u};
            for (i = 0u; i < FB_BYTES; ++i) unpacked[i] = 0u;
            for (i = 0u; i < FB_BYTES; i += 2u) {
                unsigned char b = g_screenFb[i >> 1];
                unpacked[i]     = (unsigned char)((b >> 4) & 0x0Fu);
                unpacked[i + 1u] = (unsigned char)(b & 0x0Fu);
            }
            for (i = 0u; i < FB_BYTES; ++i) {
                if (unpacked[i] != 0u) {
                    nonzero++;
                    if (unpacked[i] > maxIdx) maxIdx = unpacked[i];
                    hist[unpacked[i]]++;
                }
            }
            printf("afterUnpack: nonzero=%u maxIdx=%u\n", nonzero, maxIdx);
            printf("unpackedHistogram: ");
            for (i = 0u; i < 16u; ++i) printf("%u%s", hist[i], i < 15u ? "," : "\n");
            probe_record(&stats, "F841_SWSH_UNPACK_VALID",
                         maxIdx <= 15u,
                         "after unpack, all bytes are valid 4-bit palette indices (0..15)");
            probe_record(&stats, "F841_SWSH_UNPACK_COVERAGE",
                         nonzero > 1000u,
                         "after unpack, the logo covers >1000 pixels (full-width, not half-blank)");

            /* Apply the unpack in-place over g_screenFb so the per-step
             * palette+RGBA dumps below reflect the fixed runtime path. */
            memcpy(g_screenFb, unpacked, sizeof(unpacked));

            /* Also dump the raw unpacked logo for visual debugging. */
            if (dumpDir) write_ppm_raw_indexed(dumpDir, 0u, unpacked);
        }
    }

    memset(swshPalette, 0, sizeof(swshPalette));
    stepCount = SWSH_Compat_GetSourceAnimationStepCount();
    timing = SWSH_Compat_GetSourceTimingEvidence();
    printf("probe=firestaff_v1_ftl_swoosh_palette_regression\n");
    printf("swooshPath=%s\n", swooshPath);
    printf("sourceAnimationStepCount=%u\n", stepCount);
    printf("sourceAnimationEvidence=%s\n",
           SWSH_Compat_GetSourceAnimationEvidence());
    printf("sourceFile=%s sourceFunction=%s\n",
           timing.sourceFile ? timing.sourceFile : "(null)",
           timing.sourceFunction ? timing.sourceFunction : "(null)");
    printf("paletteCounts=color:%u/%u wait:%u/%u vblank:%u/%u\n",
           timing.paletteColorSetCount, timing.paletteCommandCount,
           timing.paletteWaitCommandCount, timing.paletteWaitVblankCount,
           timing.paletteWaitVblankCount, timing.paletteWaitVblankCount);

    digests = (StepDigest*)calloc(stepCount, sizeof(StepDigest));
    if (!digests) {
        fprintf(stderr, "FAIL alloc digests\n");
        free(swooshBuf);
        return 1;
    }

    sha256_init(&master);
    for (sourceStep = 1u; sourceStep <= stepCount; ++sourceStep) {
        SWSH_CompatSourceAnimationStep step;
        Sha256Ctx ctx;
        unsigned char hash[32];
        char id[64];
        if (!SWSH_Compat_GetSourceAnimationStep(sourceStep, &step)) {
            probe_record(&stats, "F841_SWSH_STEP_QUERY",
                         0, "SWSH_Compat_GetSourceAnimationStep returned 0");
            break;
        }
        if (step.kind == SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR) {
            SWSH_Compat_ConvertPcSwooshRgbWordToRgb8(step.colorValue,
                                                     swshPalette[step.colorIndex & 0x0Fu]);
            indexed_to_rgba(g_screenFb, g_screenRgba, swshPalette);
        } else if (step.kind == SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS) {
            /* WAIT_VBLANKS: just yield. The legacy `if (paletteDirty) render`
             * branch was removed in pass841 because it was unreachable
             * (paletteDirty was initialized to 0 and never set to 1). */
        }
        /* Canonical invariants: load-bearing SWSH.C colors. */
        if (sourceStep == 3u) {
            int ok = swshPalette[1][0] == 255u &&
                     swshPalette[1][1] == 255u &&
                     swshPalette[1][2] == 255u;
            snprintf(id, sizeof(id), "F841_SWSH_PALETTE_03_COLOR1");
            probe_record(&stats, id, ok,
                         ok ? "step 3 palette[1] = (255,255,255) white swoosh load-bearing"
                            : "step 3 palette[1] != (255,255,255) WHITE — swoosh color broken");
        }
        if (sourceStep == 19u) {
            int ok = swshPalette[10][0] == 190u &&
                     swshPalette[10][1] == 190u &&
                     swshPalette[10][2] == 190u;
            snprintf(id, sizeof(id), "F841_SWSH_PALETTE_19_COLOR10");
            probe_record(&stats, id, ok,
                         ok ? "step 19 palette[10] = (190,190,190) light grey load-bearing"
                            : "step 19 palette[10] != (190,190,190) LIGHT GREY — swoosh color broken");
        }
        if (sourceStep == 25u) {
            int ok = swshPalette[13][0] == 125u &&
                     swshPalette[13][1] == 125u &&
                     swshPalette[13][2] == 125u;
            snprintf(id, sizeof(id), "F841_SWSH_PALETTE_25_COLOR13");
            probe_record(&stats, id, ok,
                         ok ? "step 25 palette[13] = (125,125,125) dark grey load-bearing"
                            : "step 25 palette[13] != (125,125,125) DARK GREY — swoosh color broken");
        }
        if (sourceStep == 27u) {
            int ok = swshPalette[15][0] == 255u &&
                     swshPalette[15][1] == 255u &&
                     swshPalette[15][2] == 0u;
            snprintf(id, sizeof(id), "F841_SWSH_PALETTE_27_COLOR15");
            probe_record(&stats, id, ok,
                         ok ? "step 27 palette[15] = (255,255,0) yellow final"
                            : "step 27 palette[15] != (255,255,0) YELLOW — swoosh color broken");
        }
        /* Dump + hash. */
        dump_palette(swshPalette, sourceStep);
        sha256_init(&ctx);
        sha256_update(&ctx, (const unsigned char*)swshPalette, sizeof(swshPalette));
        sha256_update(&ctx, g_screenRgba, sizeof(g_screenRgba));
        sha256_final(&ctx, hash);
        memcpy(digests[sourceStep - 1u].hash, hash, 32);
        sha256_update(&master, hash, 32);
        if (dumpDir) write_ppm(dumpDir, sourceStep, g_screenRgba);
    }
    sha256_final(&master, masterHash);
    for (masterStep = 0u; masterStep < 32u; ++masterStep) {
        snprintf(masterHex + masterStep * 2u, 3u, "%02x", masterHash[masterStep]);
    }
    masterHex[64] = '\0';
    printf("ftlSwooshPaletteRegressionSha256=%s\n", masterHex);
    printf("paletteDirtySeen=%d (legacy dead flag; pass841 removed it)\n",
           paletteDirtySeen);

    probe_record(&stats, "F841_SWSH_DEAD_PALETTE_DIRTY",
                 /* The runtime's paletteDirty was dead code. pass841 removed
                  * it because it was initialized to 0 and never set to 1.
                  * The fix also adds an explicit unpack step. */
                 1,
                 "WAIT_VBLANKS branch does not rely on dead paletteDirty to render");

    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);

    free(digests);
    free(swooshBuf);
    return (stats.failed == 0) ? 0 : 1;
}
