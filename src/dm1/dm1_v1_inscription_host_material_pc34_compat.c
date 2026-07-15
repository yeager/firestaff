#include "dm1_v1_inscription_host_material_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <string.h>

int dm1_v1_inscription_get_visible_raw_text_offset_pc34(
    const struct DungeonThings_Compat* things,
    int textStringIndex,
    int* outTextDataWordOffset)
{
    const unsigned char* raw;
    unsigned short textData;
    unsigned short thing;

    if (!things || !things->textData || things->textDataWordCount <= 0 ||
        !outTextDataWordOffset || textStringIndex < 0 ||
        textStringIndex > 0x03FF) {
        return 0;
    }

    thing = (unsigned short)(((unsigned int)THING_TYPE_TEXTSTRING << 10) |
                             (unsigned int)textStringIndex);
    raw = dm1_v1_dungeon_get_thing_data_pc34(things, thing);
    if (!raw) {
        return 0;
    }

    /* TEXTSTRING.Type_Data: Visible bit 0, TextDataWordOffset bits 15:3.
     * DUNGEON.C F0168 reads this G0284 record directly. */
    textData = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
    if (!(textData & 0x0001u)) {
        return 0;
    }
    textData = (unsigned short)((textData >> 3) & 0x1FFFu);
    if (textData >= (unsigned short)things->textDataWordCount) {
        return 0;
    }

    *outTextDataWordOffset = (int)textData;
    return 1;
}

static int dm1_v1_inscription_visible_text_index_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    int* outTextDataWordOffset)
{
    unsigned short thing;
    int safety = 0;
    int textDataWordOffset;
    if (!things || !outTextDataWordOffset) {
        return -1;
    }
    if (dm1_v1_inscription_get_visible_raw_text_offset_pc34(
            things, preferredTextIndex, &textDataWordOffset)) {
        *outTextDataWordOffset = textDataWordOffset;
        return preferredTextIndex;
    }
    thing = firstThing;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_TEXTSTRING) {
            int index = (int)THING_GET_INDEX(thing);
            if (dm1_v1_inscription_get_visible_raw_text_offset_pc34(
                    things, index, &textDataWordOffset)) {
                *outTextDataWordOffset = textDataWordOffset;
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
    int textDataWordOffset;
    int cursor = 0;
    int line;

    if (!outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    textIndex = dm1_v1_inscription_visible_text_index_pc34(
        things, preferredTextIndex, firstThing, &textDataWordOffset);
    if (textIndex < 0 || !DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
            things->textData, things->textDataWordCount,
            textDataWordOffset,
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
