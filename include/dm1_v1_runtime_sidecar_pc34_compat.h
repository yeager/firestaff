#ifndef FIRESTAFF_DM1_V1_RUNTIME_SIDECAR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_RUNTIME_SIDECAR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int leaderHandObjectPresent;
    unsigned short leaderHandThing;
    unsigned short openChestThing;
    int openChestOpenedByEye;
    int candidateMirrorOrdinal;
    int candidateMirrorPartyIndex;
    int candidateMirrorPanelActive;
    int inventoryPanelActive;
    uint32_t lastPartyMovementTick;
} DM1_V1_RuntimeSidecarPc34;

const char* DM1_V1_RuntimeSidecar_SourceEvidencePc34Compat(void);
int DM1_V1_RuntimeSidecar_BuildPathPc34Compat(const char* savePath,
                                              char* out,
                                              size_t outSize);
int DM1_V1_RuntimeSidecar_WritePc34Compat(
    const DM1_V1_RuntimeSidecarPc34* sidecar,
    const char* savePath);
int DM1_V1_RuntimeSidecar_ReadPc34Compat(
    const char* savePath,
    uint32_t worldGameTick,
    DM1_V1_RuntimeSidecarPc34* outSidecar);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_RUNTIME_SIDECAR_PC34_COMPAT_H */
