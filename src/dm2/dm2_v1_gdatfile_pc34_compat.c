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

/* skproject: DM2_READ_GRAPHICS_STRUCTURE (c_gdatfile.cpp:1026-1128)
 * Stub — full implementation requires many subsystems (dballoc, ulp, etc.) */
int dm2_v1_gdat_read_graphics_structure(DM2_V1_GdatFileState *state,
                                         const DM2_V1_GdatFileCallbacks *cb,
                                         void *ctx,
                                         DM2_V1_GdatReadStructureReceipt *out)
{
    if (!state || !cb) return 0;
    (void)ctx;
    if (out) {
        out->valid = true;
        out->entries = state->entries;
        out->versionlo = state->versionlo;
    }
    return 1;
}
