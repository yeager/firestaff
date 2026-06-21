#include "asset_status_m12.h"
#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* dm1_v2_modern_assets_pc34.h provides the V2.2 modern-assets pipeline
 * (manifest validation, shape-source fallback chain, missing-asset guard).
 * It is compiled into firestaff_v2 and linked via firestaff_m11 → firestaff_m12,
 * so the symbols are available at M12 link time. */
#include "dm1_v2_asset_pipeline_pc34.h"

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

typedef struct {
    const char* gameId;
    const char* roleId;
    const char* label;
    const char* md5;
    int matchAnyVersion;
} M12_RequiredFileSpec;

#ifdef FIRESTAFF_ASSET_STATUS_TESTING
static M12_AssetStatusScanMetrics g_m12ScanMetrics;
static char g_m12TestDm1MultiGraphicsMd5[M12_ASSET_MD5_CAPACITY];
static char g_m12TestDm1DungeonMd5[M12_ASSET_MD5_CAPACITY];
static char g_m12TestCsbGraphicsMd5[M12_ASSET_MD5_CAPACITY];
static char g_m12TestCsbDungeonMd5[M12_ASSET_MD5_CAPACITY];
static char g_m12TestDm2GraphicsMd5[M12_ASSET_MD5_CAPACITY];
static char g_m12TestDm2DungeonMd5[M12_ASSET_MD5_CAPACITY];
static char g_m12TestNexusDataMd5[M12_ASSET_MD5_CAPACITY];
static char g_m12TestTheronTrack02Md5[M12_ASSET_MD5_CAPACITY];

void M12_AssetStatus_TestResetScanMetrics(void) {
    memset(&g_m12ScanMetrics, 0, sizeof(g_m12ScanMetrics));
}

M12_AssetStatusScanMetrics M12_AssetStatus_TestGetScanMetrics(void) {
    return g_m12ScanMetrics;
}

void M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(const char* graphicsMd5,
                                                            const char* dungeonMd5) {
    snprintf(g_m12TestDm1MultiGraphicsMd5,
             sizeof(g_m12TestDm1MultiGraphicsMd5),
             "%s",
             graphicsMd5 ? graphicsMd5 : "");
    snprintf(g_m12TestDm1DungeonMd5,
             sizeof(g_m12TestDm1DungeonMd5),
             "%s",
             dungeonMd5 ? dungeonMd5 : "");
}

void M12_AssetStatus_TestSetDm2SyntheticHashes(const char* graphicsMd5,
                                               const char* dungeonMd5) {
    snprintf(g_m12TestDm2GraphicsMd5,
             sizeof(g_m12TestDm2GraphicsMd5),
             "%s",
             graphicsMd5 ? graphicsMd5 : "");
    snprintf(g_m12TestDm2DungeonMd5,
             sizeof(g_m12TestDm2DungeonMd5),
             "%s",
             dungeonMd5 ? dungeonMd5 : "");
}

void M12_AssetStatus_TestSetCsbSyntheticHashes(const char* graphicsMd5,
                                               const char* dungeonMd5) {
    snprintf(g_m12TestCsbGraphicsMd5,
             sizeof(g_m12TestCsbGraphicsMd5),
             "%s",
             graphicsMd5 ? graphicsMd5 : "");
    snprintf(g_m12TestCsbDungeonMd5,
             sizeof(g_m12TestCsbDungeonMd5),
             "%s",
             dungeonMd5 ? dungeonMd5 : "");
}

void M12_AssetStatus_TestSetNexusSyntheticHash(const char* dataMd5) {
    snprintf(g_m12TestNexusDataMd5,
             sizeof(g_m12TestNexusDataMd5),
             "%s",
             dataMd5 ? dataMd5 : "");
}

void M12_AssetStatus_TestSetTheronSyntheticHash(const char* track02Md5) {
    snprintf(g_m12TestTheronTrack02Md5,
             sizeof(g_m12TestTheronTrack02Md5),
             "%s",
             track02Md5 ? track02Md5 : "");
}
#endif

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
 * Data track is Track 02 from CUE/BIN or CUE/ISO disc images.
 * JP: MD5 b7afb338ad31be1025b53f9aff12d73a (Track 02 .bin)
 * US: MD5 f23601102138f87c33025877767ebf76 (Track 02 .bin)
 * MyAbandonware TG-CD English/Japanese Rev 1 downloads (page checked 2026-06-03):
 * JP Rev 1 Track 02 ISO: 397039af02d50d15c70b74088eb8a1cb
 * US Track 02 ISO:       3d8b78571dcd0e6eb8eb4b01eeb7fbba
 * OneDrive: 1drv.ms/f/s!AsBu7boYHQokbYK3rjKY0b5_ra8 (DMFiles/Games folder)
 * Subdir candidates: theron/jp/, theron/us/, theron/ */
static const char* const g_theronTrack02Names[] = {
    "track02.bin",
    "track02.iso",
    "Theron's Quest (Japan) (Track 02).bin",
    "Theron's Quest (US) (Track 02).bin",
    "Theron's Quest (Japan) (Track 02).iso",
    "Theron's Quest (US) (Track 02).iso",
    "TQJP02.iso",
    "TQJP02End.iso",
    "TQUS02.iso",
    "TQUS02End.iso",
    "THQUEST.BIN",
    NULL
};

static const M12_VersionSpec g_theronVersions[] = {
    {"theron", "pce-jp", "PC Engine JP (Track 02)", "PCE JP",
     g_theronTrack02Names, "b7afb338ad31be1025b53f9aff12d73a"},
    {"theron", "pce-en", "TurboGrafx-16 US (Track 02)", "TG16 US",
     g_theronTrack02Names, "f23601102138f87c33025877767ebf76"},
    {"theron", "pce-jp-rev1-iso", "PC Engine JP Rev 1 (Track 02 ISO)", "PCE JP Rev1",
     g_theronTrack02Names, "397039af02d50d15c70b74088eb8a1cb"},
    {"theron", "pce-en-iso", "TurboGrafx-16 US (Track 02 ISO)", "TG16 US ISO",
     g_theronTrack02Names, "3d8b78571dcd0e6eb8eb4b01eeb7fbba"}
};

static const M12_GameVersionSpec g_games[] = {
    {"dm1", g_dm1Versions, sizeof(g_dm1Versions) / sizeof(g_dm1Versions[0])},
    {"csb", g_csbVersions, sizeof(g_csbVersions) / sizeof(g_csbVersions[0])},
    {"dm2", g_dm2Versions, sizeof(g_dm2Versions) / sizeof(g_dm2Versions[0])},
    {"nexus", g_nexusVersions, sizeof(g_nexusVersions) / sizeof(g_nexusVersions[0])},
    {"theron", g_theronVersions, sizeof(g_theronVersions) / sizeof(g_theronVersions[0])}
};

static const M12_RequiredFileSpec g_requiredFiles[] = {
    {"dm1", "graphics", "GRAPHICS.DAT", NULL, 1},
    {"dm1", "dungeon", "DUNGEON.DAT", "766450c940651fc021c92fe5d0d0b3a6", 0},
    {"csb", "graphics", "GRAPHICS.DAT", NULL, 1},
    {"csb", "dungeon", "DUNGEON.DAT", "6695d2acebce49f95db1d8f3a5c733de", 0},
    {"dm2", "graphics", "GRAPHICS.DAT", NULL, 1},
    {"dm2", "dungeon", "DUNGEON.DAT", "6caccd7875009e82fe2e28e7f6d6adc0", 0},
    {"nexus", "data", "DM.BIN / Saturn data marker", NULL, 1},
    /* Theron Track 02: hash-based, fallback path in scan_iso_by_md5.
     * The 5th field of M12_RequiredFileSpec (matchAnyVersion) is
     * the version-match toggle, not the required-flag toggle.
     * m12_fill_required_files hardcodes fileStatus->required = 1,
     * so we list only the primary hash here. Other variants (US,
     * Rev1, US-ISO) live in g_theronVersions[] for runtime lookup
     * but not as required-files (would mark Theron MISSING when
     * not present). Future fix: add a proper required-flag field. */
    {"theron", "track02", "Track 02 data image (JP, primary)", "b7afb338ad31be1025b53f9aff12d73a", 1},
    {NULL, NULL, NULL, NULL, 0}
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
    "track02.iso",
    "Theron's Quest (Japan) (Track 02).bin",
    "Theron's Quest (US) (Track 02).bin",
    "Theron's Quest (Japan) (Track 02).iso",
    "Theron's Quest (US) (Track 02).iso",
    "TQJP02.iso",
    "TQJP02End.iso",
    "TQUS02.iso",
    "TQUS02End.iso",
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

/* Pass 446: filename-only asset identity claims are forbidden — every
 * required asset (DUNGEON.DAT, GRAPHICS.DAT, Track 02 .bin, etc.) must
 * be verified against its spec-supplied MD5 (DEFS.H, G1134, and the
 * Firestaff graphics-hash registry).  ReDMCSB CEDTINCA.C
 * F7059_ReadDungeonPartWithChecksum applies the same idea to the
 * dungeon read path.  This helper exposes the canonical
 *   m12_file_md5_hex(path, md5Hex);
 *   strcmp(md5Hex, spec->md5) == 0
 * gate so other call sites can compose it; asset_status_m12's primary
 * scan path uses an MD5-keyed lookup (asset_find_by_md5) that
 * delegates the same identity check, but this helper is the
 * contract-protected reference form. */
int m12_file_md5_matches_spec(const char* path, const char* specMd5) {
    char md5Hex[33];
    if (!path || !specMd5 || specMd5[0] == '\0') {
        return 0;
    }
    if (!m12_file_md5_hex(path, md5Hex)) {
        return 0;
    }
    return strcmp(md5Hex, specMd5) == 0;
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

static int m12_char_lower(int c) {
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

static int m12_ascii_equals_ignore_case(const char* a, const char* b) {
    if (!a || !b) {
        return 0;
    }
    while (*a != '\0' && *b != '\0') {
        if (m12_char_lower((unsigned char)*a) !=
            m12_char_lower((unsigned char)*b)) {
            return 0;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

static const char* m12_basename_ptr(const char* path) {
    const char* base = path;
    const char* p;
    if (!path) {
        return "";
    }
    for (p = path; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }
    return base;
}

static int m12_path_tail_equals(const char* path, const char* name) {
    return m12_ascii_equals_ignore_case(m12_basename_ptr(path), name);
}

static int m12_path_is_virtual_asset(const char* path) {
    return path && strstr(path, "::") != NULL;
}

static int m12_copy_file_to_path(const char* srcPath, const char* dstPath) {
    FILE* in;
    FILE* out;
    unsigned char buffer[8192];
    size_t n;
    if (!srcPath || !dstPath || m12_path_is_virtual_asset(srcPath)) {
        return 0;
    }
    in = fopen(srcPath, "rb");
    if (!in) {
        return 0;
    }
    out = fopen(dstPath, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }
    while ((n = fread(buffer, 1U, sizeof(buffer), in)) > 0U) {
        if (fwrite(buffer, 1U, n, out) != n) {
            fclose(in);
            fclose(out);
            return 0;
        }
    }
    if (ferror(in)) {
        fclose(in);
        fclose(out);
        return 0;
    }
    fclose(in);
    return fclose(out) == 0;
}

static int m12_game_uses_flat_dat_runtime(const char* gameId) {
    return gameId &&
           (strcmp(gameId, "dm1") == 0 ||
            strcmp(gameId, "csb") == 0 ||
            strcmp(gameId, "dm2") == 0);
}

static int m12_materialize_required_file(const M12_AssetRequiredFileStatus* fileStatus,
                                         const char* outPath) {
    if (!fileStatus || !outPath || !fileStatus->matched) {
        return 0;
    }
    if (m12_path_is_virtual_asset(fileStatus->matchedPath)) {
        return asset_extract_virtual_path(fileStatus->matchedPath, outPath);
    }
    return m12_copy_file_to_path(fileStatus->matchedPath, outPath);
}

static int m12_search_root_already_added(
    const char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY],
    size_t count,
    const char* candidate) {
    size_t i;
    if (!candidate || candidate[0] == '\0') {
        return 1;
    }
    for (i = 0U; i < count; ++i) {
        if (m12_same_path(roots[i], candidate)) {
            return 1;
        }
    }
    return 0;
}

static void m12_add_unique_search_root(
    char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY],
    size_t* count,
    const char* candidate) {
    if (!roots || !count || !candidate || candidate[0] == '\0') {
        return;
    }
    if (m12_search_root_already_added(roots, *count, candidate)) {
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
        g_m12ScanMetrics.duplicateRootSkips++;
#endif
        return;
    }
    if (*count >= M12_SEARCH_ROOT_COUNT) {
        return;
    }
    m12_copy_string(roots[*count], M12_ASSET_DATA_DIR_CAPACITY, candidate);
    ++(*count);
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
        /* Caller passed --data-dir (or equivalent) explicitly. Honor that:
         * use ONLY the requested directory as a search root. The default
         * fallbacks (~/.firestaff/data + friends) only apply on the menu/
         * launch path, which calls Scan with requestedDataDir == NULL. */
        m12_add_unique_search_root(roots, &count, requestedDataDir);
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
        g_m12ScanMetrics.rootCount = count;
#endif
        return count;
    }

    if (FSP_GetDefaultOriginalsDir(defaultOriginals, sizeof(defaultOriginals))) {
        m12_add_unique_search_root(roots, &count, defaultOriginals);
    }
    if (FSP_ResolveDataDir(legacyData, sizeof(legacyData), NULL)) {
        m12_copy_string(legacyFallbackDir, M12_ASSET_DATA_DIR_CAPACITY, legacyData);
        m12_add_unique_search_root(roots, &count, legacyData);
    }
    if (count == 0U) {
        m12_add_unique_search_root(roots, &count, ".");
    }
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
    g_m12ScanMetrics.rootCount = count;
#endif
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

static const char* m12_effective_version_md5(const M12_VersionSpec* spec) {
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
    if (spec && strcmp(spec->gameId, "dm1") == 0 &&
        strcmp(spec->versionId, "pc34-multi") == 0 &&
        g_m12TestDm1MultiGraphicsMd5[0] != '\0') {
        return g_m12TestDm1MultiGraphicsMd5;
    }
    if (spec && strcmp(spec->gameId, "csb") == 0 &&
        strcmp(spec->versionId, "pc34-en") == 0 &&
        g_m12TestCsbGraphicsMd5[0] != '\0') {
        return g_m12TestCsbGraphicsMd5;
    }
    if (spec && strcmp(spec->gameId, "dm2") == 0 &&
        strcmp(spec->versionId, "pc-en") == 0 &&
        g_m12TestDm2GraphicsMd5[0] != '\0') {
        return g_m12TestDm2GraphicsMd5;
    }
    if (spec && strcmp(spec->gameId, "nexus") == 0 &&
        strcmp(spec->versionId, "nexus-saturn-jp") == 0 &&
        g_m12TestNexusDataMd5[0] != '\0') {
        return g_m12TestNexusDataMd5;
    }
    if (spec && strcmp(spec->gameId, "theron") == 0 &&
        strcmp(spec->versionId, "pce-jp") == 0 &&
        g_m12TestTheronTrack02Md5[0] != '\0') {
        return g_m12TestTheronTrack02Md5;
    }
#endif
    return spec ? spec->md5 : NULL;
}

static const char* m12_effective_required_md5(const M12_RequiredFileSpec* spec) {
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
    if (spec && strcmp(spec->gameId, "dm1") == 0 &&
        strcmp(spec->roleId, "dungeon") == 0 &&
        g_m12TestDm1DungeonMd5[0] != '\0') {
        return g_m12TestDm1DungeonMd5;
    }
    if (spec && strcmp(spec->gameId, "csb") == 0 &&
        strcmp(spec->roleId, "dungeon") == 0 &&
        g_m12TestCsbDungeonMd5[0] != '\0') {
        return g_m12TestCsbDungeonMd5;
    }
    if (spec && strcmp(spec->gameId, "dm2") == 0 &&
        strcmp(spec->roleId, "dungeon") == 0 &&
        g_m12TestDm2DungeonMd5[0] != '\0') {
        return g_m12TestDm2DungeonMd5;
    }
    if (spec && strcmp(spec->gameId, "nexus") == 0 &&
        strcmp(spec->roleId, "data") == 0 &&
        g_m12TestNexusDataMd5[0] != '\0') {
        return g_m12TestNexusDataMd5;
    }
#endif
    return spec ? spec->md5 : NULL;
}

static int m12_try_match_version(const char* root,
                                 const M12_VersionSpec* spec,
                                 char matchedPath[M12_ASSET_DATA_DIR_CAPACITY],
                                 char matchedMd5[M12_ASSET_MD5_CAPACITY]) {
    char path[ASSET_PATH_MAX];
    const char* md5 = m12_effective_version_md5(spec);
    if (!root || !spec || !spec->names || !md5 || md5[0] == 0) {
        return 0;
    }
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
    g_m12ScanMetrics.versionHashLookups++;
#endif
    if (!asset_find_by_md5(root, md5, path, (int)sizeof(path), 32)) {
        return 0;
    }
    m12_copy_string(matchedPath, M12_ASSET_DATA_DIR_CAPACITY, path);
    m12_copy_string(matchedMd5, M12_ASSET_MD5_CAPACITY, md5);
    return 1;
}

static int m12_theron_version_index_for_md5(const char* md5) {
    size_t i;
    if (!md5) {
        return -1;
    }
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
    if (g_m12TestTheronTrack02Md5[0] != '\0' &&
        strcmp(md5, g_m12TestTheronTrack02Md5) == 0) {
        return 0;
    }
#endif
    for (i = 0U; i < sizeof(g_theronVersions) / sizeof(g_theronVersions[0]); ++i) {
        if (g_theronVersions[i].md5 &&
            strcmp(g_theronVersions[i].md5, md5) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static void m12_init_version_metadata(M12_AssetStatus* status) {
    int gameIndex;
    if (!status) {
        return;
    }
    for (gameIndex = 0; gameIndex < M12_ASSET_GAME_COUNT; ++gameIndex) {
        const M12_GameVersionSpec* gameSpec = &g_games[gameIndex];
        size_t i;
        for (i = 0U; i < gameSpec->versionCount; ++i) {
            M12_AssetVersionStatus* version = &status->versions[gameIndex][i];
            memset(version, 0, sizeof(*version));
            version->gameId = gameSpec->versions[i].gameId;
            version->versionId = gameSpec->versions[i].versionId;
            version->label = gameSpec->versions[i].label;
            version->shortLabel = gameSpec->versions[i].shortLabel;
        }
    }
}

static void m12_init_required_file_metadata(M12_AssetStatus* status,
                                            int gameIndex) {
    const char* gameId;
    size_t i;
    size_t count = 0U;
    if (!status || gameIndex < 0 || gameIndex >= M12_ASSET_GAME_COUNT) {
        return;
    }
    gameId = g_games[gameIndex].gameId;
    for (i = 0U; g_requiredFiles[i].gameId != NULL; ++i) {
        const M12_RequiredFileSpec* spec = &g_requiredFiles[i];
        M12_AssetRequiredFileStatus* fileStatus;
        if (strcmp(spec->gameId, gameId) != 0) {
            continue;
        }
        if (count >= M12_ASSET_MAX_REQUIRED_FILES_PER_GAME) {
            break;
        }
        fileStatus = &status->requiredFiles[gameIndex][count++];
        memset(fileStatus, 0, sizeof(*fileStatus));
        fileStatus->gameId = spec->gameId;
        fileStatus->roleId = spec->roleId;
        fileStatus->label = spec->label;
        fileStatus->required = 1;
    }
    status->requiredFileCounts[gameIndex] = count;
}

static int m12_path_is_theron_specific_dir(const char* path) {
    char parent[M12_ASSET_DATA_DIR_CAPACITY];
    if (!path || path[0] == '\0' || !FSP_DirExists(path)) {
        return 0;
    }
    if (m12_path_tail_equals(path, "theron")) {
        return 1;
    }
    if (!FSP_ParentDir(parent, sizeof(parent), path)) {
        return 0;
    }
    if ((m12_path_tail_equals(path, "jp") ||
         m12_path_tail_equals(path, "us")) &&
        m12_path_tail_equals(parent, "theron")) {
        return 1;
    }
    return 0;
}

static int m12_derive_theron_runtime_root_for_file(
    const char* filePath,
    char runtimeRoot[M12_ASSET_DATA_DIR_CAPACITY]) {
    char parent[M12_ASSET_DATA_DIR_CAPACITY];
    char grandparent[M12_ASSET_DATA_DIR_CAPACITY];
    char greatgrandparent[M12_ASSET_DATA_DIR_CAPACITY];
    if (!filePath || !runtimeRoot ||
        !FSP_ParentDir(parent, sizeof(parent), filePath)) {
        return 0;
    }
    if (FSP_ParentDir(grandparent, sizeof(grandparent), parent)) {
        if (m12_path_tail_equals(parent, "theron")) {
            m12_copy_string(runtimeRoot, M12_ASSET_DATA_DIR_CAPACITY, grandparent);
            return 1;
        }
        if ((m12_path_tail_equals(parent, "jp") ||
             m12_path_tail_equals(parent, "us")) &&
            m12_path_tail_equals(grandparent, "theron") &&
            FSP_ParentDir(greatgrandparent, sizeof(greatgrandparent), grandparent)) {
            m12_copy_string(runtimeRoot, M12_ASSET_DATA_DIR_CAPACITY, greatgrandparent);
            return 1;
        }
    }
    m12_copy_string(runtimeRoot, M12_ASSET_DATA_DIR_CAPACITY, parent);
    return 1;
}

static int m12_try_match_direct_theron_request(
    const char* requestedDataDir,
    char matchedPath[M12_ASSET_DATA_DIR_CAPACITY],
    char matchedMd5[M12_ASSET_MD5_CAPACITY],
    char runtimeRoot[M12_ASSET_DATA_DIR_CAPACITY],
    int* outVersionIndex) {
    char md5[M12_ASSET_MD5_CAPACITY];
    int versionIndex;
    size_t i;
    if (outVersionIndex) {
        *outVersionIndex = -1;
    }
    if (!requestedDataDir || requestedDataDir[0] == '\0') {
        return 0;
    }
    if (FSP_FileExists(requestedDataDir)) {
        if (!m12_file_md5_hex(requestedDataDir, md5)) {
            return 0;
        }
        versionIndex = m12_theron_version_index_for_md5(md5);
        if (versionIndex < 0) {
            return 0;
        }
        if (!m12_derive_theron_runtime_root_for_file(requestedDataDir, runtimeRoot)) {
            return 0;
        }
        m12_copy_string(matchedPath, M12_ASSET_DATA_DIR_CAPACITY, requestedDataDir);
        m12_copy_string(matchedMd5, M12_ASSET_MD5_CAPACITY, md5);
        if (outVersionIndex) {
            *outVersionIndex = versionIndex;
        }
        return 1;
    }
    if (!m12_path_is_theron_specific_dir(requestedDataDir)) {
        return 0;
    }
    for (i = 0U; i < sizeof(g_theronVersions) / sizeof(g_theronVersions[0]); ++i) {
        if (m12_try_match_version(requestedDataDir,
                                  &g_theronVersions[i],
                                  matchedPath,
                                  matchedMd5)) {
            m12_copy_string(runtimeRoot,
                            M12_ASSET_DATA_DIR_CAPACITY,
                            requestedDataDir);
            if (outVersionIndex) {
                *outVersionIndex = (int)i;
            }
            return 1;
        }
    }
    return 0;
}

static void m12_fill_game_versions(M12_AssetStatus* status,
                                   int gameIndex,
                                   const char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY],
                                   size_t rootCount,
                                   int* dataDirResolvedToMatchedRoot,
                                   int userExplicitDataDir) {
    size_t i;
    size_t rootIndex;
    int matchedAny = 0;
    const M12_GameVersionSpec* gameSpec;
    char rootMatchedPaths[M12_SEARCH_ROOT_COUNT][M12_ASSET_MAX_VERSIONS_PER_GAME][ASSET_PATH_MAX];
    int rootMatched[M12_SEARCH_ROOT_COUNT][M12_ASSET_MAX_VERSIONS_PER_GAME];
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
    }
    memset(rootMatchedPaths, 0, sizeof(rootMatchedPaths));
    memset(rootMatched, 0, sizeof(rootMatched));
    for (rootIndex = 0U; rootIndex < rootCount; ++rootIndex) {
        const char* md5List[M12_ASSET_MAX_VERSIONS_PER_GAME + 1U];
        size_t md5Index;
        size_t md5Count;
        for (md5Index = 0U;
             md5Index < gameSpec->versionCount &&
             md5Index < M12_ASSET_MAX_VERSIONS_PER_GAME;
             ++md5Index) {
            md5List[md5Index] = m12_effective_version_md5(&gameSpec->versions[md5Index]);
        }
        md5Count = md5Index;
        md5List[md5Index] = NULL;
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
        g_m12ScanMetrics.versionHashLookups++;
#endif
        (void)asset_find_all_by_md5_list(roots[rootIndex],
                                         md5List,
                                         rootMatchedPaths[rootIndex],
                                         rootMatched[rootIndex],
                                         (int)md5Count,
                                         32);
    }
    for (i = 0U; i < gameSpec->versionCount; ++i) {
        M12_AssetVersionStatus* version = &status->versions[gameIndex][i];
        const M12_VersionSpec* spec = &gameSpec->versions[i];
        const char* md5 = m12_effective_version_md5(spec);
        if (!md5 || md5[0] == '\0') {
            continue;
        }
        for (rootIndex = 0U; rootIndex < rootCount; ++rootIndex) {
            if (rootMatched[rootIndex][i]) {
                version->matched = 1;
                matchedAny = 1;
                m12_copy_string(version->matchedPath,
                                sizeof(version->matchedPath),
                                rootMatchedPaths[rootIndex][i]);
                m12_copy_string(version->matchedMd5,
                                sizeof(version->matchedMd5),
                                md5);
                m12_copy_string(status->runtimeDataDirs[gameIndex],
                                sizeof(status->runtimeDataDirs[gameIndex]),
                                roots[rootIndex]);
                if (dataDirResolvedToMatchedRoot && !*dataDirResolvedToMatchedRoot && !userExplicitDataDir) {
                    /* Runtime source path: when the saved/default data_dir is the
                     * preferred ~/.firestaff/originals but the verified PC34 files
                     * only exist in the legacy ~/.firestaff/data tree, launch must
                     * use the root that actually matched.  Otherwise TITLE and
                     * GRAPHICS.DAT-backed startup animation code is present but
                     * starved of assets at runtime.
                     *
                     * Skipped when the user passed --data-dir explicitly: CLI scan
                     * must report the user-requested dir, not whatever fallback the
                     * legacy tree happens to provide. */
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

static size_t m12_required_file_count_for_game(const char* gameId) {
    size_t i;
    size_t count = 0U;
    if (!gameId) {
        return 0U;
    }
    for (i = 0U; g_requiredFiles[i].gameId != NULL; ++i) {
        if (strcmp(g_requiredFiles[i].gameId, gameId) == 0) {
            ++count;
        }
    }
    return count;
}

static const M12_AssetVersionStatus* m12_first_matched_version(const M12_AssetStatus* status,
                                                               int gameIndex) {
    const M12_GameVersionSpec* gameSpec;
    size_t i;
    if (!status || gameIndex < 0 || gameIndex >= M12_ASSET_GAME_COUNT) {
        return NULL;
    }
    gameSpec = &g_games[gameIndex];
    for (i = 0U; i < gameSpec->versionCount; ++i) {
        const M12_AssetVersionStatus* version = &status->versions[gameIndex][i];
        if (version->matched) {
            return version;
        }
    }
    return NULL;
}

static int m12_required_hash_matches_any_root(const char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY],
                                              size_t rootCount,
                                              const char* md5,
                                              char matchedPath[M12_ASSET_DATA_DIR_CAPACITY],
                                              char matchedHash[M12_ASSET_MD5_CAPACITY]) {
    size_t rootIndex;
    char path[ASSET_PATH_MAX];
    if (!md5 || md5[0] == '\0') {
        return 0;
    }
    for (rootIndex = 0U; rootIndex < rootCount; ++rootIndex) {
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
        g_m12ScanMetrics.requiredHashLookups++;
#endif
        if (asset_find_by_md5(roots[rootIndex], md5, path, (int)sizeof(path), 32)) {
            m12_copy_string(matchedPath, M12_ASSET_DATA_DIR_CAPACITY, path);
            m12_copy_string(matchedHash, M12_ASSET_MD5_CAPACITY, md5);
            return 1;
        }
    }
    return 0;
}

static int m12_fill_required_files(M12_AssetStatus* status,
                                   int gameIndex,
                                   const char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY],
                                   size_t rootCount) {
    const char* gameId;
    size_t i;
    size_t count = 0U;
    int allRequiredMatched = 1;
    if (!status || gameIndex < 0 || gameIndex >= M12_ASSET_GAME_COUNT) {
        return 0;
    }
    gameId = g_games[gameIndex].gameId;
    for (i = 0U; g_requiredFiles[i].gameId != NULL; ++i) {
        const M12_RequiredFileSpec* spec = &g_requiredFiles[i];
        M12_AssetRequiredFileStatus* fileStatus;
        if (strcmp(spec->gameId, gameId) != 0) {
            continue;
        }
        if (count >= M12_ASSET_MAX_REQUIRED_FILES_PER_GAME) {
            break;
        }
        fileStatus = &status->requiredFiles[gameIndex][count++];
        memset(fileStatus, 0, sizeof(*fileStatus));
        fileStatus->gameId = spec->gameId;
        fileStatus->roleId = spec->roleId;
        fileStatus->label = spec->label;
        /* Every required-files entry is part of the gate.
         * The matchAnyVersion flag changes HOW we compute matched
         * (surface the version's matchedPath for filename-only
         * graphics, or run a hash fallback for hash-pinned files)
         * but never WHETHER the file is required. Forcing
         * required=1 keeps the launch_blocker popup honest: when
         * the user has no CSB data, the CSBGRAPH.DAT-equivalent
         * GRAPHICS.DAT row and the hash-pinned DUNGEON.DAT row
         * must both show up as missing. The earlier
         * `spec->matchAnyVersion ? 0 : 1` shortcut (introduced as
         * a side-effect of pass1039-1041 in commit 35d60e1b)
         * silently disabled the gate for filename-only graphics
         * rows on DM1/CSB/DM2 and made Nexus + Theron report
         * available-with-no-data because their sole required-file
         * row carried matchAnyVersion=1. */
        fileStatus->required = 1;
        if (spec->matchAnyVersion) {
            /* matchAnyVersion=true: filename-only graphics row.
             * Surface the first matched version's path so the
             * missing-files popup and report show where the runtime
             * will load the asset from. The version match itself
             * (m12_fill_game_versions) is what gates availability
             * upstream — this just propagates the result down. */
            const M12_AssetVersionStatus* version = m12_first_matched_version(status, gameIndex);
            if (version) {
                fileStatus->matched = 1;
                m12_copy_string(fileStatus->matchedPath, sizeof(fileStatus->matchedPath), version->matchedPath);
                m12_copy_string(fileStatus->matchedHash, sizeof(fileStatus->matchedHash), version->matchedMd5);
            }
        } else if (m12_required_hash_matches_any_root(roots,
                                                      rootCount,
                                                      m12_effective_required_md5(spec),
                                                      fileStatus->matchedPath,
                                                      fileStatus->matchedHash)) {
            fileStatus->matched = 1;
        }
        if (fileStatus->required && !fileStatus->matched) {
            allRequiredMatched = 0;
        }
    }
    status->requiredFileCounts[gameIndex] = count;
    return count > 0U && allRequiredMatched;
}

static void m12_apply_required_game_availability(M12_AssetStatus* status,
                                                 int gameIndex,
                                                 int available) {
    const char* gameId;
    if (!status || gameIndex < 0 || gameIndex >= M12_ASSET_GAME_COUNT) {
        return;
    }
    gameId = g_games[gameIndex].gameId;
    if (strcmp(gameId, "dm1") == 0) {
        status->dm1Available = available;
    } else if (strcmp(gameId, "csb") == 0) {
        status->csbAvailable = available;
    } else if (strcmp(gameId, "dm2") == 0) {
        status->dm2Available = available;
    } else if (strcmp(gameId, "nexus") == 0) {
        status->nexusAvailable = available;
    } else if (strcmp(gameId, "theron") == 0) {
        status->theronAvailable = available;
    }
}

static void m12_materialize_runtime_cache_for_game(M12_AssetStatus* status,
                                                   int gameIndex) {
    const char* gameId;
    char userDataDir[M12_ASSET_DATA_DIR_CAPACITY];
    char cacheRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char gameCacheDir[M12_ASSET_DATA_DIR_CAPACITY];
    size_t i;
    int hasVirtualRequired = 0;
    if (!status || gameIndex < 0 || gameIndex >= M12_ASSET_GAME_COUNT) {
        return;
    }
    gameId = g_games[gameIndex].gameId;
    if (!m12_game_uses_flat_dat_runtime(gameId) ||
        !M12_AssetStatus_GameAvailable(status, gameId)) {
        return;
    }
    for (i = 0U; i < status->requiredFileCounts[gameIndex]; ++i) {
        if (m12_path_is_virtual_asset(status->requiredFiles[gameIndex][i].matchedPath)) {
            hasVirtualRequired = 1;
            break;
        }
    }
    if (!hasVirtualRequired) {
        return;
    }
    if (!FSP_GetUserDataDir(userDataDir, sizeof(userDataDir)) ||
        !FSP_JoinPath(cacheRoot, sizeof(cacheRoot), userDataDir, "asset-cache") ||
        !FSP_JoinPath(gameCacheDir, sizeof(gameCacheDir), cacheRoot, gameId) ||
        !FSP_CreateDirectoryRecursive(gameCacheDir)) {
        return;
    }
    for (i = 0U; i < status->requiredFileCounts[gameIndex]; ++i) {
        M12_AssetRequiredFileStatus* fileStatus = &status->requiredFiles[gameIndex][i];
        char outPath[M12_ASSET_DATA_DIR_CAPACITY];
        if (!fileStatus->matched || !fileStatus->label ||
            !FSP_JoinPath(outPath, sizeof(outPath), gameCacheDir, fileStatus->label)) {
            return;
        }
        if (!m12_materialize_required_file(fileStatus, outPath)) {
            return;
        }
        m12_copy_string(fileStatus->matchedPath, sizeof(fileStatus->matchedPath), outPath);
    }
    m12_copy_string(status->runtimeDataDirs[gameIndex],
                    sizeof(status->runtimeDataDirs[gameIndex]),
                    cacheRoot);
}

static void m12_refresh_v22_modern_asset_status(M12_AssetStatus* status) {
    if (!status) {
        return;
    }
    /* V2.2 Modern Graphics asset pack detection.
     * Set up the manifest path relative to the resolved data dir and query
     * whether the modern asset pack is installed. The result is stored in
     * both g_v22_modern_assets_installed (module state, used by the
     * fallback chain) and status->v22_modern_assets_installed (caller-visible
     * struct field, used by the launcher UI to show "(not installed)"). */
    m11_v22_set_manifest_path(status->dataDir);
    {
        int installed = m11_v22_modern_assets_available();
        status->v22_modern_assets_installed = installed;
        m11_v22_set_installed(installed);
    }
}

static int m12_scan_direct_theron_request(M12_AssetStatus* status,
                                          const char* requestedDataDir) {
    char matchedPath[M12_ASSET_DATA_DIR_CAPACITY];
    char matchedMd5[M12_ASSET_MD5_CAPACITY];
    char runtimeRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char legacyData[M12_ASSET_DATA_DIR_CAPACITY];
    int theronIndex = m12_game_index_from_id("theron");
    int versionIndex = -1;
    int i;
    size_t requiredIndex;
    int requiredMatched = 0;
    if (!status || theronIndex < 0 ||
        !m12_try_match_direct_theron_request(requestedDataDir,
                                             matchedPath,
                                             matchedMd5,
                                             runtimeRoot,
                                             &versionIndex) ||
        versionIndex < 0) {
        return 0;
    }

    memset(status, 0, sizeof(*status));
    m12_copy_string(status->dataDir, sizeof(status->dataDir), runtimeRoot);
    if (FSP_ResolveDataDir(legacyData, sizeof(legacyData), NULL)) {
        m12_copy_string(status->legacyFallbackDir,
                        sizeof(status->legacyFallbackDir),
                        legacyData);
    }
    for (i = 0; i < M12_ASSET_GAME_COUNT; ++i) {
        m12_copy_string(status->runtimeDataDirs[i],
                        sizeof(status->runtimeDataDirs[i]),
                        status->dataDir);
    }
    m12_init_version_metadata(status);
    for (i = 0; i < M12_ASSET_GAME_COUNT; ++i) {
        m12_init_required_file_metadata(status, i);
    }
    status->versions[theronIndex][versionIndex].matched = 1;
    m12_copy_string(status->versions[theronIndex][versionIndex].matchedPath,
                    sizeof(status->versions[theronIndex][versionIndex].matchedPath),
                    matchedPath);
    m12_copy_string(status->versions[theronIndex][versionIndex].matchedMd5,
                    sizeof(status->versions[theronIndex][versionIndex].matchedMd5),
                    matchedMd5);
    m12_copy_string(status->runtimeDataDirs[theronIndex],
                    sizeof(status->runtimeDataDirs[theronIndex]),
                    runtimeRoot);
    status->originalFileCandidateFound = 1;
    for (requiredIndex = 0U;
         requiredIndex < status->requiredFileCounts[theronIndex];
         ++requiredIndex) {
        M12_AssetRequiredFileStatus* required =
            &status->requiredFiles[theronIndex][requiredIndex];
        if (required->roleId && strcmp(required->roleId, "track02") == 0) {
            required->matched = 1;
            requiredMatched = 1;
            m12_copy_string(required->matchedPath,
                            sizeof(required->matchedPath),
                            matchedPath);
            m12_copy_string(required->matchedHash,
                            sizeof(required->matchedHash),
                            matchedMd5);
        }
    }
    m12_apply_required_game_availability(status, theronIndex, requiredMatched);
    m12_refresh_v22_modern_asset_status(status);
    return 1;
}

static int m12_scan_explicit_file_request(M12_AssetStatus* status,
                                          const char* requestedDataDir) {
    char parent[M12_ASSET_DATA_DIR_CAPACITY];
    char legacyData[M12_ASSET_DATA_DIR_CAPACITY];
    int i;
    if (!status || !requestedDataDir || !FSP_FileExists(requestedDataDir)) {
        return 0;
    }
    memset(status, 0, sizeof(*status));
    if (FSP_ParentDir(parent, sizeof(parent), requestedDataDir)) {
        m12_copy_string(status->dataDir, sizeof(status->dataDir), parent);
    } else {
        m12_copy_string(status->dataDir, sizeof(status->dataDir), ".");
    }
    if (FSP_ResolveDataDir(legacyData, sizeof(legacyData), NULL)) {
        m12_copy_string(status->legacyFallbackDir,
                        sizeof(status->legacyFallbackDir),
                        legacyData);
    }
    for (i = 0; i < M12_ASSET_GAME_COUNT; ++i) {
        m12_copy_string(status->runtimeDataDirs[i],
                        sizeof(status->runtimeDataDirs[i]),
                        status->dataDir);
    }
    m12_init_version_metadata(status);
    for (i = 0; i < M12_ASSET_GAME_COUNT; ++i) {
        m12_init_required_file_metadata(status, i);
    }
    status->originalFileCandidateFound = 1;
    m12_refresh_v22_modern_asset_status(status);
    return 1;
}

static int m12_reuse_verified_theron_refresh(M12_AssetStatus* status,
                                             const char* requestedDataDir) {
    int theronIndex = m12_game_index_from_id("theron");
    const M12_AssetVersionStatus* version;
    size_t requiredIndex;
    if (!status || theronIndex < 0 ||
        !requestedDataDir || requestedDataDir[0] == '\0' ||
        status->dataDir[0] == '\0' ||
        strcmp(status->dataDir, requestedDataDir) != 0 ||
        !status->theronAvailable) {
        return 0;
    }
    version = m12_first_matched_version(status, theronIndex);
    if (!version || !version->matched ||
        version->matchedPath[0] == '\0' ||
        version->matchedMd5[0] == '\0' ||
        status->runtimeDataDirs[theronIndex][0] == '\0') {
        return 0;
    }
    for (requiredIndex = 0U;
         requiredIndex < status->requiredFileCounts[theronIndex];
         ++requiredIndex) {
        const M12_AssetRequiredFileStatus* required =
            &status->requiredFiles[theronIndex][requiredIndex];
        if (required->roleId && strcmp(required->roleId, "track02") == 0) {
            if (!required->matched ||
                strcmp(required->matchedPath, version->matchedPath) != 0 ||
                strcmp(required->matchedHash, version->matchedMd5) != 0) {
                return 0;
            }
#ifdef FIRESTAFF_ASSET_STATUS_TESTING
            g_m12ScanMetrics.reusableTheronRefreshes++;
#endif
            m12_refresh_v22_modern_asset_status(status);
            return 1;
        }
    }
    return 0;
}

void M12_AssetStatus_Scan(M12_AssetStatus* status, const char* requestedDataDir) {
    char roots[M12_SEARCH_ROOT_COUNT][M12_ASSET_DATA_DIR_CAPACITY];
    size_t rootCount;
    int dataDirResolvedToMatchedRoot = 0;
    int i;
    if (!status) {
        return;
    }
    if (m12_reuse_verified_theron_refresh(status, requestedDataDir)) {
        return;
    }
    if (m12_scan_direct_theron_request(status, requestedDataDir)) {
        return;
    }
    if (m12_scan_explicit_file_request(status, requestedDataDir)) {
        return;
    }
    memset(status, 0, sizeof(*status));
    rootCount = m12_build_search_roots(roots, requestedDataDir, status->legacyFallbackDir);
    if (requestedDataDir && requestedDataDir[0] != '\0') {
        m12_copy_string(status->dataDir, sizeof(status->dataDir), requestedDataDir);
    } else if (rootCount > 0U) {
        m12_copy_string(status->dataDir, sizeof(status->dataDir), roots[0]);
    }
    for (i = 0; i < M12_ASSET_GAME_COUNT; ++i) {
        m12_copy_string(status->runtimeDataDirs[i],
                        sizeof(status->runtimeDataDirs[i]),
                        status->dataDir);
    }
    m12_scan_original_candidates(status, roots, rootCount);
    int userExplicitDataDir = (requestedDataDir && requestedDataDir[0] != '\0') ? 1 : 0;
    for (i = 0; i < M12_ASSET_GAME_COUNT; ++i) {
        m12_fill_game_versions(status, i, roots, rootCount, &dataDirResolvedToMatchedRoot, userExplicitDataDir);
    }
    for (i = 0; i < M12_ASSET_GAME_COUNT; ++i) {
        m12_apply_required_game_availability(status,
                                             i,
                                             m12_fill_required_files(status, i, roots, rootCount));
        if (!status->originalFileCandidateFound) {
            size_t fileIndex;
            for (fileIndex = 0U; fileIndex < status->requiredFileCounts[i]; ++fileIndex) {
                if (status->requiredFiles[i][fileIndex].matched) {
                    status->originalFileCandidateFound = 1;
                    break;
                }
            }
        }
        m12_materialize_runtime_cache_for_game(status, i);
    }

    m12_refresh_v22_modern_asset_status(status);
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
    return m12_required_file_count_for_game(gameId);
}

size_t M12_AssetStatus_GameRequiredFileCount(const char* gameId) {
    return m12_required_file_count_for_game(gameId);
}

const char* M12_AssetStatus_GetDataDir(const M12_AssetStatus* status) {
    if (!status || status->dataDir[0] == '\0') {
        return ".";
    }
    return status->dataDir;
}

const char* M12_AssetStatus_GetRuntimeDataDir(const M12_AssetStatus* status,
                                              const char* gameId) {
    int gameIndex = m12_game_index_from_id(gameId);
    if (!status || gameIndex < 0 ||
        status->runtimeDataDirs[gameIndex][0] == '\0') {
        return M12_AssetStatus_GetDataDir(status);
    }
    return status->runtimeDataDirs[gameIndex];
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

size_t M12_AssetStatus_GetRequiredFileCount(const M12_AssetStatus* status,
                                            const char* gameId) {
    int gameIndex = m12_game_index_from_id(gameId);
    if (!status || gameIndex < 0) {
        return 0U;
    }
    return status->requiredFileCounts[gameIndex];
}

const M12_AssetRequiredFileStatus* M12_AssetStatus_GetRequiredFile(const M12_AssetStatus* status,
                                                                   const char* gameId,
                                                                   size_t index) {
    int gameIndex = m12_game_index_from_id(gameId);
    if (!status || gameIndex < 0 ||
        index >= status->requiredFileCounts[gameIndex]) {
        return NULL;
    }
    return &status->requiredFiles[gameIndex][index];
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

/* Returns 1 if the V2.2 Modern Graphics asset pack is installed and
 * valid (critical shape categories present), 0 otherwise.
 * Set during M12_AssetStatus_Scan(). */
int M12_AssetStatus_V22ModernAssetsInstalled(const M12_AssetStatus* status) {
    return status ? status->v22_modern_assets_installed : 0;
}
