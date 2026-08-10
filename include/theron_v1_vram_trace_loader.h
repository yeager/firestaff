#ifndef THERON_V1_VRAM_TRACE_LOADER_H
#define THERON_V1_VRAM_TRACE_LOADER_H

#include "theron_v1_viewport.h"

#define THERON_VRAM_SIZE  65536
#define THERON_VCE_SIZE   1024
#define THERON_VRAM_TILE_BYTES 32

int theron_v1_vram_trace_load_raw(Theron_V1_Viewport *vp,
                                  const uint8_t *vram_data, int vram_size,
                                  const uint8_t *vce_data, int vce_size);

int theron_v1_vram_trace_load_files(Theron_V1_Viewport *vp,
                                    const char *vram_path,
                                    const char *vce_path);

/* Load the operator-authenticated screen-space dungeon capture.  The FNV-1a
 * values identify the complete raw files, not merely their lengths; callers
 * must supply the expected values from the capture receipt. */
int theron_v1_vram_trace_load_verified_files(
    Theron_V1_Viewport *vp,
    const char *vram_path,
    const char *vce_path,
    uint32_t expected_vram_fnv1a,
    uint32_t expected_vce_fnv1a);

/* Load one of the operator-authenticated screen-space capture pairs whose
 * complete file identities are recorded in the Theron source-lock notes.
 * This is deliberately a closed hash allow-list, not a caller-controlled
 * "trust any snapshot" switch.  It authorizes only BAT/tile/palette replay;
 * it does not authorize dungeon-square, perspective or object semantics. */
int theron_v1_vram_trace_load_known_capture_files(
    Theron_V1_Viewport *vp,
    const char *vram_path,
    const char *vce_path);

int theron_v1_vram_trace_load_tqtr(Theron_V1_Viewport *vp,
                                   const char *tqtr_path);

void theron_v1_vram_trace_unload(Theron_V1_Viewport *vp);

int theron_v1_vram_trace_populate_tiles(Theron_V1_Viewport *vp,
                                        int bat_start_word,
                                        int bat_w, int bat_h);

/* Return the source-owned atlas entry for one admitted BAT word, or -1.
 * The word is an index in the 64x32 VDC BAT, not a Theron dungeon square. */
int theron_v1_vram_trace_bat_atlas_index(const Theron_V1_Viewport *vp,
                                         int bat_word);

/* Return non-zero only after an authenticated VCE snapshot has been loaded
 * and the admitted BAT window has bound its palette-group bits to that
 * snapshot.  This receipt covers screen-space hardware binding only. */
int theron_v1_vram_trace_palette_relation_verified(
    const Theron_V1_Viewport *vp);

/* Draw an explicitly requested raw BAT window using the authenticated atlas
 * entries.  The coordinates are VDC tile-preview coordinates, not dungeon
 * squares; no world or object state is consulted.  Returns the number of
 * admitted BAT cells copied, or -1 for an invalid request. */
int theron_v1_vram_trace_render_bat_preview(Theron_V1_Viewport *vp,
                                            int bat_start_word,
                                            int bat_w,
                                            int bat_h,
                                            int dst_x,
                                            int dst_y);

/* Render the authenticated 256x224 screen-space BAT window at the native
 * framebuffer origin. This is a VDC capture presentation route only: it does
 * not claim a dungeon-square, perspective, HUD or object mapping. */
int theron_v1_vram_trace_render_authenticated_screen(Theron_V1_Viewport *vp);

#endif
