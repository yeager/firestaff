#include "dm1_v1_runtime_sidecar_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
static char *win32_mkdtemp(char *tmpl) {
    if (_mktemp(tmpl) == NULL) return NULL;
    if (_mkdir(tmpl) != 0) return NULL;
    return tmpl;
}
#define mkdtemp win32_mkdtemp
#else
#include <unistd.h>
#endif

static int expect(int ok, const char* message)
{
    if (!ok) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
#ifdef _WIN32
    char dir[] = "firestaff-dm1-v1-sidecar-XXXXXX";
#else
    char dir[] = "/tmp/firestaff-dm1-v1-sidecar-XXXXXX";
#endif
    char savePath[256];
    char sidecarPath[288];
    FILE* file;
    unsigned char bytes[48];
    DM1_V1_RuntimeSidecarPc34 in;
    DM1_V1_RuntimeSidecarPc34 out;

    if (!mkdtemp(dir)) {
        perror("mkdtemp");
        return 1;
    }
    snprintf(savePath, sizeof(savePath), "%s/save.dat", dir);

    memset(&in, 0, sizeof(in));
    in.leaderHandObjectPresent = 1;
    in.leaderHandThing = 0x1234u;
    in.openChestThing = 0x2345u;
    in.openChestOpenedByEye = 1;
    in.candidateMirrorOrdinal = 18;
    in.candidateMirrorPartyIndex = 2;
    in.candidateMirrorPanelActive = 1;
    in.inventoryPanelActive = 1;
    in.lastPartyMovementTick = 777U;

    if (!expect(DM1_V1_RuntimeSidecar_BuildPathPc34Compat(
                    savePath, sidecarPath, sizeof(sidecarPath)),
                "builds .v1runtime path")) return 1;
    if (!expect(DM1_V1_RuntimeSidecar_WritePc34Compat(&in, savePath),
                "writes sidecar")) return 1;
    if (!expect(DM1_V1_RuntimeSidecar_ReadPc34Compat(savePath, 1000U, &out),
                "reads sidecar")) return 1;
    if (!expect(memcmp(&in, &out, sizeof(in)) == 0,
                "round-trips runtime fields")) return 1;

    file = fopen(sidecarPath, "rb");
    if (!expect(file != NULL, "sidecar file exists")) return 1;
    if (!expect(fread(bytes, 1U, sizeof(bytes), file) == sizeof(bytes),
                "sidecar file is 48 bytes")) return 1;
    fclose(file);
    if (!expect(memcmp(bytes, "FSM11RT1", 8U) == 0,
                "sidecar magic is stable")) return 1;
    if (!expect(bytes[8] == 3U && bytes[9] == 0U &&
                bytes[10] == 0U && bytes[11] == 0U,
                "sidecar version is v3 little-endian")) return 1;

    bytes[44] = 0xFFU;
    bytes[45] = 0xFFU;
    bytes[46] = 0xFFU;
    bytes[47] = 0x7FU;
    file = fopen(sidecarPath, "wb");
    if (!expect(file != NULL, "sidecar opens for clamp rewrite")) return 1;
    if (!expect(fwrite(bytes, 1U, sizeof(bytes), file) == sizeof(bytes),
                "sidecar clamp fixture written")) return 1;
    fclose(file);
    if (!expect(DM1_V1_RuntimeSidecar_ReadPc34Compat(savePath, 1000U, &out),
                "reads sidecar clamp fixture")) return 1;
    if (!expect(out.lastPartyMovementTick == 1000U,
                "movement tick clamps to world tick")) return 1;

    puts("ok: DM1 V1 runtime sidecar preserves ReDMCSB transient runtime fields");
    return 0;
}
