/*
 * dm2_v1_querydb_pc34_compat.h
 *
 * DM2 database query module — ported from skproject c_querydb.cpp.
 * 82 functions covering GDAT lookups, tile/map queries, door properties,
 * creature state, player/hero stats, item/container checks, combat damage,
 * palette computation, and miscellaneous utility queries.
 *
 * Reference: skproject c_querydb.cpp
 * Architecture: Firestaff callback-based (single shared callback struct)
 *
 * 5 pure-computation functions (no callbacks):
 *   dm2_v1_query_098d_000f, dm2_v1_is_cls1_critical_for_load,
 *   dm2_v1_is_tile_blocked, dm2_v1_dir_from_5x5_pos,
 *   dm2_v1_query_creature_blit_recti
 *
 * 77 callback-dependent functions grouped by category.
 */

#ifndef FIRESTAFF_DM2_V1_QUERYDB_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_QUERYDB_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Callback struct                                                            */
/* -------------------------------------------------------------------------- */

typedef struct DM2_V1_QueryDbCallbacks {

    /* GDAT database */
    int16_t (*query_gdat_entry_data_index)(void *ctx, int8_t a, int8_t b, int8_t c, int8_t d);
    bool    (*query_gdat_entry_if_loadable)(void *ctx, int8_t a, int8_t b, int8_t c, int8_t d);
    int32_t (*query_gdat_raw_data_length)(void *ctx, int16_t dbidx);
    void   *(*query_gdat_entry_data_ptr)(void *ctx, int8_t a, int8_t b, int8_t c, int8_t d);
    void   *(*query_gdat_image_entry_buff)(void *ctx, int8_t a, int8_t b, int8_t c);
    char   *(*query_gdat_text)(void *ctx, int8_t a, int8_t b, int8_t c, char *buf);

    /* Record access */
    void    *(*get_address_of_record)(void *ctx, uint16_t record);
    int32_t  (*get_next_record_link)(void *ctx, uint16_t record);
    int16_t  (*get_tile_record_link)(void *ctx, int16_t x, int16_t y);
    uint8_t  (*query_cls1_from_record)(void *ctx, int32_t record);
    uint8_t  (*query_cls2_from_record)(void *ctx, int32_t record);
    int16_t  (*get_distinctive_itemtype)(void *ctx, int16_t record);

    /* Item */
    int16_t (*query_item_weight)(void *ctx, int16_t record);
    int16_t (*add_item_charge)(void *ctx, int16_t record, int32_t amount);
    int16_t (*get_max_charge)(void *ctx, int16_t item_cls);
    int32_t (*query_gdat_dbspec_word_value)(void *ctx, int32_t record, int32_t idx);

    /* Creature */
    void    *(*query_creature_ai_spec_from_type)(void *ctx, int32_t type);
    void    *(*query_creature_ai_spec_from_record)(void *ctx, int32_t type_byte);
    int16_t  (*query_creature_ai_spec_flags)(void *ctx, int32_t type);
    uint8_t  (*get_wall_decoration_of_actuator)(void *ctx, void *actuator);
    uint8_t  (*get_floor_decoration_of_actuator)(void *ctx, void *actuator);

    /* Map/tile */
    int32_t (*get_tile_value)(void *ctx, int32_t x, int32_t y);
    void    (*change_current_map_to)(void *ctx, int32_t level);
    int32_t (*get_wall_tile_anyitem_record)(void *ctx, int32_t x, int16_t y);

    /* Party/hero */
    int16_t (*get_hero_item)(void *ctx, int hero, int slot);
    int16_t (*get_hero_curHP)(void *ctx, int hero);
    uint8_t (*get_hero_partypos)(void *ctx, int hero);
    int16_t (*get_heros_in_party)(void *ctx);
    int16_t (*get_hero_skill)(void *ctx, int hero, int skill_major, int skill_minor);
    int16_t (*get_hero_sbonus)(void *ctx, int hero, int major, int minor);
    int16_t (*get_hero_max_load)(void *ctx, int hero);
    int16_t (*get_player_weight)(void *ctx, int hero);

    /* Misc */
    int32_t (*dm2_rand)(void *ctx);
    int16_t (*rotate_5x5_pos)(void *ctx, int16_t pos, uint16_t rotate);
    void    (*format_skstr)(void *ctx, const char *src, char *dst);
    int32_t (*dm_locate_other_level)(void *ctx, int32_t level, int32_t dir,
                                     int16_t *xp, int16_t *yp, void *extra);
    int32_t (*is_creature_allowed_on_level_cb)(void *ctx, int32_t creature, int32_t level);

} DM2_V1_QueryDbCallbacks;

/* -------------------------------------------------------------------------- */
/* Receipt struct                                                             */
/* -------------------------------------------------------------------------- */

typedef struct DM2_V1_QueryDbReceipt {
    int32_t functions_wired;    /* number of callback-dependent functions exercised */
    int32_t pure_functions;     /* number of pure-computation functions exercised   */
    int32_t gdat_queries;       /* GDAT entry lookups performed                     */
    int32_t tile_queries;       /* tile/map queries performed                       */
    int32_t creature_queries;   /* creature state queries performed                 */
    int32_t item_queries;       /* item/container queries performed                 */
    int32_t player_queries;     /* player/hero queries performed                    */
    int32_t door_queries;       /* door property queries performed                  */
    int32_t combat_queries;     /* combat/damage computations performed             */
    int32_t palette_queries;    /* palette/rendering queries performed              */
    bool    all_callbacks_set;  /* true if all required callback pointers are set   */
} DM2_V1_QueryDbReceipt;

/* -------------------------------------------------------------------------- */
/* Pure computation — no callbacks needed                                     */
/* -------------------------------------------------------------------------- */

/* 5x5 position calculation */
void dm2_v1_query_098d_000f(int16_t x, int16_t y, int16_t pos,
                            int16_t *out_w1, int16_t *out_w2);

/* Direction from 5x5 position */
int32_t dm2_v1_dir_from_5x5_pos(int32_t pos);

/* Creature blit rect index */
int16_t dm2_v1_query_creature_blit_recti(int16_t n, int16_t rotate, int16_t wb);

/* Check if class1 is critical for load (0x1b, 0x6, 0x5) */
bool dm2_v1_is_cls1_critical_for_load(int8_t cls1);

/* Check if tile blocks movement */
int32_t dm2_v1_is_tile_blocked(int32_t tile_value);

/* -------------------------------------------------------------------------- */
/* GDAT / graphics database queries (13)                                      */
/* -------------------------------------------------------------------------- */

int16_t dm2_v1_query_gdat_entry_data_index(int8_t a, int8_t b, int8_t c, int8_t d,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

bool dm2_v1_query_gdat_entry_if_loadable(int8_t a, int8_t b, int8_t c, int8_t d,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_gdat_entry_data_length(int8_t a, int8_t b, int8_t c, int8_t d,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int16_t dm2_v1_query_gdat_pict_offset(int8_t a, int8_t b, int8_t c,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* Source QUERY_GDAT_TEXT writes decoded/FORMAT_SKSTR-expanded GDAT text to
 * buf. Returns one if the source callback found an entry, otherwise zero. */
int32_t dm2_v1_query_gdat_text(int32_t cls1, int32_t cls2, int32_t field,
    char *buf, int32_t buf_len,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_gdat_image_metrics(int8_t a, int8_t b, int8_t c,
    int16_t *w, int16_t *h,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_ornate_anim_frame(int32_t a, int32_t b, int32_t c, int32_t d,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_gdat_creature_word_value(int32_t creature_type, int32_t value_idx,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_gdat_food_value_from_record(int16_t record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

const char *dm2_v1_query_gdat_item_name(int32_t a, int32_t b,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_cmdstr_text(const char *text, const char *pattern,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

const char *dm2_v1_query_cmdstr_name(int8_t a, int8_t b, int8_t c,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_cmdstr_entry(int32_t a, int32_t b, int32_t c, int32_t d,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_cur_cmdstr_entry(int32_t a,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* GDAT picture/texture queries (3)                                           */
/* -------------------------------------------------------------------------- */

void dm2_v1_query_4bpp_pict_buff_and_pal(int8_t a, int8_t b, int8_t c,
    void *buff_out, void *pal_out,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_creatures_item_mask(int32_t creature_type, int32_t mask_idx,
    uint8_t *mask_out, int32_t flag,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_message_text(char *out, int32_t record, int32_t flags,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_temp_picst(int8_t a, int8_t b, int8_t c,
    void *picst_out,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Tile / map queries (7)                                                     */
/* -------------------------------------------------------------------------- */

int32_t dm2_v1_get_glob_var(int32_t var_idx,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_get_creature_at(int32_t x, int32_t y,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_find_distinctive_item_on_tile(int32_t x, int32_t y,
    int32_t subcell, int32_t itemtype,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_find_tile_actuator(int32_t x, int32_t y,
    int32_t type, int32_t subtype,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_find_laddar_around(int32_t x, int32_t y, int32_t dir, void **out,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_0cee_0897(int16_t **out, int32_t x, int32_t y,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_0cee_06dc(int32_t x, int32_t y,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Door queries (5)                                                           */
/* -------------------------------------------------------------------------- */

int32_t dm2_v1_query_door_damage_resist(int32_t door_type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_get_door_stat_0x10(int32_t door_type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_get_graphics_for_door(int32_t door_type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_0cee_3275(int32_t door_type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_door_strength(int32_t door_type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Wall ornament queries (3)                                                  */
/* -------------------------------------------------------------------------- */

int32_t dm2_v1_is_wall_ornate_alcove(int32_t ornament,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_wall_ornate_spring(int32_t record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_get_ornate_anim_len(void *actuator, int32_t is_wall,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Item / container queries (10)                                              */
/* -------------------------------------------------------------------------- */

int32_t dm2_v1_is_miscitem_currency(int32_t record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_container_moneybox(int32_t record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_container_map(int32_t record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_container_chest(int16_t record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_distinctive_item_on_actuator(void *actuator,
    int32_t itemtype, int32_t flag,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_missile_valid_to_launcher(int32_t hero, int32_t slot, int32_t missile,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_item_fit_for_equip(int16_t item, int32_t slot, int32_t mode,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_get_item_order_in_container(int32_t container, int32_t item_record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_count_by_coin_types(int32_t container, int16_t *coin_counts,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_item_hand_activable(int32_t hero, int32_t item, int32_t hand,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Creature queries (13)                                                      */
/* -------------------------------------------------------------------------- */

int32_t dm2_v1_get_creature_weight(int32_t creature_type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_rebirth_altar(void *record_ptr,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_1c9a_08bd(void *creature_ptr,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_creature_floating(int32_t record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_object_floating(int32_t record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_object_5x5_pos(int32_t record, int32_t direction,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_creature_5x5_pos(void *creature, int32_t direction,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_creature_picst(int32_t a, int32_t b, void *c, void *d, int16_t e,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_creature_movable_there(int32_t x, int32_t y, int32_t dir, int16_t *out,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_is_creature_allowed_on_level(int32_t creature, int32_t level,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_0cee_2e09(int32_t creature_type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_0cee_2df4(int32_t creature_type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_0cee_2e35(int32_t creature_type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Player / hero queries (8)                                                  */
/* -------------------------------------------------------------------------- */

int16_t dm2_v1_query_player_skill_lv(int16_t hero, int16_t skill, int32_t include_bonus,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_querydb_get_player_at_position(int32_t pos,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_find_hand_with_empty_flask(int hero,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_find_pouch_or_scabbard_possession_pos(int32_t hero, int32_t type,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_calc_player_walk_delay(int32_t hero,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_querydb_compute_player_attack_or_throw_strength(int32_t player_idx,
    int32_t hand,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_2759_0155(int32_t item,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_2759_01fe(int32_t cmd, int32_t item,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Teleporter / level queries (3)                                             */
/* -------------------------------------------------------------------------- */

void dm2_v1_get_teleporter_detail(void *out, int32_t x, int32_t y,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_19f0_124b(int16_t *xp, int16_t *yp,
    int32_t level, int32_t dir, int32_t flags,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_29ee_18eb(int32_t x, int32_t y, int32_t level,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Environment / weather (2)                                                  */
/* -------------------------------------------------------------------------- */

void dm2_v1_query_rainfall_param(int8_t *out_a, int16_t *out_d,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_retrieve_environment_cmd_cd_fw(void *env_ptr,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Actuator / ornament scan (3)                                               */
/* -------------------------------------------------------------------------- */

void dm2_v1_query_0cee_1a46(int16_t *arr8, int32_t record,
    int32_t dir, int32_t flag,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_48ae_011a(int32_t record,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_48ae_01af(int32_t tile_byte, int32_t dir,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Combat / damage (2)                                                        */
/* -------------------------------------------------------------------------- */

int32_t dm2_v1_query_48ae_05ae(int32_t itemspec, int32_t slot,
    int32_t attack_type, int32_t creature_type,
    int32_t flag, int32_t charges,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_48ae_0767(int32_t gold, int32_t limit,
    int16_t *types, void *counts,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Palette / rendering (3)                                                    */
/* -------------------------------------------------------------------------- */

void dm2_v1_query_32cb_0804(int8_t a, int8_t b, int8_t c,
    void *pal_out,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_0b36_037e(int8_t a, int8_t b, int8_t c,
    void *pal_out,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_B073(int8_t a, int8_t b, int8_t c,
    void *pal_out,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Inventory search (2)                                                       */
/* -------------------------------------------------------------------------- */

int32_t dm2_v1_query_2fcf_164e(int32_t container, int32_t itemtype,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

int32_t dm2_v1_query_2fcf_16ff(int32_t itemtype,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

/* -------------------------------------------------------------------------- */
/* Misc (4)                                                                   */
/* -------------------------------------------------------------------------- */

int32_t dm2_v1_query_4E26(int16_t *timer_word,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void *dm2_v1_query_1c9a_02c3(void *creature, void *aispec,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_4DA3(int32_t type, int32_t state, int16_t *p, void *out,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

void dm2_v1_query_1c9a_03cf(int16_t *xp, int16_t *yp, int32_t dir,
    const DM2_V1_QueryDbCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_QUERYDB_PC34_COMPAT_H */
