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
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

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

/* ── Recursive directory scanner ──────────────────────────────── */

#ifndef _WIN32
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
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if (stat(path, &st) != 0) continue;

        if (S_ISREG(st.st_mode)) {
            /* Skip files > 10 MB (no dungeon/graphics file is that big) */
            if (st.st_size > 10 * 1024 * 1024) continue;
            /* Skip files < 16 bytes (too small to be valid) */
            if (st.st_size < 16) continue;
            if (file_md5(path, hex) && strcmp(hex, expectedMd5) == 0) {
                snprintf(outPath, outPathLen, "%s", path);
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
static int scan_dir(const char *dir, const char *expectedMd5,
                    char *outPath, int outPathLen, int depth, int maxDepth) {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char pattern[ASSET_PATH_MAX], path[ASSET_PATH_MAX];
    char hex[33];
    if (depth > maxDepth) return 0;
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        if (fd.cFileName[0] == '.') continue;
        snprintf(path, sizeof(path), "%s\\%s", dir, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (scan_dir(path, expectedMd5, outPath, outPathLen, depth+1, maxDepth)) {
                FindClose(h); return 1;
            }
        } else {
            LARGE_INTEGER sz; sz.LowPart = fd.nFileSizeLow; sz.HighPart = fd.nFileSizeHigh;
            if (sz.QuadPart > 10*1024*1024 || sz.QuadPart < 16) continue;
            if (file_md5(path, hex) && strcmp(hex, expectedMd5) == 0) {
                snprintf(outPath, outPathLen, "%s", path);
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
    if (!searchDir || !expectedMd5 || !outPath || outPathLen <= 0) return 0;
    if (strlen(expectedMd5) != 32) return 0;
    if (maxDepth < 0) maxDepth = 3;
    return scan_dir(searchDir, expectedMd5, outPath, outPathLen, 0, maxDepth);
}

int asset_find_by_md5_list(const char *searchDir, const char *const *md5List,
                           char *outPath, int outPathLen,
                           int *outMatchIndex, int maxDepth) {
    if (!searchDir || !md5List || !outPath || outPathLen <= 0) return 0;
    if (maxDepth < 0) maxDepth = 3;
    for (int i = 0; md5List[i] != NULL; i++) {
        if (strlen(md5List[i]) != 32) continue;
        if (scan_dir(searchDir, md5List[i], outPath, outPathLen, 0, maxDepth)) {
            if (outMatchIndex) *outMatchIndex = i;
            return 1;
        }
    }
    return 0;
}
