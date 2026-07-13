#include "dm1_v1_inscription_host_material_pc34_compat.h"

#include <string.h>

static int dm1_v1_inscription_visible_text_index_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing)
{
    unsigned short thing;
    int safety = 0;
    if (!things || !things->textStrings || !things->textData ||
        things->textDataWordCount <= 0) {
        return -1;
    }
    if (preferredTextIndex >= 0 &&
        preferredTextIndex < things->textStringCount &&
        things->textStrings[preferredTextIndex].visible) {
        return preferredTextIndex;
    }
    thing = firstThing;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_TEXTSTRING) {
            int index = (int)THING_GET_INDEX(thing);
            if (index >= 0 && index < things->textStringCount &&
                things->textStrings[index].visible) {
                return index;
            }
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
    }
    return -1;
}

int dm1_v1_inscription_host_material_from_world_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    DM1_V1_InscriptionHostMaterialReceiptPc34* outReceipt)
{
    DM1_V1_InscriptionHostMaterialReceiptPc34 receipt;
    int textIndex;
    int cursor = 0;
    int line;

    if (!outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    textIndex = dm1_v1_inscription_visible_text_index_pc34(
        things, preferredTextIndex, firstThing);
    if (textIndex < 0 || !DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
            things->textData, things->textDataWordCount,
            (int)things->textStrings[textIndex].textDataWordOffset,
            receipt.glyphBytes, (int)sizeof(receipt.glyphBytes))) {
        return 0;
    }
    while (receipt.glyphByteCount < (int)sizeof(receipt.glyphBytes) &&
           receipt.glyphBytes[receipt.glyphByteCount] != 0x81U) {
        ++receipt.glyphByteCount;
    }
    if (receipt.glyphByteCount <= 0 ||
        receipt.glyphByteCount >= (int)sizeof(receipt.glyphBytes)) {
        return 0;
    }
    /* ReDMCSB DUNGEON.C F0168 decodes the selected TextString; DUNVIEW.C
     * F0107:3619-3706 owns M648, line position, C10 transparency, and the
     * raw byte << 3 source-cell selection. Fail closed on malformed bytes. */
    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        DM1_V1_InscriptionFrontWallLineDrawPlanPc34* plan =
            &receipt.lines[line];
        if (!DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
                receipt.glyphBytes, (int)sizeof(receipt.glyphBytes), cursor,
                line, 160, 111, plan)) {
            return 0;
        }
        if (plan->glyphCount > 0) {
            ++receipt.lineCount;
        }
        if (plan->done) {
            break;
        }
        cursor = plan->nextCursor;
    }
    if (receipt.lineCount <= 0) {
        return 0;
    }
    receipt.valid = 1;
    receipt.textStringIndex = textIndex;
    receipt.fontGraphicIndex = DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34;
    receipt.transparentColor = DM1_V1_INSCRIPTION_TRANSPARENT_COLOR;
    *outReceipt = receipt;
    return 1;
}
