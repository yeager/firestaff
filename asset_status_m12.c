#include "asset_status_m12.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    uint32_t state[4];
    uint64_t bitCount;
    unsigned char buffer[64];
    size_t bufferSize;
} M12_Md5Context;

typedef struct {
    const char* gameId;
    const char* label;
    const char* const* names;
    const char* const* md5List;
} M12_AssetFileSpec;

typedef struct {
    const char* gameId;
    const M12_AssetFileSpec* files;
    size_t fileCount;
} M12_GameSpec;

static const char* const g_dm1GraphicsNames[] = {"GRAPHICS.DAT", NULL};
static const char* const g_dm1DungeonNames[] = {"DUNGEON.DAT", NULL};
static const char* const g_csbGraphicsNames[] = {"CSBGRAPH.DAT", NULL};
static const char* const g_csbDungeonNames[] = {"CSB.DAT", "CSB_DUNGEON.DAT", NULL};
static const char* const g_dm2GraphicsNames[] = {"DM2GRAPHICS.DAT", "SKULLKEEP.GFX", NULL};
static const char* const g_dm2DungeonNames[] = {"DM2DUNGEON.DAT", "SKULLKEEP.DAT", NULL};

static const char* const g_dm1GraphicsHashes[] = {
    "fa6b1aa29e191418713bf2cda93d962e",
    "444bcb3a3fcf8389296c49467f27e1d6",
    NULL
};
static const char* const g_dm1DungeonHashes[] = {
    "766450c940651fc021c92fe5d0d0b3a6",
    "444bcb3a3fcf8389296c49467f27e1d6",
    NULL
};

/*
 * CSB and DM2 intentionally stay empty until the project captures hashes from
 * verified retail data. The validator structure below keeps those insertion
 * points explicit so future additions only touch the relevant hash lists.
 */
static const char* const g_csbGraphicsHashes[] = {NULL};
static const char* const g_csbDungeonHashes[] = {NULL};
static const char* const g_dm2GraphicsHashes[] = {NULL};
static const char* const g_dm2DungeonHashes[] = {NULL};

static const M12_AssetFileSpec g_dm1Files[] = {
    {"dm1", "graphics", g_dm1GraphicsNames, g_dm1GraphicsHashes},
    {"dm1", "dungeon", g_dm1DungeonNames, g_dm1DungeonHashes}
};
static const M12_AssetFileSpec g_csbFiles[] = {
    {"csb", "graphics", g_csbGraphicsNames, g_csbGraphicsHashes},
    {"csb", "dungeon", g_csbDungeonNames, g_csbDungeonHashes}
};
static const M12_AssetFileSpec g_dm2Files[] = {
    {"dm2", "graphics", g_dm2GraphicsNames, g_dm2GraphicsHashes},
    {"dm2", "dungeon", g_dm2DungeonNames, g_dm2DungeonHashes}
};

static const M12_GameSpec g_gameSpecs[] = {
    {"dm1", g_dm1Files, sizeof(g_dm1Files) / sizeof(g_dm1Files[0])},
    {"csb", g_csbFiles, sizeof(g_csbFiles) / sizeof(g_csbFiles[0])},
    {"dm2", g_dm2Files, sizeof(g_dm2Files) / sizeof(g_dm2Files[0])}
};

static void m12_md5_init(M12_Md5Context* ctx) {
    if (!ctx) {
        return;
    }
    ctx->state[0] = 0x67452301U;
    ctx->state[1] = 0xefcdab89U;
    ctx->state[2] = 0x98badcfeU;
    ctx->state[3] = 0x10325476U;
    ctx->bitCount = 0U;
    ctx->bufferSize = 0U;
}

static uint32_t m12_md5_rotate_left(uint32_t value, uint32_t shift) {
    return (value << shift) | (value >> (32U - shift));
}

static uint32_t m12_md5_f(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) | (~x & z);
}

static uint32_t m12_md5_g(uint32_t x, uint32_t y, uint32_t z) {
    return (x & z) | (y & ~z);
}

static uint32_t m12_md5_h(uint32_t x, uint32_t y, uint32_t z) {
    return x ^ y ^ z;
}

static uint32_t m12_md5_i(uint32_t x, uint32_t y, uint32_t z) {
    return y ^ (x | ~z);
}

static void m12_md5_transform(uint32_t state[4], const unsigned char block[64]) {
    static const uint32_t s[] = {
        7U, 12U, 17U, 22U, 7U, 12U, 17U, 22U, 7U, 12U, 17U, 22U, 7U, 12U, 17U, 22U,
        5U, 9U, 14U, 20U, 5U, 9U, 14U, 20U, 5U, 9U, 14U, 20U, 5U, 9U, 14U, 20U,
        4U, 11U, 16U, 23U, 4U, 11U, 16U, 23U, 4U, 11U, 16U, 23U, 4U, 11U, 16U, 23U,
        6U, 10U, 15U, 21U, 6U, 10U, 15U, 21U, 6U, 10U, 15U, 21U, 6U, 10U, 15U, 21U
    };
    static const uint32_t k[] = {
        0xd76aa478U, 0xe8c7b756U, 0x242070dbU, 0xc1bdceeeU,
        0xf57c0fafU, 0x4787c62aU, 0xa8304613U, 0xfd469501U,
        0x698098d8U, 0x8b44f7afU, 0xffff5bb1U, 0x895cd7beU,
        0x6b901122U, 0xfd987193U, 0xa679438eU, 0x49b40821U,
        0xf61e2562U, 0xc040b340U, 0x265e5a51U, 0xe9b6c7aaU,
        0xd62f105dU, 0x02441453U, 0xd8a1e681U, 0xe7d3fbc8U,
        0x21e1cde6U, 0xc33707d6U, 0xf4d50d87U, 0x455a14edU,
        0xa9e3e905U, 0xfcefa3f8U, 0x676f02d9U, 0x8d2a4c8aU,
        0xfffa3942U, 0x8771f681U, 0x6d9d6122U, 0xfde5380cU,
        0xa4beea44U, 0x4bdecfa9U, 0xf6bb4b60U, 0xbebfbc70U,
        0x289b7ec6U, 0xeaa127faU, 0xd4ef3085U, 0x04881d05U,
        0xd9d4d039U, 0xe6db99e5U, 0x1fa27cf8U, 0xc4ac5665U,
        0xf4292244U, 0x432aff97U, 0xab9423a7U, 0xfc93a039U,
        0x655b59c3U, 0x8f0ccc92U, 0xffeff47dU, 0x85845dd1U,
        0x6fa87e4fU, 0xfe2ce6e0U, 0xa3014314U, 0x4e0811a1U,
        0xf7537e82U, 0xbd3af235U, 0x2ad7d2bbU, 0xeb86d391U
    };
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t x[16];
    size_t i;

    for (i = 0; i < 16U; ++i) {
        size_t j = i * 4U;
        x[i] = (uint32_t)block[j] |
               ((uint32_t)block[j + 1U] << 8U) |
               ((uint32_t)block[j + 2U] << 16U) |
               ((uint32_t)block[j + 3U] << 24U);
    }

    for (i = 0; i < 64U; ++i) {
        uint32_t f;
        uint32_t g;
        uint32_t temp = d;
        if (i < 16U) {
            f = m12_md5_f(b, c, d);
            g = (uint32_t)i;
        } else if (i < 32U) {
            f = m12_md5_g(b, c, d);
            g = (5U * (uint32_t)i + 1U) % 16U;
        } else if (i < 48U) {
            f = m12_md5_h(b, c, d);
            g = (3U * (uint32_t)i + 5U) % 16U;
        } else {
            f = m12_md5_i(b, c, d);
            g = (7U * (uint32_t)i) % 16U;
        }
        d = c;
        c = b;
        b = b + m12_md5_rotate_left(a + f + k[i] + x[g], s[i]);
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

static void m12_md5_update(M12_Md5Context* ctx, const unsigned char* data, size_t size) {
    size_t i;
    if (!ctx || (!data && size != 0U)) {
        return;
    }
    ctx->bitCount += (uint64_t)size * 8U;
    for (i = 0; i < size; ++i) {
        ctx->buffer[ctx->bufferSize++] = data[i];
        if (ctx->bufferSize == sizeof(ctx->buffer)) {
            m12_md5_transform(ctx->state, ctx->buffer);
            ctx->bufferSize = 0U;
        }
    }
}

static void m12_md5_final(M12_Md5Context* ctx, char outHex[33]) {
    static const unsigned char padding[64] = {0x80U};
    unsigned char length[8];
    unsigned char digest[16];
    size_t i;
    if (!ctx || !outHex) {
        return;
    }
    for (i = 0; i < 8U; ++i) {
        length[i] = (unsigned char)((ctx->bitCount >> (8U * i)) & 0xffU);
    }
    m12_md5_update(ctx, padding, (ctx->bufferSize < 56U) ? (56U - ctx->bufferSize) : (120U - ctx->bufferSize));
    m12_md5_update(ctx, length, sizeof(length));
    for (i = 0; i < 4U; ++i) {
        digest[i * 4U] = (unsigned char)(ctx->state[i] & 0xffU);
        digest[i * 4U + 1U] = (unsigned char)((ctx->state[i] >> 8U) & 0xffU);
        digest[i * 4U + 2U] = (unsigned char)((ctx->state[i] >> 16U) & 0xffU);
        digest[i * 4U + 3U] = (unsigned char)((ctx->state[i] >> 24U) & 0xffU);
    }
    for (i = 0; i < 16U; ++i) {
        static const char hex[] = "0123456789abcdef";
        outHex[i * 2U] = hex[(digest[i] >> 4U) & 0x0fU];
        outHex[i * 2U + 1U] = hex[digest[i] & 0x0fU];
    }
    outHex[32] = '\0';
}

static int m12_join_path(char* out,
                         size_t outSize,
                         const char* dir,
                         const char* leaf) {
    int rc;
    if (!out || outSize == 0U || !dir || !leaf) {
        return 0;
    }
    rc = snprintf(out,
                  outSize,
                  "%s%s%s",
                  dir,
                  (dir[0] != '\0' && dir[strlen(dir) - 1] == '/') ? "" : "/",
                  leaf);
    return rc > 0 && (size_t)rc < outSize;
}

static void m12_copy_data_dir(char* out,
                              size_t outSize,
                              const char* requestedDataDir) {
    const char* envDataDir = getenv("FIRESTAFF_DATA");
    const char* resolved = requestedDataDir;
    if (!out || outSize == 0U) {
        return;
    }
    if (!resolved || resolved[0] == '\0') {
        resolved = envDataDir;
    }
    if (!resolved || resolved[0] == '\0') {
        resolved = ".";
    }
    snprintf(out, outSize, "%s", resolved);
}

static int m12_file_md5_hex(const char* path, char outHex[33]) {
    unsigned char buffer[4096];
    size_t bytesRead;
    FILE* fp;
    M12_Md5Context ctx;
    if (!path || path[0] == '\0' || !outHex) {
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    m12_md5_init(&ctx);
    while ((bytesRead = fread(buffer, 1U, sizeof(buffer), fp)) > 0U) {
        m12_md5_update(&ctx, buffer, bytesRead);
    }
    if (ferror(fp)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    m12_md5_final(&ctx, outHex);
    return 1;
}

static int m12_hash_list_contains(const char* const* hashes, const char* md5Hex) {
    size_t i;
    if (!hashes || !md5Hex) {
        return 0;
    }
    for (i = 0U; hashes[i] != NULL; ++i) {
        if (strcmp(hashes[i], md5Hex) == 0) {
            return 1;
        }
    }
    return 0;
}

static size_t m12_hash_list_count(const char* const* hashes) {
    size_t i;
    if (!hashes) {
        return 0U;
    }
    for (i = 0U; hashes[i] != NULL; ++i) {
    }
    return i;
}

static const M12_GameSpec* m12_find_game_spec(const char* gameId) {
    size_t i;
    if (!gameId) {
        return NULL;
    }
    for (i = 0U; i < sizeof(g_gameSpecs) / sizeof(g_gameSpecs[0]); ++i) {
        if (strcmp(g_gameSpecs[i].gameId, gameId) == 0) {
            return &g_gameSpecs[i];
        }
    }
    return NULL;
}

static int m12_game_has_complete_hash_set(const M12_GameSpec* spec) {
    size_t i;
    if (!spec || !spec->files || spec->fileCount == 0U) {
        return 0;
    }
    for (i = 0U; i < spec->fileCount; ++i) {
        if (m12_hash_list_count(spec->files[i].md5List) == 0U) {
            return 0;
        }
    }
    return 1;
}

static size_t m12_game_known_hash_count(const M12_GameSpec* spec) {
    size_t i;
    size_t total = 0U;
    if (!spec || !spec->files) {
        return 0U;
    }
    for (i = 0U; i < spec->fileCount; ++i) {
        total += m12_hash_list_count(spec->files[i].md5List);
    }
    return total;
}

static size_t m12_game_verified_file_count(const M12_GameSpec* spec) {
    size_t i;
    size_t total = 0U;
    if (!spec || !spec->files) {
        return 0U;
    }
    for (i = 0U; i < spec->fileCount; ++i) {
        if (m12_hash_list_count(spec->files[i].md5List) > 0U) {
            total += 1U;
        }
    }
    return total;
}

static int m12_match_known_asset(const char* dir, const M12_AssetFileSpec* spec) {
    char path[M12_ASSET_DATA_DIR_CAPACITY + 64];
    char md5Hex[33];
    size_t i;
    if (!dir || !spec || !spec->names || !spec->md5List || spec->md5List[0] == NULL) {
        return 0;
    }
    for (i = 0U; spec->names[i] != NULL; ++i) {
        if (!m12_join_path(path, sizeof(path), dir, spec->names[i])) {
            continue;
        }
        if (!m12_file_md5_hex(path, md5Hex)) {
            continue;
        }
        if (m12_hash_list_contains(spec->md5List, md5Hex)) {
            return 1;
        }
    }
    return 0;
}

static int m12_detect_game(const char* dir, const M12_GameSpec* spec) {
    size_t i;
    if (!dir || !spec || !spec->files || !m12_game_has_complete_hash_set(spec)) {
        return 0;
    }
    for (i = 0U; i < spec->fileCount; ++i) {
        if (!m12_match_known_asset(dir, &spec->files[i])) {
            return 0;
        }
    }
    return 1;
}

void M12_AssetStatus_Scan(M12_AssetStatus* status, const char* requestedDataDir) {
    if (!status) {
        return;
    }

    memset(status, 0, sizeof(*status));
    m12_copy_data_dir(status->dataDir, sizeof(status->dataDir), requestedDataDir);

    status->dm1Available = m12_detect_game(status->dataDir, &g_gameSpecs[0]);
    status->csbAvailable = m12_detect_game(status->dataDir, &g_gameSpecs[1]);
    status->dm2Available = m12_detect_game(status->dataDir, &g_gameSpecs[2]);
}

int M12_AssetStatus_GameAvailable(const M12_AssetStatus* status,
                                  const char* gameId) {
    if (!status || !gameId) {
        return 0;
    }
    if (strcmp(gameId, "dm1") == 0) {
        return status->dm1Available;
    }
    if (strcmp(gameId, "csb") == 0) {
        return status->csbAvailable;
    }
    if (strcmp(gameId, "dm2") == 0) {
        return status->dm2Available;
    }
    return 0;
}

int M12_AssetStatus_GameHasCompleteHashSet(const char* gameId) {
    return m12_game_has_complete_hash_set(m12_find_game_spec(gameId));
}

size_t M12_AssetStatus_GameKnownHashCount(const char* gameId) {
    return m12_game_known_hash_count(m12_find_game_spec(gameId));
}

size_t M12_AssetStatus_GameVerifiedFileCount(const char* gameId) {
    return m12_game_verified_file_count(m12_find_game_spec(gameId));
}

size_t M12_AssetStatus_GameRequiredFileCount(const char* gameId) {
    const M12_GameSpec* spec = m12_find_game_spec(gameId);
    return spec ? spec->fileCount : 0U;
}

const char* M12_AssetStatus_GetDataDir(const M12_AssetStatus* status) {
    if (!status || status->dataDir[0] == '\0') {
        return ".";
    }
    return status->dataDir;
}
