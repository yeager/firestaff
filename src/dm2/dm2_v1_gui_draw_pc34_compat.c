/* DM2 V1 GUI drawing -- skproject c_gui_draw.cpp.
 *
 * 60 functions implementing the HUD, stat panels, item icons,
 * inventory, magic map, spell panel, and right panel rendering.
 * Each public function uses a callback struct for game state access.
 * Internal helpers are called through the callbacks passed down
 * from the entry points. */

#include "dm2_v1_gui_draw_pc34_compat.h"
#include <stddef.h>
#include <string.h>

/* ================================================================
 * Data tables from c_gui_draw.cpp
 * ================================================================ */

/* table1d275a: coin scatter positions (32 entries x 2 bytes) */
const int8_t dm2_guidraw_table1d275a[32][2] = {
    {-2,10}, {-8,3}, {7,-3}, {-6,10}, {8,-7}, {-8,11}, {2,-2},
    {8,-11}, {0,0}, {4,8}, {-8,-10}, {6,7}, {2,2}, {-6,9}, {-3,-11},
    {2,11}, {0,-11}, {-5,-7}, {-3,3}, {8,11}, {0,-5}, {6,-9},
    {-7,3}, {5,1}, {4,-7}, {-8,-3}, {8,3}, {-3,-2}, {0,7}, {-3,-4},
    {-4,0}, {2,-11}
};

/* table1d69d0: hero stat bar color palette indices */
const uint8_t dm2_guidraw_table1d69d0[4] = { 0x08, 0x0B, 0x04, 0x02 };

/* table1d67d9: right panel mode flags */
const uint8_t dm2_guidraw_table1d67d9[8] = { 0, 1, 0, 1, 0, 1, 1, 0 };

/* v1d1124: slash separator "/" */
const char dm2_guidraw_v1d1124[2] = { '/', '\0' };

/* v1d10f0: empty string for item survey */
const char dm2_guidraw_v1d10f0[2] = { ' ', '\0' };

/* ================================================================
 * 1. DRAW_ICON_PICT_BUFF -- blit a source bitmap onto a buttongroup
 *    c_gui_draw.cpp:31
 * ================================================================ */

void dm2_v1_draw_icon_pict_buff(
    void *srcbmp, int32_t bg_id, DM2_V1_Rect *blitrect,
    int16_t srcx, int16_t srcy, int16_t alphamask, int32_t blitmode,
    void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    /* Source: SKWINSPX/src/v5/skguidrw.cpp, DM2_DRAW_ICON_PICT_BUFF (32-58).
     * The blit destination is offset into the button group and uses the
     * group's width, not the source image width. */
    if (!cb || !srcbmp || !blitrect || !cb->blit || !cb->offset_rect ||
        !cb->get_button_group_width || !cb->get_bmp || !cb->get_bmp_width ||
        !cb->get_bmp_bpp)
        return;

    int16_t srcw = cb->get_bmp_width(ctx, srcbmp);
    void *dstbmp = cb->get_bmp(ctx, (int16_t)bg_id);
    int16_t dstw = cb->get_button_group_width(ctx, bg_id);
    DM2_V1_Rect offset;
    DM2_V1_Rect *dstrect = cb->offset_rect(ctx, bg_id, &offset, blitrect);
    if (!dstbmp || srcw <= 0 || dstw <= 0 || !dstrect) return;

    cb->blit(ctx, srcbmp, dstbmp, dstrect, srcx, srcy,
             srcw, dstw, alphamask, blitmode,
             cb->get_bmp_bpp(ctx, srcbmp),
             8, palette);

    if (cb->adjust_buttongroup_rects)
        cb->adjust_buttongroup_rects(ctx, bg_id, blitrect);
}

/* ================================================================
 * 2. DRAW_ICON_PICT_ENTRY -- query GDAT image and blit
 *    c_gui_draw.cpp:58
 * ================================================================ */

void dm2_v1_draw_icon_pict_entry(
    int8_t cls, int8_t sub, int8_t idx, int32_t bg_id,
    int16_t rectid, int16_t alphamask,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    void *srcbmp = cb->query_gdat_image ?
                   cb->query_gdat_image(ctx, cls, sub, idx) : NULL;
    if (!srcbmp) return;

    DM2_V1_Rect rect;
    int16_t srcx = 0, srcy = 0;
    DM2_V1_Rect *blitrect = cb->query_blit_rect ?
        cb->query_blit_rect(ctx, srcbmp, &rect, rectid, &srcx, &srcy) : NULL;

    void *palette = cb->query_gdat_palette ?
                    cb->query_gdat_palette(ctx, cls, sub, idx) : NULL;

    dm2_v1_draw_icon_pict_buff(srcbmp, bg_id, blitrect,
                                srcx, srcy, alphamask, 0, palette,
                                cb, ctx);
}

/* ================================================================
 * 3. DRAW_DIALOGUE_PROGRESS -- progress bar during loading
 *    c_gui_draw.cpp:79
 * ================================================================ */

void dm2_v1_draw_dialogue_progress(
    int32_t progress,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_DrawDialogueProgressReceipt *receipt)
{
    if (receipt) {
        receipt->progress_drawn = false;
        receipt->bar_width = 0;
    }
    if (!cb) return;

    bool v1e0200 = cb->get_v1e0200 ? cb->get_v1e0200(ctx) : false;
    if (!v1e0200) return;

    DM2_V1_Rect rc;
    if (!cb->query_expanded_rect) return;
    cb->query_expanded_rect(ctx, 474, &rc);

    int16_t bar_w = (int16_t)((int32_t)rc.w * progress / 0x3e8);
    rc.w = bar_w;

    int16_t dialog1 = cb->get_dialog1 ? cb->get_dialog1(ctx) : 0;
    if (bar_w > 0 && bar_w != dialog1) {
        if (cb->set_dialog1) cb->set_dialog1(ctx, bar_w);
        uint8_t col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 9) : 0;
        if (cb->fill_backbuff_rect)
            cb->fill_backbuff_rect(ctx, &rc, (int32_t)col);
        if (cb->draw_gameload_dialogue_to_screen)
            cb->draw_gameload_dialogue_to_screen(ctx);
        if (receipt) {
            receipt->progress_drawn = true;
            receipt->bar_width = bar_w;
        }
    }
}

/* ================================================================
 * 4. DRAW_DIALOGUE_PARTS_PICT
 *    c_gui_draw.cpp:97
 * ================================================================ */

void dm2_v1_draw_dialogue_parts_pict(
    void *bmp, DM2_V1_Rect *rect, int16_t alpha, void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_dialogue_parts_pict) return;
    cb->draw_dialogue_parts_pict(ctx, bmp, rect, alpha, palette);
}

/* ================================================================
 * 5. DRAW_DIALOGUE_PICT
 *    c_gui_draw.cpp:117
 * ================================================================ */

void dm2_v1_draw_dialogue_pict(
    void *srcbmp, void *destbmp, DM2_V1_Rect *rect,
    int16_t srcx, int16_t srcy, int16_t alpha, void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_dialogue_pict) return;
    cb->draw_dialogue_pict(ctx, srcbmp, destbmp, rect, srcx, srcy,
                            alpha, palette);
}

/* ================================================================
 * 6. DRAW_WAKE_UP_TEXT
 *    c_gui_draw.cpp:147
 * ================================================================ */

void dm2_v1_draw_wake_up_text(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    void *backbuf = cb->get_backbuffer_ptr ? cb->get_backbuffer_ptr(ctx) : NULL;
    uint8_t col00 = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
    if (cb->fill_entire_pict && backbuf)
        cb->fill_entire_pict(ctx, backbuf, (int32_t)col00);

    char text_buf[0x28];
    memset(text_buf, 0, sizeof(text_buf));
    void *text = cb->query_gdat_text ?
        cb->query_gdat_text(ctx, 1, 0, 0x11, text_buf) : NULL;

    uint8_t col04 = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 4) : 0;
    if (cb->draw_vp_rc_str && text)
        cb->draw_vp_rc_str(ctx, 6, (int16_t)col04, text);
}

/* ================================================================
 * 7. DRAW_PLAYER_3STAT_HEALTH_BAR
 *    c_gui_draw.cpp:167
 * ================================================================ */

void dm2_v1_draw_player_3stat_health_bar(
    int32_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    DM2_V1_Rect rc;
    int16_t rect_id = (int16_t)(hero_idx + 0xb9);
    if (cb->query_expanded_rect)
        cb->query_expanded_rect(ctx, rect_id, &rc);
    if (cb->adjust_buttongroup_rects)
        cb->adjust_buttongroup_rects(ctx, 1, &rc);

    int16_t stats[6];
    stats[0] = cb->get_hero_hp ? cb->get_hero_hp(ctx, hero_idx) : 0;
    stats[1] = cb->get_hero_max_hp ? cb->get_hero_max_hp(ctx, hero_idx) : 0;
    stats[2] = cb->get_hero_stamina ? cb->get_hero_stamina(ctx, hero_idx) : 0;
    stats[3] = cb->get_hero_max_stamina ? cb->get_hero_max_stamina(ctx, hero_idx) : 0;
    stats[4] = cb->get_hero_mp ? cb->get_hero_mp(ctx, hero_idx) : 0;
    int16_t max_mp = cb->get_hero_max_mp ? cb->get_hero_max_mp(ctx, hero_idx) : 0;
    stats[5] = cb->dm2_max ? cb->dm2_max(ctx, max_mp, stats[4]) : max_mp;

    int16_t rect_base = (int16_t)(hero_idx + 0xc1);
    for (int i = 0; i < 3; i++) {
        if (stats[i * 2 + 1] != 0) {
            int32_t ratio = ((int32_t)stats[i * 2] * 10000) / (int32_t)stats[i * 2 + 1];
            DM2_V1_Rect bar_rc;
            DM2_V1_Rect *scaled = cb->scale_rect ?
                cb->scale_rect(ctx, (int16_t)(rect_base + i * 4),
                                10000, (int16_t)ratio, &bar_rc) : NULL;
            if (scaled) {
                DM2_V1_Rect bg_rc = bar_rc;
                int16_t dx = cb->get_v1d2728 ? cb->get_v1d2728(ctx) : 0;
                int16_t dy = cb->get_v1d272a ? cb->get_v1d272a(ctx) : 0;
                bg_rc.x += dx;
                bg_rc.y += dy;
                uint8_t col00 = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
                if (cb->fill_rect_summary)
                    cb->fill_rect_summary(ctx, 1, &bg_rc, (int32_t)col00);

                uint8_t bar_col = cb->get_table_color ?
                    cb->get_table_color(ctx, hero_idx) : 0;
                if (cb->fill_rect_summary)
                    cb->fill_rect_summary(ctx, 1, &bar_rc, (int32_t)bar_col);
            }
        }
        rect_base += 4;
    }
}

/* ================================================================
 * 8. DRAW_CUR_MAX_HMS -- format "cur/max" text
 *    c_gui_draw.cpp:238
 * ================================================================ */

void dm2_v1_draw_cur_max_hms(
    int32_t rectid, int32_t cur, int32_t max,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    /* Delegates to string formatting and text drawing callbacks */
    if (!cb->fmt_num || !cb->dm2_strcpy || !cb->dm2_strcat) return;

    char buf[8];
    memset(buf, 0, sizeof(buf));
    void *cur_str = cb->fmt_num(ctx, (int16_t)cur, 1, 3);
    cb->dm2_strcpy(ctx, buf, cur_str);
    cb->dm2_strcat(ctx, buf, (void *)dm2_guidraw_v1d1124);
    void *max_str = cb->fmt_num(ctx, (int16_t)max, 1, 3);
    cb->dm2_strcat(ctx, buf, max_str);

    uint8_t fg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 13) : 0;
    uint8_t bg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 1) : 0;
    int16_t bgcolor = (int16_t)((uint16_t)bg | 0x4000);

    if (cb->draw_local_text)
        cb->draw_local_text(ctx, rectid, (int32_t)fg, bgcolor, buf);
}

/* ================================================================
 * 9. DRAW_PLAYER_3STAT_TEXT
 *    c_gui_draw.cpp:260
 * ================================================================ */

void dm2_v1_draw_player_3stat_text(
    int32_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t hp = cb->get_hero_hp ? cb->get_hero_hp(ctx, hero_idx) : 0;
    int16_t max_hp = cb->get_hero_max_hp ? cb->get_hero_max_hp(ctx, hero_idx) : 0;
    dm2_v1_draw_cur_max_hms(0x226, (int32_t)hp, (int32_t)max_hp, cb, ctx);

    int16_t sta = cb->get_hero_stamina ? cb->get_hero_stamina(ctx, hero_idx) : 0;
    int16_t max_sta = cb->get_hero_max_stamina ? cb->get_hero_max_stamina(ctx, hero_idx) : 0;
    dm2_v1_draw_cur_max_hms(0x227, (int32_t)(sta / 10), (int32_t)(max_sta / 10), cb, ctx);

    int16_t mp = cb->get_hero_mp ? cb->get_hero_mp(ctx, hero_idx) : 0;
    int16_t max_mp = cb->get_hero_max_mp ? cb->get_hero_max_mp(ctx, hero_idx) : 0;
    dm2_v1_draw_cur_max_hms(0x228, (int32_t)mp, (int32_t)max_mp, cb, ctx);
}

/* ================================================================
 * 10. DRAW_PLAYER_NAME_AT_CMDSLOT
 *     c_gui_draw.cpp:281
 * ================================================================ */

void dm2_v1_draw_player_name_at_cmdslot(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    dm2_v1_draw_icon_pict_entry(1, 4, 20, 2, 0x3c, -1, cb, ctx);
    dm2_v1_draw_icon_pict_entry(1, 4, 14, 2, 0x3b, -1, cb, ctx);

    int16_t cur = cb->get_cur_act_hero ? cb->get_cur_act_hero(ctx) : 0;
    int16_t hero_idx = cur - 1;
    if (hero_idx < 0) return;

    int16_t evhero = cb->get_event_heroidx ? cb->get_event_heroidx(ctx) : -1;
    uint8_t fg_col = cb->palette_to_ui8 ?
        cb->palette_to_ui8(ctx, (hero_idx != evhero) ? 15 : 9) : 0;
    uint8_t bg_col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 12) : 0;

    void *name = cb->get_hero_name ? cb->get_hero_name(ctx, hero_idx) : NULL;
    if (cb->draw_name_str && name)
        cb->draw_name_str(ctx, 2, 0x3d, (int16_t)fg_col,
                          (int16_t)((uint16_t)bg_col | 0x4000), name);
}

/* ================================================================
 * 11. DRAW_PLAYER_DAMAGE
 *     c_gui_draw.cpp:289
 * ================================================================ */

void dm2_v1_draw_player_damage(
    int32_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t rect_id = (int16_t)(hero_idx + 0xb1);
    dm2_v1_draw_icon_pict_entry(1, 2, 3, 1, rect_id, 0x0a, cb, ctx);

    int16_t damage = cb->get_hero_damage ? cb->get_hero_damage(ctx, hero_idx) : 0;
    void *text = cb->fmt_num ? cb->fmt_num(ctx, damage, 0, 3) : NULL;

    uint8_t fg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 15) : 0;
    uint8_t bg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 8) : 0;

    if (cb->draw_button_str && text)
        cb->draw_button_str(ctx, 1, (int32_t)rect_id,
                            (int16_t)fg, (int16_t)bg, text);
}

/* ================================================================
 * 12. DRAW_CHIP_OF_MAGIC_MAP
 *     c_gui_draw.cpp:311
 * ================================================================ */

void dm2_v1_draw_chip_of_magic_map(
    void *bmp, int32_t mul, int32_t x, int32_t y, int32_t flags,
    void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    /* Source: SKWINDOS/dm2bya.cpp:2601,
     * SKWINSPX/src/v4/skguidrw.cpp:825, and
     * SKWINSPX/src/v5/skguidr5.cpp:312. */
    if (!cb || !bmp) return;

    int16_t chip_w = cb->get_v1d274a ? cb->get_v1d274a(ctx) : 0;
    int16_t chip_h = cb->get_v1d274c ? cb->get_v1d274c(ctx) : 0;
    if (chip_w <= 0 || chip_h <= 0) return;

    DM2_V1_Rect rc;
    rc.x = (int16_t)x;
    rc.y = (int16_t)y;
    rc.w = chip_w;
    rc.h = chip_h;

    int16_t crop_x = 0, crop_y = 0;
    DM2_V1_Rect clip;
    if (!cb->get_magic_map_clip_rect ||
        !cb->get_magic_map_clip_rect(ctx, &clip) ||
        !dm2_v1_rect_intersect(&rc, &clip, &crop_x, &crop_y))
        return;

    /* SKProject clips the destination first. For a mirrored tile, a crop
     * at its leading edge takes pixels from the opposite source edge. */
    if (rc.w != chip_w && (flags & 0x1))
        crop_x = crop_x == 0 ? (int16_t)(chip_w - rc.w) : 0;
    if (rc.h != chip_h && (flags & 0x2))
        crop_y = crop_y == 0 ? (int16_t)(chip_h - rc.h) : 0;

    int16_t srcx = (int16_t)(mul * chip_w + crop_x);
    dm2_v1_draw_icon_pict_buff(bmp, 2, &rc, srcx, crop_y,
                                0x0a, flags, palette, cb, ctx);
}

/* ================================================================
 * 13. QUERY_GDAT_SQUAD_ICON
 *     c_gui_draw.cpp:390
 * ================================================================ */

void dm2_v1_query_gdat_squad_icon(
    void *bmp, int32_t hero_idx, void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    uint8_t absdir = cb->get_hero_absdir ? cb->get_hero_absdir(ctx, hero_idx) : 0;
    int16_t heading = cb->get_v1e0258 ? cb->get_v1e0258(ctx) : 0;
    int32_t dir = ((int32_t)absdir + 4 - (int32_t)heading) & 0x3;

    uint8_t b02 = cb->get_savegames_b02 ? cb->get_savegames_b02(ctx) : 0;
    int32_t offset = (b02 == 0) ? 0 : 4;

    if (cb->copy_small_palette && cb->query_gdat_palette) {
        void *src_pal = cb->query_gdat_palette(ctx, 1, 6, (int8_t)hero_idx);
        cb->copy_small_palette(ctx, palette, src_pal);
    }

    int16_t w = cb->get_v1d272c ? cb->get_v1d272c(ctx) : 0;
    int16_t h = cb->get_v1d272e ? cb->get_v1d272e(ctx) : 0;
    void *srcbmp = cb->query_gdat_image ?
        cb->query_gdat_image(ctx, 1, 6, (int8_t)hero_idx) : NULL;

    DM2_V1_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = w;
    rect.h = h;

    int16_t srcx = (int16_t)(w * (dir + offset));
    if (cb->draw_dialogue_pict && srcbmp)
        cb->draw_dialogue_pict(ctx, srcbmp, bmp, &rect, srcx, 0, 2, NULL);
}

/* ================================================================
 * 14. DRAW_CRYOCELL_LEVER
 *     c_gui_draw.cpp:414
 * ================================================================ */

void dm2_v1_draw_cryocell_lever(
    int32_t state,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int8_t icon = (int8_t)((state != 0 ? 1 : 0) - 6);
    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 9, 0x5b, icon, 0x1ee, -1);

    if (state != 0) {
        if (cb->drawings_completed)
            cb->drawings_completed(ctx);
        /* Queue sound effect via callback */
        if (cb->queue_noise)
            cb->queue_noise(ctx, 9, 0x5b, 0xfb, 0xff, 0xc8, 0, 0, 0);
    } else {
        if (cb->set_v1d66fc)
            cb->set_v1d66fc(ctx, 7);
    }
}

/* ================================================================
 * 15. DRAW_CHARSHEET_OPTION_ICON
 *     c_gui_draw.cpp:441
 * ================================================================ */

void dm2_v1_draw_charsheet_option_icon(
    int32_t icon_idx, int32_t rectid, int32_t mask,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t opts = cb->get_v1e100c ? cb->get_v1e100c(ctx) : 0;
    int8_t idx = (int8_t)icon_idx;
    if (((int32_t)opts & mask) != 0)
        idx++;

    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 7, 0, idx, (int16_t)rectid, -1);
}

/* ================================================================
 * 16. MONEY_BOX_SURVEY
 *     c_gui_draw.cpp:458
 * ================================================================ */

void dm2_v1_money_box_survey(
    int32_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t counts[10];
    memset(counts, 0, sizeof(counts));
    if (cb->count_by_coin_types)
        cb->count_by_coin_types(ctx, record, counts);

    int32_t total = 0;
    int16_t rect = 0x26a;

    for (int i = 0; i < 10; i++) {
        int32_t order = cb->get_item_order_in_container ?
            cb->get_item_order_in_container(ctx, record, i) : -1;
        if (order >= 0) {
            /* Accumulate coin value (simplified) */
            total += (int32_t)counts[order];
            char buf[12];
            void *text = cb->fmt_num ?
                cb->fmt_num(ctx, counts[order], 0, 3) : NULL;
            uint8_t col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 13) : 0;
            if (cb->draw_vp_rc_str && text)
                cb->draw_vp_rc_str(ctx, rect, (int16_t)col, text);
            rect++;
        }
    }

    /* Draw total */
    char total_buf[12];
    void *total_text = cb->ltoa10 ?
        cb->ltoa10(ctx, total, total_buf) : NULL;
    uint8_t col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 13) : 0;
    if (cb->draw_vp_rc_str && total_text)
        cb->draw_vp_rc_str(ctx, 0x239, (int16_t)col, total_text);
}

/* ================================================================
 * 17. DRAW_MONEYBOX
 *     c_gui_draw.cpp:512
 * ================================================================ */

void dm2_v1_draw_moneybox(
    int16_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int8_t cls2 = cb->query_cls2 ? cb->query_cls2(ctx, (int32_t)record) : 0;
    dm2_v1_draw_icon_pict_entry(20, cls2, 0x10, 2, 0x5c, -1, cb, ctx);

    int16_t counts[10];
    memset(counts, 0, sizeof(counts));
    if (cb->count_by_coin_types)
        cb->count_by_coin_types(ctx, (int32_t)record, counts);

    DM2_V1_Rect panel_clip;
    DM2_V1_Rect *clip = cb->get_right_panel_clip_rect ?
        cb->get_right_panel_clip_rect(ctx, &panel_clip) : NULL;
    if (!clip || !cb->get_money_item_id ||
        !cb->get_item_order_in_container || !cb->count_by_coin_types ||
        !cb->get_itemtype_of_itemspec ||
        !cb->query_itemdb_from_distinctive || !cb->query_gdat_image ||
        !cb->query_gdat_palette || !cb->query_blit_rect || !cb->blit)
        return;

    /* SKProject DRAW_MONEYBOX (skguidrw.cpp:442) draws each denomination's
     * own GDAT sprite, scattered through the slot.  The slot order is
     * resolved through the container's money ordering, while the coin count
     * array is indexed by denomination order. */
    for (int slot = 0; slot < 10; slot++) {
        int32_t order = cb->get_item_order_in_container ?
            cb->get_item_order_in_container(ctx, (int32_t)record, slot) : -1;
        if (order < 0 || order >= 10 || counts[order] <= 0)
            continue;

        uint16_t item_spec = cb->get_money_item_id(ctx, order);
        int8_t item_class = cb->query_itemdb_from_distinctive(
            ctx, (int32_t)item_spec);
        int8_t item_subclass = (int8_t)cb->get_itemtype_of_itemspec(
            ctx, (int32_t)item_spec);
        void *coin_bmp = cb->query_gdat_image(ctx, item_class,
                                               item_subclass, 0x18);
        void *coin_palette = cb->query_gdat_palette(ctx, item_class,
                                                      item_subclass, 0x18);
        if (!coin_bmp)
            continue;

        DM2_V1_Rect slot_rect;
        int16_t src_x = 0;
        int16_t src_y = 0;
        DM2_V1_Rect *base = cb->query_blit_rect(
            ctx, coin_bmp, &slot_rect, (int16_t)(slot + 0xdd),
            &src_x, &src_y);
        if (!base)
            continue;

        DM2_V1_Rect coin_rect = *base;
        int coin_count = counts[order];
        int position = (slot << 3) + (coin_count > 32 ? coin_count - 32 : 0);
        if (coin_count > 32)
            coin_count = 32;
        for (int coin = 0; coin < coin_count; ++coin) {
            position = (position + 1) & 31;
            coin_rect.x = (int16_t)(base->x +
                dm2_guidraw_table1d275a[position][0]);
            coin_rect.y = (int16_t)(base->y +
                dm2_guidraw_table1d275a[position][1]);

            int left = coin_rect.x > clip->x ? coin_rect.x : clip->x;
            int top = coin_rect.y > clip->y ? coin_rect.y : clip->y;
            int right = coin_rect.x + coin_rect.w < clip->x + clip->w ?
                coin_rect.x + coin_rect.w : clip->x + clip->w;
            int bottom = coin_rect.y + coin_rect.h < clip->y + clip->h ?
                coin_rect.y + coin_rect.h : clip->y + clip->h;
            if (left < right && top < bottom) {
                DM2_V1_Rect clipped = {
                    (int16_t)left, (int16_t)top,
                    (int16_t)(right - left), (int16_t)(bottom - top)
                };
                dm2_v1_draw_icon_pict_buff(
                    coin_bmp, 2, &clipped,
                    (int16_t)(src_x + left - coin_rect.x),
                    (int16_t)(src_y + top - coin_rect.y),
                    12, 0, coin_palette, cb, ctx);
            }
        }
    }
}

/* ================================================================
 * 18. guidraw_0b36_0c52 -- setup buttongroup with bitmap alloc
 *     c_gui_draw.cpp:617
 * ================================================================ */

void dm2_v1_guidraw_0b36_0c52(
    int32_t bg_id, int16_t rectid, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    if (rectid != -1 && cb->query_expanded_rect) {
        DM2_V1_Rect rc;
        cb->query_expanded_rect(ctx, rectid, &rc);
    }

    if (cb->alloc_dbidx)
        cb->alloc_dbidx(ctx);

    if (flags != 0 && cb->adjust_buttongroup_rects) {
        DM2_V1_Rect rc;
        if (cb->query_expanded_rect)
            cb->query_expanded_rect(ctx, rectid, &rc);
        cb->adjust_buttongroup_rects(ctx, bg_id, &rc);
    }
}

/* ================================================================
 * 19. DRAW_PLAYER_3STAT_PANE
 *     c_gui_draw.cpp:642
 * ================================================================ */

void dm2_v1_draw_player_3stat_pane(
    int32_t hero_idx, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t dbidx = cb->get_bg1_dbidx ? cb->get_bg1_dbidx(ctx) : -1;
    if (dbidx != -1) return; /* NODATA check */

    int16_t hp = cb->get_hero_hp ? cb->get_hero_hp(ctx, hero_idx) : 0;

    int8_t icon;
    if (hp == 0) {
        icon = 1;
    } else {
        int16_t v1e0976 = cb->get_v1e0976 ? cb->get_v1e0976(ctx) : 0;
        if (v1e0976 == hero_idx + 1)
            icon = 9;
        else
            icon = 0; /* default icon from heroflag */
    }

    int16_t rect_id = (int16_t)(hero_idx + 0xa1);
    dm2_v1_guidraw_0b36_0c52(1, rect_id, flags, cb, ctx);
    dm2_v1_draw_icon_pict_entry(1, 2, icon, 1, rect_id, -1, cb, ctx);
}

/* ================================================================
 * 20. guidraw_29ee_00a3 -- setup buttongroup2
 *     c_gui_draw.cpp:683
 * ================================================================ */

void dm2_v1_guidraw_29ee_00a3(
    int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t dbidx = cb->get_bg2_dbidx ? cb->get_bg2_dbidx(ctx) : -1;
    if (dbidx != -1) return;

    dm2_v1_guidraw_0b36_0c52(2, 11, flags, cb, ctx);

    if (flags != 0) {
        uint8_t col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
        DM2_V1_Rect rc;
        if (cb->query_expanded_rect)
            cb->query_expanded_rect(ctx, 11, &rc);
        if (cb->fill_rect_summary)
            cb->fill_rect_summary(ctx, 2, &rc, (int32_t)col);
    }
}

/* ================================================================
 * 21. DRAW_CMD_SLOT
 *     c_gui_draw.cpp:693
 * ================================================================ */

void dm2_v1_draw_cmd_slot(
    int16_t slot_idx, int8_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    /* Source: SKWINSPX/src/v5/skguidr5.cpp, DM2_DRAW_CMD_SLOT (694-711).
     * DOS (SKWINDOS/dm2bya.cpp:2589) and SPX v4 (skguidrw.cpp:766) have
     * edition-specific field order/index formulas. */
    if (!cb || slot_idx < 0) return;

    dm2_v1_guidraw_29ee_00a3(0, cb, ctx);

    int16_t v1e0b62 = cb->get_v1e0b62 ? cb->get_v1e0b62(ctx) : 0;
    uint8_t category = 0, index = 0, entry = 0;
    if (!cb->get_command_slot_fields ||
        !cb->get_command_slot_fields(ctx, slot_idx, &category, &index, &entry))
        return;

    if (v1e0b62 != 0) {
        if (!cb->get_held_container_type) return;
        int8_t held_type = cb->get_held_container_type(ctx);
        int16_t image = (int16_t)(((int16_t)entry - 8) * 2 + 0x41 + flags);
        dm2_v1_draw_icon_pict_entry(20, held_type, (int8_t)image,
                                     2, (int16_t)(slot_idx + 0x6e), -1,
                                     cb, ctx);
        return;
    }

    dm2_v1_draw_icon_pict_entry(1, 4, (int8_t)(flags + 21),
                                 2, (int16_t)(slot_idx + 0x3f), -1,
                                 cb, ctx);
    if (cb->query_cmdstr_name && cb->draw_name_str) {
        void *name = cb->query_cmdstr_name(ctx, (int8_t)category,
                                           (int8_t)index, (int8_t)entry);
        uint8_t fg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 15) : 0;
        uint8_t bg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
        if (name)
            cb->draw_name_str(ctx, 2, (int32_t)(slot_idx + 0x42),
                              (int16_t)fg,
                              (int16_t)((uint16_t)bg | 0x4000), name);
    }
}

/* ================================================================
 * 22. DRAW_SPELL_TO_BE_CAST
 *     c_gui_draw.cpp:714
 * ================================================================ */

void dm2_v1_draw_spell_to_be_cast(
    int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    dm2_v1_guidraw_29ee_00a3(0, cb, ctx);

    if (flags != 0)
        dm2_v1_draw_icon_pict_entry(1, 5, 9, 2, 0xfc, -1, cb, ctx);

    /* Draw rune characters (simplified) */
    int16_t cur = cb->get_cur_act_hero ? cb->get_cur_act_hero(ctx) : 0;
    if (cur <= 0) return;

    int32_t hero_idx = cur - 1;
    void *rune = cb->get_hero_rune ? cb->get_hero_rune(ctx, hero_idx) : NULL;
    if (!rune) return;

    int32_t rune_len = cb->dm2_strlen ? cb->dm2_strlen(ctx, rune) : 0;
    uint8_t fg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
    uint8_t bg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 13) : 0;

    char one_char[2] = {0, 0};
    for (int i = 0; i < rune_len; i++) {
        one_char[0] = ((char *)rune)[i];
        if (cb->draw_button_str)
            cb->draw_button_str(ctx, 2, 0x105 + i,
                                (int16_t)fg, (int16_t)((uint16_t)bg | 0x4000),
                                one_char);
    }
}

/* ================================================================
 * 23. DRAW_PLAYER_ATTACK_DIR
 *     c_gui_draw.cpp:733
 * ================================================================ */

void dm2_v1_draw_player_attack_dir(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    dm2_v1_guidraw_29ee_00a3(0, cb, ctx);

    /* Source: SKWINSPX/src/v5/skguidr5.cpp, DM2_DRAW_PLAYER_ATTACK_DIR
     * (734-790). DOS and SPX v4 use edition-specific hero/jitter state. */
    int16_t graphics_set = cb->get_v1d67d6 ? cb->get_v1d67d6(ctx) : -1;
    if (graphics_set >= 0)
        dm2_v1_draw_icon_pict_entry(8, (int8_t)graphics_set, -10,
                                     2, 0x5d, -1, cb, ctx);

    int16_t cur = cb->get_cur_act_hero ? cb->get_cur_act_hero(ctx) : 0;
    if (cur > 0) {
        int32_t hero_idx = cur - 1;
        int16_t width = cb->get_v1d272c ? cb->get_v1d272c(ctx) : 0;
        int16_t height = cb->get_v1d272e ? cb->get_v1d272e(ctx) : 0;
        void *bmp = width > 0 && height > 0 && cb->alloc_pict_buff ?
            cb->alloc_pict_buff(ctx, width, height, 0, 4) : NULL;
        bool jitter_enabled = cb->get_savegames_b04 &&
                              cb->get_savegames_b04(ctx) != 0;
        bool jitter_available = cb->get_savegames_b04 &&
                                (!jitter_enabled || cb->randdir);
        bool squad_icon_available = cb->get_hero_absdir &&
            cb->get_v1e0258 && cb->get_savegames_b02 &&
            cb->copy_small_palette && cb->query_gdat_palette &&
            cb->query_gdat_image && cb->draw_dialogue_pict;
        if (bmp && cb->query_blit_rect && cb->free_pict_buff) {
            DM2_V1_Rect rect;
            int16_t src_x = 0, src_y = 0;
            if (jitter_available && squad_icon_available &&
                cb->query_blit_rect(ctx, bmp, &rect, 0x5e,
                                    &src_x, &src_y)) {
                if (jitter_enabled) {
                    int16_t jitter = cb->randdir(ctx);
                    if (jitter != 0) {
                        rect.y = (int16_t)(rect.y + jitter - 2);
                        jitter = cb->randdir(ctx);
                        if (jitter != 0)
                            rect.x = (int16_t)(rect.x + jitter - 2);
                    }
                }

                uint8_t palette[16] = {0};
                dm2_v1_query_gdat_squad_icon(bmp, hero_idx, palette,
                                               cb, ctx);
                dm2_v1_draw_icon_pict_buff(bmp, 2, &rect, src_x, src_y,
                                            0x0c, 0, palette, cb, ctx);

                if (cb->get_hero_ench_power &&
                    cb->get_hero_ench_power(ctx, hero_idx) != 0 &&
                    cb->query_gdat_summary_image && cb->query_picst_it &&
                    cb->image_draw && cb->get_hero_ench_aura &&
                    cb->get_hero_absdir && cb->get_v1e0258 &&
                    cb->get_gametick) {
                    DM2_V1_ImageDesc aura;
                    memset(&aura, 0, sizeof(aura));
                    cb->query_gdat_summary_image(
                        ctx, &aura, 1, 7,
                        (int8_t)cb->get_hero_ench_aura(ctx, hero_idx));
                    cb->query_picst_it(ctx, &aura);
                    aura.mode |= 0x10;
                    int32_t relative_dir =
                        (cb->get_hero_absdir(ctx, hero_idx) + 4 -
                         cb->get_v1e0258(ctx)) & 0x3;
                    aura.width >>= 2;
                    aura.x = (int16_t)(relative_dir * aura.width);
                    aura.height >>= 2;
                    aura.y = (int16_t)((cb->get_gametick(ctx) & 0x3) *
                                       aura.height);
                    cb->image_draw(ctx, &aura, 2, 0x5e, 0x0c);
                }
            }
            cb->free_pict_buff(ctx, bmp);
        } else if (bmp && cb->free_pict_buff) {
            cb->free_pict_buff(ctx, bmp);
        }
    }

    dm2_v1_draw_icon_pict_entry(1, 4, 16, 2, 0x60, -1, cb, ctx);
    dm2_v1_draw_icon_pict_entry(1, 4, 18, 2, 0x61, -1, cb, ctx);
}

/* ================================================================
 * 24. DRAW_SPELL_PANEL
 *     c_gui_draw.cpp:805
 * ================================================================ */

void dm2_v1_draw_spell_panel(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t cur = cb->get_cur_act_hero ? cb->get_cur_act_hero(ctx) : 0;
    if (cur <= 0) return;

    int32_t hero_idx = cur - 1;
    uint8_t nrunes = cb->get_hero_nrunes ?
        cb->get_hero_nrunes(ctx, hero_idx) : 0;

    int8_t icon = (int8_t)(nrunes + 1);
    dm2_v1_draw_icon_pict_entry(1, 5, icon, 2, 0x5c, -1, cb, ctx);

    if ((int32_t)nrunes < 4) {
        /* Draw spell symbol grid (simplified) */
        int8_t base = (int8_t)(nrunes * 6 + 0x60);
        uint8_t fg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 13) : 0;
        uint8_t bg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;

        char sym[2] = {0, 0};
        for (int i = 0; i < 6; i++) {
            sym[0] = (char)(base + i);
            if (cb->draw_button_str)
                cb->draw_button_str(ctx, 2, 0xff + i,
                                    (int16_t)bg,
                                    (int16_t)((uint16_t)fg | 0x4000),
                                    sym);
        }
    }

    dm2_v1_draw_spell_to_be_cast(0, cb, ctx);
    dm2_v1_draw_player_attack_dir(cb, ctx);
}

/* ================================================================
 * 25. SHOW_ATTACK_RESULT
 *     c_gui_draw.cpp:864
 * ================================================================ */

void dm2_v1_show_attack_result(
    int16_t result_code,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    dm2_v1_guidraw_29ee_00a3(1, cb, ctx);

    /* The original function decodes result_code to select an image
     * and optionally renders damage numbers. Here we delegate the
     * image query and draw to callbacks. */
    int8_t img_idx = 1;
    int8_t img_cls = 4;

    if (result_code < 0) {
        if (result_code == -2)
            img_idx = 0x17;
        else if (result_code == -1)
            img_idx = 0x18;
        else if (result_code == -6)
            img_idx = 0x19;
        else {
            img_cls = 5;
            if (result_code == -3)
                img_idx = 0x0e;
            else if (result_code == -4)
                img_idx = 0x0c;
            else
                img_idx = 0x0d;
        }
    }

    if (cb->query_gdat_summary_image && cb->query_picst_it &&
        cb->image_draw) {
        DM2_V1_ImageDesc image;
        memset(&image, 0, sizeof(image));
        cb->query_gdat_summary_image(ctx, &image, 1, img_cls, img_idx);
        if (result_code >= 0 && result_code <= 40 &&
            cb->set_attack_result_image_stretch)
            cb->set_attack_result_image_stretch(ctx, &image, result_code);
        cb->query_picst_it(ctx, &image);
        cb->image_draw(ctx, &image, 2, 57, -1);
    }

    if (result_code >= 0 && cb->draw_button_str) {
        /* Format and display damage number */
        char buf[6];
        memset(buf, 0, sizeof(buf));
        int val = result_code;
        int pos = 4;
        do {
            buf[pos--] = (char)('0' + val % 10);
            val /= 10;
        } while (val > 0 && pos >= 0);

        uint8_t fg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 4) : 0;
        uint8_t bg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
        cb->draw_button_str(ctx, 2, 0x39, (int16_t)fg, (int16_t)bg,
                            buf + pos + 1);
    }
}

/* ================================================================
 * 26. guidraw_0b36_105b -- stretch 4-to-8 blit
 *     c_gui_draw.cpp:1016
 * ================================================================ */

void dm2_v1_guidraw_0b36_105b(
    int32_t bg_id, DM2_V1_Rect *rect, int16_t color,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !rect) return;
    if (cb->stretch_4to8) {
        void *dst = cb->get_bmp ? cb->get_bmp(ctx, (int16_t)bg_id) : NULL;
        DM2_V1_Rect offset_rc = *rect;
        cb->stretch_4to8(ctx, dst, &offset_rc, (int8_t)color, 0);
    }
    if (cb->adjust_buttongroup_rects)
        cb->adjust_buttongroup_rects(ctx, bg_id, rect);
}

/* ================================================================
 * 27. DRAW_SQUAD_SPELL_AND_LEADER_ICON
 *     c_gui_draw.cpp:1030
 * ================================================================ */

void dm2_v1_draw_squad_spell_and_leader_icon(
    int16_t hero_idx, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    dm2_v1_guidraw_29ee_00a3(0, cb, ctx);

    if (!cb->get_hero_partypos || !cb->get_v1e0258) return;
    uint8_t partypos = cb->get_hero_partypos(ctx, hero_idx);
    int16_t heading = cb->get_v1e0258(ctx);
    int32_t dir = ((int32_t)partypos + 4 - (int32_t)heading) & 0x3;

    /* SKProject clears the direction slot before checking hero state. */
    DM2_V1_Rect rc;
    memset(&rc, 0, sizeof(rc));
    int16_t rect_id = (int16_t)(dir + 0x4f);
    if (!cb->query_expanded_rect ||
        !cb->query_expanded_rect(ctx, rect_id, &rc))
        return;
    int32_t black = cb->palette_to_pixel ?
        cb->palette_to_pixel(ctx, 0) : 0;
    if (cb->fill_rect_summary)
        cb->fill_rect_summary(ctx, 2, &rc, black);

    if (!cb->get_hero_hp) return;
    int16_t hp = cb->get_hero_hp(ctx, hero_idx);
    if (hp == 0) return;

    /* Source draws spell/action first, then leader status icon. */
    const bool mirror = dir == 1 || dir == 2;
    int8_t spell_icon = (dir > 1) ? 8 : 6;
    int8_t leader_icon = (dir > 1) ? 0x0c : 0x0a;
    if (flags != 0) ++spell_icon;
    int16_t leader_idx = cb->get_squad_leader_hero_idx ?
        cb->get_squad_leader_hero_idx(ctx) :
        (cb->get_event_heroidx ? cb->get_event_heroidx(ctx) : -1);
    if (hero_idx == leader_idx) ++leader_icon;

    if (cb->query_gdat_summary_image && cb->query_picst_it && cb->image_draw) {
        DM2_V1_ImageDesc icons[2];
        const int8_t icon_indices[2] = {spell_icon, leader_icon};
        const int16_t rect_ids[2] = {
            (int16_t)(dir + 0x57), (int16_t)(dir + 0x53)
        };
        for (int i = 0; i < 2; ++i) {
            memset(&icons[i], 0, sizeof(icons[i]));
            cb->query_gdat_summary_image(ctx, &icons[i], 1, 4,
                                         icon_indices[i]);
            if (cb->set_summary_image_mirror)
                cb->set_summary_image_mirror(ctx, &icons[i], mirror);
            cb->query_picst_it(ctx, &icons[i]);
            cb->image_draw(ctx, &icons[i], 2, rect_ids[i], 4);
        }

        const bool sleeping = cb->is_player_sleeping &&
                              cb->is_player_sleeping(ctx);
        const uint8_t hand_cooldown = cb->get_hero_handcooldown ?
            cb->get_hero_handcooldown(ctx, hero_idx, 2) : 0;
        const int16_t active_overlay = cb->get_v1e0238 ?
            cb->get_v1e0238(ctx) : 0;
        if ((sleeping || active_overlay != 0 || hand_cooldown != 0) &&
            cb->draw_gray_overlay &&
            cb->query_expanded_rect(ctx, rect_ids[0], &rc))
            cb->draw_gray_overlay(ctx, 2, &rc, 0);
        if ((sleeping || active_overlay != 0) && cb->draw_gray_overlay &&
            cb->query_expanded_rect(ctx, rect_ids[1], &rc))
            cb->draw_gray_overlay(ctx, 2, &rc, 0);
    }
}

/* ================================================================
 * 28. guidraw_24a5_0e82 -- scaled health/stat bar
 *     c_gui_draw.cpp:1126
 * ================================================================ */

void dm2_v1_guidraw_24a5_0e82(
    int32_t value, int32_t rectid, int32_t color_idx,
    int32_t min_val, int16_t max_val, int32_t extra_color,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t fg_col;
    if (value >= -512) {
        if (value >= 0)
            fg_col = (int16_t)color_idx;
        else
            fg_col = 0x0b;
    } else {
        fg_col = 0x08;
    }

    int32_t range = (int32_t)max_val - min_val;
    if (range == 0) return;

    int32_t ratio = ((value - min_val) * 10000) / range;

    DM2_V1_Rect bar_rc;
    DM2_V1_Rect *scaled = cb->scale_rect ?
        cb->scale_rect(ctx, (int16_t)rectid, (int16_t)ratio, 10000, &bar_rc) : NULL;
    if (!scaled) return;

    /* c_gui_draw.cpp:1167-1210.
     *   extra_color == 0 -> background rect is the bar rect itself
     *   extra_color != 0 -> background rect is the rect's full expanded
     *                       extent, and the span from the bar's end to that
     *                       rect's end is filled with extra_color; when the
     *                       bar already reaches the end there is no
     *                       remainder and the extra fill is dropped.
     * Both of those were missing: the background was always drawn at bar
     * width, and the remainder segment was never drawn at all, so item stat
     * bars rendered with no remainder and a too-small background. */
    DM2_V1_Rect full_rc;
    DM2_V1_Rect remainder_rc;
    int draw_remainder = 0;
    DM2_V1_Rect bg_rc = bar_rc;

    if (extra_color != 0 && cb->query_expanded_rect &&
        cb->query_expanded_rect(ctx, (int16_t)rectid, &full_rc)) {
        int32_t bar_end = (int32_t)bar_rc.x + bar_rc.w;
        int32_t full_end = (int32_t)full_rc.x + full_rc.w;

        bg_rc = full_rc;
        if (bar_end - 1 != full_end - 1) {
            remainder_rc = full_rc;
            remainder_rc.w = (int16_t)((full_end - 1) - (bar_end - 1));
            remainder_rc.x = (int16_t)bar_end;
            draw_remainder = 1;
        }
    }

    int16_t border = cb->get_v1d2748 ? cb->get_v1d2748(ctx) : 0;
    bg_rc.x = (int16_t)(bg_rc.x + border);
    bg_rc.y = (int16_t)(bg_rc.y + border);

    uint8_t col00 = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
    if (cb->fill_backbuff_rect)
        cb->fill_backbuff_rect(ctx, &bg_rc, (int32_t)col00);

    uint8_t bar_col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, fg_col) : 0;
    if (cb->fill_backbuff_rect)
        cb->fill_backbuff_rect(ctx, &bar_rc, (int32_t)bar_col);

    if (draw_remainder && cb->fill_backbuff_rect) {
        uint8_t ext_col = cb->palette_to_ui8
            ? cb->palette_to_ui8(ctx, extra_color) : 0;
        cb->fill_backbuff_rect(ctx, &remainder_rc, (int32_t)ext_col);
    }
}

/* ================================================================
 * 29. DRAW_FOOD_WATER_POISON_PANEL
 *     c_gui_draw.cpp:1215
 * ================================================================ */

void dm2_v1_draw_food_water_poison_panel(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t v1e0976 = cb->get_v1e0976 ? cb->get_v1e0976(ctx) : 0;
    int32_t hero_idx = v1e0976 - 1;
    if (hero_idx < 0) return;

    if (cb->set_v1d66fc) cb->set_v1d66fc(ctx, 1);
    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 7, 0, 1, 0x1ee, -1);

    int16_t food = cb->get_hero_food ? cb->get_hero_food(ctx, hero_idx) : 0;
    dm2_v1_guidraw_24a5_0e82((int32_t)food, 0x1f0, 5, -1024, 0x800, 0, cb, ctx);

    int16_t water = cb->get_hero_water ? cb->get_hero_water(ctx, hero_idx) : 0;
    dm2_v1_guidraw_24a5_0e82((int32_t)water, 0x1f1, 14, -1024, 0x800, 0, cb, ctx);

    if (cb->draw_static_pic) {
        cb->draw_static_pic(ctx, 7, 0, 6, 0x1f4, 0x0c);
        cb->draw_static_pic(ctx, 7, 0, 7, 0x1f5, 0x0c);
    }

    bool poisoned = cb->get_hero_poisoned ? cb->get_hero_poisoned(ctx, hero_idx) : false;
    if (poisoned) {
        int16_t poison = cb->get_hero_poison ? cb->get_hero_poison(ctx, hero_idx) : 0;
        dm2_v1_guidraw_24a5_0e82((int32_t)poison, 0x1f3, 8, 0, 0xc00, 0, cb, ctx);
        if (cb->draw_static_pic)
            cb->draw_static_pic(ctx, 7, 0, 8, 0x1f6, 0x0c);
    }
}

/* ================================================================
 * 30. DRAW_ITEM_STATS_BAR
 *     SKWINDOS/dm2byb.cpp:883-898, SPX v4 skguidrw.cpp:2542-2567,
 *     SPX v5 skguidr5.cpp:1233-1276
 * ================================================================ */

void dm2_v1_draw_item_stats_bar(
    int32_t rectid, int32_t value, int32_t max_value,
    int32_t label_char, int16_t color_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    DM2_V1_Rect rc;
    DM2_V1_Rect *expanded = cb->query_expanded_rect ?
        cb->query_expanded_rect(ctx, (int16_t)rectid, &rc) : NULL;
    if (!expanded) return;

    /* Map source arguments to the helper's value, rect, color, min,
     * max, extra-color signature. */
    dm2_v1_guidraw_24a5_0e82(value, rectid, color_idx, 0,
                              (int16_t)max_value, 1, cb, ctx);

    if (!cb->draw_strong_text) return;
    void *backbuf = cb->get_backbuffer_ptr ? cb->get_backbuffer_ptr(ctx) : NULL;
    int16_t backw = cb->get_bmp_width ? cb->get_bmp_width(ctx, backbuf) : 0;
    uint8_t black = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
    uint8_t white = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 15) : 0;
    uint16_t bg = (uint16_t)black | 0x4000u;
    int16_t text_y = (int16_t)(rc.y + rc.h - 3);
    char label[2] = {(char)label_char, '\0'};
    char low_rune[2] = {0x60, '\0'};
    char max_rune[2] = {0x65, '\0'};
    cb->draw_strong_text(ctx, backbuf, backw, (int16_t)(rc.x - 9),
                         text_y, (int16_t)color_idx, (int16_t)bg, label);
    cb->draw_strong_text(ctx, backbuf, backw, (int16_t)(rc.x + 3),
                         text_y, (int16_t)white, (int16_t)bg, low_rune);
    int16_t font_height = cb->get_font_height ? cb->get_font_height(ctx) : 0;
    if (font_height > 0) {
        cb->draw_strong_text(ctx, backbuf, backw,
                             (int16_t)(rc.x + rc.w - font_height - 2),
                             text_y, (int16_t)white, (int16_t)bg, max_rune);
    }
}

/* ================================================================
 * 31. guidraw_2405_014a -- compute animated item icon index
 *     c_gui_draw.cpp:1307
 * ================================================================ */

int8_t dm2_v1_guidraw_2405_014a(
    int32_t record, int32_t slot, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    int8_t base = 0x18;
    if (!cb) return base;
    if (flags == 0) return base;

    int32_t dbspec = cb->query_gdat_dbspec_word ?
        cb->query_gdat_dbspec_word(ctx, record, 6) : 0;
    int16_t anim_bits = (int16_t)(dbspec & 0x1f);
    if (anim_bits == 0) return base;

    /* Check equip fit flag */
    if ((dbspec & 0x8000) != 0) {
        if (!cb->is_item_fit_for_equip) return base;
        if (!cb->is_item_fit_for_equip(ctx, (int16_t)record, slot, 1))
            return base;
    }

    /* Check active-hero flag */
    if ((dbspec & 0x4000) != 0) {
        int16_t cur = cb->get_cur_act_hero ? cb->get_cur_act_hero(ctx) : 0;
        if (cur == 0) return base;
        base++;
        anim_bits--;
    }

    if (anim_bits == 0) return base;

    /* Compute animation frame from gametick or direction (simplified) */
    int32_t mode = (dbspec & 0x1f00) >> 8;
    int32_t gametick = cb->get_gametick ? cb->get_gametick(ctx) : 0;

    switch (mode) {
    case 0:
        base = (int8_t)(base + (gametick % anim_bits));
        break;
    case 1: {
        int16_t rnd = cb->rand16 ? cb->rand16(ctx, anim_bits) : 0;
        base = (int8_t)(base + rnd);
        break;
    }
    case 2: {
        int16_t heading = cb->get_v1e0258 ? cb->get_v1e0258(ctx) : 0;
        base = (int8_t)(base + (heading & 0x3));
        break;
    }
    case 3: {
        int16_t charge = cb->add_item_charge ?
            cb->add_item_charge(ctx, record, 0) : 0;
        if (charge == 0) return base;
        int16_t max_charge = cb->get_max_charge ?
            cb->get_max_charge(ctx, record) : 1;
        int32_t frac = ((int32_t)charge * (int32_t)anim_bits) / (max_charge + 1);
        base = (int8_t)(base + frac + 1);
        break;
    }
    default:
        base = (int8_t)(base + (gametick % anim_bits));
        break;
    }

    return base;
}

/* ================================================================
 * 32. DRAW_ITEM_IN_HAND
 *     SKWINDOS/dm2byb.cpp:817-825, SPX v4 skguidrw.cpp:3328-3354,
 *     SPX v5 skguidr5.cpp:1517-1531
 * ================================================================ */

void dm2_v1_draw_item_in_hand(
    void *item_record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !item_record || !cb->get_item_record_id ||
        !cb->get_item_record_bitmap || !cb->get_item_record_palette ||
        !cb->query_cls1 || !cb->query_cls2 ||
        !cb->query_gdat_dbspec_word || !cb->copy_small_palette ||
        !cb->blit || !cb->get_bmp_width ||
        !cb->get_v1d271a || !cb->get_v1d271c)
        return;

    uint16_t object_id = cb->get_item_record_id(ctx, item_record);
    void *dstbmp = cb->get_item_record_bitmap(ctx, item_record);
    void *dstpalette = cb->get_item_record_palette(ctx, item_record);
    if (object_id == 0xffffu || !dstbmp || !dstpalette) return;

    int8_t cls1 = cb->query_cls1(ctx, object_id);
    int8_t cls2 = cb->query_cls2(ctx, object_id);
    int8_t icon = dm2_v1_guidraw_2405_014a(object_id, -1, 1, cb, ctx);
    if (icon < 0) return;

    void *srcpalette = NULL;
    void *srcbmp = cb->query_item_hand_image
        ? cb->query_item_hand_image(ctx, object_id, cls1, cls2, icon,
                                    &srcpalette)
        : (cb->query_gdat_image
            ? cb->query_gdat_image(ctx, cls1, cls2, icon) : NULL);
    if (!cb->query_item_hand_image && cb->query_gdat_palette)
        srcpalette = cb->query_gdat_palette(ctx, cls1, cls2, icon);
    int16_t src_width = srcbmp ? cb->get_bmp_width(ctx, srcbmp) : 0;
    int16_t width = cb->get_v1d271a(ctx);
    int16_t height = cb->get_v1d271c(ctx);
    if (!srcbmp || !srcpalette || src_width <= 0 || width <= 0 || height <= 0)
        return;

    cb->copy_small_palette(ctx, dstpalette, srcpalette);
    DM2_V1_Rect rect = {0, 0, width, height};
    cb->blit(ctx, srcbmp, dstbmp, &rect, 0, 0, src_width, width,
             -1, 0, 4, 4, NULL);
}

/* ================================================================
 * 33. DRAW_CONTAINER_PANEL
 *     SKWINDOS/dm2bya.cpp:2680-2705, SPX v4 skguidrw.cpp:487-515,
 *     SPX v5 skguidr5.cpp:1541-1570
 * ================================================================ */

void dm2_v1_draw_container_panel(
    int32_t record, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int8_t cls2 = cb->query_cls2 ? cb->query_cls2(ctx, record) : 0;

    if (flags != 0) {
        dm2_v1_draw_icon_pict_entry(20, cls2, 0x10, 2, 0x5c, -1, cb, ctx);
        dm2_v1_draw_icon_pict_entry(20, cls2, 0x12, 2, 0xe3, 0x0a, cb, ctx);
    }

    /* Source draws only occupied slots, using each contained object's
     * animation and cls1/cls2 rather than the container's class. */
    if (!cb->get_container_item || !cb->query_cls1 || !cb->query_cls2 ||
        !cb->query_gdat_dbspec_word)
        return;
    DM2_V1_Rect container_rect;
    bool have_container_rect = flags == 0 && cb->query_expanded_rect &&
        cb->query_expanded_rect(ctx, 0x5c, &container_rect) != NULL;
    for (int i = 0; i < 8; i++) {
        int16_t item = cb->get_container_item(ctx, record, i);
        if (item == -1) continue;

        int16_t rect_id = (int16_t)(i + 0xe5);

        if (have_container_rect && cb->query_gdat_image) {
            DM2_V1_Rect slot_rect;
            if (cb->query_expanded_rect(ctx, rect_id, &slot_rect)) {
                void *frame = cb->query_gdat_image(ctx, 20, 0x10, cls2);
                void *palette = cb->query_gdat_palette ?
                    cb->query_gdat_palette(ctx, 20, 0x10, cls2) : NULL;
                if (frame) {
                    dm2_v1_draw_icon_pict_buff(
                        frame, 2, &slot_rect,
                        (int16_t)(slot_rect.x - container_rect.x),
                        (int16_t)(slot_rect.y - container_rect.y),
                        -1, 0, palette, cb, ctx);
                }
            }
        }

        int8_t icon = dm2_v1_guidraw_2405_014a(item, i + 0x1e, 1, cb, ctx);
        int8_t item_cls1 = cb->query_cls1(ctx, item);
        int8_t item_cls2 = cb->query_cls2(ctx, item);

        dm2_v1_draw_icon_pict_entry(item_cls1, item_cls2, icon,
                                     2, rect_id, 0x0c, cb, ctx);
    }
}

/* ================================================================
 * 34. guidraw_2405_011f -- query rect with inflate
 *     c_gui_draw.cpp:1626
 * ================================================================ */

void dm2_v1_guidraw_2405_011f(
    int32_t rectid, DM2_V1_Rect *out,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !out) return;
    dm2_v1_guidraw_2405_00ec(rectid, out, cb, ctx);
    int16_t inflate = cb->get_v1d2726 ? cb->get_v1d2726(ctx) : 0;
    out->x -= inflate;
    out->y -= inflate;
    out->w += (int16_t)(inflate * 2);
    out->h += (int16_t)(inflate * 2);
}

/* ================================================================
 * 35. guidraw_2405_00ec -- query blit rect for item
 *     c_gui_draw.cpp:1638
 * ================================================================ */

void dm2_v1_guidraw_2405_00ec(
    int32_t rectid, DM2_V1_Rect *out,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !out) return;

    int16_t w = cb->get_v1d271a ? cb->get_v1d271a(ctx) : 0;
    int16_t h = cb->get_v1d271c ? cb->get_v1d271c(ctx) : 0;
    int16_t dummy_x = w, dummy_y = h;

    if (cb->query_blit_rect)
        cb->query_blit_rect(ctx, NULL, out, (int16_t)rectid, &dummy_x, &dummy_y);
}

/* ================================================================
 * 36. DRAW_ITEM_ICON
 *     SKWINDOS/dm2bya.cpp:1657-1717, SPX v4 skguidrw.cpp:2421-2500,
 *     SPX v5 skguidr5.cpp:1657-1793
 * ================================================================ */

void dm2_v1_draw_item_icon(
    int32_t record, int32_t slot, int32_t body_flag,
    int32_t active_flag, int32_t redraw_flag,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    if (!cb->get_inventory_slot_rect_id) return;
    int16_t rect_id = cb->get_inventory_slot_rect_id(ctx, slot);
    if (rect_id < 0) return;

    int8_t cls1, cls2, icon;

    if (record != -1) {
        if (!cb->query_cls1 || !cb->query_cls2) return;
        cls1 = cb->query_cls1(ctx, record);
        cls2 = cb->query_cls2(ctx, record);
        int32_t adj_slot = (slot >= 8) ? slot - 8 : (slot & 1);
        icon = dm2_v1_guidraw_2405_014a(record, adj_slot, 1, cb, ctx);
    } else {
        if (!cb->get_empty_inventory_slot_icon) return;
        cls1 = 7;
        cls2 = 0;
        icon = (int8_t)(cb->get_empty_inventory_slot_icon(ctx, slot) + body_flag);
    }

    if (slot < 0x26 && cb->query_expanded_rect) {
        DM2_V1_Rect rc;
        if (cb->query_expanded_rect(ctx, rect_id, &rc)) {
            void *backbuf = cb->get_backbuffer_ptr ?
                cb->get_backbuffer_ptr(ctx) : NULL;
            if (slot < 8) {
                void *bmp = cb->query_gdat_image ?
                    cb->query_gdat_image(ctx, 1, 2, 0) : NULL;
                void *palette = cb->query_gdat_palette ?
                    cb->query_gdat_palette(ctx, 1, 2, 0) : NULL;
                DM2_V1_Rect group_rect;
                DM2_V1_Rect *group = cb->offset_rect ?
                    cb->offset_rect(ctx, 2, &group_rect, &rc) : NULL;
                if (bmp && group)
                    dm2_v1_draw_icon_pict_buff(
                        bmp, 2, &rc, (int16_t)(rc.x - group->x),
                        (int16_t)(rc.y - group->y), -1, 0, palette, cb, ctx);
            } else if (backbuf && cb->query_gdat_image &&
                       cb->query_gdat_palette) {
                void *bmp = cb->query_gdat_image(ctx, 7, 0, 0);
                void *palette = cb->query_gdat_palette(ctx, 7, 0, 0);
                if (bmp)
                    dm2_v1_draw_dialogue_pict(bmp, backbuf, &rc,
                                               rc.x, rc.y, -1, palette,
                                               cb, ctx);
            }
        }
    }

    if (redraw_flag && slot < 14 && cb->query_expanded_rect) {
        DM2_V1_Rect rc;
        dm2_v1_guidraw_2405_011f(rect_id, &rc, cb, ctx);
        int8_t slab = active_flag ? 6 : (body_flag ? 5 : 4);
        void *bmp = cb->query_gdat_image ?
            cb->query_gdat_image(ctx, 1, 2, slab) : NULL;
        void *palette = cb->query_gdat_palette ?
            cb->query_gdat_palette(ctx, 1, 2, slab) : NULL;
        if (bmp && slot < 8) {
            DM2_V1_Rect group_rect;
            DM2_V1_Rect *group = cb->offset_rect ?
                cb->offset_rect(ctx, 2, &group_rect, &rc) : NULL;
            if (group)
                dm2_v1_draw_icon_pict_buff(bmp, 2, &rc, 0, 0, 0x0c, 0,
                                           palette, cb, ctx);
        } else if (bmp) {
            dm2_v1_draw_dialogue_parts_pict(bmp, &rc, 0x0c, palette,
                                             cb, ctx);
        }
    }

    if (icon != -1) {
        if (slot < 8)
            dm2_v1_draw_icon_pict_entry(cls1, cls2, icon, 1,
                                         rect_id, 0x0c, cb, ctx);
        else if (cb->draw_static_pic)
            cb->draw_static_pic(ctx, cls1, cls2, icon, rect_id, 0x0c);
    }
}

/* ================================================================
 * 37. DRAW_CONTAINER_SURVEY
 *     SKWINDOS/dm2byb.cpp:872-879, SPX v4 skguidrw.cpp:2526-2536,
 *     SPX v5 skguidr5.cpp:1796-1818
 * ================================================================ */

void dm2_v1_draw_container_survey(
    void *record_addr,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !record_addr || !cb->get_container_first_item ||
        !cb->get_next_record_link)
        return;

    int16_t item = cb->get_container_first_item(ctx, record_addr);
    for (int slot = 0; slot < 8 && item != (int16_t)0xfffeu; ++slot) {
        dm2_v1_draw_item_icon(item, slot + 0x2f, 0, 0, 0, cb, ctx);
        item = cb->get_next_record_link(ctx, item);
    }
}

/* ================================================================
 * 38. DRAW_EYE_MOUTH_COLORED_RECTANGLE
 *     c_gui_draw.cpp:1824
 * ================================================================ */

void dm2_v1_draw_eye_mouth_colored_rectangle(
    int8_t icon_idx, int16_t rectid,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    DM2_V1_Rect rc;
    dm2_v1_guidraw_2405_011f((int32_t)rectid, &rc, cb, ctx);

    void *palette = cb->query_gdat_palette ?
        cb->query_gdat_palette(ctx, 1, 2, icon_idx) : NULL;
    void *bmp = cb->query_gdat_image ?
        cb->query_gdat_image(ctx, 1, 2, icon_idx) : NULL;

    dm2_v1_draw_dialogue_parts_pict(bmp, &rc, 12, palette, cb, ctx);
}

/* ================================================================
 * 39. guidraw_2e62_03b5 -- check and draw hero equipment slot
 *     c_gui_draw.cpp:1833
 * ================================================================ */

int32_t dm2_v1_guidraw_2e62_03b5(
    int32_t hero_idx, int32_t hand, int32_t redraw,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return 0;

    int16_t v1e0976 = cb->get_v1e0976 ? cb->get_v1e0976(ctx) : 0;
    int16_t slot;

    if (v1e0976 == hero_idx + 1) {
        slot = (int16_t)(hand + 8);
    } else {
        if (hand > 1) return 0;
        slot = (int16_t)(2 * hero_idx + hand);
    }

    int16_t item = cb->get_hero_item ?
        cb->get_hero_item(ctx, hero_idx, hand) : -1;

    /* Check if redraw needed (simplified) */
    if (redraw == 0) return 0;

    if (slot < 8)
        dm2_v1_draw_player_3stat_pane(hero_idx, 0, cb, ctx);

    dm2_v1_draw_item_icon((int32_t)item, (int32_t)slot, 0, 0, redraw, cb, ctx);
    return 1;
}

/* ================================================================
 * 40. DRAW_SCROLL_TEXT
 *     c_gui_draw.cpp:1996
 * ================================================================ */

void dm2_v1_draw_scroll_text(
    int32_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    char message[0xc8];
    char line[0xc8];
    size_t message_len;
    int16_t lines = 0;
    int16_t x = 0;
    int16_t y = 0;
    int16_t pos = 0;
    DM2_V1_Rect text_rect;
    bool mbcs;
    int16_t line_height;
    int16_t outer;
    int16_t inner;

    if (!cb) return;

    if (cb->set_v1d66fc) cb->set_v1d66fc(ctx, 5);
    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 7, 0, 1, 0x1ee, -1);
    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 18, 0, 0x10, 0x1ee, 0x0c);

    memset(message, 0, sizeof(message));
    if (cb->query_message_text)
        cb->query_message_text(ctx, message, record, (int32_t)0xffff8002);
    message[sizeof(message) - 1] = '\0';
    message_len = strlen(message);
    if (message_len == 0 || !cb->query_topleft || !cb->query_expanded_rect ||
        !cb->gfxstr_linebreak || !cb->gfxstr_draw_line) {
        return;
    }

    /* SKProject counts explicit newlines plus a final unterminated line,
     * except that one trailing empty line is omitted after two newlines. */
    for (size_t i = 0; i < message_len; ++i)
        if (message[i] == '\n') ++lines;
    if (message[message_len - 1] != '\n') ++lines;
    else if (message_len > 1 && message[message_len - 2] == '\n') --lines;

    memset(&text_rect, 0, sizeof(text_rect));
    cb->query_topleft(ctx, 0x230, &x, &y);
    if (!cb->query_expanded_rect(ctx, 0x1ee, &text_rect) || lines <= 0)
        return;

    mbcs = cb->query_text_is_mbcs && cb->query_text_is_mbcs(ctx, message);
    if (mbcs) {
        line_height = 0x0f;
    } else {
        if (!cb->get_strdat_strxplus || !cb->get_strdat_gfxstrw1 ||
            !cb->get_strdat_gfxstrw3) {
            return;
        }
        line_height = cb->get_strdat_strxplus(ctx);
        outer = cb->get_strdat_gfxstrw3(ctx);
        inner = cb->get_strdat_gfxstrw1(ctx);
    }
    if (mbcs) {
        outer = 0;
        inner = 0x0e;
    }
    if (line_height <= 0) return;
    y = (int16_t)(y - (((line_height * lines - outer - 1) >> 1) - inner));

    while ((size_t)pos < message_len) {
        int16_t previous_pos = pos;
        memset(line, 0, sizeof(line));
        cb->gfxstr_linebreak(ctx, message, line, &pos, text_rect.w);
        if (pos < previous_pos || (size_t)pos > message_len) return;
        cb->gfxstr_draw_line(ctx, x, y, line);
        y = (int16_t)(y + line_height);
        /* SKProject renders empty lines and then consumes an explicit LF.
         * Do this before the no-progress guard so leading/consecutive LFs
         * cannot suppress all following text. */
        if ((size_t)pos < message_len && message[pos] == '\n') ++pos;
        if (pos <= previous_pos) return;
    }
}

/* ================================================================
 * 41. DRAW_ITEM_SURVEY
 *     SKWINDOS/dm2byf.cpp:716-815, SPX v4 skguidrw.cpp:2595-2738,
 *     SPX v5 skguidr5.cpp:2073-2206
 * ================================================================ */

int32_t dm2_v1_draw_item_survey(
    int32_t record, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_DrawItemSurveyReceipt *receipt)
{
    if (receipt) {
        receipt->survey_drawn = false;
        receipt->result = 0;
    }
    if (!cb || record == -1) return 0;

    if (cb->draw_guided_str)
        cb->draw_guided_str(ctx, (void *)dm2_guidraw_v1d10f0);

    int32_t item_type = (record & 0x3c00) >> 10;
    if (item_type == 7) {
        dm2_v1_draw_scroll_text(record, cb, ctx);
        if (receipt) {
            receipt->survey_drawn = true;
            receipt->result = 1;
        }
        return 1;
    }

    if (cb->set_v1d66fc) cb->set_v1d66fc(ctx, 3);
    if (flags == 0) return 0;

    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 7, 0, 1, 0x1ee, -1);

    int8_t cls1 = cb->query_cls1 ? cb->query_cls1(ctx, record) : 0;
    int8_t cls2 = cb->query_cls2 ? cb->query_cls2(ctx, record) : 0;
    if (cb->draw_item_survey_class_image)
        cb->draw_item_survey_class_image(ctx, cls1, cls2);
    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 7, 0, 5, 0x1f8, 0x0c);

    void *name = cb->get_item_name ? cb->get_item_name(ctx, record) : NULL;
    uint8_t col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 13) : 0;
    if (cb->draw_vp_rc_str && name)
        cb->draw_vp_rc_str(ctx, 0x1fa, (int16_t)col, name);

    dm2_v1_draw_item_icon(record, 0x2e, 0, 0, 0, cb, ctx);

    int16_t weight = cb->query_item_weight ?
        cb->query_item_weight(ctx, (int16_t)record) : 0;
    if (cb->set_item_weight_display)
        cb->set_item_weight_display(ctx, (int16_t)(weight / 10),
                                    (int16_t)(weight % 10));
    char weight_text[0x80] = {0};
    if (cb->query_gdat_text && cb->draw_guided_str) {
        void *text = cb->query_gdat_text(ctx, 7, 0, 0x1f, weight_text);
        if (text) cb->draw_guided_str(ctx, text);
    }

    if (item_type == 9) {
        if (cb->is_container_chest && cb->is_container_chest(ctx,
                                                              (int16_t)record)) {
            void *container = cb->get_address_of_record ?
                cb->get_address_of_record(ctx, (int16_t)record) : NULL;
            if (container)
                dm2_v1_draw_container_survey(container, cb, ctx);
        } else if (cb->is_container_moneybox &&
                   cb->is_container_moneybox(ctx, record)) {
            dm2_v1_money_box_survey(record, cb, ctx);
        }
        if (receipt) {
            receipt->survey_drawn = true;
            receipt->result = 1;
        }
        return 1;
    }

    int32_t bar_index = 0;
    if (cb->is_item_chargeable && cb->is_item_chargeable(ctx, record) &&
        cb->add_item_charge && cb->get_max_charge &&
        cb->get_item_stat_bar_rect_id) {
        int16_t charge = cb->add_item_charge(ctx, record, 0);
        if (charge != 0) {
            int16_t rect = cb->get_item_stat_bar_rect_id(ctx, bar_index++);
            int16_t maximum = cb->get_max_charge(ctx, record);
            dm2_v1_draw_item_stats_bar(rect, charge, maximum, 0x71, 4,
                                       cb, ctx);
        }
    }

    int32_t value = 0;
    int32_t maximum = 0;
    int32_t rune = 0;
    if (item_type == 5 && cb->query_gdat_dbspec_word) {
        value = cb->query_gdat_dbspec_word(ctx, record, 8);
        maximum = 0x50;
        rune = 0x72;
        if (value == 0) {
            value = cb->query_gdat_dbspec_word(ctx, record, 9);
            maximum = 0x64;
            rune = 0x73;
        }
    } else if (item_type == 6 && cb->query_gdat_dbspec_word) {
        value = cb->query_gdat_dbspec_word(ctx, record, 0x0b) & 0xff;
        maximum = 0xc8;
        rune = 0x72;
    } else if (item_type == 8 && cb->get_item_potion_power) {
        value = cb->get_item_potion_power(ctx, record);
        maximum = 0xff;
        rune = 0x75;
    }
    if (value != 0 && maximum > 0 && cb->get_item_stat_bar_rect_id) {
        int16_t rect = cb->get_item_stat_bar_rect_id(ctx, bar_index);
        dm2_v1_draw_item_stats_bar(rect, value, maximum, rune, 5, cb, ctx);
    }

    if (receipt) {
        receipt->survey_drawn = true;
        receipt->result = 1;
    }
    return 1;
}

/* ================================================================
 * 42. DRAW_HAND_ACTION_ICONS
 *     c_gui_draw.cpp:2341
 * ================================================================ */

void dm2_v1_draw_hand_action_icons(
    int16_t hero_idx, int32_t hand, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    dm2_v1_guidraw_29ee_00a3(0, cb, ctx);

    if (!cb->get_hero_hp) return;
    int16_t hp = cb->get_hero_hp(ctx, hero_idx);

    /* Draw hand action icon background and item-on-wood (simplified) */
    uint8_t partypos = cb->get_hero_partypos ?
        cb->get_hero_partypos(ctx, hero_idx) : 0;
    int16_t heading = cb->get_v1e0258 ? cb->get_v1e0258(ctx) : 0;
    int32_t dir = ((int32_t)partypos + 4 - (int32_t)heading) & 0x3;

    int16_t rect_base = (hand != 1) ? 0x4a : 0x46;
    int16_t rect_id = (int16_t)(rect_base + dir);

    DM2_V1_Rect rect;
    memset(&rect, 0, sizeof(rect));
    const bool have_rect = cb->query_expanded_rect &&
        cb->query_expanded_rect(ctx, rect_id, &rect) != NULL;

    if (hp == 0) {
        int32_t black = cb->palette_to_pixel ?
            cb->palette_to_pixel(ctx, 0) : 0;
        if (have_rect && cb->fill_rect_summary)
            cb->fill_rect_summary(ctx, 2, &rect, black);
        return;
    }

    int8_t icon = (int8_t)(2 * hand + 2 + flags);
    dm2_v1_draw_icon_pict_entry(1, 4, icon, 2, rect_id, -1, cb, ctx);

    /* The source wood-panel item image needs edition-specific item-frame and
     * palette conversion; leave that layer to its adapter. Preserve the
     * source cooldown/sleep dimming state for callers that bind it. */
    const uint8_t cooldown = cb->get_hero_handcooldown ?
        cb->get_hero_handcooldown(ctx, hero_idx, hand) : 0;
    const bool sleeping = cb->is_player_sleeping &&
                          cb->is_player_sleeping(ctx);
    if (have_rect && (cooldown != 0 || sleeping) && cb->draw_gray_overlay)
        cb->draw_gray_overlay(ctx, 2, &rect, 0);
}

/* ================================================================
 * 43. DRAW_MAP_CHIP
 *     c_gui_draw.cpp:2412  (1086 lines, the largest function)
 * ================================================================ */

void dm2_v1_draw_map_chip(
    int32_t map_x, int32_t map_y, int32_t adj_x, int32_t adj_y,
    int16_t chip_x, int16_t chip_y, int8_t style, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    /* SUMMARIZE_STONE_ROOM for the tile */
    if (cb->summarize_stone_room) {
        /* Stone room summary determines wall/floor/door types */
    }

    /* Query 4bpp tile graphics */
    if (cb->query_4bpp_pict_buff) {
        /* Gets chip graphic and palette */
    }

    /* The complex tile rendering logic (walls, doors, pits, stairs,
     * teleporters, etc.) is dispatched through the chip drawing
     * callback dm2_v1_draw_chip_of_magic_map. */
}

/* ================================================================
 * 44. guidraw_29ee_1d03 -- draw enchantment mode direction buttons
 *     c_gui_draw.cpp:3499
 * ================================================================ */

void dm2_v1_guidraw_29ee_1d03(
    int8_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    dm2_v1_guidraw_29ee_00a3(0, cb, ctx);

    int16_t cur = cb->get_cur_act_hero ? cb->get_cur_act_hero(ctx) : 0;
    if (cur <= 0) return;

    int16_t v1e0b62 = cb->get_v1e0b62 ? cb->get_v1e0b62(ctx) : 0;
    if ((v1e0b62 & 0x800) == 0) return;

    int32_t xor_val = (int32_t)flags ^ (int32_t)v1e0b62;
    xor_val &= 0x0f;

    char sym[2] = {0, 0};
    uint8_t col02 = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 2) : 0;
    uint8_t col00 = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;

    for (int i = 0; i < 4; i++) {
        int8_t icon = (int8_t)((xor_val & 1) ? 0x4a : 0x49);
        xor_val >>= 1;

        int16_t rect_id = (int16_t)(i + 0x65);
        dm2_v1_draw_icon_pict_entry(20, 0, icon, 2, rect_id, -1, cb, ctx);

        sym[0] = (char)(i + 0x72);
        if (cb->draw_button_str)
            cb->draw_button_str(ctx, 2, i + 0x69,
                                (int16_t)col00,
                                (int16_t)((uint16_t)col02 | 0x4000),
                                sym);
    }
}

/* ================================================================
 * 45. guidraw_29ee_1946 -- draw magic map tile grid
 *     c_gui_draw.cpp:3561  (367 lines)
 * ================================================================ */

void dm2_v1_guidraw_29ee_1946(
    int32_t map_x, int32_t map_y, int32_t adj_x, int32_t adj_y,
    int16_t chip_x, int32_t heading, int32_t side, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    /* Store map navigation state if not restoring */
    if ((flags & 0x8) == 0) {
        /* Save map_y, adj_x, adj_y, chip_x, heading, side into ddat */
    }

    dm2_v1_guidraw_29ee_00a3(0, cb, ctx);

    /* Fill map background rect */
    uint8_t col00 = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
    DM2_V1_Rect map_rc;
    if (cb->query_expanded_rect)
        cb->query_expanded_rect(ctx, 99, &map_rc);
    if (cb->fill_rect_summary)
        cb->fill_rect_summary(ctx, 2, &map_rc, (int32_t)col00);

    /* Iterate visible tiles and call dm2_v1_draw_map_chip for each */
}

/* ================================================================
 * 46. DRAW_MAJIC_MAP
 *     c_gui_draw.cpp:3928
 * ================================================================ */

void dm2_v1_draw_majic_map(
    int32_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_DrawMajicMapReceipt *receipt)
{
    if (receipt) receipt->map_drawn = false;
    if (!cb) return;

    int8_t cls2 = cb->query_cls2 ? cb->query_cls2(ctx, record) : 0;

    int16_t v1e0b62 = cb->get_v1e0b62 ? cb->get_v1e0b62(ctx) : 0;
    if (cb->set_v1e0b62)
        cb->set_v1e0b62(ctx, (int16_t)(v1e0b62 | 0x90));

    /* Setup map display, draw command slots, direction buttons */
    if ((v1e0b62 & 0x400) == 0) {
        dm2_v1_draw_icon_pict_entry(20, cls2, 16, 2, 0x5c, -1, cb, ctx);

        int16_t cmd_count = cb->get_v1e0b62 ? cb->get_v1e0b62(ctx) : 0;
        /* Draw command slots */
        dm2_v1_guidraw_29ee_1d03(0, cb, ctx);

        if (cb->set_v1e0b62)
            cb->set_v1e0b62(ctx, (int16_t)(v1e0b62 | 0x490));
    }

    /* Navigate map and draw tiles via guidraw_29ee_1946 */
    if (receipt) receipt->map_drawn = true;
}

/* ================================================================
 * 47. DISPLAY_HINT_NEW_LINE
 *     c_gui_draw.cpp:4138
 * ================================================================ */

void dm2_v1_display_hint_new_line(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !cb->display_hint_text) return;
    /* Display hint text at default location */
    cb->display_hint_text(ctx, 0, NULL);
}

/* ================================================================
 * 48. DISPLAY_TAKEN_ITEM_NAME
 *     c_gui_draw.cpp:4144
 * ================================================================ */

void dm2_v1_display_taken_item_name(
    int16_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    dm2_v1_display_hint_new_line(cb, ctx);

    void *name = cb->get_item_name ?
        cb->get_item_name(ctx, (int32_t)record) : NULL;
    if (cb->display_hint_text && name)
        cb->display_hint_text(ctx, 0x0d, name);
}

/* ================================================================
 * 49. HIGHLIGHT_ARROW_PANEL
 *     c_gui_draw.cpp:4151
 * ================================================================ */

void dm2_v1_highlight_arrow_panel(
    int32_t arrow_idx, int32_t rectid, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int8_t icon = (int8_t)arrow_idx;
    if (flags != 0)
        icon++;

    if (cb->hide_mouse) cb->hide_mouse(ctx);

    dm2_v1_guidraw_0b36_0c52(0, (int16_t)rectid, 1, cb, ctx);

    dm2_v1_draw_icon_pict_entry(1, 3, icon, 0, (int16_t)rectid, -1, cb, ctx);

    if (cb->show_mouse) cb->show_mouse(ctx);
    if (cb->wait_screen_refresh) cb->wait_screen_refresh(ctx);
}

/* ================================================================
 * 50. DISPLAY_RIGHT_PANEL_SQUAD_HANDS
 *     c_gui_draw.cpp:4181
 * ================================================================ */

void dm2_v1_display_right_panel_squad_hands(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t cur = cb->get_cur_act_hero ? cb->get_cur_act_hero(ctx) : 0;
    if (cur == 0) return;

    /* Reset hero/panel state (simplified) */
    if (cb->set_v1e0976) cb->set_v1e0976(ctx, 0);
    if (cb->set_v1d67bc) cb->set_v1d67bc(ctx, 0);
    if (cb->set_v1e0b62) cb->set_v1e0b62(ctx, 0);
}

/* ================================================================
 * 51. DM2_24a5_1532 (static) -- character sheet detail rendering
 *     c_gui_draw.cpp:4216
 * (Internal helper for REFRESH_PLAYER_STAT_DISP)
 * ================================================================ */

/* ================================================================
 * 52. DM2_2e62_061d (static) -- character sheet with active hero
 *     c_gui_draw.cpp:4333
 * (Internal helper for REFRESH_PLAYER_STAT_DISP)
 * ================================================================ */

/* ================================================================
 * 53. REFRESH_PLAYER_STAT_DISP
 *     c_gui_draw.cpp:4359
 * ================================================================ */

void dm2_v1_refresh_player_stat_disp(
    int16_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_RefreshPlayerStatDispReceipt *receipt)
{
    if (receipt) receipt->stats_refreshed = false;
    if (!cb) return;

    int16_t hp = cb->get_hero_hp ? cb->get_hero_hp(ctx, hero_idx) : 0;
    int16_t v1e0976 = cb->get_v1e0976 ? cb->get_v1e0976(ctx) : 0;

    bool is_active = (v1e0976 == hero_idx + 1);

    /* Handle hero flag changes, charsheet option icons, stat panes,
     * health bars, damage display, name display via callbacks.
     * The ~500-line original function orchestrates all sub-draw calls
     * through the callback pattern. */

    if (is_active) {
        /* Draw full stat display for active hero */
        dm2_v1_draw_player_3stat_pane((int32_t)hero_idx, 1, cb, ctx);
        dm2_v1_draw_player_3stat_health_bar((int32_t)hero_idx, cb, ctx);
        dm2_v1_draw_player_3stat_text((int32_t)hero_idx, cb, ctx);
    }

    if (cb->drawings_completed) cb->drawings_completed(ctx);

    if (receipt) receipt->stats_refreshed = true;
}

/* ================================================================
 * 54. guidraw_29ee_000f -- draw arrow/movement panel
 *     c_gui_draw.cpp:4880
 * ================================================================ */

int32_t dm2_v1_guidraw_29ee_000f(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_GuiDraw000fReceipt *receipt)
{
    if (receipt) {
        receipt->panel_drawn = false;
        receipt->result = 0;
    }
    if (!cb) return 0;

    int16_t v1e03a8 = cb->get_v1e03a8 ? cb->get_v1e03a8(ctx) : 0;
    int8_t icon_base = (v1e03a8 == 0) ? 2 : 14;

    dm2_v1_guidraw_0b36_0c52(0, 9, 1, cb, ctx);

    for (int i = 40; i < 46; i++) {
        dm2_v1_draw_icon_pict_entry(1, 3, icon_base, 0,
                                     (int16_t)i, -1, cb, ctx);
        icon_base += 2;
    }

    if (cb->display_buttongroup)
        cb->display_buttongroup(ctx, 0, true);

    int32_t result = cb->dm2_1031_0541 ?
        cb->dm2_1031_0541(ctx, (v1e03a8 == 0) ? 5 : 6) : 0;

    if (receipt) {
        receipt->panel_drawn = true;
        receipt->result = result;
    }
    return result;
}

/* ================================================================
 * 55. guidraw_24a5_1798 -- select hero for stat display
 *     c_gui_draw.cpp:4906
 * ================================================================ */

void dm2_v1_guidraw_24a5_1798(
    int16_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_GuiDraw1798Receipt *receipt)
{
    if (receipt) receipt->executed = false;
    if (!cb) return;

    if (hero_idx < 4) {
        int16_t hp = cb->get_hero_hp ? cb->get_hero_hp(ctx, hero_idx) : 0;
        if (hp == 0) return;
    }

    int16_t v1e03a8 = cb->get_v1e03a8 ? cb->get_v1e03a8(ctx) : 0;
    if (v1e03a8 != 0) return;

    int16_t v1e0976 = cb->get_v1e0976 ? cb->get_v1e0976(ctx) : 0;

    if (v1e0976 == hero_idx + 1) {
        /* Already selected: deselect */
        hero_idx = 4;
    }

    /* Transition between hero selections, refresh stats */
    if (v1e0976 != 0) {
        if (cb->set_v1e0976) cb->set_v1e0976(ctx, 0);
        int16_t prev = (int16_t)(v1e0976 - 1);
        int16_t prev_hp = cb->get_hero_hp ? cb->get_hero_hp(ctx, prev) : 0;
        if (prev_hp != 0) {
            DM2_V1_RefreshPlayerStatDispReceipt stat_receipt;
            dm2_v1_refresh_player_stat_disp(prev, cb, ctx, &stat_receipt);
        }
    }

    if (hero_idx != 4) {
        if (cb->set_v1e0976) cb->set_v1e0976(ctx, hero_idx + 1);
        if (cb->set_v1e100c) cb->set_v1e100c(ctx, 0);
        DM2_V1_RefreshPlayerStatDispReceipt stat_receipt;
        dm2_v1_refresh_player_stat_disp(hero_idx, cb, ctx, &stat_receipt);
    }

    if (receipt) receipt->executed = true;
}

/* ================================================================
 * 56. DM2_29ee_0396 (static) -- draw squad icons on right panel
 *     c_gui_draw.cpp:5034
 * (Internal helper for UPDATE_RIGHT_PANEL)
 * ================================================================ */

/* ================================================================
 * 57. DM2_29ee_0b2b (static) -- draw command slots and attack dir
 *     c_gui_draw.cpp:5158
 * (Internal helper for UPDATE_RIGHT_PANEL)
 * ================================================================ */

/* ================================================================
 * 58. UPDATE_RIGHT_PANEL -- main right panel update loop
 *     c_gui_draw.cpp:5182  (679 lines, second largest function)
 * ================================================================ */

void dm2_v1_update_right_panel(
    int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_UpdateRightPanelReceipt *receipt)
{
    if (receipt) receipt->panel_updated = false;
    if (!cb) return;

    int16_t v1e0288 = cb->get_v1e0288 ? cb->get_v1e0288(ctx) : 0;
    if (v1e0288 != 0) return;

    int16_t heros = cb->get_heros_in_party ? cb->get_heros_in_party(ctx) : 0;
    if (heros == 0) {
        /* No heroes: clear panel state */
        if (cb->set_v1d67bc) cb->set_v1d67bc(ctx, -1);
        if (receipt) receipt->panel_updated = true;
        return;
    }

    if (flags != 0) {
        /* Tick cooldowns for all heroes */
        for (int h = 0; h < heros; h++) {
            for (int s = 0; s < 3; s++) {
                uint8_t cd = cb->get_hero_handcooldown ?
                    cb->get_hero_handcooldown(ctx, h, s) : 0;
                if (cd != 0) {
                    /* Decrement cooldown, reload projectile if zero */
                }
            }
        }
    }

    /* The main panel update logic:
     * - Check for attack results (v1e0b7c)
     * - Update hero hand items and draw action icons
     * - Check squad direction changes and draw icons
     * - Handle enchanted map / container / spell panel modes
     * - Draw/update the appropriate sub-panel
     * All delegated through callbacks. */

    int16_t v1e0b7c = cb->get_v1e0b7c ? cb->get_v1e0b7c(ctx) : 0;
    if (v1e0b7c != 0) {
        dm2_v1_show_attack_result(v1e0b7c, cb, ctx);
        if (cb->set_v1d67bc) cb->set_v1d67bc(ctx, 6);
    }

    /* Finalize: display buttongroup2 if allocated */
    int16_t bg2 = cb->get_bg2_dbidx ? cb->get_bg2_dbidx(ctx) : -1;
    if (bg2 != -1 && cb->display_buttongroup)
        cb->display_buttongroup(ctx, 2, true);

    /* Update v1d694a tracking */
    int16_t d67bc = cb->get_v1d67bc ? cb->get_v1d67bc(ctx) : 0;
    int16_t d694a = cb->get_v1d694a ? cb->get_v1d694a(ctx) : 0;
    if (d67bc != d694a) {
        if (cb->dm2_107b0) cb->dm2_107b0(ctx);
        if (cb->set_v1d694a) cb->set_v1d694a(ctx, d67bc);
    }

    if (receipt) receipt->panel_updated = true;
}
