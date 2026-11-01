#include "redmcsb_f0619_slot_box_border.h"

#include <limits.h>
#include <string.h>

static int redmcsb_f0619_fit_i16(int value)
{
    return value >= INT16_MIN && value <= INT16_MAX;
}

int redmcsb_f0619_get_slot_box_border(
    int16_t zone_index, int16_t object_icon_width,
    int16_t object_icon_height, int16_t text_margin,
    RedmcsbF0619MarginBranch margin_branch,
    RedmcsbF0619ResolveZoneFn resolve_zone, void *context,
    RedmcsbF0619Receipt *out_receipt)
{
    RedmcsbF0619Receipt receipt;
    int x, y, width, height;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.zone_index = zone_index;
    receipt.initial_width = object_icon_width;
    receipt.initial_height = object_icon_height;
    receipt.text_margin = text_margin;
    receipt.margin_branch = margin_branch;

    if (!resolve_zone || object_icon_width <= 0 || object_icon_height <= 0 ||
        text_margin < 0 || margin_branch < REDMCSB_F0619_NO_MARGIN ||
        margin_branch > REDMCSB_F0619_MEDIA720_MARGIN ||
        (margin_branch == REDMCSB_F0619_NO_MARGIN && text_margin != 0)) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!resolve_zone(context, zone_index, object_icon_width,
                      object_icon_height, &receipt.rect) ||
        receipt.rect.width <= 0 || receipt.rect.height <= 0) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    x = receipt.rect.x;
    y = receipt.rect.y;
    width = receipt.rect.width;
    height = receipt.rect.height;
    if (margin_branch != REDMCSB_F0619_NO_MARGIN) {
        /* OBJECT.C:384-387 expands inclusive left/right/top/bottom bounds
         * under MEDIA508. F0628 at COORD.C:1903-1915 subtracts the same
         * margins from origin and adds twice each margin to width/height
         * under MEDIA720. Both yield this canonical rectangle. */
        x -= text_margin;
        y -= text_margin;
        width += 2 * text_margin;
        height += 2 * text_margin;
    }
    if (!redmcsb_f0619_fit_i16(x) || !redmcsb_f0619_fit_i16(y) ||
        !redmcsb_f0619_fit_i16(width) ||
        !redmcsb_f0619_fit_i16(height)) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.rect.x = (int16_t)x;
    receipt.rect.y = (int16_t)y;
    receipt.rect.width = (int16_t)width;
    receipt.rect.height = (int16_t)height;
    receipt.accepted = 1u;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}
