#ifndef FIRESTAFF_ASSET_STATUS_M12_H
#define FIRESTAFF_ASSET_STATUS_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_ASSET_DATA_DIR_CAPACITY = 512,
    M12_ASSET_MD5_CAPACITY = 33,
    M12_ASSET_MAX_VERSIONS_PER_GAME = 3,
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
    char dataDir[M12_ASSET_DATA_DIR_CAPACITY];
    char legacyFallbackDir[M12_ASSET_DATA_DIR_CAPACITY];
    int dm1Available;
    int csbAvailable;
    int dm2Available;
    int nexusAvailable;
    int theronAvailable;

    int originalFileCandidateFound;
    M12_AssetVersionStatus versions[M12_ASSET_GAME_COUNT][M12_ASSET_MAX_VERSIONS_PER_GAME];
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
const char* M12_AssetStatus_GetLegacyFallbackDir(const M12_AssetStatus* status);
size_t M12_AssetStatus_GetVersionCount(const char* gameId);
const M12_AssetVersionStatus* M12_AssetStatus_GetVersion(const M12_AssetStatus* status,
                                                         const char* gameId,
                                                         size_t index);
int M12_AssetStatus_FindVersionIndex(const char* gameId, const char* versionId);

/* MD5 hex of a file — used for asset hash verification (Theron Phase 0). */
int m12_file_md5_hex(const char* path, char outHex[33]);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ASSET_STATUS_M12_H */
