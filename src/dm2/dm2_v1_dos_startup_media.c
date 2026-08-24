#include "dm2_v1_dos_startup_media.h"

#include "asset_find_by_hash.h"
#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_mve_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__)
#  include <CommonCrypto/CommonDigest.h>
#  define DM2_V1_DOS_HAVE_COMMONCRYPTO 1
#endif

/* Small, local FIPS 180-4 SHA-256 reader.  Startup admission needs the
 * manifest's SHA-256 identity, but must not pull another platform's media
 * scanner and its unrelated archive policy into DM2's DOS boot route. */
typedef struct {
    uint32_t state[8];
    uint64_t bit_count;
    uint8_t buffer[64];
    size_t buffer_size;
} DM2_V1_DosSha256;

static const uint32_t dm2_v1_dos_sha256_constants[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

#define DM2_V1_DOS_ROR(v, n) (((v) >> (n)) | ((v) << (32u - (n))))

static void dm2_v1_dos_sha256_block(DM2_V1_DosSha256 *ctx, const uint8_t block[64])
{
    uint32_t words[64];
    uint32_t a, b, c, d, e, f, g, h;
    unsigned int i;
    for (i = 0u; i < 16u; ++i) {
        words[i] = ((uint32_t)block[i * 4u] << 24u) |
                   ((uint32_t)block[i * 4u + 1u] << 16u) |
                   ((uint32_t)block[i * 4u + 2u] << 8u) |
                   (uint32_t)block[i * 4u + 3u];
    }
    for (; i < 64u; ++i) {
        uint32_t s0 = DM2_V1_DOS_ROR(words[i - 15u], 7u) ^
                      DM2_V1_DOS_ROR(words[i - 15u], 18u) ^ (words[i - 15u] >> 3u);
        uint32_t s1 = DM2_V1_DOS_ROR(words[i - 2u], 17u) ^
                      DM2_V1_DOS_ROR(words[i - 2u], 19u) ^ (words[i - 2u] >> 10u);
        words[i] = words[i - 16u] + s0 + words[i - 7u] + s1;
    }
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    for (i = 0u; i < 64u; ++i) {
        uint32_t s1 = DM2_V1_DOS_ROR(e, 6u) ^ DM2_V1_DOS_ROR(e, 11u) ^ DM2_V1_DOS_ROR(e, 25u);
        uint32_t choice = (e & f) ^ (~e & g);
        uint32_t t1 = h + s1 + choice + dm2_v1_dos_sha256_constants[i] + words[i];
        uint32_t s0 = DM2_V1_DOS_ROR(a, 2u) ^ DM2_V1_DOS_ROR(a, 13u) ^ DM2_V1_DOS_ROR(a, 22u);
        uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = s0 + majority;
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void dm2_v1_dos_sha256_init(DM2_V1_DosSha256 *ctx)
{
    static const uint32_t initial[8] = { 0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,
                                         0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u };
    memcpy(ctx->state, initial, sizeof(initial));
    ctx->bit_count = 0u;
    ctx->buffer_size = 0u;
}

static void dm2_v1_dos_sha256_update(DM2_V1_DosSha256 *ctx, const uint8_t *data, size_t size)
{
    ctx->bit_count += (uint64_t)size * 8u;
    while (size > 0u) {
        size_t take = 64u - ctx->buffer_size;
        if (take > size) take = size;
        memcpy(ctx->buffer + ctx->buffer_size, data, take);
        ctx->buffer_size += take;
        data += take;
        size -= take;
        if (ctx->buffer_size == 64u) {
            dm2_v1_dos_sha256_block(ctx, ctx->buffer);
            ctx->buffer_size = 0u;
        }
    }
}

static void dm2_v1_dos_sha256_final(DM2_V1_DosSha256 *ctx, uint8_t digest[32])
{
    uint8_t padding[64] = {0};
    uint8_t length[8];
    uint64_t bit_count = ctx->bit_count;
    unsigned int i;
    padding[0] = 0x80u;
    dm2_v1_dos_sha256_update(ctx, padding,
                             ctx->buffer_size < 56u ? 56u - ctx->buffer_size
                                                   : 120u - ctx->buffer_size);
    for (i = 0u; i < 8u; ++i) length[i] = (uint8_t)(bit_count >> (56u - i * 8u));
    dm2_v1_dos_sha256_update(ctx, length, sizeof(length));
    for (i = 0u; i < 8u; ++i) {
        digest[i * 4u] = (uint8_t)(ctx->state[i] >> 24u);
        digest[i * 4u + 1u] = (uint8_t)(ctx->state[i] >> 16u);
        digest[i * 4u + 2u] = (uint8_t)(ctx->state[i] >> 8u);
        digest[i * 4u + 3u] = (uint8_t)ctx->state[i];
    }
}

/* Prefer macOS CommonCrypto (hardware-accelerated on Apple Silicon and modern
 * Intel).  The software path above stays for platforms without a system
 * SHA-256, and remains authoritative for identity comparison — both hashes
 * are FIPS 180-4 and must produce the same digest. */
typedef struct {
#if defined(DM2_V1_DOS_HAVE_COMMONCRYPTO)
    CC_SHA256_CTX cc;
#else
    DM2_V1_DosSha256 sw;
#endif
} DM2_V1_DosSha256Fast;

static void dm2_v1_dos_sha256_fast_init(DM2_V1_DosSha256Fast *ctx)
{
#if defined(DM2_V1_DOS_HAVE_COMMONCRYPTO)
    CC_SHA256_Init(&ctx->cc);
#else
    dm2_v1_dos_sha256_init(&ctx->sw);
#endif
}

static void dm2_v1_dos_sha256_fast_update(DM2_V1_DosSha256Fast *ctx,
                                          const uint8_t *data, size_t size)
{
#if defined(DM2_V1_DOS_HAVE_COMMONCRYPTO)
    CC_SHA256_Update(&ctx->cc, data, (CC_LONG)size);
#else
    dm2_v1_dos_sha256_update(&ctx->sw, data, size);
#endif
}

static void dm2_v1_dos_sha256_fast_final(DM2_V1_DosSha256Fast *ctx,
                                         uint8_t digest[32])
{
#if defined(DM2_V1_DOS_HAVE_COMMONCRYPTO)
    CC_SHA256_Final(digest, &ctx->cc);
#else
    dm2_v1_dos_sha256_final(&ctx->sw, digest);
#endif
}

static uint32_t dm2_v1_dos_startup_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static int dm2_v1_dos_startup_read_verified(const char *root,
                                             const char *name,
                                             uint8_t *prefix,
                                             size_t prefix_capacity,
                                             size_t *out_size)
{
    const dm2_v1_dos_file_fp_t *expected;
    char path[1024];
    uint8_t digest[32];
    uint8_t buffer[4096];
    FILE *stream;
    size_t total = 0u;
    size_t prefix_used = 0u;

    if (out_size) *out_size = 0u;
    if (!root || !root[0] || !name || !prefix || prefix_capacity == 0u) {
        return 0;
    }
    expected = dm2_v1_dos_file_fp_lookup_pc34(name);
    if (!expected) {
        return 0;
    }
    if (strstr(root, ".zip") != NULL) {
        uint8_t *bytes = NULL;
        size_t byte_count = 0u;
        DM2_V1_DosSha256Fast sha;
        if (snprintf(path, sizeof(path), "%s::%s", root, name) <= 0 ||
            strlen(path) >= sizeof(path) ||
            !asset_read_virtual_path_alloc(path, &bytes, &byte_count) ||
            !bytes || byte_count != expected->size_bytes) {
            free(bytes);
            return 0;
        }
        dm2_v1_dos_sha256_fast_init(&sha);
        dm2_v1_dos_sha256_fast_update(&sha, bytes, byte_count);
        dm2_v1_dos_sha256_fast_final(&sha, digest);
        if (!dm2_v1_dos_file_fp_matches_pc34(name, byte_count, digest)) {
            free(bytes);
            return 0;
        }
        prefix_used = byte_count < prefix_capacity ? byte_count : prefix_capacity;
        memcpy(prefix, bytes, prefix_used);
        free(bytes);
        if (out_size) *out_size = byte_count;
        return 1;
    }
    if (snprintf(path, sizeof(path), "%s/%s", root, name) <= 0 ||
        strlen(path) >= sizeof(path)) {
        return 0;
    }
    stream = fopen(path, "rb");
    if (!stream) return 0;
    {
        /* Single pass: fill the prefix buffer AND drive SHA-256 as we go, so
         * the multi-megabyte movies (intro/end) are opened, read, and hashed
         * only once instead of the earlier open-twice / rehash pattern that
         * cost seconds on external volumes. */
        DM2_V1_DosSha256Fast sha;
        dm2_v1_dos_sha256_fast_init(&sha);
        while (!ferror(stream)) {
            size_t got = fread(buffer, 1u, sizeof(buffer), stream);
            if (got == 0u) break;
            if (total > SIZE_MAX - got) {
                fclose(stream);
                return 0;
            }
            if (prefix_used < prefix_capacity) {
                size_t copy = got;
                if (copy > prefix_capacity - prefix_used) {
                    copy = prefix_capacity - prefix_used;
                }
                memcpy(prefix + prefix_used, buffer, copy);
                prefix_used += copy;
            }
            dm2_v1_dos_sha256_fast_update(&sha, buffer, got);
            total += got;
        }
        {
            int io_failed = ferror(stream) != 0;
            if (fclose(stream) != 0) io_failed = 1;
            if (io_failed || total != expected->size_bytes) {
                return 0;
            }
        }
        dm2_v1_dos_sha256_fast_final(&sha, digest);
    }
    if (!dm2_v1_dos_file_fp_matches_pc34(name, total, digest)) return 0;
    if (out_size) *out_size = total;
    return 1;
}

static int dm2_v1_dos_startup_load_verified_member(
    const char *install_root, const char *name, uint8_t **out_bytes,
    size_t *out_byte_count);

static int dm2_v1_dos_startup_contains(const uint8_t *bytes, size_t size,
                                        const char *needle, uint32_t *offset)
{
    size_t needle_size;
    size_t i;
    if (offset) *offset = 0u;
    if (!bytes || !needle || !(needle_size = strlen(needle)) || size < needle_size) {
        return 0;
    }
    for (i = 0u; i <= size - needle_size; ++i) {
        if (memcmp(bytes + i, needle, needle_size) == 0) {
            if (offset) *offset = (uint32_t)i;
            return 1;
        }
    }
    return 0;
}

int dm2_v1_dos_startup_media_probe(
    const char *install_root, DM2_V1_DosStartupMediaReceipt *out)
{
    uint8_t batch[128] = {0};
    uint8_t ibmiop[128] = {0};
    uint8_t splash[128] = {0};
    uint8_t ftl[128] = {0};
    uint8_t intro[131072] = {0};
    uint8_t end[131072] = {0};
    uint8_t intrplay[128] = {0};
    size_t size = 0u;
    uint32_t hash = 2166136261u;
    static const char mve_magic[] = "Interplay MVE File";

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!install_root || !install_root[0]) return 0;

    out->batch_dispatches_ibmiop =
        dm2_v1_dos_startup_read_verified(install_root, "dm2.bat", batch,
                                          sizeof(batch), &size) &&
        size == 26u &&
        dm2_v1_dos_startup_contains(batch, sizeof(batch), "IBMIOP SKULL.EXE", NULL);
    out->ibmiop_verified =
        dm2_v1_dos_startup_read_verified(install_root, "ibmiop.exe", ibmiop,
                                          sizeof(ibmiop), &size) &&
        size == 12626u && ibmiop[0] == 'M' && ibmiop[1] == 'Z';
    out->splash_verified =
        dm2_v1_dos_startup_read_verified(install_root, "splash", splash,
                                          sizeof(splash), &size) &&
        size == 22179u && splash[0] == 'M' && splash[1] == 'Z';
    out->ftl_verified =
        dm2_v1_dos_startup_read_verified(install_root, "ftl", ftl,
                                          sizeof(ftl), &size) &&
        size == 404966u && ftl[0] == 'M' && ftl[1] == 'Z';
    out->intro_verified =
        dm2_v1_dos_startup_read_verified(install_root, "intro", intro,
                                          sizeof(intro), &size) &&
        size == 1743936u && intro[0] == 'M' && intro[1] == 'Z';
    out->end_verified =
        dm2_v1_dos_startup_read_verified(install_root, "end", end,
                                          sizeof(end), &size) &&
        size == 4648982u && end[0] == 'M' && end[1] == 'Z';
    out->intrplay_pcx_verified =
        dm2_v1_dos_startup_read_verified(install_root, "intrplay.pcx", intrplay,
                                          sizeof(intrplay), &size) &&
        size == 14545u && intrplay[0] == 0x0au;
    /* Both programs put the retail MVE header after their DOS extender
     * stubs.  The first 128 KiB is enough to establish that fact without
     * keeping the movie in memory after verification. */
    out->intro_has_interplay_mve = out->intro_verified &&
        dm2_v1_dos_startup_contains(intro, sizeof(intro), mve_magic, NULL);
    out->end_has_interplay_mve = out->end_verified &&
        dm2_v1_dos_startup_contains(end, sizeof(end), mve_magic, NULL);
    if (out->intro_has_interplay_mve || out->end_has_interplay_mve) {
        static const char *const names[] = { "intro", "end" };
        uint32_t *const offsets[] = { &out->intro_mve_header_offset,
                                      &out->end_mve_header_offset };
        int *const valid[] = { &out->intro_has_interplay_mve,
                               &out->end_has_interplay_mve };
        size_t i;
        for (i = 0u; i < sizeof(names) / sizeof(names[0]); ++i) {
            uint8_t *movie = NULL;
            size_t movie_size = 0u;
            DM2_V1_MveStreamReceipt stream;
            if (!*valid[i] || !dm2_v1_dos_startup_load_verified_member(
                                  install_root, names[i], &movie, &movie_size) ||
                !dm2_v1_mve_stream_parse(movie, movie_size, &stream) ||
                !stream.valid || stream.mve_offset == 0u) {
                *valid[i] = 0;
                *offsets[i] = 0u;
            } else {
                *offsets[i] = stream.mve_offset;
            }
            if (movie) {
                memset(movie, 0, movie_size);
                free(movie);
            }
        }
    }
    out->complete = out->batch_dispatches_ibmiop && out->ibmiop_verified &&
        out->splash_verified && out->ftl_verified && out->intro_verified &&
        out->end_verified && out->intrplay_pcx_verified &&
        out->intro_has_interplay_mve && out->end_has_interplay_mve;
    hash = dm2_v1_dos_startup_hash_step(hash, (uint32_t)out->batch_dispatches_ibmiop);
    hash = dm2_v1_dos_startup_hash_step(hash, (uint32_t)out->ibmiop_verified);
    hash = dm2_v1_dos_startup_hash_step(hash, (uint32_t)out->splash_verified);
    hash = dm2_v1_dos_startup_hash_step(hash, (uint32_t)out->ftl_verified);
    hash = dm2_v1_dos_startup_hash_step(hash, (uint32_t)out->intro_verified);
    hash = dm2_v1_dos_startup_hash_step(hash, (uint32_t)out->end_verified);
    hash = dm2_v1_dos_startup_hash_step(hash, out->intro_mve_header_offset);
    hash = dm2_v1_dos_startup_hash_step(hash, out->end_mve_header_offset);
    out->receipt_hash = hash;
    out->valid = out->complete && out->receipt_hash != 0u;
    return out->valid;
}

static int dm2_v1_dos_startup_load_verified_member(
    const char *install_root, const char *name, uint8_t **out_bytes,
    size_t *out_byte_count)
{
    const dm2_v1_dos_file_fp_t *expected;
    DM2_V1_DosSha256Fast sha;
    uint8_t digest[32];
    uint8_t *bytes = NULL;
    char path[1024];
    FILE *stream = NULL;
    size_t read_total = 0u;

    if (out_bytes) *out_bytes = NULL;
    if (out_byte_count) *out_byte_count = 0u;
    if (!out_bytes || !out_byte_count || !install_root || !install_root[0] ||
        !name || !name[0] ||
        !(expected = dm2_v1_dos_file_fp_lookup_pc34(name))) {
        return 0;
    }
    if (strstr(install_root, ".zip") != NULL) {
        if (snprintf(path, sizeof(path), "%s::%s", install_root, name) <= 0 ||
            strlen(path) >= sizeof(path) ||
            !asset_read_virtual_path_alloc(path, &bytes, &read_total) ||
            !bytes || read_total != expected->size_bytes) {
            goto reject;
        }
        dm2_v1_dos_sha256_fast_init(&sha);
        dm2_v1_dos_sha256_fast_update(&sha, bytes, read_total);
        dm2_v1_dos_sha256_fast_final(&sha, digest);
        if (!dm2_v1_dos_file_fp_matches_pc34(name, read_total, digest)) {
            goto reject;
        }
        *out_bytes = bytes;
        *out_byte_count = read_total;
        return 1;
    }
    if (snprintf(path, sizeof(path), "%s/%s", install_root, name) <= 0 ||
        strlen(path) >= sizeof(path) ||
        !(bytes = (uint8_t *)malloc(expected->size_bytes))) {
        return 0;
    }
    stream = fopen(path, "rb");
    if (!stream) goto reject;
    while (read_total < expected->size_bytes) {
        const size_t got = fread(bytes + read_total, 1u,
                                 expected->size_bytes - read_total, stream);
        if (got == 0u) break;
        read_total += got;
    }
    if (ferror(stream) || read_total != expected->size_bytes ||
        fgetc(stream) != EOF) {
        fclose(stream);
        stream = NULL;
        goto reject;
    }
    if (fclose(stream) != 0) {
        stream = NULL;
        goto reject;
    }
    stream = NULL;
    dm2_v1_dos_sha256_fast_init(&sha);
    dm2_v1_dos_sha256_fast_update(&sha, bytes, expected->size_bytes);
    dm2_v1_dos_sha256_fast_final(&sha, digest);
    if (!dm2_v1_dos_file_fp_matches_pc34(name, expected->size_bytes,
                                           digest)) {
        goto reject;
    }
    *out_bytes = bytes;
    *out_byte_count = expected->size_bytes;
    return 1;

reject:
    if (stream) fclose(stream);
    if (bytes) {
        memset(bytes, 0, expected ? expected->size_bytes : 0u);
        free(bytes);
    }
    return 0;
}

int dm2_v1_dos_startup_media_load_intro_verified(
    const char *install_root, const DM2_V1_DosStartupMediaReceipt *receipt,
    uint8_t **out_bytes, size_t *out_byte_count)
{
    if (out_bytes) *out_bytes = NULL;
    if (out_byte_count) *out_byte_count = 0u;
    if (!receipt || !receipt->valid || !receipt->complete ||
        !receipt->intro_verified || !receipt->intro_has_interplay_mve ||
        receipt->intro_mve_header_offset == 0u)
        return 0;
    return dm2_v1_dos_startup_load_verified_member(install_root, "intro",
                                                    out_bytes, out_byte_count);
}
