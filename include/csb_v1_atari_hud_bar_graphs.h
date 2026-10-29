#ifndef CSB_V1_ATARI_HUD_BAR_GRAPHS_H
#define CSB_V1_ATARI_HUD_BAR_GRAPHS_H

#include <stddef.h>
#include <stdint.h>

typedef struct CSB_V1_AtariHudChampionBars {
    uint16_t current[3]; /* ReDMCSB CHAMDRAW.C F0287 draw order: health, stamina, mana */
    uint16_t maximum[3];
    int statistics_enabled; /* F0292 MASK0x0100_STATISTICS */
    int present;
} CSB_V1_AtariHudChampionBars;

/* Apply the original Atari ST F0287 bar bitmasks to a 320x200 indexed frame.
 * G0055/G0056 values are the Atari MEDIA009 source tables from DATA.C. */
int csb_v1_atari_hud_draw_bar_graphs(
    uint8_t *framebuffer, size_t framebuffer_size,
    const CSB_V1_AtariHudChampionBars champions[4]);

#endif
