#include "dm1_v1_hoc_presented_frame_consumer_pc34_compat.h"

#include <string.h>

static uint32_t mix(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static int mirror_valid(const DM1_V1_HocPresentedMirrorMaterialPc34 *mirror)
{
    return !mirror || !mirror->valid ||
        (mirror->backingGraphicIndex == 346 && mirror->portraitGraphicIndex == 26 &&
         mirror->backingWidth > 0 && mirror->backingHeight > 0 &&
         mirror->portraitWidth > 0 && mirror->portraitHeight > 0 &&
         mirror->backingTransparentColor == 10 && mirror->portraitTransparentColor == 10 &&
         mirror->backingHash != 0u && mirror->portraitHash != 0u);
}

static int inscription_valid(const DM1_V1_HocPresentedInscriptionMaterialPc34 *inscription)
{
    return !inscription || !inscription->valid ||
        (inscription->fontGraphicIndex == 648 && inscription->glyphByteCount > 0 &&
         inscription->lineCount > 0 && inscription->glyphSourceWidth > 0 &&
         inscription->glyphSourceHeight > 0 && inscription->glyphCellCount > 0 &&
         inscription->opaqueGlyphPixelCount > 0 &&
         inscription->transparentGlyphPixelCount > 0 &&
         inscription->textDataHash != 0u && inscription->glyphBytesHash != 0u &&
         inscription->fontPixelsHash != 0u && inscription->sourceCellsHash != 0u);
}

static int object_valid(const DM1_V1_HocPresentedObjectMaterialPc34 *object)
{
    return !object || !object->valid ||
        (object->graphicsId > 0 && object->transparentColor == 10 &&
         object->sourceZone >= 0 && object->destinationW > 0 &&
         object->destinationH > 0 && object->assetWidth > 0 &&
         object->assetHeight > 0 && object->materialHash != 0u);
}

static int action_spell_valid(const DM1_V1_HocPresentedActionSpellMaterialPc34 *as)
{
    if (!as || !as->valid) return 1;
    if (as->primaryGraphicId <= 0) return 0;
    if (as->fontGraphicId <= 0) return 0;
    if (as->serial == 0u) return 0;
    return 1;
}

static int palette_valid(const DM1_V1_HocPresentedPaletteMaterialPc34 *p)
{
    if (!p || !p->valid) return 1;
    if (p->entryCount != 16) return 0;
    if (p->paletteHash == 0u) return 0;
    return 1;
}

int dm1_v1_hoc_presented_frame_consumer_build_receipt_pc34(
    const DM1_V1_HocPresentedFrameConsumerInputPc34 *input,
    DM1_V1_HocPresentedFrameConsumerReceiptPc34 *outReceipt)
{
    DM1_V1_HocPresentedFrameConsumerReceiptPc34 receipt;
    uint32_t hash = 2166136261u;

    if (!input || !outReceipt || input->runtimeTick == 0u) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB DUNVIEW.C F0107/F0115: C127 C346/C026, M648 and F0115 "
        "materials are admitted together only after their original PC34 blits";
    if (!mirror_valid(input->mirror) || !inscription_valid(input->inscription) ||
        !object_valid(input->object) || !action_spell_valid(input->actionSpell) ||
        !palette_valid(input->palette)) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.runtimeTick = input->runtimeTick;
    receipt.consumedMirror = input->mirror && input->mirror->valid;
    receipt.consumedInscription = input->inscription && input->inscription->valid;
    receipt.consumedObject = input->object && input->object->valid;
    receipt.consumedActionSpell = input->actionSpell && input->actionSpell->valid;
    receipt.consumedPalette = input->palette && input->palette->valid;
    if (!receipt.consumedMirror && !receipt.consumedInscription &&
        !receipt.consumedObject && !receipt.consumedActionSpell &&
        !receipt.consumedPalette) {
        *outReceipt = receipt;
        return 1;
    }
    hash = mix(hash, input->runtimeTick);
    if (receipt.consumedMirror) {
        hash = mix(hash, input->mirror->backingHash);
        hash = mix(hash, input->mirror->portraitHash);
    }
    if (receipt.consumedInscription) {
        hash = mix(hash, input->inscription->textDataHash);
        hash = mix(hash, input->inscription->glyphBytesHash);
        hash = mix(hash, input->inscription->fontPixelsHash);
        hash = mix(hash, input->inscription->sourceCellsHash);
    }
    if (receipt.consumedObject) {
        hash = mix(hash, input->object->materialHash);
    }
    if (receipt.consumedActionSpell) {
        hash = mix(hash, input->actionSpell->serial);
        hash = mix(hash, (uint32_t)input->actionSpell->primaryGraphicId);
        hash = mix(hash, (uint32_t)input->actionSpell->fontGraphicId);
    }
    if (receipt.consumedPalette) {
        hash = mix(hash, input->palette->paletteHash);
    }
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.suppressFallbackVisuals = 1;
    receipt.frameHash = hash;
    *outReceipt = receipt;
    return 1;
}

const char *dm1_v1_hoc_presented_frame_consumer_source_evidence_pc34(void)
{
    return "C127 mirror, M648 inscription, and F0115 object lanes are independently "
           "source-locked and fail closed before their HoC frame is consumed.";
}
