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

int theron_v1_vram_trace_load_tqtr(Theron_V1_Viewport *vp,
                                   const char *tqtr_path);

void theron_v1_vram_trace_unload(Theron_V1_Viewport *vp);

int theron_v1_vram_trace_populate_tiles(Theron_V1_Viewport *vp,
                                        int bat_start_word,
                                        int bat_w, int bat_h);

#endif
