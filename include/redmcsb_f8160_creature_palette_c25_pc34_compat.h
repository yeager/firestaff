/* ReDMCSB VIDEODRV.C F8160 creature replacement colours, PC 3.4 C25. */
#ifndef FIRESTAFF_REDMCSB_F8160_CREATURE_PALETTE_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8160_CREATURE_PALETTE_C25_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8160_CREATURE_COLOR_SETS_PC34 14U
#define REDMCSB_F8160_CREATURE_REPLACEMENT_COUNT_PC34 6U

/* COLOR_DEF field order in DEFS.H: Index, Red, Green, Blue. */
typedef struct {
    uint8_t index;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RedmcsbF8160ColorPc34Compat;

/*
 * C25 F8160 rewrites only RGB of table rows 0..5 at replaced_color from one
 * source row in G8175_CREAT_PAL. The caller supplies the six real palette
 * table bases and their available entry count; no palette layout is invented.
 */
bool redmcsb_f8160_set_creature_replacement_colors_c25_pc34_compat(
    RedmcsbF8160ColorPc34Compat *palette_tables
        [REDMCSB_F8160_CREATURE_REPLACEMENT_COUNT_PC34],
    size_t palette_table_entry_count,
    const RedmcsbF8160ColorPc34Compat creature_palettes
        [REDMCSB_F8160_CREATURE_COLOR_SETS_PC34]
        [REDMCSB_F8160_CREATURE_REPLACEMENT_COUNT_PC34],
    int16_t replaced_color, int16_t replacement_color_set_index);

const char *redmcsb_f8160_creature_palette_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
