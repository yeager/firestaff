#ifndef FIRESTAFF_DM2_V1_GDATFILE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_GDATFILE_PC34_COMPAT_H

/*
 * dm2_v1_gdatfile_pc34_compat.h — DM2 GDAT file handling module.
 *
 * Source: skproject c_gdatfile.cpp / c_gdatfile.h.
 * GDAT is the graphics data archive format used by DM2.
 * All public functions use callback-based architecture.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Packed data types from skproject
 * ======================================================================== */

#pragma pack(push, 1)

/*
 * s_hex6 — 6-byte query descriptor.
 * skproject: struct s_hex6 in c_dballoc.h.
 * Used to specify a GDAT lookup key (category, subcategory, field selectors).
 */
typedef struct DM2_V1_GdatHex6 {
    int16_t w_00;   /* primary key / category word */
    int8_t  b_02;   /* subcategory byte 0 */
    int8_t  b_03;   /* subcategory byte 1 */
    int8_t  b_04;   /* field selector 0 */
    int8_t  b_05;   /* field selector 1 */
} DM2_V1_GdatHex6;

/*
 * s_bbw — 4-byte entry record.
 * skproject: struct s_bbw in xtypes.h.
 * Compact record pairing two byte fields with a word field.
 */
typedef struct DM2_V1_GdatBBW {
    int8_t  b_00;   /* entry byte 0 (type/flags) */
    int8_t  b_01;   /* entry byte 1 (subtype) */
    int16_t w_02;   /* entry word (data index) */
} DM2_V1_GdatBBW;

#pragma pack(pop)

/*
 * s_gdat — query state structure (size 0x20).
 * skproject: struct s_gdat in c_gdatfile.h.
 * Holds the current query context for iterating GDAT entries.
 */
typedef struct DM2_V1_GdatQueryState {
    int32_t          l_00;       /* initialization flag */
    DM2_V1_GdatHex6  s19_04;    /* primary query descriptor */
    DM2_V1_GdatHex6  s19_0a;    /* secondary query descriptor (range mode) */
    DM2_V1_GdatBBW  *u31p_10;   /* pointer to current entry record */
    int8_t           b_14;      /* outer table low bound */
    int8_t           b_15;      /* outer table high bound */
    int8_t           b_16;      /* inner table low bound */
    int8_t           b_17;      /* inner table high bound */
    int16_t          w_18;      /* current inner table position */
    int16_t          w_1a;      /* current entry index */
    int16_t          w_1c;      /* entry limit */
    int16_t          w_1e;      /* reserved */
} DM2_V1_GdatQueryState;

/*
 * s_1e09e0 — GDAT entry table structure (size 0x2c).
 * skproject: struct s_1e09e0 in c_dballoc.h.
 * Three-level lookup: w_table1[category] -> w_table2[subcat] -> u31p_08[entry].
 */
typedef struct DM2_V1_GdatTable {
    int16_t         *w_table1;   /* category index table */
    int16_t         *w_table2;   /* subcategory index table */
    DM2_V1_GdatBBW  *u31p_08;   /* flat entry record array */
    int16_t          entries;    /* max category index */
    int16_t          w_0e;      /* total subcategory slots */
    int16_t          w_10;      /* total entry records */
    int16_t          w_12;      /* accumulated record field size */
    int16_t          w_14;      /* number of active fields */
    int16_t          warr_16[7]; /* per-field offset accumulator */
    int8_t           barr_24[8]; /* per-field byte sizes */
} DM2_V1_GdatTable;

/* ========================================================================
 * GDAT file state — corresponds to c_gdatfile class
 * ======================================================================== */

typedef struct DM2_V1_GdatFileState {
    int16_t      fileopencounter;
    int16_t      filehandle;
    int16_t      xfilehandle;
    int32_t      filesize;
    int16_t      versionlo;
    uint16_t     entries;
    bool         filetype1;
    bool         filetype2;
    const char  *filename1;  /* .Z020GRAPHICS.DAT */
    const char  *filename2;  /* .Z026GRAPHIC2.DAT */
    const char  *filename3;  /* .Z020DUNGENB.DAT  */
    const char  *filename4;  /* .Z020DUNGEON.DAT  */
    const char  *filename5;  /* .Z022SKSAVE.Z023.DAT */
    const char  *filename6;  /* .Z022SKSAVE.Z023.BAK */
    const char  *filename7;  /* .Z020DUNGEON.FTL  */
} DM2_V1_GdatFileState;

/* ========================================================================
 * Callback struct — external dependencies
 * ======================================================================== */

typedef struct DM2_V1_GdatFileCallbacks {
    /* File I/O */
    int16_t (*file_open)(void *ctx, const char *name);
    void    (*file_close)(void *ctx, int16_t handle);
    bool    (*file_read)(void *ctx, int16_t handle, void *buf, int32_t len);
    bool    (*file_seek)(void *ctx, int16_t handle, int32_t pos);
    int32_t (*get_file_size)(void *ctx, int16_t handle);

    /* Memory */
    uint8_t *(*alloc_memory)(void *ctx, int32_t size, int16_t pool);
    void     (*dealloc_memory)(void *ctx, int32_t size);

    /* Utility */
    void (*copy_memory)(void *ctx, void *dest, const void *src, int32_t len);
    void (*zero_memory)(void *ctx, void *dest, int32_t len);
    void (*raise_syserr)(void *ctx, int32_t code);

    /* GDAT queries */
    int32_t (*query_raw_data_length)(void *ctx, int16_t dbidx);
    const char *(*format_skstr)(void *ctx, const char *name);
} DM2_V1_GdatFileCallbacks;

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_GdatFileInitReceipt {
    bool valid;
} DM2_V1_GdatFileInitReceipt;

typedef struct DM2_V1_GdatOpenReceipt {
    bool    opened;
    int16_t filehandle;
    int16_t xfilehandle;
    int16_t open_count;
} DM2_V1_GdatOpenReceipt;

typedef struct DM2_V1_GdatCloseReceipt {
    bool    closed;
    int16_t open_count;
} DM2_V1_GdatCloseReceipt;

typedef struct DM2_V1_GdatReadReceipt {
    bool    success;
    int32_t bytes_read;
} DM2_V1_GdatReadReceipt;

typedef struct DM2_V1_GdatQueryNextEntryReceipt {
    bool             found;
    DM2_V1_GdatBBW  *entry;
    int8_t           table_index;
    int8_t           sub_index;
} DM2_V1_GdatQueryNextEntryReceipt;

typedef struct DM2_V1_GdatEntryValueReceipt {
    int32_t value;
    bool    valid;
} DM2_V1_GdatEntryValueReceipt;

typedef struct DM2_V1_GdatRawDataFilePosReceipt {
    int32_t position;
    int16_t cached_idx;
    int32_t cached_offset;
} DM2_V1_GdatRawDataFilePosReceipt;

typedef struct DM2_V1_GdatLoadRawDataReceipt {
    bool    success;
    int32_t bytes_loaded;
} DM2_V1_GdatLoadRawDataReceipt;

typedef struct DM2_V1_GdatAllocPictReceipt {
    void   *bmp;
    int16_t width;
    int16_t height;
    int8_t  bpp;
    int32_t buffer_size;
} DM2_V1_GdatAllocPictReceipt;

typedef struct DM2_V1_GdatReadStructureReceipt {
    bool     valid;
    uint16_t entries;
    int16_t  versionlo;
} DM2_V1_GdatReadStructureReceipt;

/* ========================================================================
 * Public functions
 * ======================================================================== */

/* skproject: c_gdatfile::init (c_gdatfile.cpp:349) */
int dm2_v1_gdat_file_init(DM2_V1_GdatFileState *state,
                           DM2_V1_GdatFileInitReceipt *out);

/* skproject: DM2_GRAPHICS_DATA_OPEN (c_gdatfile.cpp:368) */
int dm2_v1_gdat_graphics_data_open(DM2_V1_GdatFileState *state,
                                    const DM2_V1_GdatFileCallbacks *cb,
                                    void *ctx,
                                    DM2_V1_GdatOpenReceipt *out);

/* skproject: DM2_GRAPHICS_DATA_CLOSE (c_gdatfile.cpp:384) */
int dm2_v1_gdat_graphics_data_close(DM2_V1_GdatFileState *state,
                                     const DM2_V1_GdatFileCallbacks *cb,
                                     void *ctx,
                                     DM2_V1_GdatCloseReceipt *out);

/* skproject: DM2_GRAPHICS_DATA_READ (c_gdatfile.cpp:399) */
int dm2_v1_gdat_graphics_data_read(DM2_V1_GdatFileState *state,
                                    int32_t offset, int32_t length,
                                    uint8_t *dest,
                                    const DM2_V1_GdatFileCallbacks *cb,
                                    void *ctx,
                                    DM2_V1_GdatReadReceipt *out);

/* skproject: DM2_QUERY_NEXT_GDAT_ENTRY (c_gdatfile.cpp:31) */
int dm2_v1_gdat_query_next_entry(DM2_V1_GdatQueryState *qs,
                                  const DM2_V1_GdatTable *table,
                                  DM2_V1_GdatQueryNextEntryReceipt *out);

/* skproject: DM2_QUERY_GDAT_ENTRY_VALUE (c_gdatfile.cpp:515) */
int dm2_v1_gdat_query_entry_value(const uint8_t *entry_data,
                                   int16_t entry_idx,
                                   int16_t field_idx,
                                   const int16_t *field_offsets,
                                   const uint8_t *field_sizes,
                                   int16_t record_size,
                                   DM2_V1_GdatEntryValueReceipt *out);

/* skproject: DM2_QUERY_GDAT_RAW_DATA_FILE_POS (c_gdatfile.cpp:454) */
int dm2_v1_gdat_raw_data_file_pos(int16_t dbidx, int32_t base_offset,
                                   int16_t cached_idx,
                                   int32_t cached_offset,
                                   const DM2_V1_GdatFileCallbacks *cb,
                                   void *ctx,
                                   DM2_V1_GdatRawDataFilePosReceipt *out);

/* skproject: DM2_LOAD_GDAT_RAW_DATA (c_gdatfile.cpp:470) */
int dm2_v1_gdat_load_raw_data(DM2_V1_GdatFileState *state,
                               int16_t dbidx, uint8_t *dest,
                               int32_t data_length,
                               int32_t file_pos,
                               const DM2_V1_GdatFileCallbacks *cb,
                               void *ctx,
                               DM2_V1_GdatLoadRawDataReceipt *out);

/* skproject: DM2_ALLOC_PICT_BUFF (c_gdatfile.cpp:529) */
int dm2_v1_gdat_alloc_pict_buff(int16_t width, int16_t height,
                                 int16_t pool, int8_t bpp,
                                 const DM2_V1_GdatFileCallbacks *cb,
                                 void *ctx,
                                 DM2_V1_GdatAllocPictReceipt *out);

/* skproject: DM2_FREE_PICT_BUFF (c_gdatfile.cpp:548) */
int dm2_v1_gdat_free_pict_buff(int16_t width, int16_t height,
                                int8_t bpp,
                                const DM2_V1_GdatFileCallbacks *cb,
                                void *ctx);

/* skproject: R_2BAD4 (c_gdatfile.cpp:575) — byte-swap 16-bit */
int16_t dm2_v1_gdat_byte_swap_16(int16_t value);

/* skproject: DM2_ALLOC_NEW_BMP (c_gdatfile.cpp:560) */
int dm2_v1_gdat_alloc_new_bmp(int16_t dbidx, int16_t width, int16_t height,
                               int8_t bpp,
                               const DM2_V1_GdatFileCallbacks *cb,
                               void *ctx,
                               DM2_V1_GdatAllocPictReceipt *out);

/* skproject: DM2_LOAD_GDAT_ENTRY_DATA_TO (c_gdatfile.cpp:818)
 * Load raw GDAT data for a given cls1/cls2/type/idx into dest buffer. */
typedef struct {
    int16_t (*query_entry_data_index)(void *ctx, int8_t cls1, int8_t cls2,
                                      int8_t entry_type, int8_t data_idx);
    int (*load_raw_data)(void *ctx, int16_t dbidx, uint8_t *dest);
} DM2_V1_GdatLoadEntryCallbacks;

int dm2_v1_gdat_load_entry_data_to(int8_t cls1, int8_t cls2,
                                    int8_t entry_type, int8_t data_idx,
                                    uint8_t *dest,
                                    const DM2_V1_GdatLoadEntryCallbacks *cb,
                                    void *ctx);

/* skproject: DM2_TRACK_UNDERLAY (c_gdatfile.cpp:997)
 * Binary search in underlay table for a matching dbidx. */
typedef struct {
    int32_t  found;
    int16_t  value;
} DM2_V1_GdatTrackUnderlayReceipt;

int dm2_v1_gdat_track_underlay(uint16_t dbidx,
                                const uint8_t *table, int16_t table_count,
                                DM2_V1_GdatTrackUnderlayReceipt *out);

/* skproject: DM2_READ_GRAPHICS_STRUCTURE (c_gdatfile.cpp:1026) */
int dm2_v1_gdat_read_graphics_structure(DM2_V1_GdatFileState *state,
                                         const DM2_V1_GdatFileCallbacks *cb,
                                         void *ctx,
                                         DM2_V1_GdatReadStructureReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GDATFILE_PC34_COMPAT_H */
