#ifndef FIRESTAFF_ASSET_STATUS_M12_H
#define FIRESTAFF_ASSET_STATUS_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_ASSET_DATA_DIR_CAPACITY = 512
};

typedef struct {
    char dataDir[M12_ASSET_DATA_DIR_CAPACITY];
    int dm1Available;
    int csbAvailable;
    int dm2Available;
} M12_AssetStatus;

void M12_AssetStatus_Scan(M12_AssetStatus* status, const char* requestedDataDir);
int M12_AssetStatus_GameAvailable(const M12_AssetStatus* status,
                                  const char* gameId);
int M12_AssetStatus_GameHasCompleteHashSet(const char* gameId);
size_t M12_AssetStatus_GameKnownHashCount(const char* gameId);
const char* M12_AssetStatus_GetDataDir(const M12_AssetStatus* status);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ASSET_STATUS_M12_H */
