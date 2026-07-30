#ifndef FIRESTAFF_CSB_V1_MAGIC_RUNE_COST_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_MAGIC_RUNE_COST_PC34_COMPAT_H

#include <stdint.h>

#include "csb_v1_csbgraphics_dat_real_scan.h"

/* CSBWin CSBCode.cpp:10277-10284 expands GRAPHICS.DAT graphic 0x230 into
 * the menu-data block. Data.h places Byte19016 at 0x4cc and Byte19010 at
 * 0x4d2 of its fixed 0x4e8-byte decoded payload. CSBCode.cpp:9074-9093
 * consumes these values for the source F0399-style rune transaction. */
#define CSB_V1_MAGIC_RUNE_TABLE_GRAPHICS_ENTRY_PC34 0x230u
#define CSB_V1_MAGIC_RUNE_TABLE_DECODED_SIZE_PC34 0x4e8u
#define CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34 0x4ccu
#define CSB_V1_MAGIC_RUNE_BASE_COST_OFFSET_PC34 0x4d2u
#define CSB_V1_MAGIC_RUNE_ROW_COUNT_PC34 4
#define CSB_V1_MAGIC_RUNE_SYMBOLS_PER_ROW_PC34 6

typedef struct {
    int valid;
    uint32_t decoded_payload_fnv1a;
    uint8_t power_multiplier[CSB_V1_MAGIC_RUNE_SYMBOLS_PER_ROW_PC34];
    uint8_t base_cost[CSB_V1_MAGIC_RUNE_ROW_COUNT_PC34]
                     [CSB_V1_MAGIC_RUNE_SYMBOLS_PER_ROW_PC34];
} CSB_V1_MagicRuneCostTablePc34;

/* Parses only the exact decoded original graphic payload. It has no default
 * table or compatibility fallback. */
int csb_v1_magic_rune_cost_table_from_decoded_graphic_pc34(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_MagicRuneCostTablePc34 *out_table);

/* Decodes graphic 0x230 through the authenticated active CSB package. */
int csb_v1_magic_rune_cost_table_from_cache_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    CSB_V1_MagicRuneCostTablePc34 *out_table);

/* CSBWin CSBCode.cpp:9076-9083. power_rune is the first encoded symbol
 * (96..101), or -1 for row zero where no multiplier is applied. */
int csb_v1_magic_rune_cost_compute_pc34(
    const CSB_V1_MagicRuneCostTablePc34 *table,
    int symbol_step, int symbol_index, int power_rune, int *out_cost);

#endif
