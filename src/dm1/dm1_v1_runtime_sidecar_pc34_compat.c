#include "dm1_v1_runtime_sidecar_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const unsigned char g_dm1_v1_runtime_sidecar_magic[8] = {
    'F', 'S', 'M', '1', '1', 'R', 'T', '1'
};

static void dm1_v1_runtime_sidecar_write_u32_le(unsigned char* dst,
                                                uint32_t value)
{
    dst[0] = (unsigned char)(value & 0xFFU);
    dst[1] = (unsigned char)((value >> 8) & 0xFFU);
    dst[2] = (unsigned char)((value >> 16) & 0xFFU);
    dst[3] = (unsigned char)((value >> 24) & 0xFFU);
}

static uint32_t dm1_v1_runtime_sidecar_read_u32_le(const unsigned char* src)
{
    return ((uint32_t)src[0]) |
           ((uint32_t)src[1] << 8) |
           ((uint32_t)src[2] << 16) |
           ((uint32_t)src[3] << 24);
}

const char* DM1_V1_RuntimeSidecar_SourceEvidencePc34Compat(void)
{
    return
        "ReDMCSB CHAMPION.C F0297/F0298 owns G4055_s_LeaderHandObject\n"
        "ReDMCSB CHEST.C F0333/F0334 owns G0426_T_OpenChest/G0425 slots\n"
        "ReDMCSB REVIVE.C F0280/F0282 and COMMAND.C F0359/F0380 own C040 mirror panel\n"
        "ReDMCSB LOADSAVE.C F0435/F0898 restore world state separately from these host-runtime fields";
}

int DM1_V1_RuntimeSidecar_BuildPathPc34Compat(const char* savePath,
                                              char* out,
                                              size_t outSize)
{
    int rc;

    if (!savePath || !savePath[0] || !out || outSize == 0U) {
        return 0;
    }
    rc = snprintf(out, outSize, "%s.v1runtime", savePath);
    return (rc > 0 && rc < (int)outSize) ? 1 : 0;
}

int DM1_V1_RuntimeSidecar_WritePc34Compat(
    const DM1_V1_RuntimeSidecarPc34* sidecar,
    const char* savePath)
{
    char sidecarPath[528];
    unsigned char buf[48];
    FILE* file;

    if (!sidecar || !DM1_V1_RuntimeSidecar_BuildPathPc34Compat(
            savePath, sidecarPath, sizeof(sidecarPath))) {
        return 0;
    }

    memset(buf, 0, sizeof(buf));
    memcpy(buf, g_dm1_v1_runtime_sidecar_magic,
           sizeof(g_dm1_v1_runtime_sidecar_magic));
    dm1_v1_runtime_sidecar_write_u32_le(buf + 8, 3U);
    dm1_v1_runtime_sidecar_write_u32_le(
        buf + 12, (uint32_t)(sidecar->leaderHandObjectPresent ? 1 : 0));
    dm1_v1_runtime_sidecar_write_u32_le(
        buf + 16, (uint32_t)sidecar->leaderHandThing);
    dm1_v1_runtime_sidecar_write_u32_le(
        buf + 20, (uint32_t)sidecar->openChestThing);
    dm1_v1_runtime_sidecar_write_u32_le(
        buf + 24, (uint32_t)(sidecar->openChestOpenedByEye ? 1 : 0));
    dm1_v1_runtime_sidecar_write_u32_le(
        buf + 28, (uint32_t)(int32_t)sidecar->candidateMirrorOrdinal);
    dm1_v1_runtime_sidecar_write_u32_le(
        buf + 32, (uint32_t)(int32_t)sidecar->candidateMirrorPartyIndex);
    dm1_v1_runtime_sidecar_write_u32_le(
        buf + 36, (uint32_t)(sidecar->candidateMirrorPanelActive ? 1 : 0));
    dm1_v1_runtime_sidecar_write_u32_le(
        buf + 40, (uint32_t)(sidecar->inventoryPanelActive ? 1 : 0));
    dm1_v1_runtime_sidecar_write_u32_le(
        buf + 44, sidecar->lastPartyMovementTick);

    file = fopen(sidecarPath, "wb");
    if (!file) {
        return 0;
    }
    if (fwrite(buf, 1U, sizeof(buf), file) != sizeof(buf)) {
        (void)fclose(file);
        return 0;
    }
    return fclose(file) == 0 ? 1 : 0;
}

int DM1_V1_RuntimeSidecar_ReadPc34Compat(
    const char* savePath,
    uint32_t worldGameTick,
    DM1_V1_RuntimeSidecarPc34* outSidecar)
{
    char sidecarPath[528];
    unsigned char buf[48];
    FILE* file;
    size_t readCount;
    uint32_t version;

    if (!outSidecar || !DM1_V1_RuntimeSidecar_BuildPathPc34Compat(
            savePath, sidecarPath, sizeof(sidecarPath))) {
        return 0;
    }

    file = fopen(sidecarPath, "rb");
    if (!file) {
        return 0;
    }
    memset(buf, 0, sizeof(buf));
    readCount = fread(buf, 1U, sizeof(buf), file);
    if (readCount != 28U && readCount != 44U && readCount != sizeof(buf)) {
        (void)fclose(file);
        return 0;
    }
    if (fclose(file) != 0) {
        return 0;
    }

    version = dm1_v1_runtime_sidecar_read_u32_le(buf + 8);
    if (memcmp(buf, g_dm1_v1_runtime_sidecar_magic,
               sizeof(g_dm1_v1_runtime_sidecar_magic)) != 0 ||
        (version != 1U && version != 2U && version != 3U) ||
        (version == 2U && readCount < 44U) ||
        (version == 3U && readCount < sizeof(buf))) {
        return 0;
    }

    memset(outSidecar, 0, sizeof(*outSidecar));
    outSidecar->candidateMirrorOrdinal = -1;
    outSidecar->candidateMirrorPartyIndex = -1;
    outSidecar->leaderHandObjectPresent =
        dm1_v1_runtime_sidecar_read_u32_le(buf + 12) ? 1 : 0;
    outSidecar->leaderHandThing =
        (unsigned short)dm1_v1_runtime_sidecar_read_u32_le(buf + 16);
    outSidecar->openChestThing =
        (unsigned short)dm1_v1_runtime_sidecar_read_u32_le(buf + 20);
    outSidecar->openChestOpenedByEye =
        dm1_v1_runtime_sidecar_read_u32_le(buf + 24) ? 1 : 0;
    if (version >= 2U) {
        outSidecar->candidateMirrorOrdinal =
            (int)(int32_t)dm1_v1_runtime_sidecar_read_u32_le(buf + 28);
        outSidecar->candidateMirrorPartyIndex =
            (int)(int32_t)dm1_v1_runtime_sidecar_read_u32_le(buf + 32);
        outSidecar->candidateMirrorPanelActive =
            dm1_v1_runtime_sidecar_read_u32_le(buf + 36) ? 1 : 0;
        outSidecar->inventoryPanelActive =
            dm1_v1_runtime_sidecar_read_u32_le(buf + 40) ? 1 : 0;
    }
    outSidecar->lastPartyMovementTick =
        version >= 3U ? dm1_v1_runtime_sidecar_read_u32_le(buf + 44)
                      : worldGameTick;
    if (outSidecar->lastPartyMovementTick > worldGameTick) {
        outSidecar->lastPartyMovementTick = worldGameTick;
    }
    return 1;
}
