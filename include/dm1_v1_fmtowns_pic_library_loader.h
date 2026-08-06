#ifndef DM1_V1_FMTOWNS_PIC_LIBRARY_LOADER_H
#define DM1_V1_FMTOWNS_PIC_LIBRARY_LOADER_H

#include "dm1_v1_fmtowns_pic_library.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * File-backed loader for the FM Towns DM1 picture library. Reads
 * `<cache_dir>/GRAPHICS.DAT` (or `<cache_dir>/<override_name>`) from
 * disk into a heap-owned buffer and returns a
 * `dm1_v1_fmtowns_pic_library_view_t` that points into it. The
 * caller must call `dm1_v1_fmtowns_pic_library_release_pc34` to
 * free the backing buffer.
 *
 * Path resolution matches how m12_materialize_dm1_fmtowns_runtime_cache
 * writes the cache: the ISO `DATA/` prefix is stripped, so the file
 * lives directly at `<cache_dir>/GRAPHICS.DAT`. See
 * `src/shared/asset_status_m12.c` around line 4067.
 *
 * This module only reads a file; it does not decode any asset. Callers
 * that need pixel data should hand the returned view to
 * `dm1_v1_fmtowns_pic_library_decode_asset_pc34` in the shipping
 * decoder module. The menu font (index 557, DIRECT+NO_HDR path) is a
 * bounded convenience wrapper `..._load_menu_font_pc34` that copies
 * the byte-verified 768-byte span into a caller-owned buffer without
 * decompression, matching INIT_TEXT's runtime call shape.
 *
 * Evidence chain:
 *   parity-evidence/dm1_fmtowns_pic_library_format.md
 *   parity-evidence/dm1_fmtowns_font_asset.md
 *   docs/wiki/DM1-FMTowns-Guide.md sections 6b/6c
 */

typedef enum {
    DM1_V1_FMTOWNS_PIC_LIB_LOAD_OK             = 0,
    DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_NULL       = 1,
    DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_PATH       = 2,
    DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_OPEN       = 3,
    DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_READ       = 4,
    DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_ALLOC      = 5,
    DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_TOO_LARGE  = 6,
    DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_CONTAINER  = 7,
    DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_FONT_SIZE  = 8
} dm1_v1_fmtowns_pic_library_load_status_t;

/* Heap-owning handle. Callers must not modify `bytes` directly. */
typedef struct {
    uint8_t                              *bytes;
    size_t                                size_bytes;
    dm1_v1_fmtowns_pic_library_view_t     view;
} dm1_v1_fmtowns_pic_library_handle_t;

/* Load `<cache_dir>/GRAPHICS.DAT` (or `<cache_dir>/<override_name>`
 * when `override_name` is non-NULL) into a heap-owned buffer and
 * populate `*out_handle` with a valid view. Returns OK on success. On
 * any failure the handle is left zeroed and no allocation is retained. */
dm1_v1_fmtowns_pic_library_load_status_t
dm1_v1_fmtowns_pic_library_load_from_cache_pc34(
    const char                          *cache_dir,
    const char                          *override_name,
    dm1_v1_fmtowns_pic_library_handle_t *out_handle);

/* Release the buffer owned by `handle`. Safe to call on a zeroed
 * handle. Zeros the handle after freeing. */
void dm1_v1_fmtowns_pic_library_release_pc34(
    dm1_v1_fmtowns_pic_library_handle_t *handle);

/* Load the menu font (picture-library index 557, DIRECT+NO_HDR path)
 * into `out_buffer`, which must be at least
 * DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES (768) bytes. This mirrors
 * INIT_TEXT's GET_MY_DECODED(0xffffc22d, buf, 0, 0) call exactly:
 * the raw span is byte-copied without invoking DECODEGRAPHIC. Returns
 * DM1_V1_FMTOWNS_PIC_LIB_LOAD_OK on success and writes exactly 768
 * bytes; returns _ERR_FONT_SIZE if the loaded library's asset 557
 * span is not 768 bytes (i.e. the file is not the hash-verified
 * DM1 FM Towns library). */
dm1_v1_fmtowns_pic_library_load_status_t
dm1_v1_fmtowns_pic_library_load_menu_font_pc34(
    const dm1_v1_fmtowns_pic_library_handle_t *handle,
    uint8_t                                   *out_buffer,
    size_t                                     out_buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_PIC_LIBRARY_LOADER_H */
