#include "dm1_v1_release_capture_manifest_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

static DM1_V1_ReleaseCaptureFramePc34 frame(unsigned int hash) {
    DM1_V1_ReleaseCaptureFramePc34 result;
    memset(&result, 0, sizeof(result));
    result.capturedFromMacWindow = 1;
    result.capturedFromReleaseApp = 1;
    result.width = 1280;
    result.height = 800;
    result.byteCount = result.width * result.height * 4;
    result.framebufferHash = hash;
    return result;
}

int main(void) {
    DM1_V1_F0437TitleMaterialReceiptPc34 title;
    DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34 hoc;
    DM1_V1_ActionSpellRuntimeCaptureReceiptPc34 hud;
    DM1_V1_ReleaseCaptureManifestInputPc34 input;
    DM1_V1_ReleaseCaptureManifestPc34 manifest;

    memset(&title, 0, sizeof(title));
    memset(&hoc, 0, sizeof(hoc));
    memset(&hud, 0, sizeof(hud));
    memset(&input, 0, sizeof(input));
    title.accepted = title.c001_title_bound = title.presents_region_bound = 1;
    title.dungeon_region_bound = title.master_region_bound = 1;
    title.presents_palette_bound = title.title_palette_bound = 1;
    title.suppress_synthetic_fallback = 1;
    hoc.ready = hoc.consumed_publish_receipt = 1;
    hoc.captured_from_mac_window = hoc.captured_from_release_app = 1;
    hoc.framebuffer_hash = 0x22334455u;
    hud.accepted = hud.runtimeCaptureCurrent = 1;
    hud.sourceAssetCount = hud.sourceCommandCount = 1;
    hud.suppressSyntheticFallback = 1;
    hud.commandFingerprint = 0x01020304u;
    hud.orderingFingerprint = 0x05060708u;
    hud.serial = 7u;
    input.titleReceipt = &title;
    input.hocReceipt = &hoc;
    input.hudReceipt = &hud;
    input.titleFrame = frame(0x11223344u);
    input.hocFrame = frame(hoc.framebuffer_hash);
    input.hudFrame = frame(0x33445566u);

    CHECK(dm1_v1_release_capture_manifest_build_pc34(&input, &manifest));
    CHECK(manifest.ready && manifest.titleReceiptConsumed &&
          manifest.hocReceiptConsumed && manifest.hudReceiptConsumed &&
          manifest.manifestHash != 0u);

    input.hudFrame.framebufferHash = input.titleFrame.framebufferHash;
    CHECK(dm1_v1_release_capture_manifest_build_pc34(&input, &manifest));
    CHECK(!manifest.ready && manifest.hudFrameCaptured);

    input.hudFrame = frame(0x33445566u);
    input.hocFrame.framebufferHash = 0x77777777u;
    CHECK(dm1_v1_release_capture_manifest_build_pc34(&input, &manifest));
    CHECK(!manifest.ready && !manifest.hocFrameCaptured);

    input.hocFrame = frame(hoc.framebuffer_hash);
    title.suppress_synthetic_fallback = 0;
    CHECK(dm1_v1_release_capture_manifest_build_pc34(&input, &manifest));
    CHECK(!manifest.ready && !manifest.titleReceiptConsumed);

    printf("dm1 release capture manifest: %d failure(s)\n", failures);
    return failures ? 1 : 0;
}
