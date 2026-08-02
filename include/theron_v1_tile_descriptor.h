#ifndef THERON_V1_TILE_DESCRIPTOR_H
#define THERON_V1_TILE_DESCRIPTOR_H

#include <stdint.h>
#include <stddef.h>

/* Tile data stream header — 8 bytes at the start of each tile data block.
 * All offsets are relative to the stream start (= graphics_base).
 * Source: disassembly of $43E4 setup routine in Track 02. */
typedef struct {
    uint16_t field1_offset;      /* +0: relative offset to field_1 data */
    uint16_t field2_offset;      /* +2: relative offset to field_2 data */
    uint16_t desc_table_offset;  /* +4: relative offset to descriptor table */
    uint16_t palette_offset;     /* +6: relative offset to palette data */
} Theron_TileStreamHeader;

#define THERON_TILE_STREAM_HEADER_SIZE 8u

/* Tile descriptor record — 32 bytes per tile.
 * Array base at CPU $4DC6. Up to 8 tiles per batch.
 * Source: disassembly of $4914 main loop in Track 02. */
#define THERON_TILE_DESC_FLAG_FLIP     0x80u  /* bit 7: flip/mirror */
#define THERON_TILE_DESC_FLAG_ANIMATED 0x02u  /* bit 1: animated */

typedef struct {
    uint8_t  flags;           /* +$00: bit 7=flip, bit 1=animated */
    uint8_t  flip_params[5];  /* +$01-$05: flip transform parameters */
    uint8_t  reserved_06[2];  /* +$06-$07 */
    uint8_t  anim_timer;      /* +$08: frame countdown timer */
    uint8_t  reserved_09[3];  /* +$09-$0B */
    uint16_t anim_seq_ptr;    /* +$0C-$0D: animation sequence pointer */
    uint16_t secondary_ptr;   /* +$0E-$0F: secondary data pointer */
    uint8_t  secondary_timer; /* +$10: secondary animation timer */
    uint8_t  reserved_11[5];  /* +$11-$15 */
    uint16_t tertiary_ptr;    /* +$16-$17: tertiary data pointer */
    uint8_t  reserved_18[8];  /* +$18-$1F: padding to 32 bytes */
} Theron_TileDescriptor;

#define THERON_TILE_DESCRIPTOR_SIZE  32u
#define THERON_TILES_PER_BATCH        8u
#define THERON_TILE_BUFFER_SIZE     512u  /* VRAM DMA transfer size */

#endif /* THERON_V1_TILE_DESCRIPTOR_H */
