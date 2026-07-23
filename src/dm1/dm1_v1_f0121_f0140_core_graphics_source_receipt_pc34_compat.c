#include "dm1_v1_f0121_f0140_core_graphics_source_receipt_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0121_f0140_fnv1a_pc34(const uint8_t *bytes, size_t byteCount)
{
    uint32_t hash = 2166136261u;
    size_t index;
    if (!bytes || byteCount == 0U) return 0U;
    for (index = 0U; index < byteCount; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static int dm1_v1_f0121_f0140_source_ok(
    const DM1_V1_F0121F0140RawSourcePc34 *source, uint32_t *outHash)
{
    uint32_t hash;
    if (outHash) *outHash = 0U;
    if (!source || !source->authenticated || !source->bytes ||
        source->byteCount == 0U) return 0;
    hash = dm1_v1_f0121_f0140_fnv1a_pc34(source->bytes, source->byteCount);
    if (hash == 0U || hash != source->fnv1a) return 0;
    if (outHash) *outHash = hash;
    return 1;
}

int dm1_v1_f0121_f0140_core_graphics_source_receipt_pc34(
    const DM1_V1_F0121F0140RuntimeInputPc34 *input,
    DM1_V1_F0121F0140ReceiptPc34 *outReceipt)
{
    uint32_t graphicsHash, dungeonHash, paletteHash, sourceHash, destinationHash;
    uint32_t fingerprint;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->suppressSyntheticPresentation = 1;
    outReceipt->f0137CpsfUnavailableOnPc34 = 1;
    outReceipt->f0139DelegatedToExistingOwner = 1;
    outReceipt->f0140DelegatedToExistingOwner = 1;
    outReceipt->sourceEvidence = dm1_v1_f0121_f0140_source_evidence_pc34();
    if (!input || !input->viewportOwned ||
        input->viewportWidth != DM1_V1_F0121_F0140_VIEWPORT_WIDTH_PC34 ||
        input->viewportHeight != DM1_V1_F0121_F0140_VIEWPORT_HEIGHT_PC34 ||
        input->viewportByteWidth != DM1_V1_F0121_F0140_VIEWPORT_BYTE_WIDTH_PC34 ||
        input->square < DM1_V1_F0121_F0140_D2C_PC34 ||
        input->square > DM1_V1_F0121_F0140_D0C_PC34 ||
        input->sourceWidth <= 0 || input->sourceHeight <= 0 ||
        input->destinationWidth <= 0 || input->destinationHeight <= 0 ||
        input->transparentColor < -1 || input->transparentColor > 15 ||
        !dm1_v1_f0121_f0140_source_ok(&input->graphicsDat, &graphicsHash) ||
        !dm1_v1_f0121_f0140_source_ok(&input->dungeonDat, &dungeonHash) ||
        !dm1_v1_f0121_f0140_source_ok(&input->paletteChanges, &paletteHash) ||
        !dm1_v1_f0121_f0140_source_ok(&input->sourceSurface, &sourceHash) ||
        !dm1_v1_f0121_f0140_source_ok(&input->destinationSurface, &destinationHash)) {
        return 0;
    }
    fingerprint = 2166136261u;
    fingerprint ^= graphicsHash; fingerprint *= 16777619u;
    fingerprint ^= dungeonHash; fingerprint *= 16777619u;
    fingerprint ^= paletteHash; fingerprint *= 16777619u;
    fingerprint ^= sourceHash; fingerprint *= 16777619u;
    fingerprint ^= destinationHash; fingerprint *= 16777619u;
    if (fingerprint == 0U) return 0;

    outReceipt->valid = 1;
    outReceipt->f0121ToF0127SquareAdmitted = 1;
    outReceipt->f0128ViewportOwnerAdmitted = 1;
    outReceipt->f0129ShrinkPaletteAdmitted = 1;
    outReceipt->f0130HorizontalFlipAdmitted = input->flipHorizontal ? 1 : 0;
    outReceipt->f0131VerticalFlipAdmitted = input->flipVertical ? 1 : 0;
    outReceipt->f0132BlitAdmitted = 1;
    outReceipt->f0133MaskedBlitAdmitted = input->transparentColor >= 0;
    outReceipt->f0136HatchAdmitted = 1;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}

const char *dm1_v1_f0121_f0140_source_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C:7244-8309 F0121-F0127 square passes; "
           "DUNVIEW.C:8318-8542 F0128 dispatch; BLTSHRNK.C:202 F0129; "
           "FLIPHORI.C:12 F0130; FLIPVERT.C:12 F0131; BLIT.C:31 F0132; "
           "BLITMASK.C:33 F0133; BLITFILL.C:252 F0136. DUNVIEW.C:6803 "
           "F0137 patches CPSF hidden code and is unavailable on PC34. "
           "DUNGEON.C:1050 F0139 and 1082 F0140 keep their existing owners.";
}
