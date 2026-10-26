#ifndef DM1_V1_FMTOWNS_HUD_LAYOUT_H
#define DM1_V1_FMTOWNS_HUD_LAYOUT_H

#include <stddef.h>
#include <stdint.h>

typedef enum DM1_V1_FmtownsHudEdition {
    DM1_V1_FMTOWNS_HUD_EDITION_F20E = 1,
    DM1_V1_FMTOWNS_HUD_EDITION_F20J = 2
} DM1_V1_FmtownsHudEdition;

typedef struct DM1_V1_FmtownsHudLayoutRecord {
    uint16_t zone_index;
    uint8_t record_type;
    uint16_t parent_index;
    int16_t data1;
    int16_t data2;
} DM1_V1_FmtownsHudLayoutRecord;

enum { DM1_V1_FMTOWNS_HUD_LAYOUT_MAX_CHAIN = 8 };

typedef struct DM1_V1_FmtownsHudLayoutChain {
    DM1_V1_FmtownsHudLayoutRecord records[
        DM1_V1_FMTOWNS_HUD_LAYOUT_MAX_CHAIN];
    size_t record_count;
} DM1_V1_FmtownsHudLayoutChain;

typedef struct DM1_V1_FmtownsHudRect {
    int x;
    int y;
    int width;
    int height;
    int source_x;
    int source_y;
} DM1_V1_FmtownsHudRect;

/* Return the source-authentic F20 static records and parent chain for a
 * supported DM1 HUD zone. Root parent index zero is a sentinel and is not
 * included in the returned chain. Unsupported records and incomplete chains
 * fail. */
int dm1_v1_fmtowns_hud_layout_chain(
    DM1_V1_FmtownsHudEdition edition,
    unsigned int zone_index,
    DM1_V1_FmtownsHudLayoutChain *out_chain);

/* Resolve a source bitmap/text extent through ReDMCSB COORD.C F0635.
 * Both native source dimensions are required; unknown text/glyph dimensions
 * must fail closed. Returns F20 destination geometry as x/y/width/height plus
 * clipped source offsets. Type-9 records are containers, not drawable leaves. */
int dm1_v1_fmtowns_hud_layout_rect(
    DM1_V1_FmtownsHudEdition edition,
    unsigned int zone_index,
    int bitmap_width,
    int bitmap_height,
    DM1_V1_FmtownsHudRect *out_rect);

/* F0635-backed indexed-pixel blit used by bounded F20 HUD consumers. Source
 * dimensions determine placement; sourceX/sourceY select a subregion. A
 * negative transparentIndex means opaque copy. */
int dm1_v1_fmtowns_hud_blit_indexed(
    DM1_V1_FmtownsHudEdition edition,
    unsigned int zone_index,
    const uint8_t *source,
    int source_stride,
    int source_height,
    int bitmap_width,
    int bitmap_height,
    int source_x,
    int source_y,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int transparent_index);

#endif
