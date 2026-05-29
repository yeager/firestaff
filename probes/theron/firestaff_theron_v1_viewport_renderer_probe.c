/*
 * firestaff_theron_v1_viewport_renderer_probe.c
 *
 * Theron's Quest V1 Phase 4 — Viewport renderer probe.
 *
 * Theron viewport rendering uses a 256x224 planar framebuffer with UI chrome
 * zones mapped to the 320x240 display layout.
 *
 * Source: THQUEST.ASM T520 (viewport tile selection), T600 (UI overlay zones)
 * See: src/theron/theron_v1_viewport.c
 *
 * Compile: see CMakeLists.txt
 * Run:     ./probe
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "theron_v1_viewport.h"
#include "theron_v1_world.h"
#include "theron_v1_mechanics.h"

/* Stub out missing symbols that the viewport module calls but are
 * not yet fully implemented in the current theron_v1 source set.      */
int  theron_v1_play_sound(int id)               { (void)id; return 0; }
void theron_v1_champion_die(void *w, int s)     { (void)w; (void)s; }

int main(void)
{
    printf("theron_v1_viewport_renderer: stub — Phase 4 implementation pending\n");
    /* TODO: instantiate TQR_PlanarFramebuffer, call tr_render_dungeon,
     *       verify output bounds and tile selection per depth.         */
    return 0;
}
