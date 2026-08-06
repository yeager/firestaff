#include "dm1_v1_fmtowns_pic_library_loader.h"
#include "dm1_v1_fmtowns_font_asset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Cap enforced by asset_status_m12.c materializer path buffer sizes
 * (M12_ASSET_DATA_DIR_CAPACITY). We use the same conservative bound
 * here so any path resolvable from that code path resolves here too. */
#ifndef DM1_V1_FMTOWNS_PIC_LIB_PATH_MAX
#define DM1_V1_FMTOWNS_PIC_LIB_PATH_MAX 1024u
#endif

/* Sanity cap on file size. The retail DM1 FM Towns GRAPHICS.DAT is
 * 396,970 bytes; allow up to 4 MB to keep an obvious rejection on a
 * truncated read or a corrupt cache. */
#ifndef DM1_V1_FMTOWNS_PIC_LIB_MAX_BYTES
#define DM1_V1_FMTOWNS_PIC_LIB_MAX_BYTES (4u * 1024u * 1024u)
#endif

static int join_path(char *dst, size_t dst_size,
                     const char *base, const char *name) {
    size_t base_len = strlen(base);
    size_t name_len = strlen(name);
    int needs_sep = base_len > 0 && base[base_len - 1] != '/';
    size_t total = base_len + (needs_sep ? 1u : 0u) + name_len;
    if (total + 1u > dst_size) return 0;
    memcpy(dst, base, base_len);
    if (needs_sep) dst[base_len] = '/';
    memcpy(dst + base_len + (needs_sep ? 1u : 0u), name, name_len);
    dst[total] = '\0';
    return 1;
}

dm1_v1_fmtowns_pic_library_load_status_t
dm1_v1_fmtowns_pic_library_load_from_cache_pc34(
    const char                          *cache_dir,
    const char                          *override_name,
    dm1_v1_fmtowns_pic_library_handle_t *out_handle) {
    char path[DM1_V1_FMTOWNS_PIC_LIB_PATH_MAX];
    FILE *fp;
    long size_long;
    size_t size;
    uint8_t *buf;
    size_t got;
    dm1_v1_fmtowns_pic_library_status_t view_status;

    if (!cache_dir || !out_handle) return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_NULL;
    memset(out_handle, 0, sizeof(*out_handle));

    if (!join_path(path, sizeof(path), cache_dir,
                   override_name ? override_name : "GRAPHICS.DAT")) {
        return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_PATH;
    }

    fp = fopen(path, "rb");
    if (!fp) return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_OPEN;

    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_READ; }
    size_long = ftell(fp);
    if (size_long < 0) { fclose(fp); return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_READ; }
    if ((unsigned long)size_long > DM1_V1_FMTOWNS_PIC_LIB_MAX_BYTES) {
        fclose(fp);
        return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_TOO_LARGE;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_READ; }

    size = (size_t)size_long;
    buf = (uint8_t *)malloc(size);
    if (!buf) { fclose(fp); return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_ALLOC; }

    got = fread(buf, 1, size, fp);
    fclose(fp);
    if (got != size) {
        free(buf);
        return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_READ;
    }

    view_status = dm1_v1_fmtowns_pic_library_open_pc34(buf, size, &out_handle->view);
    if (view_status != DM1_V1_FMTOWNS_PIC_LIB_OK) {
        free(buf);
        memset(out_handle, 0, sizeof(*out_handle));
        return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_CONTAINER;
    }

    out_handle->bytes = buf;
    out_handle->size_bytes = size;
    return DM1_V1_FMTOWNS_PIC_LIB_LOAD_OK;
}

void dm1_v1_fmtowns_pic_library_release_pc34(
    dm1_v1_fmtowns_pic_library_handle_t *handle) {
    if (!handle) return;
    if (handle->bytes) free(handle->bytes);
    memset(handle, 0, sizeof(*handle));
}

dm1_v1_fmtowns_pic_library_load_status_t
dm1_v1_fmtowns_pic_library_load_menu_font_pc34(
    const dm1_v1_fmtowns_pic_library_handle_t *handle,
    uint8_t                                   *out_buffer,
    size_t                                     out_buffer_size) {
    dm1_v1_fmtowns_pic_library_status_t st;
    uint16_t asset_size = 0;
    const uint8_t *asset_bytes = NULL;
    uint16_t got_size = 0;
    uint16_t font_index;

    if (!handle || !handle->bytes || !out_buffer)
        return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_NULL;
    if (out_buffer_size < DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES)
        return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_NULL;

    font_index = dm1_v1_fmtowns_font_pic_library_index_pc34();
    st = dm1_v1_fmtowns_pic_library_asset_size_pc34(&handle->view,
                                                    font_index,
                                                    &asset_size);
    if (st != DM1_V1_FMTOWNS_PIC_LIB_OK)
        return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_CONTAINER;
    if (asset_size != DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES)
        return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_FONT_SIZE;

    st = dm1_v1_fmtowns_pic_library_asset_bytes_pc34(&handle->view,
                                                     font_index,
                                                     &asset_bytes,
                                                     &got_size);
    if (st != DM1_V1_FMTOWNS_PIC_LIB_OK || !asset_bytes ||
        got_size != DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES)
        return DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_CONTAINER;

    /* DIRECT+NO_HDR path: byte-copy the raw span. See
     * parity-evidence/dm1_fmtowns_font_asset.md. No DECODEGRAPHIC. */
    memcpy(out_buffer, asset_bytes, DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES);
    return DM1_V1_FMTOWNS_PIC_LIB_LOAD_OK;
}
