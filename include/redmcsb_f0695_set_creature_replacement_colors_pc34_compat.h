/*
 * Source-faithful PC 3.4 C25_VGA boundary for ReDMCSB F0695.
 *
 * IMAGE.C:151-155 dispatches VIDRV_12 then invalidates G2123_.
 * VIDEODRV.C:3543-3564 supplies the C25_VGA implementation.
 */
#ifndef FIRESTAFF_REDMCSB_F0695_SET_CREATURE_REPLACEMENT_COLORS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0695_SET_CREATURE_REPLACEMENT_COLORS_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT = 6
};

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RedmcsbF0695ColorPc34Compat;

/*
 * palette_tables models G8176_PaletteTable, replacement_sets models
 * G8175_CREAT_PAL. Each supplied palette must have color_count entries.
 */
typedef struct {
    RedmcsbF0695ColorPc34Compat *palette_tables[
        REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT];
    size_t color_count;
    const RedmcsbF0695ColorPc34Compat *replacement_sets;
    size_t replacement_set_count;
    int16_t *palette_cache_index;
} RedmcsbF0695StatePc34Compat;

/*
 * Applies replacement_set_index to replaced_color in all six palettes and
 * sets the F0695 wrapper's G2123_ equivalent to -1. Invalid bounded input
 * leaves all palette entries and the cache marker unchanged.
 */
bool redmcsb_f0695_set_creature_replacement_colors_pc34_compat(
    RedmcsbF0695StatePc34Compat *state,
    int16_t replaced_color,
    int16_t replacement_set_index);

const char *redmcsb_f0695_set_creature_replacement_colors_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
