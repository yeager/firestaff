#ifndef FIRESTAFF_ASSET_STATUS_M12_H
#define FIRESTAFF_ASSET_STATUS_M12_H

#include <stddef.h>
#include <stdint.h>
#include "firestaff_theron_media_classify.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_campaign_media_discovery.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_ASSET_DATA_DIR_CAPACITY = 512,
    M12_ASSET_MD5_CAPACITY = 33,
    /* The scanner stores every declared source-verified profile. DM1 has
     * fifteen entries today, so this must never be a smaller presentation
     * bound: M12_AssetStatus_GetVersion() exposes the complete catalogue. */
    M12_ASSET_MAX_VERSIONS_PER_GAME = 16,
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
    int probed;
    int found;
    int parsed;
    int trailerFound;
    char matchedPath[M12_ASSET_DATA_DIR_CAPACITY];
    uint32_t entryCount;
    uint32_t prs3PayloadCount;
    uint32_t rawPayloadCount;
    uint32_t trailerIndex;
    uint32_t mode8bppCount;
    uint32_t mode16bppCount;
    uint32_t mode24bppCount;
    uint32_t mode32bppCount;
    uint32_t trailerModeCount;
    uint32_t trailerFirstOffset;
    uint32_t trailerSecondOffset;
} M12_NexusBpkTrailerMetadata;

typedef struct {
    int active;
    int cancelRequested;
    int cancelled;
    int complete;
    size_t totalSteps;
    size_t completedSteps;
    char currentGameId[16];
    char currentTask[64];
    char currentPath[M12_ASSET_DATA_DIR_CAPACITY];
} M12_AssetScanProgress;

typedef int (*M12_AssetStatusScanProgressFn)(
    const M12_AssetScanProgress* progress,
    void* userData);

typedef struct {
    M12_AssetStatusScanProgressFn progressFn;
    void* progressUserData;
    const int* cancelFlag;
    int honorRequestedDataDir;
    int looseFilesOnly;
} M12_AssetStatusScanOptions;

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
    M12_NexusBpkTrailerMetadata nexusBpkTrailer;
    FirestaffTheronMediaStatus theronMedia;
    Theron_Track02StartupLoaderReceipt theronTrack02LoaderReceipt;
    /* Launcher-only Track 02 admission. Unlike the generic availability
     * scan, this receipt never materializes archive members. */
    Theron_V1Track02CampaignMediaDiscoveryReceipt theronCampaignMedia;
    Theron_V1Track02CaptureTargetPlan theronCampaignMediaPlan;
    int theronCampaignMediaLaunchReady;
    M12_AssetScanProgress scanProgress;
} M12_AssetStatus;

void M12_AssetStatus_Scan(M12_AssetStatus* status, const char* requestedDataDir);
int M12_AssetStatus_ScanWithOptions(M12_AssetStatus* status,
                                    const char* requestedDataDir,
                                    const M12_AssetStatusScanOptions* options);
void M12_AssetStatus_ScanGame(M12_AssetStatus* status,
                              const char* requestedDataDir,
                              const char* gameId);
void M12_AssetStatus_ScanGameWithOptions(
    M12_AssetStatus* status,
    const char* requestedDataDir,
    const char* gameId,
    const M12_AssetStatusScanOptions* options);
/* Strict Theron launcher scan. It inspects only an explicit CUE/BIN/ISO
 * input (or the existing hash-first Track 02 discovery under one directory),
 * binds it to the supplied opaque capture plan, and never extracts or copies
 * virtual/container media. Returns 1 only for a direct, launch-ready receipt. */
int M12_AssetStatus_ScanTheronCampaignMedia(
    M12_AssetStatus* status,
    const char* requestedMediaPath,
    const char* expectedTrack02Md5,
    const Theron_V1Track02CaptureTargetPlan* plan);
const M12_AssetScanProgress* M12_AssetStatus_GetScanProgress(
    const M12_AssetStatus* status);
void M12_AssetStatus_RequestCancel(M12_AssetStatus* status);
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
/* Resolve the original-media owner for one matched edition.  This is used
 * by a version picker at launch time: a selected DM2 FM Towns or Amiga
 * archive must never inherit the first (often PC-DOS) match in a shared
 * data root.  The function only returns a path for an already matched
 * version.  Virtual archive provenance resolves to its user-owned archive;
 * loose files resolve to their containing directory. */
int M12_AssetStatus_ResolveRuntimeDataDirForVersion(
    const M12_AssetStatus* status,
    const char* gameId,
    const char* versionId,
    char* outPath,
    size_t outPathSize);
/* Materialize the selected CSB edition into a version-private runtime
 * directory.  The menu's selected package, rather than the scanner's first
 * match, owns GRAPHICS.DAT and optional presentation media.  FM Towns keeps
 * its CD-specific extractor behind the same boundary. */
int M12_AssetStatus_MaterializeCSBRuntimeVersion(
    const M12_AssetStatus* status, const char* versionId,
    char* outPath, size_t outPathSize);
/* Materialize the selected DM1 FM Towns language package from the original
 * BIN/CUE archive.  The cache contains the verified DATA/JDATA files and the
 * original FM Towns launcher/program siblings; it never substitutes PC34
 * TITLE/SWOOSH media. */
int M12_AssetStatus_MaterializeDM1FmtownsRuntimeVersion(
    const M12_AssetStatus* status, const char* versionId,
    char* outPath, size_t outPathSize);
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
const M12_NexusBpkTrailerMetadata* M12_AssetStatus_GetNexusBpkTrailerMetadata(
    const M12_AssetStatus* status);
int M12_AssetStatus_FindVersionIndex(const char* gameId, const char* versionId);
int M12_AssetStatus_GetVersionArchitecture(const char* gameId, size_t index);
const char* M12_Architecture_Label(int architecture);
const char* M12_Architecture_ShortLabel(int architecture);
int M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
    const M12_AssetStatus* status, const char* gameId, int architecture);
int M12_AssetStatus_GameHasMatchedArchitecture(
    const M12_AssetStatus* status, const char* gameId, int architecture);
const FirestaffTheronMediaStatus* M12_AssetStatus_GetTheronMediaStatus(
    const M12_AssetStatus* status);
const char* M12_AssetStatus_GetTheronLaunchMediaPath(
    const M12_AssetStatus* status);
const Theron_Track02StartupLoaderReceipt*
M12_AssetStatus_GetTheronTrack02LoaderReceipt(const M12_AssetStatus* status);
const Theron_V1Track02CampaignMediaDiscoveryReceipt*
M12_AssetStatus_GetTheronCampaignMedia(const M12_AssetStatus* status);
int M12_AssetStatus_TheronCampaignMediaLaunchReady(const M12_AssetStatus* status);

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
    /* Number of times m12_reuse_verified_theron_refresh refused to
     * reuse a cached Track 02 entry because the recorded path was
     * gone or its on-disk MD5 no longer matched the recorded MD5.
     * Each refusal forces a full scan so the launcher re-discovers
     * the actual on-disk state instead of trusting stale metadata. */
    size_t staleTheronRefreshRejections;
} M12_AssetStatusScanMetrics;

void M12_AssetStatus_TestResetScanMetrics(void);
M12_AssetStatusScanMetrics M12_AssetStatus_TestGetScanMetrics(void);
void M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(const char* graphicsMd5,
                                                          const char* dungeonMd5);
void M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(const char* graphicsMd5,
                                                            const char* dungeonMd5);
void M12_AssetStatus_TestSetDm2SyntheticHashes(const char* graphicsMd5,
                                               const char* dungeonMd5);
void M12_AssetStatus_TestSetDm2Pc98DemoSyntheticHash(const char* graphicsMd5);
void M12_AssetStatus_TestSetCsbSyntheticHashes(const char* graphicsMd5,
                                               const char* dungeonMd5);
void M12_AssetStatus_TestSetNexusSyntheticHash(const char* dataMd5);
void M12_AssetStatus_TestSetTheronSyntheticHash(const char* track02Md5);
#endif

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ASSET_STATUS_M12_H */
