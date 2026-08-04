#ifndef FIRESTAFF_DM2_V1_TMOUSE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_TMOUSE_PC34_COMPAT_H

/*
 * dm2_v1_tmouse_pc34_compat.h — DM2 mouse/touch input subsystem.
 *
 * Source: skproject c_tmouse.cpp (598 lines, 26 functions).
 *
 * Covers:
 *   - Command queue (circular FIFO for server commands)
 *   - Mouse event queue (circular FIFO for raw mouse input)
 *   - Mouse state machine (Tmouse): init, visibility, capture,
 *     block/unblock, command interpreter, event dispatch
 *   - Click-rect hit testing (T1_queue_0x20)
 *   - Event routing (T1_queue_event)
 *   - Driver mouse interrupt handler (T1_driver_mouseint)
 *   - Public API: HIDE/SHOW_MOUSE, SET/RELEASE_CAPTURE,
 *     REFRESHMOUSE, CHOOSE_CURSOR3, RELEASE_MOUSE_CAPTURES,
 *     GET_MOUSE_ENTRY_DATA
 *   - Mouse blitter (blit, blit_hline, blit_hline_masked,
 *     blit_hline_stretched) — cursor compositing
 *   - T1_drawmouse — cursor rendering with rect2 boundary check
 *
 * All public and internal functions use callback-based architecture.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Constants
 * ======================================================================== */

#define DM2_TMOUSE_COMMAND_QUEUE_LENGTH  10
#define DM2_TMOUSE_MOUSE_QUEUE_LENGTH   10
#define DM2_TMOUSE_ORIG_SWIDTH         320
#define DM2_TMOUSE_ORIG_SHEIGHT        200
#define DM2_TMOUSE_NUM_CURSORS           4

/* ========================================================================
 * Cursor index enum
 * ======================================================================== */

typedef enum {
    DM2_TMOUSE_NOCURSOR = -1,
    DM2_TMOUSE_CURSOR0  =  0,
    DM2_TMOUSE_CURSOR1  =  1,
    DM2_TMOUSE_CURSOR2  =  2,
    DM2_TMOUSE_CURSOR3  =  3
} DM2_TmouseCursorIdx;

/* ========================================================================
 * Event entry — raw mouse input (x, y, buttons)
 * ======================================================================== */

typedef struct {
    int16_t x;
    int16_t y;
    int16_t b;  /* button mask: bit 0 = left, bit 1 = right */
} DM2_TmouseEventEntry;

/* ========================================================================
 * Rectangle
 * ======================================================================== */

typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} DM2_TmouseRect;

/* ========================================================================
 * Extended mouse rect (rect + button mask)
 * ======================================================================== */

typedef struct {
    DM2_TmouseRect rc;
    int16_t        b;   /* button event code */
} DM2_TmouseXMouseRect;

/* ========================================================================
 * Click-rect node — linked list element for hit-test regions
 * ======================================================================== */

typedef struct DM2_TmouseClickRectData DM2_TmouseClickRectData;

typedef struct DM2_TmouseClickRectNode {
    int16_t  w_00;
    int8_t   b_02;       /* cursor-type indicator */
    int8_t   b_03;
    int8_t   buttons;    /* button event to queue on hover-enter */
    int8_t   b_05;       /* button event to queue on hover-leave */
    DM2_TmouseClickRectData *node;
} DM2_TmouseClickRectNode;

struct DM2_TmouseClickRectData {
    DM2_TmouseClickRectNode *next;
    DM2_TmouseRect           r;
};

/* ========================================================================
 * Mouse cursor descriptor
 * ======================================================================== */

typedef struct {
    int8_t   hx;         /* hotspot x */
    int8_t   hy;         /* hotspot y */
    int8_t   w;          /* width */
    int8_t   h;          /* height */
    uint8_t  alphamask;  /* transparent color index */
    /* pixel data not modelled here — handled by callbacks */
} DM2_TmouseCursorDesc;

/* ========================================================================
 * Server command
 * ======================================================================== */

typedef struct {
    int16_t command;
} DM2_TmouseServerCommand;

/* ========================================================================
 * Command queue — circular FIFO for server commands
 * ======================================================================== */

typedef struct {
    int16_t idx_in;
    int16_t idx_out;
    DM2_TmouseServerCommand queue[DM2_TMOUSE_COMMAND_QUEUE_LENGTH];
} DM2_TmouseCommandQueue;

/* ========================================================================
 * Mouse queue — circular FIFO for raw mouse events
 * ======================================================================== */

typedef struct {
    int16_t             counter;
    int16_t             idx_in;
    int16_t             idx_out;
    DM2_TmouseEventEntry queue[DM2_TMOUSE_MOUSE_QUEUE_LENGTH];
} DM2_TmouseMouseQueue;

/* ========================================================================
 * Tmouse state — the main mouse subsystem state machine
 * ======================================================================== */

typedef struct {
    /* Current entry (last processed position + buttons) */
    DM2_TmouseEventEntry entry;

    /* Cursor */
    DM2_TmouseCursorIdx  cursor_idx;
    int16_t              last_x;         /* DRVW_show_mx */
    int16_t              last_y;         /* DRVW_show_my */

    /* Input blocking */
    int16_t              block_mouse_input_counter;

    /* Capture mode */
    int16_t              mouse_captured_counter;

    /* Queues */
    DM2_TmouseMouseQueue mouse_queue;

    /* Visibility */
    int16_t              mouse_invisible;

    /* Click-rect hit-test cache */
    bool                 mouse_setrect;
    int16_t              mouse_rx0;
    int16_t              mouse_rx1;
    int16_t              mouse_ry0;
    int16_t              mouse_ry1;

    /* Rect boundary tracking */
    DM2_TmouseRect       mouse_rect;     /* DRVR_mouserect */
    DM2_TmouseRect       dummy_rect;     /* mdummyrect */

    DM2_TmouseXMouseRect *xmouserect_ptr; /* DRV_xmouserectptr */
    DM2_TmouseXMouseRect  rect1;          /* DRVR_rect1 */
    DM2_TmouseXMouseRect  rect2;          /* DRVR_rect2 */
    bool                  use_rect2;      /* DRVb_use_rect2 */

    /* Click-rect lists */
    DM2_TmouseClickRectNode *rectlist1;
    DM2_TmouseClickRectNode *rectlist2;
} DM2_TmouseState;

/* ========================================================================
 * Callbacks — external dependencies injected for testability
 * ======================================================================== */

typedef struct {
    void *ctx;

    /* Event queue operations */
    void (*queue_event)(void *ctx, int16_t x, int16_t y, int16_t b);

    /* Event queue state queries */
    int16_t (*get_event_heroidx)(void *ctx);   /* E_NOHERO = -1 */
    bool    (*get_event_unk0f)(void *ctx);
    bool    (*get_fetch_busy)(void *ctx);

    /* Allegro/platform mouse warp */
    void (*set_mouse_pos)(void *ctx, int16_t x, int16_t y);

    /* Capture state (ddat.vcapture1/2/3) */
    bool (*get_vcapture1)(void *ctx);
    bool (*get_vcapture2)(void *ctx);
    bool (*get_vcapture3)(void *ctx);
    void (*clear_vcaptures)(void *ctx);

    /* Click-rect node operations */
    void (*set_clickrect_datas)(void *ctx, DM2_TmouseClickRectNode *node,
                                int16_t x0, int16_t y0, int16_t x1, int16_t y1);
} DM2_TmouseCallbacks;

/* ========================================================================
 * Receipt structs — function return values
 * ======================================================================== */

typedef struct {
    bool ok;
} DM2_TmouseCommandQueuePushReceipt;

typedef struct {
    bool    ok;
    DM2_TmouseServerCommand command;
} DM2_TmouseCommandQueuePopReceipt;

typedef struct {
    bool ok;
} DM2_TmouseMouseQueuePushReceipt;

typedef struct {
    bool                 ok;
    DM2_TmouseEventEntry entry;
} DM2_TmouseMouseQueuePopReceipt;

typedef struct {
    DM2_TmouseCursorIdx cursor_idx;
} DM2_TmouseQueueEventReceipt;

typedef struct {
    bool accepted;  /* false if blocked or fetch_busy */
} DM2_TmouseDriverMouseIntReceipt;

typedef struct {
    int16_t mx;
    int16_t my;
    int16_t mb;
} DM2_TmouseGetEntryDataReceipt;

/* ========================================================================
 * Command queue functions
 * ======================================================================== */

void dm2_v1_tmouse_command_queue_init(DM2_TmouseCommandQueue *q);

DM2_TmouseCommandQueuePushReceipt
dm2_v1_tmouse_command_queue_push(DM2_TmouseCommandQueue *q, int16_t command);

DM2_TmouseCommandQueuePopReceipt
dm2_v1_tmouse_command_queue_pop(DM2_TmouseCommandQueue *q);

/* ========================================================================
 * Mouse queue functions
 * ======================================================================== */

void dm2_v1_tmouse_mouse_queue_init(DM2_TmouseMouseQueue *q);

DM2_TmouseMouseQueuePushReceipt
dm2_v1_tmouse_mouse_queue_push(DM2_TmouseMouseQueue *q, DM2_TmouseEventEntry entry);

DM2_TmouseMouseQueuePopReceipt
dm2_v1_tmouse_mouse_queue_pop(DM2_TmouseMouseQueue *q);

/* ========================================================================
 * XMouseRect
 * ======================================================================== */

void dm2_v1_tmouse_xmouserect_init(DM2_TmouseXMouseRect *xr);

/* ========================================================================
 * Tmouse state machine
 * ======================================================================== */

void dm2_v1_tmouse_init(DM2_TmouseState *state);

bool dm2_v1_tmouse_is_visible(const DM2_TmouseState *state);

void dm2_v1_tmouse_hide(DM2_TmouseState *state);

void dm2_v1_tmouse_block_mouse_input(DM2_TmouseState *state);

void dm2_v1_tmouse_unblock_mouse_input(DM2_TmouseState *state);

/* ========================================================================
 * send_command — wraps block/push/unblock
 * ======================================================================== */

void dm2_v1_tmouse_send_command(DM2_TmouseState *state,
                                DM2_TmouseCommandQueue *cq,
                                int16_t command);

/* ========================================================================
 * Command interpreter — processes command queue + mouse queue
 * ======================================================================== */

void dm2_v1_tmouse_command_interpreter(DM2_TmouseState *state,
                                       DM2_TmouseCommandQueue *cq,
                                       const DM2_TmouseCallbacks *cb);

/* ========================================================================
 * T1_queue_0x20 — click-rect hit testing (cursor determination)
 * ======================================================================== */

DM2_TmouseCursorIdx
dm2_v1_tmouse_queue_0x20(DM2_TmouseState *state,
                          int16_t x, int16_t y,
                          const DM2_TmouseCallbacks *cb);

/* ========================================================================
 * T1_queue_event — event routing
 * ======================================================================== */

DM2_TmouseQueueEventReceipt
dm2_v1_tmouse_queue_event(DM2_TmouseState *state,
                           int16_t x, int16_t y, int16_t b,
                           const DM2_TmouseCallbacks *cb);

/* ========================================================================
 * T1_driver_mouseint — driver interrupt handler
 * ======================================================================== */

DM2_TmouseDriverMouseIntReceipt
dm2_v1_tmouse_driver_mouseint(DM2_TmouseState *state,
                               DM2_TmouseEventEntry entry,
                               const DM2_TmouseCallbacks *cb);

/* ========================================================================
 * Public API wrappers
 * ======================================================================== */

void dm2_v1_tmouse_hide_mouse(DM2_TmouseState *state);

void dm2_v1_tmouse_show_mouse(DM2_TmouseState *state,
                               DM2_TmouseCommandQueue *cq);

void dm2_v1_tmouse_set_capture(DM2_TmouseState *state,
                                DM2_TmouseCommandQueue *cq);

void dm2_v1_tmouse_release_capture(DM2_TmouseState *state,
                                    DM2_TmouseCommandQueue *cq);

void dm2_v1_tmouse_refresh_mouse(DM2_TmouseState *state,
                                  DM2_TmouseCommandQueue *cq);

void dm2_v1_tmouse_choose_cursor3(DM2_TmouseState *state,
                                   DM2_TmouseCommandQueue *cq);

void dm2_v1_tmouse_release_mouse_captures(DM2_TmouseState *state,
                                           DM2_TmouseCommandQueue *cq,
                                           const DM2_TmouseCallbacks *cb);

DM2_TmouseGetEntryDataReceipt
dm2_v1_tmouse_get_mouse_entry_data(DM2_TmouseState *state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_TMOUSE_PC34_COMPAT_H */
