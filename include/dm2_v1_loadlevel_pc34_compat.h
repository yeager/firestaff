#ifndef FIRESTAFF_DM2_V1_LOADLEVEL_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_LOADLEVEL_PC34_COMPAT_H

/*
 * dm2_v1_loadlevel_pc34_compat.h -- DM2 level loading/initialization.
 *
 * Source: skproject c_loadlevel.cpp (9 functions).
 * All public functions use callback-based architecture.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Constants
 * ======================================================================== */

#define DM2_V1_LOADLEVEL_MAX_DYN_ENTRIES   400   /* 0x960 / 6 bytes each */
#define DM2_V1_LOADLEVEL_CREATURE_TABLE_SIZE 250  /* 0xFA */
#define DM2_V1_LOADLEVEL_DYN_FLAG_HIRES    0x8000

/* ========================================================================
 * Dynamic load entry
 * ======================================================================== */

typedef struct DM2_V1_DynLoadEntry {
    int16_t  flags;          /* @0: flags / override value */
    uint8_t  cat;            /* @2: GDAT category */
    uint8_t  type;           /* @3: GDAT type */
    uint8_t  sub1;           /* @4: GDAT sub1 */
    uint8_t  sub2;           /* @5: GDAT sub2 */
} DM2_V1_DynLoadEntry;

/* ========================================================================
 * DynLoad state
 * ======================================================================== */

typedef struct DM2_V1_DynLoadState {
    DM2_V1_DynLoadEntry entries[DM2_V1_LOADLEVEL_MAX_DYN_ENTRIES];
    int16_t             count;     /* v1e09a4 */
} DM2_V1_DynLoadState;

/* ========================================================================
 * Misc item sorted list
 * ======================================================================== */

#define DM2_V1_LOADLEVEL_MAX_MISC_ITEMS  40

typedef struct DM2_V1_MiscItemState {
    int16_t  sort_keys[DM2_V1_LOADLEVEL_MAX_MISC_ITEMS];   /* v1e03ac */
    int16_t  sort_vals[DM2_V1_LOADLEVEL_MAX_MISC_ITEMS];   /* v1e0394 */
    int16_t  count;           /* v1e03fe */
    bool     loaded;          /* v1d268a */
} DM2_V1_MiscItemState;

/* ========================================================================
 * Level graphics table state
 * ======================================================================== */

typedef struct DM2_V1_LevelGraphicsState {
    int16_t  party_x;         /* v1e0270 */
    int16_t  party_y;         /* v1e0272 */
    int16_t  view_map;        /* v1e0266 */
    uint8_t  wall_set[16];    /* v1e02cc */
    uint8_t  floor_set[16];   /* v1e02dc */
    uint8_t  door_set[16];    /* v1e0414 */
} DM2_V1_LevelGraphicsState;

/* ========================================================================
 * Callback struct
 * ======================================================================== */

typedef struct DM2_V1_LoadLevelCallbacks {
    /* Memory allocation */
    void *(*alloc_lobigpool)(void *ctx, int32_t size, bool clear);
    void  (*dealloc_lobigpool)(void *ctx, int32_t size);
    int32_t (*bigpool_available)(void *ctx);

    /* GDAT queries */
    int16_t (*query_gdat_entry_data_index)(void *ctx, uint8_t cat,
                                            uint8_t type, uint8_t sub1,
                                            uint8_t sub2);
    bool (*query_gdat_entry_if_loadable)(void *ctx, uint8_t cat,
                                         uint8_t type, uint8_t sub1,
                                         uint8_t sub2);
    void (*query_gdat_image_metrics)(void *ctx, uint8_t cat, uint8_t type,
                                     uint8_t sub, int16_t *w, int16_t *h);
    int16_t (*query_gdat_creature_word_value)(void *ctx, int idx, int field);

    /* Map access */
    void (*change_current_map_to)(void *ctx, int32_t map);
    int16_t (*get_current_map)(void *ctx);
    int16_t (*get_map_width)(void *ctx);
    int16_t (*get_map_height)(void *ctx);
    uint8_t (*get_tile_byte)(void *ctx, int x, int y);
    int16_t (*get_tile_record_link)(void *ctx, int x, int y);
    uint8_t *(*get_record_address)(void *ctx, uint16_t record);
    int32_t (*get_next_record_link)(void *ctx, uint16_t record);

    /* Actuator handling */
    void (*invoke_actuator)(void *ctx, uint8_t *record, int action, int delay);

    /* Creature allocation */
    void (*alloc_new_creature)(void *ctx, int16_t type, int dir,
                               int flags, int x, int y);

    /* Level geometry */
    void *(*get_level_sizee)(void *ctx);         /* v1e03c0 */
    void *(*get_level_sizee_array)(void *ctx);   /* v1e03c8 */
    void *(*get_tile_map_ptr)(void *ctx);        /* map */
    void *(*get_record_base)(void *ctx);         /* dm2_v1e038c */
    int16_t *(*get_graphics_index)(void *ctx);   /* v1e03d8 */
    int16_t *(*get_graphics_offset)(void *ctx);  /* v1e03e4 */
    int16_t *(*get_graphics_ref)(void *ctx);     /* v1e03f4 */

    /* Dynamic resource loading */
    void (*load_dyn4)(void *ctx, int16_t *entries, int count);
    void (*sound_init)(void *ctx);

    /* Party */
    int16_t (*get_party_count)(void *ctx);
    int16_t (*get_hero_type)(void *ctx, int index);
    int16_t (*get_hero_item)(void *ctx, int hero, int slot);

    /* Mouse */
    void (*show_mouse)(void *ctx);
    void (*hide_mouse)(void *ctx);

    /* Weather/light */
    void (*update_weather)(void *ctx, int mode);
    int32_t (*check_recompute_light)(void *ctx);
    int32_t (*fill_caii_cur_map)(void *ctx);

    /* UI */
    void (*event_1031_098e)(void *ctx);

    /* Flags */
    bool (*get_v1e0a84)(void *ctx);         /* dballochandler flag */
    int16_t (*get_v1e13fe_0)(void *ctx);    /* v1e13fe[0] */
    int16_t (*get_v1e13fe_1)(void *ctx);    /* v1e13fe[1] */

    /* Music map */
    uint8_t (*get_music_map_entry)(void *ctx, int map);

    /* Ornament animation */
    int16_t (*get_ornate_anim_len)(void *ctx, void *record, int flag);
    void (*try_ornate_noise)(void *ctx, void *record, int link,
                             int x, int y, int16_t len, int flag);

    /* Random */
    int16_t (*randdir)(void *ctx);

    /* Backbuffer state */
    void (*set_backbuff_rect)(void *ctx, int16_t x, int16_t y,
                              int16_t w, int16_t h);
    void (*set_backbuff1)(void *ctx, int16_t val);
    void (*set_backbuff2)(void *ctx, int16_t val);
    int16_t (*get_backbuffer_w)(void *ctx);
    int16_t (*get_backbuffer_h)(void *ctx);
    int16_t (*get_v1d2708)(void *ctx);

    /* Viewport recompute flag */
    void (*set_viewport_dirty)(void *ctx, uint32_t flags);

    void *ctx;
} DM2_V1_LoadLevelCallbacks;

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_LoadLevelReceipt {
    bool     loaded;
    int16_t  dyn_count;    /* number of dynamic resources marked */
} DM2_V1_LoadLevelReceipt;

typedef struct DM2_V1_MarkDynLoadReceipt {
    int16_t  entry_index;  /* index of the added entry */
} DM2_V1_MarkDynLoadReceipt;

/* ========================================================================
 * Public functions
 * ======================================================================== */

/*
 * Mark a resource for dynamic loading.
 * Source: DM2_MARK_DYN_LOAD in c_loadlevel.cpp.
 */
DM2_V1_MarkDynLoadReceipt dm2_v1_mark_dyn_load(
    DM2_V1_DynLoadState *state,
    int32_t resource_id);

/*
 * Mark with override flag (0x8001).
 * Source: DM2_2676_008f in c_loadlevel.cpp.
 */
void dm2_v1_mark_dyn_load_with_flag(
    DM2_V1_DynLoadState *state,
    int32_t resource_id, int32_t flag);

/*
 * Mark GDAT entry and its high byte if present.
 * Source: DM2_2676_00d0 in c_loadlevel.cpp.
 */
void dm2_v1_mark_dyn_load_gdat_entry(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_DynLoadState *state,
    uint8_t cat, uint8_t type, uint8_t sub);

/*
 * Mark with hi-res flag (0x8000).
 * Source: DM2_2676_006a in c_loadlevel.cpp.
 */
void dm2_v1_mark_dyn_load_hires(
    DM2_V1_DynLoadState *state,
    int32_t resource_id);

/*
 * Load misc items into sorted list.
 * Source: DM2_LOAD_MISCITEM in c_loadlevel.cpp.
 */
void dm2_v1_load_miscitem(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_MiscItemState *misc);

/*
 * Load all dynamic resources for current level.
 * Source: DM2_LOAD_LOCALLEVEL_DYN in c_loadlevel.cpp.
 */
DM2_V1_LoadLevelReceipt dm2_v1_load_locallevel_dyn(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_DynLoadState *dyn,
    DM2_V1_MiscItemState *misc,
    DM2_V1_LevelGraphicsState *gfx);

/*
 * Set up graphics table for current level.
 * Source: DM2_LOAD_LOCALLEVEL_GRAPHICS_TABLE in c_loadlevel.cpp.
 */
void dm2_v1_load_locallevel_graphics_table(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_LevelGraphicsState *gfx,
    int16_t x, int16_t y, int16_t map);

/*
 * Process actuators on level entry/exit.
 * Source: DM2_3a15_38b6 in c_loadlevel.cpp.
 */
void dm2_v1_process_level_actuators(
    const DM2_V1_LoadLevelCallbacks *cb,
    int32_t enter_flag);

/*
 * Top-level map loading orchestrator.
 * Source: DM2_LOAD_NEWMAP in c_loadlevel.cpp.
 */
void dm2_v1_load_newmap(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_DynLoadState *dyn,
    DM2_V1_MiscItemState *misc,
    DM2_V1_LevelGraphicsState *gfx,
    int16_t x, int16_t y, int16_t map, int32_t process_flag);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_LOADLEVEL_PC34_COMPAT_H */
