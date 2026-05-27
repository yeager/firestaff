#include "asset_status_m12.h"
#include "fs_portable_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M12_SEARCH_ROOT_COUNT 3

typedef struct {
    uint32_t state[4];
    uint64_t bitCount;
    unsigned char buffer[64];
    size_t bufferSize;
} M12_Md5Context;

typedef struct {
    const char* gameId;
    const char* versionId;
    const char* label;
    const char* shortLabel;
    const char* const* names;
    const char* md5;
} M12_VersionSpec;

typedef struct {
    const char* gameId;
    const M12_VersionSpec* versions;
    size_t versionCount;
} M12_GameVersionSpec;

static const char* const g_dm1GraphicsNames[] = {"GRAPHICS.DAT", NULL};
static const char* const g_csbGraphicsNames[] = {"GRAPHICS.DAT", "CSBGRAPH.DAT", NULL};
static const char* const g_dm2GraphicsNames[] = {"GRAPHICS.DAT", "DM2GRAPHICS.DAT", "SKULLKEEP.GFX", NULL};
static const char* const g_nexusArchiveNames[] = {
    /* Sega Saturn CD image / extracted archive markers */
    "DM.BIN",           /* primary Saturn CD image marker */
    "SEGADATA.BIN",     /* Saturn data track */
    "Dungeon-Master-Nexus_SEGA-Saturn_JA.zip",
    NULL
};

static const M12_VersionSpec g_dm1Versions[] = {
    {"dm1", "pc34-en", "PC 3.4 English", "PC 3.4 EN", g_dm1GraphicsNames, "fa6b1aa29e191418713bf2cda93d962e"},
    {"dm1", "pc34-multi", "PC 3.4 Multilanguage", "PC 3.4 ML", g_dm1GraphicsNames, "f934d97e43e1ba6e5159839acbcd0611"},
    {"dm1", "st12-en", "Atari ST 1.2 English", "ST 1.2 EN", g_dm1GraphicsNames, "9ce2eaf7a9e78620e3f17594437caffa"}
};

static const M12_VersionSpec g_csbVersions[] = {
    {"csb", "pc34-en", "PC 3.4 English", "PC 3.4 EN", g_csbGraphicsNames, "61fbfd56887c94adc26888a9491c6611"},
    {"csb", "st20-21-en", "Atari ST 2.0/2.1 English", "ST 2.1 EN", g_csbGraphicsNames, "ebf6a57af3f27782e358c0490bfd2f2e"},
    {"csb", "amiga35-en", "Amiga 3.5 English", "Amiga 3.5 EN", g_csbGraphicsNames, "291e1bc6803e3dc4b974c60117ca5d68"},
    {"csb", "amiga35-multi", "Amiga 3.5 Multilanguage", "Amiga 3.5 ML", g_csbGraphicsNames, "cefaddfdf5651df2c91f61b5611a8362"}
};

static const M12_VersionSpec g_dm2Versions[] = {
    {"dm2", "pc-en", "PC English", "PC EN", g_dm2GraphicsNames, "25247ede4dabb6a71e5dabdfbcd5907d"},
    {"dm2", "pc-fr", "PC French", "PC FR", g_dm2GraphicsNames, "b4d733576ea60c41737f79f212faf528"},
    {"dm2", "pc-jewel", "PC German/English JewelCase", "PC JewelCase", g_dm2GraphicsNames, "e52ab5e01715042b16a4dcff02052e5d"}
};

static const M12_VersionSpec g_nexusVersions[] = {
    {"nexus", "nexus-saturn-jp", "Nexus Sega Saturn JP (extracted)", "Saturn JP", g_nexusArchiveNames, "e88d60859f65f08fa622e1992b02280f"},
    {"nexus", "nexus", "Nexus original Sega Saturn JP", "nexus", g_nexusArchiveNames, "96e511c8d36ccbe30a48ba36c59df194"},
    {"nexus", "nexus2", "Nexus V2 upscaled graphics", "nexus2", g_nexusArchiveNames, ""}
};

/* Theron's Quest — PC Engine / TurboGrafx-16 (Hudson Soft, 1992).
 * Phase 0 gate PASSED (2026-05-27): CD-ROM Track 02 hashes confirmed from cdromance.org.
 * Data track is Track 02 .bin from CUE/BIN disc images.
 * JP: MD5 b7afb338ad31be1025b53f9aff12d73a (Track 02 .bin)
 * US: MD5 f23601102138f87c33025877767ebf76 (Track 02 .bin)
 * OneDrive: 1drv.ms/f/s!AsBu7boYHQokbYK3rjKY0b5_ra8 (DMFiles/Games folder)
 * Subdir candidates: theron/jp/, theron/us/, theron/ */
static const char* const g_theronTrack02Names[] = {
    "track02.bin",
    "Theron's Quest (Japan) (Track 02).bin",
    "Theron's Quest (US) (Track 02).bin",
    "THQUEST.BIN",
    NULL
};

static const M12_VersionSpec g_theronVersions[] = {
    {"theron", "pce-jp", "PC Engine JP (Track 02)", "PCE JP",
     g_theronTrack02Names, "b7afb338ad31be1025b53f9aff12d73a"},
    {"theron", "pce-en", "TurboGrafx-16 US (Track 02)", "TG16 US",
     g_theronTrack02Names, "f23601102138f87c33025877767ebf76"}
};

static const M12_GameVersionSpec g_games[] = {
    {"dm1", g_dm1Versions, sizeof(g_dm1Versions) / sizeof(g_dm1Versions[0])},
    {"csb", g_csbVersions, sizeof(g_csbVersions) / sizeof(g_csbVersions[0])},
    {"dm2", g_dm2Versions, sizeof(g_dm2Versions) / sizeof(g_dm2Versions[0])},
    {"nexus", g_nexusVersions, sizeof(g_nexusVersions) / sizeof(g_nexusVersions[0])},
    {"theron", g_theronVersions, sizeof(g_theronVersions) / sizeof(g_theronVersions[0])}
};

static const char* const g_assetCandidateSubdirs[] = {
    "dm1", "", "csb", "dm2", "nexus",
    "dm1-multilingual", "theron", "theron/jp", "theron/us", NULL
};

static const char* const g_originalCandidateNames[] = {
    "GRAPHICS.DAT",
    "DUNGEON.DAT",
    "DUNGEONF.DAT",
    "DUNGEONG.DAT",
    "CSBGRAPH.DAT",
    "CSB.DAT",
    "DM2GRAPHICS.DAT",
    "DM2DUNGEON.DAT",
    "SKULLKEEP.GFX",
    "DM.BIN",                  /* Nexus Sega Saturn primary CD image */
    "SEGADATA.BIN",       /* Nexus Sega Saturn data track */
    "Dungeon-Master-Nexus_SEGA-Saturn_JA.zip",
    "track02.bin",
    "Theron's Quest (Japan) (Track 02).bin",
    "Theron's Quest (US) (Track 02).bin",
    "THQUEST.BIN",
    NULL
};

static int m12_game_index_from_id(const char* gameId) {
    size_t i;
    if (!gameId) {
        return -1;
    }
    for (i = 0U; i < sizeof(g_games) / sizeof(g_games[0]); ++i) {
        if (strcmp(g_games[i].gameId, gameId) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static const M12_GameVersionSpec* m12_find_game_spec(const char* gameId) {
    int index = m12_game_index_from_id(gameId);
    return index >= 0 ? &g_games[index] : NULL;
}

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
    m12_md5_update(ctx,
                   padding,
                   (ctx->bufferSize < 56U) ? (56U - ctx->bufferSize) : (120U - ctx->bufferSize));
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

int m12_file_md5_hex(const char* path, char outHex[33]) {
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

static void m12_copy_string(char* out, size_t outSize, const char* value) {
    if (!out || outSize == 0U) {
        return;
    }
    if (!value) {
        value = "";
    }
    snprintf(out, outSize, "%s", value);
}

static int m12_same_path(const char* a, const char* b) {
    return a && b && strcmp(a, b) == 0;
}

static size_t m12_build_search_roots(char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY],
                                     const char* requestedDataDir,
                                     char legacyFallbackDir[M12_ASSET_DATA_DIR_CAPACITY]) {
    char defaultOriginals[M12_ASSET_DATA_DIR_CAPACITY];
    char legacyData[M12_ASSET_DATA_DIR_CAPACITY];
    size_t count = 0U;

    legacyFallbackDir[0] = '\0';
    defaultOriginals[0] = '\0';
    legacyData[0] = '\0';

    if (requestedDataDir && requestedDataDir[0] != '\0') {
        m12_copy_string(roots[count++], M12_ASSET_DATA_DIR_CAPACITY, requestedDataDir);
    }
    if (FSP_GetDefaultOriginalsDir(defaultOriginals, sizeof(defaultOriginals))) {
        if (count == 0U || !m12_same_path(roots[count - 1U], defaultOriginals)) {
            m12_copy_string(roots[count++], M12_ASSET_DATA_DIR_CAPACITY, defaultOriginals);
        }
    }
    if (FSP_ResolveDataDir(legacyData, sizeof(legacyData), NULL)) {
        m12_copy_string(legacyFallbackDir, M12_ASSET_DATA_DIR_CAPACITY, legacyData);
        if ((count == 0U || !m12_same_path(roots[count - 1U], legacyData)) && count < M12_SEARCH_ROOT_COUNT) {
            m12_copy_string(roots[count++], M12_ASSET_DATA_DIR_CAPACITY, legacyData);
        }
    }
    if (count == 0U) {
        m12_copy_string(roots[count++], M12_ASSET_DATA_DIR_CAPACITY, ".");
    }
    return count;
}


static int m12_root_has_original_candidate(const char* root) {
    char path[M12_ASSET_DATA_DIR_CAPACITY + 64];
    size_t i;
    int s;
    if (!root || root[0] == '\0') {
        return 0;
    }
    for (s = 0; g_assetCandidateSubdirs[s] != NULL; ++s) {
        char subroot[M12_ASSET_DATA_DIR_CAPACITY];
        if (g_assetCandidateSubdirs[s][0] == '\0') {
            m12_copy_string(subroot, sizeof(subroot), root);
        } else {
            snprintf(subroot, sizeof(subroot), "%s/%s", root, g_assetCandidateSubdirs[s]);
        }
        for (i = 0U; g_originalCandidateNames[i] != NULL; ++i) {
            FILE* fp;
            if (!FSP_JoinPath(path, sizeof(path), subroot, g_originalCandidateNames[i])) {
                continue;
            }
            fp = fopen(path, "rb");
            if (fp) {
                fclose(fp);
                return 1;
            }
        }
    }
    return 0;
}

static void m12_scan_original_candidates(M12_AssetStatus* status,
                                         const char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY],
                                         size_t rootCount) {
    size_t rootIndex;
    if (!status) {
        return;
    }
    for (rootIndex = 0U; rootIndex < rootCount; ++rootIndex) {
        if (m12_root_has_original_candidate(roots[rootIndex])) {
            status->originalFileCandidateFound = 1;
            return;
        }
    }
}

static int m12_try_match_version(const char* root,
                                 const M12_VersionSpec* spec,
                                 char matchedPath[M12_ASSET_DATA_DIR_CAPACITY],
                                 char matchedMd5[M12_ASSET_MD5_CAPACITY]) {
    char path[M12_ASSET_DATA_DIR_CAPACITY + 64];
    char md5Hex[M12_ASSET_MD5_CAPACITY];
    size_t i;
    int s;
    if (!root || !spec || !spec->names || !spec->md5 || spec->md5[0] == 0) {
        return 0;
    }
    for (s = 0; g_assetCandidateSubdirs[s] != NULL; ++s) {
        char subroot[M12_ASSET_DATA_DIR_CAPACITY];
        if (g_assetCandidateSubdirs[s][0] == 0) {
            m12_copy_string(subroot, sizeof(subroot), root);
        } else {
            snprintf(subroot, sizeof(subroot), "%s/%s", root, g_assetCandidateSubdirs[s]);
        }
        for (i = 0U; spec->names[i] != NULL; ++i) {
            if (!FSP_JoinPath(path, sizeof(path), subroot, spec->names[i])) {
                continue;
            }
            if (!m12_file_md5_hex(path, md5Hex)) {
                continue;
            }
            if (strcmp(md5Hex, spec->md5) == 0) {
                m12_copy_string(matchedPath, M12_ASSET_DATA_DIR_CAPACITY, path);
                m12_copy_string(matchedMd5, M12_ASSET_MD5_CAPACITY, md5Hex);
                return 1;
            }
        }
    }
    return 0;
}

static void m12_fill_game_versions(M12_AssetStatus* status,
                                   int gameIndex,
                                   const char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY],
                                   size_t rootCount,
                                   int* dataDirResolvedToMatchedRoot) {
    size_t i;
    size_t rootIndex;
    int matchedAny = 0;
    const M12_GameVersionSpec* gameSpec;
    if (!status || gameIndex < 0 || gameIndex >= M12_ASSET_GAME_COUNT) {
        return;
    }
    gameSpec = &g_games[gameIndex];
    for (i = 0U; i < gameSpec->versionCount; ++i) {
        M12_AssetVersionStatus* version = &status->versions[gameIndex][i];
        const M12_VersionSpec* spec = &gameSpec->versions[i];
        memset(version, 0, sizeof(*version));
        version->gameId = spec->gameId;
        version->versionId = spec->versionId;
        version->label = spec->label;
        version->shortLabel = spec->shortLabel;
        for (rootIndex = 0U; rootIndex < rootCount; ++rootIndex) {
            if (m12_try_match_version(roots[rootIndex],
                                      spec,
                                      version->matchedPath,
                                      version->matchedMd5)) {
                version->matched = 1;
                matchedAny = 1;
                if (dataDirResolvedToMatchedRoot && !*dataDirResolvedToMatchedRoot) {
                    /* Runtime source path: when the saved/default data_dir is the
                     * preferred ~/.firestaff/originals but the verified PC34 files
                     * only exist in the legacy ~/.firestaff/data tree, launch must
                     * use the root that actually matched.  Otherwise TITLE and
                     * GRAPHICS.DAT-backed startup animation code is present but
                     * starved of assets at runtime. */
                    m12_copy_string(status->dataDir, sizeof(status->dataDir), roots[rootIndex]);
                    *dataDirResolvedToMatchedRoot = 1;
                }
                break;
            }
        }
    }

    if (strcmp(gameSpec->gameId, "dm1") == 0) {
        status->dm1Available = matchedAny;
    } else if (strcmp(gameSpec->gameId, "csb") == 0) {
        status->csbAvailable = matchedAny;
    } else if (strcmp(gameSpec->gameId, "dm2") == 0) {
        status->dm2Available = matchedAny;
    } else if (strcmp(gameSpec->gameId, "nexus") == 0) {
        status->nexusAvailable = matchedAny;
    } else if (strcmp(gameSpec->gameId, "theron") == 0) {
        status->theronAvailable = matchedAny;
    }
}

void M12_AssetStatus_Scan(M12_AssetStatus* status, const char* requestedDataDir) {
    char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY];
    size_t rootCount;
    int dataDirResolvedToMatchedRoot = 0;
    int i;
    if (!status) {
        return;
    }
    memset(status, 0, sizeof(*status));
    rootCount = m12_build_search_roots(roots, requestedDataDir, status->legacyFallbackDir);
    if (requestedDataDir && requestedDataDir[0] != '\0') {
        m12_copy_string(status->dataDir, sizeof(status->dataDir), requestedDataDir);
    } else if (rootCount > 0U) {
        m12_copy_string(status->dataDir, sizeof(status->dataDir), roots[0]);
    }
    m12_scan_original_candidates(status, roots, rootCount);
    for (i = 0; i < M12_ASSET_GAME_COUNT; ++i) {
        m12_fill_game_versions(status, i, roots, rootCount, &dataDirResolvedToMatchedRoot);
    }
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
    if (strcmp(gameId, "nexus") == 0) {
        return status->nexusAvailable;
    }
    if (strcmp(gameId, "theron") == 0) {
        return status->theronAvailable;
    }
    return 0;
}

int M12_AssetStatus_HasOriginalFileCandidate(const M12_AssetStatus* status) {
    return status && status->originalFileCandidateFound ? 1 : 0;
}

int M12_AssetStatus_GameHasCompleteHashSet(const char* gameId) {
    const M12_GameVersionSpec* spec = m12_find_game_spec(gameId);
    return spec && spec->versionCount > 0U && spec->versions[0].md5 && spec->versions[0].md5[0] != 0 ? 1 : 0;
}

size_t M12_AssetStatus_GameKnownHashCount(const char* gameId) {
    const M12_GameVersionSpec* spec = m12_find_game_spec(gameId);
    size_t i;
    size_t count = 0U;
    if (!spec) {
        return 0U;
    }
    for (i = 0U; i < spec->versionCount; ++i) {
        if (spec->versions[i].md5 && spec->versions[i].md5[0] != 0) {
            ++count;
        }
    }
    return count;
}

size_t M12_AssetStatus_GameVerifiedFileCount(const char* gameId) {
    const M12_GameVersionSpec* spec = m12_find_game_spec(gameId);
    return spec && spec->versionCount > 0U ? 1U : 0U;
}

size_t M12_AssetStatus_GameRequiredFileCount(const char* gameId) {
    const M12_GameVersionSpec* spec = m12_find_game_spec(gameId);
    return spec && spec->versionCount > 0U ? 1U : 0U;
}

const char* M12_AssetStatus_GetDataDir(const M12_AssetStatus* status) {
    if (!status || status->dataDir[0] == '\0') {
        return ".";
    }
    return status->dataDir;
}

const char* M12_AssetStatus_GetLegacyFallbackDir(const M12_AssetStatus* status) {
    if (!status || status->legacyFallbackDir[0] == '\0') {
        return "";
    }
    return status->legacyFallbackDir;
}

size_t M12_AssetStatus_GetVersionCount(const char* gameId) {
    const M12_GameVersionSpec* spec = m12_find_game_spec(gameId);
    return spec ? spec->versionCount : 0U;
}

const M12_AssetVersionStatus* M12_AssetStatus_GetVersion(const M12_AssetStatus* status,
                                                         const char* gameId,
                                                         size_t index) {
    int gameIndex = m12_game_index_from_id(gameId);
    size_t count = M12_AssetStatus_GetVersionCount(gameId);
    if (!status || gameIndex < 0 || index >= count) {
        return NULL;
    }
    return &status->versions[gameIndex][index];
}

int M12_AssetStatus_FindVersionIndex(const char* gameId, const char* versionId) {
    const M12_GameVersionSpec* spec = m12_find_game_spec(gameId);
    size_t i;
    if (!spec || !versionId) {
        return -1;
    }
    for (i = 0U; i < spec->versionCount; ++i) {
        if (strcmp(spec->versions[i].versionId, versionId) == 0) {
            return (int)i;
        }
    }
    return -1;
}
