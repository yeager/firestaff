#include "redmcsb_f0696_update_message_area_pc34_compat.h"

int redmcsb_f0696_update_message_area_pc34_compat(
    const RedmcsbF0696VideoOpsPc34Compat *ops,
    const RedmcsbF0696StatePc34Compat *state,
    const uint8_t *message_area_new_row_bitmap)
{
    RedmcsbF0696BoxPc34Compat message_area;
    RedmcsbF0696BoxPc34Compat new_row;

    if (ops == 0 || state == 0 || message_area_new_row_bitmap == 0 ||
        ops->enable_screen_update == 0 || ops->disable_screen_update == 0 ||
        ops->get_zone == 0 || ops->write_page == 0 || ops->set_viewport == 0 ||
        ops->part_scroll == 0 || ops->blit_new_row == 0 ||
        state->text_line_height <= 0) {
        return 0;
    }

    /* ReDMCSB DRAWMSGA.C:50-65, F20E EGB setup and upward scroll. */
    ops->enable_screen_update(ops->context);
    ops->write_page(ops->context, state->work_page, state->write_page);
    ops->get_zone(ops->context, 15U, &message_area); /* C015_ZONE_MESSAGE_AREA */
    ops->set_viewport(ops->context, state->work_page, &message_area);
    ops->part_scroll(ops->context, state->work_page, 0, 0,
                     (int16_t)-state->text_line_height, &message_area);

    /* DRAWMSGA.C:80-82,107: only the final text-line-high strip is blitted. */
    new_row = message_area;
    new_row.top = (int16_t)(new_row.bottom - state->text_line_height + 1);
    ops->blit_new_row(ops->context, message_area_new_row_bitmap, &new_row,
                      0, 0, state->message_area_width,
                      state->screen_pixel_width, state->color_no_transparency);
    ops->disable_screen_update(ops->context);
    return 1;
}

const char *redmcsb_f0696_update_message_area_source_evidence_pc34(void)
{
    return "ReDMCSB DRAWMSGA.C:50-65,80-82,107,115: F20E enables screen "
           "updates, selects G4103_WRITE_PAGE, scrolls C015_ZONE_MESSAGE_AREA "
           "up by G2088_C7_TextLineHeight, and blits the new row at its bottom.";
}
