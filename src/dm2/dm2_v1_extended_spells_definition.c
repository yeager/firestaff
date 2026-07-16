#include "dm2_v1_extended_spells_definition.h"

#include <stdio.h>
#include <string.h>

static uint32_t dm2_ext_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static int dm2_ext_word_u8(const DM2_V1_AssetLoader* loader,
                           int index,
                           int field,
                           uint8_t* outValue,
                           int* outMissing,
                           int* outWide)
{
    uint16_t word = 0u;
    if (outValue) *outValue = 0u;
    if (!dm2_v1_asset_load_word_value(loader,
                                      DM2_GDAT_CATEGORY_SPELL_DEF,
                                      index,
                                      field,
                                      &word)) {
        if (outMissing) *outMissing = 1;
        return 0;
    }
    if (word > 0xffu) {
        if (outWide) *outWide = 1;
        return 0;
    }
    if (outValue) *outValue = (uint8_t)word;
    return 1;
}

static uint32_t dm2_ext_copy_text(const DM2_V1_AssetLoader* loader,
                                  int index,
                                  char* out,
                                  size_t outSize,
                                  int* outLoaded)
{
    const uint8_t* text;
    size_t textSize = 0u;
    size_t i;
    size_t n;
    uint32_t hash = 2166136261u;

    if (outLoaded) *outLoaded = 0;
    if (out && outSize > 0u) out[0] = '\0';
    text = dm2_v1_asset_load_typed_sized(loader,
                                         DM2_GDAT_CATEGORY_SPELL_DEF,
                                         index,
                                         DM2_GDAT_ENTRY_TYPE_TEXT,
                                         0x18,
                                         &textSize);
    if (!text || textSize == 0u) {
        return 0u;
    }
    if (outLoaded) *outLoaded = 1;
    n = textSize;
    if (out && outSize > 0u && n >= outSize) {
        n = outSize - 1u;
    }
    for (i = 0u; i < textSize; ++i) {
        hash = dm2_ext_hash_step(hash, text[i]);
    }
    if (out && outSize > 0u) {
        for (i = 0u; i < n && text[i] != '\0'; ++i) {
            out[i] = (char)text[i];
        }
        out[i] = '\0';
    }
    return hash;
}

int dm2_v1_extended_load_spells_definition(
    const DM2_V1_AssetLoader* loader,
    int extendedMode,
    const uint16_t* originalW6,
    size_t originalCount,
    DM2_V1_ExtendedSpellsReceipt* outReceipt)
{
    DM2_V1_ExtendedSpellsReceipt local;
    DM2_V1_ExtendedSpellsReceipt* receipt = outReceipt ? outReceipt : &local;
    uint32_t hash = 2166136261u;
    int index;
    size_t originalLimit;

    memset(receipt, 0, sizeof(*receipt));
    receipt->handled = 1;
    receipt->extendedMode = extendedMode ? 1 : 0;
    receipt->firstLoadedIndex = -1;
    receipt->lastLoadedIndex = -1;
    if (!extendedMode) {
        return 0;
    }
    if (!loader || !loader->loaded) {
        receipt->failClosed = 1;
        return 0;
    }

    for (index = 0; index < DM2_V1_EXT_SPELLS_CUSTOM_LOAD_COUNT; ++index) {
        DM2_V1_ExtendedSpellDefinition* spell;
        uint8_t rune1 = 0u;
        uint8_t rune2 = 0u;
        uint8_t rune3 = 0u;
        uint8_t difficulty = 0u;
        uint8_t requiredSkill = 0u;
        uint8_t type = 0u;
        uint8_t result = 0u;
        int missing = 0;
        int wide = 0;
        int textLoaded = 0;

        if (!dm2_ext_word_u8(loader, index, 0x01, &rune1, &missing, &wide)) {
            if (wide) {
                receipt->wideRequiredField = 1;
                receipt->failClosed = 1;
                return 0;
            }
            ++receipt->customSkippedCount;
            continue;
        }
        if (rune1 == 0u) {
            ++receipt->customSkippedCount;
            continue;
        }
        if (!dm2_ext_word_u8(loader, index, 0x02, &rune2, &missing, &wide) ||
            !dm2_ext_word_u8(loader, index, 0x03, &rune3, &missing, &wide) ||
            !dm2_ext_word_u8(loader, index, 0x04, &difficulty, &missing, &wide) ||
            !dm2_ext_word_u8(loader, index, 0x05, &requiredSkill, &missing, &wide) ||
            !dm2_ext_word_u8(loader, index, 0x06, &type, &missing, &wide) ||
            !dm2_ext_word_u8(loader, index, 0x07, &result, &missing, &wide)) {
            receipt->missingRequiredField = missing ? 1 : 0;
            receipt->wideRequiredField = wide ? 1 : 0;
            receipt->failClosed = 1;
            return 0;
        }

        spell = &receipt->custom[index];
        spell->loaded = 1u;
        spell->rune1 = rune1;
        spell->rune2 = rune2;
        spell->rune3 = rune3;
        spell->difficulty = difficulty;
        spell->requiredSkill = requiredSkill;
        spell->type = type;
        spell->result = result;
        spell->dw0 = ((uint32_t)rune3) |
                     ((uint32_t)rune2 << 8) |
                     ((uint32_t)rune1 << 16);
        spell->w6 = (uint16_t)((type & 0x0fu) |
                               ((uint16_t)(result & 0x3fu) << 4));
        spell->spellValue = result;
        spell->textHash = dm2_ext_copy_text(loader,
                                            index,
                                            spell->name,
                                            sizeof(spell->name),
                                            &textLoaded);
        spell->textLoaded = textLoaded ? 1u : 0u;

        if (receipt->firstLoadedIndex < 0) {
            receipt->firstLoadedIndex = index;
        }
        receipt->lastLoadedIndex = index;
        ++receipt->customLoadedCount;
        hash = dm2_ext_hash_step(hash, (uint32_t)index);
        hash = dm2_ext_hash_step(hash, spell->dw0);
        hash = dm2_ext_hash_step(hash, spell->difficulty);
        hash = dm2_ext_hash_step(hash, spell->requiredSkill);
        hash = dm2_ext_hash_step(hash, spell->w6);
        hash = dm2_ext_hash_step(hash, spell->spellValue);
        hash = dm2_ext_hash_step(hash, spell->textHash);
    }

    originalLimit = originalCount;
    if (originalLimit > DM2_V1_EXT_SPELLS_ORIGINAL_ADAPT_COUNT) {
        originalLimit = DM2_V1_EXT_SPELLS_ORIGINAL_ADAPT_COUNT;
    }
    for (index = 0; index < (int)originalLimit; ++index) {
        uint8_t spellValue = (uint8_t)((originalW6[index] >> 4) & 0x3fu);
        receipt->originalSpellValue[index] = spellValue;
        ++receipt->originalAdaptedCount;
        hash = dm2_ext_hash_step(hash, (uint32_t)spellValue);
    }

    receipt->loaded = 1;
    receipt->tableHash = hash;
    return 1;
}

const char* dm2_v1_extended_load_spells_definition_source_evidence(void)
{
    return
        "skproject/SKWIN/SkWinCore.cpp:189 EXTENDED_LOAD_SPELLS_DEFINITION\n"
        "skproject/SKWIN/SkGlobal.h:41-58 MAXSPELL_ORIGINAL/MAXSPELL_CUSTOM/MkssymVal\n"
        "skproject/SKWIN/DME.h:2113-2133 SpellDefinition/spellValue";
}
