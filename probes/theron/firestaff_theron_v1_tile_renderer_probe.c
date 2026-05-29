/*
 * firestaff_theron_v1_tile_renderer_probe.c
 *
 * Theron's Quest V1 Phase 4 — Tile renderer probe.
 *
 * Theron uses a 2D tile grid (not the DM1/CSB/DM2 3D first-person view).
 * Tile size is 8x8 pixels, square size is 16x16 pixels.
 *
 * Source: THQUEST.ASM T520; ReDMCSB DUNVIEW.C F0116-F0127
 * See: src/theron/theron_v1_tile_renderer.c
 *
 * Compile: see CMakeLists.txt
 * Run:     ./probe
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "theron_v1_tile_renderer.h"
#include "theron_v1_world.h"

/* Stub out missing symbols that the tile renderer calls but are
 * not yet fully implemented in the current theron_v1 source set.   */
int  theron_v1_play_sound(int id)       { (void)id; return 0; }
void theron_v1_champion_die(void *w, int s) { (void)w; (void)s; }

int main(void)
{
    printf("theron_v1_tile_renderer: stub — Phase 4 implementation pending\n");
    /* TODO: create planar framebuffer, call tr_render_dungeon,
     *       verify tile index selection per square_type and depth. */
    return 0;
}
