#ifndef FIRESTAFF_DM2_V1_0AAF_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_0AAF_PC34_COMPAT_H

/*
 * dm2_v1_0aaf_pc34_compat.h -- DM2 segment 0AAF dialogue/menu logic.
 *
 * Source: skproject c_0aaf.cpp (3 functions).
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

#define DM2_V1_0AAF_MAX_TEXT_ENTRIES   20
#define DM2_V1_0AAF_TEXT_BUF_SIZE      0x50
#define DM2_V1_0AAF_MAX_CHOICE_PARTS   2

/* ========================================================================
 * Dialogue part descriptor
 * ======================================================================== */

typedef struct DM2_V1_0aafTextPart {
    const char *text;
    int16_t     text_width;
    int16_t     text_height;
} DM2_V1_0aafTextPart;

/* ========================================================================
 * Rect struct (minimal)
 * ======================================================================== */

typedef struct DM2_V1_0aafRect {
    int16_t x, y, w, h;
} DM2_V1_0aafRect;

/* ========================================================================
 * Callback struct
 * ======================================================================== */

typedef struct DM2_V1_0aafCallbacks {
    /* GDAT text query */
    const char *(*query_gdat_text)(void *ctx, uint8_t cat, uint8_t type,
                                   uint8_t sub, char *buf);
    /* GDAT data index query */
    int16_t (*query_gdat_entry_data_index)(void *ctx, uint8_t cat,
                                           uint8_t type, uint8_t sub1,
                                           uint8_t sub2);
    /* GDAT image query */
    void *(*query_gdat_image_entry_buff)(void *ctx, uint8_t cat,
                                         uint8_t type, uint8_t sub);
    void *(*query_gdat_image_localpal)(void *ctx, uint8_t cat,
                                       uint8_t type, uint8_t sub);
    bool (*query_gdat_entry_if_loadable)(void *ctx, uint8_t cat,
                                         uint8_t type, uint8_t sub1,
                                         uint8_t sub2);

    /* Image metrics */
    int16_t (*get_image_width)(void *ctx, void *bmp);
    int16_t (*get_image_height)(void *ctx, void *bmp);
    int32_t (*calc_image_byte_length)(void *ctx, void *bmp);

    /* Drawing */
    void (*fill_backbuff_rect)(void *ctx, const DM2_V1_0aafRect *r,
                               uint8_t pixel);
    void (*draw_dialogue_parts_pict)(void *ctx, void *bmp,
                                     const DM2_V1_0aafRect *r,
                                     int16_t mode, void *pal);
    void (*draw_vp_rc_str)(void *ctx, int16_t rect_id, int16_t color,
                           const char *text);
    void (*draw_vp_str)(void *ctx, int16_t x, int16_t y, int16_t color,
                        const char *text);
    void (*free_pict_entry)(void *ctx, void *pixels);
    void (*draw_gameload_dialogue_to_screen)(void *ctx);

    /* Rect queries */
    DM2_V1_0aafRect *(*query_expanded_rect)(void *ctx, int16_t query,
                                             DM2_V1_0aafRect *r);
    void (*query_topleft_of_rect)(void *ctx, int16_t query,
                                  int16_t *x, int16_t *y);

    /* String metrics */
    void (*query_str_metrics)(void *ctx, const char *text,
                              int16_t *w, int16_t *h);
    int16_t (*gfxstr_split_line)(void *ctx, const char *text, char *buf,
                                 int16_t *pos, int16_t max_width);

    /* Mouse */
    void (*show_mouse)(void *ctx);
    void (*hide_mouse)(void *ctx);
    bool (*is_mouse_visible)(void *ctx);

    /* Event loop */
    void (*event_loop)(void *ctx);
    void (*wait_screen_refresh)(void *ctx);
    bool (*has_key)(void *ctx);
    int16_t (*getkey_translated)(void *ctx);

    /* Palette */
    uint8_t (*palette_to_ui8)(void *ctx, int color_index);

    /* UI state */
    void (*fade_screen)(void *ctx, int mode);
    void (*sleep_several_time)(void *ctx, int ticks);

    /* Backbuffer dims */
    int16_t (*get_backbuffer_w)(void *ctx);
    int16_t (*get_backbuffer_h)(void *ctx);

    /* Font metrics */
    int16_t (*get_gfxstr_w1)(void *ctx);   /* strdat.gfxstrw1 */
    int16_t (*get_gfxstr_w3)(void *ctx);   /* strdat.gfxstrw3 */
    int16_t (*get_str_xplus)(void *ctx);   /* strdat.strxplus */
    int16_t (*get_v1d2736)(void *ctx);     /* ddat.v1d2736 */

    /* 1031 UI mode */
    int32_t (*mode_1031_0675)(void *ctx, int16_t mode);
    void (*mode_1031_06a5)(void *ctx);
    void (*mode_1031_0781)(void *ctx, int32_t param);

    /* Global state */
    bool (*get_v1e0a88)(void *ctx);        /* image-based dialogue flag */
    bool (*get_gfxalloc_done)(void *ctx);  /* dballochandler flag */
    int16_t (*get_dialog2)(void *ctx);     /* ddat.dialog2 */
    int16_t (*get_v1e0204)(void *ctx);
    void (*set_v1e0204)(void *ctx, int16_t val);
    void (*set_backbuff2)(void *ctx, int16_t val);

    /* Event state */
    int16_t (*get_event_unk06)(void *ctx);
    void (*set_event_unk06)(void *ctx, int16_t val);
    int16_t (*get_event_unk09)(void *ctx);
    int16_t (*get_event_unk0a)(void *ctx);

    /* Image entry state */
    uint8_t (*get_v1e0206)(void *ctx);
    uint8_t (*get_v1e0207)(void *ctx);
    uint8_t (*get_v1e0208)(void *ctx);

    /* Table references */
    int16_t (*get_table1d27c4)(void *ctx, int index);
    int16_t (*get_table1d27d4)(void *ctx, int index);

    /* v1d1044 text */
    const char *(*get_v1d1044)(void *ctx);

    /* Bigpool check */
    int32_t (*bigpool_available)(void *ctx);

    void *ctx;
} DM2_V1_0aafCallbacks;

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_0aafMenuReceipt {
    int16_t  selection;     /* selected menu item or key code */
} DM2_V1_0aafMenuReceipt;

typedef struct DM2_V1_0aafDialogueReceipt {
    int32_t  result;        /* return value from dialogue construction */
    uint8_t  dialogue_type; /* the effective type used */
} DM2_V1_0aafDialogueReceipt;

/* ========================================================================
 * Public functions
 * ======================================================================== */

/*
 * Display menu and wait for selection.
 * Source: DM2_0aaf_0067 in c_0aaf.cpp.
 */
DM2_V1_0aafMenuReceipt dm2_v1_0aaf_menu_select(
    const DM2_V1_0aafCallbacks *cb,
    int32_t menu_param);

/*
 * Draw a single dialogue part (palette fill or image).
 * Source: DM2_0aaf_01db in c_0aaf.cpp.
 */
void dm2_v1_0aaf_draw_part(
    const DM2_V1_0aafCallbacks *cb,
    int16_t rect_id, int32_t draw_mode);

/*
 * Construct and display full dialogue with text/images.
 * Source: DM2_0aaf_02f8 in c_0aaf.cpp (recursive).
 */
DM2_V1_0aafDialogueReceipt dm2_v1_0aaf_construct_dialogue(
    const DM2_V1_0aafCallbacks *cb,
    int32_t type_param, int32_t sub_param);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_0AAF_PC34_COMPAT_H */
