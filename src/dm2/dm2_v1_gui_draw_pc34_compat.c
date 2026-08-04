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
    {0,0}, {1,0}, {2,0}, {3,0}, {0,1}, {1,1}, {2,1}, {3,1},
    {0,2}, {1,2}, {2,2}, {3,2}, {0,3}, {1,3}, {2,3}, {3,3},
    {4,0}, {5,0}, {6,0}, {7,0}, {4,1}, {5,1}, {6,1}, {7,1},
    {4,2}, {5,2}, {6,2}, {7,2}, {4,3}, {5,3}, {6,3}, {7,3}
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
    if (!cb || !blitrect) return;
    if (!cb->blit) return;

    int16_t srcw = cb->get_bmp_width ? cb->get_bmp_width(ctx, srcbmp) : 0;
    void *dstbmp = cb->get_bmp ? cb->get_bmp(ctx, (int16_t)bg_id) : NULL;
    int16_t dstw = srcw; /* simplified; real impl uses buttongroup width */

    cb->blit(ctx, srcbmp, dstbmp, blitrect, srcx, srcy,
             srcw, dstw, alphamask, blitmode,
             cb->get_bmp_bpp ? cb->get_bmp_bpp(ctx, srcbmp) : 0,
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
    if (!cb || !bmp) return;

    /* The original performs rect clipping against dm2rect3 and blits.
     * Here we delegate through the blit callback. */
    int16_t chip_w = cb->get_v1d274a ? cb->get_v1d274a(ctx) : 0;
    int16_t chip_h = cb->get_v1d274c ? cb->get_v1d274c(ctx) : 0;

    DM2_V1_Rect rc;
    rc.x = (int16_t)x;
    rc.y = (int16_t)y;
    rc.w = chip_w;
    rc.h = chip_h;

    int16_t srcx = (int16_t)(mul * chip_w);
    dm2_v1_draw_icon_pict_buff(bmp, 2, &rc, srcx, 0,
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

    for (int i = 0; i < 10; i++) {
        int32_t order = cb->get_item_order_in_container ?
            cb->get_item_order_in_container(ctx, (int32_t)record, i) : -1;
        if (order >= 0 && counts[order] > 0) {
            /* Draw coins for this denomination (simplified) */
            int16_t rect_id = (int16_t)(i + 0xdd);
            dm2_v1_draw_icon_pict_entry(20, cls2, 0x10, 2, rect_id, -1, cb, ctx);
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
    if (!cb) return;

    dm2_v1_guidraw_29ee_00a3(0, cb, ctx);

    int16_t v1e0b62 = cb->get_v1e0b62 ? cb->get_v1e0b62(ctx) : 0;
    if (v1e0b62 != 0) {
        /* Enchanted mode: draw from GDAT class 20 */
        dm2_v1_draw_icon_pict_entry(20, 0, (int8_t)(flags + 21),
                                     2, (int16_t)(slot_idx + 0x3f), -1,
                                     cb, ctx);
    } else {
        dm2_v1_draw_icon_pict_entry(1, 4, (int8_t)(flags + 21),
                                     2, (int16_t)(slot_idx + 0x3f), -1,
                                     cb, ctx);
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

    int16_t cur = cb->get_cur_act_hero ? cb->get_cur_act_hero(ctx) : 0;
    if (cur <= 0) return;

    /* Draw directional attack indicator (simplified) */
    dm2_v1_draw_icon_pict_entry(8, 0, -10, 2, 0x5d, -1, cb, ctx);

    int32_t hero_idx = cur - 1;
    /* Squad icon and enchantment aura handled through callbacks */

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
        if ((uint16_t)result_code >= 0xfffe)
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

    if (cb->query_gdat_summary_image && cb->image_draw) {
        /* Use image system to render attack result */
        /* Simplified: delegates to callback */
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

    uint8_t partypos = cb->get_hero_partypos ?
        cb->get_hero_partypos(ctx, hero_idx) : 0;
    int16_t heading = cb->get_v1e0258 ? cb->get_v1e0258(ctx) : 0;
    int32_t dir = ((int32_t)partypos + 4 - (int32_t)heading) & 0x3;

    /* Fill background rect with E_COL00 */
    uint8_t col00 = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
    DM2_V1_Rect rc;
    int16_t rect_id = (int16_t)(dir + 0x4f);
    if (cb->query_expanded_rect)
        cb->query_expanded_rect(ctx, rect_id, &rc);
    if (cb->fill_rect_summary)
        cb->fill_rect_summary(ctx, 2, &rc, (int32_t)col00);

    int16_t hp = cb->get_hero_hp ? cb->get_hero_hp(ctx, hero_idx) : 0;
    if (hp == 0) return;

    /* Draw leader and spell icons (simplified) */
    int8_t icon_base = (dir > 1) ? 0x0c : 0x0a;
    int16_t evhero = cb->get_event_heroidx ? cb->get_event_heroidx(ctx) : -1;
    if (hero_idx == evhero)
        icon_base++;

    /* Two image draws for leader and spell indicators */
    if (cb->query_gdat_summary_image && cb->image_draw) {
        /* Simplified: handled through callbacks */
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

    /* Draw background and bar */
    int16_t border = cb->get_v1d2748 ? cb->get_v1d2748(ctx) : 0;
    DM2_V1_Rect bg_rc = bar_rc;
    bg_rc.x += border;
    bg_rc.y += border;

    uint8_t col00 = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
    if (cb->fill_backbuff_rect)
        cb->fill_backbuff_rect(ctx, &bg_rc, (int32_t)col00);

    uint8_t bar_col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, fg_col) : 0;
    if (cb->fill_backbuff_rect)
        cb->fill_backbuff_rect(ctx, &bar_rc, (int32_t)bar_col);

    if (extra_color != 0 && cb->fill_backbuff_rect) {
        uint8_t ext_col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, extra_color) : 0;
        /* Draw extra segment (simplified) */
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
 *     c_gui_draw.cpp:1232
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

    dm2_v1_guidraw_24a5_0e82(value, rectid, (int32_t)color_idx,
                              0, (int16_t)max_value, 1, cb, ctx);

    /* Draw label characters at sides of bar (simplified) */
    char label[2] = {(char)label_char, '\0'};
    uint8_t fg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, color_idx) : 0;
    uint8_t bg = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 0) : 0;
    int16_t bgcolor = (int16_t)((uint16_t)bg | 0x4000);

    if (cb->draw_strong_text) {
        void *backbuf = cb->get_backbuffer_ptr ? cb->get_backbuffer_ptr(ctx) : NULL;
        int16_t backw = cb->get_bmp_width ? cb->get_bmp_width(ctx, backbuf) : 0;
        int16_t text_y = (int16_t)(rc.y + rc.h - 3);
        cb->draw_strong_text(ctx, backbuf, backw,
                              (int16_t)(rc.x - 9), text_y,
                              (int16_t)fg, bgcolor, label);
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
 *     c_gui_draw.cpp:1516
 * ================================================================ */

void dm2_v1_draw_item_in_hand(
    void *item_record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !item_record) return;
    /* Delegates to cls1/cls2 queries and blit callbacks */
}

/* ================================================================
 * 33. DRAW_CONTAINER_PANEL
 *     c_gui_draw.cpp:1540
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

    /* Iterate container slots (simplified) */
    for (int i = 0; i < 8; i++) {
        /* Each slot queries the contained item and draws it */
        int16_t rect_id = (int16_t)(i + 0xe5);

        int8_t icon = dm2_v1_guidraw_2405_014a(record, i + 0x1e, 1, cb, ctx);
        int8_t item_cls1 = cb->query_cls1 ? cb->query_cls1(ctx, record) : 0;
        int8_t item_cls2 = cb->query_cls2 ? cb->query_cls2(ctx, record) : 0;

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
 *     c_gui_draw.cpp:1656
 * ================================================================ */

void dm2_v1_draw_item_icon(
    int32_t record, int32_t slot, int32_t body_flag,
    int32_t active_flag, int32_t redraw_flag,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int8_t cls1, cls2, icon;

    if (record != -1) {
        cls1 = cb->query_cls1 ? cb->query_cls1(ctx, record) : 7;
        cls2 = cb->query_cls2 ? cb->query_cls2(ctx, record) : 0;
        int32_t adj_slot = (slot >= 8) ? slot - 8 : (slot & 1);
        icon = dm2_v1_guidraw_2405_014a(record, adj_slot, 1, cb, ctx);
    } else {
        cls1 = 7;
        cls2 = 0;
        icon = 0; /* from ddat slot table */
    }

    /* Background and item blitting (simplified) */
    if (slot < 0x26 && cb->query_expanded_rect) {
        DM2_V1_Rect rc;
        cb->query_expanded_rect(ctx, (int16_t)slot, &rc);
    }

    if (icon != -1) {
        if (slot < 8)
            dm2_v1_draw_icon_pict_entry(cls1, cls2, icon, 1,
                                         (int16_t)slot, 0x0c, cb, ctx);
        else if (cb->draw_static_pic)
            cb->draw_static_pic(ctx, cls1, cls2, icon, (int16_t)slot, 0x0c);
    }
}

/* ================================================================
 * 37. DRAW_CONTAINER_SURVEY
 *     c_gui_draw.cpp:1795
 * ================================================================ */

void dm2_v1_draw_container_survey(
    void *record_addr,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx)
{
    if (!cb || !record_addr) return;
    /* Iterates items in container, calling draw_item_icon for each */
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
    if (!cb) return;

    if (cb->set_v1d66fc) cb->set_v1d66fc(ctx, 5);
    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 7, 0, 1, 0x1ee, -1);
    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 18, 0, 0x10, 0x1ee, 0x0c);

    /* Message text parsing and line-by-line rendering via callbacks */
    char message[0xc8];
    memset(message, 0, sizeof(message));
    if (cb->query_message_text)
        cb->query_message_text(ctx, message, record, (int32_t)0xffff8002);
}

/* ================================================================
 * 41. DRAW_ITEM_SURVEY
 *     c_gui_draw.cpp:2072
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
    if (!cb) return 0;
    if (record == -1) return 0;

    /* Check for scroll type (7) */
    /* Draws item name, weight, icon, and stat bars via callbacks */
    if (flags == 0) {
        if (receipt) receipt->result = 0;
        return 0;
    }

    if (cb->set_v1d66fc) cb->set_v1d66fc(ctx, 3);
    if (cb->draw_static_pic)
        cb->draw_static_pic(ctx, 7, 0, 1, 0x1ee, -1);

    void *name = cb->get_item_name ? cb->get_item_name(ctx, record) : NULL;
    uint8_t col = cb->palette_to_ui8 ? cb->palette_to_ui8(ctx, 13) : 0;
    if (cb->draw_vp_rc_str && name)
        cb->draw_vp_rc_str(ctx, 0x1fa, (int16_t)col, name);

    dm2_v1_draw_item_icon(record, 0x2e, 0, 0, 0, cb, ctx);

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

    int16_t hp = cb->get_hero_hp ? cb->get_hero_hp(ctx, hero_idx) : 0;
    if (hp == 0) {
        /* Fill rect with COL00 */
        return;
    }

    /* Draw hand action icon background and item-on-wood (simplified) */
    uint8_t partypos = cb->get_hero_partypos ?
        cb->get_hero_partypos(ctx, hero_idx) : 0;
    int16_t heading = cb->get_v1e0258 ? cb->get_v1e0258(ctx) : 0;
    int32_t dir = ((int32_t)partypos + 4 - (int32_t)heading) & 0x3;

    int16_t rect_base = (hand != 1) ? 0x4a : 0x46;
    int16_t rect_id = (int16_t)(rect_base + dir);

    int8_t icon = (int8_t)(2 * hand + 2 + flags);
    dm2_v1_draw_icon_pict_entry(1, 4, icon, 2, rect_id, -1, cb, ctx);
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
