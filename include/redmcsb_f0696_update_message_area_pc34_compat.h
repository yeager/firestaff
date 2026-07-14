/*
 * ReDMCSB DRAWMSGA.C F0696_UpdateMessageArea, PC 3.4 (F20E) route.
 *
 * The original does not manipulate a generic text buffer.  It locks screen
 * updates, scrolls C015_ZONE_MESSAGE_AREA by exactly G2088_C7_TextLineHeight,
 * then blits G0356_puc_Bitmap_MessageAreaNewRow into the newly exposed bottom
 * line.  This adapter leaves the actual EGA/VGA presentation to the caller.
 */
#ifndef FIRESTAFF_REDMCSB_F0696_UPDATE_MESSAGE_AREA_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0696_UPDATE_MESSAGE_AREA_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
} RedmcsbF0696BoxPc34Compat;

typedef struct {
    void (*enable_screen_update)(void *context);
    void (*disable_screen_update)(void *context);
    void (*get_zone)(void *context, uint16_t zone,
                     RedmcsbF0696BoxPc34Compat *out_box);
    void (*write_page)(void *context, int16_t work_page, int16_t write_page);
    void (*set_viewport)(void *context, int16_t work_page,
                         const RedmcsbF0696BoxPc34Compat *box);
    void (*part_scroll)(void *context, int16_t work_page, int16_t x_offset,
                        int16_t y_offset, int16_t line_count,
                        const RedmcsbF0696BoxPc34Compat *box);
    void (*blit_new_row)(void *context, const uint8_t *new_row_bitmap,
                         const RedmcsbF0696BoxPc34Compat *destination_box,
                         int16_t source_x, int16_t source_y,
                         int16_t source_width, int16_t destination_width,
                         int16_t transparency_color);
    void *context;
} RedmcsbF0696VideoOpsPc34Compat;

typedef struct {
    int16_t work_page;
    int16_t write_page;
    int16_t text_line_height;
    int16_t message_area_width;
    int16_t screen_pixel_width;
    int16_t color_no_transparency;
} RedmcsbF0696StatePc34Compat;

/* Returns 1 only after the complete F20E route has been issued. */
int redmcsb_f0696_update_message_area_pc34_compat(
    const RedmcsbF0696VideoOpsPc34Compat *ops,
    const RedmcsbF0696StatePc34Compat *state,
    const uint8_t *message_area_new_row_bitmap);

const char *redmcsb_f0696_update_message_area_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0696_UPDATE_MESSAGE_AREA_PC34_COMPAT_H */
