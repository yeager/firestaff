#include "dm1_v1_release_capture_manifest_pc34_compat.h"

#include <string.h>

static int dm1_v1_release_capture_frame_valid_pc34(
    const DM1_V1_ReleaseCaptureFramePc34 *frame) {
    return frame && frame->capturedFromMacWindow &&
           frame->capturedFromReleaseApp && frame->width >= 320 &&
           frame->height >= 200 &&
           frame->byteCount >= frame->width * frame->height * 4 &&
           frame->framebufferHash != 0u;
}

static unsigned int dm1_v1_release_capture_manifest_hash_pc34(
    unsigned int titleHash,
    unsigned int hocHash,
    unsigned int hudHash,
    unsigned int hudSerial) {
    unsigned int hash = 2166136261u;
    const unsigned int values[] = {titleHash, hocHash, hudHash, hudSerial};
    unsigned int valueIndex;
    unsigned int byteIndex;

    for (valueIndex = 0u; valueIndex < sizeof(values) / sizeof(values[0]);
         ++valueIndex) {
        for (byteIndex = 0u; byteIndex < 4u; ++byteIndex) {
            hash ^= (values[valueIndex] >> (byteIndex * 8u)) & 0xffu;
            hash *= 16777619u;
        }
    }
    return hash;
}

int dm1_v1_release_capture_manifest_build_pc34(
    const DM1_V1_ReleaseCaptureManifestInputPc34 *input,
    DM1_V1_ReleaseCaptureManifestPc34 *outManifest) {
    DM1_V1_ReleaseCaptureManifestPc34 manifest;

    if (!outManifest) {
        return 0;
    }
    memset(&manifest, 0, sizeof(manifest));
    if (!input || !input->titleReceipt || !input->hocReceipt ||
        !input->hudReceipt) {
        *outManifest = manifest;
        return input ? 1 : 0;
    }

    manifest.handled = 1;
    manifest.titleReceiptConsumed =
        input->titleReceipt->accepted && input->titleReceipt->c001_title_bound &&
        input->titleReceipt->presents_region_bound &&
        input->titleReceipt->dungeon_region_bound &&
        input->titleReceipt->master_region_bound &&
        input->titleReceipt->presents_palette_bound &&
        input->titleReceipt->title_palette_bound &&
        input->titleReceipt->suppress_synthetic_fallback;
    manifest.hocReceiptConsumed = input->hocReceipt->ready &&
        input->hocReceipt->consumed_publish_receipt &&
        input->hocReceipt->captured_from_mac_window &&
        input->hocReceipt->captured_from_release_app;
    manifest.hudReceiptConsumed = input->hudReceipt->accepted &&
        input->hudReceipt->runtimeCaptureCurrent &&
        input->hudReceipt->sourceAssetCount > 0 &&
        input->hudReceipt->sourceCommandCount > 0 &&
        input->hudReceipt->suppressSyntheticFallback &&
        input->hudReceipt->commandFingerprint != 0u &&
        input->hudReceipt->orderingFingerprint != 0u;
    manifest.titleFrameCaptured =
        dm1_v1_release_capture_frame_valid_pc34(&input->titleFrame);
    manifest.hocFrameCaptured =
        dm1_v1_release_capture_frame_valid_pc34(&input->hocFrame) &&
        input->hocFrame.framebufferHash == input->hocReceipt->framebuffer_hash;
    manifest.hudFrameCaptured =
        dm1_v1_release_capture_frame_valid_pc34(&input->hudFrame);
    manifest.titleFrameHash = input->titleFrame.framebufferHash;
    manifest.hocFrameHash = input->hocFrame.framebufferHash;
    manifest.hudFrameHash = input->hudFrame.framebufferHash;
    manifest.manifestHash = dm1_v1_release_capture_manifest_hash_pc34(
        manifest.titleFrameHash, manifest.hocFrameHash, manifest.hudFrameHash,
        input->hudReceipt->serial);
    manifest.sourceEvidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441; DRAWVIEW.C F0097; "
        "M11 action/spell source-frame receipt";
    manifest.ready = manifest.titleReceiptConsumed &&
        manifest.hocReceiptConsumed && manifest.hudReceiptConsumed &&
        manifest.titleFrameCaptured && manifest.hocFrameCaptured &&
        manifest.hudFrameCaptured && manifest.titleFrameHash != manifest.hocFrameHash &&
        manifest.titleFrameHash != manifest.hudFrameHash &&
        manifest.hocFrameHash != manifest.hudFrameHash &&
        manifest.manifestHash != 0u;
    *outManifest = manifest;
    return 1;
}
