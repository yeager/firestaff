#ifndef FIRESTAFF_SAVE_BYTE_MANIFEST_M12_H
#define FIRESTAFF_SAVE_BYTE_MANIFEST_M12_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define M12_SAVE_BYTE_MANIFEST_TYPE "firestaff-per-game-save-byte-manifest"
#define M12_SAVE_BYTE_MANIFEST_VERSION 1
#define M12_SAVE_BYTE_MANIFEST_PATH_MAX 512
#define M12_SAVE_BYTE_MANIFEST_ID_MAX 32

typedef struct {
    int manifestVersion;
    int runtimeSaveBytesIncluded;
    char gameId[M12_SAVE_BYTE_MANIFEST_ID_MAX];
    char formatId[M12_SAVE_BYTE_MANIFEST_ID_MAX];
    char compatibility[M12_SAVE_BYTE_MANIFEST_ID_MAX * 2];
    char sourceFilename[M12_SAVE_BYTE_MANIFEST_PATH_MAX];
    uint32_t byteCount;
    uint32_t crc32;
} M12_SaveByteManifest;

/* Export a verified per-game save byte payload plus a versioned manifest.
 * Currently supports Firestaff-native DM1 V1 saves (`gameId = "dm1"`).
 * Returns 0 on success. */
int M12_SaveByteManifest_ExportGameSave(const char* gameId,
                                        const char* savePath,
                                        const char* exportDir,
                                        char* outManifestPath,
                                        int outManifestPathSize,
                                        char* outPayloadPath,
                                        int outPayloadPathSize);

/* Import a payload described by a per-game save byte manifest into dataDir.
 * The payload must live beside the manifest and match byte count, CRC, and
 * per-game format marker. Existing destination saves are not overwritten. */
int M12_SaveByteManifest_ImportGameSave(const char* dataDir,
                                        const char* manifestPath,
                                        char* outImportedPath,
                                        int outImportedPathSize);

/* Parse a manifest without importing the payload. */
int M12_SaveByteManifest_Read(const char* manifestPath,
                              M12_SaveByteManifest* outManifest);

/* Validate the payload beside a parsed manifest. */
int M12_SaveByteManifest_VerifyPayload(const char* manifestPath,
                                       const M12_SaveByteManifest* manifest);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SAVE_BYTE_MANIFEST_M12_H */
