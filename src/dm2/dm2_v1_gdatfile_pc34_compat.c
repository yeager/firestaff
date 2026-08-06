/*
 * dm2_v1_gdatfile_pc34_compat.c — DM2 GDAT file handling implementation.
 *
 * Source: skproject c_gdatfile.cpp.
 * Ported to pure C with callback-based architecture.
 */

#include "dm2_v1_gdatfile_pc34_compat.h"
#include <string.h>

/* skproject filename constants (c_gdatfile.cpp:21-27) */
static const char gdatfn1[] = ".Z020GRAPHICS.DAT";
static const char gdatfn2[] = ".Z026GRAPHIC2.DAT";
static const char gdatfn3[] = ".Z020DUNGENB.DAT";
static const char gdatfn4[] = ".Z020DUNGEON.DAT";
static const char gdatfn5[] = ".Z022SKSAVE.Z023.DAT";
static const char gdatfn6[] = ".Z022SKSAVE.Z023.BAK";
static const char gdatfn7[] = ".Z020DUNGEON.FTL";

/* ======================================================================== */

/* skproject: c_gdatfile::init (c_gdatfile.cpp:349-366) */
int dm2_v1_gdat_file_init(DM2_V1_GdatFileState *state,
                           DM2_V1_GdatFileInitReceipt *out)
{
    if (!state) return 0;
    memset(state, 0, sizeof(*state));
    state->filename1 = gdatfn1;
    state->filename2 = gdatfn2;
    state->filename3 = gdatfn3;
    state->filename4 = gdatfn4;
    state->filename5 = gdatfn5;
    state->filename6 = gdatfn6;
    state->filename7 = gdatfn7;
    if (out) { out->valid = true; }
    return 1;
}

/* skproject: DM2_GRAPHICS_DATA_OPEN (c_gdatfile.cpp:368-382) */
int dm2_v1_gdat_graphics_data_open(DM2_V1_GdatFileState *state,
                                    const DM2_V1_GdatFileCallbacks *cb,
                                    void *ctx,
                                    DM2_V1_GdatOpenReceipt *out)
{
    if (!state || !cb || !cb->file_open) return 0;

    state->fileopencounter++;
    if (state->fileopencounter == 1) {
        const char *name = state->filename1;
        if (cb->format_skstr)
            name = cb->format_skstr(ctx, state->filename1);
        state->filehandle = cb->file_open(ctx, name);
        if (state->filehandle < 0) {
            if (cb->raise_syserr) cb->raise_syserr(ctx, 0x29);
            if (out) { out->opened = false; out->open_count = state->fileopencounter; }
            return 0;
        }
        /* skproject: if (!filetype1 && filetype2) open second file */
        if (!state->filetype1 && state->filetype2) {
            name = state->filename2;
            if (cb->format_skstr)
                name = cb->format_skstr(ctx, state->filename2);
            state->xfilehandle = cb->file_open(ctx, name);
            if (state->xfilehandle < 0) {
                if (cb->raise_syserr) cb->raise_syserr(ctx, 0x1f);
                if (out) { out->opened = false; out->open_count = state->fileopencounter; }
                return 0;
            }
        }
    }
    if (out) {
        out->opened = true;
        out->filehandle = state->filehandle;
        out->xfilehandle = state->xfilehandle;
        out->open_count = state->fileopencounter;
    }
    return 1;
}

/* skproject: DM2_GRAPHICS_DATA_CLOSE (c_gdatfile.cpp:384-395) */
int dm2_v1_gdat_graphics_data_close(DM2_V1_GdatFileState *state,
                                     const DM2_V1_GdatFileCallbacks *cb,
                                     void *ctx,
                                     DM2_V1_GdatCloseReceipt *out)
{
    if (!state || !cb) return 0;

    int16_t cnt = state->fileopencounter - 1;
    state->fileopencounter = cnt;
    if (cnt == 0) {
        if (cb->file_close) cb->file_close(ctx, state->filehandle);
        if (!state->filetype1 && state->filetype2) {
            if (cb->file_close) cb->file_close(ctx, state->xfilehandle);
        }
    }
    if (out) {
        out->closed = (cnt == 0);
        out->open_count = cnt;
    }
    return 1;
}

/* skproject: DM2_GRAPHICS_DATA_READ (c_gdatfile.cpp:399-452) */
int dm2_v1_gdat_graphics_data_read(DM2_V1_GdatFileState *state,
                                    int32_t offset, int32_t length,
                                    uint8_t *dest,
                                    const DM2_V1_GdatFileCallbacks *cb,
                                    void *ctx,
                                    DM2_V1_GdatReadReceipt *out)
{
    if (!state || !cb || !dest || length <= 0) return 0;
    if (!cb->file_seek || !cb->file_read) return 0;

    /* Single-file case */
    int32_t offsets[2];
    int32_t lengths[2];
    int16_t handles[2];

    offsets[0] = offset;
    lengths[0] = length;
    lengths[1] = 0;
    handles[0] = state->filehandle;
    handles[1] = state->xfilehandle;

    if (state->filetype2) {
        /* Split read across two files at filesize boundary */
        int32_t xoff = offset - state->filesize;
        offsets[1] = xoff;
        if (xoff < 0) {
            /* Start in first file, may span into second */
            int32_t overflow = xoff + length;
            if (overflow <= 0) {
                lengths[1] = 0;
            } else {
                lengths[1] = overflow;
                offsets[1] = 0;
            }
        } else {
            /* Entirely in second file */
            lengths[1] = length;
        }
        lengths[0] -= lengths[1];
    }

    int32_t total = 0;
    for (int i = 0; i < 2; i++) {
        if (lengths[i] != 0) {
            if (!cb->file_seek(ctx, handles[i], offsets[i])) {
                /* Read failed — close and raise error */
                dm2_v1_gdat_graphics_data_close(state, cb, ctx, NULL);
                if (cb->raise_syserr)
                    cb->raise_syserr(ctx, (i != 0) ? 0x20 : 0x2e);
                if (out) { out->success = false; out->bytes_read = total; }
                return 0;
            }
            if (!cb->file_read(ctx, handles[i], dest + total, lengths[i])) {
                dm2_v1_gdat_graphics_data_close(state, cb, ctx, NULL);
                if (cb->raise_syserr)
                    cb->raise_syserr(ctx, (i != 0) ? 0x20 : 0x2e);
                if (out) { out->success = false; out->bytes_read = total; }
                return 0;
            }
            total += lengths[i];
        }
    }
    if (out) { out->success = true; out->bytes_read = total; }
    return 1;
}

/* skproject: DM2_QUERY_NEXT_GDAT_ENTRY (c_gdatfile.cpp:31-271)
 *
 * Complex binary-search iterator over the three-level GDAT table.
 * Searches for entries matching the query descriptor(s) in s19_04/s19_0a.
 * When s19_04.w_00 & 0x7fff == 1, range mode uses both descriptors.
 */
int dm2_v1_gdat_query_next_entry(DM2_V1_GdatQueryState *qs,
                                  const DM2_V1_GdatTable *table,
                                  DM2_V1_GdatQueryNextEntryReceipt *out)
{
    if (!qs || !table) return 0;

    int8_t vb_10 = qs->s19_04.b_03;
    int8_t byterg3lo = qs->s19_04.b_05;
    int32_t longrg5 = ((uint32_t)(qs->s19_04.w_00) & 0xffff7fffU) == 1U ? 1 : 0;
    int8_t vb_0c, vb_14, vb_18 = 0;

    if (longrg5 == 0) {
        vb_0c = vb_10;
    } else {
        vb_0c = qs->s19_0a.b_03;
        vb_18 = qs->s19_0a.b_05;
    }

    int32_t vl_00 = qs->l_00;
    if (vl_00 != 0) {
        qs->l_00 = 0;
        qs->b_14 = qs->s19_04.b_02;
        if (qs->s19_04.b_02 != (int8_t)-1) {
            if (longrg5 == 0) {
                qs->b_15 = qs->s19_04.b_02;
            } else {
                qs->b_15 = qs->s19_0a.b_02;
                if (qs->s19_0a.b_02 == (int8_t)-1)
                    qs->b_15 = (int8_t)table->entries;
            }
        } else {
            qs->b_14 = 0;
            qs->b_15 = (int8_t)table->entries;
        }
        if ((uint8_t)qs->b_14 > (uint8_t)qs->b_15) goto not_found;
        if ((uint32_t)(uint8_t)qs->b_15 > (uint32_t)table->entries) goto not_found;
    }

    int16_t wordrg2;
    DM2_V1_GdatBBW *u31prg4b;

    for (;;) { /* outer loop over b_14..b_15 */
        int16_t *wptrrg2 = &table->w_table1[(uint8_t)qs->b_14];
        int16_t wordrg4 = wptrrg2[0];

        if (wordrg4 != wptrrg2[1]) {
            bool brkpsloop = false;

            if (vl_00 != 0) {
                /* Initialize sub-range */
                int8_t byterg2lo = (int8_t)((uint8_t)wptrrg2[1] - (uint8_t)wordrg4 - 1);
                int8_t byterg2hi = qs->s19_04.b_04;
                qs->b_16 = byterg2hi;

                if (byterg2hi != (int8_t)-1) {
                    if ((uint8_t)byterg2hi > (uint8_t)byterg2lo) goto advance_outer;
                    if (longrg5 == 0) {
                        qs->b_17 = byterg2hi;
                    } else {
                        byterg2hi = qs->s19_0a.b_04;
                        qs->b_17 = byterg2hi;
                        if (byterg2hi == (int8_t)-1)
                            qs->b_17 = byterg2lo;
                    }
                } else {
                    qs->b_16 = 0;
                    qs->b_17 = byterg2lo;
                }

                wordrg4 += (uint8_t)qs->b_16;
                wordrg2 = table->w_table2[wordrg4];
                qs->w_1a = wordrg2;
                qs->w_18 = ++wordrg4;
                wordrg4 = table->w_table2[wordrg4];
                qs->w_1c = wordrg4;
                u31prg4b = &table->u31p_08[(uint16_t)wordrg2];
                qs->u31p_10 = u31prg4b;
            } else {
                /* Resume: re-enter inner loop */
                wordrg2 = qs->w_1a + 1;
                u31prg4b = qs->u31p_10 + 1;
                if ((uint16_t)wordrg2 >= (uint16_t)qs->w_1c)
                    goto inner_advance;
            }

            /* Inner entry scan */
            for (;;) {
                if ((uint16_t)wordrg2 >= (uint16_t)qs->w_1c)
                    goto inner_advance;

                if (vb_10 != (int8_t)0xff) {
                    /* Binary search within entry range */
                    if (byterg3lo != (int8_t)-1)
                        vb_14 = byterg3lo;
                    else
                        vb_14 = 0;

                    int16_t lo = wordrg2;
                    int16_t hi = qs->w_1c + 1;

                    for (;;) {
                        int16_t mid = (int16_t)(((uint32_t)lo + (uint32_t)hi) / 2);
                        DM2_V1_GdatBBW *midp = &table->u31p_08[(uint16_t)mid - 1];
                        if (mid != lo) {
                            if (vb_10 != midp->b_00) {
                                if ((uint8_t)vb_10 >= (uint8_t)midp->b_00)
                                    lo = mid;
                                else
                                    hi = mid;
                                continue;
                            }
                            if ((uint8_t)vb_14 < (uint8_t)midp->b_01) {
                                hi = mid;
                                continue;
                            }
                        } else {
                            if ((uint16_t)mid >= (uint16_t)qs->w_1c)
                                goto inner_advance;
                            mid++;
                            midp++;
                            if ((uint8_t)vb_0c < (uint8_t)midp->b_00)
                                goto inner_advance;
                        }
                        wordrg2 = mid - 1;
                        u31prg4b = midp;
                        break;
                    }
                }

                /* Linear scan for matching entries */
                for (;;) {
                    if ((uint8_t)u31prg4b->b_00 > (uint8_t)vb_0c)
                        break;

                    if (longrg5 == 0) {
                        if (byterg3lo == (int8_t)-1 || byterg3lo == u31prg4b->b_01)
                            goto found;
                    } else {
                        if (byterg3lo == (int8_t)-1)
                            goto found;
                        if ((uint8_t)byterg3lo <= (uint8_t)u31prg4b->b_01) {
                            if ((uint8_t)vb_18 >= (uint8_t)u31prg4b->b_01)
                                goto found;
                        }
                    }
                    if ((uint16_t)(++wordrg2) >= (uint16_t)qs->w_1c)
                        break;
                    u31prg4b++;
                }

inner_advance:
                {
                    int8_t byterg2 = qs->b_16 + 1;
                    qs->b_16 = byterg2;
                    if ((uint8_t)byterg2 > (uint8_t)qs->b_17) {
                        brkpsloop = true;
                        break;
                    }
                    wordrg2 = qs->w_1c;
                    int16_t next = qs->w_18 + 1;
                    qs->w_18 = next;
                    int16_t nval = table->w_table2[(uint16_t)next];
                    qs->w_1c = nval;
                }
            }
            if (brkpsloop) goto advance_outer;
        }

advance_outer:
        {
            int8_t b = qs->b_14 + 1;
            qs->b_14 = b;
            if ((uint8_t)b > (uint8_t)qs->b_15) goto not_found;
            vl_00 = 1;
        }
    }

found:
    qs->w_1a = wordrg2;
    qs->u31p_10 = u31prg4b;
    if (out) {
        out->found = true;
        out->entry = u31prg4b;
        out->table_index = qs->b_14;
        out->sub_index = qs->b_16;
    }
    return 1;

not_found:
    qs->u31p_10 = NULL;
    qs->b_14 = (int8_t)-1;
    qs->b_16 = (int8_t)-1;
    if (out) {
        out->found = false;
        out->entry = NULL;
        out->table_index = -1;
        out->sub_index = -1;
    }
    return 1;
}

/* skproject: DM2_QUERY_GDAT_ENTRY_VALUE (c_gdatfile.cpp:515-526) */
int dm2_v1_gdat_query_entry_value(const uint8_t *entry_data,
                                   int16_t entry_idx,
                                   int16_t field_idx,
                                   const int16_t *field_offsets,
                                   const uint8_t *field_sizes,
                                   int16_t record_size,
                                   DM2_V1_GdatEntryValueReceipt *out)
{
    if (!entry_data || !field_offsets || !field_sizes || !out) return 0;

    const uint8_t *p = entry_data
                     + (uint32_t)entry_idx * (uint32_t)record_size
                     + field_offsets[field_idx];
    uint8_t size = field_sizes[field_idx];
    int32_t value = 0;
    while (size-- > 0) {
        value <<= 8;
        value += (uint8_t)*p++;
    }
    out->value = value;
    out->valid = true;
    return 1;
}

/* skproject: DM2_QUERY_GDAT_RAW_DATA_FILE_POS (c_gdatfile.cpp:454-468) */
int dm2_v1_gdat_raw_data_file_pos(int16_t dbidx, int32_t base_offset,
                                   int16_t cached_idx,
                                   int32_t cached_offset,
                                   const DM2_V1_GdatFileCallbacks *cb,
                                   void *ctx,
                                   DM2_V1_GdatRawDataFilePosReceipt *out)
{
    if (!cb || !cb->query_raw_data_length || !out) return 0;

    int32_t pos = base_offset;
    int16_t start = 0;
    if (dbidx >= cached_idx) {
        start = cached_idx;
        pos += cached_offset;
    }
    while (start < dbidx) {
        pos += cb->query_raw_data_length(ctx, start++) & 0xffff;
    }
    out->position = pos;
    out->cached_idx = dbidx;
    out->cached_offset = pos - base_offset;
    return 1;
}

/* skproject: DM2_LOAD_GDAT_RAW_DATA (c_gdatfile.cpp:470-513) */
int dm2_v1_gdat_load_raw_data(DM2_V1_GdatFileState *state,
                               int16_t dbidx, uint8_t *dest,
                               int32_t data_length,
                               int32_t file_pos,
                               const DM2_V1_GdatFileCallbacks *cb,
                               void *ctx,
                               DM2_V1_GdatLoadRawDataReceipt *out)
{
    if (!state || !dest || !cb) return 0;
    (void)dbidx; /* used by caller to compute file_pos and data_length */

    /* Open, read in 0x400-byte pages, close */
    DM2_V1_GdatOpenReceipt orp;
    if (!dm2_v1_gdat_graphics_data_open(state, cb, ctx, &orp)) return 0;

    int32_t remaining = data_length;
    int32_t pos = file_pos;
    int32_t total = 0;

    /* skproject reads through a 0x400-byte cache page; for simplicity we
     * read directly since we have the callbacks. */
    while (remaining > 0) {
        int32_t chunk = remaining;
        DM2_V1_GdatReadReceipt rr;
        if (!dm2_v1_gdat_graphics_data_read(state, pos, chunk, dest + total,
                                             cb, ctx, &rr)) {
            dm2_v1_gdat_graphics_data_close(state, cb, ctx, NULL);
            if (out) { out->success = false; out->bytes_loaded = total; }
            return 0;
        }
        total += chunk;
        remaining -= chunk;
        pos += chunk;
    }

    dm2_v1_gdat_graphics_data_close(state, cb, ctx, NULL);
    if (out) { out->success = true; out->bytes_loaded = total; }
    return 1;
}

/* skproject: DM2_ALLOC_PICT_BUFF (c_gdatfile.cpp:529-546) */
int dm2_v1_gdat_alloc_pict_buff(int16_t width, int16_t height,
                                 int16_t pool, int8_t bpp,
                                 const DM2_V1_GdatFileCallbacks *cb,
                                 void *ctx,
                                 DM2_V1_GdatAllocPictReceipt *out)
{
    if (!cb || !cb->alloc_memory || !out) return 0;

    int32_t row_bytes;
    if (bpp != 4)
        row_bytes = (int32_t)(uint16_t)width;
    else
        row_bytes = (int32_t)(((uint16_t)width + 1) & 0xfffe) >> 1;

    int32_t pixel_size = (row_bytes & 0xffff) * (int32_t)(uint16_t)height;

    /* Allocate pixel_size + 6 bytes for header (res, unused, width, height) */
    uint8_t *mem = cb->alloc_memory(ctx, pixel_size + 6, pool);
    if (!mem) {
        out->bmp = NULL;
        return 0;
    }

    /* Fill 6-byte header: [0]=bpp, [1]=0, [2-3]=width, [4-5]=height */
    mem[0] = (uint8_t)bpp;
    mem[1] = 0;
    mem[2] = (uint8_t)(width & 0xff);
    mem[3] = (uint8_t)((width >> 8) & 0xff);
    mem[4] = (uint8_t)(height & 0xff);
    mem[5] = (uint8_t)((height >> 8) & 0xff);

    out->bmp = mem + 6;
    out->width = width;
    out->height = height;
    out->bpp = bpp;
    out->buffer_size = pixel_size + 6;
    return 1;
}

/* skproject: DM2_FREE_PICT_BUFF (c_gdatfile.cpp:548-557) */
int dm2_v1_gdat_free_pict_buff(int16_t width, int16_t height,
                                int8_t bpp,
                                const DM2_V1_GdatFileCallbacks *cb,
                                void *ctx)
{
    if (!cb || !cb->dealloc_memory) return 0;

    int32_t row_bytes;
    if (bpp != 4)
        row_bytes = (int32_t)(uint16_t)width;
    else
        row_bytes = (int32_t)(((uint16_t)width + 1) & 0xfffe) >> 1;

    int32_t pixel_size = (int32_t)(uint16_t)height * row_bytes;
    cb->dealloc_memory(ctx, (int32_t)(pixel_size + 6));
    return 1;
}

/* skproject: R_2BAD4 (c_gdatfile.cpp:575-578) */
int16_t dm2_v1_gdat_byte_swap_16(int16_t value)
{
    uint16_t v = (uint16_t)value;
    return (int16_t)((v >> 8) | (v << 8));
}

/* skproject: DM2_ALLOC_NEW_BMP (c_gdatfile.cpp:560-573) */
int dm2_v1_gdat_alloc_new_bmp(int16_t dbidx, int16_t width, int16_t height,
                               int8_t bpp,
                               const DM2_V1_GdatFileCallbacks *cb,
                               void *ctx,
                               DM2_V1_GdatAllocPictReceipt *out)
{
    if (!cb || !cb->alloc_memory || !out) return 0;
    (void)dbidx; /* used by caller for cpxheap allocation */

    int32_t row_bytes;
    if (bpp != 4)
        row_bytes = (int32_t)(uint16_t)width;
    else
        row_bytes = (int32_t)(((uint16_t)width + 1) & 0xfffe) >> 1;

    int32_t pixel_size = (row_bytes & 0xffff) * (int32_t)(uint16_t)height;

    uint8_t *mem = cb->alloc_memory(ctx, pixel_size + 6, 0);
    if (!mem) {
        out->bmp = NULL;
        return 0;
    }
    mem[0] = (uint8_t)bpp;
    mem[1] = 0;
    mem[2] = (uint8_t)(width & 0xff);
    mem[3] = (uint8_t)((width >> 8) & 0xff);
    mem[4] = (uint8_t)(height & 0xff);
    mem[5] = (uint8_t)((height >> 8) & 0xff);

    out->bmp = mem + 6;
    out->width = width;
    out->height = height;
    out->bpp = bpp;
    out->buffer_size = pixel_size + 6;
    return 1;
}

/* skproject: DM2_LOAD_GDAT_ENTRY_DATA_TO (c_gdatfile.cpp:818-821) */
int dm2_v1_gdat_load_entry_data_to(int8_t cls1, int8_t cls2,
                                    int8_t entry_type, int8_t data_idx,
                                    uint8_t *dest,
                                    const DM2_V1_GdatLoadEntryCallbacks *cb,
                                    void *ctx)
{
    if (!cb || !dest) return 0;
    int16_t dbidx = cb->query_entry_data_index(ctx, cls1, cls2,
                                                entry_type, data_idx);
    if (dbidx < 0) return 0;
    return cb->load_raw_data(ctx, dbidx, dest);
}

/* skproject: DM2_TRACK_UNDERLAY (c_gdatfile.cpp:997-1024) */
int dm2_v1_gdat_track_underlay(uint16_t dbidx,
                                const uint8_t *table, int16_t table_count,
                                DM2_V1_GdatTrackUnderlayReceipt *out)
{
    if (!table || !out || table_count <= 0) {
        if (out) { out->found = 0; out->value = -1; }
        return 0;
    }

    /* Binary search in 4-byte records: [uint16_t key, int16_t value] */
    int16_t lo = 0;
    int16_t hi = table_count + 1;
    for (;;) {
        int16_t mid = (int16_t)((lo + hi) / 2);
        if (mid == lo) {
            out->found = 0;
            out->value = -1;
            return 1;
        }
        const uint8_t *entry = table + 4 * (mid - 1);
        uint16_t key = (uint16_t)(entry[0] | (entry[1] << 8));
        if (dbidx > key) {
            lo = mid;
        } else if (dbidx < key) {
            hi = mid;
        } else {
            int16_t val = (int16_t)(entry[2] | (entry[3] << 8));
            out->found = 1;
            out->value = val;
            return 1;
        }
    }
}

/* ================================================================
 * DM2_BUILD_GDAT_ENTRY_DATA — c_gdatfile.cpp:674-815
 *
 * Builds the three-level GDAT lookup table:
 *   w_table1[category] -> w_table2[subcat] -> u31p_08[entry]
 *
 * Pass 1: count entries per (category, subcategory) pair using a
 *         temporary 29x16 histogram (0x3A0 bytes).
 * Pass 2: build index tables from histogram.
 * Pass 3: populate entry records (b_00, b_01, w_02) from raw data.
 * ================================================================ */
int dm2_v1_gdat_build_entry_data(DM2_V1_GdatTable *table,
                                  const int8_t *extra_fields,
                                  const DM2_V1_GdatBuildCallbacks *cb,
                                  void *ctx)
{
    if (!table || !cb) return 0;

    int16_t *wp = cb->alloc_hibigpool(ctx, 0x3A0);
    if (!wp) return 0;

    table->entries = 0;
    table->w_10 = 0;

    /* Pass 1: histogram. wp is 29 rows x 16 columns of int16_t.
     * Row r: columns 0..14 are per-subcategory counts, column 15
     * is the max subcategory index + 1 for that row. */
    for (int16_t i = 0; i < cb->total_entries; i++) {
        if (!cb->is_valid_entry(ctx, i)) continue;

        int32_t v0 = cb->query_entry_value(ctx, i, 0);
        int8_t cat = (int8_t)(v0 & 0xFF);
        int8_t sub = (int8_t)(cb->query_entry_value(ctx, i, 2) & 0xFF);

        if ((uint8_t)cat > 0x1C || (uint8_t)sub > 0x0E) continue;

        table->w_10++;
        if ((int16_t)(uint8_t)cat > table->entries)
            table->entries = (int16_t)(uint8_t)cat;

        wp[16 * (uint8_t)cat + (uint8_t)sub]++;

        int16_t *row = &wp[(uint8_t)cat * 16];
        if ((uint8_t)sub >= (uint16_t)row[15])
            row[15] = (int16_t)((uint8_t)sub + 1);
    }

    /* Compute w_0e: total subcategory slots */
    int16_t total_subcats = 0;
    for (int8_t c = 0; (uint8_t)c <= (uint16_t)table->entries; c++)
        total_subcats += wp[(uint8_t)c * 16 + 15];
    table->w_0e = total_subcats;

    /* Set up field offset table */
    table->w_12 = 0;
    table->w_14 = 0;
    for (int i = 0; i < 7; i++) {
        if (i > 4) {
            if (extra_fields[i] != 0) {
                table->warr_16[i] = table->w_12;
                table->barr_24[i] = (int8_t)cb->field_sizes[i];
                table->w_12 += (int16_t)(uint8_t)cb->field_sizes[i];
                table->w_14++;
            }
        } else {
            table->barr_24[i] = extra_fields[i];
            table->w_12 += (int16_t)(uint8_t)extra_fields[i];
            table->warr_16[i] = -1;
            if ((uint8_t)extra_fields[i] > 0)
                table->w_14++;
        }
    }

    /* Allocate index tables */
    table->w_table1 = cb->alloc_freepool_w(ctx,
        (int32_t)sizeof(int16_t) * ((int32_t)table->entries + 2));
    table->w_table2 = cb->alloc_freepool_w(ctx,
        (int32_t)sizeof(int16_t) * ((int32_t)table->w_0e + 1));
    table->u31p_08 = cb->alloc_freepool_bbw(ctx,
        (int32_t)sizeof(DM2_V1_GdatBBW) * (int32_t)table->w_10);

    /* Pass 2: build index tables */
    int16_t cat_acc = 0;
    int16_t entry_acc = 0;
    for (int8_t c = 0; (uint8_t)c <= (uint16_t)table->entries; c++) {
        table->w_table1[(uint8_t)c] = cat_acc;
        int16_t num_subs = wp[(uint8_t)c * 16 + 15];
        for (int8_t s = 0; (uint8_t)s < (uint16_t)num_subs; s++) {
            table->w_table2[cat_acc++] = entry_acc;
            entry_acc += wp[(uint8_t)c * 16 + (uint8_t)s];
        }
    }
    table->w_table1[(uint16_t)table->entries + 1] = cat_acc;
    table->w_table2[table->w_0e] = table->w_10;

    /* Clear histogram and entry records */
    cb->zero_memory(ctx, wp, 0x3A0);
    cb->zero_memory(ctx, table->u31p_08,
                    (int32_t)sizeof(DM2_V1_GdatBBW) * (int32_t)table->w_10);

    /* Pass 3: populate entries */
    for (int16_t i = 0; i < cb->total_entries; i++) {
        if (!cb->is_valid_entry(ctx, i)) continue;

        int32_t v0 = cb->query_entry_value(ctx, i, 0);
        int8_t cat = (int8_t)(v0 & 0xFF);
        int8_t sub = (int8_t)(cb->query_entry_value(ctx, i, 2) & 0xFF);

        if ((uint8_t)cat > 0x1C || (uint8_t)sub > 0x0E) continue;

        int16_t slot = wp[16 * (uint8_t)cat + (uint8_t)sub]++;
        int16_t base_idx = table->w_table1[(uint8_t)cat];
        int16_t sub_base = table->w_table2[base_idx + (uint8_t)sub];
        DM2_V1_GdatBBW *e = &table->u31p_08[sub_base + slot];

        e->b_00 = (int8_t)(cb->query_entry_value(ctx, i, 1) & 0xFF);
        e->b_01 = (int8_t)(cb->query_entry_value(ctx, i, 3) & 0xFF);
        e->w_02 = (int16_t)(cb->query_entry_value(ctx, i, 4) & 0xFFFF);
        if ((uint8_t)(cb->query_entry_value(ctx, i, 6) & 0xFF) == 1)
            e->w_02 |= (int16_t)0x8000;
    }

    cb->dealloc_hibigpool(ctx, 0x3A0);
    return 1;
}

/* ================================================================
 * DM2_47eb_00a4 — c_gdatfile.cpp:850-882
 *
 * Decode a sound sample descriptor. XOR all sample bytes with 0x80
 * (signed-to-unsigned PCM conversion). Insert into a linked list.
 *
 * The 6-byte header preceding xp_00 controls decode behavior:
 *   header[4..5] (int16_t w_04): 0 = check bit 0 of header[3],
 *   1 = already decoded, other = set to 1 and decode.
 * ================================================================ */
void dm2_v1_gdat_decode_sound_sample(DM2_V1_GdatSampleDesc *desc,
                                      DM2_V1_GdatSampleDesc **list_head,
                                      DM2_V1_GdatSampleDecodeReceipt *out)
{
    if (!desc || !list_head) {
        if (out) out->decoded = false;
        return;
    }

    desc->next = *list_head;
    *list_head = desc;

    uint8_t *header = desc->xp_00 - 6;
    int16_t control = (int16_t)(header[4] | (header[5] << 8));

    if (control == 1) {
        if (out) out->decoded = false;
        return;
    }

    bool do_decode;
    if (control != 0) {
        header[4] = 1;
        header[5] = 0;
        do_decode = true;
    } else {
        if ((header[3] & 0x01) == 0) {
            if (out) out->decoded = false;
            return;
        }
        header[3] &= 0xFE;
        do_decode = true;
    }

    if (do_decode) {
        int16_t count = desc->w_04;
        uint8_t *p = desc->xp_00;
        while (count-- > 0)
            *p++ ^= 0x80;
    }

    if (out) out->decoded = true;
}

/* ================================================================
 * DM2_482b_0684 — c_gdatfile.cpp:932-975
 *
 * Resolve deferred sound queue entries. Entries with w_05 == -1
 * have not yet been bound to a GDAT sample. For each:
 *   1. Query GDAT entry data index for (b_02, b_03, 2, b_04).
 *   2. Check if that index is already loaded (via sound7_lookup).
 *   3. If not loaded and room in the pool, bind the raw data and
 *      decode the sample.
 *   4. If already loaded, share the existing pool slot.
 * ================================================================ */
int dm2_v1_gdat_resolve_deferred_sounds(
    DM2_V1_GdatSoundEntry *queue, uint16_t queue_count,
    uint8_t *sample_pool, uint16_t sample_pool_stride,
    uint16_t *active_count, uint16_t max_active,
    int16_t sound_format,
    DM2_V1_GdatSampleDesc **list_head,
    const DM2_V1_GdatResolveSoundCallbacks *cb, void *ctx,
    DM2_V1_GdatResolveSoundReceipt *out)
{
    if (!queue || !cb) return 0;
    int16_t resolved = 0;

    for (uint16_t i = 0; i < queue_count; i++) {
        DM2_V1_GdatSoundEntry *e = &queue[i];
        if (e->w_05 != -1) continue;

        int16_t data_idx = cb->query_entry_data_index(ctx,
            e->b_02, e->b_03, 2, e->b_04);
        uint16_t existing = cb->sound7_lookup(ctx, data_idx);

        if (existing == 0) {
            if (*active_count >= max_active) {
                if (out) out->resolved_count = resolved;
                return 1;
            }
            e->w_05 = data_idx;
            e->w_00 = (int16_t)*active_count;

            int16_t header_skip = (sound_format == 0) ? 2 : 6;
            DM2_V1_GdatSampleDesc *desc = (DM2_V1_GdatSampleDesc *)(void *)
                (sample_pool + sample_pool_stride * (uint16_t)e->w_00);

            uint8_t *raw = cb->query_entry_data_ptr(ctx,
                e->b_02, e->b_03, 2, e->b_04);
            int32_t raw_len = cb->query_entry_data_length(ctx,
                e->b_02, e->b_03, 2, e->b_04);

            desc->xp_00 = raw + header_skip;
            desc->w_04 = (int16_t)(raw_len - header_skip);

            if (sound_format == 0)
                desc->w_06 = 0x157C;
            else
                desc->w_06 = (int16_t)(raw[0] | (raw[1] << 8));

            dm2_v1_gdat_decode_sound_sample(desc, list_head, NULL);
            (*active_count)++;
            resolved++;
        } else {
            e->w_00 = queue[existing - 1].w_00;
            e->w_05 = data_idx;
        }
    }

    if (out) out->resolved_count = resolved;
    return 1;
}

/* skproject: DM2_READ_GRAPHICS_STRUCTURE (bgdat.cpp:1027-1141).
 *
 * The source routine opens GRAPHICS.DAT, validates the four-byte header,
 * constructs the ULP offset table, loads ENT1, optionally retains the
 * underlay table, and configures the image allocator. This compatibility
 * adapter has none of those owners; reporting a valid structure from only
 * pre-filled entries/version words was a synthetic success receipt. Keep
 * the public seam fail-closed until that complete source transaction exists.
 */
int dm2_v1_gdat_read_graphics_structure(DM2_V1_GdatFileState *state,
                                         const DM2_V1_GdatFileCallbacks *cb,
                                         void *ctx,
                                         DM2_V1_GdatReadStructureReceipt *out)
{
    (void)state;
    (void)cb;
    (void)ctx;
    if (out) memset(out, 0, sizeof(*out));
    return 0;
}
