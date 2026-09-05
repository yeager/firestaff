#include "dm1_v1_inscription_host_material_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <string.h>

static uint32_t dm1_v1_inscription_fnv1a_bytes_pc34(
    const unsigned char* bytes,
    int byteCount)
{
    uint32_t hash = 2166136261u;
    int i;

    if (!bytes || byteCount <= 0) {
        return 0u;
    }
    for (i = 0; i < byteCount; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int dm1_v1_inscription_packed_span_pc34(
    const unsigned short* textData,
    int textDataWordCount,
    int textDataWordOffset,
    int* outWordCount,
    uint32_t* outFNV1a)
{
    int wordIndex;
    int words = 0;
    uint32_t hash = 2166136261u;

    if (!textData || textDataWordOffset < 0 ||
        textDataWordOffset >= textDataWordCount || !outWordCount ||
        !outFNV1a) {
        return 0;
    }
    for (wordIndex = textDataWordOffset;
         wordIndex < textDataWordCount;
         ++wordIndex) {
        const unsigned short word = textData[wordIndex];
        const int codes[3] = {
            (word >> 10) & 0x1f,
            (word >> 5) & 0x1f,
            word & 0x1f
        };
        int i;

        hash ^= (unsigned char)(word & 0xffu);
        hash *= 16777619u;
        hash ^= (unsigned char)(word >> 8);
        hash *= 16777619u;
        ++words;
        for (i = 0; i < 3; ++i) {
            if (codes[i] == 31) {
                *outWordCount = words;
                *outFNV1a = hash;
                return 1;
            }
        }
    }
    return 0;
}

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

static int dm1_v1_inscription_host_material_from_selected_offset_pc34(
    const struct DungeonThings_Compat* things,
    int textIndex,
    int textDataWordOffset,
    DM1_V1_InscriptionHostMaterialReceiptPc34* outReceipt)
{
    DM1_V1_InscriptionHostMaterialReceiptPc34 receipt;
    int cursor = 0;
    int line;

    if (outReceipt) {
        memset(outReceipt, 0, sizeof(*outReceipt));
    }
    if (!things || !things->textData || things->textDataWordCount <= 0 ||
        !outReceipt || textIndex < 0 || textDataWordOffset < 0) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
            things->textData, things->textDataWordCount,
            textDataWordOffset,
            receipt.glyphBytes, (int)sizeof(receipt.glyphBytes))) {
        return 0;
    }
    if (!dm1_v1_inscription_packed_span_pc34(
            things->textData, things->textDataWordCount, textDataWordOffset,
            &receipt.textDataWordCount, &receipt.textDataFNV1a)) {
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
    {
        int glyph;
        for (glyph = 0; glyph < receipt.glyphByteCount; ++glyph) {
            const unsigned char source = receipt.glyphBytes[glyph];
            char decoded;
            if (source < 26u) decoded = (char)('A' + source);
            else if (source == 26u) decoded = ' ';
            else if (source == 27u) decoded = '.';
            else if (source == 0x80u) decoded = '\n';
            else {
                /* Escape-symbol cells have no stable Unicode spelling.
                 * Their authentic M648 byte path remains the only owner. */
                receipt.sourceText[0] = '\0';
                break;
            }
            receipt.sourceText[glyph] = decoded;
            receipt.sourceText[glyph + 1] = '\0';
        }
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
    receipt.textDataWordOffset = textDataWordOffset;
    receipt.fontGraphicIndex = DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34;
    receipt.transparentColor = DM1_V1_INSCRIPTION_TRANSPARENT_COLOR;
    receipt.glyphBytesFNV1a = dm1_v1_inscription_fnv1a_bytes_pc34(
        receipt.glyphBytes, receipt.glyphByteCount + 1);
    if (receipt.glyphBytesFNV1a == 0u) {
        return 0;
    }
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_inscription_host_material_from_selected_wall_pc34(
    const struct DungeonThings_Compat* things,
    int selectedTextIndex,
    DM1_V1_InscriptionHostMaterialReceiptPc34* outReceipt)
{
    int textDataWordOffset;

    if (outReceipt) {
        memset(outReceipt, 0, sizeof(*outReceipt));
    }
    /* ReDMCSB DUNGEON.C F0172 publishes one current G0290 TextString for
     * the visible wall.  F0107 consumes that exact record; it must not scan
     * an unrelated list member when the selected raw record is unavailable. */
    if (!dm1_v1_inscription_get_visible_raw_text_offset_pc34(
            things, selectedTextIndex, &textDataWordOffset)) {
        return 0;
    }
    return dm1_v1_inscription_host_material_from_selected_offset_pc34(
        things, selectedTextIndex, textDataWordOffset, outReceipt);
}

int dm1_v1_inscription_host_material_from_world_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    DM1_V1_InscriptionHostMaterialReceiptPc34* outReceipt)
{
    int textIndex;
    int textDataWordOffset;

    if (outReceipt) {
        memset(outReceipt, 0, sizeof(*outReceipt));
    }
    if (!outReceipt) {
        return 0;
    }
    textIndex = dm1_v1_inscription_visible_text_index_pc34(
        things, preferredTextIndex, firstThing, &textDataWordOffset);
    if (textIndex < 0) {
        return 0;
    }
    return dm1_v1_inscription_host_material_from_selected_offset_pc34(
        things, textIndex, textDataWordOffset, outReceipt);
}
