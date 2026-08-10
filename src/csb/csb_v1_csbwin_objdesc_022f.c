#include "csb_v1_csbwin_objdesc_022f.h"

#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

static uint16_t csb_v1_csbwin_objdesc_022f_read_u16be(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

int csb_v1_csbwin_objdesc_022f_decode(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_CSBWinObjectDescriptionTable022f *out_table)
{
    size_t index;

    if (!out_table) return 0;
    memset(out_table, 0, sizeof(*out_table));
    if (!decoded_graphic || decoded_size !=
            CSB_V1_CSBWIN_OBJDESC_022F_DECODED_SIZE) return 0;
    for (index = 0u; index < CSB_V1_CSBWIN_OBJDESC_022F_COUNT; ++index) {
        const uint8_t *record = decoded_graphic +
            CSB_V1_CSBWIN_OBJDESC_022F_TABLE_OFFSET +
            index * CSB_V1_CSBWIN_OBJDESC_022F_RECORD_SIZE;
        out_table->entries[index].object_type =
            (int16_t)csb_v1_csbwin_objdesc_022f_read_u16be(record);
        out_table->entries[index].graphic_class = record[2];
        out_table->entries[index].attack_class = (int8_t)record[3];
        out_table->entries[index].carry_locations =
            csb_v1_csbwin_objdesc_022f_read_u16be(record + 4u);
    }
    out_table->valid = 1;
    return 1;
}

int csb_v1_csbwin_objdesc_022f_read_graphics_dat(
    const char *graphics_dat_path,
    CSB_V1_CSBWinObjectDescriptionTable022f *out_table)
{
    CSB_AtariStLoader loader;
    uint8_t *decoded = NULL;
    int ok = 0;

    if (!out_table) return 0;
    memset(out_table, 0, sizeof(*out_table));
    if (!graphics_dat_path || !graphics_dat_path[0]) return 0;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, graphics_dat_path) ||
        loader.item_count != 563u ||
        loader.items[CSB_V1_CSBWIN_OBJDESC_022F_ITEM_INDEX].decompressed_size !=
            CSB_V1_CSBWIN_OBJDESC_022F_DECODED_SIZE) goto done;
    decoded = (uint8_t *)malloc(CSB_V1_CSBWIN_OBJDESC_022F_DECODED_SIZE);
    if (!decoded || csb_atari_st_graphics_loader_read_item(
            &loader, CSB_V1_CSBWIN_OBJDESC_022F_ITEM_INDEX, decoded,
            CSB_V1_CSBWIN_OBJDESC_022F_DECODED_SIZE) !=
            (int)CSB_V1_CSBWIN_OBJDESC_022F_DECODED_SIZE) goto done;
    ok = csb_v1_csbwin_objdesc_022f_decode(
        decoded, CSB_V1_CSBWIN_OBJDESC_022F_DECODED_SIZE, out_table);
done:
    free(decoded);
    csb_atari_st_graphics_loader_close(&loader);
    return ok;
}
