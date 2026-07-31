#ifndef NEXUS_V1_FONT012_H
#define NEXUS_V1_FONT012_H

#include <stddef.h>
#include <stdint.h>

/* DMWeb Nexus Translation Kit FONT resource admission.  This is deliberately
 * a header/resource receipt only: glyph packing, character codes and Saturn
 * VDP2 placement remain separate evidence gates. */
typedef struct {
    int valid;
    uint32_t resource_index;
    uint32_t header_word12;
    uint32_t format_word;
    uint16_t character_count;
    uint16_t character_width;
    uint16_t character_height;
    uint32_t resource_size;
} Nexus_V1_Font012Receipt;

int nexus_v1_font012_parse(const uint8_t *data, size_t size,
                           uint32_t resource_index,
                           Nexus_V1_Font012Receipt *out);

#endif
