#ifndef FIRESTAFF_DM2_V1_GUI_DRAW_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_GUI_DRAW_PC34_COMPAT_H

/*
 * dm2_v1_gui_draw_pc34_compat.h -- DM2 GUI drawing module.
 *
 * Source: skproject c_gui_draw.cpp (60 functions).
 * All public and internal functions use callback-based architecture.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Forward types
 * ======================================================================== */

typedef struct DM2_V1_Rect {
    int16_t x, y, w, h;
} DM2_V1_Rect;

typedef struct DM2_V1_ImageDesc {
    int16_t x, y;
    int16_t width, height;
    int16_t mode;
} DM2_V1_ImageDesc;

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_DrawDialogueProgressReceipt {
    bool progress_drawn;
    int16_t bar_width;
} DM2_V1_DrawDialogueProgressReceipt;

typedef struct DM2_V1_DrawItemSurveyReceipt {
    bool survey_drawn;
    int32_t result;
} DM2_V1_DrawItemSurveyReceipt;

typedef struct DM2_V1_UpdateRightPanelReceipt {
    bool panel_updated;
} DM2_V1_UpdateRightPanelReceipt;

typedef struct DM2_V1_RefreshPlayerStatDispReceipt {
    bool stats_refreshed;
} DM2_V1_RefreshPlayerStatDispReceipt;

typedef struct DM2_V1_DrawMajicMapReceipt {
    bool map_drawn;
} DM2_V1_DrawMajicMapReceipt;

typedef struct DM2_V1_GuiDraw000fReceipt {
    bool panel_drawn;
    int32_t result;
} DM2_V1_GuiDraw000fReceipt;

typedef struct DM2_V1_GuiDraw1798Receipt {
    bool executed;
} DM2_V1_GuiDraw1798Receipt;

/* ========================================================================
 * Callback struct -- external dependencies for GUI drawing
 * ======================================================================== */

typedef struct DM2_V1_GuiDrawCallbacks {
    /* ---- GDAT queries ---- */
    void *(*query_gdat_image)(void *ctx, int8_t cls, int8_t sub, int8_t idx);
    void *(*query_gdat_palette)(void *ctx, int8_t cls, int8_t sub, int8_t idx);
    int16_t (*query_gdat_entry_data_index)(void *ctx, int32_t cls, int32_t sub,
                                           int32_t idx, int32_t field);
    int32_t (*query_gdat_dbspec_word)(void *ctx, int32_t record, int32_t field);
    void *(*query_gdat_text)(void *ctx, int8_t cls, int8_t sub, int8_t idx,
                             void *buf);
    bool (*query_gdat_entry_loadable)(void *ctx, int8_t cls, int8_t sub,
                                      int8_t idx, int32_t field);

    /* ---- Graphics / blitting ---- */
    void (*blit)(void *ctx, void *src, void *dst, DM2_V1_Rect *rect,
                 int16_t srcx, int16_t srcy, int16_t srcw, int16_t dstw,
                 int16_t alpha, int32_t mode, int32_t srcbpp, int32_t dstbpp,
                 void *palette);
    void (*stretch_4to8)(void *ctx, void *dst, DM2_V1_Rect *rect,
                         int8_t color, int16_t width);
    void (*fill_backbuff_rect)(void *ctx, DM2_V1_Rect *rect, int32_t pixel);
    void (*fill_rect_summary)(void *ctx, int32_t buttongroup_id,
                              DM2_V1_Rect *rect, int32_t pixel);
    void (*fill_entire_pict)(void *ctx, void *bmp, int32_t pixel);
    void (*draw_dialogue_pict)(void *ctx, void *srcbmp, void *dstbmp,
                                DM2_V1_Rect *rect, int16_t srcx, int16_t srcy,
                                int16_t alpha, void *palette);
    void (*draw_dialogue_parts_pict)(void *ctx, void *bmp, DM2_V1_Rect *rect,
                                      int16_t alpha, void *palette);
    void (*draw_static_pic)(void *ctx, int8_t cls, int8_t sub, int8_t idx,
                            int16_t rectid, int16_t alpha);
    void (*draw_strong_text)(void *ctx, void *bmpbits, int16_t width,
                             int16_t x, int16_t y, int16_t fgcolor,
                             int16_t bgcolor, void *text);
    void (*draw_vp_rc_str)(void *ctx, int32_t rectid, int16_t color,
                           void *text);
    void (*draw_local_text)(void *ctx, int32_t rectid, int32_t fgcolor,
                            int16_t bgcolor, void *text);
    void (*draw_name_str)(void *ctx, int32_t buttongroup_id, int32_t rectid,
                          int16_t fgcolor, int16_t bgcolor, void *text);
    void (*draw_button_str)(void *ctx, int32_t buttongroup_id, int32_t rectid,
                            int16_t fgcolor, int16_t bgcolor, void *text);
    void (*draw_guided_str)(void *ctx, void *text);
    void (*drawings_completed)(void *ctx);

    /* ---- Rect / layout queries ---- */
    DM2_V1_Rect *(*query_expanded_rect)(void *ctx, int16_t id,
                                         DM2_V1_Rect *out);
    DM2_V1_Rect *(*query_blit_rect)(void *ctx, void *bmp, DM2_V1_Rect *out,
                                     int16_t id, int16_t *srcx, int16_t *srcy);
    DM2_V1_Rect *(*scale_rect)(void *ctx, int16_t id, int16_t num,
                                int16_t den, DM2_V1_Rect *out);
    DM2_V1_Rect *(*offset_rect)(void *ctx, int32_t buttongroup_id,
                                 DM2_V1_Rect *out, DM2_V1_Rect *in);
    void (*adjust_buttongroup_rects)(void *ctx, int32_t buttongroup_id,
                                     DM2_V1_Rect *rect);
    void (*query_topleft)(void *ctx, int32_t rectid,
                          int16_t *x, int16_t *y);

    /* ---- Bitmap management ---- */
    void *(*get_bmp)(void *ctx, int16_t dbidx);
    int16_t (*get_bmp_width)(void *ctx, void *bmp);
    int16_t (*get_bmp_height)(void *ctx, void *bmp);
    int32_t (*get_bmp_bpp)(void *ctx, void *bmp);
    void *(*get_bmp_pixels)(void *ctx, void *bmp);
    void *(*get_screen_ptr)(void *ctx);
    void *(*get_backbuffer_ptr)(void *ctx);
    int16_t (*alloc_dbidx)(void *ctx);
    void (*alloc_new_bmp)(void *ctx, int16_t dbidx, int16_t w, int16_t h,
                          int32_t bpp);
    void *(*alloc_pict_buff)(void *ctx, int16_t w, int16_t h,
                             int32_t pool, int32_t bpp);
    void (*free_pict_buff)(void *ctx, void *bmp);

    /* ---- Palette ---- */
    uint8_t (*palette_to_ui8)(void *ctx, int32_t color_index);
    int32_t (*palette_to_pixel)(void *ctx, int32_t color_index);
    void (*ui8_to_palette_color)(void *ctx, void *dst, uint8_t val);
    void (*copy_small_palette)(void *ctx, void *dst, void *src);
    void (*convert_palette256)(void *ctx, void *palette, int8_t a,
                               int32_t b, int8_t c, void *colors);

    /* ---- Hero / party queries ---- */
    int16_t (*get_hero_hp)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_max_hp)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_stamina)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_max_stamina)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_mp)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_max_mp)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_food)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_water)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_poison)(void *ctx, int32_t hero_idx);
    bool (*get_hero_poisoned)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_damage)(void *ctx, int32_t hero_idx);
    uint8_t (*get_hero_partypos)(void *ctx, int32_t hero_idx);
    uint8_t (*get_hero_absdir)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_bodyflag)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_heroflag)(void *ctx, int32_t hero_idx);
    void (*set_hero_heroflag)(void *ctx, int32_t hero_idx, int16_t val);
    int16_t (*get_hero_item)(void *ctx, int32_t hero_idx, int32_t slot);
    uint8_t (*get_hero_nrunes)(void *ctx, int32_t hero_idx);
    void *(*get_hero_rune)(void *ctx, int32_t hero_idx);
    void *(*get_hero_name)(void *ctx, int32_t hero_idx);
    int16_t (*get_hero_ench_power)(void *ctx, int32_t hero_idx);
    uint8_t (*get_hero_ench_aura)(void *ctx, int32_t hero_idx);
    uint8_t (*get_hero_handcooldown)(void *ctx, int32_t hero_idx, int32_t slot);
    int16_t (*get_hero_timeridx)(void *ctx, int32_t hero_idx);
    int16_t (*get_cur_act_hero)(void *ctx);
    int16_t (*get_cur_act_mode)(void *ctx);
    int16_t (*get_heros_in_party)(void *ctx);
    int16_t (*get_event_heroidx)(void *ctx);

    /* ---- Item queries ---- */
    int8_t (*query_cls1)(void *ctx, int32_t record);
    int8_t (*query_cls2)(void *ctx, int32_t record);
    int16_t (*get_next_record_link)(void *ctx, int16_t record);
    void *(*get_address_of_record)(void *ctx, int16_t record);
    void *(*get_item_name)(void *ctx, int32_t record);
    int16_t (*query_item_weight)(void *ctx, int16_t record);
    bool (*is_container_chest)(void *ctx, int16_t record);
    bool (*is_container_moneybox)(void *ctx, int32_t record);
    bool (*is_container_map)(void *ctx, int32_t record);
    bool (*is_item_hand_activable)(void *ctx, int32_t hero, int32_t item,
                                   int32_t mode);
    bool (*is_item_fit_for_equip)(void *ctx, int16_t item, int32_t slot,
                                  int32_t mode);
    int16_t (*add_item_charge)(void *ctx, int32_t record, int32_t delta);
    int16_t (*get_max_charge)(void *ctx, int32_t record);
    int32_t (*get_item_order_in_container)(void *ctx, int32_t container,
                                           int32_t idx);
    int32_t (*get_itemtype_of_itemspec)(void *ctx, int32_t spec);
    int8_t (*query_itemdb_from_distinctive)(void *ctx, int32_t spec);
    void (*count_by_coin_types)(void *ctx, int32_t record, int16_t *out);
    void *(*query_cmdstr_name)(void *ctx, int8_t a, int8_t b, int8_t c);

    /* ---- UI state ---- */
    int16_t (*get_v1e0976)(void *ctx);
    void (*set_v1e0976)(void *ctx, int16_t val);
    int16_t (*get_v1e0258)(void *ctx);
    int16_t (*get_v1d2728)(void *ctx);
    int16_t (*get_v1d272a)(void *ctx);
    int16_t (*get_v1d272c)(void *ctx);
    int16_t (*get_v1d272e)(void *ctx);
    int16_t (*get_v1d271a)(void *ctx);
    int16_t (*get_v1d271c)(void *ctx);
    int16_t (*get_v1d274a)(void *ctx);
    int16_t (*get_v1d274c)(void *ctx);
    int16_t (*get_v1d274e)(void *ctx);
    int16_t (*get_v1d2750)(void *ctx);
    int16_t (*get_v1d2748)(void *ctx);
    int16_t (*get_v1d2722)(void *ctx);
    int16_t (*get_v1d2724)(void *ctx);
    int16_t (*get_v1d2726)(void *ctx);
    int16_t (*get_dialog1)(void *ctx);
    void (*set_dialog1)(void *ctx, int16_t val);
    bool (*get_v1e0200)(void *ctx);
    int16_t (*get_v1d66fc)(void *ctx);
    void (*set_v1d66fc)(void *ctx, int16_t val);
    int16_t (*get_v1d67bc)(void *ctx);
    void (*set_v1d67bc)(void *ctx, int16_t val);
    int16_t (*get_v1d694a)(void *ctx);
    void (*set_v1d694a)(void *ctx, int16_t val);
    int32_t (*get_gametick)(void *ctx);
    int16_t (*get_v1e0b62)(void *ctx);
    void (*set_v1e0b62)(void *ctx, int16_t val);
    int16_t (*get_v1e100c)(void *ctx);
    void (*set_v1e100c)(void *ctx, int16_t val);
    int16_t (*get_v1e0288)(void *ctx);
    uint8_t (*get_savegames_b02)(void *ctx);
    uint8_t (*get_savegames_b04)(void *ctx);
    int16_t (*get_v1e0b7c)(void *ctx);
    int16_t (*get_v1e0b78)(void *ctx);
    int16_t (*get_v1e0b72)(void *ctx);
    int16_t (*get_v1e0238)(void *ctx);
    int16_t (*get_v1e03a8)(void *ctx);
    int32_t (*get_backbuff2)(void *ctx);
    void (*set_backbuff2)(void *ctx, int32_t val);

    /* ---- Buttongroup management ---- */
    int16_t (*get_bg1_dbidx)(void *ctx);
    int16_t (*get_bg2_dbidx)(void *ctx);
    void (*setup_buttongroup)(void *ctx, int32_t bg_id, int16_t rectid,
                              int32_t flags);
    void (*display_buttongroup)(void *ctx, int32_t bg_id, bool flag);

    /* ---- Image system ---- */
    void (*query_gdat_summary_image)(void *ctx, void *image, int8_t cls,
                                     int8_t sub, int8_t idx);
    void (*query_picst_it)(void *ctx, void *image);
    void *(*set_image)(void *ctx, int16_t dbidx, void *imgdesc);
    void (*image_draw)(void *ctx, void *image, int32_t bg_id,
                       int16_t rectid, int16_t alpha);
    void (*image_cleanup)(void *ctx, void *imgdesc);
    void *(*query_pict_bits)(void *ctx, void *imgdesc);

    /* ---- Random ---- */
    int16_t (*rand16)(void *ctx, int16_t range);
    int16_t (*randdir)(void *ctx);

    /* ---- Misc ---- */
    void (*draw_gameload_dialogue_to_screen)(void *ctx);
    void (*queue_noise)(void *ctx, int32_t a, int32_t b, int32_t c,
                        int32_t d, int32_t e, int16_t f, int16_t g, int32_t h);
    void (*query_message_text)(void *ctx, void *buf, int32_t id, int32_t flags);
    void (*gfxstr_linebreak)(void *ctx, void *src, void *dst,
                             int16_t *pos, int32_t width);
    void (*gfxstr_draw_line)(void *ctx, int32_t x, int32_t y, void *text);
    void (*display_hint_text)(void *ctx, int32_t color, void *text);
    void (*hide_mouse)(void *ctx);
    void (*show_mouse)(void *ctx);
    void (*refresh_mouse)(void *ctx);
    void (*wait_screen_refresh)(void *ctx);
    void (*init_backbuff)(void *ctx);
    int32_t (*dm2_107b0)(void *ctx);
    int32_t (*dm2_1031_0541)(void *ctx, int32_t arg);
    void (*fill_halftone_recti)(void *ctx, int32_t id);
    void (*zero_memory)(void *ctx, void *ptr, int32_t size);
    int32_t (*put_object_into_container)(void *ctx, int32_t val);
    void (*check_room_for_container)(void *ctx, int32_t record, void *addr);
    void (*load_projectile_to_hand)(void *ctx, int32_t hero, int32_t slot);
    int32_t (*get_player_at_position)(void *ctx, int16_t pos);
    void *(*get_missile_ref_of_minion)(void *ctx, int32_t record,
                                       int32_t item);
    void (*query_4bpp_pict_buff)(void *ctx, int32_t cls, int32_t sub,
                                  void **pixels, void *palette);
    void (*summarize_stone_room)(void *ctx, void *out, int32_t heading,
                                 int32_t x, int32_t y);

    /* ---- String helpers ---- */
    void *(*fmt_num)(void *ctx, int16_t val, int32_t flags, int32_t width);
    void *(*ltoa10)(void *ctx, int32_t val, void *buf);
    int32_t (*dm2_strlen)(void *ctx, void *str);
    void (*dm2_strcpy)(void *ctx, void *dst, void *src);
    void (*dm2_strcat)(void *ctx, void *dst, void *src);
    int16_t (*dm2_max)(void *ctx, int16_t a, int16_t b);
    int16_t (*dm2_min)(void *ctx, int16_t a, int16_t b);
    int16_t (*dm2_abs)(void *ctx, int16_t a);

    /* ---- Data table queries ---- */
    uint8_t (*get_table_color)(void *ctx, int32_t hero_idx);
    int16_t (*get_strdat_strxplus)(void *ctx);
    int16_t (*get_strdat_gfxstrw1)(void *ctx);
    int16_t (*get_strdat_gfxstrw3)(void *ctx);
} DM2_V1_GuiDrawCallbacks;

/* ========================================================================
 * Data tables from c_gui_draw.cpp
 * ======================================================================== */

/* table1d275a: 32 entries of (x_offset, y_offset) for coin scatter */
extern const int8_t dm2_guidraw_table1d275a[32][2];

/* table1d69d0: hero color indices */
extern const uint8_t dm2_guidraw_table1d69d0[4];

/* table1d67d9: right panel mode flags */
extern const uint8_t dm2_guidraw_table1d67d9[8];

/* v1d1124: slash separator string */
extern const char dm2_guidraw_v1d1124[2];

/* v1d10f0: empty string for item survey */
extern const char dm2_guidraw_v1d10f0[2];

/* ========================================================================
 * Public functions (callback pattern)
 * ======================================================================== */

void dm2_v1_draw_icon_pict_buff(
    void *srcbmp, int32_t bg_id, DM2_V1_Rect *blitrect,
    int16_t srcx, int16_t srcy, int16_t alphamask, int32_t blitmode,
    void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_icon_pict_entry(
    int8_t cls, int8_t sub, int8_t idx, int32_t bg_id,
    int16_t rectid, int16_t alphamask,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_dialogue_progress(
    int32_t progress,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_DrawDialogueProgressReceipt *receipt);

void dm2_v1_draw_dialogue_parts_pict(
    void *bmp, DM2_V1_Rect *rect, int16_t alpha, void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_dialogue_pict(
    void *srcbmp, void *destbmp, DM2_V1_Rect *rect,
    int16_t srcx, int16_t srcy, int16_t alpha, void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_wake_up_text(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_player_3stat_health_bar(
    int32_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_cur_max_hms(
    int32_t rectid, int32_t cur, int32_t max,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_player_3stat_text(
    int32_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_player_name_at_cmdslot(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_player_damage(
    int32_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_chip_of_magic_map(
    void *bmp, int32_t mul, int32_t x, int32_t y, int32_t flags,
    void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_query_gdat_squad_icon(
    void *bmp, int32_t hero_idx, void *palette,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_cryocell_lever(
    int32_t state,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_charsheet_option_icon(
    int32_t icon_idx, int32_t rectid, int32_t mask,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_money_box_survey(
    int32_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_moneybox(
    int16_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_player_3stat_pane(
    int32_t hero_idx, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_cmd_slot(
    int16_t slot_idx, int8_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_spell_to_be_cast(
    int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_player_attack_dir(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_spell_panel(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_show_attack_result(
    int16_t result_code,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_squad_spell_and_leader_icon(
    int16_t hero_idx, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_food_water_poison_panel(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_item_stats_bar(
    int32_t rectid, int32_t value, int32_t max_value,
    int32_t label_char, int16_t color_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_item_in_hand(
    void *item_record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_container_panel(
    int32_t record, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_item_icon(
    int32_t record, int32_t slot, int32_t body_flag,
    int32_t active_flag, int32_t redraw_flag,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_container_survey(
    void *record_addr,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_eye_mouth_colored_rectangle(
    int8_t icon_idx, int16_t rectid,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_scroll_text(
    int32_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

int32_t dm2_v1_draw_item_survey(
    int32_t record, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_DrawItemSurveyReceipt *receipt);

void dm2_v1_draw_hand_action_icons(
    int16_t hero_idx, int32_t hand, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_map_chip(
    int32_t map_x, int32_t map_y, int32_t adj_x, int32_t adj_y,
    int16_t chip_x, int16_t chip_y, int8_t style, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_draw_majic_map(
    int32_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_DrawMajicMapReceipt *receipt);

void dm2_v1_display_hint_new_line(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_display_taken_item_name(
    int16_t record,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_highlight_arrow_panel(
    int32_t arrow_idx, int32_t rectid, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_display_right_panel_squad_hands(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_refresh_player_stat_disp(
    int16_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_RefreshPlayerStatDispReceipt *receipt);

int32_t dm2_v1_guidraw_29ee_000f(
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_GuiDraw000fReceipt *receipt);

void dm2_v1_guidraw_24a5_1798(
    int16_t hero_idx,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_GuiDraw1798Receipt *receipt);

void dm2_v1_update_right_panel(
    int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx,
    DM2_V1_UpdateRightPanelReceipt *receipt);

/* ========================================================================
 * Internal functions (all take callbacks)
 * ======================================================================== */

void dm2_v1_guidraw_0b36_0c52(
    int32_t bg_id, int16_t rectid, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_guidraw_29ee_00a3(
    int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_guidraw_0b36_105b(
    int32_t bg_id, DM2_V1_Rect *rect, int16_t color,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_guidraw_24a5_0e82(
    int32_t value, int32_t rectid, int32_t color_idx,
    int32_t min_val, int16_t max_val, int32_t extra_color,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

int8_t dm2_v1_guidraw_2405_014a(
    int32_t record, int32_t slot, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_guidraw_2405_011f(
    int32_t rectid, DM2_V1_Rect *out,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_guidraw_2405_00ec(
    int32_t rectid, DM2_V1_Rect *out,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

int32_t dm2_v1_guidraw_2e62_03b5(
    int32_t hero_idx, int32_t hand, int32_t redraw,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_guidraw_29ee_1d03(
    int8_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

void dm2_v1_guidraw_29ee_1946(
    int32_t map_x, int32_t map_y, int32_t adj_x, int32_t adj_y,
    int16_t chip_x, int32_t heading, int32_t side, int32_t flags,
    const DM2_V1_GuiDrawCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GUI_DRAW_PC34_COMPAT_H */
