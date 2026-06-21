#ifndef FIRESTAFF_ASSET_STATUS_M12_H
#define FIRESTAFF_ASSET_STATUS_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_ASSET_DATA_DIR_CAPACITY = 512,
    M12_ASSET_MD5_CAPACITY = 33,
    M12_ASSET_MAX_VERSIONS_PER_GAME = 4,
    M12_ASSET_MAX_REQUIRED_FILES_PER_GAME = 4,
    M12_ASSET_GAME_COUNT = 5  /* DM1, CSB, DM2, Nexus, Theron */

};

typedef struct {
    const char* gameId;
    const char* versionId;
    const char* label;
    const char* shortLabel;
    int matched;
    char matchedPath[M12_ASSET_DATA_DIR_CAPACITY];
    char matchedMd5[M12_ASSET_MD5_CAPACITY];
} M12_AssetVersionStatus;

typedef struct {
    const char* gameId;
    const char* roleId;
    const char* label;
    int required;
    int matched;
    char matchedPath[M12_ASSET_DATA_DIR_CAPACITY];
    char matchedHash[M12_ASSET_MD5_CAPACITY];
} M12_AssetRequiredFileStatus;

typedef struct {
    char dataDir[M12_ASSET_DATA_DIR_CAPACITY];
    char legacyFallbackDir[M12_ASSET_DATA_DIR_CAPACITY];
    int dm1Available;
    int csbAvailable;
    int dm2Available;
    int nexusAvailable;
    int theronAvailable;

    int originalFileCandidateFound;

    /* V2.2 Modern Graphics asset pack — set to 1 if
     * ~/.firestaff/assets/dm1/modern/modern_asset_manifest.json
     * exists and validates (critical categories present). */
    int v22_modern_assets_installed;

    M12_AssetVersionStatus versions[M12_ASSET_GAME_COUNT][M12_ASSET_MAX_VERSIONS_PER_GAME];
    M12_AssetRequiredFileStatus requiredFiles[M12_ASSET_GAME_COUNT][M12_ASSET_MAX_REQUIRED_FILES_PER_GAME];
    size_t requiredFileCounts[M12_ASSET_GAME_COUNT];
    char runtimeDataDirs[M12_ASSET_GAME_COUNT][M12_ASSET_DATA_DIR_CAPACITY];
} M12_AssetStatus;

void M12_AssetStatus_Scan(M12_AssetStatus* status, const char* requestedDataDir);
int M12_AssetStatus_GameAvailable(const M12_AssetStatus* status,
                                  const char* gameId);
int M12_AssetStatus_HasOriginalFileCandidate(const M12_AssetStatus* status);
int M12_AssetStatus_GameHasCompleteHashSet(const char* gameId);
size_t M12_AssetStatus_GameKnownHashCount(const char* gameId);
size_t M12_AssetStatus_GameVerifiedFileCount(const char* gameId);
size_t M12_AssetStatus_GameRequiredFileCount(const char* gameId);
const char* M12_AssetStatus_GetDataDir(const M12_AssetStatus* status);
const char* M12_AssetStatus_GetRuntimeDataDir(const M12_AssetStatus* status,
                                              const char* gameId);
const char* M12_AssetStatus_GetLegacyFallbackDir(const M12_AssetStatus* status);
size_t M12_AssetStatus_GetVersionCount(const char* gameId);
const M12_AssetVersionStatus* M12_AssetStatus_GetVersion(const M12_AssetStatus* status,
                                                         const char* gameId,
                                                         size_t index);
const M12_AssetVersionStatus* M12_AssetStatus_GetFirstMatchedVersion(const M12_AssetStatus* status,
                                                                    const char* gameId);
size_t M12_AssetStatus_GetRequiredFileCount(const M12_AssetStatus* status,
                                            const char* gameId);
const M12_AssetRequiredFileStatus* M12_AssetStatus_GetRequiredFile(const M12_AssetStatus* status,
                                                                   const char* gameId,
                                                                   size_t index);
int M12_AssetStatus_FindVersionIndex(const char* gameId, const char* versionId);

/* Returns 1 if the V2.2 Modern Graphics asset pack is installed and
 * valid (critical shape categories present), 0 otherwise.
 * Set during M12_AssetStatus_Scan(). */
int M12_AssetStatus_V22ModernAssetsInstalled(const M12_AssetStatus* status);

/* MD5 hex of a file — used for asset hash verification (Theron Phase 0). */
int m12_file_md5_hex(const char* path, char outHex[33]);

#ifdef FIRESTAFF_ASSET_STATUS_TESTING
typedef struct {
    size_t rootCount;
    size_t duplicateRootSkips;
    size_t versionHashLookups;
    size_t requiredHashLookups;
    size_t reusableTheronRefreshes;
} M12_AssetStatusScanMetrics;

void M12_AssetStatus_TestResetScanMetrics(void);
M12_AssetStatusScanMetrics M12_AssetStatus_TestGetScanMetrics(void);
void M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(const char* graphicsMd5,
                                                            const char* dungeonMd5);
void M12_AssetStatus_TestSetDm2SyntheticHashes(const char* graphicsMd5,
                                               const char* dungeonMd5);
void M12_AssetStatus_TestSetCsbSyntheticHashes(const char* graphicsMd5,
                                               const char* dungeonMd5);
void M12_AssetStatus_TestSetNexusSyntheticHash(const char* dataMd5);
void M12_AssetStatus_TestSetTheronSyntheticHash(const char* track02Md5);
#endif

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ASSET_STATUS_M12_H */
