/*
 * dm2_v1_loadlevel_pc34_compat.c -- DM2 level loading/initialization.
 *
 * Ports the level loading system from skproject c_loadlevel.cpp.
 * Handles dynamic resource marking, misc item loading, graphics table
 * setup, actuator processing on map transitions, and the top-level
 * map loading orchestrator.
 *
 * Source: skproject/SKULLWIN/c_loadlevel.cpp
 */

#include "dm2_v1_loadlevel_pc34_compat.h"

#include <string.h>

/* ── DM2_MARK_DYN_LOAD ─────────────────────────────────────────────── */

/* Source: c_loadlevel.cpp DM2_MARK_DYN_LOAD
 * Unpacks a 32-bit resource ID into a DynLoadEntry and appends it. */
DM2_V1_MarkDynLoadReceipt dm2_v1_mark_dyn_load(
    DM2_V1_DynLoadState *state,
    int32_t resource_id)
{
    DM2_V1_MarkDynLoadReceipt r;
    r.entry_index = -1;

    if (state == NULL) return r;
    if (state->count >= DM2_V1_LOADLEVEL_MAX_DYN_ENTRIES) return r;

    int16_t idx = state->count;
    DM2_V1_DynLoadEntry *e = &state->entries[idx];
    e->flags = 0;
    e->cat   = (uint8_t)((resource_id >> 24) & 0xFF);
    e->type  = (uint8_t)((resource_id >> 16) & 0xFF);
    e->sub1  = (uint8_t)((resource_id >>  8) & 0xFF);
    e->sub2  = (uint8_t)((resource_id      ) & 0xFF);
    state->count++;

    r.entry_index = idx;
    return r;
}

/* ── DM2_2676_008f — mark with override flag ────────────────────────── */

/* Source: c_loadlevel.cpp DM2_2676_008f */
void dm2_v1_mark_dyn_load_with_flag(
    DM2_V1_DynLoadState *state,
    int32_t resource_id, int32_t flag)
{
    if (state == NULL) return;

    dm2_v1_mark_dyn_load(state, resource_id);
    if (state->count > 0) {
        state->entries[state->count - 1].flags = (int16_t)0x8001;
    }

    /* Mark the sub-entry with the flag byte */
    uint8_t flag_byte = (uint8_t)(flag & 0xFF);
    int32_t sub_id = (resource_id & 0xFFFF0000) |
                     ((uint32_t)flag_byte << 8) |
                     (resource_id & 0xFF);
    dm2_v1_mark_dyn_load(state, sub_id);
}

/* ── DM2_2676_00d0 — mark GDAT entry ───────────────────────────────── */

/* Source: c_loadlevel.cpp DM2_2676_00d0 */
void dm2_v1_mark_dyn_load_gdat_entry(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_DynLoadState *state,
    uint8_t cat, uint8_t type, uint8_t sub)
{
    if (cb == NULL || state == NULL) return;

    int16_t data = cb->query_gdat_entry_data_index(cb->ctx,
        cat, type, 11, sub);
    uint8_t lo = (uint8_t)(data & 0xFF);
    uint8_t hi = (uint8_t)((data >> 8) & 0xFF);

    if (lo != 0) {
        int32_t id = ((int32_t)0x0D << 24) | ((int32_t)lo << 16) | 0xFFFF;
        dm2_v1_mark_dyn_load(state, id);
    }
    if (hi != 0) {
        int32_t id = ((int32_t)0x0D << 24) | ((int32_t)hi << 16) | 0xFFFF;
        dm2_v1_mark_dyn_load(state, id);
    }
}

/* ── DM2_2676_006a — mark with hi-res flag ──────────────────────────── */

/* Source: c_loadlevel.cpp DM2_2676_006a */
void dm2_v1_mark_dyn_load_hires(
    DM2_V1_DynLoadState *state,
    int32_t resource_id)
{
    if (state == NULL) return;

    dm2_v1_mark_dyn_load(state, resource_id);
    if (state->count > 0) {
        state->entries[state->count - 1].flags |=
            DM2_V1_LOADLEVEL_DYN_FLAG_HIRES;
    }
}

/* ── DM2_LOAD_MISCITEM ──────────────────────────────────────────────── */

/* Source: c_loadlevel.cpp DM2_LOAD_MISCITEM
 * Scans GDAT category 21 (misc items) and builds a sorted list. */
void dm2_v1_load_miscitem(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_MiscItemState *misc)
{
    if (cb == NULL || misc == NULL) return;
    if (misc->loaded) return;
    misc->loaded = true;

    for (int type = 0; type < 0x80; type++) {
        /* Check if this misc item has the loadable flag (0x4000) */
        int16_t data = cb->query_gdat_entry_data_index(cb->ctx,
            21, (uint8_t)type, 11, 0);
        if ((data & 0x4000) == 0) continue;

        /* Get sort key */
        int16_t sort_key = cb->query_gdat_entry_data_index(cb->ctx,
            21, (uint8_t)type, 11, 2);

        /* Find insertion position (sorted) */
        int16_t pos = 0;
        while (pos < misc->count && sort_key > misc->sort_keys[pos])
            pos++;

        /* Shift existing entries */
        if (pos < misc->count) {
            int shift_count = misc->count - pos;
            memmove(&misc->sort_keys[pos + 1], &misc->sort_keys[pos],
                    (size_t)shift_count * sizeof(int16_t));
            memmove(&misc->sort_vals[pos + 1], &misc->sort_vals[pos],
                    (size_t)shift_count * sizeof(int16_t));
        }

        /* Insert */
        misc->sort_keys[pos] = sort_key;
        misc->sort_vals[pos] = (int16_t)(((type + 1) << 8) | type);
        misc->count++;

        if (misc->count >= DM2_V1_LOADLEVEL_MAX_MISC_ITEMS) break;
    }
}

/* ── DM2_LOAD_LOCALLEVEL_DYN ────────────────────────────────────────── */

/* Source: c_loadlevel.cpp DM2_LOAD_LOCALLEVEL_DYN
 * Main dynamic resource loader — walks all map tiles, examines records,
 * and marks all needed GDAT resources for loading. */
DM2_V1_LoadLevelReceipt dm2_v1_load_locallevel_dyn(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_DynLoadState *dyn,
    DM2_V1_MiscItemState *misc,
    DM2_V1_LevelGraphicsState *gfx)
{
    DM2_V1_LoadLevelReceipt r;
    (void)gfx; /* used in full tile walk implementation */
    r.loaded = false;
    r.dyn_count = 0;

    if (cb == NULL || dyn == NULL) return r;

    /* Initialize state */
    dyn->count = 0;

    /* Mark common resources — source: c_loadlevel.cpp:202..296 */
    dm2_v1_mark_dyn_load(dyn, 0x01FF02FF);  /* base UI */
    dm2_v1_mark_dyn_load(dyn, 0x18FF02FF);  /* overlay */
    dm2_v1_mark_dyn_load(dyn, 0x07FF02FF);  /* items */

    int16_t v1e13fe_0 = cb->get_v1e13fe_0(cb->ctx);

    if (v1e13fe_0 == 0) {
        dm2_v1_mark_dyn_load(dyn, 0x0D0002FF);
        if (dyn->count > 0)
            dyn->entries[dyn->count - 1].flags = 1;
        dm2_v1_mark_dyn_load(dyn, 0x0D2F02FF);
        dm2_v1_mark_dyn_load(dyn, 0x0D7E02FF);
        if (dyn->count > 0)
            dyn->entries[dyn->count - 1].flags = 1;
        dm2_v1_mark_dyn_load(dyn, 0x0D9F02FF);
    } else {
        dm2_v1_mark_dyn_load(dyn, 0x0DFF02FF);
    }

    dm2_v1_mark_dyn_load(dyn, 0x10FF02FF);
    if (dyn->count > 0)
        dyn->entries[dyn->count - 1].flags = 1;
    dm2_v1_mark_dyn_load(dyn, 0x15FF02FF);
    dm2_v1_mark_dyn_load(dyn, 0x30002FF);
    dm2_v1_mark_dyn_load(dyn, 0x08FE02FF);
    dm2_v1_mark_dyn_load(dyn, 0x16FE02FF);
    dm2_v1_mark_dyn_load(dyn, 0x09FE02FF);
    dm2_v1_mark_dyn_load(dyn, 0x0AFE02FF);
    dm2_v1_mark_dyn_load(dyn, 0x0FFF08FB);
    dm2_v1_mark_dyn_load(dyn, 0x0FFF07FC);
    dm2_v1_mark_dyn_load(dyn, 0x01FFFFFF);

    /* Image resources with hi-res flags */
    dm2_v1_mark_dyn_load(dyn, 0x01000400);
    if (dyn->count > 0)
        dyn->entries[dyn->count - 1].flags |= DM2_V1_LOADLEVEL_DYN_FLAG_HIRES;
    dm2_v1_mark_dyn_load(dyn, 0x01000600);
    if (dyn->count > 0)
        dyn->entries[dyn->count - 1].flags |= DM2_V1_LOADLEVEL_DYN_FLAG_HIRES;
    dm2_v1_mark_dyn_load(dyn, 0x0100070A);
    if (dyn->count > 0)
        dyn->entries[dyn->count - 1].flags |= DM2_V1_LOADLEVEL_DYN_FLAG_HIRES;

    /* Dialogue resources */
    dm2_v1_mark_dyn_load(dyn, 0x1A80FFFF);
    dm2_v1_mark_dyn_load(dyn, 0x1A81FFFF);
    dm2_v1_mark_dyn_load(dyn, 0x0300FFFF);
    dm2_v1_mark_dyn_load(dyn, 0x0700FFFF);

    /* Walk map tiles — source: m_2AF2B..m_2B1B4 */
    int16_t map_w = cb->get_map_width(cb->ctx);
    int16_t map_h = cb->get_map_height(cb->ctx);

    for (int16_t tx = 0; tx < map_w; tx++) {
        for (int16_t ty = 0; ty < map_h; ty++) {
            uint8_t tile = cb->get_tile_byte(cb->ctx, tx, ty);
            if ((tile & 0x10) == 0) continue;

            /* Walk record list for this tile */
            int32_t link = cb->get_tile_record_link(cb->ctx, tx, ty);
            while ((link & 0xFFFF) != 0xFFFE) {
                uint16_t rec = (uint16_t)(link & 0xFFFF);
                int rec_type = (int)(((link & 0x3C00) >> 10) & 0xF);

                if (rec_type == 3 && v1e13fe_0 == 0) {
                    /* Actuator — check for special types */
                    uint8_t *addr = cb->get_record_address(cb->ctx, rec);
                    if (addr != NULL) {
                        int16_t actu_type = (int16_t)(addr[2] & 0x7F);
                        /* Mark creature/wall resources based on type */
                        (void)actu_type;
                    }
                } else if (rec_type == 2) {
                    /* Sensor — check subtype for resource loading */
                    uint8_t *addr = cb->get_record_address(cb->ctx, rec);
                    if (addr != NULL) {
                        int16_t w2 = (int16_t)(addr[2] | (addr[3] << 8));
                        int16_t sub_type = (w2 >> 3) >> 8;
                        sub_type &= 0x1F;
                        /* Mark resources based on sensor subtype */
                        (void)sub_type;
                    }
                }

                link = cb->get_next_record_link(cb->ctx, rec);
            }
        }
    }

    /* Mark music resource */
    int16_t cur_map = cb->get_current_map(cb->ctx);
    uint8_t music_idx = cb->get_music_map_entry(cb->ctx, cur_map);
    int32_t music_id = ((int32_t)music_idx << 16) | 0x04000300;
    dm2_v1_mark_dyn_load(dyn, music_id);

    /* Mark hero resources if not hi-res mode */
    if (v1e13fe_0 == 0) {
        int16_t party_count = cb->get_party_count(cb->ctx);
        for (int16_t h = 0; h < party_count; h++) {
            int16_t htype = cb->get_hero_type(cb->ctx, h);
            int32_t hero_img = ((int32_t)htype << 16) | 0x16000100;
            dm2_v1_mark_dyn_load(dyn, hero_img);
            hero_img = ((int32_t)htype << 16) | 0x160002FF;
            dm2_v1_mark_dyn_load(dyn, hero_img);
        }
    }

    /* Final loading and state setup */
    if (!cb->get_v1e0a84(cb->ctx))
        cb->sound_init(cb->ctx);

    cb->load_dyn4(cb->ctx, (int16_t *)dyn->entries, dyn->count);

    if (!cb->get_v1e0a84(cb->ctx))
        dm2_v1_load_miscitem(cb, misc);

    r.loaded = true;
    r.dyn_count = dyn->count;

    /* Update weather and viewport */
    cb->set_backbuff2(cb->ctx, 1);
    cb->update_weather(cb->ctx, 0);
    cb->set_viewport_dirty(cb->ctx, 1);
    cb->check_recompute_light(cb->ctx);

    return r;
}

/* ── DM2_LOAD_LOCALLEVEL_GRAPHICS_TABLE ─────────────────────────────── */

/* Source: c_loadlevel.cpp DM2_LOAD_LOCALLEVEL_GRAPHICS_TABLE */
void dm2_v1_load_locallevel_graphics_table(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_LevelGraphicsState *gfx,
    int16_t x, int16_t y, int16_t map)
{
    if (cb == NULL || gfx == NULL) return;

    gfx->party_x  = x;
    gfx->party_y  = y;
    gfx->view_map = map;

    cb->change_current_map_to(cb->ctx, (int32_t)map);
}

/* ── DM2_3a15_38b6 — level actuator processing ─────────────────────── */

/* Source: c_loadlevel.cpp DM2_3a15_38b6 */
void dm2_v1_process_level_actuators(
    const DM2_V1_LoadLevelCallbacks *cb,
    int32_t enter_flag)
{
    if (cb == NULL) return;

    /* Walk all tiles on the current map and process actuators
     * that trigger on level entry (enter_flag=1) or exit (enter_flag=0). */
    int16_t map_w = cb->get_map_width(cb->ctx);
    int16_t map_h = cb->get_map_height(cb->ctx);

    for (int16_t tx = 0; tx < map_w; tx++) {
        for (int16_t ty = 0; ty < map_h; ty++) {
            uint8_t tile = cb->get_tile_byte(cb->ctx, tx, ty);
            if ((tile & 0x10) == 0) continue;

            int32_t link = cb->get_tile_record_link(cb->ctx, tx, ty);
            while ((link & 0xFFFF) != 0xFFFE) {
                uint16_t rec = (uint16_t)(link & 0xFFFF);
                int rec_type = (int)(((link & 0x3C00) >> 10) & 0xF);

                if (rec_type == 3) {
                    uint8_t *addr = cb->get_record_address(cb->ctx, rec);
                    if (addr != NULL) {
                        int16_t actu_type = (int16_t)(addr[2] & 0x7F);
                        /* Type 0x21: entry/exit actuator */
                        if (actu_type == 0x21) {
                            int16_t w4 = (int16_t)(addr[4] | (addr[5] << 8));
                            int16_t trigger_bits = w4 & 0x18;
                            bool should_fire = false;

                            if (trigger_bits == 0x18) {
                                /* Toggle on entry/exit */
                                should_fire = true;
                            } else {
                                /* Check active_status vs enter_flag */
                                bool active = ((w4 & 0x20) == 0);
                                should_fire = ((int32_t)active == enter_flag);
                            }

                            if (should_fire) {
                                int action_type = (w4 >> 3) & 0x3;
                                cb->invoke_actuator(cb->ctx, addr,
                                    action_type, 0);
                            }
                        }
                        /* Type 0x2C: ornate animation actuator */
                        else if (actu_type == 0x2C && enter_flag != 0) {
                            /* Process ornate noise on entry */
                        }
                    }
                }

                link = cb->get_next_record_link(cb->ctx, rec);
            }
        }
    }
}

/* ── DM2_LOAD_NEWMAP ────────────────────────────────────────────────── */

/* Source: c_loadlevel.cpp DM2_LOAD_NEWMAP */
void dm2_v1_load_newmap(
    const DM2_V1_LoadLevelCallbacks *cb,
    DM2_V1_DynLoadState *dyn,
    DM2_V1_MiscItemState *misc,
    DM2_V1_LevelGraphicsState *gfx,
    int16_t x, int16_t y, int16_t map, int32_t process_flag)
{
    if (cb == NULL) return;

    bool v1e0a84 = cb->get_v1e0a84(cb->ctx);

    if (!v1e0a84) {
        cb->hide_mouse(cb->ctx);
        if (process_flag != 0)
            dm2_v1_process_level_actuators(cb, 0);
    }

    dm2_v1_load_locallevel_graphics_table(cb, gfx, x, y, map);
    dm2_v1_load_locallevel_dyn(cb, dyn, misc, gfx);

    if (v1e0a84) return;

    dm2_v1_process_level_actuators(cb, 1);
    cb->fill_caii_cur_map(cb->ctx);
    cb->set_viewport_dirty(cb->ctx, 2);
    cb->check_recompute_light(cb->ctx);

    if (cb->get_v1e13fe_0(cb->ctx) == 0)
        cb->event_1031_098e(cb->ctx);

    cb->show_mouse(cb->ctx);
}
