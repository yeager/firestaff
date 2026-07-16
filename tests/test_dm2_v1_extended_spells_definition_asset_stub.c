#include "dm2_v1_asset_loader.h"

#include <stddef.h>

static const DM2_V1_GdatEntry* find_entry(const DM2_V1_AssetLoader* loader,
                                          int category,
                                          int index,
                                          int type,
                                          int field)
{
    uint16_t i;

    if (!loader || !loader->loaded || !loader->entries) return NULL;
    if (category < 0 || category > 0xff || index < 0 || index > 0xff ||
        type < 0 || type > 0xff || field < 0 || field > 0xff) {
        return NULL;
    }
    for (i = 0; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry* entry = &loader->entries[i];
        if (entry->cls1 == (uint8_t)category &&
            entry->cls2 == (uint8_t)index &&
            entry->cls3 == (uint8_t)type &&
            entry->cls4 == (uint8_t)field) {
            return entry;
        }
    }
    return NULL;
}

const uint8_t* dm2_v1_asset_load_typed_sized(
    const DM2_V1_AssetLoader* loader,
    int category,
    int index,
    int type,
    int field,
    size_t* out_size)
{
    const DM2_V1_GdatEntry* entry;
    uint16_t raw_index;
    uint32_t offset;
    uint32_t size;

    if (out_size) *out_size = 0u;
    entry = find_entry(loader, category, index, type, field);
    if (!entry || !loader->raw_offsets || !loader->raw_sizes || !loader->data) {
        return NULL;
    }
    raw_index = entry->data_index;
    if (raw_index >= loader->raw_data_count) return NULL;
    offset = loader->raw_offsets[raw_index];
    size = loader->raw_sizes[raw_index];
    if (offset > loader->data_size || size > loader->data_size - offset) {
        return NULL;
    }
    if (out_size) *out_size = size;
    return loader->data + offset;
}

const uint8_t* dm2_v1_asset_load_text_sized(
    const DM2_V1_AssetLoader* loader,
    int category,
    int index,
    int field,
    size_t* out_size)
{
    return dm2_v1_asset_load_typed_sized(loader,
                                         category,
                                         index,
                                         DM2_GDAT_ENTRY_TYPE_TEXT,
                                         field,
                                         out_size);
}

int dm2_v1_asset_load_word_value(const DM2_V1_AssetLoader* loader,
                                 int category,
                                 int index,
                                 int field,
                                 uint16_t* out_value)
{
    const DM2_V1_GdatEntry* entry;

    if (out_value) *out_value = 0u;
    if (!out_value) return 0;
    entry = find_entry(loader,
                       category,
                       index,
                       DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
                       field);
    if (!entry) return 0;
    *out_value = entry->data_index;
    return 1;
}
