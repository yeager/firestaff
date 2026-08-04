/* DM2 V1 viewport rendering — skproject c_gui_vp.cpp.
 *
 * 48 functions implementing the dungeon viewport rendering pipeline.
 * Each public function uses a callback struct for game state access.
 * Internal helpers are called through the callbacks passed down
 * from the entry points. */

#include "dm2_v1_gui_vp_pc34_compat.h"
#include <stddef.h>
#include <string.h>

/* ================================================================
 * Data tables from c_gui_vp.cpp
 * ================================================================ */

const int16_t dm2_guivp_table1d27a0[16] = {
    -1, 0x1149, 0x1162, -1, 0x1194, 0x11ad, -1, 0x11df,
    0x11f8, 0x1211, 0x122a, -1, 0x125c, 0x1275, 0x128e, 0x12a7
};

/* Tile iteration order for DM2_DRAW_DUNGEON_TILES.
 * 20 entries: tile indices processed back-to-front. */
const uint8_t dm2_guivp_table1d7029[20] = {
    0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09,
    0x08, 0x07, 0x06, 0x05, 0x04, 0x03
};

/* ================================================================
 * 1. TRIM_BLIT_RECT — set global blit rectangle
 *    c_gui_vp.cpp:570
 * ================================================================ */

void dm2_v1_trim_blit_rect(
    int16_t x, int16_t y, int16_t w, int16_t h,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->set_blit_rect) return;
    int16_t backbuffer_w = cb->get_backbuffer_w(ctx);
    int16_t backbuffer_h = cb->get_backbuffer_h(ctx);
    int16_t rect[4];
    rect[0] = x;
    rect[1] = y;
    rect[2] = backbuffer_w - (x + w);
    rect[3] = backbuffer_h - (y + h);
    cb->set_blit_rect(ctx, rect[0], rect[1], rect[2], rect[3]);
}

/* ================================================================
 * 2. guivp_098d_0cd7 — compute GDAT index from position
 *    c_gui_vp.cpp:150
 * ================================================================ */

int16_t dm2_v1_guivp_098d_0cd7(int16_t wa, int16_t wc, bool vbool)
{
    if (!vbool)
        return (int16_t)(25 * wa + 3100 + wc);
    return (int16_t)(dm2_guivp_table1d27a0[wa] + wc);
}

/* ================================================================
 * 3. SET_GRAPHICS_FLIP_FROM_POSITION — compute flip state
 *    c_gui_vp.cpp:79 (SKW_32cb_59ca)
 * ================================================================ */

int32_t dm2_v1_set_graphics_flip_from_position(
    int32_t mode, int32_t dir, int32_t col, int32_t row,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->set_graphics_flip) return 0;
    return cb->set_graphics_flip(ctx, mode, dir, col, row);
}

/* ================================================================
 * 4. DRAW_MIRRORED_PIC — mirror-blit a bitmap
 *    c_gui_vp.cpp:47
 * ================================================================ */

void dm2_v1_draw_mirrored_pic(
    void *srcbmp, void *destbmp,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_mirrored_pic) return;
    cb->draw_mirrored_pic(ctx, srcbmp, destbmp);
}

/* ================================================================
 * 5. guivp_32cb_0649 — query and transform local palette
 *    c_gui_vp.cpp:793
 * ================================================================ */

void *dm2_v1_guivp_32cb_0649(
    int8_t cls, int8_t sub, int8_t ornament, int16_t color_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_palette) return NULL;
    return cb->query_palette(ctx, cls, sub, ornament, color_idx);
}

/* ================================================================
 * 6. guivp_32cb_00f1 — point-in-rect hit test
 *    c_gui_vp.cpp:723 (public)
 * ================================================================ */

bool dm2_v1_guivp_32cb_00f1(
    int32_t x, int32_t y, int32_t flags,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx,
    DM2_V1_GuiVp00f1Receipt *receipt)
{
    DM2_V1_GuiVp00f1Receipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb || !cb->viewport_hit_test) return false;

    int16_t out_da = 0, out_d8 = 0;
    bool hit = cb->viewport_hit_test(ctx, (int16_t)x, (int16_t)y,
                                     (uint8_t)(flags & 0xFF),
                                     &out_da, &out_d8);
    if (receipt) {
        receipt->hit = hit;
        receipt->viewport_da = out_da;
        receipt->viewport_d8 = out_d8;
    }
    return hit;
}

/* ================================================================
 * 7. guivp_32cb_0a4c — store clickable zone
 *    c_gui_vp.cpp:966
 * ================================================================ */

void dm2_v1_guivp_32cb_0a4c(
    int16_t rx, int16_t ry, int16_t rw, int16_t rh,
    int8_t type_byte, int8_t sub_byte,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->store_clickzone) return;
    cb->store_clickzone(ctx, rx, ry, rw, rh, type_byte, sub_byte);
}

/* ================================================================
 * 8. guivp_32cb_0c7d — fill/rain-composite image background
 *    c_gui_vp.cpp:993 (public)
 * ================================================================ */

void dm2_v1_guivp_32cb_0c7d(
    void *image, int16_t bmpid, int16_t colidx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->fill_image_background) return;
    cb->fill_image_background(ctx, image, bmpid, colidx);
}

/* ================================================================
 * 9. DRAW_PIT_ROOF — draw ceiling above pit
 *    c_gui_vp.cpp:158 (SKW_32cb_2367)
 * ================================================================ */

void dm2_v1_draw_pit_roof(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_pit_roof) return;
    cb->draw_pit_roof(ctx, tile_idx);
}

/* ================================================================
 * 10. DRAW_PIT_TILE — draw pit opening
 *     c_gui_vp.cpp:234 (SKW_32cb_245a)
 * ================================================================ */

void dm2_v1_draw_pit_tile(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_pit_tile) return;
    cb->draw_pit_tile(ctx, tile_idx);
}

/* ================================================================
 * 11. DRAW_STAIRS_FRONT — draw stairs facing player
 *     c_gui_vp.cpp:468 (SKW_32cb_4e1c)
 * ================================================================ */

void dm2_v1_draw_stairs_front(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_stairs_front) return;
    cb->draw_stairs_front(ctx, tile_idx);
}

/* ================================================================
 * 12. DRAW_STAIRS_SIDE — draw stairs from side
 *     c_gui_vp.cpp:532 (SKW_32cb_4ecc)
 * ================================================================ */

void dm2_v1_draw_stairs_side(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_stairs_side) return;
    cb->draw_stairs_side(ctx, tile_idx);
}

/* ================================================================
 * 13. DRAW_WALL — draw wall face
 *     c_gui_vp.cpp:575
 * ================================================================ */

void dm2_v1_draw_wall(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_wall) return;
    cb->draw_wall(ctx, tile_idx);
}

/* ================================================================
 * 14. DRAW_WALL_TILE — dispatch wall drawing + ornaments
 *     c_gui_vp.cpp:6703
 * ================================================================ */

void dm2_v1_draw_wall_tile(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb) return;
    dm2_v1_draw_wall(tile_idx, cb, ctx);

    if (cb->get_wall_ornament_type) {
        int8_t orn_type = cb->get_wall_ornament_type(ctx, tile_idx);
        if (orn_type == 1) {
            dm2_v1_guivp_32cb_15b8(tile_idx, 0, 1, cb, ctx, NULL);
        } else if (orn_type == 2) {
            dm2_v1_guivp_32cb_15b8(tile_idx, 0, 1, cb, ctx, NULL);
        } else if (orn_type == 3) {
            dm2_v1_guivp_32cb_15b8(tile_idx, 0, 1, cb, ctx, NULL);
        }
    }

    if (cb->get_wall_side_indicator) {
        int16_t side = cb->get_wall_side_indicator(ctx, tile_idx);
        dm2_v1_guivp_32cb_15b8(tile_idx, side, 1, cb, ctx, NULL);
    }
}

/* ================================================================
 * 15. DRAW_DOOR_TILE — dispatch door drawing
 *     c_gui_vp.cpp:5125 (SKW_32cb_4cdf)
 * ================================================================ */

void dm2_v1_draw_door_tile(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_door_tile) return;
    cb->draw_door_tile(ctx, tile_idx);
}

/* ================================================================
 * 16. DRAW_DOOR — draw door panel
 *     c_gui_vp.cpp:4806
 * ================================================================ */

void dm2_v1_draw_door(
    int32_t tile_idx, int32_t frame_flags,
    int32_t static_flags, int32_t extra_flags,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_door) return;
    cb->draw_door(ctx, tile_idx, frame_flags, static_flags, extra_flags);
}

/* ================================================================
 * 17. DRAW_DOOR_FRAMES — draw door frame ornaments
 *     c_gui_vp.cpp:2333
 * ================================================================ */

void dm2_v1_draw_door_frames(
    int32_t tile_idx, int32_t flags,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_door_frames) return;
    cb->draw_door_frames(ctx, tile_idx, flags);
}

/* ================================================================
 * 18. DRAW_DEFAULT_DOOR_BUTTON — draw button on door
 *     c_gui_vp.cpp:1904
 * ================================================================ */

void dm2_v1_draw_default_door_button(
    int32_t type, int32_t sub, int32_t ornament, int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_door_button) return;
    cb->draw_door_button(ctx, type, sub, ornament, tile_idx);
}

/* ================================================================
 * 19. DRAW_TELEPORTER_TILE — draw teleporter effect
 *     c_gui_vp.cpp:824 (SKW_32cb_24fb)
 * ================================================================ */

void dm2_v1_draw_teleporter_tile(
    int32_t tile_idx, int32_t type, int32_t flags,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_teleporter_tile) return;
    cb->draw_teleporter_tile(ctx, tile_idx, type, flags);
}

/* ================================================================
 * 20. DRAW_EXTERNAL_TILE — draw wall ornaments / external elements
 *     c_gui_vp.cpp:4427 (SKW_32cb_1f3e)
 * ================================================================ */

int32_t dm2_v1_draw_external_tile(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_external_tile) return 0;
    return cb->draw_external_tile(ctx, tile_idx);
}

/* ================================================================
 * 21. DRAW_PLAYER_TILE — draw player's own tile
 *     c_gui_vp.cpp:5288 (SKW_32cb_5340)
 * ================================================================ */

int32_t dm2_v1_draw_player_tile(
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_player_tile) return 0;
    return cb->draw_player_tile(ctx);
}

/* ================================================================
 * 22. DRAW_RAIN — draw rain overlay
 *     c_gui_vp.cpp:1994 (SKW_32cb_0b11)
 * ================================================================ */

void dm2_v1_draw_rain(
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_rain) return;
    cb->draw_rain(ctx);
}

/* ================================================================
 * 23. DRAW_ITEM — draw item on tile (public)
 *     c_gui_vp.cpp:2078 (SKW_32cb_3672)
 * ================================================================ */

void dm2_v1_draw_item(
    int32_t record, int32_t tile, int32_t flags1, int32_t flags2,
    int16_t pos, void *creature_ptr, int32_t mode,
    int16_t dir, int32_t argl4,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_item) return;
    cb->draw_item(ctx, record, tile, flags1, flags2,
                  pos, creature_ptr, mode, dir, argl4);
}

/* ================================================================
 * 24. DRAW_FLYING_ITEM — draw projectile in flight
 *     c_gui_vp.cpp:3458
 * ================================================================ */

void dm2_v1_draw_flying_item(
    int32_t record, int32_t tile, int32_t pos,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_flying_item) return;
    cb->draw_flying_item(ctx, record, tile, pos);
}

/* ================================================================
 * 25. DRAW_PUT_DOWN_ITEM — draw item on ground
 *     c_gui_vp.cpp:3893
 * ================================================================ */

void dm2_v1_draw_put_down_item(
    int32_t record, int32_t tile, int32_t dir, void *creature_ptr,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_put_down_item) return;
    cb->draw_put_down_item(ctx, record, tile, dir, creature_ptr);
}

/* ================================================================
 * 26. MAKE_PUT_DOWN_ITEM_CLICKABLE_ZONE — register item clickzone
 *     c_gui_vp.cpp:3816
 * ================================================================ */

void dm2_v1_make_put_down_item_clickable_zone(
    int32_t tile_idx, int32_t flags1, int32_t flags2, int32_t flags3,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->make_item_clickzone) return;
    cb->make_item_clickzone(ctx, tile_idx, flags1, flags2, flags3);
}

/* ================================================================
 * 27. DRAW_STATIC_OBJECT — draw static objects on tile
 *     c_gui_vp.cpp:4239 (SKW_32cb_3b9d)
 * ================================================================ */

void dm2_v1_draw_static_object(
    int32_t tile_idx, int32_t mask, int32_t flags,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_static_object) return;
    cb->draw_static_object(ctx, tile_idx, mask, flags);
}

/* ================================================================
 * 28. SUMMARY_DRAW_CREATURE — draw all creatures on tile
 *     c_gui_vp.cpp:4144
 * ================================================================ */

int32_t dm2_v1_summary_draw_creature(
    int32_t record, int32_t tile_idx, int32_t mask,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->summary_draw_creature) return 0;
    return cb->summary_draw_creature(ctx, record, tile_idx, mask);
}

/* ================================================================
 * 29. guivp_32cb_3e08 — draw put-down items around creature
 *     c_gui_vp.cpp:4070
 * ================================================================ */

int32_t dm2_v1_guivp_32cb_3e08(
    int32_t record, int32_t tile_idx, int32_t mask, void *creature_ptr,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_creature_items) return 0;
    return cb->draw_creature_items(ctx, record, tile_idx, mask, creature_ptr);
}

/* ================================================================
 * 30. guivp_32cb_3edd — finalize deferred put-down items
 *     c_gui_vp.cpp:5106
 * ================================================================ */

int32_t dm2_v1_guivp_32cb_3edd(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->finalize_deferred_items) return 0;
    return cb->finalize_deferred_items(ctx, tile_idx);
}

/* ================================================================
 * 31. guivp_32cb_2cf3 — draw creature summary image
 *     c_gui_vp.cpp:1057
 * ================================================================ */

void dm2_v1_guivp_32cb_2cf3(
    int32_t type, int32_t flags, int32_t blitmode, int32_t query1,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_creature_summary) return;
    cb->draw_creature_summary(ctx, type, flags, blitmode, query1);
}

/* ================================================================
 * 32. guivp_32cb_2d8c — draw creature items on tile
 *     c_gui_vp.cpp:1091
 * ================================================================ */

int32_t dm2_v1_guivp_32cb_2d8c(
    int32_t record, int32_t tile_idx, int32_t mask,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_creature_tile_items) return 0;
    return cb->draw_creature_tile_items(ctx, record, tile_idx, mask);
}

/* ================================================================
 * 33. guivp_32cb_35c1 — adjust item position for perspective
 *     c_gui_vp.cpp:1408
 * ================================================================ */

int32_t dm2_v1_guivp_32cb_35c1(
    int16_t *pos_ptr, int16_t *dist_ptr,
    int32_t dx, int32_t dy,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    (void)cb; (void)ctx;
    if (!pos_ptr || !dist_ptr) return 0;

    int16_t pos = *pos_ptr;
    int16_t dist = *dist_ptr;

    if ((int16_t)dy != 0) {
        dist -= (int16_t)(dy * 5);
        if (dist >= 0) {
            if (dist > 0x18) {
                if (cb && cb->get_tile_adjacency)
                    pos = cb->get_tile_adjacency(ctx, pos, 3);
                dist -= 0x14;
            }
        } else {
            if (cb && cb->get_tile_adjacency)
                pos = cb->get_tile_adjacency(ctx, pos, 2);
            dist += 0x14;
        }
        if (pos < 0) return 0;
    }

    if ((int16_t)dx != 0) {
        int32_t tmp = dist;
        int32_t rem = tmp % 5;
        rem += (int16_t)dx;
        if (dist <= 4) {
            if (rem >= 0) {
                dist += (int16_t)dx;
            } else {
                dist += (int16_t)dx + 4;
                if (cb && cb->get_tile_adjacency)
                    pos = cb->get_tile_adjacency(ctx, pos, 0);
            }
        } else {
            dist += (int16_t)dx - 4;
            if (cb && cb->get_tile_adjacency)
                pos = cb->get_tile_adjacency(ctx, pos, 1);
        }
        if (pos < 0) return 0;
    }

    *pos_ptr = pos;
    *dist_ptr = dist;
    return 1;
}

/* ================================================================
 * 34. guivp_32cb_4069 — interpolate creature movement
 *     c_gui_vp.cpp:1495
 * ================================================================ */

void dm2_v1_guivp_32cb_4069(
    int32_t x, int32_t y, int32_t distance,
    int16_t *pos_x_ptr, int16_t *pos_y_ptr,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->interpolate_creature_move) return;
    cb->interpolate_creature_move(ctx, x, y, distance, pos_x_ptr, pos_y_ptr);
}

/* ================================================================
 * 35. guivp_32cb_48d5 — compute stretched door dimension
 *     c_gui_vp.cpp:1599
 * ================================================================ */

int32_t dm2_v1_guivp_32cb_48d5(
    int32_t size, int32_t scale,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->calc_stretched_size) return 0;
    int32_t usize = size & 0xFFFF;
    int32_t half = (usize * 128 + 0x40) / scale / 2;
    int16_t stretched = cb->calc_stretched_size(ctx, (int16_t)scale, (int16_t)half);
    return (stretched >= (int16_t)usize) ? half : half + 1;
}

/* ================================================================
 * 36. ENVIRONMENT_DRAW_DISTANT_ELEMENT
 *     c_gui_vp.cpp:296 (SKW_32cb_56bc)
 * ================================================================ */

void dm2_v1_environment_draw_distant_element(
    void *hexa_ptr, int32_t dir, int32_t col, int32_t row,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_distant_element) return;
    cb->draw_distant_element(ctx, hexa_ptr, dir, col, row);
}

/* ================================================================
 * 37. ENVIRONMENT_SET_DISTANT_ELEMENT
 *     c_gui_vp.cpp:6746 (SKW_32cb_5598)
 * ================================================================ */

int32_t dm2_v1_environment_set_distant_element(
    void *hexa_ptr, void *text, int32_t dir, int32_t col, int32_t arg,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->set_distant_element) return 0;
    return cb->set_distant_element(ctx, hexa_ptr, text, dir, col, arg);
}

/* ================================================================
 * 38. ENVIRONMENT_DISPLAY_ELEMENTS
 *     c_gui_vp.cpp:6815 (SKW_32cb_5824)
 * ================================================================ */

void dm2_v1_environment_display_elements(
    int32_t heading, int32_t dir, int32_t col,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->display_environment_elements) return;
    cb->display_environment_elements(ctx, heading, dir, col);
}

/* ================================================================
 * 39. guivp_32cb_54ce — compute distance/direction to element
 *     c_gui_vp.cpp:1621
 * ================================================================ */

int32_t dm2_v1_guivp_32cb_54ce(
    int32_t heading, int16_t *x_ptr, int16_t *y_ptr,
    int32_t dir, int32_t arg)
{
    if (!x_ptr || !y_ptr) return 0;

    int16_t rx, ry;
    uint16_t uheading = (uint16_t)heading;

    if (uheading <= 3) {
        switch (uheading) {
        case 0:
            rx = (int16_t)(*y_ptr - arg);
            ry = (int16_t)(dir - *x_ptr);
            break;
        case 1:
            rx = (int16_t)(dir - *x_ptr);
            ry = (int16_t)(arg - *y_ptr);
            break;
        case 2:
            rx = (int16_t)(arg - *y_ptr);
            ry = (int16_t)(*x_ptr - dir);
            break;
        case 3:
            rx = (int16_t)(*x_ptr - dir);
            ry = (int16_t)(*y_ptr - arg);
            break;
        default:
            return 0;
        }
    } else {
        return 0;
    }

    *x_ptr = ry;
    *y_ptr = rx;

    if (rx < 1) return 0;

    int32_t d2 = ry * ry + rx * rx;
    uint16_t ud = (uint16_t)d2;
    if (ud <= 2) return 1;

    uint16_t guess = ud >> 1;
    for (;;) {
        uint16_t prev = guess;
        guess = (uint16_t)((ud / guess + guess) >> 1);
        if (guess >= prev)
            return (int32_t)prev;
    }
}

/* ================================================================
 * 40. guivp_32cb_15b8 — draw wall ornament (public)
 *     c_gui_vp.cpp:6043
 * ================================================================ */

int32_t dm2_v1_guivp_32cb_15b8(
    int32_t tile_idx, int32_t side, int32_t mode,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx,
    DM2_V1_GuiVp15b8Receipt *receipt)
{
    DM2_V1_GuiVp15b8Receipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;

    if (!cb || !cb->draw_wall_ornament) return -1;
    int32_t result = cb->draw_wall_ornament(ctx, tile_idx, side, mode);
    if (receipt) {
        receipt->ornament_drawn = (result >= 0);
        receipt->ornament_id = (int16_t)(result & 0xFF);
    }
    return result;
}

/* ================================================================
 * 41. guivp_32cb_3f0d — draw wall ornament actuator items
 *     c_gui_vp.cpp:5430
 * ================================================================ */

void dm2_v1_guivp_32cb_3f0d(
    int32_t tile_idx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_ornament_actuator_items) return;
    cb->draw_ornament_actuator_items(ctx, tile_idx);
}

/* ================================================================
 * 42. guivp_32cb_0f82 — draw creature item masks on ornament
 *     c_gui_vp.cpp:5537
 * ================================================================ */

void dm2_v1_guivp_32cb_0f82(
    void *creature_ptr, int32_t type, int32_t flags, int32_t dir,
    int16_t arg0, int16_t arg1, int16_t arg2, int16_t arg3, int16_t arg4,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_creature_item_masks) return;
    cb->draw_creature_item_masks(ctx, creature_ptr, type, flags, dir,
                                  arg0, arg1, arg2, arg3, arg4);
}

/* ================================================================
 * 43. SUMMARIZE_STONE_ROOM — compute tile summary (public)
 *     c_gui_vp.cpp:2497 (SKW_0cee_1dbe)
 * ================================================================ */

void dm2_v1_summarize_stone_room(
    DM2_V1_StoneRoomSummary *out,
    int32_t heading, int32_t x, int32_t y,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));
    out->record_link = 0xFF;
    out->ornament_word6 = 0xFF;
    out->ornament_word8 = 0xFF;
    out->ornament_wordA = 0xFF;
    out->ornament_wordC = 0xFF;
    out->ornament_wordE = 0;

    if (!cb || !cb->summarize_stone_room) return;
    cb->summarize_stone_room(ctx, out, heading, x, y);
}

/* ================================================================
 * 44. guivp_32cb_4185 — compute tile info table entry
 *     c_gui_vp.cpp:3089
 * ================================================================ */

void dm2_v1_guivp_32cb_4185(
    int32_t dir, int32_t col, int32_t distance, int32_t heading,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->compute_tile_info) return;
    cb->compute_tile_info(ctx, dir, col, distance, heading);
}

/* ================================================================
 * 45. guivp_32cb_5a8f — compute wall visibility mask
 *     c_gui_vp.cpp:1705
 * ================================================================ */

void dm2_v1_guivp_32cb_5a8f(
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->compute_wall_visibility) return;
    cb->compute_wall_visibility(ctx);
}

/* ================================================================
 * 46. guivp_32cb_5c67 — clear visibility for teleporter tiles
 *     c_gui_vp.cpp:1850
 * ================================================================ */

int32_t dm2_v1_guivp_32cb_5c67(
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->clear_teleporter_visibility) return 0;
    return cb->clear_teleporter_visibility(ctx);
}

/* ================================================================
 * 47. DRAW_DUNGEON_TILES — main viewport draw loop
 *     c_gui_vp.cpp:6932
 * ================================================================ */

int32_t dm2_v1_draw_dungeon_tiles(
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->draw_dungeon_tiles) return 0;
    return cb->draw_dungeon_tiles(ctx);
}

/* ================================================================
 * 48. CHANCE_TABLE_OPERATION — compute mouse-based arrows
 *     c_gui_vp.cpp:7177
 * ================================================================ */

void dm2_v1_chance_table_operation(
    const DM2_V1_GuiVpCallbacks *cb, void *ctx)
{
    if (!cb || !cb->chance_table_operation) return;
    cb->chance_table_operation(ctx);
}

/* ================================================================
 * 49. DISPLAY_VIEWPORT — top-level viewport render (public)
 *     c_gui_vp.cpp:7325 (SKW_32cb_5d0d)
 * ================================================================ */

void dm2_v1_display_viewport(
    int32_t heading, int32_t dir, int32_t col,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx,
    DM2_V1_DisplayViewportReceipt *receipt)
{
    DM2_V1_DisplayViewportReceipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;

    if (!cb || !cb->display_viewport) return;

    cb->display_viewport(ctx, heading, dir, col);

    if (receipt) {
        receipt->viewport_rendered = true;
    }
}
