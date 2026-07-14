#include "redmcsb_f0694_set_multiple_colors_in_palette_pc34_compat.h"

int redmcsb_f0694_set_multiple_colors_in_palette_pc34_compat(
    const redmcsb_f0694_palette_definition_pc34_compat *palette_table,
    size_t palette_table_count,
    int16_t palette_index,
    uint8_t full_palette[32][3],
    int curtain_flag,
    redmcsb_f0694_palette_upload_pc34_compat upload,
    void *upload_context)
{
    const redmcsb_f0694_palette_definition_pc34_compat *definition;
    size_t entry_index;

    if (palette_table == NULL || full_palette == NULL || palette_index < 0 ||
        (size_t)palette_index >= palette_table_count) {
        return 0;
    }

    definition = &palette_table[(size_t)palette_index];
    if (definition->entries == NULL) {
        return 0;
    }

    for (entry_index = 0U; entry_index < definition->entry_count;
         ++entry_index) {
        const redmcsb_f0694_palette_entry_pc34_compat *entry =
            &definition->entries[entry_index];

        if (entry->index < 0) {
            if (curtain_flag == 1 && upload != NULL) {
                upload(upload_context, full_palette);
            }
            return 1;
        }
        if (entry->index < 32) {
            full_palette[(size_t)entry->index][0] = entry->red;
            full_palette[(size_t)entry->index][1] = entry->green;
            full_palette[(size_t)entry->index][2] = entry->blue;
        }
    }

    return 0;
}
