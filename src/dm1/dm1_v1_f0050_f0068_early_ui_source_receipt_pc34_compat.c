#include "dm1_v1_f0050_f0068_early_ui_source_receipt_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0050_f0068_fnv1a_pc34(const uint8_t *bytes, size_t byteCount)
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

static int dm1_v1_f0050_f0068_material_ok(
    const DM1_V1_F0050F0068RawMaterialPc34 *material,
    int graphicIndexA, int graphicIndexB, int width, int height,
    size_t exactByteCount, uint32_t *outHash)
{
    uint32_t hash;

    if (outHash) *outHash = 0U;
    if (!material || !material->sourceAuthenticated ||
        (material->graphicIndex != graphicIndexA &&
         material->graphicIndex != graphicIndexB) ||
        material->width != width || material->height != height ||
        !material->bytes ||
        (exactByteCount != 0U && material->byteCount != exactByteCount) ||
        (exactByteCount == 0U && material->byteCount == 0U)) {
        return 0;
    }
    hash = dm1_v1_f0050_f0068_fnv1a_pc34(material->bytes, material->byteCount);
    if (hash == 0U || hash != material->fnv1a) return 0;
    if (outHash) *outHash = hash;
    return 1;
}

int dm1_v1_f0050_f0068_early_ui_source_receipt_pc34(
    const DM1_V1_F0050F0068RuntimeInputPc34 *input,
    DM1_V1_F0050F0068ReceiptPc34 *outReceipt)
{
    uint32_t fontHash;
    uint32_t arrowHash;
    uint32_t handHash;
    uint32_t fingerprint;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->suppressSyntheticUi = 1;
    outReceipt->sourceEvidence = dm1_v1_f0050_f0068_source_evidence_pc34();
    if (!input || !input->viewportBitmapOwned ||
        input->viewportWidth != DM1_V1_F0052_VIEWPORT_WIDTH_PC34 ||
        input->viewportHeight != DM1_V1_F0052_VIEWPORT_HEIGHT_PC34 ||
        !input->logicalScreenBitmapOwned ||
        input->logicalScreenWidth != DM1_V1_F0053_SCREEN_WIDTH_PC34 ||
        input->logicalScreenHeight != DM1_V1_F0053_SCREEN_HEIGHT_PC34 ||
        !input->ioDriverPointerRegistrationBound ||
        !input->ioDriverMouseHandlerBound ||
        !input->ioDriverScreenRegionBound ||
        !dm1_v1_f0050_f0068_material_ok(&input->interfaceFont,
            DM1_V1_F0054_M653_GRAPHIC_PC34,
            DM1_V1_F0054_M653_GRAPHIC_LEGACY_PC34, 1024, 6,
            DM1_V1_F0054_M653_BYTE_COUNT_PC34, &fontHash) ||
        !dm1_v1_f0050_f0068_material_ok(&input->arrowPointer, 42, -1,
            DM1_V1_F0066_ARROW_POINTER_WIDTH_PC34,
            DM1_V1_F0066_ARROW_POINTER_HEIGHT_PC34, 0U, &arrowHash) ||
        !dm1_v1_f0050_f0068_material_ok(&input->handPointer, 43, -1,
            DM1_V1_F0066_HAND_POINTER_WIDTH_PC34,
            DM1_V1_F0066_HAND_POINTER_HEIGHT_PC34, 0U, &handHash)) {
        return 0;
    }

    /* Pointer bitmaps are platform tables, not GRAPHICS.DAT records.  The
     * no-fixed-byte-count branch above still requires non-empty, hashed data. */
    fingerprint = 2166136261u;
    fingerprint ^= fontHash; fingerprint *= 16777619u;
    fingerprint ^= arrowHash; fingerprint *= 16777619u;
    fingerprint ^= handHash; fingerprint *= 16777619u;
    if (fingerprint == 0U) return 0;

    outReceipt->valid = 1;
    outReceipt->f0050SpaceUsesM653 = 1;
    outReceipt->f0051LineFeedUsesM653 = 1;
    outReceipt->f0052ViewportTextAdmitted = 1;
    outReceipt->f0053LogicalScreenTextAdmitted = 1;
    outReceipt->f0054InitializeFontAdmitted = 1;
    outReceipt->f0066MouseInitializeAdmitted = 1;
    outReceipt->f0067NormalPointerAdmitted = 1;
    outReceipt->f0068ObjectPointerAdmitted = 1;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}

const char *dm1_v1_f0050_f0068_source_evidence_pc34(void)
{
    return "ReDMCSB TEXT.C:1863-2027 F0050-F0054: M653 one-bit "
           "interface font, C112xC136 viewport, C160xC200 screen; "
           "IO.C:1343-1405 F0066 registers G0042 arrow/G0043 hand with "
           "IODRV on I34E/I34M; IO.C:1826-2150 F0067/F0068 select normal "
           "or object pointers. SOUND.C F0060-F0065 remains a distinct "
           "authentic-audio owner and is intentionally not duplicated.";
}
