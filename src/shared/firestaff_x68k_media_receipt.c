/*
 * firestaff_x68k_media_receipt.c
 *
 * Implementation of the skip-safe real-media receipt + classification
 * gate for the DM1 / CSB X68000 HDM media import lane.
 *
 * See include/firestaff_x68k_media_receipt.h for the public
 * contract, the known-hash list, and the source-of-truth
 * references (DMWeb DM X68000 + CSB X68000 edition pages,
 * DMWeb copy-protection Sharp X68000 section, the
 * X68000 media classifier, the FTL container parser).
 *
 * Implementation notes:
 *
 *   - SHA-256 and MD5 are self-contained public-domain
 *     implementations inlined below so the module does not
 *     depend on OpenSSL, CommonCrypto, or any other
 *     platform crypto library. The receipt module is small
 *     and only needs to compute one digest per file, so a
 *     few hundred lines of straightforward crypto is
 *     cheaper than dragging in a 2 MB dependency.
 *
 *   - The known-hash list is small and is hand-pinned to
 *     the public DMFiles / community preservation HDMs that
 *     DMWeb lists. Adding a new kind is intentionally a
 *     code change: we want the documented DMWeb vocabulary
 *     (Original / Cracked / Save Disk) to remain the
 *     contract.
 *
 *   - The receipt result is computed by
 *     firestaff_x68k_media_receipt_finalize() against the
 *     documented expected values. The probe consumes
 *     `result` directly; a non-zero value is a per-receipt
 *     failure and never aborts the whole scan.
 *
 *   - Virtual container paths (ZIP/ISO entries) are
 *     materialized into the local Firestaff asset cache
 *     exactly the way the existing hint-oracle real-scan
 *     module does it, via asset_extract_virtual_path().
 *     The materialized file is then read with ordinary
 *     stdio.
 */

#include "firestaff_x68k_media_receipt.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"

/* ── Known-hash list ─────────────────────────────────────────────── */

static const FirestaffX68kMediaReceipt_KnownHash g_known_hashes[] = {
    {
        FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_ORIGINAL,
        "dm1-v3.0-jp-original-not-working",
        "315c2d59edc7d394cf5418d5315c51d4",
        "4196715dfcbe3a0af78e309bb3e90eacd148d47f7041679c07e604cb44dc8c4b",
        1261568u,
        FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM,
        /* DMWeb: public Original lacks the HPR-0007 sector
         * and cannot boot on a real X68000. Byte-level
         * observation: the HPR-0007 sentinel is absent
         * (the only operational check) but the MFM
         * controller fills the protection-region bytes
         * with 0xE5 ("deleted data" mark), so the
         * PROTECTION_AREA_BLANK flag does NOT fire on the
         * real image. We only pin the bits the byte-level
         * classifier is documented to report. */
        (FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT |
         FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK |
         FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT),
        0u
    },
    {
        FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_CRACKED,
        "dm1-v3.0-jp-cracked",
        "25a36e1b69ece2a12f127ba90700f2ea",
        "b84c98619a680364defa7a7e79c213e585a99d45597e1b1681542d4735021bd1",
        1261568u,
        FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM,
        /* DMWeb: the crack patches DM.X to bypass the
         * runtime copy-protection check, but the HDM
         * itself is still a full 1232 KB image without
         * the HPR-0007 sentinel. Same byte-level
         * observation as Original: MFM 0xE5 fills the
         * protection region, no HPR-0007 sentinel. */
        (FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT |
         FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK |
         FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT),
        0u
    },
    {
        FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_SAVE_DISK,
        "dm1-v3.0-jp-save-disk",
        "1c8a79d48d6a694b0c8dfce188eb3707",
        "35eff1d7590fdf6cd225268e861f80bfbe09818f725e6fade1ddeb0d6b108675",
        1261568u,
        FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM,
        /* The real X68000-formatted save disk contains a
         * Human68k filesystem header at offset 0 and the
         * MFM controller fills the sector payloads with 0xE5
         * ("deleted data" mark), so the X68k classifier's
         * BLANK_SAVE_DISK and PROTECTION_AREA_BLANK flags do
         * NOT fire. We classify the save disk the same way
         * as Original / Cracked at the byte level: full 1232
         * KB, no HPR-0007 sentinel, no FTL magic at offset 0.
         * The "save disk" identity is recorded by the
         * documented MD5 + SHA-256 + path label, not by a
         * classifier flag. */
        (FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT |
         FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK |
         FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT),
        0u
    },
    {
        FIRESTAFF_X68K_RECEIPT_KIND_CSB_V31_JP_ORIGINAL,
        "csb-v3.1-jp-original-not-working",
        "96e83673cb1c74478954ca27517210bb",
        "3ba49658d47bf5afdccfb018e0e9eebba8346c40937e79d7589dbc8677828d77",
        1261568u,
        FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM,
        /* DMWeb: CSB shares the DM copy-protection scheme;
         * the public Original lacks the HPR-0007 sector.
         * Same byte-level observation: MFM 0xE5 fills the
         * protection region, no HPR-0007 sentinel. */
        (FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT |
         FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK |
         FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT),
        0u
    },
    {
        FIRESTAFF_X68K_RECEIPT_KIND_CSB_V31_JP_CRACKED,
        "csb-v3.1-jp-cracked",
        "0e02b372eab9dfe0c2061b13ade4774c",
        "e912addf1881b6c2b3cde4207507061a43459748082c75953cbc3c305fdf24e1",
        1261568u,
        FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM,
        /* DMWeb: the CSB crack is a separate "CK.R" program
         * but the byte-level shape matches the DM crack:
         * full 1232 KB, no HPR-0007 sentinel. */
        (FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT |
         FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK |
         FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT),
        0u
    },
    {
        FIRESTAFF_X68K_RECEIPT_KIND_CSB_V31_JP_SAVE_DISK,
        "csb-v3.1-jp-save-disk",
        "2e4550d675290491e2bee79072857676",
        "437ad38b66175f679605722e26cd112e6e473f33a7fe63db549743359788ce37",
        1261568u,
        FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM,
        /* Same X68000-formatted save disk observation as the
         * DM1 save disk: Human68k filesystem header + MFM
         * 0xE5 fills, no HPR-0007 sentinel, no FTL magic. */
        (FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT |
         FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK |
         FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT),
        0u
    }
};

const FirestaffX68kMediaReceipt_KnownHash *
firestaff_x68k_media_receipt_known_hashes(size_t *out_count)
{
    if (out_count) {
        *out_count = sizeof(g_known_hashes) / sizeof(g_known_hashes[0]);
    }
    return g_known_hashes;
}

/* ── Self-contained SHA-256 (FIPS 180-4) ─────────────────────────── */

typedef struct {
    uint32_t state[8];
    uint64_t bit_count;
    uint8_t  buf[64];
    size_t   buf_len;
} Sha256Ctx;

static const uint32_t k_sha256_k[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u,
    0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u, 0xe49b69c1u, 0xefbe4786u,
    0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
    0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u, 0xa2bfe8a1u, 0xa81a664bu,
    0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au,
    0x5b9cca4fu, 0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

#define SHA256_ROR(x, n) (((x) >> (n)) | ((x) << (32u - (n))))
#define SHA256_CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_BSIG0(x) (SHA256_ROR((x), 2) ^ SHA256_ROR((x), 13) ^ SHA256_ROR((x), 22))
#define SHA256_BSIG1(x) (SHA256_ROR((x), 6) ^ SHA256_ROR((x), 11) ^ SHA256_ROR((x), 25))
#define SHA256_SSIG0(x) (SHA256_ROR((x), 7) ^ SHA256_ROR((x), 18) ^ ((x) >> 3))
#define SHA256_SSIG1(x) (SHA256_ROR((x), 17) ^ SHA256_ROR((x), 19) ^ ((x) >> 10))

static void sha256_compress(Sha256Ctx *ctx, const uint8_t block[64])
{
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = ((uint32_t)block[i * 4 + 0] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8)  |
               ((uint32_t)block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        w[i] = SHA256_SSIG1(w[i - 2]) + w[i - 7] +
               SHA256_SSIG0(w[i - 15]) + w[i - 16];
    }
    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + SHA256_BSIG1(e) + SHA256_CH(e, f, g) +
                      k_sha256_k[i] + w[i];
        uint32_t t2 = SHA256_BSIG0(a) + SHA256_MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha256_init(Sha256Ctx *ctx)
{
    ctx->state[0] = 0x6a09e667u;
    ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u;
    ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu;
    ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu;
    ctx->state[7] = 0x5be0cd19u;
    ctx->bit_count = 0;
    ctx->buf_len = 0;
}

static void sha256_update(Sha256Ctx *ctx, const uint8_t *data, size_t len)
{
    ctx->bit_count += (uint64_t)len * 8u;
    while (len > 0) {
        size_t take = 64u - ctx->buf_len;
        if (take > len) take = len;
        memcpy(ctx->buf + ctx->buf_len, data, take);
        ctx->buf_len += take;
        data += take;
        len -= take;
        if (ctx->buf_len == 64u) {
            sha256_compress(ctx, ctx->buf);
            ctx->buf_len = 0;
        }
    }
}

static void sha256_final(Sha256Ctx *ctx, uint8_t out[32])
{
    /* Pad: 0x80, then zeros, then 64-bit big-endian bit count.
     * The bit count must be captured BEFORE we feed any
     * padding bytes through sha256_update(), otherwise the
     * final 8-byte length field would encode the bit count of
     * the padded input rather than the original message. */
    uint8_t pad[64];
    memset(pad, 0, sizeof(pad));
    pad[0] = 0x80u;
    size_t pad_len = (ctx->buf_len < 56u) ? (56u - ctx->buf_len)
                                          : (120u - ctx->buf_len);
    uint64_t bit_count = ctx->bit_count;
    sha256_update(ctx, pad, pad_len);
    uint8_t lenbuf[8];
    for (int i = 0; i < 8; ++i) {
        lenbuf[i] = (uint8_t)(bit_count >> (56 - 8 * i));
    }
    sha256_update(ctx, lenbuf, 8);
    for (int i = 0; i < 8; ++i) {
        out[i * 4 + 0] = (uint8_t)(ctx->state[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        out[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
    memset(ctx, 0, sizeof(*ctx));
}

int firestaff_x68k_media_receipt_sha256_hex(const uint8_t *data,
                                            size_t data_size,
                                            char *out_hex,
                                            size_t out_hex_cap)
{
    if (!out_hex || out_hex_cap < 65u) return -1;
    if (!data && data_size > 0u) return -1;
    Sha256Ctx ctx;
    sha256_init(&ctx);
    if (data_size > 0u) {
        sha256_update(&ctx, data, data_size);
    }
    uint8_t digest[32];
    sha256_final(&ctx, digest);
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        out_hex[i * 2 + 0] = hex[(digest[i] >> 4) & 0xfu];
        out_hex[i * 2 + 1] = hex[digest[i] & 0xfu];
    }
    out_hex[64] = '\0';
    return 0;
}

/* ── Self-contained MD5 (RFC 1321) ───────────────────────────────── */

typedef struct {
    uint32_t state[4];
    uint64_t bit_count;
    uint8_t  buf[64];
    size_t   buf_len;
} Md5Ctx;

#define MD5_F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | ~(z)))
#define MD5_RL(x, n) (((x) << (n)) | ((x) >> (32u - (n))))

static void md5_compress(Md5Ctx *ctx, const uint8_t block[64])
{
    static const uint32_t k_md5_k[64] = {
        0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu,
        0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
        0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu,
        0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
        0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau,
        0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
        0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu,
        0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
        0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu,
        0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
        0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u,
        0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
        0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u,
        0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
        0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u,
        0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u
    };
    static const uint32_t k_md5_s[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    uint32_t M[16];
    for (int i = 0; i < 16; ++i) {
        M[i] = ((uint32_t)block[i * 4 + 0]) |
               ((uint32_t)block[i * 4 + 1] << 8) |
               ((uint32_t)block[i * 4 + 2] << 16) |
               ((uint32_t)block[i * 4 + 3] << 24);
    }
    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    for (int i = 0; i < 64; ++i) {
        uint32_t f, g;
        if (i < 16) {
            f = MD5_F(b, c, d);
            g = i;
        } else if (i < 32) {
            f = MD5_G(b, c, d);
            g = (5u * (uint32_t)i + 1u) % 16u;
        } else if (i < 48) {
            f = MD5_H(b, c, d);
            g = (3u * (uint32_t)i + 5u) % 16u;
        } else {
            f = MD5_I(b, c, d);
            g = (7u * (uint32_t)i) % 16u;
        }
        uint32_t temp = d;
        d = c;
        c = b;
        b = b + MD5_RL(a + f + k_md5_k[i] + M[g], k_md5_s[i]);
        a = temp;
    }
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
}

static void md5_init(Md5Ctx *ctx)
{
    ctx->state[0] = 0x67452301u;
    ctx->state[1] = 0xefcdab89u;
    ctx->state[2] = 0x98badcfeu;
    ctx->state[3] = 0x10325476u;
    ctx->bit_count = 0;
    ctx->buf_len = 0;
}

static void md5_update(Md5Ctx *ctx, const uint8_t *data, size_t len)
{
    ctx->bit_count += (uint64_t)len * 8u;
    while (len > 0) {
        size_t take = 64u - ctx->buf_len;
        if (take > len) take = len;
        memcpy(ctx->buf + ctx->buf_len, data, take);
        ctx->buf_len += take;
        data += take;
        len -= take;
        if (ctx->buf_len == 64u) {
            md5_compress(ctx, ctx->buf);
            ctx->buf_len = 0;
        }
    }
}

static void md5_final(Md5Ctx *ctx, uint8_t out[16])
{
    uint8_t pad[64];
    memset(pad, 0, sizeof(pad));
    pad[0] = 0x80u;
    size_t pad_len = (ctx->buf_len < 56u) ? (56u - ctx->buf_len)
                                          : (120u - ctx->buf_len);
    /* Capture the bit count before we feed any padding
     * bytes through md5_update(); the trailing 8-byte
     * little-endian length field encodes the original
     * message length, not the padded one. */
    uint64_t bit_count = ctx->bit_count;
    md5_update(ctx, pad, pad_len);
    uint8_t lenbuf[8];
    for (int i = 0; i < 8; ++i) {
        lenbuf[i] = (uint8_t)(bit_count >> (8 * i));
    }
    md5_update(ctx, lenbuf, 8);
    for (int i = 0; i < 4; ++i) {
        out[i * 4 + 0] = (uint8_t)(ctx->state[i]);
        out[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 8);
        out[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 16);
        out[i * 4 + 3] = (uint8_t)(ctx->state[i] >> 24);
    }
    memset(ctx, 0, sizeof(*ctx));
}

int firestaff_x68k_media_receipt_md5_hex(const uint8_t *data,
                                          size_t data_size,
                                          char *out_hex,
                                          size_t out_hex_cap)
{
    if (!out_hex || out_hex_cap < 33u) return -1;
    if (!data && data_size > 0u) return -1;
    Md5Ctx ctx;
    md5_init(&ctx);
    if (data_size > 0u) {
        md5_update(&ctx, data, data_size);
    }
    uint8_t digest[16];
    md5_final(&ctx, digest);
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 16; ++i) {
        out_hex[i * 2 + 0] = hex[(digest[i] >> 4) & 0xfu];
        out_hex[i * 2 + 1] = hex[digest[i] & 0xfu];
    }
    out_hex[32] = '\0';
    return 0;
}

/* ── Result-name tables ──────────────────────────────────────────── */

const char *firestaff_x68k_media_receipt_kind_name(
    FirestaffX68kMediaReceipt_Kind kind)
{
    switch (kind) {
    case FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_ORIGINAL:
        return "dm1-v3.0-jp-original";
    case FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_CRACKED:
        return "dm1-v3.0-jp-cracked";
    case FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_SAVE_DISK:
        return "dm1-v3.0-jp-save-disk";
    case FIRESTAFF_X68K_RECEIPT_KIND_CSB_V31_JP_ORIGINAL:
        return "csb-v3.1-jp-original";
    case FIRESTAFF_X68K_RECEIPT_KIND_CSB_V31_JP_CRACKED:
        return "csb-v3.1-jp-cracked";
    case FIRESTAFF_X68K_RECEIPT_KIND_CSB_V31_JP_SAVE_DISK:
        return "csb-v3.1-jp-save-disk";
    case FIRESTAFF_X68K_RECEIPT_KIND_UNKNOWN:
    default:
        return "unknown";
    }
}

const char *firestaff_x68k_media_receipt_class_name(
    FirestaffX68kMediaReceipt_Class cls)
{
    switch (cls) {
    case FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM:
        return "unprotected-public-hdm";
    case FIRESTAFF_X68K_RECEIPT_CLASS_BLANK_SAVE_DISK:
        return "blank-save-disk";
    default:
        return "unknown";
    }
}

const char *firestaff_x68k_media_receipt_result_name(int result)
{
    switch (result) {
    case FIRESTAFF_X68K_RECEIPT_OK: return "OK";
    case FIRESTAFF_X68K_RECEIPT_ERR_ARGUMENT: return "argument";
    case FIRESTAFF_X68K_RECEIPT_ERR_NO_DATA_DIR: return "no-data-dir";
    case FIRESTAFF_X68K_RECEIPT_ERR_NOT_FOUND: return "not-found";
    case FIRESTAFF_X68K_RECEIPT_ERR_READ: return "read";
    case FIRESTAFF_X68K_RECEIPT_ERR_SIZE_MISMATCH: return "size-mismatch";
    case FIRESTAFF_X68K_RECEIPT_ERR_HASH_MISMATCH: return "hash-mismatch";
    case FIRESTAFF_X68K_RECEIPT_ERR_CLASS_UNEXPECTED: return "class-unexpected";
    default: return "unknown";
    }
}

const char *firestaff_x68k_media_receipt_media_class_name(uint32_t media_class)
{
    switch (media_class) {
    case FIRESTAFF_X68K_MEDIA_EMPTY:       return "empty";
    case FIRESTAFF_X68K_MEDIA_TOO_SMALL:   return "too-small";
    case FIRESTAFF_X68K_MEDIA_SINGLE_SIDE: return "single-side";
    case FIRESTAFF_X68K_MEDIA_FULL_DISK:   return "full-disk";
    case FIRESTAFF_X68K_MEDIA_OVERSIZE:    return "oversize";
    default:                                return "unknown";
    }
}

const char *firestaff_x68k_media_receipt_flag_name(uint32_t flag)
{
    if (flag == FIRESTAFF_X68K_SCAN_FLAG_NONE) return "none";
    if (flag == FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT) return "sentinel-present";
    if (flag == FIRESTAFF_X68K_SCAN_FLAG_PROTECTION_AREA_BLANK) return "protection-area-blank";
    if (flag == FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK) return "blank-save-disk";
    if (flag == FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT) return "ftl-present";
    return "unknown-flag";
}

/* ── String helpers ──────────────────────────────────────────────── */

static int copy_string(char *dst, size_t dst_size, const char *src)
{
    size_t len;
    if (!dst || dst_size == 0u) return 0;
    if (!src) {
        dst[0] = '\0';
        return 0;
    }
    len = strlen(src);
    if (len + 1u > dst_size) {
        dst[0] = '\0';
        return 0;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
    return 1;
}

static int is_virtual_path(const char *path)
{
    if (!path) return 0;
    return strstr(path, "::") != NULL;
}

/* ── Receipt lifecycle ──────────────────────────────────────────── */

void firestaff_x68k_media_receipt_init(FirestaffX68kMediaReceipt *r)
{
    if (!r) return;
    memset(r, 0, sizeof(*r));
    r->kind = FIRESTAFF_X68K_RECEIPT_KIND_UNKNOWN;
    r->media_class = FIRESTAFF_X68K_MEDIA_EMPTY;
}

int firestaff_x68k_media_receipt_resolve_cache_dir(
    const char *cache_dir,
    char *out_dir,
    size_t out_dir_cap)
{
    if (!out_dir || out_dir_cap < 16u) return -1;
    if (cache_dir && cache_dir[0] != '\0') {
        return copy_string(out_dir, out_dir_cap, cache_dir) ? 0 : -1;
    }
    const char *home = getenv("HOME");
    if (!home || home[0] == '\0') {
        out_dir[0] = '\0';
        return -1;
    }
    int n = snprintf(out_dir, out_dir_cap,
                     "%s/.firestaff/asset-cache/x68k-receipt", home);
    if (n <= 0 || (size_t)n >= out_dir_cap) {
        out_dir[0] = '\0';
        return -1;
    }
    return 0;
}

/* Find the known-hash entry for `kind`. Returns NULL when the
 * kind is not in the documented list. */
static const FirestaffX68kMediaReceipt_KnownHash *
find_known_hash(FirestaffX68kMediaReceipt_Kind kind)
{
    size_t n = 0;
    const FirestaffX68kMediaReceipt_KnownHash *table =
        firestaff_x68k_media_receipt_known_hashes(&n);
    for (size_t i = 0; i < n; ++i) {
        if (table[i].kind == kind) return &table[i];
    }
    return NULL;
}

int firestaff_x68k_media_receipt_ingest_path(
    FirestaffX68kMediaReceipt *r,
    const char *path,
    uint8_t **out_data,
    size_t   *out_data_size)
{
    if (out_data) *out_data = NULL;
    if (out_data_size) *out_data_size = 0u;
    if (!r || !path || !out_data || !out_data_size) {
        return FIRESTAFF_X68K_RECEIPT_ERR_ARGUMENT;
    }
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return FIRESTAFF_X68K_RECEIPT_ERR_READ;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return FIRESTAFF_X68K_RECEIPT_ERR_READ;
    }
    long end = ftell(fp);
    if (end < 0) {
        fclose(fp);
        return FIRESTAFF_X68K_RECEIPT_ERR_READ;
    }
    rewind(fp);
    size_t size = (size_t)end;
    uint8_t *buf = (uint8_t*)malloc(size > 0u ? size : 1u);
    if (!buf) {
        fclose(fp);
        return FIRESTAFF_X68K_RECEIPT_ERR_READ;
    }
    if (size > 0u) {
        size_t got = fread(buf, 1, size, fp);
        if (got != size) {
            free(buf);
            fclose(fp);
            return FIRESTAFF_X68K_RECEIPT_ERR_READ;
        }
    }
    fclose(fp);
    char md5_hex[33];
    char sha_hex[65];
    if (firestaff_x68k_media_receipt_md5_hex(buf, size,
                                              md5_hex, sizeof(md5_hex)) != 0 ||
        firestaff_x68k_media_receipt_sha256_hex(buf, size,
                                                 sha_hex, sizeof(sha_hex)) != 0) {
        free(buf);
        return FIRESTAFF_X68K_RECEIPT_ERR_READ;
    }
    copy_string(r->actual_md5, sizeof(r->actual_md5), md5_hex);
    copy_string(r->actual_sha256, sizeof(r->actual_sha256), sha_hex);
    r->actual_size_bytes = size;

    /* Run the X68k media classifier. */
    FirestaffX68kMediaClassifyResult cls;
    FirestaffX68kMedia_Classify(buf, size, &cls);
    r->media_class = cls.media_class;
    r->flags = cls.flags;
    r->bytes_per_sector = cls.bytes_per_sector;
    r->sentinel_offset = cls.sentinel_offset;
    r->has_ftl_magic = cls.has_ftl_magic;
    r->ftl_magic_candidate_count = cls.ftl_magic_candidate_count;

    /* FTL handoff: ask the classifier whether a FTL-declared
     * in-memory area_1 size of the documented X68k 1232 KB
     * fits the on-disk HDM. We compute both full-disk and
     * single-side verdicts so a half-disk dump can be
     * distinguished from a full-disk dump. */
    r->ftl_handoff_fits_full_disk =
        FirestaffX68kMedia_FTLHandoffFits(
            &cls, (uint32_t)FIRESTAFF_X68K_BYTES_PER_DISK);
    r->ftl_handoff_fits_single_side =
        FirestaffX68kMedia_FTLHandoffFits(
            &cls, (uint32_t)FIRESTAFF_X68K_BYTES_PER_SIDE);

    *out_data = buf;
    *out_data_size = size;
    return FIRESTAFF_X68K_RECEIPT_OK;
}

int firestaff_x68k_media_receipt_finalize(
    FirestaffX68kMediaReceipt *r)
{
    if (!r) return FIRESTAFF_X68K_RECEIPT_ERR_ARGUMENT;
    const FirestaffX68kMediaReceipt_KnownHash *kh =
        find_known_hash(r->kind);
    if (!kh) {
        r->result = FIRESTAFF_X68K_RECEIPT_ERR_ARGUMENT;
        return r->result;
    }
    r->expected_class = kh->expected_class;
    if (r->actual_size_bytes != kh->size_bytes) {
        r->result = FIRESTAFF_X68K_RECEIPT_ERR_SIZE_MISMATCH;
        r->expected_class_match = 0;
        return r->result;
    }
    if (strcmp(r->actual_md5, kh->md5) != 0 ||
        strcmp(r->actual_sha256, kh->sha256) != 0) {
        r->result = FIRESTAFF_X68K_RECEIPT_ERR_HASH_MISMATCH;
        r->expected_class_match = 0;
        return r->result;
    }
    /* DMWeb-documented classifier invariants. The expected
     * media class and the expected flag-bits tuple are
     * pinned per kind in the known-hash table. */
    int media_class_ok = 0;
    if (kh->expected_class == FIRESTAFF_X68K_RECEIPT_CLASS_BLANK_SAVE_DISK) {
        media_class_ok = (r->media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK) ? 1 : 0;
    } else {
        /* Unprotected public HDM: full 1232 KB, not oversize. */
        media_class_ok = (r->media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK) ? 1 : 0;
    }
    int flags_ok =
        ((r->flags & kh->expected_flags_mask) == kh->expected_flags_value) ? 1 : 0;
    if (!media_class_ok || !flags_ok) {
        r->result = FIRESTAFF_X68K_RECEIPT_ERR_CLASS_UNEXPECTED;
        r->expected_class_match = 0;
        return r->result;
    }
    r->result = FIRESTAFF_X68K_RECEIPT_OK;
    r->expected_class_match = 1;
    return r->result;
}

int firestaff_x68k_media_receipt_scan_one(
    FirestaffX68kMediaReceipt_Kind kind,
    const char *data_dir,
    const char *cache_dir,
    int max_depth,
    FirestaffX68kMediaReceipt *receipt)
{
    if (!receipt) return FIRESTAFF_X68K_RECEIPT_ERR_ARGUMENT;
    firestaff_x68k_media_receipt_init(receipt);
    receipt->kind = kind;

    const FirestaffX68kMediaReceipt_KnownHash *kh = find_known_hash(kind);
    if (!kh) {
        receipt->result = FIRESTAFF_X68K_RECEIPT_ERR_ARGUMENT;
        return receipt->result;
    }
    copy_string(receipt->label, sizeof(receipt->label), kh->label);
    copy_string(receipt->expected_md5, sizeof(receipt->expected_md5), kh->md5);
    copy_string(receipt->expected_sha256, sizeof(receipt->expected_sha256), kh->sha256);
    receipt->expected_size_bytes = kh->size_bytes;
    receipt->expected_class = kh->expected_class;

    /* Resolve the data dir + cache dir. */
    char resolved_data[ASSET_PATH_MAX];
    char resolved_cache[FIRESTAFF_X68K_RECEIPT_PATH_CAP];
    const char *data_arg = data_dir;
    if (!data_arg || data_arg[0] == '\0') {
        const char *env = getenv("FIRESTAFF_DATA_DIR");
        if (env && env[0] != '\0') {
            data_arg = env;
        } else {
            const char *home = getenv("HOME");
            if (!home || home[0] == '\0') {
                receipt->result = FIRESTAFF_X68K_RECEIPT_ERR_NO_DATA_DIR;
                return receipt->result;
            }
            int n = snprintf(resolved_data, sizeof(resolved_data),
                             "%s/.firestaff/data", home);
            if (n <= 0 || (size_t)n >= sizeof(resolved_data)) {
                receipt->result = FIRESTAFF_X68K_RECEIPT_ERR_NO_DATA_DIR;
                return receipt->result;
            }
            data_arg = resolved_data;
        }
    }
    if (firestaff_x68k_media_receipt_resolve_cache_dir(
            cache_dir, resolved_cache, sizeof(resolved_cache)) != 0) {
        resolved_cache[0] = '\0';
    }

    const char *md5_list[2] = { kh->md5, NULL };
    char found_path[ASSET_PATH_MAX];
    int match_index = 0;
    int found = asset_find_by_md5_list(data_arg, md5_list,
                                        found_path, sizeof(found_path),
                                        &match_index, max_depth);
    if (!found) {
        receipt->result = FIRESTAFF_X68K_RECEIPT_ERR_NOT_FOUND;
        return receipt->result;
    }
    copy_string(receipt->resolved_path, sizeof(receipt->resolved_path), found_path);

    /* If the path is a virtual container path, materialize it. */
    char *read_path = found_path;
    char materialized[FIRESTAFF_X68K_RECEIPT_PATH_CAP];
    materialized[0] = '\0';
    if (is_virtual_path(found_path) && resolved_cache[0] != '\0') {
        if (asset_extract_virtual_path(found_path, materialized) == 1 &&
            materialized[0] != '\0') {
            read_path = materialized;
        }
    }

    uint8_t *buf = NULL;
    size_t buf_size = 0u;
    int rc = firestaff_x68k_media_receipt_ingest_path(
        receipt, read_path, &buf, &buf_size);
    if (buf) free(buf);
    if (rc != FIRESTAFF_X68K_RECEIPT_OK) {
        receipt->result = rc;
        return receipt->result;
    }
    receipt->present = 1;
    return firestaff_x68k_media_receipt_finalize(receipt);
}

int firestaff_x68k_media_receipt_scan_all(
    const char *data_dir,
    const char *cache_dir,
    int max_depth,
    FirestaffX68kMediaReceipt *receipts,
    size_t receipts_capacity,
    size_t *out_present_count)
{
    if (out_present_count) *out_present_count = 0u;
    if (!receipts) return FIRESTAFF_X68K_RECEIPT_ERR_ARGUMENT;
    size_t total = 0;
    const FirestaffX68kMediaReceipt_KnownHash *table =
        firestaff_x68k_media_receipt_known_hashes(&total);
    if (receipts_capacity < total) {
        return FIRESTAFF_X68K_RECEIPT_ERR_ARGUMENT;
    }
    size_t present = 0;
    size_t present_with_class_error = 0;
    int first_hard_error = 0;
    for (size_t i = 0; i < total; ++i) {
        int rc = firestaff_x68k_media_receipt_scan_one(
            table[i].kind, data_dir, cache_dir, max_depth,
            &receipts[i]);
        if (rc == FIRESTAFF_X68K_RECEIPT_OK) {
            ++present;
        } else if (rc == FIRESTAFF_X68K_RECEIPT_ERR_NOT_FOUND) {
            /* not present on this host; that's fine, keep going. */
        } else if (rc == FIRESTAFF_X68K_RECEIPT_ERR_CLASS_UNEXPECTED) {
            /* File was found and the bytes match the documented
             * hash, but the X68k classifier verdict does not
             * match the DMWeb-documented expected class. We
             * surface this per-receipt but do NOT abort the
             * whole scan: the probe and callers want to see
             * which kinds match and which need a follow-up. */
            ++present_with_class_error;
        } else {
            /* Hard error: I/O failure, hash mismatch, size
             * mismatch, etc. Surface the first one. */
            if (first_hard_error == 0) first_hard_error = rc;
        }
    }
    if (out_present_count) {
        /* "Present" counts both fully-OK receipts and
         * receipts that the asset scanner located but the
         * X68k classifier rejected; either way, the file
         * is on disk and the known-hash list matched. */
        *out_present_count = present + present_with_class_error;
    }
    if (present == 0 && present_with_class_error == 0 &&
        first_hard_error == 0) {
        return FIRESTAFF_X68K_RECEIPT_ERR_NOT_FOUND;
    }
    if (first_hard_error != 0) {
        return first_hard_error;
    }
    return FIRESTAFF_X68K_RECEIPT_OK;
}

/* ── Report writer ───────────────────────────────────────────────── */

static int appendf(char **cursor, char *end, const char *fmt, ...)
{
    if (!cursor || !*cursor || *cursor >= end) return -1;
    va_list ap;
    int n;
    va_start(ap, fmt);
    int avail = (int)(end - *cursor);
    n = vsnprintf(*cursor, (size_t)avail, fmt, ap);
    va_end(ap);
    if (n < 0 || n >= avail) {
        *cursor = end;
        return -1;
    }
    *cursor += n;
    return n;
}

int firestaff_x68k_media_receipt_write_report(
    const FirestaffX68kMediaReceipt *r,
    char *out_buf,
    size_t out_buf_cap)
{
    if (!r || !out_buf || out_buf_cap == 0u) return -1;
    char *cursor = out_buf;
    char *end = out_buf + out_buf_cap;
    out_buf[0] = '\0';
    appendf(&cursor, end, "kind=%s\n",
            firestaff_x68k_media_receipt_kind_name(r->kind));
    appendf(&cursor, end, "label=%s\n",
            r->label[0] ? r->label : "-");
    appendf(&cursor, end, "present=%d\n", r->present);
    appendf(&cursor, end, "result=%s\n",
            firestaff_x68k_media_receipt_result_name(r->result));
    appendf(&cursor, end, "resolved_path=%s\n",
            r->resolved_path[0] ? r->resolved_path : "-");
    appendf(&cursor, end, "expected_md5=%s\n",
            r->expected_md5[0] ? r->expected_md5 : "-");
    appendf(&cursor, end, "expected_sha256=%s\n",
            r->expected_sha256[0] ? r->expected_sha256 : "-");
    appendf(&cursor, end, "expected_size=%zu\n", r->expected_size_bytes);
    appendf(&cursor, end, "expected_class=%s\n",
            firestaff_x68k_media_receipt_class_name(r->expected_class));
    appendf(&cursor, end, "actual_md5=%s\n",
            r->actual_md5[0] ? r->actual_md5 : "-");
    appendf(&cursor, end, "actual_sha256=%s\n",
            r->actual_sha256[0] ? r->actual_sha256 : "-");
    appendf(&cursor, end, "actual_size=%zu\n", r->actual_size_bytes);
    appendf(&cursor, end, "media_class=%s\n",
            firestaff_x68k_media_receipt_media_class_name(r->media_class));
    appendf(&cursor, end, "flags=0x%08x\n", r->flags);
    appendf(&cursor, end, "  protection_area_blank=%d\n",
            (r->flags & FIRESTAFF_X68K_SCAN_FLAG_PROTECTION_AREA_BLANK) ? 1 : 0);
    appendf(&cursor, end, "  sentinel_present=%d\n",
            (r->flags & FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT) ? 1 : 0);
    appendf(&cursor, end, "  blank_save_disk=%d\n",
            (r->flags & FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK) ? 1 : 0);
    appendf(&cursor, end, "  ftl_present=%d\n",
            (r->flags & FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT) ? 1 : 0);
    appendf(&cursor, end, "ftl_magic_at_offset0=%d\n", r->has_ftl_magic);
    appendf(&cursor, end, "ftl_magic_candidate_count=%u\n",
            r->ftl_magic_candidate_count);
    appendf(&cursor, end, "ftl_handoff_fits_full_disk=%d\n",
            r->ftl_handoff_fits_full_disk);
    appendf(&cursor, end, "ftl_handoff_fits_single_side=%d\n",
            r->ftl_handoff_fits_single_side);
    appendf(&cursor, end, "expected_class_match=%d\n",
            r->expected_class_match);
    return (int)(cursor - out_buf);
}

/* ── Self-test ───────────────────────────────────────────────────── */

#define ST_ASSERT(cond, msg) do {                                       \
    if (!(cond)) {                                                      \
        fprintf(stderr,                                                 \
                "firestaff_x68k_media_receipt self-test: %s @ %s:%d\n",\
                (msg), __FILE__, __LINE__);                             \
        return -1;                                                      \
    }                                                                   \
} while (0)

static int test_sha256_known_vector(void) {
    /* FIPS 180-4 "abc" test vector:
     *   SHA-256("abc") =
     *   ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad */
    const char *got_expected =
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    char got[65];
    int rc = firestaff_x68k_media_receipt_sha256_hex(
        (const uint8_t*)"abc", 3, got, sizeof(got));
    ST_ASSERT(rc == 0, "sha256_abc rc");
    ST_ASSERT(strcmp(got, got_expected) == 0, "sha256_abc hex");
    return 0;
}

static int test_md5_known_vector(void) {
    /* RFC 1321 "abc" test vector:
     *   MD5("abc") = 900150983cd24fb0d6963f7d28e17f72 */
    const char *got_expected = "900150983cd24fb0d6963f7d28e17f72";
    char got[33];
    int rc = firestaff_x68k_media_receipt_md5_hex(
        (const uint8_t*)"abc", 3, got, sizeof(got));
    ST_ASSERT(rc == 0, "md5_abc rc");
    ST_ASSERT(strcmp(got, got_expected) == 0, "md5_abc hex");
    return 0;
}

static int test_sha256_md5_known_x68k_geometry(void) {
    /* A 1232 KB buffer of 0x00 must hash to a specific value.
     * We do not pin the exact value here (a self-test should
     * be self-contained and not depend on a separate fixture);
     * we only assert that the digest length is correct and
     * that two consecutive calls produce the same value. */
    static uint8_t *buf = NULL;
    if (!buf) {
        buf = (uint8_t*)calloc(1, FIRESTAFF_X68K_BYTES_PER_DISK);
        ST_ASSERT(buf != NULL, "calloc");
    }
    char a[65], b[65];
    ST_ASSERT(firestaff_x68k_media_receipt_sha256_hex(
                  buf, FIRESTAFF_X68K_BYTES_PER_DISK, a, sizeof(a)) == 0,
              "sha256 full disk rc");
    ST_ASSERT(firestaff_x68k_media_receipt_sha256_hex(
                  buf, FIRESTAFF_X68K_BYTES_PER_DISK, b, sizeof(b)) == 0,
              "sha256 full disk rc2");
    ST_ASSERT(strcmp(a, b) == 0, "sha256 deterministic");
    char m1[33], m2[33];
    ST_ASSERT(firestaff_x68k_media_receipt_md5_hex(
                  buf, FIRESTAFF_X68K_BYTES_PER_DISK, m1, sizeof(m1)) == 0,
              "md5 full disk rc");
    ST_ASSERT(firestaff_x68k_media_receipt_md5_hex(
                  buf, FIRESTAFF_X68K_BYTES_PER_DISK, m2, sizeof(m2)) == 0,
              "md5 full disk rc2");
    ST_ASSERT(strcmp(m1, m2) == 0, "md5 deterministic");
    return 0;
}

static int test_known_hashes_table(void) {
    size_t n = 0;
    const FirestaffX68kMediaReceipt_KnownHash *table =
        firestaff_x68k_media_receipt_known_hashes(&n);
    ST_ASSERT(n == 6u, "known hash count == 6");
    for (size_t i = 0; i < n; ++i) {
        ST_ASSERT(table[i].md5 != NULL, "known hash md5 not null");
        ST_ASSERT(strlen(table[i].md5) == 32u, "known hash md5 len 32");
        ST_ASSERT(table[i].sha256 != NULL, "known hash sha256 not null");
        ST_ASSERT(strlen(table[i].sha256) == 64u, "known hash sha256 len 64");
        ST_ASSERT(table[i].size_bytes == FIRESTAFF_X68K_BYTES_PER_DISK,
                  "known hash size 1232 KB");
        ST_ASSERT(table[i].label != NULL, "known hash label not null");
    }
    return 0;
}

static int test_receipt_init_and_finalize_synthetic(void) {
    /* A full-disk zero buffer has DMWeb "blank save disk" shape.
     * We can hand-build a receipt and finalize it without going
     * through the file path: that exercises the
     * MD5 / SHA-256 / size / classifier invariants. */
    static uint8_t *buf = NULL;
    if (!buf) {
        buf = (uint8_t*)calloc(1, FIRESTAFF_X68K_BYTES_PER_DISK);
        ST_ASSERT(buf != NULL, "calloc");
    }
    FirestaffX68kMediaReceipt r;
    firestaff_x68k_media_receipt_init(&r);
    r.kind = FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_SAVE_DISK;
    copy_string(r.label, sizeof(r.label), "synthetic-blank-save-disk");
    size_t total = 0;
    const FirestaffX68kMediaReceipt_KnownHash *table =
        firestaff_x68k_media_receipt_known_hashes(&total);
    const FirestaffX68kMediaReceipt_KnownHash *kh = NULL;
    for (size_t i = 0; i < total; ++i) {
        if (table[i].kind == r.kind) { kh = &table[i]; break; }
    }
    ST_ASSERT(kh != NULL, "find kind");
    copy_string(r.expected_md5, sizeof(r.expected_md5), kh->md5);
    copy_string(r.expected_sha256, sizeof(r.expected_sha256), kh->sha256);
    r.expected_size_bytes = kh->size_bytes;

    ST_ASSERT(firestaff_x68k_media_receipt_md5_hex(
                  buf, FIRESTAFF_X68K_BYTES_PER_DISK,
                  r.actual_md5, sizeof(r.actual_md5)) == 0,
              "md5 full disk rc");
    ST_ASSERT(firestaff_x68k_media_receipt_sha256_hex(
                  buf, FIRESTAFF_X68K_BYTES_PER_DISK,
                  r.actual_sha256, sizeof(r.actual_sha256)) == 0,
              "sha256 full disk rc");
    r.actual_size_bytes = FIRESTAFF_X68K_BYTES_PER_DISK;

    FirestaffX68kMediaClassifyResult cls;
    FirestaffX68kMedia_Classify(buf, FIRESTAFF_X68K_BYTES_PER_DISK, &cls);
    r.media_class = cls.media_class;
    r.flags = cls.flags;
    r.bytes_per_sector = cls.bytes_per_sector;
    r.sentinel_offset = cls.sentinel_offset;
    r.has_ftl_magic = cls.has_ftl_magic;
    r.ftl_magic_candidate_count = cls.ftl_magic_candidate_count;
    r.ftl_handoff_fits_full_disk =
        FirestaffX68kMedia_FTLHandoffFits(
            &cls, (uint32_t)FIRESTAFF_X68K_BYTES_PER_DISK);
    r.ftl_handoff_fits_single_side =
        FirestaffX68kMedia_FTLHandoffFits(
            &cls, (uint32_t)FIRESTAFF_X68K_BYTES_PER_SIDE);

    /* The synthetic zero buffer is NOT the documented save-disk
     * MD5 (the real save disk is on real media and has a
     * different MD5), so finalize will report HASH_MISMATCH.
     * We only verify the receipt routes to the right error
     * code. */
    int rc = firestaff_x68k_media_receipt_finalize(&r);
    ST_ASSERT(rc == FIRESTAFF_X68K_RECEIPT_ERR_HASH_MISMATCH,
              "synthetic blank -> hash mismatch (expected)");
    ST_ASSERT(r.expected_class == kh->expected_class, "expected_class set");
    return 0;
}

static int test_report_writer(void) {
    FirestaffX68kMediaReceipt r;
    firestaff_x68k_media_receipt_init(&r);
    r.kind = FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_ORIGINAL;
    copy_string(r.label, sizeof(r.label), "dm1-v3.0-jp-original");
    r.media_class = FIRESTAFF_X68K_MEDIA_FULL_DISK;
    r.flags = FIRESTAFF_X68K_SCAN_FLAG_PROTECTION_AREA_BLANK;
    r.actual_size_bytes = FIRESTAFF_X68K_BYTES_PER_DISK;
    r.expected_size_bytes = FIRESTAFF_X68K_BYTES_PER_DISK;
    r.expected_class = FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM;
    r.expected_class_match = 1;
    r.present = 1;
    r.result = FIRESTAFF_X68K_RECEIPT_OK;
    r.ftl_handoff_fits_full_disk = 1;
    char buf[1024];
    int n = firestaff_x68k_media_receipt_write_report(
        &r, buf, sizeof(buf));
    ST_ASSERT(n > 0, "report n > 0");
    ST_ASSERT(strstr(buf, "kind=dm1-v3.0-jp-original") != NULL,
              "report contains kind");
    ST_ASSERT(strstr(buf, "media_class=full-disk") != NULL,
              "report contains media_class");
    ST_ASSERT(strstr(buf, "expected_class=unprotected-public-hdm") != NULL,
              "report contains expected_class");
    ST_ASSERT(strstr(buf, "ftl_handoff_fits_full_disk=1") != NULL,
              "report contains ftl handoff verdict");
    return 0;
}

int firestaff_x68k_media_receipt_self_test(void)
{
    int rc;
    rc = test_sha256_known_vector();
    if (rc != 0) return rc;
    rc = test_md5_known_vector();
    if (rc != 0) return rc;
    rc = test_sha256_md5_known_x68k_geometry();
    if (rc != 0) return rc;
    rc = test_known_hashes_table();
    if (rc != 0) return rc;
    rc = test_receipt_init_and_finalize_synthetic();
    if (rc != 0) return rc;
    rc = test_report_writer();
    if (rc != 0) return rc;
    return 0;
}
