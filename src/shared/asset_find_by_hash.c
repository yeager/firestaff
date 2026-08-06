#if !defined(_WIN32) && !defined(_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE
#endif
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
#include <direct.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

/* Keep the scanner independently linkable: focused probes compile this file
 * directly, while the normal M11 build also exports the reader separately. */
#define FIRESTAFF_AMIGA_ADF_API static
#include "firestaff_amiga_adf.c"
#undef FIRESTAFF_AMIGA_ADF_API

#define ASSET_SCAN_MAX_FILE_BYTES (32LL * 1024LL * 1024LL)
#define ASSET_ZIP_MAX_ENTRY_BYTES (16U * 1024U * 1024U)
#define ASSET_TAR_MAX_ENTRY_BYTES (32U * 1024U * 1024U)
#define ASSET_GZIP_TAR_MAX_BYTES (128U * 1024U * 1024U)
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

/* ── Inline scan cache (avoids external library dependency) ───── */

#define SCAN_CACHE_MAX 16384

typedef struct {
    char     path[512];
    int64_t  mtime;
    int64_t  size;
    char     md5[33];
} ScanCacheEntry_I;

typedef struct {
    ScanCacheEntry_I entries[SCAN_CACHE_MAX];
    int count;
    int dirty;
} ScanCache_I;

static const char *scache_home(void) {
    const char *h = getenv("HOME");
#ifdef _WIN32
    if (!h) h = getenv("USERPROFILE");
#endif
    return h;
}

static int scache_path(char *buf, size_t len) {
    const char *h = scache_home();
    if (!h) return -1;
    snprintf(buf, len, "%s/.firestaff/cache/asset_scan_cache.dat", h);
    return 0;
}

static void scache_ensure_dir(void) {
    char dir[512];
    const char *h = scache_home();
    if (!h) return;
    snprintf(dir, sizeof(dir), "%s/.firestaff", h);
#ifdef _WIN32
    _mkdir(dir);
#else
    mkdir(dir, 0755);
#endif
    snprintf(dir, sizeof(dir), "%s/.firestaff/cache", h);
#ifdef _WIN32
    _mkdir(dir);
#else
    mkdir(dir, 0755);
#endif
}

static void scache_init(ScanCache_I *c) { memset(c, 0, sizeof(*c)); }

static int scache_load(ScanCache_I *c) {
    char p[512]; FILE *f; char line[640];
    scache_init(c);
    if (scache_path(p, sizeof(p)) != 0) return -1;
    f = fopen(p, "rb");
    if (!f) return -1;
    while (c->count < SCAN_CACHE_MAX && fgets(line, sizeof(line), f)) {
        ScanCacheEntry_I *e = &c->entries[c->count];
        char *t1 = strchr(line, '\t');
        if (!t1) continue;
        char *t2 = strchr(t1+1, '\t');
        if (!t2) continue;
        char *t3 = strchr(t2+1, '\t');
        if (!t3) continue;
        size_t pl = (size_t)(t1 - line);
        if (pl >= sizeof(e->path)) continue;
        memcpy(e->path, line, pl); e->path[pl] = '\0';
        e->mtime = strtoll(t1+1, NULL, 10);
        e->size = strtoll(t2+1, NULL, 10);
        size_t ml = strlen(t3+1);
        while (ml > 0 && (t3[ml]=='\n'||t3[ml]=='\r')) ml--;
        if (ml < 32) continue;
        memcpy(e->md5, t3+1, 32); e->md5[32] = '\0';
        c->count++;
    }
    fclose(f);
    return 0;
}

static int scache_save(const ScanCache_I *c) {
    char p[512]; FILE *f; int i;
    if (!c->dirty) return 0;
    if (scache_path(p, sizeof(p)) != 0) return -1;
    scache_ensure_dir();
    f = fopen(p, "wb");
    if (!f) return -1;
    for (i = 0; i < c->count; i++) {
        const ScanCacheEntry_I *e = &c->entries[i];
        fprintf(f, "%s\t%lld\t%lld\t%s\n",
                e->path, (long long)e->mtime, (long long)e->size, e->md5);
    }
    fclose(f);
    return 0;
}

static int scache_lookup(const ScanCache_I *c,
                         const char *path, int64_t mt, int64_t sz,
                         char out[33]) {
    int i;
    for (i = 0; i < c->count; i++) {
        if (strcmp(c->entries[i].path, path) == 0 &&
            c->entries[i].mtime == mt && c->entries[i].size == sz) {
            memcpy(out, c->entries[i].md5, 33);
            return 1;
        }
    }
    return 0;
}

static void scache_put(ScanCache_I *c,
                       const char *path, int64_t mt, int64_t sz,
                       const char *md5) {
    int i;
    for (i = 0; i < c->count; i++) {
        if (strcmp(c->entries[i].path, path) == 0) {
            c->entries[i].mtime = mt;
            c->entries[i].size = sz;
            memcpy(c->entries[i].md5, md5, 33);
            c->dirty = 1;
            return;
        }
    }
    if (c->count < SCAN_CACHE_MAX) {
        ScanCacheEntry_I *e = &c->entries[c->count++];
        size_t pathLen = strlen(path);
        if (pathLen >= sizeof(e->path)) pathLen = sizeof(e->path) - 1;
        memcpy(e->path, path, pathLen);
        e->path[pathLen] = '\0';
        e->mtime = mt; e->size = sz;
        memcpy(e->md5, md5, 33);
        c->dirty = 1;
    }
}

static ScanCache_I *s_scan_cache;

static int file_md5_raw(const char *path, char outHex[33]) {
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

static int file_md5(const char *path, char outHex[33]) {
    if (s_scan_cache) {
        struct stat st;
        if (stat(path, &st) == 0) {
            if (scache_lookup(s_scan_cache, path,
                              (int64_t)st.st_mtime, (int64_t)st.st_size,
                              outHex))
                return 1;
            if (file_md5_raw(path, outHex)) {
                scache_put(s_scan_cache, path,
                           (int64_t)st.st_mtime, (int64_t)st.st_size,
                           outHex);
                return 1;
            }
            return 0;
        }
    }
    return file_md5_raw(path, outHex);
}

int asset_file_matches_md5(const char *path, const char *expectedMd5) {
    char actual[33];

    return path && expectedMd5 && strlen(expectedMd5) == 32U &&
        file_md5(path, actual) && strcmp(actual, expectedMd5) == 0;
}

int asset_file_md5_hex(const char *path, char outHex[33]) {
    if (!outHex) return 0;
    outHex[0] = '\0';
    return path && file_md5(path, outHex);
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

typedef enum {
    ASSET_CONTAINER_NONE = 0,
    ASSET_CONTAINER_ZIP,
    ASSET_CONTAINER_ISO,
    ASSET_CONTAINER_CUE,
    ASSET_CONTAINER_TAR,
    ASSET_CONTAINER_TGZ,
    ASSET_CONTAINER_GZIP,
    ASSET_CONTAINER_LHA,
    ASSET_CONTAINER_CHD,
    ASSET_CONTAINER_ADF,
    ASSET_CONTAINER_ATARI_ST,
    ASSET_CONTAINER_ATARI_MSA,
    ASSET_CONTAINER_EXTERNAL
} AssetContainerKind;

static int is_zip_path(const char *path) {
    return has_case_suffix(path, ".zip") || has_case_suffix(path, ".cbz") ||
           has_case_suffix(path, ".zipx") || has_case_suffix(path, ".fsart") ||
           has_case_suffix(path, ".pk3") || has_case_suffix(path, ".jar") ||
           has_case_suffix(path, ".apk") || has_case_suffix(path, ".ipa") ||
           has_case_suffix(path, ".xpi") || has_case_suffix(path, ".whl") ||
           has_case_suffix(path, ".wsz") || has_case_suffix(path, ".kmz") ||
           has_case_suffix(path, ".pk4") || has_case_suffix(path, ".nupkg");
}

static int is_iso_path(const char *path) {
    return has_case_suffix(path, ".iso") || has_case_suffix(path, ".bin") ||
           has_case_suffix(path, ".img") || has_case_suffix(path, ".cdr") ||
           has_case_suffix(path, ".toast") || has_case_suffix(path, ".raw") ||
           has_case_suffix(path, ".mdf");
}

static int is_cue_path(const char *path) {
    return has_case_suffix(path, ".cue");
}

static int is_tar_path(const char *path) {
    return has_case_suffix(path, ".tar");
}

static int is_tgz_path(const char *path) {
    return has_case_suffix(path, ".tgz") || has_case_suffix(path, ".tar.gz");
}

static int is_external_tar_archive_path(const char *path) {
    return has_case_suffix(path, ".tbz") ||
           has_case_suffix(path, ".tbz2") ||
           has_case_suffix(path, ".tar.bz2") ||
           has_case_suffix(path, ".txz") ||
           has_case_suffix(path, ".tar.xz") ||
           has_case_suffix(path, ".tlz") ||
           has_case_suffix(path, ".tar.lz") ||
           has_case_suffix(path, ".tar.lzma") ||
           has_case_suffix(path, ".tzst") ||
           has_case_suffix(path, ".tar.zst") ||
           has_case_suffix(path, ".taz") ||
           has_case_suffix(path, ".tar.z");
}

static int is_gzip_path(const char *path) {
    return (has_case_suffix(path, ".gz") || has_case_suffix(path, ".gzip")) &&
           !is_tgz_path(path);
}

static int is_lha_path(const char *path) {
    return has_case_suffix(path, ".lha") || has_case_suffix(path, ".lzh") ||
           has_case_suffix(path, ".lzs");
}

static int is_chd_path(const char *path) {
    return has_case_suffix(path, ".chd");
}

static int is_adf_path(const char *path) {
    return has_case_suffix(path, ".adf");
}

/* A plain .ST image is a sector-for-sector GEMDOS floppy, unlike protected
 * .STX and compressed .MSA media.  It is safe to parse without invoking an
 * emulator or a host archive tool. */
static int is_atari_st_path(const char *path) {
    return has_case_suffix(path, ".st");
}

static int is_atari_msa_path(const char *path) {
    return has_case_suffix(path, ".msa");
}

/* KryoFlux streams store one raw flux track per file as
 * "<track>.<side>.raw" (for example "79.1.raw").  That transport is not an
 * ISO image and its individual track files cannot directly contain a named
 * GRAPHICS.DAT/DUNGEON.DAT payload.  In particular, do not make an enclosing
 * archive scan extract and MD5 every track as though it were an ordinary
 * game file: an Amiga set can contain hundreds of them beside an already
 * usable .adf.  This deliberately applies only to archive members; a plain
 * top-level .raw keeps the existing ISO/BIN image path. */
static int is_kryoflux_raw_track_path(const char *path) {
    size_t length;
    size_t side_begin;
    size_t track_begin;
    if (!path || !has_case_suffix(path, ".raw")) return 0;
    length = strlen(path);
    if (length < 8U) return 0; /* minimum: 0.0.raw */
    side_begin = length - 4U;
    while (side_begin > 0U &&
           isdigit((unsigned char)path[side_begin - 1U])) {
        --side_begin;
    }
    if (side_begin == length - 4U || side_begin == 0U ||
        path[side_begin - 1U] != '.') return 0;
    track_begin = side_begin - 1U;
    while (track_begin > 0U &&
           isdigit((unsigned char)path[track_begin - 1U])) {
        --track_begin;
    }
    return track_begin < side_begin - 1U;
}

static int is_external_archive_path(const char *path) {
    return is_external_tar_archive_path(path) ||
           has_case_suffix(path, ".7z") || has_case_suffix(path, ".rar") ||
           has_case_suffix(path, ".arj") || has_case_suffix(path, ".arc") ||
           has_case_suffix(path, ".cab") || has_case_suffix(path, ".zoo") ||
           has_case_suffix(path, ".ace") || has_case_suffix(path, ".ha") ||
           has_case_suffix(path, ".uc2") || has_case_suffix(path, ".zpaq") ||
           has_case_suffix(path, ".pak") || has_case_suffix(path, ".sit") ||
           has_case_suffix(path, ".sitx") || has_case_suffix(path, ".dmg") ||
           has_case_suffix(path, ".hqx") || has_case_suffix(path, ".sea") ||
           has_case_suffix(path, ".dms") || has_case_suffix(path, ".lzx") ||
           has_case_suffix(path, ".lha") || has_case_suffix(path, ".lbr") ||
           has_case_suffix(path, ".cpio") ||
           has_case_suffix(path, ".ar") || has_case_suffix(path, ".deb") ||
           has_case_suffix(path, ".rpm") || has_case_suffix(path, ".xar") ||
           has_case_suffix(path, ".bz2") || has_case_suffix(path, ".xz") ||
           has_case_suffix(path, ".zst") || has_case_suffix(path, ".lzma") ||
           has_case_suffix(path, ".lz") || has_case_suffix(path, ".z") ||
           has_case_suffix(path, ".rz") ||
           has_case_suffix(path, ".adf") || has_case_suffix(path, ".adz") ||
           has_case_suffix(path, ".st") || has_case_suffix(path, ".stx") ||
           has_case_suffix(path, ".ipf") ||
           has_case_suffix(path, ".hfe") || has_case_suffix(path, ".hdm") ||
           has_case_suffix(path, ".dsk") || has_case_suffix(path, ".ima") ||
           has_case_suffix(path, ".nrg") || has_case_suffix(path, ".cdi") ||
           has_case_suffix(path, ".ccd") || has_case_suffix(path, ".sub") ||
           has_case_suffix(path, ".cso") || has_case_suffix(path, ".isz") ||
           has_case_suffix(path, ".ecm") || has_case_suffix(path, ".mds") ||
           has_case_suffix(path, ".mdx") || has_case_suffix(path, ".b5t") ||
           has_case_suffix(path, ".b6t") || has_case_suffix(path, ".bwt") ||
           has_case_suffix(path, ".pdi") || has_case_suffix(path, ".gdi") ||
           has_case_suffix(path, ".toc");
}

static AssetContainerKind asset_container_kind_from_suffix(const char *path) {
    if (is_zip_path(path)) return ASSET_CONTAINER_ZIP;
    if (is_iso_path(path)) return ASSET_CONTAINER_ISO;
    if (is_cue_path(path)) return ASSET_CONTAINER_CUE;
    if (is_tar_path(path)) return ASSET_CONTAINER_TAR;
    if (is_tgz_path(path)) return ASSET_CONTAINER_TGZ;
    if (is_gzip_path(path)) return ASSET_CONTAINER_GZIP;
    if (is_lha_path(path)) return ASSET_CONTAINER_LHA;
    if (is_chd_path(path)) return ASSET_CONTAINER_CHD;
    if (is_adf_path(path)) return ASSET_CONTAINER_ADF;
    if (is_atari_st_path(path)) return ASSET_CONTAINER_ATARI_ST;
    if (is_atari_msa_path(path)) return ASSET_CONTAINER_ATARI_MSA;
    if (is_external_archive_path(path)) return ASSET_CONTAINER_EXTERNAL;
    return ASSET_CONTAINER_NONE;
}

static int asset_magic_at(FILE *fp, long offset, const char *magic, size_t magicLen) {
    unsigned char buf[8];
    if (!fp || !magic || magicLen == 0U || magicLen > sizeof(buf)) return 0;
    if (fseek(fp, offset, SEEK_SET) != 0) return 0;
    if (fread(buf, 1U, magicLen, fp) != magicLen) return 0;
    return memcmp(buf, magic, magicLen) == 0;
}

static AssetContainerKind asset_container_kind_from_magic(const char *path) {
    FILE *fp;
    unsigned char header[32];
    size_t got;
    if (!path) return ASSET_CONTAINER_NONE;
    fp = fopen(path, "rb");
    if (!fp) return ASSET_CONTAINER_NONE;
    got = fread(header, 1U, sizeof(header), fp);
    if (got >= 4U && header[0] == 0x50 && header[1] == 0x4b &&
        (header[2] == 0x03 || header[2] == 0x05 || header[2] == 0x07) &&
        (header[3] == 0x04 || header[3] == 0x06 || header[3] == 0x08)) {
        fclose(fp);
        return ASSET_CONTAINER_ZIP;
    }
    if (got >= 2U && header[0] == 0x1f && header[1] == 0x8b) {
        fclose(fp);
        return ASSET_CONTAINER_GZIP;
    }
    if (got >= 4U && memcmp(header, "DOS\0", 4U) == 0) {
        fclose(fp);
        return ASSET_CONTAINER_ADF;
    }
    if (got >= 2U && header[0] == 0x0eU && header[1] == 0x0fU) {
        fclose(fp);
        return ASSET_CONTAINER_ATARI_MSA;
    }
    if ((got >= 3U && memcmp(header, "BZh", 3U) == 0) ||
        (got >= 6U && memcmp(header, "\xfd" "7zXZ\0", 6U) == 0) ||
        (got >= 4U && header[0] == 0x28 && header[1] == 0xb5 &&
         header[2] == 0x2f && header[3] == 0xfd) ||
        (got >= 4U && memcmp(header, "LZIP", 4U) == 0) ||
        (got >= 4U && memcmp(header, "LRZI", 4U) == 0) ||
        (got >= 4U && memcmp(header, "RZIP", 4U) == 0) ||
        (got >= 4U && memcmp(header, "xar!", 4U) == 0) ||
        (got >= 4U && memcmp(header, "MSCF", 4U) == 0) ||
        (got >= 4U && memcmp(header, "CISO", 4U) == 0) ||
        (got >= 4U && memcmp(header, "ECM\0", 4U) == 0) ||
        (got >= 4U && memcmp(header, "ZPAQ", 4U) == 0) ||
        (got >= 2U && header[0] == 0x60 && header[1] == 0xea) ||
        (got >= 2U && header[0] == 0x1f &&
         (header[1] == 0x9d || header[1] == 0xa0)) ||
        (got >= 8U && memcmp(header, "!<arch>\n", 8U) == 0) ||
        (got >= 6U && (memcmp(header, "070701", 6U) == 0 ||
                       memcmp(header, "070702", 6U) == 0 ||
                       memcmp(header, "070707", 6U) == 0))) {
        fclose(fp);
        return ASSET_CONTAINER_EXTERNAL;
    }
    if (got >= 7U && header[0] > 0U &&
        header[2] == '-' && header[3] == 'l' && header[4] == 'h') {
        fclose(fp);
        return ASSET_CONTAINER_LHA;
    }
    if (got >= 8U && memcmp(header, "MComprHD", 8U) == 0) {
        fclose(fp);
        return ASSET_CONTAINER_CHD;
    }
    if (got >= 6U && memcmp(header, "7z\xbc\xaf\x27\x1c", 6U) == 0) {
        fclose(fp);
        return ASSET_CONTAINER_EXTERNAL;
    }
    if ((got >= 7U && memcmp(header, "Rar!\x1a\x07\x00", 7U) == 0) ||
        (got >= 8U && memcmp(header, "Rar!\x1a\x07\x01\x00", 8U) == 0)) {
        fclose(fp);
        return ASSET_CONTAINER_EXTERNAL;
    }
    if (asset_magic_at(fp, 257L, "ustar", 5U)) {
        fclose(fp);
        return ASSET_CONTAINER_TAR;
    }
    if (asset_magic_at(fp, 16L * 2048L + 1L, "CD001", 5U) ||
        asset_magic_at(fp, 16L * 2352L + 16L + 1L, "CD001", 5U)) {
        fclose(fp);
        return ASSET_CONTAINER_ISO;
    }
    fclose(fp);
    return ASSET_CONTAINER_NONE;
}

static AssetContainerKind asset_container_kind_for_path(const char *path) {
    AssetContainerKind kind = asset_container_kind_from_suffix(path);
    AssetContainerKind magicKind = asset_container_kind_from_magic(path);
    if (kind == ASSET_CONTAINER_TGZ || kind == ASSET_CONTAINER_CUE) return kind;
    if (magicKind != ASSET_CONTAINER_NONE) return magicKind;
    return kind;
}

static int is_supported_container_path(const char *path) {
    return asset_container_kind_for_path(path) != ASSET_CONTAINER_NONE;
}

static int is_known_large_whole_file_hash(const char *expectedMd5) {
    static const char *const largeHashes[] = {
        "e88d60859f65f08fa622e1992b02280f", /* Nexus Saturn data image */
        "96e511c8d36ccbe30a48ba36c59df194", /* Nexus Saturn data image */
        "b7afb338ad31be1025b53f9aff12d73a", /* Theron's Quest JP Track 02 */
        "f23601102138f87c33025877767ebf76", /* Theron's Quest US Track 02 */
        "397039af02d50d15c70b74088eb8a1cb", /* Theron's Quest JP Rev 1 Track 02 ISO */
        "ceb02343868f80cec899e9b239aff2da", /* Theron's Quest US Track 02 ISO */
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
    for (;;) {
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
        zs.next_out = outBuf;
        zs.avail_out = (uInt)sizeof(outBuf);
        /* The final compressed input can share a buffer with earlier bytes.
         * Z_NO_FLUSH lets zlib consume that tail and report STREAM_END;
         * switching to Z_FINISH merely because no more file bytes remain
         * can return Z_BUF_ERROR before the tail is consumed. */
        ret = inflate(&zs, Z_NO_FLUSH);
        if (ret == Z_BUF_ERROR && remaining == 0U && zs.avail_in == 0U &&
            produced == uncompressedSize) {
            break;
        }
        if (ret != Z_OK && ret != Z_STREAM_END) {
            /* Z_BUF_ERROR is only benign while more input remains. With no
             * input left it means the archive entry was truncated. */
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
        if (ret == Z_STREAM_END) {
            break;
        }
        /* A highly-compressible entry can consume its last input before it
         * has drained zlib's pending output. Keep calling inflate while the
         * output buffer was filled; only a no-progress call is malformed. */
        if (zs.avail_in == 0 && remaining == 0U && chunk == 0U) {
            inflateEnd(&zs);
            fclose(out);
            return 0;
        }
    }
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

/* Read one ordinary ZIP member into bounded memory.  Nested ADF/ST/MSA
 * images need their complete disk image for the filesystem visitors; do not
 * shell out merely because the image is wrapped in a deflated ZIP member. */
static uint8_t *zip_read_entry_bytes(FILE *fp, uint16_t method,
                                     uint32_t dataOffset,
                                     uint32_t compressedSize,
                                     uint32_t uncompressedSize,
                                     size_t *outSize) {
    uint8_t *bytes;
    if (!fp || !outSize || uncompressedSize == 0U ||
        uncompressedSize > ASSET_ZIP_MAX_ENTRY_BYTES) return NULL;
    bytes = (uint8_t *)malloc(uncompressedSize);
    if (!bytes || fseek(fp, (long)dataOffset, SEEK_SET) != 0) {
        free(bytes);
        return NULL;
    }
    if (method == 0U) {
        if (fread(bytes, 1U, uncompressedSize, fp) != uncompressedSize) {
            free(bytes);
            return NULL;
        }
        *outSize = uncompressedSize;
        return bytes;
    }
#ifdef FIRESTAFF_HAS_ZLIB
    if (method == 8U) {
        uint8_t inBuf[8192];
        uint32_t remaining = compressedSize;
        uint32_t produced = 0U;
        z_stream zs;
        int ret = Z_OK;
        memset(&zs, 0, sizeof(zs));
        if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) {
            free(bytes);
            return NULL;
        }
        while (ret != Z_STREAM_END) {
            size_t chunk;
            if (zs.avail_in == 0U && remaining > 0U) {
                chunk = remaining > sizeof(inBuf) ? sizeof(inBuf) : (size_t)remaining;
                if (fread(inBuf, 1U, chunk, fp) != chunk) break;
                remaining -= (uint32_t)chunk;
                zs.next_in = inBuf;
                zs.avail_in = (uInt)chunk;
            }
            if (produced >= uncompressedSize) break;
            zs.next_out = bytes + produced;
            zs.avail_out = (uInt)(uncompressedSize - produced);
            ret = inflate(&zs, Z_NO_FLUSH);
            produced = uncompressedSize - zs.avail_out;
            if (ret != Z_OK && ret != Z_STREAM_END) break;
            if (zs.avail_in == 0U && remaining == 0U && ret != Z_STREAM_END) break;
        }
        inflateEnd(&zs);
        if (ret == Z_STREAM_END && produced == uncompressedSize) {
            *outSize = uncompressedSize;
            return bytes;
        }
    }
#else
    (void)compressedSize;
#endif
    free(bytes);
    return NULL;
}

static uint8_t *zip_load_entry_bytes(const char *zipPath, const char *entryName,
                                     size_t *outSize) {
    FILE *fp;
    long fileSize, searchStart, eocdOffset = -1;
    unsigned char *tail;
    size_t tailSize;
    uint32_t cdOffset = 0U, cdSize = 0U, pos;
    uint16_t entryCount = 0U, i;
    if (!zipPath || !entryName || !outSize) return NULL;
    fp = fopen(zipPath, "rb");
    if (!fp || fseek(fp, 0L, SEEK_END) != 0 || (fileSize = ftell(fp)) < 22L) {
        if (fp) fclose(fp);
        return NULL;
    }
    tailSize = (size_t)(fileSize < 65557L ? fileSize : 65557L);
    searchStart = fileSize - (long)tailSize;
    tail = (unsigned char *)malloc(tailSize);
    if (!tail || fseek(fp, searchStart, SEEK_SET) != 0 ||
        fread(tail, 1U, tailSize, fp) != tailSize) {
        free(tail);
        fclose(fp);
        return NULL;
    }
    for (long j = (long)tailSize - 22L; j >= 0L; --j) {
        if (read_u32le(tail + j) == 0x06054b50U) {
            entryCount = read_u16le(tail + j + 10);
            cdSize = read_u32le(tail + j + 12);
            cdOffset = read_u32le(tail + j + 16);
            eocdOffset = searchStart + j;
            break;
        }
    }
    free(tail);
    if (eocdOffset < 0L || cdOffset + cdSize > (uint32_t)fileSize) {
        fclose(fp);
        return NULL;
    }
    pos = cdOffset;
    for (i = 0U; i < entryCount && pos + 46U <= cdOffset + cdSize; ++i) {
        unsigned char hdr[46], local[30];
        uint16_t method, nameLen, extraLen, commentLen, localNameLen, localExtraLen;
        uint32_t compressedSize, uncompressedSize, localOffset, dataOffset;
        char name[256];
        if (fseek(fp, (long)pos, SEEK_SET) != 0 ||
            fread(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr) ||
            read_u32le(hdr) != 0x02014b50U) break;
        method = read_u16le(hdr + 10);
        compressedSize = read_u32le(hdr + 20);
        uncompressedSize = read_u32le(hdr + 24);
        nameLen = read_u16le(hdr + 28);
        extraLen = read_u16le(hdr + 30);
        commentLen = read_u16le(hdr + 32);
        localOffset = read_u32le(hdr + 42);
        if (nameLen == 0U || nameLen >= sizeof(name) ||
            fread(name, 1U, nameLen, fp) != nameLen) break;
        name[nameLen] = '\0';
        pos += 46U + nameLen + extraLen + commentLen;
        if (strcmp(name, entryName) != 0 ||
            fseek(fp, (long)localOffset, SEEK_SET) != 0 ||
            fread(local, 1U, sizeof(local), fp) != sizeof(local) ||
            read_u32le(local) != 0x04034b50U) continue;
        localNameLen = read_u16le(local + 26);
        localExtraLen = read_u16le(local + 28);
        dataOffset = localOffset + 30U + localNameLen + localExtraLen;
        {
            uint8_t *bytes = zip_read_entry_bytes(fp, method, dataOffset,
                                                   compressedSize, uncompressedSize,
                                                   outSize);
            fclose(fp);
            return bytes;
        }
    }
    fclose(fp);
    return NULL;
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

/* A ZIP can itself carry a native disk image.  The regular ZIP reader owns
 * ordinary members, while the external reader already knows how to walk an
 * ADF/ST/MSA filesystem after extracting that member.  Keep the fallback
 * here rather than treating a ZIP-distributed disk as a flat opaque file. */
static int scan_external_archive_by_md5(const char *archivePath,
                                        const char *expectedMd5,
                                        char *outPath,
                                        int outPathLen);
static int scan_external_archive_by_md5_list(const char *archivePath,
                                             const char *const *md5List,
                                             int md5Count,
                                             char outPaths[][ASSET_PATH_MAX],
                                             int matched[]);
static int external_tool_available_for_path(const char *path);

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
    struct stat zipStat;
    int64_t zipMtime = 0, zipSize = 0;
    int haveZipStat = 0;

    if (s_scan_cache && stat(zipPath, &zipStat) == 0) {
        zipMtime = (int64_t)zipStat.st_mtime;
        zipSize = (int64_t)zipStat.st_size;
        haveZipStat = 1;
    }

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
        {
            char cacheKey[512];
            int cached = 0;
            if (haveZipStat && s_scan_cache) {
                snprintf(cacheKey, sizeof(cacheKey), "%s::%s", zipPath, name);
                cached = scache_lookup(s_scan_cache, cacheKey,
                                       zipMtime, zipSize, hex);
            }
            if (!cached) {
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
                if (haveZipStat && s_scan_cache) {
                    snprintf(cacheKey, sizeof(cacheKey), "%s::%s", zipPath, name);
                    scache_put(s_scan_cache, cacheKey, zipMtime, zipSize, hex);
                }
            }
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
                               const char *directory,
                               const char *expectedMd5,
                               const char *isoPath,
                               char *outPath, int outPathLen) {
    unsigned char sector[ASSET_ISO_SECTOR_SIZE];
    uint32_t sectors = (dirSize + ASSET_ISO_SECTOR_SIZE - 1U) / ASSET_ISO_SECTOR_SIZE;
    uint32_t s;
    char bestName[256];
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
            char fullName[256];
            recLen = iso_parse_dir_record(sector, offset, &lba, &size, &isDir, name);
            if (recLen == 0) break;
            if (name[0] != '\0' && name[0] != 1) {
                if (directory && directory[0] != '\0') {
                    if (snprintf(fullName, sizeof(fullName), "%s/%s",
                                 directory, name) >= (int)sizeof(fullName)) {
                        return 0;
                    }
                } else {
                    if (snprintf(fullName, sizeof(fullName), "%s", name) >=
                        (int)sizeof(fullName)) {
                        return 0;
                    }
                }
                if (isDir) {
                    if (scan_iso_dir_by_md5(fp, raw2352, lba, size, depth + 1,
                                            fullName, expectedMd5, isoPath,
                                            outPath, outPathLen)) {
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
                        is_better_iso_entry(fullName, hasMatch ? bestName : NULL)) {
                        size_t nameLen = strlen(fullName);
                        if (nameLen < sizeof(bestName)) {
                            memcpy(bestName, fullName, nameLen + 1U);
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
                                    const char *directory,
                                    const char *const *md5List,
                                    int md5Count,
                                    const char *isoPath,
                                    char outPaths[][ASSET_PATH_MAX],
                                    int matched[]) {
    unsigned char sector[ASSET_ISO_SECTOR_SIZE];
    uint32_t sectors = (dirSize + ASSET_ISO_SECTOR_SIZE - 1U) / ASSET_ISO_SECTOR_SIZE;
    uint32_t s;
    int foundCount = 0;
    /* The old four-slot bound predates CSB's English and Japanese FM Towns
     * profiles and skipped their MD5 slots during one-pass ISO scans. */
    enum { ASSET_ISO_LIST_BEST_MAX = 16 };
    char bestNames[ASSET_ISO_LIST_BEST_MAX][256];
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
            char fullName[256];
            recLen = iso_parse_dir_record(sector, offset, &lba, &size, &isDir, name);
            if (recLen == 0) break;
            if (name[0] != '\0' && name[0] != 1) {
                if (directory && directory[0] != '\0') {
                    if (snprintf(fullName, sizeof(fullName), "%s/%s",
                                 directory, name) >= (int)sizeof(fullName)) {
                        return foundCount;
                    }
                } else {
                    if (snprintf(fullName, sizeof(fullName), "%s", name) >=
                        (int)sizeof(fullName)) {
                        return foundCount;
                    }
                }
                if (isDir) {
                    foundCount += scan_iso_dir_by_md5_list(fp, raw2352, lba, size,
                                                           depth + 1, fullName, md5List,
                                                           md5Count, isoPath,
                                                           outPaths, matched);
                    if (foundCount >= md5Count) return foundCount;
                } else if (size >= 16U && size <= ASSET_ZIP_MAX_ENTRY_BYTES) {
                    char hex[33];
                    int matchIndex;
                    if (iso_file_md5(fp, raw2352, lba, size, hex)) {
                        matchIndex = md5_list_match_index(hex, md5List, matched, md5Count);
                        if (matchIndex >= 0 && matchIndex < ASSET_ISO_LIST_BEST_MAX &&
                            is_better_iso_entry(fullName, hasBest[matchIndex] ? bestNames[matchIndex] : NULL)) {
                            size_t nameLen = strlen(fullName);
                            if (nameLen < sizeof(bestNames[matchIndex])) {
                                memcpy(bestNames[matchIndex], fullName, nameLen + 1U);
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
                                    const char *directory,
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
            char fullName[256];
            recLen = iso_parse_dir_record(sector, offset, &lba, &size, &isDir, name);
            if (recLen == 0) break;
            if (name[0] != '\0' && name[0] != 1) {
                if (directory && directory[0] != '\0') {
                    if (snprintf(fullName, sizeof(fullName), "%s/%s",
                                 directory, name) >= (int)sizeof(fullName)) {
                        return 0;
                    }
                } else {
                    if (snprintf(fullName, sizeof(fullName), "%s", name) >=
                        (int)sizeof(fullName)) {
                        return 0;
                    }
                }
                if (isDir) {
                    if (iso_extract_entry_in_dir(fp, raw2352, lba, size, depth + 1,
                                                 fullName, entryName, outFilePath)) {
                        return 1;
                    }
                } else if (asset_casecmp(fullName, entryName) == 0) {
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
                                     "", entryName, outFilePath)) {
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
                                "", expectedMd5, isoPath, outPath, outPathLen)) {
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
                                               "", md5List, md5Count, isoPath,
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

static int cue_contains_case(const char *line, const char *needle) {
    size_t needleLen;
    const char *p;
    if (!line || !needle) return 0;
    needleLen = strlen(needle);
    if (needleLen == 0U) return 1;
    for (p = line; *p; ++p) {
        size_t i;
        for (i = 0U; i < needleLen; ++i) {
            if (p[i] == '\0' ||
                tolower((unsigned char)p[i]) !=
                tolower((unsigned char)needle[i])) {
                break;
            }
        }
        if (i == needleLen) return 1;
    }
    return 0;
}

static int cue_track_is_data(const char *line) {
    const char *p;
    if (!line) return 0;
    p = cue_ltrim(line);
    if (!cue_starts_with_keyword(p, "TRACK")) return 0;
    return cue_contains_case(p, "MODE1/2048") ||
           cue_contains_case(p, "MODE1/2352") ||
           cue_contains_case(p, "MODE2/2048") ||
           cue_contains_case(p, "MODE2/2352");
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

static int tar_block_is_zero(const unsigned char *hdr) {
    int i;
    if (!hdr) return 1;
    for (i = 0; i < 512; ++i) {
        if (hdr[i] != 0U) return 0;
    }
    return 1;
}

static int tar_parse_octal(const unsigned char *field, size_t fieldSize,
                           uint32_t *outValue) {
    uint64_t value = 0U;
    size_t i;
    int seen = 0;
    if (!field || fieldSize == 0U || !outValue) return 0;
    for (i = 0U; i < fieldSize; ++i) {
        unsigned char c = field[i];
        if (c == '\0' || c == ' ') {
            if (seen) break;
            continue;
        }
        if (c < '0' || c > '7') return 0;
        seen = 1;
        value = (value << 3U) + (uint64_t)(c - '0');
        if (value > UINT32_MAX) return 0;
    }
    if (!seen) return 0;
    *outValue = (uint32_t)value;
    return 1;
}

static int tar_entry_name(const unsigned char *hdr, char *out, size_t outSize) {
    char name[101];
    char prefix[156];
    size_t nameLen;
    size_t prefixLen;
    if (!hdr || !out || outSize == 0U) return 0;
    memcpy(name, hdr, 100U);
    name[100] = '\0';
    memcpy(prefix, hdr + 345, 155U);
    prefix[155] = '\0';
    nameLen = strlen(name);
    prefixLen = strlen(prefix);
    if (nameLen == 0U) return 0;
    if (prefixLen > 0U) {
        return snprintf(out, outSize, "%s/%s", prefix, name) < (int)outSize;
    }
    return snprintf(out, outSize, "%s", name) < (int)outSize;
}

#ifdef FIRESTAFF_HAS_ZLIB
static int memory_range_md5(const unsigned char *data, uint32_t size,
                            char outHex[33]) {
    AssetMd5Ctx ctx;
    if (!data || !outHex) return 0;
    md5_init(&ctx);
    md5_update(&ctx, data, size);
    md5_final(&ctx, outHex);
    return 1;
}

static int copy_memory_range_to_path(const unsigned char *data, uint32_t size,
                                     const char *outFilePath) {
    FILE *out;
    if (!data || !outFilePath) return 0;
    out = fopen(outFilePath, "wb");
    if (!out) return 0;
    if (fwrite(data, 1U, size, out) != size) {
        fclose(out);
        return 0;
    }
    return fclose(out) == 0;
}
#endif

static int tar_scan_file_by_md5(const char *tarPath, const char *expectedMd5,
                                char *outPath, int outPathLen) {
    FILE *fp;
    long pos = 0;
    char bestName[ASSET_PATH_MAX];
    int hasMatch = 0;
    if (!tarPath || !expectedMd5 || !outPath || outPathLen <= 0) return 0;
    fp = fopen(tarPath, "rb");
    if (!fp) return 0;
    bestName[0] = '\0';
    for (;;) {
        unsigned char hdr[512];
        uint32_t size;
        uint32_t dataOffset;
        char name[ASSET_PATH_MAX];
        char hex[33];
        char typeflag;
        long nextPos;
        if (fseek(fp, pos, SEEK_SET) != 0 ||
            fread(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr)) {
            break;
        }
        if (tar_block_is_zero(hdr)) break;
        if (!tar_parse_octal(hdr + 124, 12U, &size)) break;
        typeflag = (char)hdr[156];
        dataOffset = (uint32_t)(pos + 512L);
        nextPos = pos + 512L + (long)(((size + 511U) / 512U) * 512U);
        if ((typeflag == '\0' || typeflag == '0') &&
            size <= ASSET_TAR_MAX_ENTRY_BYTES &&
            tar_entry_name(hdr, name, sizeof(name)) &&
            file_range_md5(fp, (long)dataOffset, size, hex) &&
            strcmp(hex, expectedMd5) == 0 &&
            is_better_zip_entry(name, hasMatch ? bestName : NULL)) {
            strcpy(bestName, name);
            hasMatch = 1;
        }
        pos = nextPos;
    }
    fclose(fp);
    return hasMatch ? copy_virtual_match_path(tarPath, bestName, outPath, outPathLen) : 0;
}

static int tar_scan_file_by_md5_list(const char *tarPath,
                                     const char *const *md5List,
                                     int md5Count,
                                     char outPaths[][ASSET_PATH_MAX],
                                     int matched[]) {
    FILE *fp;
    long pos = 0;
    char bestNames[64][ASSET_PATH_MAX];
    int hasBest[64];
    int foundCount = 0;
    int i;
    if (!tarPath || !md5List || md5Count <= 0 || md5Count > 64 ||
        !outPaths || !matched) {
        return 0;
    }
    memset(bestNames, 0, sizeof(bestNames));
    memset(hasBest, 0, sizeof(hasBest));
    fp = fopen(tarPath, "rb");
    if (!fp) return 0;
    for (;;) {
        unsigned char hdr[512];
        uint32_t size;
        uint32_t dataOffset;
        char name[ASSET_PATH_MAX];
        char hex[33];
        char typeflag;
        int matchIndex;
        long nextPos;
        if (fseek(fp, pos, SEEK_SET) != 0 ||
            fread(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr)) {
            break;
        }
        if (tar_block_is_zero(hdr)) break;
        if (!tar_parse_octal(hdr + 124, 12U, &size)) break;
        typeflag = (char)hdr[156];
        dataOffset = (uint32_t)(pos + 512L);
        nextPos = pos + 512L + (long)(((size + 511U) / 512U) * 512U);
        if ((typeflag == '\0' || typeflag == '0') &&
            size <= ASSET_TAR_MAX_ENTRY_BYTES &&
            tar_entry_name(hdr, name, sizeof(name)) &&
            file_range_md5(fp, (long)dataOffset, size, hex)) {
            matchIndex = md5_list_match_index(hex, md5List, NULL, md5Count);
            if (matchIndex >= 0 &&
                is_better_zip_entry(name, hasBest[matchIndex] ? bestNames[matchIndex] : NULL)) {
                strcpy(bestNames[matchIndex], name);
                hasBest[matchIndex] = 1;
            }
        }
        pos = nextPos;
    }
    fclose(fp);
    for (i = 0; i < md5Count; ++i) {
        if (!hasBest[i] || matched[i]) continue;
        if (copy_virtual_match_path(tarPath, bestNames[i], outPaths[i], ASSET_PATH_MAX)) {
            matched[i] = 1;
            ++foundCount;
        }
    }
    return foundCount;
}

static int tar_extract_file_entry(const char *tarPath, const char *entryName,
                                  const char *outFilePath) {
    FILE *fp;
    long pos = 0;
    if (!tarPath || !entryName || !outFilePath) return 0;
    fp = fopen(tarPath, "rb");
    if (!fp) return 0;
    for (;;) {
        unsigned char hdr[512];
        uint32_t size;
        uint32_t dataOffset;
        char name[ASSET_PATH_MAX];
        char typeflag;
        long nextPos;
        if (fseek(fp, pos, SEEK_SET) != 0 ||
            fread(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr)) {
            break;
        }
        if (tar_block_is_zero(hdr)) break;
        if (!tar_parse_octal(hdr + 124, 12U, &size)) break;
        typeflag = (char)hdr[156];
        dataOffset = (uint32_t)(pos + 512L);
        nextPos = pos + 512L + (long)(((size + 511U) / 512U) * 512U);
        if ((typeflag == '\0' || typeflag == '0') &&
            size <= ASSET_TAR_MAX_ENTRY_BYTES &&
            tar_entry_name(hdr, name, sizeof(name)) &&
            strcmp(name, entryName) == 0) {
            int ok = copy_file_range_to_path(fp, dataOffset, size, outFilePath);
            fclose(fp);
            return ok;
        }
        pos = nextPos;
    }
    fclose(fp);
    return 0;
}

#ifdef FIRESTAFF_HAS_ZLIB
static int inflate_gzip_file_to_memory(const char *path,
                                       unsigned char **outData,
                                       uint32_t *outSize) {
    unsigned char outBuf[8192];
    unsigned char *gzData = NULL;
    size_t gzSize;
    size_t deflateOffset;
    size_t deflateSize;
    unsigned char *data = NULL;
    size_t dataSize = 0U;
    size_t dataCap = 0U;
    FILE *fp;
    z_stream zs;
    int ret = Z_OK;
    if (!path || !outData || !outSize) return 0;
    *outData = NULL;
    *outSize = 0U;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    {
        long endPos = ftell(fp);
        if (endPos < 18L || endPos > (long)ASSET_GZIP_TAR_MAX_BYTES) {
            fclose(fp);
            return 0;
        }
        gzSize = (size_t)endPos;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    gzData = (unsigned char*)malloc(gzSize);
    if (!gzData) {
        fclose(fp);
        return 0;
    }
    if (fread(gzData, 1U, gzSize, fp) != gzSize) {
        fclose(fp);
        free(gzData);
        return 0;
    }
    fclose(fp);
    if (gzData[0] != 0x1fU || gzData[1] != 0x8bU || gzData[2] != 8U) {
        free(gzData);
        return 0;
    }
    deflateOffset = 10U;
    if (gzData[3] & 0x04U) {
        uint16_t extraLen;
        if (deflateOffset + 2U > gzSize) {
            free(gzData);
            return 0;
        }
        extraLen = read_u16le(gzData + deflateOffset);
        deflateOffset += 2U + (size_t)extraLen;
    }
    if (gzData[3] & 0x08U) {
        while (deflateOffset < gzSize && gzData[deflateOffset++] != 0U) {}
    }
    if (gzData[3] & 0x10U) {
        while (deflateOffset < gzSize && gzData[deflateOffset++] != 0U) {}
    }
    if (gzData[3] & 0x02U) {
        deflateOffset += 2U;
    }
    if (deflateOffset >= gzSize || deflateOffset + 8U > gzSize) {
        free(gzData);
        return 0;
    }
    deflateSize = gzSize - deflateOffset - 8U;
    memset(&zs, 0, sizeof(zs));
    if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) {
        free(gzData);
        return 0;
    }
    zs.next_in = gzData + deflateOffset;
    zs.avail_in = (uInt)deflateSize;
    do {
        size_t produced;
        zs.next_out = outBuf;
        zs.avail_out = (uInt)sizeof(outBuf);
        ret = inflate(&zs, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
            inflateEnd(&zs);
            free(gzData);
            free(data);
            return 0;
        }
        produced = sizeof(outBuf) - zs.avail_out;
        if (produced > 0U) {
            if (dataSize + produced > ASSET_GZIP_TAR_MAX_BYTES) {
                inflateEnd(&zs);
                free(gzData);
                free(data);
                return 0;
            }
            if (dataSize + produced > dataCap) {
                size_t newCap = dataCap ? dataCap * 2U : 65536U;
                unsigned char *grown;
                while (newCap < dataSize + produced) newCap *= 2U;
                if (newCap > ASSET_GZIP_TAR_MAX_BYTES) newCap = ASSET_GZIP_TAR_MAX_BYTES;
                grown = (unsigned char*)realloc(data, newCap);
                if (!grown) {
                    inflateEnd(&zs);
                    free(gzData);
                    free(data);
                    return 0;
                }
                data = grown;
                dataCap = newCap;
            }
            memcpy(data + dataSize, outBuf, produced);
            dataSize += produced;
        }
    } while (ret != Z_STREAM_END);
    inflateEnd(&zs);
    free(gzData);
    if (dataSize > UINT32_MAX) {
        free(data);
        return 0;
    }
    *outData = data;
    *outSize = (uint32_t)dataSize;
    return 1;
}
#endif

#ifdef FIRESTAFF_HAS_ZLIB
static int tar_scan_memory_by_md5(const unsigned char *tarData,
                                  uint32_t tarSize,
                                  const char *containerPath,
                                  const char *expectedMd5,
                                  char *outPath,
                                  int outPathLen) {
    uint32_t pos = 0U;
    char bestName[ASSET_PATH_MAX];
    int hasMatch = 0;
    if (!tarData || !containerPath || !expectedMd5 || !outPath || outPathLen <= 0) return 0;
    bestName[0] = '\0';
    while (pos + 512U <= tarSize) {
        const unsigned char *hdr = tarData + pos;
        uint32_t size;
        uint32_t dataOffset;
        uint32_t paddedSize;
        char name[ASSET_PATH_MAX];
        char hex[33];
        char typeflag;
        if (tar_block_is_zero(hdr)) break;
        if (!tar_parse_octal(hdr + 124, 12U, &size)) break;
        typeflag = (char)hdr[156];
        dataOffset = pos + 512U;
        paddedSize = ((size + 511U) / 512U) * 512U;
        if (dataOffset > tarSize || paddedSize > tarSize - dataOffset) break;
        if ((typeflag == '\0' || typeflag == '0') &&
            size <= ASSET_TAR_MAX_ENTRY_BYTES &&
            tar_entry_name(hdr, name, sizeof(name)) &&
            memory_range_md5(tarData + dataOffset, size, hex) &&
            strcmp(hex, expectedMd5) == 0 &&
            is_better_zip_entry(name, hasMatch ? bestName : NULL)) {
            strcpy(bestName, name);
            hasMatch = 1;
        }
        pos = dataOffset + paddedSize;
    }
    return hasMatch ? copy_virtual_match_path(containerPath, bestName, outPath, outPathLen) : 0;
}

static int tar_scan_memory_by_md5_list(const unsigned char *tarData,
                                       uint32_t tarSize,
                                       const char *containerPath,
                                       const char *const *md5List,
                                       int md5Count,
                                       char outPaths[][ASSET_PATH_MAX],
                                       int matched[]) {
    uint32_t pos = 0U;
    char bestNames[64][ASSET_PATH_MAX];
    int hasBest[64];
    int foundCount = 0;
    int i;
    if (!tarData || !containerPath || !md5List || md5Count <= 0 ||
        md5Count > 64 || !outPaths || !matched) {
        return 0;
    }
    memset(bestNames, 0, sizeof(bestNames));
    memset(hasBest, 0, sizeof(hasBest));
    while (pos + 512U <= tarSize) {
        const unsigned char *hdr = tarData + pos;
        uint32_t size;
        uint32_t dataOffset;
        uint32_t paddedSize;
        char name[ASSET_PATH_MAX];
        char hex[33];
        char typeflag;
        int matchIndex;
        if (tar_block_is_zero(hdr)) break;
        if (!tar_parse_octal(hdr + 124, 12U, &size)) break;
        typeflag = (char)hdr[156];
        dataOffset = pos + 512U;
        paddedSize = ((size + 511U) / 512U) * 512U;
        if (dataOffset > tarSize || paddedSize > tarSize - dataOffset) break;
        if ((typeflag == '\0' || typeflag == '0') &&
            size <= ASSET_TAR_MAX_ENTRY_BYTES &&
            tar_entry_name(hdr, name, sizeof(name)) &&
            memory_range_md5(tarData + dataOffset, size, hex)) {
            matchIndex = md5_list_match_index(hex, md5List, NULL, md5Count);
            if (matchIndex >= 0 &&
                is_better_zip_entry(name, hasBest[matchIndex] ? bestNames[matchIndex] : NULL)) {
                strcpy(bestNames[matchIndex], name);
                hasBest[matchIndex] = 1;
            }
        }
        pos = dataOffset + paddedSize;
    }
    for (i = 0; i < md5Count; ++i) {
        if (!hasBest[i] || matched[i]) continue;
        if (copy_virtual_match_path(containerPath, bestNames[i], outPaths[i], ASSET_PATH_MAX)) {
            matched[i] = 1;
            ++foundCount;
        }
    }
    return foundCount;
}

static int tar_extract_memory_entry(const unsigned char *tarData,
                                    uint32_t tarSize,
                                    const char *entryName,
                                    const char *outFilePath) {
    uint32_t pos = 0U;
    if (!tarData || !entryName || !outFilePath) return 0;
    while (pos + 512U <= tarSize) {
        const unsigned char *hdr = tarData + pos;
        uint32_t size;
        uint32_t dataOffset;
        uint32_t paddedSize;
        char name[ASSET_PATH_MAX];
        char typeflag;
        if (tar_block_is_zero(hdr)) break;
        if (!tar_parse_octal(hdr + 124, 12U, &size)) break;
        typeflag = (char)hdr[156];
        dataOffset = pos + 512U;
        paddedSize = ((size + 511U) / 512U) * 512U;
        if (dataOffset > tarSize || paddedSize > tarSize - dataOffset) break;
        if ((typeflag == '\0' || typeflag == '0') &&
            size <= ASSET_TAR_MAX_ENTRY_BYTES &&
            tar_entry_name(hdr, name, sizeof(name)) &&
            strcmp(name, entryName) == 0) {
            return copy_memory_range_to_path(tarData + dataOffset, size, outFilePath);
        }
        pos = dataOffset + paddedSize;
    }
    return 0;
}
#endif

static int scan_tgz_by_md5(const char *tgzPath, const char *expectedMd5,
                           char *outPath, int outPathLen) {
#ifdef FIRESTAFF_HAS_ZLIB
    unsigned char *tarData = NULL;
    uint32_t tarSize = 0U;
    int ok;
    if (!inflate_gzip_file_to_memory(tgzPath, &tarData, &tarSize)) return 0;
    ok = tar_scan_memory_by_md5(tarData, tarSize, tgzPath, expectedMd5,
                                outPath, outPathLen);
    free(tarData);
    return ok;
#else
    (void)tgzPath; (void)expectedMd5; (void)outPath; (void)outPathLen;
    return 0;
#endif
}

static int scan_tgz_by_md5_list(const char *tgzPath,
                                const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[]) {
#ifdef FIRESTAFF_HAS_ZLIB
    unsigned char *tarData = NULL;
    uint32_t tarSize = 0U;
    int foundCount;
    if (!inflate_gzip_file_to_memory(tgzPath, &tarData, &tarSize)) return 0;
    foundCount = tar_scan_memory_by_md5_list(tarData, tarSize, tgzPath,
                                             md5List, md5Count, outPaths, matched);
    free(tarData);
    return foundCount;
#else
    (void)tgzPath; (void)md5List; (void)md5Count; (void)outPaths; (void)matched;
    return 0;
#endif
}

static int tgz_extract_entry_to_path(const char *tgzPath, const char *entryName,
                                     const char *outFilePath) {
#ifdef FIRESTAFF_HAS_ZLIB
    unsigned char *tarData = NULL;
    uint32_t tarSize = 0U;
    int ok;
    if (!inflate_gzip_file_to_memory(tgzPath, &tarData, &tarSize)) return 0;
    ok = tar_extract_memory_entry(tarData, tarSize, entryName, outFilePath);
    free(tarData);
    return ok;
#else
    (void)tgzPath; (void)entryName; (void)outFilePath;
    return 0;
#endif
}

#ifdef FIRESTAFF_HAS_ZLIB
static int gzip_entry_name(const char *gzipPath, char *out, size_t outSize) {
    const char *base;
    size_t len;
    if (!gzipPath || !out || outSize == 0U) return 0;
    base = strrchr(gzipPath, '/');
    if (!base) base = strrchr(gzipPath, '\\');
    base = base ? base + 1 : gzipPath;
    len = strlen(base);
    if (has_case_suffix(base, ".gzip")) {
        len -= 5U;
    } else if (has_case_suffix(base, ".gz")) {
        len -= 3U;
    } else {
        return 0;
    }
    if (len == 0U || len >= outSize) return 0;
    memcpy(out, base, len);
    out[len] = '\0';
    return 1;
}
#endif

static int scan_gzip_by_md5(const char *gzipPath, const char *expectedMd5,
                            char *outPath, int outPathLen) {
#ifdef FIRESTAFF_HAS_ZLIB
    unsigned char *data = NULL;
    uint32_t size = 0U;
    char hex[33];
    char entryName[ASSET_PATH_MAX];
    int ok = 0;
    if (!inflate_gzip_file_to_memory(gzipPath, &data, &size)) return 0;
    if (size >= 16U && memory_range_md5(data, size, hex) &&
        strcmp(hex, expectedMd5) == 0 &&
        gzip_entry_name(gzipPath, entryName, sizeof(entryName))) {
        ok = copy_virtual_match_path(gzipPath, entryName, outPath, outPathLen);
    }
    free(data);
    return ok;
#else
    (void)gzipPath; (void)expectedMd5; (void)outPath; (void)outPathLen;
    return 0;
#endif
}

static int scan_gzip_by_md5_list(const char *gzipPath,
                                 const char *const *md5List,
                                 int md5Count,
                                 char outPaths[][ASSET_PATH_MAX],
                                 int matched[]) {
#ifdef FIRESTAFF_HAS_ZLIB
    unsigned char *data = NULL;
    uint32_t size = 0U;
    char hex[33];
    char entryName[ASSET_PATH_MAX];
    int matchIndex;
    int found = 0;
    if (!inflate_gzip_file_to_memory(gzipPath, &data, &size)) return 0;
    if (size >= 16U && memory_range_md5(data, size, hex) &&
        gzip_entry_name(gzipPath, entryName, sizeof(entryName))) {
        matchIndex = md5_list_match_index(hex, md5List, matched, md5Count);
        if (matchIndex >= 0 &&
            copy_virtual_match_path(gzipPath, entryName,
                                    outPaths[matchIndex], ASSET_PATH_MAX)) {
            matched[matchIndex] = 1;
            found = 1;
        }
    }
    free(data);
    return found;
#else
    (void)gzipPath; (void)md5List; (void)md5Count; (void)outPaths; (void)matched;
    return 0;
#endif
}

static int gzip_extract_entry_to_path(const char *gzipPath, const char *entryName,
                                      const char *outFilePath) {
#ifdef FIRESTAFF_HAS_ZLIB
    unsigned char *data = NULL;
    uint32_t size = 0U;
    char inferredName[ASSET_PATH_MAX];
    int ok = 0;
    if (!gzip_entry_name(gzipPath, inferredName, sizeof(inferredName)) ||
        strcmp(inferredName, entryName) != 0) {
        return 0;
    }
    if (!inflate_gzip_file_to_memory(gzipPath, &data, &size)) return 0;
    ok = copy_memory_range_to_path(data, size, outFilePath);
    free(data);
    return ok;
#else
    (void)gzipPath; (void)entryName; (void)outFilePath;
    return 0;
#endif
}

static int lha_parse_header(FILE *fp, long pos, char method[6],
                            uint32_t *outPackedSize,
                            uint32_t *outOriginalSize,
                            char *outName,
                            size_t outNameSize,
                            long *outDataOffset,
                            long *outNextPos) {
    unsigned char fixed[22];
    uint8_t headerSize;
    uint8_t level;
    uint8_t nameLen;
    long dataOffset;
    if (!fp || !method || !outPackedSize || !outOriginalSize ||
        !outName || outNameSize == 0U || !outDataOffset || !outNextPos) {
        return 0;
    }
    if (fseek(fp, pos, SEEK_SET) != 0 ||
        fread(&headerSize, 1U, 1U, fp) != 1U) {
        return 0;
    }
    if (headerSize == 0U) return 0;
    if (headerSize < 21U) return 0;
    if (fread(fixed, 1U, 21U, fp) != 21U) return 0;
    memcpy(method, fixed + 1, 5U);
    method[5] = '\0';
    *outPackedSize = read_u32le(fixed + 6);
    *outOriginalSize = read_u32le(fixed + 10);
    level = fixed[19];
    nameLen = fixed[20];
    if ((level != 0U && level != 1U) ||
        nameLen == 0U ||
        21U + (uint32_t)nameLen > (uint32_t)headerSize ||
        (size_t)nameLen >= outNameSize) {
        return 0;
    }
    if (fread(outName, 1U, nameLen, fp) != nameLen) return 0;
    outName[nameLen] = '\0';
    dataOffset = pos + 1L + (long)headerSize;
    if (*outPackedSize > ASSET_TAR_MAX_ENTRY_BYTES ||
        *outOriginalSize > ASSET_TAR_MAX_ENTRY_BYTES) {
        return 0;
    }
    *outDataOffset = dataOffset;
    *outNextPos = dataOffset + (long)*outPackedSize;
    return 1;
}

static int lha_entry_is_stored(const char *method,
                               uint32_t packedSize,
                               uint32_t originalSize) {
    return method && strcmp(method, "-lh0-") == 0 && packedSize == originalSize;
}

static int scan_lha_by_md5(const char *lhaPath, const char *expectedMd5,
                           char *outPath, int outPathLen) {
    FILE *fp;
    long pos = 0L;
    char bestName[ASSET_PATH_MAX];
    int hasMatch = 0;
    if (!lhaPath || !expectedMd5 || !outPath || outPathLen <= 0) return 0;
    fp = fopen(lhaPath, "rb");
    if (!fp) return 0;
    bestName[0] = '\0';
    for (;;) {
        char method[6];
        char name[ASSET_PATH_MAX];
        uint32_t packedSize;
        uint32_t originalSize;
        long dataOffset;
        long nextPos;
        char hex[33];
        if (!lha_parse_header(fp, pos, method, &packedSize, &originalSize,
                              name, sizeof(name), &dataOffset, &nextPos)) {
            break;
        }
        if (lha_entry_is_stored(method, packedSize, originalSize) &&
            originalSize >= 16U &&
            file_range_md5(fp, dataOffset, originalSize, hex) &&
            strcmp(hex, expectedMd5) == 0 &&
            is_better_zip_entry(name, hasMatch ? bestName : NULL)) {
            strcpy(bestName, name);
            hasMatch = 1;
        }
        pos = nextPos;
    }
    fclose(fp);
    return hasMatch ? copy_virtual_match_path(lhaPath, bestName, outPath, outPathLen) : 0;
}

static int scan_lha_by_md5_list(const char *lhaPath,
                                const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[]) {
    FILE *fp;
    long pos = 0L;
    char bestNames[64][ASSET_PATH_MAX];
    int hasBest[64];
    int foundCount = 0;
    int i;
    if (!lhaPath || !md5List || md5Count <= 0 || md5Count > 64 ||
        !outPaths || !matched) {
        return 0;
    }
    memset(bestNames, 0, sizeof(bestNames));
    memset(hasBest, 0, sizeof(hasBest));
    fp = fopen(lhaPath, "rb");
    if (!fp) return 0;
    for (;;) {
        char method[6];
        char name[ASSET_PATH_MAX];
        uint32_t packedSize;
        uint32_t originalSize;
        long dataOffset;
        long nextPos;
        char hex[33];
        int matchIndex;
        if (!lha_parse_header(fp, pos, method, &packedSize, &originalSize,
                              name, sizeof(name), &dataOffset, &nextPos)) {
            break;
        }
        if (lha_entry_is_stored(method, packedSize, originalSize) &&
            originalSize >= 16U &&
            file_range_md5(fp, dataOffset, originalSize, hex)) {
            matchIndex = md5_list_match_index(hex, md5List, NULL, md5Count);
            if (matchIndex >= 0 &&
                is_better_zip_entry(name, hasBest[matchIndex] ? bestNames[matchIndex] : NULL)) {
                strcpy(bestNames[matchIndex], name);
                hasBest[matchIndex] = 1;
            }
        }
        pos = nextPos;
    }
    fclose(fp);
    for (i = 0; i < md5Count; ++i) {
        if (!hasBest[i] || matched[i]) continue;
        if (copy_virtual_match_path(lhaPath, bestNames[i], outPaths[i], ASSET_PATH_MAX)) {
            matched[i] = 1;
            ++foundCount;
        }
    }
    return foundCount;
}

static int lha_extract_entry_to_path(const char *lhaPath, const char *entryName,
                                     const char *outFilePath) {
    FILE *fp;
    long pos = 0L;
    if (!lhaPath || !entryName || !outFilePath) return 0;
    fp = fopen(lhaPath, "rb");
    if (!fp) return 0;
    for (;;) {
        char method[6];
        char name[ASSET_PATH_MAX];
        uint32_t packedSize;
        uint32_t originalSize;
        long dataOffset;
        long nextPos;
        if (!lha_parse_header(fp, pos, method, &packedSize, &originalSize,
                              name, sizeof(name), &dataOffset, &nextPos)) {
            break;
        }
        if (strcmp(name, entryName) == 0 &&
            lha_entry_is_stored(method, packedSize, originalSize)) {
            int ok = copy_file_range_to_path(fp, (uint32_t)dataOffset,
                                             originalSize, outFilePath);
            fclose(fp);
            return ok;
        }
        pos = nextPos;
    }
    fclose(fp);
    return 0;
}

#ifndef _WIN32
static int shell_append_quoted(char *cmd, size_t cmdSize, const char *arg) {
    size_t len;
    if (!cmd || !arg || cmdSize == 0U) return 0;
    len = strlen(cmd);
    if (len + 2U >= cmdSize) return 0;
    cmd[len++] = '\'';
    cmd[len] = '\0';
    while (*arg) {
        if (*arg == '\'') {
            if (len + 5U >= cmdSize) return 0;
            memcpy(cmd + len, "'\\''", 4U);
            len += 4U;
        } else {
            if (len + 2U >= cmdSize) return 0;
            cmd[len++] = *arg;
        }
        cmd[len] = '\0';
        ++arg;
    }
    if (len + 2U >= cmdSize) return 0;
    cmd[len++] = '\'';
    cmd[len] = '\0';
    return 1;
}

static int shell_tool_exists(const char *tool) {
#if defined(__ANDROID__) || defined(FIRESTAFF_IOS)
    (void)tool;
    return 0;
#else
    char cmd[128];
    if (!tool || !*tool) return 0;
    if (snprintf(cmd, sizeof(cmd), "command -v %s >/dev/null 2>&1", tool) >=
        (int)sizeof(cmd)) {
        return 0;
    }
    return system(cmd) == 0;
#endif
}

/* Defined with the scanner diagnostics below. CHD media is a container too,
 * but chdman is deliberately separate from the generic archive tools. */
static void record_missing_tool(const char *mediaPath, const char *tools);

static int chd_tool_available(void) {
    /* Keep the test-only scanner override consistent for every host helper,
     * including CHD, rather than making CI depend on its installed tools. */
    if (getenv("FIRESTAFF_TEST_DISABLE_EXTERNAL_ARCHIVE_TOOLS") != NULL) {
        return 0;
    }
    return shell_tool_exists("chdman");
}

static int make_chd_temp_dir(char *outDir, size_t outDirSize) {
    char tmpl[ASSET_PATH_MAX];
    char *made;
    if (!outDir || outDirSize == 0U) return 0;
    if (snprintf(tmpl, sizeof(tmpl), "/tmp/firestaff-chd-scan-XXXXXX") >=
        (int)sizeof(tmpl)) {
        return 0;
    }
    made = mkdtemp(tmpl);
    if (!made) return 0;
    if (strlen(made) + 1U > outDirSize) {
        rmdir(made);
        return 0;
    }
    strcpy(outDir, made);
    return 1;
}

static int chd_extractcd_to_cue(const char *chdPath,
                                char *outCuePath,
                                size_t outCuePathSize,
                                char *outTempDir,
                                size_t outTempDirSize) {
#if defined(__ANDROID__) || defined(FIRESTAFF_IOS)
    (void)chdPath; (void)outCuePath; (void)outCuePathSize;
    (void)outTempDir; (void)outTempDirSize;
    return 0;
#else
    char cmd[ASSET_PATH_MAX * 3];
    if (!chdPath || !outCuePath || outCuePathSize == 0U ||
        !outTempDir || outTempDirSize == 0U) {
        return 0;
    }
    if (!chd_tool_available()) {
        record_missing_tool(chdPath, "chdman");
        return 0;
    }
    if (!make_chd_temp_dir(outTempDir, outTempDirSize)) return 0;
    if (snprintf(outCuePath, outCuePathSize, "%s/disc.cue", outTempDir) >=
        (int)outCuePathSize) {
        rmdir(outTempDir);
        outTempDir[0] = '\0';
        return 0;
    }
    if (snprintf(cmd, sizeof(cmd), "chdman extractcd -f -i ") >= (int)sizeof(cmd) ||
        !shell_append_quoted(cmd, sizeof(cmd), chdPath) ||
        strlen(cmd) + 4U >= sizeof(cmd)) {
        rmdir(outTempDir);
        outTempDir[0] = '\0';
        return 0;
    }
    strcat(cmd, " -o ");
    if (!shell_append_quoted(cmd, sizeof(cmd), outCuePath) ||
        strlen(cmd) + 23U >= sizeof(cmd)) {
        rmdir(outTempDir);
        outTempDir[0] = '\0';
        return 0;
    }
    strcat(cmd, " >/dev/null 2>&1");
    if (system(cmd) != 0) {
        remove(outCuePath);
        rmdir(outTempDir);
        outTempDir[0] = '\0';
        return 0;
    }
    return 1;
#endif
}

static void cleanup_chd_temp(const char *tempDir, const char *cuePath) {
    char binPath[ASSET_PATH_MAX];
    char tocPath[ASSET_PATH_MAX];
    if (cuePath && *cuePath) remove(cuePath);
    if (tempDir && *tempDir) {
        if (snprintf(binPath, sizeof(binPath), "%s/disc.bin", tempDir) < (int)sizeof(binPath)) {
            remove(binPath);
        }
        if (snprintf(tocPath, sizeof(tocPath), "%s/disc.toc", tempDir) < (int)sizeof(tocPath)) {
            remove(tocPath);
        }
        rmdir(tempDir);
    }
}

static int chd_virtual_path_from_temp_match(const char *chdPath,
                                            const char *tempMatchPath,
                                            char *outPath,
                                            int outPathLen) {
    const char *sep;
    if (!chdPath || !tempMatchPath || !outPath || outPathLen <= 0) return 0;
    sep = strstr(tempMatchPath, "::");
    if (sep && sep[2] != '\0') {
        return copy_virtual_match_path(chdPath, sep + 2, outPath, outPathLen);
    }
    return copy_virtual_match_path(chdPath, "TRACK02.BIN", outPath, outPathLen);
}

static int scan_chd_by_md5(const char *chdPath, const char *expectedMd5,
                           char *outPath, int outPathLen) {
    char tempDir[ASSET_PATH_MAX];
    char cuePath[ASSET_PATH_MAX];
    char tempMatch[ASSET_PATH_MAX];
    int ok = 0;
    tempDir[0] = '\0';
    cuePath[0] = '\0';
    if (!chd_extractcd_to_cue(chdPath, cuePath, sizeof(cuePath),
                              tempDir, sizeof(tempDir))) {
        return 0;
    }
    tempMatch[0] = '\0';
    if (scan_cue_by_md5(cuePath, expectedMd5, tempMatch, (int)sizeof(tempMatch))) {
        ok = chd_virtual_path_from_temp_match(chdPath, tempMatch, outPath, outPathLen);
    }
    cleanup_chd_temp(tempDir, cuePath);
    return ok;
}

static int scan_chd_by_md5_list(const char *chdPath,
                                const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[]) {
    char tempDir[ASSET_PATH_MAX];
    char cuePath[ASSET_PATH_MAX];
    char tempPaths[64][ASSET_PATH_MAX];
    int tempMatched[64];
    int foundCount = 0;
    int i;
    tempDir[0] = '\0';
    cuePath[0] = '\0';
    if (!chdPath || !md5List || md5Count <= 0 || md5Count > 64 ||
        !outPaths || !matched) {
        return 0;
    }
    if (!chd_extractcd_to_cue(chdPath, cuePath, sizeof(cuePath),
                              tempDir, sizeof(tempDir))) {
        return 0;
    }
    memset(tempPaths, 0, sizeof(tempPaths));
    memcpy(tempMatched, matched, (size_t)md5Count * sizeof(tempMatched[0]));
    (void)scan_cue_by_md5_list(cuePath, md5List, md5Count, tempPaths, tempMatched);
    for (i = 0; i < md5Count; ++i) {
        if (matched[i] || !tempMatched[i]) continue;
        if (chd_virtual_path_from_temp_match(chdPath, tempPaths[i],
                                             outPaths[i], ASSET_PATH_MAX)) {
            matched[i] = 1;
            ++foundCount;
        }
    }
    cleanup_chd_temp(tempDir, cuePath);
    return foundCount;
}

static int cue_first_data_payload_path(const char *cuePath,
                                       char *outPath,
                                       size_t outPathSize) {
    FILE *fp;
    char line[1024];
    char currentFile[ASSET_PATH_MAX];
    currentFile[0] = '\0';
    if (!cuePath || !outPath || outPathSize == 0U) return 0;
    fp = fopen(cuePath, "rb");
    if (!fp) return 0;
    while (fgets(line, sizeof(line), fp)) {
        char fileName[ASSET_PATH_MAX];
        if (cue_extract_file_name(line, fileName, sizeof(fileName))) {
            strcpy(currentFile, fileName);
            continue;
        }
        if (cue_track_is_data(line) && currentFile[0] != '\0' &&
            cue_resolve_payload_path(cuePath, currentFile, outPath, outPathSize)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static int copy_whole_file_to_path(const char *inPath, const char *outFilePath) {
    unsigned char buf[8192];
    FILE *in;
    FILE *out;
    size_t n;
    int ok = 1;
    if (!inPath || !outFilePath) return 0;
    in = fopen(inPath, "rb");
    if (!in) return 0;
    out = fopen(outFilePath, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }
    while ((n = fread(buf, 1U, sizeof(buf), in)) > 0U) {
        if (fwrite(buf, 1U, n, out) != n) ok = 0;
    }
    if (ferror(in)) ok = 0;
    if (fclose(in) != 0) ok = 0;
    if (fclose(out) != 0) ok = 0;
    return ok;
}

static int chd_extract_entry_to_path(const char *chdPath,
                                     const char *entryName,
                                     const char *outFilePath) {
    char tempDir[ASSET_PATH_MAX];
    char cuePath[ASSET_PATH_MAX];
    char payloadPath[ASSET_PATH_MAX];
    int ok = 0;
    tempDir[0] = '\0';
    cuePath[0] = '\0';
    if (!chd_extractcd_to_cue(chdPath, cuePath, sizeof(cuePath),
                              tempDir, sizeof(tempDir))) {
        return 0;
    }
    if (entryName && strcmp(entryName, "TRACK02.BIN") == 0 &&
        cue_first_data_payload_path(cuePath, payloadPath, sizeof(payloadPath))) {
        ok = copy_whole_file_to_path(payloadPath, outFilePath);
    } else {
        if (cue_first_data_payload_path(cuePath, payloadPath, sizeof(payloadPath))) {
            ok = iso_extract_entry_to_path(payloadPath, entryName, outFilePath);
        }
    }
    cleanup_chd_temp(tempDir, cuePath);
    return ok;
}

static const char *external_archive_tool_for_path(const char *archivePath) {
    static const char *const defaultTools[] = {"7zz", "7z", "bsdtar", NULL};
    static const char *const tarTools[] = {"bsdtar", "7zz", "7z", NULL};
    static const char *const rarTools[] = {"unrar", "7zz", "7z", "bsdtar", NULL};
    const char *const *tools =
        is_external_tar_archive_path(archivePath) ? tarTools :
        has_case_suffix(archivePath, ".rar") ? rarTools : defaultTools;
    int i;
    for (i = 0; tools[i] != NULL; ++i) {
        if (shell_tool_exists(tools[i])) {
            return tools[i];
        }
    }
    return NULL;
}

static int external_entry_command(char *cmd,
                                  size_t cmdSize,
                                  const char *tool,
                                  const char *archivePath,
                                  const char *entryName) {
    if (!cmd || cmdSize == 0U || !tool || !archivePath || !entryName) {
        return 0;
    }
    if (strcmp(tool, "bsdtar") == 0) {
        if (snprintf(cmd, cmdSize, "%s -xOf ", tool) >= (int)cmdSize) {
            return 0;
        }
        if (!shell_append_quoted(cmd, cmdSize, archivePath)) return 0;
        if (strlen(cmd) + 2U >= cmdSize) return 0;
        strcat(cmd, " ");
        if (!shell_append_quoted(cmd, cmdSize, entryName)) return 0;
        if (strlen(cmd) + 13U >= cmdSize) return 0;
        strcat(cmd, " 2>/dev/null");
        return 1;
    }
    if (strcmp(tool, "unrar") == 0) {
        if (snprintf(cmd, cmdSize, "%s p -inul ", tool) >= (int)cmdSize) {
            return 0;
        }
        if (!shell_append_quoted(cmd, cmdSize, archivePath)) return 0;
        if (strlen(cmd) + 2U >= cmdSize) return 0;
        strcat(cmd, " ");
        if (!shell_append_quoted(cmd, cmdSize, entryName)) return 0;
        if (strlen(cmd) + 13U >= cmdSize) return 0;
        strcat(cmd, " 2>/dev/null");
        return 1;
    }
    if (snprintf(cmd, cmdSize, "%s x -so -- ", tool) >= (int)cmdSize) {
        return 0;
    }
    if (!shell_append_quoted(cmd, cmdSize, archivePath)) return 0;
    if (strlen(cmd) + 2U >= cmdSize) return 0;
    strcat(cmd, " ");
    if (!shell_append_quoted(cmd, cmdSize, entryName)) return 0;
    if (strlen(cmd) + 13U >= cmdSize) return 0;
    strcat(cmd, " 2>/dev/null");
    return 1;
}

static int external_list_command(char *cmd,
                                 size_t cmdSize,
                                 const char *tool,
                                 const char *archivePath) {
    if (!cmd || cmdSize == 0U || !tool || !archivePath) {
        return 0;
    }
    if (strcmp(tool, "bsdtar") == 0) {
        if (snprintf(cmd, cmdSize, "%s -tf ", tool) >= (int)cmdSize) {
            return 0;
        }
        if (!shell_append_quoted(cmd, cmdSize, archivePath)) return 0;
        if (strlen(cmd) + 13U >= cmdSize) return 0;
        strcat(cmd, " 2>/dev/null");
        return 1;
    }
    if (strcmp(tool, "unrar") == 0) {
        if (snprintf(cmd, cmdSize, "%s lb ", tool) >= (int)cmdSize) {
            return 0;
        }
        if (!shell_append_quoted(cmd, cmdSize, archivePath)) return 0;
        if (strlen(cmd) + 13U >= cmdSize) return 0;
        strcat(cmd, " 2>/dev/null");
        return 1;
    }
    if (snprintf(cmd, cmdSize, "%s l -slt -- ", tool) >= (int)cmdSize) {
        return 0;
    }
    if (!shell_append_quoted(cmd, cmdSize, archivePath)) return 0;
    if (strlen(cmd) + 13U >= cmdSize) return 0;
    strcat(cmd, " 2>/dev/null");
    return 1;
}

static int external_entry_md5(const char *archivePath, const char *entryName,
                              char outHex[33]) {
    char cmd[ASSET_PATH_MAX * 4];
    char cacheKey[sizeof(((ScanCacheEntry_I *)0)->path)];
    const char *tool;
    unsigned char buf[8192];
    AssetMd5Ctx ctx;
    FILE *pipe;
    size_t n;
    int ok = 1;
    int cacheable = 0;
    int keyLength;
    struct stat st;
    if (!archivePath || !entryName || !outHex) return 0;

    /* 7zz must start a new process for every external member read.  A full
     * data-root scan asks several game profiles about the same archives, so
     * cache an entry digest behind the containing archive's identity.  Keep
     * the key bounded: ScanCache_I truncates paths for ordinary files, but a
     * truncated virtual path could alias two distinct archive members. */
    keyLength = snprintf(cacheKey, sizeof(cacheKey), "%s::%s",
                         archivePath, entryName);
    if (s_scan_cache && stat(archivePath, &st) == 0 && keyLength >= 0 &&
        (size_t)keyLength < sizeof(cacheKey)) {
        cacheable = 1;
        if (scache_lookup(s_scan_cache, cacheKey, (int64_t)st.st_mtime,
                          (int64_t)st.st_size, outHex)) {
            return 1;
        }
    }
    tool = external_archive_tool_for_path(archivePath);
    if (!tool) {
        return 0;
    }
    if (!external_entry_command(cmd, sizeof(cmd), tool, archivePath, entryName)) return 0;
    pipe = popen(cmd, "r");
    if (!pipe) return 0;
    md5_init(&ctx);
    while ((n = fread(buf, 1U, sizeof(buf), pipe)) > 0U) {
        md5_update(&ctx, buf, (unsigned int)n);
    }
    if (ferror(pipe)) ok = 0;
    if (pclose(pipe) != 0) ok = 0;
    if (!ok) return 0;
    md5_final(&ctx, outHex);
    if (cacheable) {
        scache_put(s_scan_cache, cacheKey, (int64_t)st.st_mtime,
                   (int64_t)st.st_size, outHex);
    }
    return 1;
}

static int external_extract_entry_to_path(const char *archivePath,
                                          const char *entryName,
                                          const char *outFilePath) {
    char cmd[ASSET_PATH_MAX * 4];
    const char *tool;
    unsigned char buf[8192];
    FILE *pipe;
    FILE *out;
    size_t n;
    int ok = 1;
    if (!archivePath || !entryName || !outFilePath) return 0;
    tool = external_archive_tool_for_path(archivePath);
    if (!tool) {
        return 0;
    }
    if (!external_entry_command(cmd, sizeof(cmd), tool, archivePath, entryName)) return 0;
    pipe = popen(cmd, "r");
    if (!pipe) return 0;
    out = fopen(outFilePath, "wb");
    if (!out) {
        pclose(pipe);
        return 0;
    }
    while ((n = fread(buf, 1U, sizeof(buf), pipe)) > 0U) {
        if (fwrite(buf, 1U, n, out) != n) ok = 0;
    }
    if (ferror(pipe)) ok = 0;
    if (pclose(pipe) != 0) ok = 0;
    if (fclose(out) != 0) ok = 0;
    return ok;
}

static uint8_t *external_read_entry_bytes(const char *archivePath,
                                          const char *entryName,
                                          size_t *out_size) {
    char cmd[ASSET_PATH_MAX * 4];
    const char *tool;
    FILE *pipe;
    uint8_t *bytes = NULL;
    size_t used = 0U, capacity = 0U;
    int ok = 1;
    if (!archivePath || !entryName || !out_size) return NULL;
    tool = external_archive_tool_for_path(archivePath);
    if (!tool || !external_entry_command(cmd, sizeof(cmd), tool, archivePath, entryName)) {
        return NULL;
    }
    pipe = popen(cmd, "r");
    if (!pipe) return NULL;
    for (;;) {
        uint8_t chunk[8192];
        size_t got = fread(chunk, 1U, sizeof(chunk), pipe);
        if (got > 0U) {
            if (used > (size_t)ASSET_SCAN_MAX_FILE_BYTES - got) {
                ok = 0;
                break;
            }
            if (used + got > capacity) {
                size_t next = capacity ? capacity * 2U : 65536U;
                uint8_t *grown;
                while (next < used + got) next *= 2U;
                grown = (uint8_t *)realloc(bytes, next);
                if (!grown) { ok = 0; break; }
                bytes = grown;
                capacity = next;
            }
            memcpy(bytes + used, chunk, got);
            used += got;
        }
        if (got < sizeof(chunk)) {
            if (ferror(pipe)) ok = 0;
            break;
        }
    }
    if (pclose(pipe) != 0) ok = 0;
    if (!ok || used == 0U) {
        free(bytes);
        return NULL;
    }
    *out_size = used;
    return bytes;
}

static int copy_nested_virtual_match_path(const char *archive, const char *adf,
                                          const char *entry, char *out_path,
                                          int out_path_len) {
    char virtual_path[ASSET_PATH_MAX];
    if (!archive || !adf || !entry || !out_path || out_path_len <= 0 ||
        snprintf(virtual_path, sizeof(virtual_path), "%s::%s::%s",
                 archive, adf, entry) >= (int)sizeof(virtual_path)) {
        return 0;
    }
    return copy_match_path(virtual_path, out_path, out_path_len);
}

typedef struct {
    const char *archive;
    const char *disk;
    const char *const *md5_list;
    int md5_count;
    char (*out_paths)[ASSET_PATH_MAX];
    int *matched;
    int found_count;
} NestedDiskListMatch;

typedef int (*AtariStFileVisitor)(const char *name, const uint8_t *bytes,
                                  size_t byte_count, void *user_data);
static uint8_t *atari_msa_decode_image(const uint8_t *source, size_t source_size,
                                       size_t *out_size);
static int atari_st_visit_files(const uint8_t *image, size_t image_size,
                                AtariStFileVisitor visitor, void *user_data);

static int nested_disk_find_list_visitor(const char *name, const uint8_t *bytes,
                                         size_t byte_count, void *user_data) {
    NestedDiskListMatch *matches = (NestedDiskListMatch *)user_data;
    AssetMd5Ctx ctx;
    char hex[33];
    int index;
    md5_init(&ctx);
    md5_update(&ctx, bytes, (unsigned int)byte_count);
    md5_final(&ctx, hex);
    index = md5_list_match_index(hex, matches->md5_list, matches->matched,
                                 matches->md5_count);
    if (index >= 0 && !matches->matched[index] &&
        copy_nested_virtual_match_path(matches->archive, matches->disk, name,
                                       matches->out_paths[index], ASSET_PATH_MAX)) {
        matches->matched[index] = 1;
        ++matches->found_count;
    }
    return 0;
}

static int scan_zip_nested_disk_by_md5_list(const char *zip_path,
                                            const char *const *md5_list,
                                            int md5_count,
                                            char out_paths[][ASSET_PATH_MAX],
                                            int matched[]) {
    FILE *fp;
    long file_size, search_start, eocd_offset = -1L;
    unsigned char *tail;
    size_t tail_size;
    uint32_t cd_offset = 0U, cd_size = 0U, pos;
    uint16_t entry_count = 0U, i;
    int found_count = 0;
    if (!zip_path || !md5_list || md5_count <= 0 || !out_paths || !matched) return 0;
    fp = fopen(zip_path, "rb");
    if (!fp || fseek(fp, 0L, SEEK_END) != 0 || (file_size = ftell(fp)) < 22L) {
        if (fp) fclose(fp);
        return 0;
    }
    tail_size = (size_t)(file_size < 65557L ? file_size : 65557L);
    search_start = file_size - (long)tail_size;
    tail = (unsigned char *)malloc(tail_size);
    if (!tail || fseek(fp, search_start, SEEK_SET) != 0 ||
        fread(tail, 1U, tail_size, fp) != tail_size) {
        free(tail);
        fclose(fp);
        return 0;
    }
    for (long j = (long)tail_size - 22L; j >= 0L; --j) {
        if (read_u32le(tail + j) == 0x06054b50U) {
            entry_count = read_u16le(tail + j + 10);
            cd_size = read_u32le(tail + j + 12);
            cd_offset = read_u32le(tail + j + 16);
            eocd_offset = search_start + j;
            break;
        }
    }
    free(tail);
    if (eocd_offset < 0L || cd_offset + cd_size > (uint32_t)file_size) {
        fclose(fp);
        return 0;
    }
    pos = cd_offset;
    for (i = 0U; i < entry_count && pos + 46U <= cd_offset + cd_size; ++i) {
        unsigned char hdr[46];
        uint16_t name_len, extra_len, comment_len;
        char name[256];
        if (fseek(fp, (long)pos, SEEK_SET) != 0 ||
            fread(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr) ||
            read_u32le(hdr) != 0x02014b50U) break;
        name_len = read_u16le(hdr + 28);
        extra_len = read_u16le(hdr + 30);
        comment_len = read_u16le(hdr + 32);
        if (name_len == 0U || name_len >= sizeof(name) ||
            fread(name, 1U, name_len, fp) != name_len) break;
        name[name_len] = '\0';
        pos += 46U + name_len + extra_len + comment_len;
        if (is_adf_path(name) || is_atari_st_path(name) || is_atari_msa_path(name)) {
            size_t image_size = 0U;
            uint8_t *image = zip_load_entry_bytes(zip_path, name, &image_size);
            NestedDiskListMatch matches;
            int result = -1;
            if (!image) continue;
            memset(&matches, 0, sizeof(matches));
            matches.archive = zip_path;
            matches.disk = name;
            matches.md5_list = md5_list;
            matches.md5_count = md5_count;
            matches.out_paths = out_paths;
            matches.matched = matched;
            if (is_adf_path(name)) {
                result = firestaff_amiga_adf_visit_ofs_files(
                    image, image_size, nested_disk_find_list_visitor, &matches);
            } else if (is_atari_st_path(name)) {
                result = atari_st_visit_files(image, image_size,
                                              nested_disk_find_list_visitor, &matches);
            } else {
                uint8_t *decoded;
                size_t decoded_size;
                decoded = atari_msa_decode_image(image, image_size, &decoded_size);
                if (decoded) {
                    result = atari_st_visit_files(decoded, decoded_size,
                                                  nested_disk_find_list_visitor, &matches);
                    free(decoded);
                }
            }
            free(image);
            if (result >= 0) found_count += matches.found_count;
            if (found_count >= md5_count) break;
        }
    }
    fclose(fp);
    return found_count;
}

static int external_archive_commit_entry(const char *archivePath,
                                         const char *expectedMd5,
                                         const char *entryName,
                                         uint32_t entrySize,
                                         char *bestName,
                                         int *hasMatch) {
    char hex[33];
    if (!archivePath || !expectedMd5 || !entryName || !bestName || !hasMatch) return 0;
    if (entryName[0] == '\0' || strcmp(entryName, archivePath) == 0 ||
        entryName[strlen(entryName) - 1U] == '/' ||
        (entrySize != UINT32_MAX &&
         (entrySize < 16U || entrySize > ASSET_TAR_MAX_ENTRY_BYTES))) {
        return 0;
    }
    if (external_entry_md5(archivePath, entryName, hex) &&
        strcmp(hex, expectedMd5) == 0 &&
        is_better_zip_entry(entryName, *hasMatch ? bestName : NULL)) {
        strcpy(bestName, entryName);
        *hasMatch = 1;
    }
    return 1;
}

static int scan_external_adf_by_md5(const char *archive_path,
                                    const char *adf_entry,
                                    const char *expected_md5,
                                    char *out_path, int out_path_len);
static int scan_external_adf_by_md5_list(const char *archive_path,
                                         const char *adf_entry,
                                         const char *const *md5_list,
                                         int md5_count,
                                         char out_paths[][ASSET_PATH_MAX],
                                         int matched[]);

static int scan_external_atari_st_by_md5(const char *archive_path,
                                         const char *st_entry,
                                         const char *expected_md5,
                                         char *out_path, int out_path_len);
static int scan_external_atari_st_by_md5_list(
    const char *archive_path, const char *st_entry, const char *const *md5_list,
    int md5_count, char out_paths[][ASSET_PATH_MAX], int matched[]);
static int scan_external_atari_msa_by_md5(const char *archive_path,
                                          const char *msa_entry,
                                          const char *expected_md5,
                                          char *out_path, int out_path_len);
static int scan_external_atari_msa_by_md5_list(
    const char *archive_path, const char *msa_entry, const char *const *md5_list,
    int md5_count, char out_paths[][ASSET_PATH_MAX], int matched[]);

static int scan_external_archive_by_md5(const char *archivePath,
                                        const char *expectedMd5,
                                        char *outPath,
                                        int outPathLen) {
    char cmd[ASSET_PATH_MAX * 3];
    char line[1024];
    char entryName[ASSET_PATH_MAX];
    uint32_t entrySize = 0U;
    int hasEntry = 0;
    int isFolder = 0;
    char bestName[ASSET_PATH_MAX];
    int hasMatch = 0;
    FILE *pipe;
    const char *tool = external_archive_tool_for_path(archivePath);
    if (!tool || !archivePath || !expectedMd5 || !outPath || outPathLen <= 0) {
        return 0;
    }
    if (!external_list_command(cmd, sizeof(cmd), tool, archivePath)) return 0;
    pipe = popen(cmd, "r");
    if (!pipe) return 0;
    entryName[0] = '\0';
    bestName[0] = '\0';
    while (fgets(line, sizeof(line), pipe)) {
        char *value;
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(tool, "bsdtar") == 0 || strcmp(tool, "unrar") == 0) {
            if (line[0] != '\0') {
                if (is_adf_path(line) && scan_external_adf_by_md5(
                        archivePath, line, expectedMd5, outPath, outPathLen)) {
                    (void)pclose(pipe);
                    return 1;
                }
                if (is_atari_st_path(line) && scan_external_atari_st_by_md5(
                        archivePath, line, expectedMd5, outPath, outPathLen)) {
                    (void)pclose(pipe);
                    return 1;
                }
                if (is_atari_msa_path(line) && scan_external_atari_msa_by_md5(
                        archivePath, line, expectedMd5, outPath, outPathLen)) {
                    (void)pclose(pipe);
                    return 1;
                }
                if (!is_kryoflux_raw_track_path(line)) {
                    (void)external_archive_commit_entry(archivePath, expectedMd5,
                                                        line, UINT32_MAX,
                                                        bestName, &hasMatch);
                }
            }
            continue;
        }
        if (strncmp(line, "Path = ", 7) == 0) {
            if (hasEntry && !isFolder) {
                if (is_adf_path(entryName) && scan_external_adf_by_md5(
                        archivePath, entryName, expectedMd5, outPath, outPathLen)) {
                    (void)pclose(pipe);
                    return 1;
                }
                if (is_atari_st_path(entryName) && scan_external_atari_st_by_md5(
                        archivePath, entryName, expectedMd5, outPath, outPathLen)) {
                    (void)pclose(pipe);
                    return 1;
                }
                if (is_atari_msa_path(entryName) && scan_external_atari_msa_by_md5(
                        archivePath, entryName, expectedMd5, outPath, outPathLen)) {
                    (void)pclose(pipe);
                    return 1;
                }
                if (!is_kryoflux_raw_track_path(entryName)) {
                    (void)external_archive_commit_entry(archivePath, expectedMd5,
                                                        entryName, entrySize,
                                                        bestName, &hasMatch);
                }
            }
            value = line + 7;
            strncpy(entryName, value, sizeof(entryName) - 1U);
            entryName[sizeof(entryName) - 1U] = '\0';
            entrySize = 0U;
            isFolder = 0;
            hasEntry = 1;
        } else if (strncmp(line, "Size = ", 7) == 0) {
            unsigned long parsed = strtoul(line + 7, NULL, 10);
            entrySize = parsed > UINT32_MAX ? UINT32_MAX : (uint32_t)parsed;
        } else if (strncmp(line, "Folder = +", 10) == 0) {
            isFolder = 1;
        }
    }
    if (hasEntry && !isFolder) {
        if (is_adf_path(entryName) && scan_external_adf_by_md5(
                archivePath, entryName, expectedMd5, outPath, outPathLen)) {
            (void)pclose(pipe);
            return 1;
        }
        if (is_atari_st_path(entryName) && scan_external_atari_st_by_md5(
                archivePath, entryName, expectedMd5, outPath, outPathLen)) {
            (void)pclose(pipe);
            return 1;
        }
        if (is_atari_msa_path(entryName) && scan_external_atari_msa_by_md5(
                archivePath, entryName, expectedMd5, outPath, outPathLen)) {
            (void)pclose(pipe);
            return 1;
        }
        if (!is_kryoflux_raw_track_path(entryName)) {
            (void)external_archive_commit_entry(archivePath, expectedMd5,
                                                entryName, entrySize,
                                                bestName, &hasMatch);
        }
    }
    (void)pclose(pipe);
    return hasMatch ? copy_virtual_match_path(archivePath, bestName,
                                              outPath, outPathLen) : 0;
}

static int scan_external_archive_by_md5_list(const char *archivePath,
                                             const char *const *md5List,
                                             int md5Count,
                                             char outPaths[][ASSET_PATH_MAX],
                                             int matched[]) {
    char cmd[ASSET_PATH_MAX * 3];
    char line[1024];
    char entryName[ASSET_PATH_MAX];
    char bestNames[64][ASSET_PATH_MAX];
    int hasBest[64];
    uint32_t entrySize = 0U;
    int hasEntry = 0;
    int isFolder = 0;
    int foundCount = 0;
    int diskImageOnly = md5Count > 1;
    int i;
    FILE *pipe;
    const char *tool = external_archive_tool_for_path(archivePath);
    if (!tool || !archivePath || !md5List ||
        md5Count <= 0 || md5Count > 64 || !outPaths || !matched) {
        return 0;
    }
    if (!external_list_command(cmd, sizeof(cmd), tool, archivePath)) return 0;
    /* A repeated digest list is an occurrence query.  M12 uses this only
     * for Amiga package TITL.DAT, where the relevant occurrence is inside an
     * ADF/ST/MSA image.  Do not stream every unrelated CD track from a large
     * external archive merely to collect those disk-image candidates. */
    for (i = 1; i < md5Count && diskImageOnly; ++i) {
        if (strcmp(md5List[0], md5List[i]) != 0) diskImageOnly = 0;
    }
    memset(bestNames, 0, sizeof(bestNames));
    memset(hasBest, 0, sizeof(hasBest));
    pipe = popen(cmd, "r");
    if (!pipe) return 0;
    entryName[0] = '\0';
    while (fgets(line, sizeof(line), pipe)) {
        int commit = 0;
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(tool, "bsdtar") == 0 || strcmp(tool, "unrar") == 0) {
            if (line[0] != '\0' && line[strlen(line) - 1U] != '/') {
                if (is_adf_path(line)) {
                    foundCount += scan_external_adf_by_md5_list(
                        archivePath, line, md5List, md5Count, outPaths, matched);
                }
                if (is_atari_st_path(line)) {
                    foundCount += scan_external_atari_st_by_md5_list(
                        archivePath, line, md5List, md5Count, outPaths, matched);
                }
                if (is_atari_msa_path(line)) {
                    foundCount += scan_external_atari_msa_by_md5_list(
                        archivePath, line, md5List, md5Count, outPaths, matched);
                }
                char hex[33];
                int matchIndex;
                if (!diskImageOnly && !is_kryoflux_raw_track_path(line) &&
                    external_entry_md5(archivePath, line, hex)) {
                    matchIndex = md5_list_match_index(hex, md5List, NULL, md5Count);
                    if (matchIndex >= 0 &&
                        is_better_zip_entry(line,
                                            hasBest[matchIndex] ? bestNames[matchIndex] : NULL)) {
                        strcpy(bestNames[matchIndex], line);
                        hasBest[matchIndex] = 1;
                    }
                }
            }
            continue;
        }
        if (strncmp(line, "Path = ", 7) == 0) {
            commit = hasEntry && !isFolder;
        }
        if (commit && entryName[0] != '\0' && strcmp(entryName, archivePath) != 0 &&
            entrySize >= 16U && entrySize <= ASSET_TAR_MAX_ENTRY_BYTES) {
            if (is_adf_path(entryName)) {
                foundCount += scan_external_adf_by_md5_list(
                    archivePath, entryName, md5List, md5Count, outPaths, matched);
            }
            if (is_atari_st_path(entryName)) {
                foundCount += scan_external_atari_st_by_md5_list(
                    archivePath, entryName, md5List, md5Count, outPaths, matched);
            }
            if (is_atari_msa_path(entryName)) {
                foundCount += scan_external_atari_msa_by_md5_list(
                    archivePath, entryName, md5List, md5Count, outPaths, matched);
            }
            char hex[33];
            int matchIndex;
            if (!diskImageOnly && !is_kryoflux_raw_track_path(entryName) &&
                external_entry_md5(archivePath, entryName, hex)) {
                matchIndex = md5_list_match_index(hex, md5List, NULL, md5Count);
                if (matchIndex >= 0 &&
                    is_better_zip_entry(entryName,
                                        hasBest[matchIndex] ? bestNames[matchIndex] : NULL)) {
                    strcpy(bestNames[matchIndex], entryName);
                    hasBest[matchIndex] = 1;
                }
            }
        }
        if (strncmp(line, "Path = ", 7) == 0) {
            strncpy(entryName, line + 7, sizeof(entryName) - 1U);
            entryName[sizeof(entryName) - 1U] = '\0';
            entrySize = 0U;
            isFolder = 0;
            hasEntry = 1;
        } else if (strncmp(line, "Size = ", 7) == 0) {
            unsigned long parsed = strtoul(line + 7, NULL, 10);
            entrySize = parsed > UINT32_MAX ? UINT32_MAX : (uint32_t)parsed;
        } else if (strncmp(line, "Folder = +", 10) == 0) {
            isFolder = 1;
        }
    }
    if (hasEntry && !isFolder && entryName[0] != '\0' &&
        strcmp(entryName, archivePath) != 0 &&
        entrySize >= 16U && entrySize <= ASSET_TAR_MAX_ENTRY_BYTES) {
        if (is_adf_path(entryName)) {
            foundCount += scan_external_adf_by_md5_list(
                archivePath, entryName, md5List, md5Count, outPaths, matched);
        }
        if (is_atari_st_path(entryName)) {
            foundCount += scan_external_atari_st_by_md5_list(
                archivePath, entryName, md5List, md5Count, outPaths, matched);
        }
        if (is_atari_msa_path(entryName)) {
            foundCount += scan_external_atari_msa_by_md5_list(
                archivePath, entryName, md5List, md5Count, outPaths, matched);
        }
        char hex[33];
        int matchIndex;
        if (!diskImageOnly && !is_kryoflux_raw_track_path(entryName) &&
            external_entry_md5(archivePath, entryName, hex)) {
            matchIndex = md5_list_match_index(hex, md5List, NULL, md5Count);
            if (matchIndex >= 0 &&
                is_better_zip_entry(entryName,
                                    hasBest[matchIndex] ? bestNames[matchIndex] : NULL)) {
                strcpy(bestNames[matchIndex], entryName);
                hasBest[matchIndex] = 1;
            }
        }
    }
    (void)pclose(pipe);
    for (i = 0; i < md5Count; ++i) {
        if (!hasBest[i] || matched[i]) continue;
        if (copy_virtual_match_path(archivePath, bestNames[i],
                                    outPaths[i], ASSET_PATH_MAX)) {
            matched[i] = 1;
            ++foundCount;
        }
    }
    return foundCount;
}
#else
static int scan_chd_by_md5(const char *chdPath, const char *expectedMd5,
                           char *outPath, int outPathLen) {
    (void)chdPath;
    (void)expectedMd5;
    (void)outPath;
    (void)outPathLen;
    return 0;
}

static int scan_chd_by_md5_list(const char *chdPath,
                                const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[]) {
    (void)chdPath;
    (void)md5List;
    (void)md5Count;
    (void)outPaths;
    (void)matched;
    return 0;
}

static int chd_extract_entry_to_path(const char *chdPath,
                                     const char *entryName,
                                     const char *outFilePath) {
    (void)chdPath;
    (void)entryName;
    (void)outFilePath;
    return 0;
}

static int scan_external_archive_by_md5(const char *archivePath,
                                        const char *expectedMd5,
                                        char *outPath,
                                        int outPathLen) {
    (void)archivePath;
    (void)expectedMd5;
    (void)outPath;
    (void)outPathLen;
    return 0;
}

static int scan_external_archive_by_md5_list(const char *archivePath,
                                             const char *const *md5List,
                                             int md5Count,
                                             char outPaths[][ASSET_PATH_MAX],
                                             int matched[]) {
    (void)archivePath;
    (void)md5List;
    (void)md5Count;
    (void)outPaths;
    (void)matched;
    return 0;
}

static int external_extract_entry_to_path(const char *archivePath,
                                          const char *entryName,
                                          const char *outFilePath) {
    (void)archivePath;
    (void)entryName;
    (void)outFilePath;
    return 0;
}

#endif

/* ── Missing-extractor diagnostics ───────────────────────────────
 * When the scanner meets an external archive (.7z, .rar, .cab, ...) but
 * no supported extractor (7zz/7z/bsdtar) is installed, the archive is
 * silently skipped by the hash scan. Record a bounded, deduplicated
 * diagnostic so the launcher / --scan-data output can tell the user
 * which archives were skipped and which tool would unlock them.
 * The store is process-global and single-threaded by convention (the
 * scanner runs on one thread); callers clear it explicitly before a
 * fresh multi-game scan pass. */
#define ASSET_SCAN_MISSING_EXTRACTOR_MAX 16

static char g_missingExtractorPaths[ASSET_SCAN_MISSING_EXTRACTOR_MAX][ASSET_PATH_MAX];
static char g_missingExtractorTools[ASSET_SCAN_MISSING_EXTRACTOR_MAX][24];
static int g_missingExtractorCount;

void asset_scan_clear_missing_extractor_diagnostics(void) {
    g_missingExtractorCount = 0;
}

int asset_scan_missing_extractor_count(void) {
    return g_missingExtractorCount;
}

const char *asset_scan_missing_extractor_path(int index) {
    if (index < 0 || index >= g_missingExtractorCount) return NULL;
    return g_missingExtractorPaths[index];
}

const char *asset_scan_missing_extractor_tools(int index) {
    if (index < 0 || index >= g_missingExtractorCount) return NULL;
    return g_missingExtractorTools[index];
}

static void record_missing_tool(const char *mediaPath, const char *tools) {
    int i;
    if (!mediaPath || mediaPath[0] == '\0' || !tools || tools[0] == '\0') return;
    for (i = 0; i < g_missingExtractorCount; ++i) {
        if (strcmp(g_missingExtractorPaths[i], mediaPath) == 0) return;
    }
    if (g_missingExtractorCount >= ASSET_SCAN_MISSING_EXTRACTOR_MAX) return;
    snprintf(g_missingExtractorPaths[g_missingExtractorCount],
             ASSET_PATH_MAX, "%s", mediaPath);
    snprintf(g_missingExtractorTools[g_missingExtractorCount],
             sizeof(g_missingExtractorTools[0]), "%s", tools);
    ++g_missingExtractorCount;
}

static void record_missing_extractor(const char *archivePath) {
    const char *tools;
    if (!archivePath || archivePath[0] == '\0') return;
    tools = is_external_tar_archive_path(archivePath)
                ? "bsdtar/7zz/7z"
                : has_case_suffix(archivePath, ".rar")
                    ? "unrar/7zz/7z/bsdtar"
                    : "7zz/7z/bsdtar";
    record_missing_tool(archivePath, tools);
}

static int external_tool_available_for_path(const char *path) {
    /* Test/CI escape hatch: force the "no extractor installed" branch so the
     * diagnostic path is verifiable on hosts that have 7z/bsdtar. */
    if (getenv("FIRESTAFF_TEST_DISABLE_EXTERNAL_ARCHIVE_TOOLS") != NULL) {
        (void)path;
        return 0;
    }
#ifdef _WIN32
    /* The external-extractor shell-out path is POSIX-only; on Windows an
     * external archive is always skipped. */
    (void)path;
    return 0;
#else
    return external_archive_tool_for_path(path) != NULL;
#endif
}

/* GEMDOS FAT12 reader for ordinary Atari ST .ST floppy images.
 *
 * This deliberately accepts only the standard, contiguous sector image and
 * bounded root-directory files. Protected STX has different transport
 * semantics; standard MSA is decoded below into this same sector layout.
 * ReDMCSB COMPILE.H A31E/A35E is Amiga-specific; Atari CSB S20E/S21E uses
 * GEMDOS filenames such as GRAPHICS.DAT and DUNGEON.DAT. */
static uint16_t atari_st_le16(const uint8_t *p) {
    return (uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8);
}

static uint32_t atari_st_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int atari_st_range_ok(size_t offset, size_t count, size_t size) {
    return offset <= size && count <= size - offset;
}

/* Magic Shadow Archiver (MSA) is a transport for the same logical-sector
 * layout as a raw .ST image.  Atari Wiki's STTOMSA format note specifies the
 * big-endian 0x0e0f header, track/side order, and 0xe5/value/BE16 RLE runs.
 * Decode only bounded standard tracks, then use the ordinary GEMDOS reader;
 * no filename or game-specific media assumption is involved. */
static uint16_t atari_st_be16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] << 8) | (uint16_t)p[1];
}

static uint8_t *atari_msa_decode_image(const uint8_t *source, size_t source_size,
                                       size_t *out_size) {
    uint16_t sectors, sides, first_track, last_track;
    size_t track_size, track_count, total, input = 10u, output = 0u;
    uint8_t *image;
    size_t track;
    if (!source || !out_size || source_size < 12u ||
        atari_st_be16(source) != 0x0e0fu) return NULL;
    sectors = atari_st_be16(source + 2u);
    sides = atari_st_be16(source + 4u);
    first_track = atari_st_be16(source + 6u);
    last_track = atari_st_be16(source + 8u);
    if (sectors == 0u || sectors > 36u || sides > 1u ||
        last_track < first_track) return NULL;
    track_size = (size_t)sectors * 512u;
    track_count = ((size_t)last_track - first_track + 1u) * ((size_t)sides + 1u);
    if (track_count == 0u || track_count > ASSET_SCAN_MAX_FILE_BYTES / track_size ||
        (total = track_count * track_size) > ASSET_SCAN_MAX_FILE_BYTES) return NULL;
    image = (uint8_t *)malloc(total);
    if (!image) return NULL;
    for (track = 0u; track < track_count; ++track) {
        uint16_t stored_size;
        size_t end, written = 0u;
        if (!atari_st_range_ok(input, 2u, source_size)) goto fail;
        stored_size = atari_st_be16(source + input);
        input += 2u;
        if (stored_size == 0u || !atari_st_range_ok(input, stored_size, source_size)) goto fail;
        end = input + stored_size;
        if (stored_size == track_size) {
            memcpy(image + output, source + input, track_size);
            input = end;
            output += track_size;
            continue;
        }
        while (input < end && written < track_size) {
            uint8_t byte = source[input++];
            if (byte != 0xe5u) {
                image[output + written++] = byte;
            } else {
                uint16_t run;
                uint8_t value;
                if (end - input < 3u) goto fail;
                value = source[input++];
                run = atari_st_be16(source + input);
                input += 2u;
                if (run == 0u || run > track_size - written) goto fail;
                memset(image + output + written, value, run);
                written += run;
            }
        }
        if (input != end || written != track_size) goto fail;
        output += track_size;
    }
    if (input != source_size || output != total) goto fail;
    *out_size = total;
    return image;
fail:
    free(image);
    return NULL;
}

static int atari_st_entry_name(const uint8_t entry[32], char out[13]) {
    size_t base = 8u;
    size_t ext = 3u;
    size_t pos = 0u;
    size_t i;
    if (!entry || !out || entry[0] == 0u || entry[0] == 0xe5u ||
        (entry[11] & 0x18u) != 0u) return 0;
    while (base > 0u && entry[base - 1u] == ' ') --base;
    while (ext > 0u && entry[8u + ext - 1u] == ' ') --ext;
    if (base == 0u || base > 8u || ext > 3u) return 0;
    for (i = 0u; i < base; ++i) out[pos++] = (char)toupper(entry[i]);
    if (ext > 0u) {
        out[pos++] = '.';
        for (i = 0u; i < ext; ++i) out[pos++] = (char)toupper(entry[8u + i]);
    }
    out[pos] = '\0';
    return 1;
}

static int atari_st_visit_files(const uint8_t *image, size_t image_size,
                                AtariStFileVisitor visitor, void *user_data) {
    uint16_t bytes_per_sector;
    uint16_t reserved_sectors;
    uint16_t root_entries;
    uint16_t sectors_per_fat;
    uint32_t total_sectors;
    uint32_t root_start;
    uint32_t root_sectors;
    uint32_t data_start;
    uint32_t cluster_count;
    uint8_t sectors_per_cluster;
    uint8_t fat_count;
    const uint8_t *fat;
    uint32_t entry_index;
    if (!image || !visitor || image_size < 512u) return -1;
    bytes_per_sector = atari_st_le16(image + 11u);
    sectors_per_cluster = image[13u];
    reserved_sectors = atari_st_le16(image + 14u);
    fat_count = image[16u];
    root_entries = atari_st_le16(image + 17u);
    sectors_per_fat = atari_st_le16(image + 22u);
    total_sectors = atari_st_le16(image + 19u);
    if (total_sectors == 0u) total_sectors = atari_st_le32(image + 32u);
    if (bytes_per_sector != 512u || sectors_per_cluster == 0u || fat_count == 0u ||
        fat_count > 2u || reserved_sectors == 0u || root_entries == 0u ||
        sectors_per_fat == 0u || total_sectors == 0u ||
        (uint64_t)total_sectors * bytes_per_sector > image_size) return -1;
    root_sectors = ((uint32_t)root_entries * 32u + bytes_per_sector - 1u) /
                   bytes_per_sector;
    root_start = (uint32_t)reserved_sectors + (uint32_t)fat_count * sectors_per_fat;
    data_start = root_start + root_sectors;
    if (data_start >= total_sectors || !atari_st_range_ok(
            (size_t)reserved_sectors * bytes_per_sector,
            (size_t)sectors_per_fat * bytes_per_sector, image_size) ||
        !atari_st_range_ok((size_t)root_start * bytes_per_sector,
                           (size_t)root_sectors * bytes_per_sector, image_size)) return -1;
    cluster_count = (total_sectors - data_start) / sectors_per_cluster;
    if (cluster_count < 1u || cluster_count >= 4085u) return -1;
    fat = image + (size_t)reserved_sectors * bytes_per_sector;
    for (entry_index = 0u; entry_index < root_entries; ++entry_index) {
        const uint8_t *entry = image + (size_t)root_start * bytes_per_sector +
                               (size_t)entry_index * 32u;
        uint32_t first_cluster;
        uint32_t file_size;
        uint8_t *bytes;
        size_t written = 0u;
        uint32_t cluster;
        uint32_t hops = 0u;
        char name[13];
        int result;
        if (entry[0] == 0u) break;
        if (!atari_st_entry_name(entry, name)) continue;
        first_cluster = atari_st_le16(entry + 26u);
        file_size = atari_st_le32(entry + 28u);
        if (file_size > ASSET_SCAN_MAX_FILE_BYTES ||
            (file_size != 0u && (first_cluster < 2u || first_cluster >= cluster_count + 2u))) return -1;
        bytes = (uint8_t *)malloc(file_size ? file_size : 1u);
        if (!bytes) return -1;
        cluster = first_cluster;
        while (written < file_size) {
            uint32_t fat_offset;
            uint32_t next;
            size_t offset;
            size_t chunk;
            if (cluster < 2u || cluster >= cluster_count + 2u || ++hops > cluster_count) {
                free(bytes); return -1;
            }
            offset = (size_t)(data_start + (cluster - 2u) * sectors_per_cluster) * bytes_per_sector;
            chunk = (size_t)sectors_per_cluster * bytes_per_sector;
            if (!atari_st_range_ok(offset, chunk, image_size)) { free(bytes); return -1; }
            if (chunk > file_size - written) chunk = file_size - written;
            memcpy(bytes + written, image + offset, chunk);
            written += chunk;
            if (written == file_size) break;
            fat_offset = cluster + cluster / 2u;
            if (fat_offset + 1u >= (uint32_t)sectors_per_fat * bytes_per_sector) {
                free(bytes); return -1;
            }
            next = (cluster & 1u) ? ((fat[fat_offset] >> 4) | ((uint32_t)fat[fat_offset + 1u] << 4))
                                 : (fat[fat_offset] | ((uint32_t)(fat[fat_offset + 1u] & 0x0fu) << 8));
            cluster = next & 0xfffu;
        }
        result = visitor(name, bytes, file_size, user_data);
        free(bytes);
        if (result != 0) return result;
    }
    return 0;
}

/* AmigaDOS OFS disks are filesystems, not opaque archives. Load the bounded
 * image once and ask the shared reader for its validated files. This keeps
 * ADF discovery independent of a host-side extractor and works for every
 * supported game rather than naming CSB files specially. */
static uint8_t *adf_load_image(const char *path, size_t *outSize) {
    FILE *fp;
    long size;
    uint8_t *image;
    if (!path || !outSize || !(fp = fopen(path, "rb"))) return NULL;
    if (fseek(fp, 0L, SEEK_END) != 0 || (size = ftell(fp)) < 0L ||
        size > ASSET_SCAN_MAX_FILE_BYTES || fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    image = (uint8_t *)malloc((size_t)size);
    if (!image || fread(image, 1U, (size_t)size, fp) != (size_t)size) {
        free(image);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    *outSize = (size_t)size;
    return image;
}

typedef struct {
    const char *expected_md5;
    char name[31];
    int found;
} AdfSingleMatch;

static int adf_find_single_visitor(const char *name, const uint8_t *bytes,
                                   size_t byte_count, void *user_data) {
    AdfSingleMatch *match = (AdfSingleMatch *)user_data;
    AssetMd5Ctx ctx;
    char hex[33];
    md5_init(&ctx);
    md5_update(&ctx, bytes, (unsigned int)byte_count);
    md5_final(&ctx, hex);
    if (strcmp(hex, match->expected_md5) == 0 &&
        (!match->found || strcmp(name, match->name) < 0)) {
        snprintf(match->name, sizeof(match->name), "%s", name);
        match->found = 1;
    }
    return 0;
}

static int scan_adf_by_md5(const char *adfPath, const char *expectedMd5,
                           char *outPath, int outPathLen) {
    uint8_t *image;
    size_t imageSize;
    AdfSingleMatch match;
    int result;
    memset(&match, 0, sizeof(match));
    match.expected_md5 = expectedMd5;
    image = adf_load_image(adfPath, &imageSize);
    if (!image) return 0;
    result = firestaff_amiga_adf_visit_ofs_files(image, imageSize,
                                                  adf_find_single_visitor, &match);
    free(image);
    return result >= 0 && match.found &&
           copy_virtual_match_path(adfPath, match.name, outPath, outPathLen);
}

typedef struct {
    const char *container;
    const char *const *md5_list;
    int md5_count;
    char (*out_paths)[ASSET_PATH_MAX];
    int *matched;
    int found_count;
} AdfListMatch;

static int adf_find_list_visitor(const char *name, const uint8_t *bytes,
                                 size_t byte_count, void *user_data) {
    AdfListMatch *matches = (AdfListMatch *)user_data;
    AssetMd5Ctx ctx;
    char hex[33];
    int index;
    md5_init(&ctx);
    md5_update(&ctx, bytes, (unsigned int)byte_count);
    md5_final(&ctx, hex);
    index = md5_list_match_index(hex, matches->md5_list, matches->matched,
                                 matches->md5_count);
    if (index >= 0) {
        int should_update = !matches->matched[index];
        if (!should_update) {
            const char *old_name = strstr(matches->out_paths[index], "::");
            should_update = old_name != NULL && strcmp(name, old_name + 2) < 0;
        }
        if (should_update && copy_virtual_match_path(matches->container, name,
                                                      matches->out_paths[index],
                                                      ASSET_PATH_MAX)) {
            if (!matches->matched[index]) {
                matches->matched[index] = 1;
                ++matches->found_count;
            }
        }
    }
    return 0;
}

static int scan_adf_by_md5_list(const char *adfPath, const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX], int matched[]) {
    uint8_t *image;
    size_t imageSize;
    AdfListMatch matches;
    int result;
    if (!md5List || md5Count <= 0 || !outPaths || !matched) return 0;
    image = adf_load_image(adfPath, &imageSize);
    if (!image) return 0;
    memset(&matches, 0, sizeof(matches));
    matches.container = adfPath;
    matches.md5_list = md5List;
    matches.md5_count = md5Count;
    matches.out_paths = outPaths;
    matches.matched = matched;
    result = firestaff_amiga_adf_visit_ofs_files(image, imageSize,
                                                  adf_find_list_visitor, &matches);
    free(image);
    return result < 0 ? 0 : matches.found_count;
}

typedef struct {
    const char *entry;
    const char *out_path;
    int extracted;
} AdfExtractMatch;

static int adf_extract_visitor(const char *name, const uint8_t *bytes,
                               size_t byte_count, void *user_data) {
    AdfExtractMatch *extract = (AdfExtractMatch *)user_data;
    FILE *out;
    if (asset_casecmp(name, extract->entry) != 0) return 0;
    out = fopen(extract->out_path, "wb");
    if (!out) return -1;
    if (fwrite(bytes, 1U, byte_count, out) != byte_count) {
        fclose(out);
        remove(extract->out_path);
        return -1;
    }
    if (fclose(out) != 0) {
        remove(extract->out_path);
        return -1;
    }
    extract->extracted = 1;
    return 0;
}

static int adf_extract_entry_to_path(const char *adfPath, const char *entry,
                                     const char *outFilePath) {
    uint8_t *image;
    size_t imageSize;
    AdfExtractMatch extract;
    int result;
    if (!entry || !outFilePath) return 0;
    image = adf_load_image(adfPath, &imageSize);
    if (!image) return 0;
    extract.entry = entry;
    extract.out_path = outFilePath;
    extract.extracted = 0;
    result = firestaff_amiga_adf_visit_ofs_files(image, imageSize,
                                                  adf_extract_visitor, &extract);
    free(image);
    return result >= 0 && extract.extracted;
}

static int scan_atari_st_by_md5(const char *st_path, const char *expected_md5,
                                char *out_path, int out_path_len) {
    uint8_t *image;
    size_t image_size;
    AdfSingleMatch match;
    int result;
    if (!st_path || !expected_md5) return 0;
    memset(&match, 0, sizeof(match));
    match.expected_md5 = expected_md5;
    image = adf_load_image(st_path, &image_size);
    if (!image) return 0;
    result = atari_st_visit_files(image, image_size, adf_find_single_visitor, &match);
    free(image);
    return result >= 0 && match.found &&
           copy_virtual_match_path(st_path, match.name, out_path, out_path_len);
}

static int scan_atari_st_by_md5_list(const char *st_path,
                                     const char *const *md5_list, int md5_count,
                                     char out_paths[][ASSET_PATH_MAX], int matched[]) {
    uint8_t *image;
    size_t image_size;
    AdfListMatch matches;
    int result;
    if (!st_path || !md5_list || md5_count <= 0 || !out_paths || !matched) return 0;
    image = adf_load_image(st_path, &image_size);
    if (!image) return 0;
    memset(&matches, 0, sizeof(matches));
    matches.container = st_path;
    matches.md5_list = md5_list;
    matches.md5_count = md5_count;
    matches.out_paths = out_paths;
    matches.matched = matched;
    result = atari_st_visit_files(image, image_size, adf_find_list_visitor, &matches);
    free(image);
    return result < 0 ? 0 : matches.found_count;
}

static int atari_st_extract_entry_to_path(const char *st_path, const char *entry,
                                          const char *out_file_path) {
    uint8_t *image;
    size_t image_size;
    AdfExtractMatch extract;
    int result;
    if (!st_path || !entry || !out_file_path) return 0;
    image = adf_load_image(st_path, &image_size);
    if (!image) return 0;
    extract.entry = entry;
    extract.out_path = out_file_path;
    extract.extracted = 0;
    result = atari_st_visit_files(image, image_size, adf_extract_visitor, &extract);
    free(image);
    return result >= 0 && extract.extracted;
}

static int scan_atari_msa_by_md5(const char *msa_path, const char *expected_md5,
                                 char *out_path, int out_path_len) {
    uint8_t *packed, *image;
    size_t packed_size, image_size;
    AdfSingleMatch match;
    int result;
    if (!msa_path || !expected_md5) return 0;
    packed = adf_load_image(msa_path, &packed_size);
    if (!packed) return 0;
    image = atari_msa_decode_image(packed, packed_size, &image_size);
    free(packed);
    if (!image) return 0;
    memset(&match, 0, sizeof(match));
    match.expected_md5 = expected_md5;
    result = atari_st_visit_files(image, image_size, adf_find_single_visitor, &match);
    free(image);
    return result >= 0 && match.found &&
           copy_virtual_match_path(msa_path, match.name, out_path, out_path_len);
}

static int scan_atari_msa_by_md5_list(const char *msa_path,
                                      const char *const *md5_list, int md5_count,
                                      char out_paths[][ASSET_PATH_MAX], int matched[]) {
    uint8_t *packed, *image;
    size_t packed_size, image_size;
    AdfListMatch matches;
    int result;
    if (!msa_path || !md5_list || md5_count <= 0 || !out_paths || !matched) return 0;
    packed = adf_load_image(msa_path, &packed_size);
    if (!packed) return 0;
    image = atari_msa_decode_image(packed, packed_size, &image_size);
    free(packed);
    if (!image) return 0;
    memset(&matches, 0, sizeof(matches));
    matches.container = msa_path;
    matches.md5_list = md5_list;
    matches.md5_count = md5_count;
    matches.out_paths = out_paths;
    matches.matched = matched;
    result = atari_st_visit_files(image, image_size, adf_find_list_visitor, &matches);
    free(image);
    return result < 0 ? 0 : matches.found_count;
}

static int atari_msa_extract_entry_to_path(const char *msa_path, const char *entry,
                                           const char *out_file_path) {
    uint8_t *packed, *image;
    size_t packed_size, image_size;
    AdfExtractMatch extract;
    int result;
    if (!msa_path || !entry || !out_file_path) return 0;
    packed = adf_load_image(msa_path, &packed_size);
    if (!packed) return 0;
    image = atari_msa_decode_image(packed, packed_size, &image_size);
    free(packed);
    if (!image) return 0;
    extract.entry = entry;
    extract.out_path = out_file_path;
    extract.extracted = 0;
    result = atari_st_visit_files(image, image_size, adf_extract_visitor, &extract);
    free(image);
    return result >= 0 && extract.extracted;
}

static int zip_extract_nested_disk_entry_to_path(const char *zip_path,
                                                 const char *disk_entry,
                                                 const char *entry,
                                                 const char *out_file_path) {
    uint8_t *image;
    size_t image_size;
    AdfExtractMatch extract;
    int result = -1;
    if (!zip_path || !disk_entry || !entry || !out_file_path) return 0;
    image = zip_load_entry_bytes(zip_path, disk_entry, &image_size);
    if (!image) return 0;
    extract.entry = entry;
    extract.out_path = out_file_path;
    extract.extracted = 0;
    if (is_adf_path(disk_entry)) {
        result = firestaff_amiga_adf_visit_ofs_files(image, image_size,
                                                      adf_extract_visitor, &extract);
    } else if (is_atari_st_path(disk_entry)) {
        result = atari_st_visit_files(image, image_size, adf_extract_visitor, &extract);
    } else if (is_atari_msa_path(disk_entry)) {
        uint8_t *decoded;
        size_t decoded_size;
        decoded = atari_msa_decode_image(image, image_size, &decoded_size);
        if (decoded) {
            result = atari_st_visit_files(decoded, decoded_size,
                                          adf_extract_visitor, &extract);
            free(decoded);
        }
    }
    free(image);
    return result >= 0 && extract.extracted;
}

#ifndef _WIN32
static int scan_external_adf_by_md5(const char *archive_path,
                                    const char *adf_entry,
                                    const char *expected_md5,
                                    char *out_path, int out_path_len) {
    uint8_t *image;
    size_t image_size;
    AdfSingleMatch match;
    int result;
    if (!archive_path || !adf_entry || !expected_md5) return 0;
    image = external_read_entry_bytes(archive_path, adf_entry, &image_size);
    if (!image) return 0;
    memset(&match, 0, sizeof(match));
    match.expected_md5 = expected_md5;
    result = firestaff_amiga_adf_visit_ofs_files(image, image_size,
                                                  adf_find_single_visitor, &match);
    free(image);
    return result >= 0 && match.found &&
           copy_nested_virtual_match_path(archive_path, adf_entry, match.name,
                                          out_path, out_path_len);
}

typedef struct {
    const char *archive;
    const char *adf;
    const char *const *md5_list;
    int md5_count;
    char (*out_paths)[ASSET_PATH_MAX];
    int *matched;
    int found_count;
} ExternalAdfListMatch;

static int external_adf_find_list_visitor(const char *name, const uint8_t *bytes,
                                          size_t byte_count, void *user_data) {
    ExternalAdfListMatch *matches = (ExternalAdfListMatch *)user_data;
    AssetMd5Ctx ctx;
    char hex[33];
    int index;
    md5_init(&ctx);
    md5_update(&ctx, bytes, (unsigned int)byte_count);
    md5_final(&ctx, hex);
    index = md5_list_match_index(hex, matches->md5_list, matches->matched,
                                 matches->md5_count);
    if (index >= 0 && !matches->matched[index] &&
        copy_nested_virtual_match_path(matches->archive, matches->adf, name,
                                       matches->out_paths[index], ASSET_PATH_MAX)) {
        matches->matched[index] = 1;
        ++matches->found_count;
    }
    return 0;
}

static int scan_external_adf_by_md5_list(const char *archive_path,
                                         const char *adf_entry,
                                         const char *const *md5_list,
                                         int md5_count,
                                         char out_paths[][ASSET_PATH_MAX],
                                         int matched[]) {
    uint8_t *image;
    size_t image_size;
    ExternalAdfListMatch matches;
    int result;
    if (!archive_path || !adf_entry || !md5_list || md5_count <= 0 ||
        !out_paths || !matched) return 0;
    image = external_read_entry_bytes(archive_path, adf_entry, &image_size);
    if (!image) return 0;
    memset(&matches, 0, sizeof(matches));
    matches.archive = archive_path;
    matches.adf = adf_entry;
    matches.md5_list = md5_list;
    matches.md5_count = md5_count;
    matches.out_paths = out_paths;
    matches.matched = matched;
    result = firestaff_amiga_adf_visit_ofs_files(image, image_size,
                                                  external_adf_find_list_visitor,
                                                  &matches);
    free(image);
    return result < 0 ? 0 : matches.found_count;
}
static int external_adf_extract_entry_to_path(const char *archive_path,
                                              const char *adf_entry,
                                              const char *entry,
                                              const char *out_file_path) {
    uint8_t *image;
    size_t image_size;
    AdfExtractMatch extract;
    int result;
    if (!archive_path || !adf_entry || !entry || !out_file_path) return 0;
    image = external_read_entry_bytes(archive_path, adf_entry, &image_size);
    if (!image) return 0;
    extract.entry = entry;
    extract.out_path = out_file_path;
    extract.extracted = 0;
    result = firestaff_amiga_adf_visit_ofs_files(image, image_size,
                                                  adf_extract_visitor, &extract);
    free(image);
    return result >= 0 && extract.extracted;
}

static int scan_external_atari_st_by_md5(const char *archive_path,
                                         const char *st_entry,
                                         const char *expected_md5,
                                         char *out_path, int out_path_len) {
    uint8_t *image;
    size_t image_size;
    AdfSingleMatch match;
    int result;
    if (!archive_path || !st_entry || !expected_md5) return 0;
    image = external_read_entry_bytes(archive_path, st_entry, &image_size);
    if (!image) return 0;
    memset(&match, 0, sizeof(match));
    match.expected_md5 = expected_md5;
    result = atari_st_visit_files(image, image_size, adf_find_single_visitor, &match);
    free(image);
    return result >= 0 && match.found &&
           copy_nested_virtual_match_path(archive_path, st_entry, match.name,
                                          out_path, out_path_len);
}

static int scan_external_atari_st_by_md5_list(
    const char *archive_path, const char *st_entry, const char *const *md5_list,
    int md5_count, char out_paths[][ASSET_PATH_MAX], int matched[]) {
    uint8_t *image;
    size_t image_size;
    AdfListMatch matches;
    char local_paths[64][ASSET_PATH_MAX];
    int local_matched[64];
    int result;
    if (!archive_path || !st_entry || !md5_list || md5_count <= 0 ||
        !out_paths || !matched) return 0;
    image = external_read_entry_bytes(archive_path, st_entry, &image_size);
    if (!image) return 0;
    if (md5_count > 64) { free(image); return 0; }
    memset(&matches, 0, sizeof(matches));
    memset(local_paths, 0, sizeof(local_paths));
    memset(local_matched, 0, sizeof(local_matched));
    matches.container = archive_path;
    matches.md5_list = md5_list;
    matches.md5_count = md5_count;
    matches.out_paths = local_paths;
    matches.matched = local_matched;
    result = atari_st_visit_files(image, image_size, adf_find_list_visitor, &matches);
    if (result >= 0) {
        int added = 0;
        int i;
        for (i = 0; i < md5_count; ++i) {
            const char *inner;
            if (!local_matched[i] || matched[i]) continue;
            inner = strstr(local_paths[i], "::");
            if (!inner || !copy_nested_virtual_match_path(
                    archive_path, st_entry, inner + 2, out_paths[i], ASSET_PATH_MAX)) {
                continue;
            }
            matched[i] = 1;
            ++added;
        }
        matches.found_count = added;
    }
    free(image);
    return result < 0 ? 0 : matches.found_count;
}

static int scan_external_atari_msa_by_md5(const char *archive_path,
                                          const char *msa_entry,
                                          const char *expected_md5,
                                          char *out_path, int out_path_len) {
    uint8_t *packed, *image;
    size_t packed_size, image_size;
    AdfSingleMatch match;
    int result;
    if (!archive_path || !msa_entry || !expected_md5) return 0;
    packed = external_read_entry_bytes(archive_path, msa_entry, &packed_size);
    if (!packed) return 0;
    image = atari_msa_decode_image(packed, packed_size, &image_size);
    free(packed);
    if (!image) return 0;
    memset(&match, 0, sizeof(match));
    match.expected_md5 = expected_md5;
    result = atari_st_visit_files(image, image_size, adf_find_single_visitor, &match);
    free(image);
    return result >= 0 && match.found &&
           copy_nested_virtual_match_path(archive_path, msa_entry, match.name,
                                          out_path, out_path_len);
}

static int scan_external_atari_msa_by_md5_list(
    const char *archive_path, const char *msa_entry, const char *const *md5_list,
    int md5_count, char out_paths[][ASSET_PATH_MAX], int matched[]) {
    uint8_t *packed, *image;
    size_t packed_size, image_size;
    AdfListMatch matches;
    char local_paths[64][ASSET_PATH_MAX];
    int local_matched[64];
    int result;
    if (!archive_path || !msa_entry || !md5_list || md5_count <= 0 ||
        !out_paths || !matched) return 0;
    /* Decode a nested MSA once.  Profile discovery normally asks for both
     * GRAPHICS.DAT and DUNGEON.DAT; extracting the enclosing 7z member once
     * per hash made original Atari media needlessly expensive to scan.
     * ReDMCSB-compatible FAT12 matching remains byte-identical to the
     * direct-MSA path above. */
    packed = external_read_entry_bytes(archive_path, msa_entry, &packed_size);
    if (!packed) return 0;
    image = atari_msa_decode_image(packed, packed_size, &image_size);
    free(packed);
    if (!image) return 0;
    if (md5_count > 64) { free(image); return 0; }
    memset(&matches, 0, sizeof(matches));
    memset(local_paths, 0, sizeof(local_paths));
    memset(local_matched, 0, sizeof(local_matched));
    matches.container = archive_path;
    matches.md5_list = md5_list;
    matches.md5_count = md5_count;
    matches.out_paths = local_paths;
    matches.matched = local_matched;
    result = atari_st_visit_files(image, image_size, adf_find_list_visitor, &matches);
    if (result >= 0) {
        int added = 0;
        int i;
        for (i = 0; i < md5_count; ++i) {
            const char *inner;
            if (!local_matched[i] || matched[i]) continue;
            inner = strstr(local_paths[i], "::");
            if (!inner || !copy_nested_virtual_match_path(
                    archive_path, msa_entry, inner + 2, out_paths[i], ASSET_PATH_MAX)) {
                continue;
            }
            matched[i] = 1;
            ++added;
        }
        matches.found_count = added;
    }
    free(image);
    return result < 0 ? 0 : matches.found_count;
}

static int external_atari_msa_extract_entry_to_path(const char *archive_path,
                                                    const char *msa_entry,
                                                    const char *entry,
                                                    const char *out_file_path) {
    uint8_t *packed, *image;
    size_t packed_size, image_size;
    AdfExtractMatch extract;
    int result;
    if (!archive_path || !msa_entry || !entry || !out_file_path) return 0;
    packed = external_read_entry_bytes(archive_path, msa_entry, &packed_size);
    if (!packed) return 0;
    image = atari_msa_decode_image(packed, packed_size, &image_size);
    free(packed);
    if (!image) return 0;
    extract.entry = entry;
    extract.out_path = out_file_path;
    extract.extracted = 0;
    result = atari_st_visit_files(image, image_size, adf_extract_visitor, &extract);
    free(image);
    return result >= 0 && extract.extracted;
}

static int external_atari_st_extract_entry_to_path(const char *archive_path,
                                                   const char *st_entry,
                                                   const char *entry,
                                                   const char *out_file_path) {
    uint8_t *image;
    size_t image_size;
    AdfExtractMatch extract;
    int result;
    if (!archive_path || !st_entry || !entry || !out_file_path) return 0;
    image = external_read_entry_bytes(archive_path, st_entry, &image_size);
    if (!image) return 0;
    extract.entry = entry;
    extract.out_path = out_file_path;
    extract.extracted = 0;
    result = atari_st_visit_files(image, image_size, adf_extract_visitor, &extract);
    free(image);
    return result >= 0 && extract.extracted;
}
#else
static int external_adf_extract_entry_to_path(const char *archivePath,
                                              const char *adfEntry,
                                              const char *entry,
                                              const char *outFilePath) {
    /* External archive extraction uses POSIX pipes and is unavailable here. */
    (void)archivePath;
    (void)adfEntry;
    (void)entry;
    (void)outFilePath;
    return 0;
}

/* Keep virtual nested-disk extraction fail-closed on Windows as well.  The
 * external archive reader above depends on POSIX pipes, so these formats
 * cannot be materialized there until a Windows-native archive backend is
 * supplied.  Matching direct .st/.msa files is still handled in-process. */
static int external_atari_st_extract_entry_to_path(const char *archivePath,
                                                   const char *stEntry,
                                                   const char *entry,
                                                   const char *outFilePath) {
    (void)archivePath;
    (void)stEntry;
    (void)entry;
    (void)outFilePath;
    return 0;
}

static int external_atari_msa_extract_entry_to_path(const char *archivePath,
                                                    const char *msaEntry,
                                                    const char *entry,
                                                    const char *outFilePath) {
    (void)archivePath;
    (void)msaEntry;
    (void)entry;
    (void)outFilePath;
    return 0;
}
#endif

static int scan_container_by_md5(const char *path, const char *expectedMd5,
                                 char *outPath, int outPathLen) {
    AssetContainerKind kind = asset_container_kind_for_path(path);
    if (kind == ASSET_CONTAINER_ZIP) {
        if (scan_zip_by_md5(path, expectedMd5, outPath, outPathLen)) {
            return 1;
        }
        {
            const char *const md5_list[] = {expectedMd5};
            char nested_paths[1][ASSET_PATH_MAX];
            int nested_matched[1] = {0};
            if (scan_zip_nested_disk_by_md5_list(path, md5_list, 1,
                                                 nested_paths, nested_matched) > 0 &&
                nested_matched[0] &&
                copy_match_path(nested_paths[0], outPath, outPathLen)) {
                return 1;
            }
        }
        /* Retail Amiga releases are often ZIP → ADF.  The in-process ZIP
         * path handles standard deflate and disk filesystems.  Preserve the
         * external route only for a ZIP compression method we do not own. */
        return external_tool_available_for_path(path)
                   ? scan_external_archive_by_md5(path, expectedMd5,
                                                  outPath, outPathLen)
                   : 0;
    }
    if (kind == ASSET_CONTAINER_ISO) {
        return scan_iso_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_CUE) {
        return scan_cue_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_TAR) {
        return tar_scan_file_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_TGZ) {
        return scan_tgz_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_GZIP) {
        return scan_gzip_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_LHA) {
        if (scan_lha_by_md5(path, expectedMd5, outPath, outPathLen)) {
            return 1;
        }
        return scan_external_archive_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_CHD) {
        return scan_chd_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_ADF) {
        return scan_adf_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_ATARI_ST) {
        return scan_atari_st_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_ATARI_MSA) {
        return scan_atari_msa_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    if (kind == ASSET_CONTAINER_EXTERNAL) {
        if (!external_tool_available_for_path(path)) {
            record_missing_extractor(path);
            return 0;
        }
        return scan_external_archive_by_md5(path, expectedMd5, outPath, outPathLen);
    }
    return 0;
}

static int scan_container_by_md5_list(const char *path, const char *const *md5List,
                                      int md5Count,
                                      char outPaths[][ASSET_PATH_MAX],
                                      int matched[]) {
    AssetContainerKind kind = asset_container_kind_for_path(path);
    if (kind == ASSET_CONTAINER_ZIP) {
        int found = scan_zip_by_md5_list(path, md5List, md5Count,
                                         outPaths, matched);
        if (found < md5Count) {
            found += scan_zip_nested_disk_by_md5_list(path, md5List, md5Count,
                                                       outPaths, matched);
        }
        if (found >= md5Count || !external_tool_available_for_path(path)) {
            return found;
        }
        return found + scan_external_archive_by_md5_list(path, md5List,
                                                         md5Count, outPaths,
                                                         matched);
    }
    if (kind == ASSET_CONTAINER_ISO) {
        return scan_iso_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_CUE) {
        return scan_cue_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_TAR) {
        return tar_scan_file_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_TGZ) {
        return scan_tgz_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_GZIP) {
        return scan_gzip_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_LHA) {
        int found = scan_lha_by_md5_list(path, md5List, md5Count, outPaths, matched);
        if (found >= md5Count) return found;
        return found + scan_external_archive_by_md5_list(path, md5List, md5Count,
                                                         outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_CHD) {
        return scan_chd_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_ADF) {
        return scan_adf_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_ATARI_ST) {
        return scan_atari_st_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_ATARI_MSA) {
        return scan_atari_msa_by_md5_list(path, md5List, md5Count, outPaths, matched);
    }
    if (kind == ASSET_CONTAINER_EXTERNAL) {
        if (!external_tool_available_for_path(path)) {
            record_missing_extractor(path);
            return 0;
        }
        return scan_external_archive_by_md5_list(path, md5List, md5Count,
                                                 outPaths, matched);
    }
    return 0;
}

/* ── Recursive directory scanner ──────────────────────────────── */

#ifndef _WIN32
static int scan_dir_by_md5_list(const char *dir, const char *const *md5List,
                                int md5Count,
                                char outPaths[][ASSET_PATH_MAX],
                                int matched[], int depth, int maxDepth,
                                int scanContainers) {
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
            if (is_supported_container_path(path)) {
                if (!scanContainers) {
                    continue;
                }
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
                                               depth + 1, maxDepth,
                                               scanContainers);
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
            if (is_supported_container_path(path) &&
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
                                int matched[], int depth, int maxDepth,
                                int scanContainers) {
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
                                               depth + 1, maxDepth,
                                               scanContainers);
            if (foundCount >= md5Count) {
                FindClose(h);
                return foundCount;
            }
        } else {
            LARGE_INTEGER sz;
            int matchIndex;
            sz.LowPart = fd.nFileSizeLow;
            sz.HighPart = fd.nFileSizeHigh;
            if (is_supported_container_path(path)) {
                if (!scanContainers) {
                    continue;
                }
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
            if (is_supported_container_path(path) &&
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

/* ── Scan cache lifecycle ─────────────────────────────────────── */

static ScanCache_I s_scan_cache_storage;
static int s_scan_cache_active;

static void scan_cache_begin(void) {
    if (s_scan_cache_active) return;
    scache_init(&s_scan_cache_storage);
    scache_load(&s_scan_cache_storage);
    s_scan_cache = &s_scan_cache_storage;
    s_scan_cache_active = 1;
}

static void scan_cache_end(void) {
    if (!s_scan_cache_active) return;
    scache_save(&s_scan_cache_storage);
    s_scan_cache = NULL;
    s_scan_cache_active = 0;
}

/* ── Public API ───────────────────────────────────────────────── */

int asset_find_by_md5(const char *searchDir, const char *expectedMd5,
                      char *outPath, int outPathLen, int maxDepth) {
    char normalizedMd5[33];
    char actualMd5[33];
    int result;
    if (!searchDir || !expectedMd5 || !outPath || outPathLen <= 0) return 0;
    if (!normalize_md5(expectedMd5, normalizedMd5)) return 0;
    if (maxDepth < 0) maxDepth = 3;
    scan_cache_begin();
    if (scan_container_by_md5(searchDir, normalizedMd5, outPath, outPathLen)) {
        scan_cache_end();
        return 1;
    }
    /* Keep the normalized caller expectation separate from the root-file
     * digest.  Reusing normalizedMd5 here made every direct file search a
     * false positive after file_md5() overwrote the expected value. */
    if (file_md5(searchDir, actualMd5) &&
        strcmp(actualMd5, normalizedMd5) == 0 &&
        copy_match_path(searchDir, outPath, outPathLen)) {
        scan_cache_end();
        return 1;
    }
    result = scan_dir(searchDir, normalizedMd5, outPath, outPathLen, 0, maxDepth);
    scan_cache_end();
    return result;
}

int asset_find_by_md5_list(const char *searchDir, const char *const *md5List,
                           char *outPath, int outPathLen,
                           int *outMatchIndex, int maxDepth) {
    char (*normalized)[33];
    const char **normalizedPtrs;
    char (*foundPaths)[ASSET_PATH_MAX];
    int *matched;
    int *originalIndices;
    int inputCount = 0;
    int normalizedCount = 0;
    int i;
    if (!searchDir || !md5List || !outPath || outPathLen <= 0) return 0;
    if (maxDepth < 0) maxDepth = 3;
    while (md5List[inputCount] != NULL) {
        ++inputCount;
    }
    if (inputCount == 0) return 0;
    scan_cache_begin();
    normalized = (char (*)[33])calloc((size_t)inputCount, sizeof(*normalized));
    normalizedPtrs = (const char**)calloc((size_t)inputCount + 1U, sizeof(*normalizedPtrs));
    foundPaths = (char (*)[ASSET_PATH_MAX])calloc((size_t)inputCount, sizeof(*foundPaths));
    matched = (int*)calloc((size_t)inputCount, sizeof(*matched));
    originalIndices = (int*)calloc((size_t)inputCount, sizeof(*originalIndices));
    if (!normalized || !normalizedPtrs || !foundPaths || !matched || !originalIndices) {
        free(normalized);
        free(normalizedPtrs);
        free(foundPaths);
        free(matched);
        free(originalIndices);
        scan_cache_end();
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
        /* Keep direct archive selection equivalent to selecting its parent
         * directory. This is needed for launcher-selected .7z/.zip images. */
        (void)scan_container_by_md5_list(searchDir, normalizedPtrs,
                                         normalizedCount, foundPaths, matched);
        if (!matched[0]) {
            char directHex[33];
            int directIndex;
            if (file_md5(searchDir, directHex)) {
                directIndex = md5_list_match_index(directHex, normalizedPtrs,
                                                   matched, normalizedCount);
                if (directIndex >= 0 &&
                    copy_match_path(searchDir, foundPaths[directIndex],
                                    ASSET_PATH_MAX)) {
                    matched[directIndex] = 1;
                }
            }
        }
        (void)scan_dir_by_md5_list(searchDir, normalizedPtrs, normalizedCount,
                                   foundPaths, matched, 0, maxDepth, 1);
        for (i = 0; i < normalizedCount; ++i) {
            if (matched[i] &&
                copy_match_path(foundPaths[i], outPath, outPathLen)) {
                if (outMatchIndex) *outMatchIndex = originalIndices[i];
                free(normalized);
                free(normalizedPtrs);
                free(foundPaths);
                free(matched);
                free(originalIndices);
                scan_cache_end();
                return 1;
            }
        }
    }
    free(normalized);
    free(normalizedPtrs);
    free(foundPaths);
    free(matched);
    free(originalIndices);
    scan_cache_end();
    return 0;
}

static int asset_find_all_by_md5_list_internal(
    const char *searchDir, const char *const *md5List,
    char outPaths[][ASSET_PATH_MAX], int *outMatched, int maxMatches,
    int maxDepth, int scanContainers) {
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
        /* A selected archive is itself a valid hash-search root.  The
         * single-hash API has always handled this case, but the batched API
         * previously called opendir() only and silently returned no archive
         * members.  That made a direct --data-dir <game.7z> fall back to a
         * sibling installation. */
        if (scanContainers) {
            foundCount += scan_container_by_md5_list(
                searchDir, normalizedPtrs, normalizedCount, normalizedPaths,
                normalizedMatched);
        }
        if (foundCount < normalizedCount) {
            char direct_hex[33];
            int direct_index;
            if (file_md5(searchDir, direct_hex) &&
                (direct_index = md5_list_match_index(
                     direct_hex, normalizedPtrs, normalizedMatched,
                     normalizedCount)) >= 0 &&
                copy_match_path(searchDir, normalizedPaths[direct_index],
                                ASSET_PATH_MAX)) {
                normalizedMatched[direct_index] = 1;
                ++foundCount;
            }
        }
        foundCount += scan_dir_by_md5_list(searchDir, normalizedPtrs,
                                            normalizedCount, normalizedPaths,
                                            normalizedMatched, 0, maxDepth,
                                            scanContainers);
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

int asset_find_all_by_md5_list(const char *searchDir, const char *const *md5List,
                               char outPaths[][ASSET_PATH_MAX],
                               int *outMatched, int maxMatches,
                               int maxDepth) {
    int result;
    scan_cache_begin();
    result = asset_find_all_by_md5_list_internal(searchDir, md5List, outPaths,
                                                  outMatched, maxMatches, maxDepth, 1);
    scan_cache_end();
    return result;
}

int asset_find_all_files_by_md5_list(const char *searchDir,
                                     const char *const *md5List,
                                     char outPaths[][ASSET_PATH_MAX],
                                     int *outMatched, int maxMatches,
                                     int maxDepth) {
    int result;
    scan_cache_begin();
    result = asset_find_all_by_md5_list_internal(searchDir, md5List, outPaths,
                                                  outMatched, maxMatches, maxDepth, 0);
    scan_cache_end();
    return result;
}

int asset_extract_virtual_path(const char *virtualPath, const char *outFilePath) {
    const char *sep;
    char container[ASSET_PATH_MAX];
    const char *entry;
    size_t containerLen;
    AssetContainerKind kind;
    if (!virtualPath || !outFilePath) return 0;
    sep = strstr(virtualPath, "::");
    if (!sep) return 0;
    containerLen = (size_t)(sep - virtualPath);
    if (containerLen == 0U || containerLen >= sizeof(container)) return 0;
    memcpy(container, virtualPath, containerLen);
    container[containerLen] = '\0';
    entry = sep + 2;
    if (entry[0] == '\0') return 0;
    kind = asset_container_kind_for_path(container);
    if (kind == ASSET_CONTAINER_ZIP) {
        const char *nested = strstr(entry, "::");
        if (nested) {
            char disk_entry[ASSET_PATH_MAX];
            size_t disk_length = (size_t)(nested - entry);
            if (disk_length == 0U || disk_length >= sizeof(disk_entry) ||
                nested[2] == '\0') {
                return 0;
            }
            memcpy(disk_entry, entry, disk_length);
            disk_entry[disk_length] = '\0';
            if (zip_extract_nested_disk_entry_to_path(container, disk_entry,
                                                      nested + 2, outFilePath)) {
                return 1;
            }
            if (!external_tool_available_for_path(container)) return 0;
            if (is_atari_st_path(disk_entry)) {
                return external_atari_st_extract_entry_to_path(
                    container, disk_entry, nested + 2, outFilePath);
            }
            if (is_atari_msa_path(disk_entry)) {
                return external_atari_msa_extract_entry_to_path(
                    container, disk_entry, nested + 2, outFilePath);
            }
            return external_adf_extract_entry_to_path(container, disk_entry,
                                                      nested + 2, outFilePath);
        }
        return zip_extract_entry_to_path(container, entry, outFilePath);
    }
    if (kind == ASSET_CONTAINER_TAR) {
        return tar_extract_file_entry(container, entry, outFilePath);
    }
    if (kind == ASSET_CONTAINER_TGZ) {
        return tgz_extract_entry_to_path(container, entry, outFilePath);
    }
    if (kind == ASSET_CONTAINER_GZIP) {
        return gzip_extract_entry_to_path(container, entry, outFilePath);
    }
    if (kind == ASSET_CONTAINER_LHA) {
        if (lha_extract_entry_to_path(container, entry, outFilePath)) {
            return 1;
        }
        return external_extract_entry_to_path(container, entry, outFilePath);
    }
    if (kind == ASSET_CONTAINER_CHD) {
        return chd_extract_entry_to_path(container, entry, outFilePath);
    }
    if (kind == ASSET_CONTAINER_ADF) {
        return adf_extract_entry_to_path(container, entry, outFilePath);
    }
    if (kind == ASSET_CONTAINER_ATARI_ST) {
        return atari_st_extract_entry_to_path(container, entry, outFilePath);
    }
    if (kind == ASSET_CONTAINER_ATARI_MSA) {
        return atari_msa_extract_entry_to_path(container, entry, outFilePath);
    }
    if (kind == ASSET_CONTAINER_EXTERNAL) {
        const char *nested = strstr(entry, "::");
        if (nested) {
            char adf_entry[ASSET_PATH_MAX];
            size_t adf_len = (size_t)(nested - entry);
            if (adf_len == 0U || adf_len >= sizeof(adf_entry) || nested[2] == '\0') {
                return 0;
            }
            memcpy(adf_entry, entry, adf_len);
            adf_entry[adf_len] = '\0';
            if (is_atari_st_path(adf_entry)) {
                return external_atari_st_extract_entry_to_path(
                    container, adf_entry, nested + 2, outFilePath);
            }
            if (is_atari_msa_path(adf_entry)) {
                return external_atari_msa_extract_entry_to_path(
                    container, adf_entry, nested + 2, outFilePath);
            }
            return external_adf_extract_entry_to_path(container, adf_entry,
                                                      nested + 2, outFilePath);
        }
        if (!external_tool_available_for_path(container)) {
            record_missing_extractor(container);
            return 0;
        }
        return external_extract_entry_to_path(container, entry, outFilePath);
    }
    /* CUE sheets may reference a data image with no .iso/.bin suffix.
     * Virtual paths are produced only after the scanner has already walked
     * the referenced payload as an ISO-like image, so extraction must not
     * reapply the filename-suffix gate here. */
    return iso_extract_entry_to_path(container, entry, outFilePath);
}
