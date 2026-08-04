#ifndef FIRESTAFF_DM2_V1_1031_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_1031_PC34_COMPAT_H

/*
 * dm2_v1_1031_pc34_compat.h -- DM2 segment 1031 UI logic.
 *
 * Source: skproject c_1031.cpp (14 functions).
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

#define DM2_V1_1031_TABLE_BBW_SIZE     0x4C  /* table1d3ba0 */
#define DM2_V1_1031_TABLE_ED5_SIZE     0x0A  /* table1d3ed5 */
#define DM2_V1_1031_TABLE_D23_SIZE     0x3E  /* table1d3d23 */
#define DM2_V1_1031_TABLE_D8_SIZE      0x12  /* table1d32d8 */
#define DM2_V1_1031_TABLE_V1D338C_SIZE 0x108
#define DM2_V1_1031_TABLE_V1D39BC_SIZE 0x79

/* ========================================================================
 * Table entry structs
 * ======================================================================== */

/* s_bbw: 4-byte entry with condition byte, value byte, and index word */
typedef struct DM2_V1_1031_Bbw {
    uint8_t  b_00;    /* condition type | 0x80 = recurse */
    int8_t   b_01;    /* condition value */
    int16_t  w_02;    /* table index or child list index */
} DM2_V1_1031_Bbw;

/* s_wwwb: 7-byte table entry for UI element descriptors */
typedef struct DM2_V1_1031_Wwwb {
    int16_t  w_00;    /* rect query ID */
    int16_t  w_02;    /* v1d338c index */
    int16_t  w_04;    /* v1d39bc index */
    uint8_t  b_06;    /* visibility flags: 0x40=visible, 0x80=hidden */
} DM2_V1_1031_Wwwb;

/* Click rect node: 5-byte structure for clickable areas */
typedef struct DM2_V1_1031_ClickNode {
    int16_t  w_00;
    int16_t  w_02;
    uint8_t  b_03;    /* flags: 0x10=needs show, 0x20=needs hide */
} DM2_V1_1031_ClickNode;

/* s_www: 6-byte click rect entry */
typedef struct DM2_V1_1031_Www {
    int16_t  w_00;    /* ID (bits 0-12) | flags (bits 13-15) */
    int16_t  w_02;    /* rect query ID | offset flags */
    int16_t  w_04;    /* click mask (low byte) | aux (high byte) */
} DM2_V1_1031_Www;

/* ========================================================================
 * Rect struct
 * ======================================================================== */

typedef struct DM2_V1_1031_Rect {
    int16_t x, y, w, h;
} DM2_V1_1031_Rect;

/* ========================================================================
 * UI state
 * ======================================================================== */

typedef struct DM2_V1_1031_State {
    int16_t  current_mode;      /* v1d3ff1 */
    int16_t  saved_mode;        /* v1e0510 */
    int16_t  v1e0204;
    int16_t  v1e0976;
    int16_t  v1e0258;           /* party direction */
    int16_t  v1e00b8;
    int16_t  v1e0b7a;
    int16_t  v1e0b62;
    int16_t  v1d67bc;
    int16_t  v1e03a8;
    int16_t  v1e048c;
    int16_t  v1e0478;
    int16_t  v1e051e;
    int16_t  dialog2;
} DM2_V1_1031_State;

/* ========================================================================
 * Callback struct
 * ======================================================================== */

typedef struct DM2_V1_1031_Callbacks {
    /* Rect queries */
    DM2_V1_1031_Rect *(*query_expanded_rect)(void *ctx, int16_t query,
                                              DM2_V1_1031_Rect *r);
    void (*query_topleft_of_rect)(void *ctx, int16_t query,
                                  int16_t *x, int16_t *y);

    /* Champion/party state */
    int16_t (*get_curacthero)(void *ctx);
    int16_t (*get_hero_hp)(void *ctx, int index);
    int16_t (*get_hero_nrunes)(void *ctx, int index);
    int16_t (*get_player_at_position)(void *ctx, int16_t pos);

    /* Event queue */
    void (*queue_event)(void *ctx, int16_t x, int16_t y, int16_t code);
    int16_t (*get_event_unk05)(void *ctx);
    void (*set_event_unk05)(void *ctx, int16_t val);
    int16_t (*get_event_unk06)(void *ctx);
    void (*set_event_unk06)(void *ctx, int16_t val);
    int16_t (*get_event_unk07)(void *ctx);
    void (*set_event_unk07)(void *ctx, int16_t val);
    int16_t (*get_event_unk08)(void *ctx);
    void (*set_event_unk08)(void *ctx, int16_t val);
    int16_t (*get_event_unk09)(void *ctx);
    void (*set_event_unk09)(void *ctx, int16_t val);
    int16_t (*get_event_unk0a)(void *ctx);
    void (*set_event_unk0a)(void *ctx, int16_t val);

    /* UI drawing */
    void (*guidraw_refresh)(void *ctx);
    void (*refresh_clickrect_1)(void *ctx, int index);
    void (*refresh_clickrect_2)(void *ctx, int index);
    void (*alloc_clickrectdata)(void *ctx, int index);

    /* Mouse capture */
    void (*release_mouse_captures)(void *ctx);
    void (*champion_squad_recompute)(void *ctx);

    /* Button group */
    void (*guidraw_0b36_0c52)(void *ctx, void *buttongroup,
                               int32_t param, int mode);

    /* Map access (for magical map) */
    void (*change_current_map_to)(void *ctx, int32_t map);
    int16_t (*get_current_map)(void *ctx);
    uint8_t (*get_tile_value)(void *ctx, int x, int y);
    void *(*get_record_address)(void *ctx, uint16_t record);
    void *(*get_missile_ref)(void *ctx, int16_t w2, int link);
    void (*set_destination_of_minion_map)(void *ctx, int link,
                                          int x, int y, int map);
    int32_t (*dm_locate_other_level)(void *ctx, int level, int mode,
                                     int16_t *x, int16_t *y, void *extra);
    void (*calc_vector_w_dir)(void *ctx, int8_t dir, int16_t dy, int16_t dx,
                              int16_t *ox, int16_t *oy);
    void (*update_right_panel)(void *ctx, int mode);
    void (*v1c9a_0247)(void *ctx, int32_t param);

    /* Tables (external, read-only) */
    const DM2_V1_1031_Bbw  *table_bbw;      /* table1d3ba0 */
    const DM2_V1_1031_Bbw  *table_ed5;      /* table1d3ed5 */
    DM2_V1_1031_Wwwb       *table_d23;      /* table1d3d23 (mutable) */
    DM2_V1_1031_ClickNode  *table_d8;       /* table1d32d8 (mutable) */
    const int8_t           *table_cd0;       /* table1d3cd0 */
    DM2_V1_1031_Www        *table_338c;      /* v1d338c */
    DM2_V1_1031_Www        *table_39bc;      /* v1d39bc */

    /* Event data */
    void (*set_ev_table)(void *ctx, void *ptr);

    /* Magical map state */
    int16_t (*get_v1d274a)(void *ctx);
    int16_t (*get_v1d274c)(void *ctx);
    int16_t (*get_v1d274e)(void *ctx);
    int16_t (*get_v1d2750)(void *ctx);

    void *ctx;
} DM2_V1_1031_Callbacks;

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_1031_HitTestReceipt {
    int32_t  result;        /* nonzero if hit */
} DM2_V1_1031_HitTestReceipt;

/* ========================================================================
 * Public functions
 * ======================================================================== */

/*
 * Query expanded rect with optional offset for viewport/panel.
 * Source: DM2_1031_01d5 in c_1031.cpp.
 */
DM2_V1_1031_Rect *dm2_v1_1031_query_rect_with_offset(
    const DM2_V1_1031_Callbacks *cb,
    int16_t query, DM2_V1_1031_Rect *r);

/*
 * Evaluate a UI gate condition (12 cases).
 * Source: gate_1031 in c_1031.cpp.
 */
bool dm2_v1_1031_gate(
    const DM2_V1_1031_Callbacks *cb,
    const DM2_V1_1031_State *state,
    int32_t condition, const DM2_V1_1031_Bbw *entry);

/*
 * Recursively walk UI tree, marking visible elements.
 * Source: DM2_1031_027e in c_1031.cpp.
 */
void dm2_v1_1031_mark_visible(
    const DM2_V1_1031_Callbacks *cb,
    const DM2_V1_1031_State *state,
    const DM2_V1_1031_Bbw *root);

/*
 * Recursive hit-test in UI tree.
 * Source: DM2_1031_030a in c_1031.cpp.
 */
DM2_V1_1031_HitTestReceipt dm2_v1_1031_hit_test(
    const DM2_V1_1031_Callbacks *cb,
    const DM2_V1_1031_State *state,
    const DM2_V1_1031_Bbw *root,
    int16_t x, int16_t y, int16_t mask);

/*
 * Main UI state update loop.
 * Source: DM2_1031_0541 in c_1031.cpp.
 */
int32_t dm2_v1_1031_update_state(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state,
    int16_t mode);

/*
 * Switch to a new UI mode (saves current).
 * Source: DM2_1031_0675 in c_1031.cpp.
 */
int32_t dm2_v1_1031_switch_mode(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state,
    int16_t new_mode);

/*
 * Restore previous UI mode.
 * Source: DM2_1031_06a5 in c_1031.cpp.
 */
int32_t dm2_v1_1031_restore_mode(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state);

/*
 * Reset UI capture state.
 * Source: DM2_10777 in c_1031.cpp.
 */
void dm2_v1_1031_reset_state(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state);

/*
 * Hit-test click rect entries.
 * Source: DM2_1031_0a88 in c_1031.cpp.
 */
int32_t dm2_v1_1031_hit_test_clickrects(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state,
    const DM2_V1_1031_Www *entries,
    int16_t x, int16_t y, int16_t mask);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_1031_PC34_COMPAT_H */
