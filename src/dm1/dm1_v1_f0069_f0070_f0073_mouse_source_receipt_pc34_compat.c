#include "dm1_v1_f0069_f0070_f0073_mouse_source_receipt_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0069_f0070_f0073_fnv1a_pc34(
    const uint8_t *bytes, size_t byteCount)
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

static int dm1_v1_f0070_c028_is_authenticated_pc34(
    const DM1_V1_F0069F0070SourceMaterialPc34 *material,
    uint32_t *outHash)
{
    uint32_t hash;
    size_t required = (size_t)DM1_V1_F0070_C028_WIDTH_PC34 *
                      DM1_V1_F0070_C028_HEIGHT_PC34;

    if (outHash) *outHash = 0U;
    if (!material || !material->graphicsDatAuthenticated ||
        material->graphicIndex != DM1_V1_F0070_C028_GRAPHIC_PC34 ||
        material->width != DM1_V1_F0070_C028_WIDTH_PC34 ||
        material->height != DM1_V1_F0070_C028_HEIGHT_PC34 ||
        !material->indexedPixels || material->indexedPixelCount != required) {
        return 0;
    }
    hash = dm1_v1_f0069_f0070_f0073_fnv1a_pc34(
        material->indexedPixels, material->indexedPixelCount);
    if (hash == 0U || hash != material->indexedPixelsFnv1a) return 0;
    if (outHash) *outHash = hash;
    return 1;
}

int dm1_v1_f0069_f0070_f0073_mouse_runtime_receipt_pc34(
    const DM1_V1_F0069F0070RuntimeInputPc34 *input,
    DM1_V1_F0069F0070F0073ReceiptPc34 *outReceipt)
{
    uint32_t c028Hash;
    int targetCell;
    int championIndex;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->suppressSyntheticUi = 1;
    outReceipt->f0073Pc34ScreenAreaUnavailable = 1;
    outReceipt->f0073SyntheticScreenAreaSuppressed = 1;
    outReceipt->sourceEvidence = dm1_v1_f0069_f0070_f0073_source_evidence_pc34();
    if (!input || !dm1_v1_f0070_c028_is_authenticated_pc34(
                      &input->championIcons, &c028Hash) ||
        input->partyDirection < 0 || input->partyDirection >= 4 ||
        input->targetChampionIconIndex < 0 ||
        input->targetChampionIconIndex >= DM1_V1_F0070_CHAMPION_ICON_COUNT_PC34) {
        return 0;
    }

    if (input->leaderEmptyHanded) {
        outReceipt->f0069Pointer = input->leaderIndex < 0
            ? DM1_V1_F0069_POINTER_ARROW_PC34
            : DM1_V1_F0069_POINTER_HAND_PC34;
    } else {
        if (!input->leaderObjectPointerPixels ||
            input->leaderObjectPointerPixelCount == 0U ||
            dm1_v1_f0069_f0070_f0073_fnv1a_pc34(
                input->leaderObjectPointerPixels,
                input->leaderObjectPointerPixelCount) == 0U ||
            dm1_v1_f0069_f0070_f0073_fnv1a_pc34(
                input->leaderObjectPointerPixels,
                input->leaderObjectPointerPixelCount) != input->leaderObjectPointerFnv1a) {
            return 0;
        }
        outReceipt->f0069Pointer = DM1_V1_F0069_POINTER_OBJECT_PC34;
    }

    targetCell = (input->targetChampionIconIndex + input->partyDirection) & 3;
    championIndex = input->championIndexByCell[targetCell];
    if (input->heldChampionIconOrdinal == 0U) {
        if (championIndex < 0) return 0;
        outReceipt->f0070PickedUp = 1;
        outReceipt->nextHeldChampionIconOrdinal =
            (unsigned int)input->targetChampionIconIndex + 1U;
        outReceipt->f0069Pointer = DM1_V1_F0069_POINTER_CHAMPION_PC34;
    } else {
        unsigned int heldIndex = input->heldChampionIconOrdinal - 1U;
        if (heldIndex >= DM1_V1_F0070_CHAMPION_ICON_COUNT_PC34) return 0;
        outReceipt->f0070Dropped = 1;
        outReceipt->nextHeldChampionIconOrdinal = 0U;
    }

    outReceipt->valid = 1;
    outReceipt->f0070TargetIconIndex = input->targetChampionIconIndex;
    outReceipt->f0070ChampionIndex = championIndex;
    outReceipt->c028MaterialFnv1a = c028Hash;
    return 1;
}

const char *dm1_v1_f0069_f0070_f0073_source_evidence_pc34(void)
{
    return "ReDMCSB MOUSESET.C:5-16 F0069 selects arrow/hand/object from "
           "G0415/G0411/G4055; IO.C:2395-2647 F0070 validates the party "
           "cell, consumes C028 champion icons and toggles ordinal state. "
           "IO.C:2741 F0073 saved-screen bitmap compositor is MEDIA007 only, "
           "not I34E/I34M, so PC34 suppresses a synthetic screen-area buffer.";
}
