/* DM1 V1 Dialog/Scroll Message System — source-locked from ReDMCSB
 * DIALOG.C: dialog set management, message bar rendering
 * SCRLMGMT.C: scroll text display, message queue
 * TEXT.C: text rendering for dialog messages */
#ifndef FIRESTAFF_DM1_V1_DIALOG_SCROLL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DIALOG_SCROLL_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_DG_MAX_MSG_LEN     128
#define DM1_DG_MSG_QUEUE_SIZE   16
#define DM1_DG_DISPLAY_TICKS    60   /* Frames to show message */

/* Dialog set indices — DIALOG.C G2062 pattern */
typedef enum {
    M11_DG_SET_VIEWPORT = 0,    /* C0_DIALOG_SET_VIEWPORT */
    M11_DG_SET_INVENTORY,
    M11_DG_SET_MAP,
    M11_DG_SET_ENTRANCE,
    M11_DG_SET_RESURRECT,
    M11_DG_SET_COUNT
} M11_DG_DialogSet;

typedef struct {
    char     text[DM1_DG_MAX_MSG_LEN];
    uint8_t  color;
    int16_t  display_ticks;      /* Remaining display time */
    bool     active;
} M11_DG_Message;

typedef struct {
    M11_DG_Message queue[DM1_DG_MSG_QUEUE_SIZE];
    uint8_t head, tail;
    uint8_t count;
    M11_DG_Message current;      /* Currently displayed message */
    M11_DG_DialogSet active_set;
    int16_t bar_x, bar_y;       /* Message bar screen position */
    int16_t bar_w, bar_h;
} M11_DG_State;

void m11_dg_init(M11_DG_State* state);
void m11_dg_set_bar_position(M11_DG_State* state, int16_t x, int16_t y,
                              int16_t w, int16_t h);
void m11_dg_set_active(M11_DG_State* state, M11_DG_DialogSet set);
bool m11_dg_push_message(M11_DG_State* state, const char* text, uint8_t color);
void m11_dg_tick(M11_DG_State* state);
bool m11_dg_has_message(const M11_DG_State* state);
const char* m11_dg_get_current_text(const M11_DG_State* state);
uint8_t m11_dg_get_current_color(const M11_DG_State* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DIALOG_SCROLL_PC34_COMPAT_H */
