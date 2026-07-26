#include "dm1_v1_hoc_presented_frame_consumer_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    DM1_V1_HocPresentedMirrorMaterialPc34 mirror;
    DM1_V1_HocPresentedInscriptionMaterialPc34 inscription;
    DM1_V1_HocPresentedObjectMaterialPc34 object;
    DM1_V1_HocPresentedFrameConsumerInputPc34 input;
    DM1_V1_HocPresentedFrameConsumerReceiptPc34 receipt;
    int ok = 1;

    memset(&mirror, 0, sizeof(mirror));
    memset(&inscription, 0, sizeof(inscription));
    memset(&object, 0, sizeof(object));
    mirror.valid = 1; mirror.backingGraphicIndex = 346; mirror.portraitGraphicIndex = 26;
    mirror.backingWidth = 64; mirror.backingHeight = 64; mirror.portraitWidth = 32; mirror.portraitHeight = 29;
    mirror.backingTransparentColor = 10; mirror.portraitTransparentColor = 10;
    mirror.backingHash = 0x346u; mirror.portraitHash = 0x026u;
    inscription.valid = 1; inscription.fontGraphicIndex = 648; inscription.glyphByteCount = 14;
    inscription.lineCount = 2; inscription.glyphSourceWidth = 8; inscription.glyphSourceHeight = 6;
    inscription.glyphCellCount = 14; inscription.opaqueGlyphPixelCount = 31; inscription.transparentGlyphPixelCount = 53;
    inscription.textDataHash = 1u; inscription.glyphBytesHash = 2u; inscription.fontPixelsHash = 3u; inscription.sourceCellsHash = 4u;
    object.valid = 1; object.graphicsId = 537; object.transparentColor = 10; object.sourceZone = 2;
    object.destinationW = 32; object.destinationH = 20; object.assetWidth = 64; object.assetHeight = 64; object.materialHash = 5u;
    memset(&input, 0, sizeof(input));
    input.runtimeTick = 77u; input.mirror = &mirror; input.inscription = &inscription; input.object = &object;

    ok &= expect(dm1_v1_hoc_presented_frame_consumer_build_receipt_pc34(&input, &receipt) &&
                 receipt.valid && receipt.sourceOwned && receipt.consumedMirror &&
                 receipt.consumedInscription && receipt.consumedObject &&
                 receipt.suppressFallbackVisuals && receipt.frameHash != 0u,
                 "all three real source lanes admit one HoC frame");
    mirror.portraitGraphicIndex = 27;
    ok &= expect(dm1_v1_hoc_presented_frame_consumer_build_receipt_pc34(&input, &receipt) && !receipt.valid,
                 "wrong portrait source rejects the complete frame");
    mirror.portraitGraphicIndex = 26;
    object.materialHash = 0u;
    ok &= expect(dm1_v1_hoc_presented_frame_consumer_build_receipt_pc34(&input, &receipt) && !receipt.valid,
                 "unhashed object source rejects the complete frame");
    object.materialHash = 5u;
    inscription.fontPixelsHash = 0u;
    ok &= expect(dm1_v1_hoc_presented_frame_consumer_build_receipt_pc34(&input, &receipt) && !receipt.valid,
                 "unhashed M648 source rejects the complete frame");
    /* action/spell lane */
    inscription.fontPixelsHash = 3u;
    {
        DM1_V1_HocPresentedActionSpellMaterialPc34 actionSpell;
        memset(&actionSpell, 0, sizeof(actionSpell));
        actionSpell.valid = 1;
        actionSpell.primaryGraphicId = 560;
        actionSpell.fontGraphicId = 557;
        actionSpell.serial = 42u;
        input.actionSpell = &actionSpell;
        ok &= expect(dm1_v1_hoc_presented_frame_consumer_build_receipt_pc34(&input, &receipt) &&
                     receipt.valid && receipt.consumedActionSpell &&
                     receipt.suppressFallbackVisuals,
                     "action/spell lane consumed with valid material");
        actionSpell.primaryGraphicId = 0;
        ok &= expect(dm1_v1_hoc_presented_frame_consumer_build_receipt_pc34(&input, &receipt) &&
                     !receipt.valid,
                     "action/spell with zero graphic rejects frame");
        actionSpell.primaryGraphicId = 560;
        actionSpell.serial = 0u;
        ok &= expect(dm1_v1_hoc_presented_frame_consumer_build_receipt_pc34(&input, &receipt) &&
                     !receipt.valid,
                     "action/spell with zero serial rejects frame");
    }
    if (!ok) return 1;
    puts("ok: DM1 HoC frame consumer admits source-owned mirror/text/object/action-spell material");
    return 0;
}
