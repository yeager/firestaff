#ifndef FIRESTAFF_DM2_V1_EXTENDED_SPELLS_DEFINITION_H
#define FIRESTAFF_DM2_V1_EXTENDED_SPELLS_DEFINITION_H

#include "dm2_v1_asset_loader.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM2_V1_EXT_SPELLS_CUSTOM_COUNT = 255,
    DM2_V1_EXT_SPELLS_CUSTOM_LOAD_COUNT = 254,
    DM2_V1_EXT_SPELLS_ORIGINAL_COUNT = 34,
    DM2_V1_EXT_SPELLS_ORIGINAL_ADAPT_COUNT = 33,
    DM2_V1_EXT_SPELLS_NAME_CAPACITY = 128
};

typedef struct {
    uint32_t dw0;
    uint8_t difficulty;
    uint8_t requiredSkill;
    uint16_t w6;
    uint8_t spellValue;
    uint8_t rune1;
    uint8_t rune2;
    uint8_t rune3;
    uint8_t type;
    uint8_t result;
    uint8_t loaded;
    uint8_t textLoaded;
    uint32_t textHash;
    char name[DM2_V1_EXT_SPELLS_NAME_CAPACITY];
} DM2_V1_ExtendedSpellDefinition;

typedef struct {
    int handled;
    int extendedMode;
    int loaded;
    int failClosed;
    int missingRequiredField;
    int wideRequiredField;
    int customLoadedCount;
    int customSkippedCount;
    int originalAdaptedCount;
    int firstLoadedIndex;
    int lastLoadedIndex;
    uint32_t tableHash;
    DM2_V1_ExtendedSpellDefinition custom[DM2_V1_EXT_SPELLS_CUSTOM_COUNT];
    uint8_t originalSpellValue[DM2_V1_EXT_SPELLS_ORIGINAL_COUNT];
} DM2_V1_ExtendedSpellsReceipt;

int dm2_v1_extended_load_spells_definition(
    const DM2_V1_AssetLoader* loader,
    int extendedMode,
    const uint16_t* originalW6,
    size_t originalCount,
    DM2_V1_ExtendedSpellsReceipt* outReceipt);

const char* dm2_v1_extended_load_spells_definition_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_EXTENDED_SPELLS_DEFINITION_H */
