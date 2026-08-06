/*
 * dm2_v1_0aaf_pc34_compat.c -- DM2 segment 0AAF dialogue/menu logic.
 *
 * Ports the dialogue construction and menu selection system from
 * skproject c_0aaf.cpp. Three main functions: menu selection with
 * mouse/keyboard input, dialogue part drawing, and recursive dialogue
 * construction with text layout.
 *
 * Source: skproject/SKULLWIN/c_0aaf.cpp
 */

#include "dm2_v1_0aaf_pc34_compat.h"

#include <string.h>

/* ── DM2_0aaf_0067 — menu selection ────────────────────────────────── */

/* Source: c_0aaf.cpp DM2_0aaf_0067
 * Scans GDAT text entries for menu items, displays them, and waits for
 * mouse/keyboard selection. Returns the selected item code. */
DM2_V1_0aafMenuReceipt dm2_v1_0aaf_menu_select(
    const DM2_V1_0aafCallbacks *cb,
    int32_t menu_param)
{
    DM2_V1_0aafMenuReceipt receipt;
    receipt.selection = -1;

    if (cb == NULL) return receipt;

    uint8_t type_byte = (uint8_t)(menu_param & 0xFF);
    uint8_t sub_start = (uint8_t)((menu_param >> 8) & 0xFF);
    int16_t item_count = 0;
    int16_t last_valid = -1;
    int32_t last_type = 0; (void)last_type;
    char text_buf[DM2_V1_0AAF_TEXT_BUF_SIZE];
    /* c_0aaf.cpp keeps the selection bytes in tarr_00 at +0x28.  The
     * source event ordinal is one-based, so EVENT 1 reads tarr_00[0x28]
     * through the 0x26 + 2 * event expression below. */
    uint8_t entry_data[DM2_V1_0AAF_TEXT_BUF_SIZE];
    int show_count = 0;

    memset(entry_data, 0, sizeof(entry_data));

    /* Scan GDAT text entries — source: m_A191..m_A204 */
    for (uint8_t sub = sub_start; sub < 0x14; sub++) {
        const char *txt = cb->query_gdat_text(cb->ctx, 0x1a,
            (uint8_t)(menu_param & 0xFF), sub, text_buf);
        if (txt != NULL && txt[0] != '\0') {
            int16_t data_idx = cb->query_gdat_entry_data_index(
                cb->ctx, 0x1a, type_byte, 0x0b, sub);
            uint8_t lo = (uint8_t)(data_idx & 0xFF);
            uint8_t hi = (uint8_t)((data_idx >> 8) & 0xFF);

            entry_data[0x28 + item_count * 2]     = (lo == 0) ? sub : lo;
            entry_data[0x28 + item_count * 2 + 1] = hi;

            if (hi != 0) {
                last_valid = (int16_t)item_count;
                last_type  = (int32_t)entry_data[0x28 + item_count * 2];
            }
            item_count++;
        }
    }

    /* Set v1e0204 */
    if (type_byte != (uint8_t)0x87) {
        cb->set_v1e0204(cb->ctx, item_count);
    } else {
        cb->set_v1e0204(cb->ctx, 7);
    }

    /* Handle single-item case */
    if (last_valid == -1 && item_count == 1)
        last_valid = 1;

    /* Enter UI mode 4 for menu */
    cb->mode_1031_0675(cb->ctx, 4);

    /* Show mouse */
    while (!cb->is_mouse_visible(cb->ctx)) {
        cb->show_mouse(cb->ctx);
        show_count++;
    }
    cb->set_event_unk06(cb->ctx, 0xFF);

    /* Event loop — source: m_A26F */
    int16_t result = 0;
    for (;;) {
        cb->event_loop(cb->ctx);
        cb->wait_screen_refresh(cb->ctx);

        /* Check for auto-selection */
        if (last_valid != -1) {
            /* SKProject SKWINSPX/src/v4/skgame.cpp:2575-2579 names
             * _476d_04ed_DOES_NOTHING: it unconditionally returns zero.
             * Keep the source branch explicit, but do not fabricate an
             * auto-selection or a callback for a routine that is a no-op. */
        }

        /* Check for keyboard input */
        int16_t evt = cb->get_event_unk06(cb->ctx);
        if (evt == 0xFF) {
            if (cb->has_key(cb->ctx)) {
                int16_t key = cb->getkey_translated(cb->ctx);
                if (key == 0x1C) {
                    cb->mode_1031_0781(cb->ctx, 0xDB);
                }
            }
        }

        /* Check for selection */
        evt = cb->get_event_unk06(cb->ctx);
        if (evt != 0xFF) {
            if (type_byte != (uint8_t)0x87) {
                /* c_0aaf.cpp m_A2F4: event_unk06 is a one-based item
                 * ordinal. Reject a malformed source event instead of
                 * indexing past the 80-byte source stack buffer. */
                if (evt < 1 || evt > item_count) {
                    result = -1;
                } else {
                    result = entry_data[0x26 + 2 * evt];
                }
            } else {
                result = cb->get_event_unk0a(cb->ctx);
            }
            break;
        }
    }

    /* Hide mouse */
    while (show_count > 0) {
        cb->hide_mouse(cb->ctx);
        show_count--;
    }

    cb->set_backbuff2(cb->ctx, 1);
    cb->mode_1031_06a5(cb->ctx);

    receipt.selection = result;
    return receipt;
}

/* ── DM2_0aaf_01db — draw dialogue part ────────────────────────────── */

/* Source: c_0aaf.cpp DM2_0aaf_01db
 * Draws a single dialogue element — either a palette-filled rectangle
 * or a GDAT image centred in the target rect. */
void dm2_v1_0aaf_draw_part(
    const DM2_V1_0aafCallbacks *cb,
    int16_t rect_id, int32_t draw_mode)
{
    if (cb == NULL) return;

    if (!cb->get_v1e0a88(cb->ctx)) {
        /* Palette-based drawing — source: m_A3F0..m_A41D */
        uint8_t pixel;
        if (draw_mode == 0)
            pixel = cb->palette_to_ui8(cb->ctx, 1); /* E_COL01 */
        else
            pixel = cb->palette_to_ui8(cb->ctx, 0); /* E_COL00 */

        DM2_V1_0aafRect rc;
        DM2_V1_0aafRect *r = cb->query_expanded_rect(cb->ctx, rect_id, &rc);
        if (r != NULL)
            cb->fill_backbuff_rect(cb->ctx, r, pixel);
    } else {
        /* Image-based drawing */
        if (draw_mode == 0) return;

        void *bmp = cb->query_gdat_image_entry_buff(cb->ctx,
            cb->get_v1e0206(cb->ctx),
            cb->get_v1e0207(cb->ctx),
            cb->get_v1e0208(cb->ctx));
        if (bmp == NULL) return;

        int16_t bw = cb->get_image_width(cb->ctx, bmp);
        int16_t bh = cb->get_image_height(cb->ctx, bmp);

        /* Centre image in dm2rect4 */
        DM2_V1_0aafRect rc;
        rc.x = 0; rc.y = 0; rc.w = bw; rc.h = bh;
        /* (centering calculation simplified) */

        /* Get palette */
        void *pal;
        if (!cb->get_gfxalloc_done(cb->ctx)) {
            int32_t img_len = cb->calc_image_byte_length(cb->ctx, bmp);
            pal = (void *)((uint8_t *)bmp + img_len);
        } else {
            pal = cb->query_gdat_image_localpal(cb->ctx,
                cb->get_v1e0206(cb->ctx),
                cb->get_v1e0207(cb->ctx),
                cb->get_v1e0208(cb->ctx));
        }

        cb->draw_dialogue_parts_pict(cb->ctx, bmp, &rc, 7, pal);
        cb->free_pict_entry(cb->ctx, bmp);
    }

    cb->sleep_several_time(cb->ctx, 20);
}

/* ── DM2_0aaf_02f8 — construct dialogue ────────────────────────────── */

/* Source: c_0aaf.cpp DM2_0aaf_02f8 (recursive)
 * Builds and displays a full dialogue with text entries, optional images,
 * and multi-line text layout. */
DM2_V1_0aafDialogueReceipt dm2_v1_0aaf_construct_dialogue(
    const DM2_V1_0aafCallbacks *cb,
    int32_t type_param, int32_t sub_param)
{
    DM2_V1_0aafDialogueReceipt receipt;
    receipt.result = 0;
    receipt.dialogue_type = (uint8_t)(type_param & 0xFF);

    if (cb == NULL) return receipt;

    uint8_t type_byte = (uint8_t)(type_param & 0xFF);
    uint8_t sub_byte  = (uint8_t)(sub_param & 0xFF);
    bool needs_fade = true;

    /* Handle type aliasing — source: m_A459..m_A462 */
    if (type_byte == 0x0E || type_byte == 0x87) {
        if (sub_byte != 0)
            needs_fade = false;
    }

    /* Check for loadable alternate type */
    if (type_byte == 0x07 || type_byte == 0x13) {
        if (cb->query_gdat_entry_if_loadable(cb->ctx, 0x1a, 0x59, 0x01, 0))
            type_byte = 0x59;
    }

    /* Recursive sub-dialogue for non-zero sub — source: m_A4A1..m_A50F */
    if (sub_byte != 0 && type_byte != 0 && type_byte != 0x0E) {
        if (cb->query_gdat_entry_if_loadable(cb->ctx, 0x1a, 0, 0x01, 0)) {
            DM2_V1_0aafDialogueReceipt sub_r =
                dm2_v1_0aaf_construct_dialogue(cb, 0, (int32_t)sub_byte);
            dm2_v1_0aaf_menu_select(cb, (int32_t)(sub_r.dialogue_type & 0xFF));
            sub_byte = 0;
        }
    }

    /* Fade screen if needed */
    if (needs_fade && cb->get_dialog2(cb->ctx) != 0)
        cb->fade_screen(cb->ctx, 1);

    /* Scan text entries — source: m_A510..m_A555 */
    int16_t text_count = 0;
    char text_bufs[DM2_V1_0AAF_MAX_TEXT_ENTRIES][40];
    const char *text_ptrs[DM2_V1_0AAF_MAX_TEXT_ENTRIES];

    for (uint8_t sub = (uint8_t)((sub_param >> 8) & 0xFF); sub < 0x14; sub++) {
        const char *txt = cb->query_gdat_text(cb->ctx, 0x1a, type_byte,
            sub, text_bufs[text_count]);
        text_ptrs[text_count] = txt;
        if (txt != NULL && txt[0] != '\0')
            text_count++;
    }

    /* Draw background — image or palette fill */
    bool use_image = cb->get_v1e0a88(cb->ctx);
    if (use_image) {
        bool can_draw = cb->get_gfxalloc_done(cb->ctx);
        if (!can_draw) {
            int16_t bw = cb->get_backbuffer_w(cb->ctx);
            int32_t needed = (int32_t)((((bw + 1) & ~1) >> 1) *
                             cb->get_backbuffer_h(cb->ctx)) + 0x1E;
            can_draw = (needed <= cb->bigpool_available(cb->ctx));
        }
        if (can_draw) {
            /* Draw background image */
            void *bmp = cb->query_gdat_image_entry_buff(cb->ctx,
                26, type_byte, 0);
            if (bmp != NULL) {
                void *pal;
                if (!cb->get_gfxalloc_done(cb->ctx)) {
                    int32_t len = cb->calc_image_byte_length(cb->ctx, bmp);
                    pal = (void *)((uint8_t *)bmp + len);
                } else {
                    pal = cb->query_gdat_image_localpal(cb->ctx,
                        26, type_byte, 0);
                }
                DM2_V1_0aafRect rc;
                cb->query_expanded_rect(cb->ctx, 4, &rc);
                cb->draw_dialogue_parts_pict(cb->ctx, bmp, &rc, -1, pal);
                cb->free_pict_entry(cb->ctx, bmp);
            }
        } else {
            /* Palette fill fallback */
            uint8_t bg = cb->palette_to_ui8(cb->ctx, 1);
            DM2_V1_0aafRect rc;
            rc.x = 0; rc.y = 0;
            rc.w = cb->get_backbuffer_w(cb->ctx);
            rc.h = cb->get_backbuffer_h(cb->ctx);
            cb->fill_backbuff_rect(cb->ctx, &rc, bg);

            uint8_t fg = cb->palette_to_ui8(cb->ctx, 5);
            rc.x += 10; rc.y += 10;
            rc.w -= 20; rc.h -= 20;
            cb->fill_backbuff_rect(cb->ctx, &rc, fg);

            /* Draw individual dialogue parts */
            int16_t start_idx = 0;
            if (text_count >= 3 && text_count <= 3) start_idx = 1;
            else if (text_count == 4) start_idx = (int16_t)text_count;

            for (int16_t i = 0; i < text_count; i++) {
                int16_t tbl_idx = i + start_idx;
                dm2_v1_0aaf_draw_part(cb,
                    cb->get_table1d27c4(cb->ctx, tbl_idx), 0);
            }
        }
    }

    /* Draw title text — source: m_A6A6 */
    uint8_t title_col = cb->palette_to_ui8(cb->ctx, 12); /* E_COL12 */
    cb->draw_vp_rc_str(cb->ctx, 0x1C2, (int16_t)title_col,
                       cb->get_v1d1044(cb->ctx));

    /* Draw text entries — source: m_A711..m_A740 */
    int16_t text_y_offset = 0;
    if (type_byte != (uint8_t)0x87) {
        /* Draw text entries with metrics */
        for (int16_t i = 0; i < text_count; i++) {
            uint8_t col = cb->palette_to_ui8(cb->ctx, 11); /* E_COL11 */
            int16_t rect_idx = cb->get_table1d27d4(cb->ctx, i + text_y_offset);
            cb->draw_vp_rc_str(cb->ctx, rect_idx, (int16_t)col,
                               text_ptrs[i]);
        }
    }

    /* Finalize */
    if (needs_fade) {
        cb->draw_gameload_dialogue_to_screen(cb->ctx);
        if (cb->get_dialog2(cb->ctx) != 0)
            cb->fade_screen(cb->ctx, 0);
    }

    cb->set_backbuff2(cb->ctx, 1);
    receipt.result = (int32_t)type_byte;
    receipt.dialogue_type = type_byte;
    return receipt;
}
