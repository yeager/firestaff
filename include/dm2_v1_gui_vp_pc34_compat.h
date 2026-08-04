#ifndef FIRESTAFF_DM2_V1_GUI_VP_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_GUI_VP_PC34_COMPAT_H

/*
 * dm2_v1_gui_vp_pc34_compat.h — DM2 viewport rendering module.
 *
 * Source: skproject c_gui_vp.cpp (49 functions).
 * All public and internal functions use callback-based architecture.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Stone room summary — output of SUMMARIZE_STONE_ROOM
 * ======================================================================== */

typedef struct DM2_V1_StoneRoomSummary {
    uint8_t  record_link;
    uint8_t  ornament_word6;
    uint8_t  ornament_word8;
    uint8_t  ornament_wordA;
    uint8_t  ornament_wordC;
    uint8_t  ornament_wordE;
    int16_t  tile_type;
    int16_t  floor_orn;
    int16_t  ceil_orn;
    int16_t  wall_orn[4];
    uint16_t tile_flags;
} DM2_V1_StoneRoomSummary;

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_DisplayViewportReceipt {
    bool viewport_rendered;
} DM2_V1_DisplayViewportReceipt;

typedef struct DM2_V1_GuiVp00f1Receipt {
    bool    hit;
    int16_t viewport_da;
    int16_t viewport_d8;
} DM2_V1_GuiVp00f1Receipt;

typedef struct DM2_V1_GuiVp15b8Receipt {
    bool    ornament_drawn;
    int16_t ornament_id;
} DM2_V1_GuiVp15b8Receipt;

/* ========================================================================
 * Callback struct — external dependencies for the viewport renderer
 * ======================================================================== */

typedef struct DM2_V1_GuiVpCallbacks {
    /* Backbuffer dimensions */
    int16_t (*get_backbuffer_w)(void *ctx);
    int16_t (*get_backbuffer_h)(void *ctx);

    /* Blit rect */
    void (*set_blit_rect)(void *ctx, int16_t x, int16_t y, int16_t w, int16_t h);

    /* Graphics flip */
    int32_t (*set_graphics_flip)(void *ctx, int32_t mode, int32_t dir,
                                 int32_t col, int32_t row);

    /* Mirror blit */
    void (*draw_mirrored_pic)(void *ctx, void *srcbmp, void *destbmp);

    /* Palette query */
    void *(*query_palette)(void *ctx, int8_t cls, int8_t sub, int8_t ornament,
                           int16_t color_idx);

    /* Viewport hit test */
    bool (*viewport_hit_test)(void *ctx, int16_t x, int16_t y, uint8_t flags,
                              int16_t *out_da, int16_t *out_d8);

    /* Click zone storage */
    void (*store_clickzone)(void *ctx, int16_t rx, int16_t ry,
                            int16_t rw, int16_t rh,
                            int8_t type_byte, int8_t sub_byte);

    /* Image background fill */
    void (*fill_image_background)(void *ctx, void *image, int16_t bmpid,
                                  int16_t colidx);

    /* Tile drawing callbacks */
    void (*draw_pit_roof)(void *ctx, int32_t tile_idx);
    void (*draw_pit_tile)(void *ctx, int32_t tile_idx);
    void (*draw_stairs_front)(void *ctx, int32_t tile_idx);
    void (*draw_stairs_side)(void *ctx, int32_t tile_idx);
    void (*draw_wall)(void *ctx, int32_t tile_idx);
    void (*draw_door_tile)(void *ctx, int32_t tile_idx);
    void (*draw_door)(void *ctx, int32_t tile_idx, int32_t frame_flags,
                      int32_t static_flags, int32_t extra_flags);
    void (*draw_door_frames)(void *ctx, int32_t tile_idx, int32_t flags);
    void (*draw_door_button)(void *ctx, int32_t type, int32_t sub,
                             int32_t ornament, int32_t tile_idx);
    void (*draw_teleporter_tile)(void *ctx, int32_t tile_idx, int32_t type,
                                 int32_t flags);
    int32_t (*draw_external_tile)(void *ctx, int32_t tile_idx);
    int32_t (*draw_player_tile)(void *ctx);
    void (*draw_rain)(void *ctx);

    /* Item / creature drawing */
    void (*draw_item)(void *ctx, int32_t record, int32_t tile,
                      int32_t flags1, int32_t flags2,
                      int16_t pos, void *creature_ptr, int32_t mode,
                      int16_t dir, int32_t argl4);
    void (*draw_flying_item)(void *ctx, int32_t record, int32_t tile,
                             int32_t pos);
    void (*draw_put_down_item)(void *ctx, int32_t record, int32_t tile,
                               int32_t dir, void *creature_ptr);
    void (*make_item_clickzone)(void *ctx, int32_t tile_idx,
                                int32_t flags1, int32_t flags2, int32_t flags3);
    void (*draw_static_object)(void *ctx, int32_t tile_idx, int32_t mask,
                               int32_t flags);
    int32_t (*summary_draw_creature)(void *ctx, int32_t record,
                                     int32_t tile_idx, int32_t mask);
    int32_t (*draw_creature_items)(void *ctx, int32_t record,
                                   int32_t tile_idx, int32_t mask,
                                   void *creature_ptr);
    int32_t (*finalize_deferred_items)(void *ctx, int32_t tile_idx);

    /* Creature rendering support */
    void (*draw_creature_summary)(void *ctx, int32_t type, int32_t flags,
                                  int32_t blitmode, int32_t query1);
    int32_t (*draw_creature_tile_items)(void *ctx, int32_t record,
                                        int32_t tile_idx, int32_t mask);
    int16_t (*get_tile_adjacency)(void *ctx, int16_t pos, int32_t dir);
    void (*interpolate_creature_move)(void *ctx, int32_t x, int32_t y,
                                      int32_t distance,
                                      int16_t *pos_x_ptr, int16_t *pos_y_ptr);
    int16_t (*calc_stretched_size)(void *ctx, int16_t scale, int16_t half);

    /* Environment */
    void (*draw_distant_element)(void *ctx, void *hexa_ptr, int32_t dir,
                                 int32_t col, int32_t row);
    int32_t (*set_distant_element)(void *ctx, void *hexa_ptr, void *text,
                                   int32_t dir, int32_t col, int32_t arg);
    void (*display_environment_elements)(void *ctx, int32_t heading,
                                         int32_t dir, int32_t col);

    /* Wall ornament */
    int32_t (*draw_wall_ornament)(void *ctx, int32_t tile_idx, int32_t side,
                                  int32_t mode);
    int8_t (*get_wall_ornament_type)(void *ctx, int32_t tile_idx);
    int16_t (*get_wall_side_indicator)(void *ctx, int32_t tile_idx);
    void (*draw_ornament_actuator_items)(void *ctx, int32_t tile_idx);
    void (*draw_creature_item_masks)(void *ctx, void *creature_ptr,
                                     int32_t type, int32_t flags, int32_t dir,
                                     int16_t arg0, int16_t arg1, int16_t arg2,
                                     int16_t arg3, int16_t arg4);

    /* Stone room summary */
    void (*summarize_stone_room)(void *ctx, DM2_V1_StoneRoomSummary *out,
                                 int32_t heading, int32_t x, int32_t y);

    /* Tile table setup */
    void (*compute_tile_info)(void *ctx, int32_t dir, int32_t col,
                              int32_t distance, int32_t heading);
    void (*compute_wall_visibility)(void *ctx);
    int32_t (*clear_teleporter_visibility)(void *ctx);

    /* Main pipeline */
    int32_t (*draw_dungeon_tiles)(void *ctx);
    void (*chance_table_operation)(void *ctx);
    void (*display_viewport)(void *ctx, int32_t heading, int32_t dir,
                             int32_t col);
} DM2_V1_GuiVpCallbacks;

/* ========================================================================
 * Data tables
 * ======================================================================== */

extern const int16_t dm2_guivp_table1d27a0[16];
extern const uint8_t dm2_guivp_table1d7029[20];

/* ========================================================================
 * Public functions (callback pattern)
 * ======================================================================== */

void dm2_v1_display_viewport(
    int32_t heading, int32_t dir, int32_t col,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx,
    DM2_V1_DisplayViewportReceipt *receipt);

void dm2_v1_summarize_stone_room(
    DM2_V1_StoneRoomSummary *out,
    int32_t heading, int32_t x, int32_t y,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx);

void dm2_v1_draw_item(
    int32_t record, int32_t tile, int32_t flags1, int32_t flags2,
    int16_t pos, void *creature_ptr, int32_t mode,
    int16_t dir, int32_t argl4,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx);

bool dm2_v1_guivp_32cb_00f1(
    int32_t x, int32_t y, int32_t flags,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx,
    DM2_V1_GuiVp00f1Receipt *receipt);

void dm2_v1_guivp_32cb_0c7d(
    void *image, int16_t bmpid, int16_t colidx,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx);

int32_t dm2_v1_guivp_32cb_15b8(
    int32_t tile_idx, int32_t side, int32_t mode,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx,
    DM2_V1_GuiVp15b8Receipt *receipt);

/* ========================================================================
 * Internal functions (all take callbacks)
 * ======================================================================== */

/* Utility */
void dm2_v1_trim_blit_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                            const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int16_t dm2_v1_guivp_098d_0cd7(int16_t wa, int16_t wc, bool vbool);
int32_t dm2_v1_set_graphics_flip_from_position(
    int32_t mode, int32_t dir, int32_t col, int32_t row,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_mirrored_pic(void *srcbmp, void *destbmp,
                               const DM2_V1_GuiVpCallbacks *cb, void *ctx);

/* Palette/image */
void *dm2_v1_guivp_32cb_0649(int8_t cls, int8_t sub, int8_t ornament,
                              int16_t color_idx,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_guivp_32cb_0a4c(int16_t rx, int16_t ry, int16_t rw, int16_t rh,
                              int8_t type_byte, int8_t sub_byte,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);

/* Tile drawing */
void dm2_v1_draw_pit_roof(int32_t tile_idx,
                           const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_pit_tile(int32_t tile_idx,
                           const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_stairs_front(int32_t tile_idx,
                               const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_stairs_side(int32_t tile_idx,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_wall(int32_t tile_idx,
                       const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_wall_tile(int32_t tile_idx,
                            const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_door_tile(int32_t tile_idx,
                            const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_door(int32_t tile_idx, int32_t frame_flags,
                       int32_t static_flags, int32_t extra_flags,
                       const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_door_frames(int32_t tile_idx, int32_t flags,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_default_door_button(int32_t type, int32_t sub,
                                      int32_t ornament, int32_t tile_idx,
                                      const DM2_V1_GuiVpCallbacks *cb,
                                      void *ctx);
void dm2_v1_draw_teleporter_tile(int32_t tile_idx, int32_t type, int32_t flags,
                                  const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_draw_external_tile(int32_t tile_idx,
                                   const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_draw_player_tile(const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_rain(const DM2_V1_GuiVpCallbacks *cb, void *ctx);

/* Item / creature */
void dm2_v1_draw_flying_item(int32_t record, int32_t tile, int32_t pos,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_put_down_item(int32_t record, int32_t tile, int32_t dir,
                                void *creature_ptr,
                                const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_make_put_down_item_clickable_zone(
    int32_t tile_idx, int32_t flags1, int32_t flags2, int32_t flags3,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_draw_static_object(int32_t tile_idx, int32_t mask, int32_t flags,
                                const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_summary_draw_creature(int32_t record, int32_t tile_idx,
                                      int32_t mask,
                                      const DM2_V1_GuiVpCallbacks *cb,
                                      void *ctx);
int32_t dm2_v1_guivp_32cb_3e08(int32_t record, int32_t tile_idx, int32_t mask,
                                void *creature_ptr,
                                const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_guivp_32cb_3edd(int32_t tile_idx,
                                const DM2_V1_GuiVpCallbacks *cb, void *ctx);

/* Creature rendering support */
void dm2_v1_guivp_32cb_2cf3(int32_t type, int32_t flags, int32_t blitmode,
                              int32_t query1,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_guivp_32cb_2d8c(int32_t record, int32_t tile_idx, int32_t mask,
                                const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_guivp_32cb_35c1(int16_t *pos_ptr, int16_t *dist_ptr,
                                int32_t dx, int32_t dy,
                                const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_guivp_32cb_4069(int32_t x, int32_t y, int32_t distance,
                              int16_t *pos_x_ptr, int16_t *pos_y_ptr,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_guivp_32cb_48d5(int32_t size, int32_t scale,
                                const DM2_V1_GuiVpCallbacks *cb, void *ctx);

/* Environment */
void dm2_v1_environment_draw_distant_element(
    void *hexa_ptr, int32_t dir, int32_t col, int32_t row,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_environment_set_distant_element(
    void *hexa_ptr, void *text, int32_t dir, int32_t col, int32_t arg,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_environment_display_elements(
    int32_t heading, int32_t dir, int32_t col,
    const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_guivp_32cb_54ce(int32_t heading, int16_t *x_ptr,
                                int16_t *y_ptr, int32_t dir, int32_t arg);

/* Wall ornament */
void dm2_v1_guivp_32cb_3f0d(int32_t tile_idx,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_guivp_32cb_0f82(void *creature_ptr, int32_t type, int32_t flags,
                              int32_t dir,
                              int16_t arg0, int16_t arg1, int16_t arg2,
                              int16_t arg3, int16_t arg4,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);

/* Tile table setup */
void dm2_v1_guivp_32cb_4185(int32_t dir, int32_t col, int32_t distance,
                              int32_t heading,
                              const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_guivp_32cb_5a8f(const DM2_V1_GuiVpCallbacks *cb, void *ctx);
int32_t dm2_v1_guivp_32cb_5c67(const DM2_V1_GuiVpCallbacks *cb, void *ctx);

/* Main pipeline */
int32_t dm2_v1_draw_dungeon_tiles(const DM2_V1_GuiVpCallbacks *cb, void *ctx);
void dm2_v1_chance_table_operation(const DM2_V1_GuiVpCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GUI_VP_PC34_COMPAT_H */
