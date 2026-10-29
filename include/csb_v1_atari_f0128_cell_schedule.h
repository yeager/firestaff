#ifndef CSB_V1_ATARI_F0128_CELL_SCHEDULE_H
#define CSB_V1_ATARI_F0128_CELL_SCHEDULE_H

#include <stddef.h>
#include <stdint.h>

typedef struct CSB_V1_AtariF0128CellScheduleEntry {
    uint8_t source_function; /* ReDMCSB F0115 or F0116 through F0127 */
    int8_t forward;
    int8_t right;
    int8_t view_square; /* MEDIA020 D4 view-square ids are negative */
    uint8_t has_conditional_alcove_pass; /* F0115 order word 0x0000 */
    uint8_t normal_order_count;
    uint16_t normal_orders[2];
    uint16_t door_pass1_order; /* zero when this square has no front door */
    uint16_t door_pass2_order;
} CSB_V1_AtariF0128CellScheduleEntry;

typedef struct CSB_V1_AtariF0128StaticEdge {
    int8_t forward;
    int8_t right;
} CSB_V1_AtariF0128StaticEdge;

/* F0128's three D4 object-only F0115 calls, then F0116-F0127 in order. */
const CSB_V1_AtariF0128CellScheduleEntry *
csb_v1_atari_f0128_cell_schedule(size_t *count);

/* F0128 static wall edges have no matching F0115 square call. */
const CSB_V1_AtariF0128StaticEdge *
csb_v1_atari_f0128_static_edges(size_t *count);

#endif
