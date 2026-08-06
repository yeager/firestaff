#include "dm1_v1_fmtowns_menu_render.h"
#include "dm1_v1_fmtowns_menu_regions.h"
#include "dm1_v1_fmtowns_dynamenu.h"
#include "dm1_v1_fmtowns_dyna_buttons.h"
#include "dm1_v1_fmtowns_dyna_buttons_ja.h"
#include "dm1_v1_fmtowns_text_geometry.h"
#include "dm1_v1_fmtowns_egb_shim.h"

#include <string.h>

int dm1_v1_fmtowns_menu_render_pc34(
    uint8_t                                    *fb,
    int                                         fb_width,
    int                                         fb_height,
    int                                         fb_stride,
    const dm1_v1_fmtowns_menu_render_config_t  *config,
    dm1_v1_fmtowns_menu_render_result_t        *result) {
    DM1_V1_FmtownsRegionRecord panel_size, panel_anchor;
    uint8_t colour;
    int x1, y1, x2, y2;
    size_t painted;
    unsigned int slot;

    if (result) memset(result, 0, sizeof(*result));
    if (!fb || !config || !config->dynamenu_record) return 0;
    if (fb_width <= 0 || fb_height <= 0 || fb_stride < fb_width) return 0;

    /* Region 10 supplies the panel size (byte-verified 87x45).
     * Region 11 supplies the anchor (319, 77) that DRAW_DMENU uses
     * as the panel's bottom-right corner; the top-left is offset by
     * (width, 0) from the anchor. */
    if (!dm1_v1_fmtowns_region_menu_panel_pc34(&panel_size)) return 0;
    if (!dm1_v1_fmtowns_region_menu_clear_area_pc34(&panel_anchor)) return 0;

    x1 = panel_anchor.a - panel_size.a;
    y1 = panel_anchor.b;
    x2 = panel_anchor.a - 1;
    y2 = panel_anchor.b + panel_size.b - 1;

    colour = dm1_v1_fmtowns_dynamenu_panel_colour_pc34(config->dynamenu_record);
    painted = dm1_v1_fmtowns_egb_fill_rect_pc34(fb, fb_width, fb_height,
                                                fb_stride, x1, y1, x2, y2,
                                                colour);

    if (result) {
        result->panel_filled          = painted > 0 ? 1 : 0;
        result->panel_pixels_written  = painted;
        result->panel_colour          = colour;
        result->panel_x1 = x1; result->panel_y1 = y1;
        result->panel_x2 = x2; result->panel_y2 = y2;
    }

    if (painted == 0) return 0;

    /* Three-slot label loop. Slots use DYNAMENU byte offsets +1..+3
     * (byte-verified in DRAW_DMENU 0x46c4..0x46fd). Each slot's label
     * index goes through the language-appropriate GET_LABEL lookup,
     * which converts 0xFF into the empty string. */
    for (slot = 0; slot < DM1_V1_FMTOWNS_DYNAMENU_BUTTON_COUNT; ++slot) {
        uint8_t label_index;
        const char *label_bytes;
        int label_y;
        int rc = 0;
        label_index = dm1_v1_fmtowns_dynamenu_slot_label_pc34(
            config->dynamenu_record, slot);
        if (config->language == DM1_V1_FMTOWNS_MENU_LANG_JA) {
            label_bytes = dm1_v1_fmtowns_dyna_button_label_ja_pc34(label_index);
        } else {
            label_bytes = dm1_v1_fmtowns_dyna_button_label_pc34(label_index);
        }
        if (result) ++result->label_slots_visited;
        if (!label_bytes || label_bytes[0] == '\0') {
            if (result) result->label_slot_returns[slot] = 0;
            continue;
        }
        label_y = y1 + (int)(slot * DM1_V1_FMTOWNS_CHAR_Y_HYT);
        if (config->glyph_draw && config->menu_font_raster) {
            rc = config->glyph_draw(
                config->glyph_draw_user, fb, fb_width, fb_height, fb_stride,
                x1, label_y, config->label_fg, config->label_bg,
                config->language, slot, label_bytes);
        }
        if (result) {
            result->label_slot_returns[slot] = rc;
            if (rc > 0) ++result->label_slots_drawn;
        }
    }

    return 1;
}
