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

#define DM1_V1_DIALOG_MAX_MSG_LEN_PC34     128
#define DM1_V1_DIALOG_MSG_QUEUE_SIZE_PC34   16
#define DM1_V1_DIALOG_DISPLAY_TICKS_PC34    60   /* Frames to show message */

/* Dialog set indices — DIALOG.C G2062 pattern */
typedef enum {
    DM1_V1_DIALOG_SET_VIEWPORT_PC34 = 0,    /* C0_DIALOG_SET_VIEWPORT */
    DM1_V1_DIALOG_SET_INVENTORY_PC34,
    DM1_V1_DIALOG_SET_MAP_PC34,
    DM1_V1_DIALOG_SET_ENTRANCE_PC34,
    DM1_V1_DIALOG_SET_RESURRECT_PC34,
    DM1_V1_DIALOG_SET_COUNT_PC34
} DM1_V1_DialogSetPc34;

typedef struct {
    char     text[DM1_V1_DIALOG_MAX_MSG_LEN_PC34];
    uint8_t  color;
    int16_t  display_ticks;      /* Remaining display time */
    bool     active;
} DM1_V1_DialogMessagePc34;

typedef struct {
    DM1_V1_DialogMessagePc34 queue[DM1_V1_DIALOG_MSG_QUEUE_SIZE_PC34];
    uint8_t head, tail;
    uint8_t count;
    DM1_V1_DialogMessagePc34 current;      /* Currently displayed message */
    DM1_V1_DialogSetPc34 active_set;
    int16_t bar_x, bar_y;       /* Message bar screen position */
    int16_t bar_w, bar_h;
} DM1_V1_DialogStatePc34;

void DM1_V1_Dialog_InitPc34Compat(DM1_V1_DialogStatePc34* state);
void DM1_V1_Dialog_SetBarPositionPc34Compat(DM1_V1_DialogStatePc34* state, int16_t x, int16_t y,
                              int16_t w, int16_t h);
void DM1_V1_Dialog_SetActivePc34Compat(DM1_V1_DialogStatePc34* state, DM1_V1_DialogSetPc34 set);
bool DM1_V1_Dialog_PushMessagePc34Compat(DM1_V1_DialogStatePc34* state, const char* text, uint8_t color);
void DM1_V1_Dialog_TickPc34Compat(DM1_V1_DialogStatePc34* state);
bool DM1_V1_Dialog_HasMessagePc34Compat(const DM1_V1_DialogStatePc34* state);
const char* DM1_V1_Dialog_GetCurrentTextPc34Compat(const DM1_V1_DialogStatePc34* state);
uint8_t DM1_V1_Dialog_GetCurrentColorPc34Compat(const DM1_V1_DialogStatePc34* state);

typedef DM1_V1_DialogSetPc34 M11_DG_DialogSet;
typedef DM1_V1_DialogMessagePc34 M11_DG_Message;
typedef DM1_V1_DialogStatePc34 M11_DG_State;

#define DM1_DG_MAX_MSG_LEN DM1_V1_DIALOG_MAX_MSG_LEN_PC34
#define DM1_DG_MSG_QUEUE_SIZE DM1_V1_DIALOG_MSG_QUEUE_SIZE_PC34
#define DM1_DG_DISPLAY_TICKS DM1_V1_DIALOG_DISPLAY_TICKS_PC34

#define M11_DG_SET_VIEWPORT DM1_V1_DIALOG_SET_VIEWPORT_PC34
#define M11_DG_SET_INVENTORY DM1_V1_DIALOG_SET_INVENTORY_PC34
#define M11_DG_SET_MAP DM1_V1_DIALOG_SET_MAP_PC34
#define M11_DG_SET_ENTRANCE DM1_V1_DIALOG_SET_ENTRANCE_PC34
#define M11_DG_SET_RESURRECT DM1_V1_DIALOG_SET_RESURRECT_PC34
#define M11_DG_SET_COUNT DM1_V1_DIALOG_SET_COUNT_PC34

#define m11_dg_init DM1_V1_Dialog_InitPc34Compat
#define m11_dg_set_bar_position DM1_V1_Dialog_SetBarPositionPc34Compat
#define m11_dg_set_active DM1_V1_Dialog_SetActivePc34Compat
#define m11_dg_push_message DM1_V1_Dialog_PushMessagePc34Compat
#define m11_dg_tick DM1_V1_Dialog_TickPc34Compat
#define m11_dg_has_message DM1_V1_Dialog_HasMessagePc34Compat
#define m11_dg_get_current_text DM1_V1_Dialog_GetCurrentTextPc34Compat
#define m11_dg_get_current_color DM1_V1_Dialog_GetCurrentColorPc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DIALOG_SCROLL_PC34_COMPAT_H */
