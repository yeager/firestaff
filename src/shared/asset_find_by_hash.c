/*
 * asset_find_by_hash.c — Hash-based asset file discovery
 *
 * Scans directories recursively and matches files by MD5 hash.
 * Eliminates the need to hardcode platform-specific filenames
 * (DUNGEON.DAT vs Dungeon.DAT vs DM2DUNGEON.DAT etc).
 *
 * Uses the same MD5 routines as asset_status_m12.c.
 */

#include "asset_find_by_hash.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef FIRESTAFF_HAS_ZLIB
#include <zlib.h>
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#define ASSET_SCAN_MAX_FILE_BYTES (32LL * 1024LL * 1024LL)
#define ASSET_ZIP_MAX_ENTRY_BYTES (16U * 1024U * 1024U)
#define ASSET_ISO_SECTOR_SIZE 2048U
#define ASSET_ISO_RAW_SECTOR_SIZE 2352U
#define ASSET_ISO_RAW_DATA_OFFSET 16U
#define ASSET_ISO_MAX_DIR_DEPTH 8

/* ── Embedded MD5 (same as asset_status_m12.c) ────────────────── */

typedef struct {
    unsigned int state[4];
    unsigned int count[2];
    unsigned char buffer[64];
} AssetMd5Ctx;

static void md5_body(AssetMd5Ctx *ctx, const unsigned char *data);

#define F(x,y,z) (((x)&(y))|((~(x))&(z)))
#define G(x,y,z) (((x)&(z))|((y)&(~(z))))
#define H(x,y,z) ((x)^(y)^(z))
#define I(x,y,z) ((y)^((x)|(~(z))))
#define ROT(x,n) (((x)<<(n))|((x)>>(32-(n))))

static const unsigned char md5_padding[64] = {0x80};

static void md5_init(AssetMd5Ctx *ctx) {
    ctx->count[0] = ctx->count[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
}

static void md5_update(AssetMd5Ctx *ctx, const unsigned char *input, unsigned int len) {
    unsigned int idx = (ctx->count[0] >> 3) & 0x3F;
    ctx->count[0] += (len << 3);
    if (ctx->count[0] < (len << 3)) ctx->count[1]++;
    ctx->count[1] += (len >> 29);
    unsigned int partLen = 64 - idx;
    unsigned int i = 0;
    if (len >= partLen) {
        memcpy(&ctx->buffer[idx], input, partLen);
        md5_body(ctx, ctx->buffer);
        for (i = partLen; i + 63 < len; i += 64)
            md5_body(ctx, &input[i]);
        idx = 0;
    }
    memcpy(&ctx->buffer[idx], &input[i], len - i);
}

static void md5_final(AssetMd5Ctx *ctx, char outHex[33]) {
    unsigned char digest[16];
    unsigned char bits[8];
    unsigned int idx, padLen;
    for (int i = 0; i < 4; i++) {
        bits[i] = (unsigned char)(ctx->count[0] >> (i * 8));
        bits[i+4] = (unsigned char)(ctx->count[1] >> (i * 8));
    }
    idx = (ctx->count[0] >> 3) & 0x3f;
    padLen = (idx < 56) ? (56 - idx) : (120 - idx);
    md5_update(ctx, md5_padding, padLen);
    md5_update(ctx, bits, 8);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            digest[i*4+j] = (unsigned char)(ctx->state[i] >> (j * 8));
    for (int i = 0; i < 16; i++)
        sprintf(&outHex[i*2], "%02x", digest[i]);
    outHex[32] = 0;
}

static void md5_body(AssetMd5Ctx *ctx, const unsigned char *block) {
    unsigned int a = ctx->state[0], b = ctx->state[1];
    unsigned int c = ctx->state[2], d = ctx->state[3];
    unsigned int x[16];
    for (int i = 0; i < 16; i++)
        x[i] = (unsigned int)block[i*4] | ((unsigned int)block[i*4+1]<<8) |
               ((unsigned int)block[i*4+2]<<16) | ((unsigned int)block[i*4+3]<<24);
    #define STEP(f,a,b,c,d,x,s,ac) { \
        (a) += f((b),(c),(d)) + (x) + (unsigned int)(ac); \
        (a) = ROT((a),(s)); (a) += (b); }
    STEP(F,a,b,c,d,x[ 0], 7,0xd76aa478) STEP(F,d,a,b,c,x[ 1],12,0xe8c7b756)
    STEP(F,c,d,a,b,x[ 2],17,0x242070db) STEP(F,b,c,d,a,x[ 3],22,0xc1bdceee)
    STEP(F,a,b,c,d,x[ 4], 7,0xf57c0faf) STEP(F,d,a,b,c,x[ 5],12,0x4787c62a)
    STEP(F,c,d,a,b,x[ 6],17,0xa8304613) STEP(F,b,c,d,a,x[ 7],22,0xfd469501)
    STEP(F,a,b,c,d,x[ 8], 7,0x698098d8) STEP(F,d,a,b,c,x[ 9],12,0x8b44f7af)
    STEP(F,c,d,a,b,x[10],17,0xffff5bb1) STEP(F,b,c,d,a,x[11],22,0x895cd7be)
    STEP(F,a,b,c,d,x[12], 7,0x6b901122) STEP(F,d,a,b,c,x[13],12,0xfd987193)
    STEP(F,c,d,a,b,x[14],17,0xa679438e) STEP(F,b,c,d,a,x[15],22,0x49b40821)
    STEP(G,a,b,c,d,x[ 1], 5,0xf61e2562) STEP(G,d,a,b,c,x[ 6], 9,0xc040b340)
    STEP(G,c,d,a,b,x[11],14,0x265e5a51) STEP(G,b,c,d,a,x[ 0],20,0xe9b6c7aa)
    STEP(G,a,b,c,d,x[ 5], 5,0xd62f105d) STEP(G,d,a,b,c,x[10], 9,0x02441453)
    STEP(G,c,d,a,b,x[15],14,0xd8a1e681) STEP(G,b,c,d,a,x[ 4],20,0xe7d3fbc8)
    STEP(G,a,b,c,d,x[ 9], 5,0x21e1cde6) STEP(G,d,a,b,c,x[14], 9,0xc33707d6)
    STEP(G,c,d,a,b,x[ 3],14,0xf4d50d87) STEP(G,b,c,d,a,x[ 8],20,0x455a14ed)
    STEP(G,a,b,c,d,x[13], 5,0xa9e3e905) STEP(G,d,a,b,c,x[ 2], 9,0xfcefa3f8)
    STEP(G,c,d,a,b,x[ 7],14,0x676f02d9) STEP(G,b,c,d,a,x[12],20,0x8d2a4c8a)
    STEP(H,a,b,c,d,x[ 5], 4,0xfffa3942) STEP(H,d,a,b,c,x[ 8],11,0x8771f681)
    STEP(H,c,d,a,b,x[11],16,0x6d9d6122) STEP(H,b,c,d,a,x[14],23,0xfde5380c)
    STEP(H,a,b,c,d,x[ 1], 4,0xa4beea44) STEP(H,d,a,b,c,x[ 4],11,0x4bdecfa9)
    STEP(H,c,d,a,b,x[ 7],16,0xf6bb4b60) STEP(H,b,c,d,a,x[10],23,0xbebfbc70)
    STEP(H,a,b,c,d,x[13], 4,0x289b7ec6) STEP(H,d,a,b,c,x[ 0],11,0xeaa127fa)
    STEP(H,c,d,a,b,x[ 3],16,0xd4ef3085) STEP(H,b,c,d,a,x[ 6],23,0x04881d05)
    STEP(H,a,b,c,d,x[ 9], 4,0xd9d4d039) STEP(H,d,a,b,c,x[12],11,0xe6db99e5)
    STEP(H,c,d,a,b,x[15],16,0x1fa27cf8) STEP(H,b,c,d,a,x[ 2],23,0xc4ac5665)
    STEP(I,a,b,c,d,x[ 0], 6,0xf4292244) STEP(I,d,a,b,c,x[ 7],10,0x432aff97)
    STEP(I,c,d,a,b,x[14],15,0xab9423a7) STEP(I,b,c,d,a,x[ 5],21,0xfc93a039)
    STEP(I,a,b,c,d,x[12], 6,0x655b59c3) STEP(I,d,a,b,c,x[ 3],10,0x8f0ccc92)
    STEP(I,c,d,a,b,x[10],15,0xffeff47d) STEP(I,b,c,d,a,x[ 1],21,0x85845dd1)
    STEP(I,a,b,c,d,x[ 8], 6,0x6fa87e4f) STEP(I,d,a,b,c,x[15],10,0xfe2ce6e0)
    STEP(I,c,d,a,b,x[ 6],15,0xa3014314) STEP(I,b,c,d,a,x[13],21,0x4e0811a1)
    STEP(I,a,b,c,d,x[ 4], 6,0xf7537e82) STEP(I,d,a,b,c,x[11],10,0xbd3af235)
    STEP(I,c,d,a,b,x[ 2],15,0x2ad7d2bb) STEP(I,b,c,d,a,x[ 9],21,0xeb86d391)
    ctx->state[0] += a; ctx->state[1] += b;
    ctx->state[2] += c; ctx->state[3] += d;
    #undef STEP
}

/* ── File MD5 helper ──────────────────────────────────────────── */

static int file_md5(const char *path, char outHex[33]) {
    unsigned char buf[8192];
    AssetMd5Ctx ctx;
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;
    md5_init(&ctx);
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        md5_update(&ctx, buf, (unsigned int)n);
    int ok = !ferror(fp);
    fclose(fp);
    if (!ok) return 0;
    md5_final(&ctx, outHex);
    return 1;
}

static int stream_md5_update_file(FILE *fp, long offset, uint32_t size,
                                  AssetMd5Ctx *ctx) {
    unsigned char buf[8192];
    uint32_t remaining = size;
    if (!fp || !ctx || fseek(fp, offset, SEEK_SET) != 0) return 0;
    while (remaining > 0U) {
        size_t want = remaining > sizeof(buf) ? sizeof(buf) : (size_t)remaining;
        size_t got = fread(buf, 1U, want, fp);
        if (got != want) return 0;
        md5_update(ctx, buf, (unsigned int)got);
        remaining -= (uint32_t)got;
    }
    return 1;
}

static int file_range_md5(FILE *fp, long offset, uint32_t size, char outHex[33]) {
    AssetMd5Ctx ctx;
    md5_init(&ctx);
    if (!stream_md5_update_file(fp, offset, size, &ctx)) return 0;
    md5_final(&ctx, outHex);
    return 1;
}

static int is_hex_char(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static char lower_hex_char(char c) {
    return (c >= 'A' && c <= 'F') ? (char)(c - 'A' + 'a') : c;
}

static int normalize_md5(const char *expectedMd5, char outExpected[33]) {
    int i;
    if (!expectedMd5 || strlen(expectedMd5) != 32) return 0;
    for (i = 0; i < 32; ++i) {
        if (!is_hex_char(expectedMd5[i])) return 0;
        outExpected[i] = lower_hex_char(expectedMd5[i]);
    }
    outExpected[32] = '\0';
    return 1;
}

static int copy_match_path(const char *path, char *outPath, int outPathLen) {
    size_t len;
    if (!path || !outPath || outPathLen <= 0) return 0;
    len = strlen(path);
    if (len >= (size_t)outPathLen) return 0;
    memcpy(outPath, path, len + 1U);
    return 1;
}

static int copy_virtual_match_path(const char *container, const char *entry,
                                   char *outPath, int outPathLen) {
    char virtualPath[ASSET_PATH_MAX];
    if (!container || !entry || !outPath || outPathLen <= 0) return 0;
    if (snprintf(virtualPath, sizeof(virtualPath), "%s::%s", container, entry) >=
        (int)sizeof(virtualPath)) {
        return 0;
    }
    return copy_match_path(virtualPath, outPath, outPathLen);
}

static int is_better_zip_entry(const char *candidate, const char *current) {
    if (!candidate || !*candidate) return 0;
    if (!current || !*current) return 1;
    return strcmp(candidate, current) < 0;
}

static uint16_t read_u16le(const unsigned char *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8U);
}

static uint32_t read_u32le(const unsigned char *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8U) |
           ((uint32_t)p[2] << 16U) |
           ((uint32_t)p[3] << 24U);
}

static int has_case_suffix(const char *path, const char *suffix) {
    size_t pathLen, suffixLen, i;
    if (!path || !suffix) return 0;
    pathLen = strlen(path);
    suffixLen = strlen(suffix);
    if (pathLen < suffixLen) return 0;
    path += pathLen - suffixLen;
    for (i = 0; i < suffixLen; ++i) {
        char a = path[i];
        char b = suffix[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b) return 0;
    }
    return 1;
}

static int asset_casecmp(const char *a, const char *b) {
    while (a && b && *a && *b) {
        char ca = *a++;
        char cb = *b++;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
        if (ca != cb) return (int)(unsigned char)ca - (int)(unsigned char)cb;
    }
    if (!a || !b) return a == b ? 0 : (a ? 1 : -1);
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

/* Case-insensitive variant of is_better_zip_entry for ISO 9660 directory
 * entries. ISO 9660:1988 §7.5 mandates that file identifiers be compared
 * without regard to case at the filesystem level, even though the on-disk
 * bytes may preserve Joliet/lower-case form. Using case-sensitive
 * strcmp would cause a directory containing both "DUNGEON.DAT" and
 * "dungeon.dat" (Joliet extension) to deterministically pick the
 * uppercase form, which is the wrong semantic for ISO 9660. */
static int is_better_iso_entry(const char *candidate, const char *current) {
    if (!candidate || !*candidate) return 0;
    if (!current || !*current) return 1;
    return asset_casecmp(candidate, current) < 0;
}

static int is_zip_path(const char *path) {
    return has_case_suffix(path, ".zip");
}

static int is_iso_path(const char *path) {
    return has_case_suffix(path, ".iso") || has_case_suffix(path, ".bin");
}

static int is_cue_path(const char *path) {
    return has_case_suffix(path, ".cue");
}

static int is_known_large_whole_file_hash(const char *expectedMd5) {
    static const char *const largeHashes[] = {
        "e88d60859f65f08fa622e1992b02280f", /* Nexus Saturn data image */
        "96e511c8d36ccbe30a48ba36c59df194", /* Nexus Saturn data image */
        "b7afb338ad31be1025b53f9aff12d73a", /* Theron's Quest JP Track 02 */
        "f23601102138f87c33025877767ebf76", /* Theron's Quest US Track 02 */
        "397039af02d50d15c70b74088eb8a1cb", /* Theron's Quest JP Rev 1 Track 02 ISO */
        "3d8b78571dcd0e6eb8eb4b01eeb7fbba", /* Theron's Quest US Track 02 ISO */
        NULL
    };
    int i;
    if (!expectedMd5) return 0;
    for (i = 0; largeHashes[i] != NULL; ++i) {
        if (strcmp(expectedMd5, largeHashes[i]) == 0) return 1;
    }
    return 0;
}

static int md5_list_contains_large_whole_file_hash(const char *const *md5List,
                                                   int md5Count) {
    int i;
    if (!md5List || md5Count <= 0) return 0;
    for (i = 0; i < md5Count; ++i) {
        if (is_known_large_whole_file_hash(md5List[i])) return 1;
    }
    return 0;
}

static int md5_list_match_index(const char *hex, const char *const *md5List,
                                const int *matched, int md5Count) {
    int i;
    if (!hex || !md5List || md5Count <= 0) return -1;
    for (i = 0; i < md5Count; ++i) {
        if ((!matched || !matched[i]) && strcmp(hex, md5List[i]) == 0) {
            return i;
        }
    }
    return -1;
}

static int zip_stored_entry_md5(FILE *fp, uint32_t dataOffset, uint32_t size,
                                char outHex[33]) {
    return file_range_md5(fp, (long)dataOffset, size, outHex);
}

static int zip_deflated_entry_md5(FILE *fp, uint32_t dataOffset,
                                  uint32_t compressedSize,
                                  uint32_t uncompressedSize,
                                  char outHex[33]) {
#ifdef FIRESTAFF_HAS_ZLIB
    unsigned char inBuf[8192];
    unsigned char outBuf[8192];
    uint32_t remaining = compressedSize;
    uint32_t produced = 0U;
    z_stream zs;
    AssetMd5Ctx md5;
    int ret;
    if (!fp || !outHex || uncompressedSize > ASSET_ZIP_MAX_ENTRY_BYTES) return 0;
    if (fseek(fp, (long)dataOffset, SEEK_SET) != 0) return 0;
    memset(&zs, 0, sizeof(zs));
    if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) return 0;
    md5_init(&md5);
    do {
        size_t chunk;
        if (zs.avail_in == 0 && remaining > 0U) {
            chunk = remaining > sizeof(inBuf) ? sizeof(inBuf) : (size_t)remaining;
            if (fread(inBuf, 1U, chunk, fp) != chunk) {
                inflateEnd(&zs);
                return 0;
            }
            remaining -= (uint32_t)chunk;
            zs.next_in = inBuf;
            zs.avail_in = (uInt)chunk;
        }
        do {
            zs.next_out = outBuf;
            zs.avail_out = (uInt)sizeof(outBuf);
            ret = inflate(&zs, remaining == 0U ? Z_FINISH : Z_NO_FLUSH);
            if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
                inflateEnd(&zs);
                return 0;
            }
            chunk = sizeof(outBuf) - zs.avail_out;
            if (chunk > 0U) {
                md5_update(&md5, outBuf, (unsigned int)chunk);
                produced += (uint32_t)chunk;
                if (produced > uncompressedSize) {
                    inflateEnd(&zs);
                    return 0;
                }
            }
        } while (zs.avail_out == 0);
    } while (ret != Z_STREAM_END && (remaining > 0U || zs.avail_in > 0U));
    inflateEnd(&zs);
    if (produced != uncompressedSize) return 0;
    md5_final(&md5, outHex);
    return 1;
#else
    (void)fp;
    (void)dataOffset;
    (void)compressedSize;
    (void)uncompressedSize;
    (void)outHex;
    return 0;
#endif
}

static int copy_file_range_to_path(FILE *fp, uint32_t dataOffset, uint32_t size,
                                   const char *outFilePath) {
    unsigned char buf[8192];
    uint32_t remaining = size;
    FILE *out;
    if (!fp || !outFilePath || fseek(fp, (long)dataOffset, SEEK_SET) != 0) return 0;
    out = fopen(outFilePath, "wb");
    if (!out) return 0;
    while (remaining > 0U) {
        size_t want = remaining > sizeof(buf) ? sizeof(buf) : (size_t)remaining;
        if (fread(buf, 1U, want, fp) != want ||
            fwrite(buf, 1U, want, out) != want) {
            fclose(out);
            return 0;
        }
        remaining -= (uint32_t)want;
    }
    return fclose(out) == 0;
}

static int zip_deflated_entry_extract(FILE *fp, uint32_t dataOffset,
                                      uint32_t compressedSize,
                                      uint32_t uncompressedSize,
                                      const char *outFilePath) {
#ifdef FIRESTAFF_HAS_ZLIB
    unsigned char inBuf[8192];
    unsigned char outBuf[8192];
    uint32_t remaining = compressedSize;
    uint32_t produced = 0U;
    z_stream zs;
    FILE *out;
    int ret = Z_OK;
    if (!fp || !outFilePath || uncompressedSize > ASSET_ZIP_MAX_ENTRY_BYTES) return 0;
    if (fseek(fp, (long)dataOffset, SEEK_SET) != 0) return 0;
    out = fopen(outFilePath, "wb");
    if (!out) return 0;
    memset(&zs, 0, sizeof(zs));
    if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) {
        fclose(out);
        return 0;
    }
    do {
        size_t chunk;
        if (zs.avail_in == 0 && remaining > 0U) {
            chunk = remaining > sizeof(inBuf) ? sizeof(inBuf) : (size_t)remaining;
            if (fread(inBuf, 1U, chunk, fp) != chunk) {
                inflateEnd(&zs);
                fclose(out);
                return 0;
            }
            remaining -= (uint32_t)chunk;
            zs.next_in = inBuf;
            zs.avail_in = (uInt)chunk;
        }
        do {
            zs.next_out = outBuf;
            zs.avail_out = (uInt)sizeof(outBuf);
            ret = inflate(&zs, remaining == 0U ? Z_FINISH : Z_NO_FLUSH);
            if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
                inflateEnd(&zs);
                fclose(out);
                return 0;
            }
            chunk = sizeof(outBuf) - zs.avail_out;
            if (chunk > 0U) {
                if (fwrite(outBuf, 1U, chunk, out) != chunk) {
                    inflateEnd(&zs);
                    fclose(out);
                    return 0;
                }
                produced += (uint32_t)chunk;
                if (produced > uncompressedSize) {
                    inflateEnd(&zs);
                    fclose(out);
                    return 0;
                }
            }
        } while (zs.avail_out == 0);
    } while (ret != Z_STREAM_END && (remaining > 0U || zs.avail_in > 0U));
    inflateEnd(&zs);
    if (produced != uncompressedSize) {
        fclose(out);
        return 0;
    }
    return fclose(out) == 0;
#else
    (void)fp;
    (void)dataOffset;
    (void)compressedSize;
    (void)uncompressedSize;
    (void)outFilePath;
    return 0;
#endif
}

static int zip_extract_entry_to_path(const char *zipPath, const char *entryName,
                                     const char *outFilePath) {
    FILE *fp;
    long fileSize;
    long searchStart;
    long eocdOffset = -1;
    unsigned char *tail;
    size_t tailSize;
    uint32_t cdOffset = 0U, cdSize = 0U;
    uint16_t entryCount = 0U;
    uint32_t pos;
    uint16_t i;
    if (!zipPath || !entryName || !outFilePath) return 0;
    fp = fopen(zipPath, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    fileSize = ftell(fp);
    if (fileSize < 22) {
        fclose(fp);
        return 0;
    }
    tailSize = (size_t)(fileSize < (long)(65557U) ? fileSize : (long)65557U);
    searchStart = fileSize - (long)tailSize;
    tail = (unsigned char*)malloc(tailSize);
    if (!tail) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, searchStart, SEEK_SET) != 0 ||
        fread(tail, 1U, tailSize, fp) != tailSize) {
        free(tail);
        fclose(fp);
        return 0;
    }
    for (long j = (long)tailSize - 22; j >= 0; --j) {
        if (tail[j] == 0x50 && tail[j + 1] == 0x4b &&
            tail[j + 2] == 0x05 && tail[j + 3] == 0x06) {
            eocdOffset = searchStart + j;
            entryCount = read_u16le(tail + j + 10);
            cdSize = read_u32le(tail + j + 12);
            cdOffset = read_u32le(tail + j + 16);
            break;
        }
    }
    free(tail);
    if (eocdOffset < 0 || cdOffset + cdSize > (uint32_t)fileSize) {
        fclose(fp);
        return 0;
    }
    pos = cdOffset;
    for (i = 0; i < entryCount && pos + 46U <= cdOffset + cdSize; ++i) {
        unsigned char hdr[46];
        uint16_t method, nameLen, extraLen, commentLen;
        uint32_t compressedSize, uncompressedSize, localOffset;
        char name[256];
        unsigned char local[30];
        uint16_t localNameLen, localExtraLen;
        uint32_t dataOffset;
        if (fseek(fp, (long)pos, SEEK_SET) != 0 ||
            fread(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr)) break;
        if (read_u32le(hdr) != 0x02014b50U) break;
        method = read_u16le(hdr + 10);
        compressedSize = read_u32le(hdr + 20);
        uncompressedSize = read_u32le(hdr + 24);
        nameLen = read_u16le(hdr + 28);
        extraLen = read_u16le(hdr + 30);
        commentLen = read_u16le(hdr + 32);
        localOffset = read_u32le(hdr + 42);
        if (nameLen == 0U || nameLen >= sizeof(name)) {
            pos += 46U + nameLen + extraLen + commentLen;
            continue;
        }
        if (fread(name, 1U, nameLen, fp) != nameLen) break;
        name[nameLen] = '\0';
        pos += 46U + nameLen + extraLen + commentLen;
        if (strcmp(name, entryName) != 0) continue;
        if (fseek(fp, (long)localOffset, SEEK_SET) != 0 ||
            fread(local, 1U, sizeof(local), fp) != sizeof(local) ||
            read_u32le(local) != 0x04034b50U) break;
        localNameLen = read_u16le(local + 26);
        localExtraLen = read_u16le(local + 28);
        dataOffset = localOffset + 30U + localNameLen + localExtraLen;
        if (method == 0U) {
            int ok = copy_file_range_to_path(fp, dataOffset, uncompressedSize, outFilePath);
            fclose(fp);
            return ok;
        }
        if (method == 8U) {
            int ok = zip_deflated_entry_extract(fp, dataOffset, compressedSize,
                                                uncompressedSize, outFilePath);
            fclose(fp);
            return ok;
        }
        break;
    }
    fclose(fp);
    return 0;
}

static int scan_zip_by_md5(const char *zipPath, const char *expectedMd5,
                            char *outPath, int outPathLen) {
    FILE *fp;
    long fileSize;
    long searchStart;
    long eocdOffset = -1;
    unsigned char *tail;
    size_t tailSize;
    uint32_t cdOffset, cdSize;
    uint16_t entryCount;
    uint32_t pos;
    uint16_t i;
    char bestName[256];
    int hasMatch = 0;
    int found = 0;

    fp = fopen(zipPath, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    fileSize = ftell(fp);
    if (fileSize < 22) {
        fclose(fp);
        return 0;
    }
    tailSize = (size_t)(fileSize < (long)(65557U) ? fileSize : (long)65557U);
    searchStart = fileSize - (long)tailSize;
    tail = (unsigned char*)malloc(tailSize);
    if (!tail) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, searchStart, SEEK_SET) != 0 ||
        fread(tail, 1U, tailSize, fp) != tailSize) {
        free(tail);
        fclose(fp);
        return 0;
    }
    for (long j = (long)tailSize - 22; j >= 0; --j) {
        if (tail[j] == 0x50 && tail[j + 1] == 0x4b &&
            tail[j + 2] == 0x05 && tail[j + 3] == 0x06) {
            eocdOffset = searchStart + j;
            entryCount = read_u16le(tail + j + 10);
            cdSize = read_u32le(tail + j + 12);
            cdOffset = read_u32le(tail + j + 16);
            break;
        }
    }
    free(tail);
    if (eocdOffset < 0 || cdOffset + cdSize > (uint32_t)fileSize) {
        fclose(fp);
        return 0;
    }

    bestName[0] = '\0';

    pos = cdOffset;
    for (i = 0; i < entryCount && pos + 46U <= cdOffset + cdSize; ++i) {
        unsigned char hdr[46];
        uint16_t method, nameLen, extraLen, commentLen;
        uint32_t compressedSize, uncompressedSize, localOffset;
        char name[256];
        unsigned char local[30];
        uint16_t localNameLen, localExtraLen;
        uint32_t dataOffset;
        char hex[33];

        if (fseek(fp, (long)pos, SEEK_SET) != 0 ||
            fread(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr)) break;
        if (read_u32le(hdr) != 0x02014b50U) break;
        method = read_u16le(hdr + 10);
        compressedSize = read_u32le(hdr + 20);
        uncompressedSize = read_u32le(hdr + 24);
        nameLen = read_u16le(hdr + 28);
        extraLen = read_u16le(hdr + 30);
        commentLen = read_u16le(hdr + 32);
        localOffset = read_u32le(hdr + 42);

        if (nameLen == 0U || nameLen >= sizeof(name)) {
            pos += 46U + nameLen + extraLen + commentLen;
            continue;
        }
        if (fread(name, 1U, nameLen, fp) != nameLen) break;
        name[nameLen] = '\0';
        pos += 46U + nameLen + extraLen + commentLen;
        if (name[nameLen - 1U] == '/' || uncompressedSize < 16U ||
            uncompressedSize > ASSET_ZIP_MAX_ENTRY_BYTES) {
            continue;
        }
        if (fseek(fp, (long)localOffset, SEEK_SET) != 0 ||
            fread(local, 1U, sizeof(local), fp) != sizeof(local) ||
            read_u32le(local) != 0x04034b50U) {
            continue;
        }
        localNameLen = read_u16le(local + 26);
        localExtraLen = read_u16le(local + 28);
        dataOffset = localOffset + 30U + localNameLen + localExtraLen;
        if (method == 0U) {
            found = zip_stored_entry_md5(fp, dataOffset, uncompressedSize, hex);
        } else if (method == 8U) {
            found = zip_deflated_entry_md5(fp, dataOffset, compressedSize,
                                           uncompressedSize, hex);
        } else {
            found = 0;
        }
        if (found && strcmp(hex, expectedMd5) == 0) {
            if (is_better_zip_entry(name, bestName)) {
                size_t nameLen = strlen(name);
                if (nameLen < sizeof(bestName)) {
                    memcpy(bestName, name, nameLen + 1U);
                    hasMatch = 1;
                }
            }
        }
    }
    fclose(fp);
    if (!hasMatch) return 0;
    return copy_virtual_match_path(zipPath, bestName, outPath, outPathLen);
}

static int scan_zip_by_md5_list(const char *zipPath, const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[]) {
    FILE *fp;
    long fileSize;
    long searchStart;
    long eocdOffset = -1;
    unsigned char *tail;
    size_t tailSize;
    uint32_t cdOffset, cdSize;
    uint16_t entryCount;
    uint32_t pos;
    uint16_t i;
    int foundCount = 0;

    fp = fopen(zipPath, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    fileSize = ftell(fp);
    if (fileSize < 22) {
        fclose(fp);
        return 0;
    }
    tailSize = (size_t)(fileSize < (long)(65557U) ? fileSize : (long)65557U);
    searchStart = fileSize - (long)tailSize;
    tail = (unsigned char*)malloc(tailSize);
    if (!tail) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, searchStart, SEEK_SET) != 0 ||
        fread(tail, 1U, tailSize, fp) != tailSize) {
        free(tail);
        fclose(fp);
        return 0;
    }
    for (long j = (long)tailSize - 22; j >= 0; --j) {
        if (tail[j] == 0x50 && tail[j + 1] == 0x4b &&
            tail[j + 2] == 0x05 && tail[j + 3] == 0x06) {
            eocdOffset = searchStart + j;
            entryCount = read_u16le(tail + j + 10);
            cdSize = read_u32le(tail + j + 12);
            cdOffset = read_u32le(tail + j + 16);
            break;
        }
    }
    free(tail);
    if (eocdOffset < 0 || cdOffset + cdSize > (uint32_t)fileSize) {
        fclose(fp);
        return 0;
    }

    pos = cdOffset;
    for (i = 0; i < entryCount && pos + 46U <= cdOffset + cdSize; ++i) {
        unsigned char hdr[46];
        uint16_t method, nameLen, extraLen, commentLen;
        uint32_t compressedSize, uncompressedSize, localOffset;
        char name[256];
        unsigned char local[30];
        uint16_t localNameLen, localExtraLen;
        uint32_t dataOffset;
        char hex[33];
        int matchIndex;
        char existingPath[ASSET_PATH_MAX];
        const char *slashPath;

        if (fseek(fp, (long)pos, SEEK_SET) != 0 ||
            fread(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr)) break;
        if (read_u32le(hdr) != 0x02014b50U) break;
        method = read_u16le(hdr + 10);
        compressedSize = read_u32le(hdr + 20);
        uncompressedSize = read_u32le(hdr + 24);
        nameLen = read_u16le(hdr + 28);
        extraLen = read_u16le(hdr + 30);
        commentLen = read_u16le(hdr + 32);
        localOffset = read_u32le(hdr + 42);

        if (nameLen == 0U || nameLen >= sizeof(name)) {
            pos += 46U + nameLen + extraLen + commentLen;
            continue;
        }
        if (fread(name, 1U, nameLen, fp) != nameLen) break;
        name[nameLen] = '\0';
        pos += 46U + nameLen + extraLen + commentLen;
        if (name[nameLen - 1U] == '/' || uncompressedSize < 16U ||
            uncompressedSize > ASSET_ZIP_MAX_ENTRY_BYTES) {
            continue;
        }
        if (fseek(fp, (long)localOffset, SEEK_SET) != 0 ||
            fread(local, 1U, sizeof(local), fp) != sizeof(local) ||
            read_u32le(local) != 0x04034b50U) {
            continue;
        }
        localNameLen = read_u16le(local + 26);
        localExtraLen = read_u16le(local + 28);
        dataOffset = localOffset + 30U + localNameLen + localExtraLen;
        if (method == 0U) {
            if (!zip_stored_entry_md5(fp, dataOffset, uncompressedSize, hex)) continue;
        } else if (method == 8U) {
            if (!zip_deflated_entry_md5(fp, dataOffset, compressedSize,
                                        uncompressedSize, hex)) {
                continue;
            }
        } else {
            continue;
        }
        matchIndex = md5_list_match_index(hex, md5List, NULL, md5Count);
        if (matchIndex >= 0) {
            int shouldUpdate;
            if (matched[matchIndex]) {
                if (!copy_match_path(outPaths[matchIndex], existingPath, (int)sizeof(existingPath))) {
                    continue;
                }
                slashPath = strstr(existingPath, "::");
                if (!slashPath) {
                    slashPath = existingPath;
                } else {
                    slashPath += 2;
                }
                shouldUpdate = is_better_zip_entry(name, slashPath);
            } else {
                shouldUpdate = 1;
            }
            if (!shouldUpdate) {
                continue;
            }
            if (!copy_virtual_match_path(zipPath, name, outPaths[matchIndex], ASSET_PATH_MAX)) {
                continue;
            }
            if (!matched[matchIndex]) {
                matched[matchIndex] = 1;
                ++foundCount;
            }
        }
    }
    fclose(fp);
    return foundCount;
}

static int iso_read_sector(FILE *fp, int raw2352, uint32_t sector,
                           unsigned char out[ASSET_ISO_SECTOR_SIZE]) {
    long offset = raw2352
        ? (long)sector * (long)ASSET_ISO_RAW_SECTOR_SIZE + (long)ASSET_ISO_RAW_DATA_OFFSET
        : (long)sector * (long)ASSET_ISO_SECTOR_SIZE;
    if (fseek(fp, offset, SEEK_SET) != 0) return 0;
    return fread(out, 1U, ASSET_ISO_SECTOR_SIZE, fp) == ASSET_ISO_SECTOR_SIZE;
}

static int iso_file_md5(FILE *fp, int raw2352, uint32_t lba, uint32_t size,
                        char outHex[33]) {
    unsigned char sector[ASSET_ISO_SECTOR_SIZE];
    uint32_t remaining = size;
    uint32_t sectorIndex = lba;
    AssetMd5Ctx ctx;
    md5_init(&ctx);
    while (remaining > 0U) {
        uint32_t chunk = remaining > ASSET_ISO_SECTOR_SIZE ? ASSET_ISO_SECTOR_SIZE : remaining;
        if (!iso_read_sector(fp, raw2352, sectorIndex, sector)) return 0;
        md5_update(&ctx, sector, chunk);
        remaining -= chunk;
        ++sectorIndex;
    }
    md5_final(&ctx, outHex);
    return 1;
}

static int iso_extract_file(FILE *fp, int raw2352, uint32_t lba, uint32_t size,
                            const char *outFilePath) {
    unsigned char sector[ASSET_ISO_SECTOR_SIZE];
    uint32_t remaining = size;
    uint32_t sectorIndex = lba;
    FILE *out;
    if (!fp || !outFilePath) return 0;
    out = fopen(outFilePath, "wb");
    if (!out) return 0;
    while (remaining > 0U) {
        uint32_t chunk = remaining > ASSET_ISO_SECTOR_SIZE ? ASSET_ISO_SECTOR_SIZE : remaining;
        if (!iso_read_sector(fp, raw2352, sectorIndex, sector) ||
            fwrite(sector, 1U, chunk, out) != chunk) {
            fclose(out);
            return 0;
        }
        remaining -= chunk;
        ++sectorIndex;
    }
    return fclose(out) == 0;
}

static int iso_parse_dir_record(const unsigned char *sector, int offset,
                                uint32_t *outLba, uint32_t *outSize,
                                int *outIsDir, char name[128]) {
    int recLen = sector[offset];
    int nameLen;
    int copyLen;
    if (recLen == 0 || offset + recLen > (int)ASSET_ISO_SECTOR_SIZE) return 0;
    *outLba = read_u32le(sector + offset + 2);
    *outSize = read_u32le(sector + offset + 10);
    *outIsDir = (sector[offset + 25] & 0x02) != 0;
    nameLen = sector[offset + 32];
    copyLen = nameLen < 127 ? nameLen : 127;
    memcpy(name, sector + offset + 33, (size_t)copyLen);
    name[copyLen] = '\0';
    {
        char *semi = strchr(name, ';');
        if (semi) *semi = '\0';
    }
    return recLen;
}

static int scan_iso_dir_by_md5(FILE *fp, int raw2352, uint32_t dirLba,
                               uint32_t dirSize, int depth,
                               const char *expectedMd5,
                               const char *isoPath,
                               char *outPath, int outPathLen) {
    unsigned char sector[ASSET_ISO_SECTOR_SIZE];
    uint32_t sectors = (dirSize + ASSET_ISO_SECTOR_SIZE - 1U) / ASSET_ISO_SECTOR_SIZE;
    uint32_t s;
    char bestName[128];
    int hasMatch = 0;
    if (depth > ASSET_ISO_MAX_DIR_DEPTH) return 0;
    bestName[0] = '\0';
    for (s = 0U; s < sectors; ++s) {
        int offset = 0;
        if (!iso_read_sector(fp, raw2352, dirLba + s, sector)) return 0;
        while (offset < (int)ASSET_ISO_SECTOR_SIZE) {
            uint32_t lba, size;
            int isDir;
            int recLen;
            char name[128];
            recLen = iso_parse_dir_record(sector, offset, &lba, &size, &isDir, name);
            if (recLen == 0) break;
            if (name[0] != '\0' && name[0] != 1) {
                if (isDir) {
                    if (scan_iso_dir_by_md5(fp, raw2352, lba, size, depth + 1,
                                            expectedMd5, isoPath, outPath, outPathLen)) {
                        return 1;
                    }
                } else if (size >= 16U && size <= ASSET_ZIP_MAX_ENTRY_BYTES) {
                    char hex[33];
                    /* Duplicate-hash tiebreak (mirrors the ZIP
                     * is_better_zip_entry logic in scan_zip_by_md5):
                     * walk the entire directory before returning so the
                     * reported virtual path is the case-insensitively
                     * smallest ISO entry name with the matching MD5
                     * (ISO 9660:1988 §7.5 — file identifiers compare
                     * case-insensitively at the filesystem level). */
                    if (iso_file_md5(fp, raw2352, lba, size, hex) &&
                        strcmp(hex, expectedMd5) == 0 &&
                        is_better_iso_entry(name, hasMatch ? bestName : NULL)) {
                        size_t nameLen = strlen(name);
                        if (nameLen < sizeof(bestName)) {
                            memcpy(bestName, name, nameLen + 1U);
                            hasMatch = 1;
                        }
                    }
                }
            }
            offset += recLen;
        }
    }
    if (!hasMatch) return 0;
    return copy_virtual_match_path(isoPath, bestName, outPath, outPathLen);
}

static int scan_iso_dir_by_md5_list(FILE *fp, int raw2352, uint32_t dirLba,
                                    uint32_t dirSize, int depth,
                                    const char *const *md5List,
                                    int md5Count,
                                    const char *isoPath,
                                    char outPaths[][ASSET_PATH_MAX],
                                    int matched[]) {
    unsigned char sector[ASSET_ISO_SECTOR_SIZE];
    uint32_t sectors = (dirSize + ASSET_ISO_SECTOR_SIZE - 1U) / ASSET_ISO_SECTOR_SIZE;
    uint32_t s;
    int foundCount = 0;
    /* Duplicate-hash tiebreak buffers (mirrors the ZIP
     * is_better_zip_entry tiebreak in scan_zip_by_md5_list). Sized to
     * ASSET_PATH_MAX / 128 worst-case bestNames = 4, which is well
     * above any realistic md5Count for a single game-data ISO (CSB,
     * DM2, Nexus, Theron all use <=4 unique file MD5s per ISO).
     * Caller-supplied md5Count larger than this falls back to
     * first-match-wins for those extra slots (conservative). */
    enum { ASSET_ISO_LIST_BEST_MAX = 4 };
    char bestNames[ASSET_ISO_LIST_BEST_MAX][128];
    int hasBest[ASSET_ISO_LIST_BEST_MAX];
    int i;
    for (i = 0; i < ASSET_ISO_LIST_BEST_MAX; ++i) {
        bestNames[i][0] = '\0';
        hasBest[i] = 0;
    }
    if (depth > ASSET_ISO_MAX_DIR_DEPTH) return 0;
    for (s = 0U; s < sectors; ++s) {
        int offset = 0;
        if (!iso_read_sector(fp, raw2352, dirLba + s, sector)) return foundCount;
        while (offset < (int)ASSET_ISO_SECTOR_SIZE) {
            uint32_t lba, size;
            int isDir;
            int recLen;
            char name[128];
            recLen = iso_parse_dir_record(sector, offset, &lba, &size, &isDir, name);
            if (recLen == 0) break;
            if (name[0] != '\0' && name[0] != 1) {
                if (isDir) {
                    foundCount += scan_iso_dir_by_md5_list(fp, raw2352, lba, size,
                                                           depth + 1, md5List,
                                                           md5Count, isoPath,
                                                           outPaths, matched);
                    if (foundCount >= md5Count) return foundCount;
                } else if (size >= 16U && size <= ASSET_ZIP_MAX_ENTRY_BYTES) {
                    char hex[33];
                    int matchIndex;
                    if (iso_file_md5(fp, raw2352, lba, size, hex)) {
                        matchIndex = md5_list_match_index(hex, md5List, matched, md5Count);
                        if (matchIndex >= 0 && matchIndex < ASSET_ISO_LIST_BEST_MAX &&
                            is_better_iso_entry(name, hasBest[matchIndex] ? bestNames[matchIndex] : NULL)) {
                            size_t nameLen = strlen(name);
                            if (nameLen < sizeof(bestNames[matchIndex])) {
                                memcpy(bestNames[matchIndex], name, nameLen + 1U);
                                hasBest[matchIndex] = 1;
                            }
                        }
                    }
                }
            }
            offset += recLen;
        }
    }
    /* Commit best-name slots now that the directory walk is complete,
     * so each md5List slot is written with the lexicographically
     * smallest matching ISO entry name. If a recursive child already
     * wrote a name, compare and overwrite only if ours is smaller. */
    for (i = 0; i < ASSET_ISO_LIST_BEST_MAX && i < md5Count; ++i) {
        char existingEntry[128];
        if (!hasBest[i]) continue;
        if (matched[i]) {
            const char* slash = strstr(outPaths[i], "::");
            const char* currentEntry = (slash != NULL) ? (slash + 2) : outPaths[i];
            size_t currentLen = strlen(currentEntry);
            if (currentLen >= sizeof(existingEntry)) {
                continue;
            }
            memcpy(existingEntry, currentEntry, currentLen + 1U);
            if (!is_better_iso_entry(bestNames[i], existingEntry)) continue;
            (void)copy_virtual_match_path(isoPath, bestNames[i],
                                          outPaths[i], ASSET_PATH_MAX);
        } else {
            if (copy_virtual_match_path(isoPath, bestNames[i],
                                        outPaths[i], ASSET_PATH_MAX)) {
                matched[i] = 1;
                ++foundCount;
                if (foundCount >= md5Count) return foundCount;
            }
        }
    }
    return foundCount;
}

static int iso_extract_entry_in_dir(FILE *fp, int raw2352, uint32_t dirLba,
                                    uint32_t dirSize, int depth,
                                    const char *entryName,
                                    const char *outFilePath) {
    unsigned char sector[ASSET_ISO_SECTOR_SIZE];
    uint32_t sectors = (dirSize + ASSET_ISO_SECTOR_SIZE - 1U) / ASSET_ISO_SECTOR_SIZE;
    uint32_t s;
    if (depth > ASSET_ISO_MAX_DIR_DEPTH) return 0;
    for (s = 0U; s < sectors; ++s) {
        int offset = 0;
        if (!iso_read_sector(fp, raw2352, dirLba + s, sector)) return 0;
        while (offset < (int)ASSET_ISO_SECTOR_SIZE) {
            uint32_t lba, size;
            int isDir;
            int recLen;
            char name[128];
            recLen = iso_parse_dir_record(sector, offset, &lba, &size, &isDir, name);
            if (recLen == 0) break;
            if (name[0] != '\0' && name[0] != 1) {
                if (isDir) {
                    if (iso_extract_entry_in_dir(fp, raw2352, lba, size, depth + 1,
                                                 entryName, outFilePath)) {
                        return 1;
                    }
                } else if (asset_casecmp(name, entryName) == 0) {
                    return iso_extract_file(fp, raw2352, lba, size, outFilePath);
                }
            }
            offset += recLen;
        }
    }
    return 0;
}

static int iso_extract_entry_to_path(const char *isoPath, const char *entryName,
                                     const char *outFilePath) {
    FILE *fp = fopen(isoPath, "rb");
    unsigned char pvd[ASSET_ISO_SECTOR_SIZE];
    int raw;
    if (!fp) return 0;
    for (raw = 0; raw <= 1; ++raw) {
        uint32_t rootLba, rootSize;
        if (!iso_read_sector(fp, raw, 16U, pvd)) continue;
        if (pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) continue;
        rootLba = read_u32le(pvd + 158);
        rootSize = read_u32le(pvd + 166);
        if (iso_extract_entry_in_dir(fp, raw, rootLba, rootSize, 0,
                                     entryName, outFilePath)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static int scan_iso_by_md5(const char *isoPath, const char *expectedMd5,
                           char *outPath, int outPathLen) {
    FILE *fp = fopen(isoPath, "rb");
    unsigned char pvd[ASSET_ISO_SECTOR_SIZE];
    int raw;
    char hex[33];
    if (!fp) return 0;
    /* Primary path: ISO 9660 directory walk. */
    for (raw = 0; raw <= 1; ++raw) {
        uint32_t rootLba, rootSize;
        if (!iso_read_sector(fp, raw, 16U, pvd)) continue;
        if (pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) continue;
        rootLba = read_u32le(pvd + 158);
        rootSize = read_u32le(pvd + 166);
        if (scan_iso_dir_by_md5(fp, raw, rootLba, rootSize, 0,
                                expectedMd5, isoPath, outPath, outPathLen)) {
            fclose(fp);
            return 1;
        }
    }
    /* Fallback: raw CD data image (no ISO 9660 PVD).
     * Some game assets ship as raw sector dumps (PC Engine CD, raw
     * Saturn tracks). Compute MD5 of the whole file and compare.
     * The reference hashes for these are documented in the asset
     * registry as the post-rip file content. */
    if (file_md5(isoPath, hex) && strcmp(hex, expectedMd5) == 0) {
        if (copy_match_path(isoPath, outPath, outPathLen)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static int scan_iso_by_md5_list(const char *isoPath, const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[]) {
    FILE *fp = fopen(isoPath, "rb");
    unsigned char pvd[ASSET_ISO_SECTOR_SIZE];
    int raw;
    int foundCount = 0;
    char hex[33];
    int i;
    if (!fp) return 0;
    /* Primary path: ISO 9660 directory walk. */
    for (raw = 0; raw <= 1; ++raw) {
        uint32_t rootLba, rootSize;
        if (!iso_read_sector(fp, raw, 16U, pvd)) continue;
        if (pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) continue;
        rootLba = read_u32le(pvd + 158);
        rootSize = read_u32le(pvd + 166);
        foundCount += scan_iso_dir_by_md5_list(fp, raw, rootLba, rootSize, 0,
                                               md5List, md5Count, isoPath,
                                               outPaths, matched);
        if (foundCount >= md5Count) {
            fclose(fp);
            return foundCount;
        }
    }
    /* Fallback: whole-file MD5 for raw CD images without ISO 9660 PVD. */
    if (file_md5(isoPath, hex)) {
        for (i = 0; i < md5Count; ++i) {
            if (matched[i]) continue;
            if (md5List[i] && strcmp(hex, md5List[i]) == 0) {
                if (copy_match_path(isoPath, outPaths[i],
                                    (int)ASSET_PATH_MAX)) {
                    matched[i] = 1;
                    ++foundCount;
                }
                break;
            }
        }
    }
    fclose(fp);
    return foundCount;
}

static const char *cue_ltrim(const char *s) {
    while (s && *s && isspace((unsigned char)*s)) {
        ++s;
    }
    return s ? s : "";
}

static int cue_starts_with_keyword(const char *line, const char *keyword) {
    size_t n;
    size_t i;
    if (!line || !keyword) return 0;
    n = strlen(keyword);
    for (i = 0U; i < n; ++i) {
        if (tolower((unsigned char)line[i]) !=
            tolower((unsigned char)keyword[i])) {
            return 0;
        }
    }
    return line[n] == '\0' || isspace((unsigned char)line[n]);
}

static int cue_extract_file_name(const char *line, char *out, size_t outSize) {
    const char *p;
    size_t i = 0U;
    if (!line || !out || outSize == 0U) return 0;
    out[0] = '\0';
    p = cue_ltrim(line);
    if (!cue_starts_with_keyword(p, "FILE")) return 0;
    p = cue_ltrim(p + 4);
    if (*p == '"') {
        ++p;
        while (*p && *p != '"' && i + 1U < outSize) {
            out[i++] = *p++;
        }
        out[i] = '\0';
        return *p == '"' && i > 0U;
    }
    while (*p && !isspace((unsigned char)*p) && i + 1U < outSize) {
        out[i++] = *p++;
    }
    out[i] = '\0';
    return i > 0U;
}

static int cue_track_is_data(const char *line) {
    const char *p;
    if (!line) return 0;
    p = cue_ltrim(line);
    if (!cue_starts_with_keyword(p, "TRACK")) return 0;
    return strstr(p, "MODE1/2048") != NULL ||
           strstr(p, "MODE1/2352") != NULL ||
           strstr(p, "MODE2/2352") != NULL ||
           strstr(p, "mode1/2048") != NULL ||
           strstr(p, "mode1/2352") != NULL ||
           strstr(p, "mode2/2352") != NULL;
}

static int path_is_absolute_local(const char *path) {
    if (!path || path[0] == '\0') return 0;
    if (path[0] == '/' || path[0] == '\\') return 1;
    if (isalpha((unsigned char)path[0]) && path[1] == ':') return 1;
    return 0;
}

static int path_parent_local(const char *path, char *out, size_t outSize) {
    const char *slash;
    const char *backslash;
    const char *sep;
    size_t n;
    if (!path || !out || outSize == 0U) return 0;
    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    sep = slash;
    if (backslash && (!sep || backslash > sep)) sep = backslash;
    if (!sep) {
        if (outSize < 2U) return 0;
        out[0] = '.';
        out[1] = '\0';
        return 1;
    }
    n = (size_t)(sep - path);
    if (n == 0U) n = 1U;
    if (n + 1U > outSize) return 0;
    memcpy(out, path, n);
    out[n] = '\0';
    return 1;
}

static int cue_resolve_payload_path(const char *cuePath,
                                    const char *fileName,
                                    char *out,
                                    size_t outSize) {
    char parent[ASSET_PATH_MAX];
    if (!cuePath || !fileName || !out || outSize == 0U) return 0;
    if (path_is_absolute_local(fileName)) {
        if (strlen(fileName) + 1U > outSize) return 0;
        strcpy(out, fileName);
        return 1;
    }
    if (!path_parent_local(cuePath, parent, sizeof(parent))) return 0;
    if (snprintf(out, outSize, "%s/%s", parent, fileName) >= (int)outSize) {
        return 0;
    }
    return 1;
}

static int scan_cue_by_md5(const char *cuePath, const char *expectedMd5,
                           char *outPath, int outPathLen) {
    FILE *fp;
    char line[1024];
    char currentFile[ASSET_PATH_MAX];
    currentFile[0] = '\0';
    if (!cuePath || !expectedMd5 || !outPath || outPathLen <= 0) return 0;
    fp = fopen(cuePath, "rb");
    if (!fp) return 0;
    while (fgets(line, sizeof(line), fp)) {
        char fileName[ASSET_PATH_MAX];
        char payloadPath[ASSET_PATH_MAX];
        if (cue_extract_file_name(line, fileName, sizeof(fileName))) {
            strcpy(currentFile, fileName);
            continue;
        }
        if (cue_track_is_data(line) && currentFile[0] != '\0' &&
            cue_resolve_payload_path(cuePath, currentFile,
                                     payloadPath, sizeof(payloadPath)) &&
            scan_iso_by_md5(payloadPath, expectedMd5, outPath, outPathLen)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static int scan_cue_by_md5_list(const char *cuePath,
                                const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[]) {
    FILE *fp;
    char line[1024];
    char currentFile[ASSET_PATH_MAX];
    int foundCount = 0;
    currentFile[0] = '\0';
    if (!cuePath || !md5List || md5Count <= 0 || !outPaths || !matched) {
        return 0;
    }
    fp = fopen(cuePath, "rb");
    if (!fp) return 0;
    while (fgets(line, sizeof(line), fp)) {
        char fileName[ASSET_PATH_MAX];
        char payloadPath[ASSET_PATH_MAX];
        if (cue_extract_file_name(line, fileName, sizeof(fileName))) {
            strcpy(currentFile, fileName);
            continue;
        }
        if (cue_track_is_data(line) && currentFile[0] != '\0' &&
            cue_resolve_payload_path(cuePath, currentFile,
                                     payloadPath, sizeof(payloadPath))) {
            foundCount += scan_iso_by_md5_list(payloadPath, md5List, md5Count,
                                               outPaths, matched);
            if (foundCount >= md5Count) {
                fclose(fp);
                return foundCount;
            }
        }
    }
    fclose(fp);
    return foundCount;
}

static int scan_container_by_md5(const char *path, const char *expectedMd5,
                                 char *outPath, int outPathLen) {
    if (is_zip_path(path)) {
        return scan_zip_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (is_iso_path(path)) {
        return scan_iso_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (is_cue_path(path)) {
        return scan_cue_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    return 0;
}

static int scan_container_by_md5_list(const char *path, const char *const *md5List,
                                      int md5Count,
                                      char outPaths[][ASSET_PATH_MAX],
                                      int matched[]) {
    if (is_zip_path(path)) {
        return scan_zip_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (is_iso_path(path)) {
        return scan_iso_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (is_cue_path(path)) {
        return scan_cue_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    return 0;
}

/* ── Recursive directory scanner ──────────────────────────────── */

#ifndef _WIN32
static int scan_dir_by_md5_list(const char *dir, const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[], int depth, int maxDepth) {
    DIR *d;
    struct dirent *ent;
    struct stat st;
    char path[ASSET_PATH_MAX];
    char hex[33];
    int foundCount = 0;
    int allowLargeWholeFile;

    if (depth > maxDepth) return 0;
    d = opendir(dir);
    if (!d) return 0;
    allowLargeWholeFile = md5_list_contains_large_whole_file_hash(md5List, md5Count);

    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        if (snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name) >= (int)sizeof(path)) {
            continue;
        }
        if (stat(path, &st) != 0) continue;

        if (S_ISREG(st.st_mode)) {
            int matchIndex;
            if (is_zip_path(path) || is_iso_path(path) || is_cue_path(path)) {
                foundCount += scan_container_by_md5_list(path, md5List, md5Count,
                                                         outPaths, matched);
                if (foundCount >= md5Count) {
                    closedir(d);
                    return foundCount;
                }
            }
            if (st.st_size > ASSET_SCAN_MAX_FILE_BYTES && !allowLargeWholeFile) {
                continue;
            }
            if (st.st_size < 16) continue;
            if (!file_md5(path, hex)) continue;
            matchIndex = md5_list_match_index(hex, md5List, matched, md5Count);
            if (matchIndex >= 0 &&
                copy_match_path(path, outPaths[matchIndex], ASSET_PATH_MAX)) {
                matched[matchIndex] = 1;
                ++foundCount;
                if (foundCount >= md5Count) {
                    closedir(d);
                    return foundCount;
                }
            }
        } else if (S_ISDIR(st.st_mode)) {
            foundCount += scan_dir_by_md5_list(path, md5List, md5Count,
                                               outPaths, matched,
                                               depth + 1, maxDepth);
            if (foundCount >= md5Count) {
                closedir(d);
                return foundCount;
            }
        }
    }
    closedir(d);
    return foundCount;
}

static int scan_dir(const char *dir, const char *expectedMd5,
                    char *outPath, int outPathLen, int depth, int maxDepth) {
    DIR *d;
    struct dirent *ent;
    struct stat st;
    char path[ASSET_PATH_MAX];
    char hex[33];

    if (depth > maxDepth) return 0;
    d = opendir(dir);
    if (!d) return 0;

    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        if (snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name) >= (int)sizeof(path)) {
            continue;
        }
        if (stat(path, &st) != 0) continue;

        if (S_ISREG(st.st_mode)) {
            if ((is_zip_path(path) || is_iso_path(path) || is_cue_path(path)) &&
                scan_container_by_md5(path, expectedMd5, outPath, outPathLen)) {
                closedir(d);
                return 1;
            }
            if (st.st_size > ASSET_SCAN_MAX_FILE_BYTES &&
                !is_known_large_whole_file_hash(expectedMd5)) {
                continue;
            }
            /* Skip files < 16 bytes (too small to be valid) */
            if (st.st_size < 16) continue;
            if (file_md5(path, hex) && strcmp(hex, expectedMd5) == 0) {
                if (!copy_match_path(path, outPath, outPathLen)) {
                    closedir(d);
                    return 0;
                }
                closedir(d);
                return 1;
            }
        } else if (S_ISDIR(st.st_mode)) {
            if (scan_dir(path, expectedMd5, outPath, outPathLen,
                         depth + 1, maxDepth)) {
                closedir(d);
                return 1;
            }
        }
    }
    closedir(d);
    return 0;
}
#else
static int scan_dir_by_md5_list(const char *dir, const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[], int depth, int maxDepth) {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char pattern[ASSET_PATH_MAX], path[ASSET_PATH_MAX];
    char hex[33];
    int foundCount = 0;
    int allowLargeWholeFile;
    if (depth > maxDepth) return 0;
    if (snprintf(pattern, sizeof(pattern), "%s\\*", dir) >= (int)sizeof(pattern)) return 0;
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    allowLargeWholeFile = md5_list_contains_large_whole_file_hash(md5List, md5Count);
    do {
        if (fd.cFileName[0] == '.') continue;
        if (snprintf(path, sizeof(path), "%s\\%s", dir, fd.cFileName) >= (int)sizeof(path)) continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            foundCount += scan_dir_by_md5_list(path, md5List, md5Count,
                                               outPaths, matched,
                                               depth + 1, maxDepth);
            if (foundCount >= md5Count) {
                FindClose(h);
                return foundCount;
            }
        } else {
            LARGE_INTEGER sz;
            int matchIndex;
            sz.LowPart = fd.nFileSizeLow;
            sz.HighPart = fd.nFileSizeHigh;
            if (is_zip_path(path) || is_iso_path(path) || is_cue_path(path)) {
                foundCount += scan_container_by_md5_list(path, md5List, md5Count,
                                                         outPaths, matched);
                if (foundCount >= md5Count) {
                    FindClose(h);
                    return foundCount;
                }
            }
            if ((sz.QuadPart > ASSET_SCAN_MAX_FILE_BYTES && !allowLargeWholeFile) ||
                sz.QuadPart < 16) {
                continue;
            }
            if (!file_md5(path, hex)) continue;
            matchIndex = md5_list_match_index(hex, md5List, matched, md5Count);
            if (matchIndex >= 0 &&
                copy_match_path(path, outPaths[matchIndex], ASSET_PATH_MAX)) {
                matched[matchIndex] = 1;
                ++foundCount;
                if (foundCount >= md5Count) {
                    FindClose(h);
                    return foundCount;
                }
            }
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return foundCount;
}

static int scan_dir(const char *dir, const char *expectedMd5,
                    char *outPath, int outPathLen, int depth, int maxDepth) {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char pattern[ASSET_PATH_MAX], path[ASSET_PATH_MAX];
    char hex[33];
    if (depth > maxDepth) return 0;
    if (snprintf(pattern, sizeof(pattern), "%s\\*", dir) >= (int)sizeof(pattern)) return 0;
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        if (fd.cFileName[0] == '.') continue;
        if (snprintf(path, sizeof(path), "%s\\%s", dir, fd.cFileName) >= (int)sizeof(path)) continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (scan_dir(path, expectedMd5, outPath, outPathLen, depth+1, maxDepth)) {
                FindClose(h); return 1;
            }
        } else {
            LARGE_INTEGER sz; sz.LowPart = fd.nFileSizeLow; sz.HighPart = fd.nFileSizeHigh;
            if ((is_zip_path(path) || is_iso_path(path) || is_cue_path(path)) &&
                scan_container_by_md5(path, expectedMd5, outPath, outPathLen)) {
                FindClose(h);
                return 1;
            }
            if ((sz.QuadPart > ASSET_SCAN_MAX_FILE_BYTES &&
                 !is_known_large_whole_file_hash(expectedMd5)) ||
                sz.QuadPart < 16) {
                continue;
            }
            if (file_md5(path, hex) && strcmp(hex, expectedMd5) == 0) {
                if (!copy_match_path(path, outPath, outPathLen)) {
                    FindClose(h);
                    return 0;
                }
                FindClose(h); return 1;
            }
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return 0;
}
#endif

/* ── Public API ───────────────────────────────────────────────── */

int asset_find_by_md5(const char *searchDir, const char *expectedMd5,
                      char *outPath, int outPathLen, int maxDepth) {
    char normalizedMd5[33];
    if (!searchDir || !expectedMd5 || !outPath || outPathLen <= 0) return 0;
    if (!normalize_md5(expectedMd5, normalizedMd5)) return 0;
    if (maxDepth < 0) maxDepth = 3;
    return scan_dir(searchDir, normalizedMd5, outPath, outPathLen, 0, maxDepth);
}

int asset_find_by_md5_list(const char *searchDir, const char *const *md5List,
                           char *outPath, int outPathLen,
                           int *outMatchIndex, int maxDepth) {
    if (!searchDir || !md5List || !outPath || outPathLen <= 0) return 0;
    if (maxDepth < 0) maxDepth = 3;
    for (int i = 0; md5List[i] != NULL; i++) {
        char normalizedMd5[33];
        if (!normalize_md5(md5List[i], normalizedMd5)) continue;
        if (scan_dir(searchDir, normalizedMd5, outPath, outPathLen, 0, maxDepth)) {
            if (outMatchIndex) *outMatchIndex = i;
            return 1;
        }
    }
    return 0;
}

int asset_find_all_by_md5_list(const char *searchDir, const char *const *md5List,
                               char outPaths[][ASSET_PATH_MAX],
                               int *outMatched, int maxMatches,
                               int maxDepth) {
    char (*normalized)[33];
    const char **normalizedPtrs;
    char (*normalizedPaths)[ASSET_PATH_MAX];
    int *normalizedMatched;
    int *originalIndices;
    int inputCount = 0;
    int normalizedCount = 0;
    int foundCount = 0;
    int i;

    if (!searchDir || !md5List || !outPaths || maxMatches <= 0) return 0;
    if (maxDepth < 0) maxDepth = 3;
    while (inputCount < maxMatches && md5List[inputCount] != NULL) {
        outPaths[inputCount][0] = '\0';
        if (outMatched) outMatched[inputCount] = 0;
        ++inputCount;
    }
    if (inputCount == 0) return 0;

    normalized = (char (*)[33])calloc((size_t)inputCount, sizeof(*normalized));
    normalizedPtrs = (const char**)calloc((size_t)inputCount + 1U, sizeof(*normalizedPtrs));
    normalizedPaths = (char (*)[ASSET_PATH_MAX])calloc((size_t)inputCount,
                                                       sizeof(*normalizedPaths));
    normalizedMatched = (int*)calloc((size_t)inputCount, sizeof(*normalizedMatched));
    originalIndices = (int*)calloc((size_t)inputCount, sizeof(*originalIndices));
    if (!normalized || !normalizedPtrs || !normalizedPaths ||
        !normalizedMatched || !originalIndices) {
        free(normalized);
        free(normalizedPtrs);
        free(normalizedPaths);
        free(normalizedMatched);
        free(originalIndices);
        return 0;
    }

    for (i = 0; i < inputCount; ++i) {
        if (normalize_md5(md5List[i], normalized[normalizedCount])) {
            normalizedPtrs[normalizedCount] = normalized[normalizedCount];
            originalIndices[normalizedCount] = i;
            ++normalizedCount;
        }
    }
    if (normalizedCount > 0) {
        foundCount = scan_dir_by_md5_list(searchDir, normalizedPtrs, normalizedCount,
                                          normalizedPaths, normalizedMatched,
                                          0, maxDepth);
        for (i = 0; i < normalizedCount; ++i) {
            if (normalizedMatched[i]) {
                int original = originalIndices[i];
                if (copy_match_path(normalizedPaths[i], outPaths[original], ASSET_PATH_MAX)) {
                    if (outMatched) outMatched[original] = 1;
                }
            }
        }
    }

    free(normalized);
    free(normalizedPtrs);
    free(normalizedPaths);
    free(normalizedMatched);
    free(originalIndices);
    return foundCount;
}

int asset_extract_virtual_path(const char *virtualPath, const char *outFilePath) {
    const char *sep;
    char container[ASSET_PATH_MAX];
    const char *entry;
    size_t containerLen;
    if (!virtualPath || !outFilePath) return 0;
    sep = strstr(virtualPath, "::");
    if (!sep) return 0;
    containerLen = (size_t)(sep - virtualPath);
    if (containerLen == 0U || containerLen >= sizeof(container)) return 0;
    memcpy(container, virtualPath, containerLen);
    container[containerLen] = '\0';
    entry = sep + 2;
    if (entry[0] == '\0') return 0;
    if (is_zip_path(container)) {
        return zip_extract_entry_to_path(container, entry, outFilePath);
    }
    /* CUE sheets may reference a data image with no .iso/.bin suffix.
     * Virtual paths are produced only after the scanner has already walked
     * the referenced payload as an ISO-like image, so extraction must not
     * reapply the filename-suffix gate here. */
    return iso_extract_entry_to_path(container, entry, outFilePath);
}
