#include "data_validator_m12.h"
#include "fs_portable_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ── Embedded MD5 (same implementation as asset_status_m12.c) ──────── */

typedef struct {
    uint32_t state[4];
    uint64_t bitCount;
    unsigned char buffer[64];
    size_t bufferSize;
} M12_ValidatorMd5Ctx;

static void dv_md5_init(M12_ValidatorMd5Ctx* ctx) {
    if (!ctx) return;
    ctx->state[0] = 0x67452301U;
    ctx->state[1] = 0xefcdab89U;
    ctx->state[2] = 0x98badcfeU;
    ctx->state[3] = 0x10325476U;
    ctx->bitCount = 0U;
    ctx->bufferSize = 0U;
}

static uint32_t dv_md5_rotate_left(uint32_t v, uint32_t s) {
    return (v << s) | (v >> (32U - s));
}

static void dv_md5_transform(uint32_t state[4], const unsigned char block[64]) {
    static const uint32_t s[] = {
        7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
        5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
        4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
        6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
    };
    static const uint32_t k[] = {
        0xd76aa478U,0xe8c7b756U,0x242070dbU,0xc1bdceeeU,
        0xf57c0fafU,0x4787c62aU,0xa8304613U,0xfd469501U,
        0x698098d8U,0x8b44f7afU,0xffff5bb1U,0x895cd7beU,
        0x6b901122U,0xfd987193U,0xa679438eU,0x49b40821U,
        0xf61e2562U,0xc040b340U,0x265e5a51U,0xe9b6c7aaU,
        0xd62f105dU,0x02441453U,0xd8a1e681U,0xe7d3fbc8U,
        0x21e1cde6U,0xc33707d6U,0xf4d50d87U,0x455a14edU,
        0xa9e3e905U,0xfcefa3f8U,0x676f02d9U,0x8d2a4c8aU,
        0xfffa3942U,0x8771f681U,0x6d9d6122U,0xfde5380cU,
        0xa4beea44U,0x4bdecfa9U,0xf6bb4b60U,0xbebfbc70U,
        0x289b7ec6U,0xeaa127faU,0xd4ef3085U,0x04881d05U,
        0xd9d4d039U,0xe6db99e5U,0x1fa27cf8U,0xc4ac5665U,
        0xf4292244U,0x432aff97U,0xab9423a7U,0xfc93a039U,
        0x655b59c3U,0x8f0ccc92U,0xffeff47dU,0x85845dd1U,
        0x6fa87e4fU,0xfe2ce6e0U,0xa3014314U,0x4e0811a1U,
        0xf7537e82U,0xbd3af235U,0x2ad7d2bbU,0xeb86d391U
    };
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t x[16];
    size_t i;
    for (i = 0; i < 16U; ++i) {
        size_t j = i * 4U;
        x[i] = (uint32_t)block[j] |
               ((uint32_t)block[j+1] << 8U) |
               ((uint32_t)block[j+2] << 16U) |
               ((uint32_t)block[j+3] << 24U);
    }
    for (i = 0; i < 64U; ++i) {
        uint32_t f, g, temp = d;
        if (i < 16U) {
            f = (b & c) | (~b & d);
            g = (uint32_t)i;
        } else if (i < 32U) {
            f = (d & b) | (c & ~d);
            g = (5U * (uint32_t)i + 1U) % 16U;
        } else if (i < 48U) {
            f = b ^ c ^ d;
            g = (3U * (uint32_t)i + 5U) % 16U;
        } else {
            f = c ^ (b | ~d);
            g = (7U * (uint32_t)i) % 16U;
        }
        d = c; c = b;
        b = b + dv_md5_rotate_left(a + f + k[i] + x[g], s[i]);
        a = temp;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
}

static void dv_md5_update(M12_ValidatorMd5Ctx* ctx, const unsigned char* data, size_t size) {
    size_t i;
    if (!ctx || (!data && size != 0U)) return;
    ctx->bitCount += (uint64_t)size * 8U;
    for (i = 0; i < size; ++i) {
        ctx->buffer[ctx->bufferSize++] = data[i];
        if (ctx->bufferSize == sizeof(ctx->buffer)) {
            dv_md5_transform(ctx->state, ctx->buffer);
            ctx->bufferSize = 0U;
        }
    }
}

static void dv_md5_final(M12_ValidatorMd5Ctx* ctx, char outHex[33]) {
    static const unsigned char padding[64] = {0x80U};
    unsigned char length[8];
    unsigned char digest[16];
    size_t i;
    if (!ctx || !outHex) return;
    for (i = 0; i < 8U; ++i)
        length[i] = (unsigned char)((ctx->bitCount >> (8U * i)) & 0xffU);
    dv_md5_update(ctx, padding,
                  (ctx->bufferSize < 56U) ? (56U - ctx->bufferSize) : (120U - ctx->bufferSize));
    dv_md5_update(ctx, length, sizeof(length));
    for (i = 0; i < 4U; ++i) {
        digest[i*4]     = (unsigned char)(ctx->state[i] & 0xffU);
        digest[i*4 + 1] = (unsigned char)((ctx->state[i] >> 8U) & 0xffU);
        digest[i*4 + 2] = (unsigned char)((ctx->state[i] >> 16U) & 0xffU);
        digest[i*4 + 3] = (unsigned char)((ctx->state[i] >> 24U) & 0xffU);
    }
    for (i = 0; i < 16U; ++i) {
        static const char hex[] = "0123456789abcdef";
        outHex[i*2]     = hex[(digest[i] >> 4U) & 0x0fU];
        outHex[i*2 + 1] = hex[digest[i] & 0x0fU];
    }
    outHex[32] = '\0';
}

static int dv_file_md5_hex(const char* path, char outHex[33]) {
    unsigned char buffer[4096];
    size_t bytesRead;
    FILE* fp;
    M12_ValidatorMd5Ctx ctx;
    if (!path || path[0] == '\0' || !outHex) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    dv_md5_init(&ctx);
    while ((bytesRead = fread(buffer, 1U, sizeof(buffer), fp)) > 0U)
        dv_md5_update(&ctx, buffer, bytesRead);
    if (ferror(fp)) { fclose(fp); return 0; }
    fclose(fp);
    dv_md5_final(&ctx, outHex);
    return 1;
}

/* ── Known checksum database (from asset_validator_checksums_m12.json) ── */

typedef struct {
    const char* gameId;
    const char* filename;
    const char* md5;
    const char* label;
} M12_KnownChecksum;

static const M12_KnownChecksum g_knownChecksums[] = {
    {"csb", "GRAPHICS.DAT", "61fbfd56887c94adc26888a9491c6611", "CSB Amiga 3.1 and 3.3 Multilanguage GRAPHICS.DAT"},
    {"csb", "GRAPHICS.DAT", "291e1bc6803e3dc4b974c60117ca5d68", "CSB Amiga 3.5 English GRAPHICS.DAT"},
    {"csb", "GRAPHICS.DAT", "cefaddfdf5651df2c91f61b5611a8362", "CSB Amiga 3.5 Multilanguage GRAPHICS.DAT"},
    {"csb", "DRAGON.AMG", "bd85386535697df62bdbae73740fe435", "CSB Amiga Utility Disk DRAGON.AMG"},
    {"csb", "NAKED.AMG", "4d14d2f12752653bcb1d4464840a7230", "CSB Amiga Utility Disk English NAKED.AMG"},
    {"csb", "HCSB.DAT", "708e113c869ab922633e885aa72a3c77", "CSB Amiga Utility Disk English Release 1 Release X and Atari ST 2.0 EN HCSB.DAT"},
    {"csb", "HCSB.DAT", "7496b3b8b9ff6e2368eac9a16be8230b", "CSB Amiga Utility Disk English Release 2/3 HCSB.DAT"},
    {"csb", "CEDTLS.DAT", "8aef8165975a36a426aa2ea39823c149", "CSB Amiga Utility Disk English Release 2/3 CEDTLS.DAT"},
    {"csb", "EXPLOS1.AMG", "370a3c46aecee04bdc6eae73771208c0", "CSB Amiga Utility Disk EXPLOS1.AMG"},
    {"csb", "NAKED.AMG", "dc22b3dae4c1c799e7e21f66f291aa9c", "CSB Amiga Utility Disk French and German NAKED.AMG"},
    {"csb", "CEDTLS.DAT", "b367d58374a799de88bc1a24c6320771", "CSB Amiga Utility Disk French CEDTLS.DAT"},
    {"csb", "HCSB.DAT", "bbf3ada2da9722577feea4fa213b32f1", "CSB Amiga Utility Disk French Release HCSB.DAT"},
    {"csb", "CEDTLS.DAT", "a3cff52ef8e4d85853282181219cde63", "CSB Amiga Utility Disk German CEDTLS.DAT"},
    {"csb", "HCSB.DAT", "9e0da6c5a569859c6191201dcc6e6aae", "CSB Amiga Utility Disk German Release HCSB.DAT"},
    {"csb", "MAGEXPLO.AMG", "59ce0ca034656595b0e0a7a9b1a71316", "CSB Amiga Utility Disk MAGEXPLO.AMG"},
    {"csb", "SWIPE.AMG", "cca41290e6121fbb451ee55283ce4abb", "CSB Amiga Utility Disk SWIPE.AMG"},
    {"csb", "TELE2.AMG", "70528f7143bd0a3ca7bfd3988722a8a8", "CSB Amiga Utility Disk TELE2.AMG"},
    {"csb", "GRAPHICS.DAT", "21197b1d4994fd835c403d5a33dcac2b", "CSB Amiga X.X and 3.1 English GRAPHICS.DAT"},
    {"csb", "GRAPHICS.DAT", "ebf6a57af3f27782e358c0490bfd2f2e", "CSB Atari ST 2.0 and 2.1 English GRAPHICS.DAT"},
    {"csb", "ANIMATE.DAT", "9f8feb269c959c9fe722ac08f99d9c35", "CSB Atari ST Utility Disk English ANIMATE.DAT"},
    {"csb", "GRAPHICS.DAT", "405b757038eea3c263e60f240854d6de", "CSB FM-Towns English GRAPHICS.DAT"},
    {"csb", "GRAPHICS.DAT", "761d6fc588b31aeaaa9caf3725e111b9", "CSB FM-Towns Japanese GRAPHICS.DAT"},
    {"csb", "ENTER.SNG", "61f51d7ffbe0a8ccd6a49c2fec3295fc", "CSB PC-98 ENTER.SNG"},
    {"csb", "GRAPHICS.DAT", "761d6fc588b31aeaaa9caf3725e111b9", "CSB PC-98 Japanese GRAPHICS.DAT"},
    {"csb", "ENTER.SNG", "ee1ec8a63e0d41d45d2e073bfcdf5df7", "CSB X68000 Japanese ENTER.SNG"},
    {"csb", "GRAPHICS.DAT", "8fe59d4f2af5b57a4cb14447c011d3f1", "CSB X68000 Japanese GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "0679e39da9dcc2e855cb33c6c64ddcb5", "DM Amiga 2.0 and 2.2 German GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "6a2f135b53c2220f0251fa103e2a6e7e", "DM Amiga 2.0 English GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "dd373954b3fb127db7387946131ea322", "DM Amiga 2.0 French GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "b35931b55db649a1bd2d415b61b29801", "DM Amiga 2.1 and 2.2 English GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "7f9458e4a3972d06e649a6fa85a7f34b", "DM Amiga 3.6 Multilanguage GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "491ca939f9abb33ceeb26619b841fe91", "DM Amiga Demo English GRAPHICS.DAT"},
    {"dm1", "DEMOIIGS.DAT", "12edef8658079697aae1c2dcb16d5f67", "DM Apple IIGS English DEMOIIGS.DAT"},
    {"dm1", "GRAPHICS.GAME", "1045a36952e64eb2bc7b4c7b2965b112", "DM Apple IIGS English GRAPHICS.GAME"},
    {"dm1", "GRAPHICS.DAT", "b3cfd84e44cdf07ce2eeba47e87f772b", "DM Atari ST 1.0 (1987-12-08) English GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "7eee396993745e8af212f44d75ff6c1a", "DM Atari ST 1.0 (1987-12-11) English GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "5095a13692702235d2e74f6b2b1367a9", "DM Atari ST 1.1 English GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "9ce2eaf7a9e78620e3f17594437caffa", "DM Atari ST 1.2 English GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "2bdc5f431f84c0ece738f54dbd787c3b", "DM Atari ST 1.2 German GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "0d7af44dd14f383464288abdcec76afc", "DM Atari ST 1.3 French GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "c10c512f63461ebe79b5ac365115b61b", "DM FM-Towns English GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "edf47d7da5de8184604d6d80477ef01f", "DM FM-Towns Japanese GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "fa6b1aa29e191418713bf2cda93d962e", "DM PC 3.4 English GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "f934d97e43e1ba6e5159839acbcd0611", "DM PC 3.4 Multilanguage GRAPHICS.DAT"},
    {"dm1", "SONG.DAT", "c20e5b8f756e360a631595cc9260f62d", "DM PC SONG.DAT"},
    {"dm1", "GRAPHICS.DAT", "eaec2131541573658da99c13865c2e67", "DM PC-98 2.0a Japanese GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "3e3b9b1800c67b4ce850e087813c325d", "DM PC-98 2.0b Japanese GRAPHICS.DAT"},
    {"dm1", "GRAPHICS.DAT", "fe08a97c64766614b71164fa06eda545", "DM X68000 Japanese GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "1c940ea95703eaea0ecdf84d17e954b9", "DMII Amiga GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "027ff3b8ddc2c4c4cdda7ada0b0bc46c", "DMII FM-Towns Japanese GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "f926a7554bdfb5852105179e67b8a264", "DMII IBM PSV Japanese GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "4bf28b3d84e6799d7686c6aaf96cbf23", "DMII Macintosh English Demo GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "5cab25f6b975957eae4a203174e7f2a6", "DMII Macintosh English GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "283d5456c4f676609489e200219605bb", "DMII Macintosh Japanese GRAPHICS.DAT"},
    {"dm2", "DMCOORD.DAT", "5dc5d15ae4a3ee85757b3d3622ed2221", "DMII PC 0.9 Beta English DMCOORD.DAT"},
    {"dm2", "GRAPHICS.DAT", "0a63e22cd83fe3c90aacffda5c0f062c", "DMII PC 0.9 Beta English GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "bd2d316eb77c6d6d217bfb76bd0d7e41", "DMII PC English Demo 19950112 GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "43cf7e8579e83e9f1fa9b411695842fd", "DMII PC English Demo 19950509 GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "9a6aa706ae9bed5ddd68ffd730524476", "DMII PC English Demo 19950713 GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "25247ede4dabb6a71e5dabdfbcd5907d", "DMII PC English GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "b4d733576ea60c41737f79f212faf528", "DMII PC French GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "e52ab5e01715042b16a4dcff02052e5d", "DMII PC German and English JewelCase GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "a0277195099b2ace51d4e085f7eef835", "DMII PC-9801 Japanese Demo GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "a669adf2a6ff887e0d451d93c846f57f", "DMII PC-9801 Japanese GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "a31023db49d5d85e469c9323671812c7", "DMII PC-9821 Japanese Release 1 GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "a80c555a858ef7770e1d7f3d2e37fec3", "DMII PC-9821 Japanese Release 2 GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "dbced13a38d3036f42b9797175b7ec88", "DMII Sega CD English GRAPHICS.DAT"},
    {"dm2", "GRAPHICS.DAT", "a654ba19e9a6919f46818ecd23d7ea9d", "DMII Sega CD Japanese GRAPHICS.DAT"}
};

static const size_t g_knownChecksumCount =
    sizeof(g_knownChecksums) / sizeof(g_knownChecksums[0]);

/* ── Helper ────────────────────────────────────────────────────────── */

static void dv_copy_string(char* out, size_t cap, const char* src) {
    size_t len;
    if (!out || cap == 0U) return;
    if (!src) { out[0] = '\0'; return; }
    len = strlen(src);
    if (len >= cap) len = cap - 1U;
    memcpy(out, src, len);
    out[len] = '\0';
}

/* ── Public API ────────────────────────────────────────────────────── */

void M12_DataValidator_Init(M12_DataValidatorState* state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

void M12_DataValidator_Scan(M12_DataValidatorState* state, const char* dataDir) {
    size_t i;
    char path[M12_VALIDATOR_PATH_CAPACITY];
    size_t dirLen;

    if (!state) return;
    M12_DataValidator_Init(state);

    if (!dataDir || dataDir[0] == '\0') return;

    dirLen = strlen(dataDir);

    /* Collect unique filenames from the database and hash each one
     * found on disk. Then match against all known checksums. */
    for (i = 0; i < g_knownChecksumCount && state->resultCount < M12_VALIDATOR_MAX_FILES; ++i) {
        const M12_KnownChecksum* ck = &g_knownChecksums[i];
        char md5Hex[M12_VALIDATOR_MD5_CAPACITY];
        size_t fnLen = strlen(ck->filename);
        M12_FileValidationResult* r;

        /* Build path: dataDir / filename */
        if (dirLen + 1 + fnLen >= sizeof(path)) continue;
        memcpy(path, dataDir, dirLen);
        path[dirLen] = '/';
        memcpy(path + dirLen + 1, ck->filename, fnLen);
        path[dirLen + 1 + fnLen] = '\0';

        if (!dv_file_md5_hex(path, md5Hex)) {
            /* File not present or unreadable — only record MISSING once
             * per unique (gameId, filename) to avoid flooding results. */
            size_t j;
            int already = 0;
            for (j = 0; j < state->resultCount; ++j) {
                if (strcmp(state->results[j].filename, ck->filename) == 0 &&
                    strcmp(state->results[j].gameId, ck->gameId) == 0 &&
                    state->results[j].status == M12_FILE_STATUS_MISSING) {
                    already = 1;
                    break;
                }
            }
            if (already) continue;

            r = &state->results[state->resultCount++];
            dv_copy_string(r->filename, sizeof(r->filename), ck->filename);
            dv_copy_string(r->gameId, sizeof(r->gameId), ck->gameId);
            dv_copy_string(r->expectedMd5, sizeof(r->expectedMd5), ck->md5);
            r->actualMd5[0] = '\0';
            dv_copy_string(r->label, sizeof(r->label), ck->label);
            r->status = M12_FILE_STATUS_MISSING;
            state->missingCount++;
            continue;
        }

        /* File exists — check if this MD5 matches */
        if (strcmp(md5Hex, ck->md5) == 0) {
            /* Check if we already recorded a VALID match for this exact file */
            size_t j;
            int already = 0;
            for (j = 0; j < state->resultCount; ++j) {
                if (strcmp(state->results[j].filename, ck->filename) == 0 &&
                    strcmp(state->results[j].gameId, ck->gameId) == 0 &&
                    state->results[j].status == M12_FILE_STATUS_VALID) {
                    already = 1;
                    break;
                }
            }
            if (already) continue;

            r = &state->results[state->resultCount++];
            dv_copy_string(r->filename, sizeof(r->filename), ck->filename);
            dv_copy_string(r->gameId, sizeof(r->gameId), ck->gameId);
            dv_copy_string(r->expectedMd5, sizeof(r->expectedMd5), ck->md5);
            dv_copy_string(r->actualMd5, sizeof(r->actualMd5), md5Hex);
            dv_copy_string(r->label, sizeof(r->label), ck->label);
            r->status = M12_FILE_STATUS_VALID;
            state->validCount++;
        }
        /* For files that exist but don't match this particular entry,
         * we don't record a mismatch yet — another entry might match.
         * We do a second pass below. */
    }

    /* Second pass: for files on disk whose MD5 didn't match ANY known
     * checksum entry, record one UNKNOWN result per unique filename
     * found on disk (unless already VALID). */
    {
        /* Collect unique filenames from the DB */
        const char* seenFiles[M12_VALIDATOR_MAX_FILES];
        const char* seenGames[M12_VALIDATOR_MAX_FILES];
        size_t seenCount = 0;

        for (i = 0; i < g_knownChecksumCount && seenCount < M12_VALIDATOR_MAX_FILES; ++i) {
            const M12_KnownChecksum* ck = &g_knownChecksums[i];
            size_t j;
            int dup = 0;
            for (j = 0; j < seenCount; ++j) {
                if (strcmp(seenFiles[j], ck->filename) == 0 &&
                    strcmp(seenGames[j], ck->gameId) == 0) {
                    dup = 1;
                    break;
                }
            }
            if (!dup) {
                seenFiles[seenCount] = ck->filename;
                seenGames[seenCount] = ck->gameId;
                seenCount++;
            }
        }

        for (i = 0; i < seenCount && state->resultCount < M12_VALIDATOR_MAX_FILES; ++i) {
            char md5Hex[M12_VALIDATOR_MD5_CAPACITY];
            size_t fnLen = strlen(seenFiles[i]);
            size_t j;
            int hasResult = 0;

            /* Check if we already have a result for this file */
            for (j = 0; j < state->resultCount; ++j) {
                if (strcmp(state->results[j].filename, seenFiles[i]) == 0 &&
                    strcmp(state->results[j].gameId, seenGames[i]) == 0) {
                    hasResult = 1;
                    break;
                }
            }
            if (hasResult) continue;

            /* File exists on disk but didn't match any known checksum */
            if (dirLen + 1 + fnLen >= sizeof(path)) continue;
            memcpy(path, dataDir, dirLen);
            path[dirLen] = '/';
            memcpy(path + dirLen + 1, seenFiles[i], fnLen);
            path[dirLen + 1 + fnLen] = '\0';

            if (dv_file_md5_hex(path, md5Hex)) {
                M12_FileValidationResult* r = &state->results[state->resultCount++];
                dv_copy_string(r->filename, sizeof(r->filename), seenFiles[i]);
                dv_copy_string(r->gameId, sizeof(r->gameId), seenGames[i]);
                r->expectedMd5[0] = '\0';
                dv_copy_string(r->actualMd5, sizeof(r->actualMd5), md5Hex);
                dv_copy_string(r->label, sizeof(r->label), "Unknown version");
                r->status = M12_FILE_STATUS_UNKNOWN;
            }
        }
    }

    state->scanned = 1;
}

const char* M12_DataValidator_Summary(const M12_DataValidatorState* state) {
    static char buf[256];
    if (!state || !state->scanned) {
        return "Not scanned";
    }
    snprintf(buf, sizeof(buf),
             "%zu file(s) checked: %zu valid, %zu missing, %zu mismatch, %zu error",
             state->resultCount, state->validCount,
             state->missingCount, state->mismatchCount, state->errorCount);
    return buf;
}

const char* M12_DataValidator_StatusLabel(M12_FileValidationStatus status) {
    switch (status) {
    case M12_FILE_STATUS_VALID:    return "VALID";
    case M12_FILE_STATUS_MISMATCH: return "MISMATCH";
    case M12_FILE_STATUS_MISSING:  return "MISSING";
    case M12_FILE_STATUS_ERROR:    return "ERROR";
    case M12_FILE_STATUS_UNKNOWN:  return "UNKNOWN";
    default:                       return "?";
    }
}
